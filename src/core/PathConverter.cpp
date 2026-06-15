#include "PathConverter.h"

#include <cwctype>

namespace wslpath {

namespace {

void ReplaceBackslashes(std::wstring& s) {
    for (wchar_t& c : s) {
        if (c == L'\\') c = L'/';
    }
}

void TrimTrailingSlashes(std::wstring& s) {
    while (!s.empty() && s.back() == L'/') s.pop_back();
}

bool IsDrivePath(const std::wstring& p) {
    return p.size() >= 2 && std::iswalpha(p[0]) && p[1] == L':';
}

bool IsUncPath(const std::wstring& p) {
    return p.size() >= 2 &&
           (p[0] == L'\\' || p[0] == L'/') &&
           (p[1] == L'\\' || p[1] == L'/');
}

}  // namespace

std::wstring PathConverter::StringConvert(const std::wstring& winPath) {
    if (winPath.empty()) return std::wstring();

    if (IsDrivePath(winPath)) {
        wchar_t letter = static_cast<wchar_t>(std::towlower(winPath[0]));
        std::wstring rest = winPath.substr(2);  // everything after "X:"
        ReplaceBackslashes(rest);
        TrimTrailingSlashes(rest);

        std::wstring out = L"/mnt/";
        out += letter;
        if (rest.empty()) {
            out += L'/';  // drive root: C:\ -> /mnt/c/
        } else {
            if (rest.front() != L'/') out += L'/';
            out += rest;
        }
        return out;
    }

    // UNC or anything else: best-effort slash normalization.
    std::wstring out = winPath;
    ReplaceBackslashes(out);
    if (!IsUncPath(winPath)) {
        // Relative / unknown form: normalize slashes, trim trailing.
        TrimTrailingSlashes(out);
    } else {
        TrimTrailingSlashes(out);
    }
    return out;
}

std::wstring PathConverter::ToWslPath(const std::wstring& winPath) const {
    if (winPath.empty()) return std::wstring();

    WslPathResult r = runner_.RunWslPath(winPath);
    if (r.launched && r.exitCode == 0 && !r.output.empty()) {
        return r.output;
    }
    return StringConvert(winPath);
}

std::wstring PathConverter::ToWslPathsJoined(
    const std::vector<std::wstring>& winPaths) const {
    std::wstring joined;
    bool first = true;
    for (const auto& p : winPaths) {
        if (!first) joined += L'\n';
        joined += ToWslPath(p);
        first = false;
    }
    return joined;
}

}  // namespace wslpath
