#pragma once

#include <optional>
#include <string>

namespace wslpath {

// Result of attempting to run `wsl.exe wslpath -a <path>`.
struct WslPathResult {
    bool launched = false;     // did wsl.exe start at all?
    int exitCode = -1;         // process exit code (valid when launched)
    std::wstring output;       // trimmed stdout (the converted path)
};

// Abstraction over invoking `wsl.exe wslpath`, so conversion logic is
// unit-testable without a real WSL install. Inject a fake in tests.
class IWslRunner {
public:
    virtual ~IWslRunner() = default;
    // Runs `wsl.exe wslpath -a <winPath>` and returns the captured result.
    virtual WslPathResult RunWslPath(const std::wstring& winPath) = 0;
};

// Production runner: launches wsl.exe with CreateProcess, redirected stdout,
// no visible window. Returns launched=false when wsl.exe cannot be started
// (e.g. WSL not installed).
class WslRunner : public IWslRunner {
public:
    WslPathResult RunWslPath(const std::wstring& winPath) override;
};

}  // namespace wslpath
