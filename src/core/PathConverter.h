#pragma once

#include <string>
#include <vector>

#include "WslProcess.h"

namespace wslpath {

// Converts Windows paths to their WSL equivalents.
//
// Primary strategy: shell out to `wsl.exe wslpath -a` (correct for UNC,
// mapped drives, and custom /etc/wsl.conf mount roots). Fallback: in-process
// string conversion (C:\Users\x -> /mnt/c/Users/x) when WSL is unavailable or
// the wslpath call fails, so the feature always produces something usable.
class PathConverter {
public:
    explicit PathConverter(IWslRunner& runner) : runner_(runner) {}

    // Convert a single Windows path. Never throws; falls back to string
    // conversion on any wslpath failure.
    std::wstring ToWslPath(const std::wstring& winPath) const;

    // Convert many paths and join with LF ('\n'), preserving input order.
    // Targets a WSL shell, so LF (not CRLF) is intentional. No trailing
    // newline. Empty input -> empty string.
    std::wstring ToWslPathsJoined(const std::vector<std::wstring>& winPaths) const;

    // Pure in-process conversion, exposed for testing and used as the
    // fallback. Lowercases the drive letter: C:\Users -> /mnt/c/Users.
    // UNC paths (\\server\share) are converted best-effort to //server/share.
    static std::wstring StringConvert(const std::wstring& winPath);

private:
    IWslRunner& runner_;
};

}  // namespace wslpath
