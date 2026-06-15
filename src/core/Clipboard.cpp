#include "Clipboard.h"

#include <windows.h>

namespace wslpath {

namespace {

constexpr int kOpenRetries = 5;
constexpr DWORD kRetryDelayMs = 30;

bool OpenWithRetry() {
    for (int i = 0; i < kOpenRetries; ++i) {
        if (OpenClipboard(nullptr)) return true;
        Sleep(kRetryDelayMs);
    }
    return false;
}

}  // namespace

bool Clipboard::SetText(const std::wstring& text) {
    if (!OpenWithRetry()) return false;

    bool ok = false;
    if (EmptyClipboard()) {
        // Allocate including the null terminator.
        const size_t bytes = (text.size() + 1) * sizeof(wchar_t);
        HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, bytes);
        if (mem) {
            void* dst = GlobalLock(mem);
            if (dst) {
                memcpy(dst, text.c_str(), bytes);
                GlobalUnlock(mem);
                if (SetClipboardData(CF_UNICODETEXT, mem)) {
                    ok = true;  // ownership transferred to the clipboard
                } else {
                    GlobalFree(mem);
                }
            } else {
                GlobalFree(mem);
            }
        }
    }

    CloseClipboard();
    return ok;
}

}  // namespace wslpath
