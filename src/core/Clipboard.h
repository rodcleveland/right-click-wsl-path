#pragma once

#include <string>

namespace wslpath {

class Clipboard {
public:
    // Places UTF-16 text on the Windows clipboard as CF_UNICODETEXT.
    // Retries a few times if the clipboard is momentarily held by another
    // process. Returns true on success.
    static bool SetText(const std::wstring& text);
};

}  // namespace wslpath
