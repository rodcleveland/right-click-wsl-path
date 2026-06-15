// wslpathcopy.exe — classic-registration helper.
//
// Invoked by HKCU shell verbs: `wslpathcopy.exe "<path>" ["<path>" ...]`.
// Converts each argument to its WSL path and copies the LF-joined result to
// the clipboard. Built as a GUI-subsystem app (no console window flash);
// arguments come from the command line, not a console.

#include <windows.h>
#include <shellapi.h>

#include <string>
#include <vector>

#include "Clipboard.h"
#include "PathConverter.h"
#include "WslProcess.h"

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) return 2;

    std::vector<std::wstring> paths;
    for (int i = 1; i < argc; ++i) {  // skip argv[0] (exe path)
        if (argv[i] && argv[i][0] != L'\0') {
            paths.emplace_back(argv[i]);
        }
    }
    LocalFree(argv);

    if (paths.empty()) return 1;  // no paths supplied

    wslpath::WslRunner runner;
    wslpath::PathConverter converter(runner);
    std::wstring joined = converter.ToWslPathsJoined(paths);

    return wslpath::Clipboard::SetText(joined) ? 0 : 3;
}
