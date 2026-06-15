# Copy as WSL Path — Right-Click Context Menu

**Date:** 2026-06-14
**Status:** Requirements (pre-planning)
**Scope tier:** Standard (greenfield)

## Problem & Outcome

Developers who work across Windows Explorer and WSL constantly need a file/folder's
path in WSL form (`/mnt/c/...`) to paste into a WSL terminal, script, or config.
Today this is manual: copy the Windows path, then hand-translate the drive letter and
slashes — error-prone, and breaks for network/UNC paths or custom mount roots.

**Outcome:** Right-click any item in Explorer → "Copy as WSL path" → correct WSL path
is on the clipboard, ready to paste.

## Users

Primary: developers running WSL on Windows who move between Explorer and a WSL shell.
Single-user desktop utility; no multi-user or server concern.

## Behavior / Requirements

- **R1 — Menu targets.** The entry appears on:
  - Files
  - Folders
  - Folder background (right-click empty space → copies the open folder's path)
  - Drives (e.g. `C:` → `/mnt/c`)
- **R2 — Label.** Menu text: `Copy as WSL path`.
- **R3 — Conversion.** Compute the WSL path by shelling out to `wsl wslpath -a '<windows-path>'`.
  This delegates correctness to WSL itself: handles custom `/etc/wsl.conf` automount roots,
  UNC/network shares, and mapped drives, rather than hardcoding `/mnt/<letter>`.
- **R4 — Multi-select.** When several items are selected, copy **all** their WSL paths,
  one per line (newline-joined), in selection order.
- **R5 — Clipboard.** Result is placed on the Windows clipboard as plain text.
- **R6 — Menu mechanism (dual).**
  - **Win11:** top-level main-menu entry via a modern `IExplorerCommand` COM handler,
    registered through a **sparse MSIX package**.
  - **Win10 (and Win11 "Show more options"):** classic shell verbs under `HKCU`
    registry keys as a fallback.

## Scope Boundaries

**In scope:** the 4 target types (R1), wslpath-based conversion (R3),
newline-joined multi-select (R4), dual menu mechanism (R6).

**Deferred for later:**
- Language / runtime choice for the COM handler and CLI helper → decide in planning.
- Code-signing / distribution pipeline (MSIX signing, installer).
- Any settings/config UI (e.g. choosing distro, quote style, separator).
- "Open in WSL terminal here" or reverse (WSL→Windows) conversion.

## Dependencies / Assumptions

- **WSL installed.** `wsl wslpath` requires WSL present. Behavior when WSL is absent
  (hide the entry vs. show an error toast vs. silent no-op) is a **planning decision**.
- Default target distro is whatever `wsl` resolves to; per-distro selection is deferred.
- Modern Win11 entry requires the sparse-package registration path; classic registry
  alone is hidden under "Show more options" on Win11 (the reason R6 is dual).

## Success Criteria

- Right-clicking any of the 4 target types shows "Copy as WSL path".
- Clicking it on a single item yields the same string as running
  `wsl wslpath -a` on that item's full path.
- Multi-select yields N correct lines.
- Entry appears in the Win11 **main** context menu (not only "Show more options")
  and in the Win10 context menu.

## Open Questions (for planning)

1. Language/stack for the handler (C++ native COM vs C# + CsWin32 vs Rust) — affects
   runtime dependencies and packaging.
2. No-WSL fallback behavior.
3. How paths are passed to the handler for multi-select (verb command line vs COM
   `IShellItemArray`) — the modern handler gets the array natively; the classic verb
   path may need per-item invocation + aggregation.
