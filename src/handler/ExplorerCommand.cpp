#include "ExplorerCommand.h"

#include <shlwapi.h>

#include <string>
#include <vector>

#include "Clipboard.h"
#include "PathConverter.h"
#include "WslProcess.h"

using namespace Microsoft::WRL;

namespace wslpath {

namespace {

constexpr wchar_t kTitle[] = L"Copy as WSL path";

// Duplicate a wide string into a CoTaskMem buffer (ownership passes to caller).
HRESULT CloneToCoTaskMem(const wchar_t* src, PWSTR* out) {
    return SHStrDupW(src, out);
}

// Collect filesystem paths from the selection. Items without a real
// filesystem path (virtual shell items) are skipped.
std::vector<std::wstring> CollectPaths(IShellItemArray* items) {
    std::vector<std::wstring> paths;
    if (!items) return paths;

    DWORD count = 0;
    if (FAILED(items->GetCount(&count))) return paths;

    for (DWORD i = 0; i < count; ++i) {
        IShellItem* item = nullptr;
        if (FAILED(items->GetItemAt(i, &item)) || !item) continue;

        PWSTR path = nullptr;
        if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path)) && path) {
            paths.emplace_back(path);
            CoTaskMemFree(path);
        }
        item->Release();
    }
    return paths;
}

}  // namespace

IFACEMETHODIMP CopyAsWslPathCommand::GetTitle(IShellItemArray*, PWSTR* name) {
    if (!name) return E_POINTER;
    return CloneToCoTaskMem(kTitle, name);
}

IFACEMETHODIMP CopyAsWslPathCommand::GetIcon(IShellItemArray*, PWSTR* icon) {
    if (!icon) return E_POINTER;
    *icon = nullptr;
    return E_NOTIMPL;  // no custom icon for v1
}

IFACEMETHODIMP CopyAsWslPathCommand::GetToolTip(IShellItemArray*, PWSTR* infoTip) {
    if (!infoTip) return E_POINTER;
    *infoTip = nullptr;
    return E_NOTIMPL;
}

IFACEMETHODIMP CopyAsWslPathCommand::GetCanonicalName(GUID* guidCommandName) {
    if (!guidCommandName) return E_POINTER;
    *guidCommandName = CLSID_CopyAsWslPathCommand;
    return S_OK;
}

IFACEMETHODIMP CopyAsWslPathCommand::GetState(IShellItemArray* items, BOOL,
                                              EXPCMDSTATE* cmdState) {
    if (!cmdState) return E_POINTER;
    // Enabled whenever there is a selection (files, folders, drives). The
    // folder-background case passes a null/empty array but is still valid.
    *cmdState = ECS_ENABLED;
    (void)items;
    return S_OK;
}

IFACEMETHODIMP CopyAsWslPathCommand::Invoke(IShellItemArray* items, IBindCtx*) {
    std::vector<std::wstring> paths = CollectPaths(items);
    if (paths.empty()) return S_OK;  // nothing convertible; no-op

    WslRunner runner;
    PathConverter converter(runner);
    std::wstring joined = converter.ToWslPathsJoined(paths);

    Clipboard::SetText(joined);
    return S_OK;
}

IFACEMETHODIMP CopyAsWslPathCommand::GetFlags(EXPCMDFLAGS* flags) {
    if (!flags) return E_POINTER;
    *flags = ECF_DEFAULT;
    return S_OK;
}

IFACEMETHODIMP CopyAsWslPathCommand::EnumSubCommands(IEnumExplorerCommand** enumCommands) {
    if (!enumCommands) return E_POINTER;
    *enumCommands = nullptr;
    return E_NOTIMPL;  // not a flyout
}

}  // namespace wslpath
