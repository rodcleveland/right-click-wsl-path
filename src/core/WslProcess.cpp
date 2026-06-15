#include "WslProcess.h"

#include <windows.h>

#include <vector>

namespace wslpath {

namespace {

// Trim trailing CR/LF and surrounding whitespace from wslpath output.
std::wstring TrimLine(const std::wstring& s) {
    size_t begin = 0;
    size_t end = s.size();
    auto isws = [](wchar_t c) {
        return c == L' ' || c == L'\t' || c == L'\r' || c == L'\n';
    };
    while (begin < end && isws(s[begin])) ++begin;
    while (end > begin && isws(s[end - 1])) --end;
    return s.substr(begin, end - begin);
}

// Build the command line: wsl.exe wslpath -a "<winPath>".
// The path is passed as a single argument to wsl.exe, which forwards it to
// the wslpath utility. Quote it to survive spaces.
std::wstring BuildCommandLine(const std::wstring& winPath) {
    std::wstring cmd = L"wsl.exe wslpath -a \"";
    cmd += winPath;
    cmd += L"\"";
    return cmd;
}

}  // namespace

WslPathResult WslRunner::RunWslPath(const std::wstring& winPath) {
    WslPathResult result;

    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    HANDLE readPipe = nullptr;
    HANDLE writePipe = nullptr;
    if (!CreatePipe(&readPipe, &writePipe, &sa, 0)) {
        return result;  // launched=false
    }
    // The read end must not be inherited by the child.
    SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = writePipe;
    si.hStdError = writePipe;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION pi{};

    std::wstring cmd = BuildCommandLine(winPath);
    // CreateProcessW may modify the command-line buffer, so use a writable copy.
    std::vector<wchar_t> cmdBuf(cmd.begin(), cmd.end());
    cmdBuf.push_back(L'\0');

    BOOL ok = CreateProcessW(
        nullptr,
        cmdBuf.data(),
        nullptr,
        nullptr,
        TRUE,                       // inherit handles (the write pipe)
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si,
        &pi);

    // Parent no longer needs the write end; close so ReadFile sees EOF.
    CloseHandle(writePipe);

    if (!ok) {
        CloseHandle(readPipe);
        return result;  // launched=false -> caller falls back to string convert
    }

    result.launched = true;

    // Drain stdout. wslpath output is UTF-8; convert once at the end.
    std::string raw;
    char buf[512];
    DWORD read = 0;
    while (ReadFile(readPipe, buf, sizeof(buf), &read, nullptr) && read > 0) {
        raw.append(buf, read);
    }
    CloseHandle(readPipe);

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    result.exitCode = static_cast<int>(code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (!raw.empty()) {
        int wlen = MultiByteToWideChar(CP_UTF8, 0, raw.data(),
                                       static_cast<int>(raw.size()), nullptr, 0);
        if (wlen > 0) {
            std::wstring wide(wlen, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, raw.data(),
                                static_cast<int>(raw.size()), wide.data(), wlen);
            result.output = TrimLine(wide);
        }
    }

    return result;
}

}  // namespace wslpath
