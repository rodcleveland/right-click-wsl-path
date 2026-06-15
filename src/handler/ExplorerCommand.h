#pragma once

#include <windows.h>
#include <shobjidl_core.h>
#include <wrl/implements.h>

// Stable CLSID for the "Copy as WSL path" command handler.
// Must match the AppxManifest.xml com:Class Id and any registry entries.
// {7A3D9E54-2C1B-4F8A-9E6D-1B2C3D4E5F60}
DEFINE_GUID(CLSID_CopyAsWslPathCommand,
    0x7a3d9e54, 0x2c1b, 0x4f8a, 0x9e, 0x6d, 0x1b, 0x2c, 0x3d, 0x4e, 0x5f, 0x60);

namespace wslpath {

// IExplorerCommand implementation that adds "Copy as WSL path" to the Win11
// main context menu for files, folders, the folder background, and drives.
class __declspec(uuid("7A3D9E54-2C1B-4F8A-9E6D-1B2C3D4E5F60"))
    CopyAsWslPathCommand
    : public Microsoft::WRL::RuntimeClass<
          Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>,
          IExplorerCommand> {
public:
    CopyAsWslPathCommand() = default;

    // IExplorerCommand
    IFACEMETHODIMP GetTitle(IShellItemArray* items, PWSTR* name) override;
    IFACEMETHODIMP GetIcon(IShellItemArray* items, PWSTR* icon) override;
    IFACEMETHODIMP GetToolTip(IShellItemArray* items, PWSTR* infoTip) override;
    IFACEMETHODIMP GetCanonicalName(GUID* guidCommandName) override;
    IFACEMETHODIMP GetState(IShellItemArray* items, BOOL slowOk,
                            EXPCMDSTATE* cmdState) override;
    IFACEMETHODIMP Invoke(IShellItemArray* items,
                          IBindCtx* bindCtx) override;
    IFACEMETHODIMP GetFlags(EXPCMDFLAGS* flags) override;
    IFACEMETHODIMP EnumSubCommands(IEnumExplorerCommand** enumCommands) override;
};

}  // namespace wslpath
