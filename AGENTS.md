# Repository Guidelines

## Project Structure & Module Organization

- `src/core/` contains shared path conversion, WSL process, and clipboard logic in the `wslpath` namespace.
- `src/handler/` builds the modern Windows 11 `IExplorerCommand` COM DLL.
- `src/cli/` builds the classic Windows 10 helper executable.
- `test/core_tests.cpp` contains dependency-free unit tests for deterministic core behavior.
- `packaging/` holds the sparse MSIX manifest, PowerShell registration scripts, registry files, and package assets.
- `docs/` records requirements and implementation plans. Build output belongs in `build/` and should not be committed.

## Build, Test, and Development Commands

Run commands from a Visual Studio 2022 Developer PowerShell on Windows:

```powershell
cmake -B build -A x64 -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The build produces `build\Release\WslPathHandler.dll`, `wslpathcopy.exe`, and `core_tests.exe`. Install the development package and registry verbs with `./packaging/register.ps1`; remove them with `./packaging/unregister.ps1`. This project requires MSVC and the Windows 10/11 SDK and intentionally rejects non-Windows builds.

## Coding Style & Naming Conventions

Use C++17 and match the existing style: four-space indentation, braces on the same line, and concise comments only for non-obvious behavior. Use `PascalCase` for classes and functions (`PathConverter`, `ToWslPath`), `camelCase` for parameters and locals (`winPath`, `exitCode`), and trailing underscores for private members. Keep headers beside their implementations and use wide strings for Windows paths. No formatter or linter is configured, so preserve surrounding formatting and keep warning-free MSVC builds.

## Testing Guidelines

Add focused checks to `test/core_tests.cpp` using the existing `ExpectEq` helper and fake `IWslRunner`. Cover success, fallback, empty input, and aggregation edge cases. CTest is the automated test entry point; clipboard, Explorer integration, and real `wsl.exe` behavior require the manual verification steps in `README.md`.

## Commit & Pull Request Guidelines

History follows scoped Conventional Commits, for example `feat(core): add path conversion` and `fix(packaging): select newest SDK`. Keep each commit focused on one subsystem. Pull requests should explain user-visible behavior, list automated and manual verification, link relevant issues or plans, and include screenshots for Explorer menu or packaging UI changes. Call out Windows-version-specific behavior and any registry, certificate, or installation impact.
