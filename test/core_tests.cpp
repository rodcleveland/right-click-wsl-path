// Lightweight, dependency-free test harness for the conversion core.
// Build with BUILD_TESTING=ON; run the `core_tests` executable (CTest).
//
// Clipboard round-trip and Win32 process tests require a Windows session and
// are intentionally NOT covered here (they live as manual verification in the
// plan). These tests cover pure conversion + aggregation logic, which is the
// part that benefits most from fast, deterministic checks.

#include <cstdio>
#include <string>
#include <vector>

#include "PathConverter.h"
#include "WslProcess.h"

using namespace wslpath;

namespace {

int g_failures = 0;
int g_checks = 0;

std::string Narrow(const std::wstring& w) {
    std::string s;
    s.reserve(w.size());
    for (wchar_t c : w) s += (c < 128) ? static_cast<char>(c) : '?';
    return s;
}

void ExpectEq(const std::wstring& actual, const std::wstring& expected,
              const char* what) {
    ++g_checks;
    if (actual != expected) {
        ++g_failures;
        std::printf("FAIL: %s\n  expected: \"%s\"\n  actual:   \"%s\"\n",
                    what, Narrow(expected).c_str(), Narrow(actual).c_str());
    }
}

// Fake runner: simulates wsl.exe behavior for deterministic tests.
class FakeRunner : public IWslRunner {
public:
    bool launched = true;
    int exitCode = 0;
    std::wstring output;       // returned verbatim when launched && exit==0
    bool useEcho = false;      // if true, echo a custom-mount transform

    WslPathResult RunWslPath(const std::wstring& winPath) override {
        WslPathResult r;
        r.launched = launched;
        r.exitCode = exitCode;
        if (launched && exitCode == 0) {
            r.output = useEcho ? (L"/echo" + winPath) : output;
        }
        return r;
    }
};

}  // namespace

int main() {
    // ---- StringConvert (fallback path) ----
    ExpectEq(PathConverter::StringConvert(L"C:\\Users\\foo\\bar"),
             L"/mnt/c/Users/foo/bar", "string: basic path");
    ExpectEq(PathConverter::StringConvert(L"D:\\Temp"),
             L"/mnt/d/Temp", "string: lowercases drive");
    ExpectEq(PathConverter::StringConvert(L"C:\\Users\\foo\\"),
             L"/mnt/c/Users/foo", "string: trims trailing backslash");
    ExpectEq(PathConverter::StringConvert(L"C:\\Program Files\\x"),
             L"/mnt/c/Program Files/x", "string: preserves spaces");
    ExpectEq(PathConverter::StringConvert(L"C:\\"),
             L"/mnt/c/", "string: drive root keeps slash");
    ExpectEq(PathConverter::StringConvert(L""),
             L"", "string: empty input");
    ExpectEq(PathConverter::StringConvert(L"\\\\server\\share"),
             L"//server/share", "string: UNC best-effort");

    // ---- ToWslPath: wslpath wins when it succeeds ----
    {
        FakeRunner fake;
        fake.output = L"/custom/mount/foo";
        PathConverter conv(fake);
        ExpectEq(conv.ToWslPath(L"C:\\anything"),
                 L"/custom/mount/foo", "wslpath output wins over string convert");
    }

    // ---- ToWslPath: fallback when wsl not launched ----
    {
        FakeRunner fake;
        fake.launched = false;
        PathConverter conv(fake);
        ExpectEq(conv.ToWslPath(L"C:\\Users\\foo"),
                 L"/mnt/c/Users/foo", "fallback when wsl absent");
    }

    // ---- ToWslPath: fallback when wslpath exits non-zero ----
    {
        FakeRunner fake;
        fake.launched = true;
        fake.exitCode = 1;
        PathConverter conv(fake);
        ExpectEq(conv.ToWslPath(L"C:\\Users\\foo"),
                 L"/mnt/c/Users/foo", "fallback when wslpath fails");
    }

    // ---- ToWslPathsJoined ----
    {
        FakeRunner fake;
        fake.launched = false;  // force deterministic string conversion
        PathConverter conv(fake);

        ExpectEq(conv.ToWslPathsJoined({L"C:\\a", L"D:\\b"}),
                 L"/mnt/c/a\n/mnt/d/b", "join: two paths, order preserved");
        ExpectEq(conv.ToWslPathsJoined({L"C:\\only"}),
                 L"/mnt/c/only", "join: single path no trailing newline");
        ExpectEq(conv.ToWslPathsJoined({}),
                 L"", "join: empty list");
    }

    // ---- ToWslPathsJoined: mixed success/failure ----
    {
        // First path: wslpath succeeds; second: simulate via separate runners
        // is awkward with one fake, so verify the failure-applies-fallback
        // path through a non-launching runner above. Here confirm a launched,
        // echoing runner aggregates both.
        FakeRunner fake;
        fake.useEcho = true;
        PathConverter conv(fake);
        ExpectEq(conv.ToWslPathsJoined({L"X", L"Y"}),
                 L"/echoX\n/echoY", "join: aggregates wslpath outputs");
    }

    if (g_failures == 0) {
        std::printf("OK: %d checks passed\n", g_checks);
        return 0;
    }
    std::printf("\n%d/%d checks FAILED\n", g_failures, g_checks);
    return 1;
}
