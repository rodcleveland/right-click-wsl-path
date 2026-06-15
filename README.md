# Copy as WSL Path

Adds a **"Copy as WSL path"** entry to the Windows Explorer right-click menu for
files, folders, the folder background, and drives. Selecting it converts each
selected Windows path to its WSL form (e.g. `C:\Users\you\proj` →
`/mnt/c/Users/you/proj`) and copies the result to the clipboard, ready to paste
into a WSL terminal, script, or config.

- **Win11:** appears in the main context menu (modern `IExplorerCommand` handler).
- **Win10 / Win11 "Show more options":** classic registry shell verbs.
- **Conversion:** uses `wsl.exe wslpath -a` for correctness (UNC paths, mapped
  drives, custom `/etc/wsl.conf` mount roots). Falls back to in-process string
  conversion when WSL is not installed.
- **Multi-select:** the Win11 handler copies all selected paths, newline-joined.
  See [Known limitations](#known-limitations).

## Requirements

- Windows 10 2004 (build 19041) or later.
- Visual Studio 2022 with the **Desktop development with C++** workload, or the
  Windows 10/11 SDK + CMake.
- WSL is optional at runtime — without it the string-conversion fallback is used.

## Build

```powershell
cmake -B build -A x64
cmake --build build --config Release
```

Outputs (in `build\Release`):
- `WslPathHandler.dll` — modern Win11 COM handler
- `wslpathcopy.exe` — classic Win10 helper

### Run the unit tests

```powershell
cmake -B build -A x64 -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release
```

The tests cover path conversion and multi-path aggregation. Clipboard and the
real `wsl.exe` invocation are verified manually (see below) since they need a
live Windows session.

## Install

From the `packaging` directory, after building:

```powershell
.\packaging\register.ps1            # installs modern (Win11) + classic (Win10)
```

This will:
1. Create a self-signed dev certificate (`CN=RightClickWslPath Dev`) and add it
   to **Trusted People** so Windows accepts the sparse package signature.
2. Pack + sign a sparse `.msix` and register it with `-ExternalLocation`
   pointing at your build output (binaries are used in place — no repackaging on
   rebuild).
3. Install the classic `HKCU` shell verbs pointing at `wslpathcopy.exe`.

Options:
- `-BuildDir <path>` — point at a non-default build output (default
  `build\Release`).
- `-SkipClassic` — modern handler only.

If the entry does not show immediately, restart Explorer:

```powershell
Stop-Process -Name explorer -Force
```

### Classic-only via registry import

If you only want the Win10-style entry without packaging, edit
`packaging\classic-install.reg` (replace the placeholder exe path) and import it.

## Uninstall

```powershell
.\packaging\unregister.ps1
```

Removes the sparse package and the classic shell verbs.

## Known limitations

- **Win10 multi-select copies per-item.** Classic shell verbs invoke the command
  once per selected item, so selecting several items and clicking the entry
  copies them individually (last one wins on the clipboard). True newline-joined
  multi-select is only guaranteed on the modern Win11 handler, which receives the
  whole selection at once.
- **UNC paths without WSL.** When WSL is unavailable, the string fallback for UNC
  paths (`\\server\share`) is best-effort (`//server/share`); `wslpath` is the
  correct path for these and is used whenever WSL is present.
- **Placeholder icons.** `packaging/Assets/*.png` are 1×1 placeholders so the
  package builds. Replace them with real logos before any real distribution.

## Project layout

```
src/core/      shared conversion + clipboard logic (static lib)
src/handler/   modern Win11 IExplorerCommand COM DLL
src/cli/       classic Win10 helper exe
packaging/     MSIX manifest, register/unregister scripts, classic .reg
test/          unit tests for the conversion core
docs/          brainstorm + plan
```

## Verifying manually

1. `wslpathcopy.exe "C:\Windows"` → clipboard should contain `/mnt/c/Windows`.
2. After `register.ps1`, right-click a file / folder / drive / folder background
   on Win11 → **Copy as WSL path** appears in the main menu → click → paste to
   confirm the converted path(s).
3. Confirm the same on Win10 (or under "Show more options" on Win11).
