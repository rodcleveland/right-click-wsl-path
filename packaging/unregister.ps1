<#
.SYNOPSIS
  Removes the sparse package (modern Win11 handler) and the classic HKCU shell
  verbs installed by register.ps1.
#>
param(
    [switch]$SkipClassic
)

$ErrorActionPreference = "Continue"
$packagingDir = $PSScriptRoot

# --- Remove sparse package ---
$pkg = Get-AppxPackage -Name "RightClickWslPath" -ErrorAction SilentlyContinue
if ($pkg) {
    Write-Host "Removing sparse package $($pkg.PackageFullName)..."
    Remove-AppxPackage -Package $pkg.PackageFullName
} else {
    Write-Host "Sparse package not installed."
}

# --- Remove classic HKCU shell verbs ---
if (-not $SkipClassic) {
    & (Join-Path $packagingDir "uninstall-classic.ps1")
}

Write-Host "Done. Restart Explorer to clear cached menu entries if needed:"
Write-Host "  Stop-Process -Name explorer -Force"
