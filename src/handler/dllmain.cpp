#include <windows.h>
#include <wrl/module.h>

#include "ExplorerCommand.h"

using namespace Microsoft::WRL;
using wslpath::CopyAsWslPathCommand;

// Register the command class with the WRL module so the framework provides the
// class factory. Activation is via DllGetClassObject (classic COM, in-proc).
// Note: CoCreatableClass pastes the type name into generated identifiers, so it
// must be given an unqualified name (hence the using-declaration above).
CoCreatableClass(CopyAsWslPathCommand);

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(module);
            break;
        default:
            break;
    }
    return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv) {
    return Module<InProc>::GetModule().GetClassObject(rclsid, riid, ppv);
}

STDAPI DllCanUnloadNow() {
    return Module<InProc>::GetModule().GetObjectCount() == 0 ? S_OK : S_FALSE;
}
