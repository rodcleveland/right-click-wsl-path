<#
.SYNOPSIS
  Build outputs are signed with a dev self-signed cert, then registered as a
  sparse package so the Win11 main-menu "Copy as WSL path" entry appears.
  Also installs the classic HKCU shell verbs (Win10 / "Show more options").

.DESCRIPTION
  Run from an elevated-or-normal PowerShell after building the solution. The
  sparse package references the built binaries in-place via -ExternalLocation,
  so no repackaging is needed on each rebuild.

.PARAMETER BuildDir
  Directory containing the built WslPathHandler.dll and wslpathcopy.exe
  (e.g. build\Release or build\Debug). Defaults to .\build\Release.

.PARAMETER SkipClassic
  Skip installing the classic HKCU shell verbs.
#>
param(
    [string]$BuildDir = (Join-Path $PSScriptRoot "..\build\Release"),
    [switch]$SkipClassic
)

$ErrorActionPreference = "Stop"
$packagingDir = $PSScriptRoot
$certSubject = "CN=RightClickWslPath Dev"   # must match AppxManifest Publisher

$BuildDir = (Resolve-Path $BuildDir).Path
$handlerDll = Join-Path $BuildDir "WslPathHandler.dll"
$cliExe     = Join-Path $BuildDir "wslpathcopy.exe"

if (-not (Test-Path $handlerDll)) { throw "Handler DLL not found: $handlerDll. Build the solution first." }
if (-not (Test-Path $cliExe))     { throw "CLI helper not found: $cliExe. Build the solution first." }

# --- 1. Ensure a dev signing cert exists (self-signed, in CurrentUser\My) ---
$cert = Get-ChildItem Cert:\CurrentUser\My | Where-Object { $_.Subject -eq $certSubject } | Select-Object -First 1
if (-not $cert) {
    Write-Host "Creating dev signing certificate ($certSubject)..."
    $cert = New-SelfSignedCertificate `
        -Type Custom -Subject $certSubject `
        -KeyUsage DigitalSignature -FriendlyName "RightClickWslPath Dev" `
        -CertStoreLocation "Cert:\CurrentUser\My" `
        -TextExtension @("2.5.29.37={text}1.3.6.1.5.5.7.3.3", "2.5.29.19={text}")
}

# Trust the cert so Windows will accept the sparse package signature.
$cerPath = Join-Path $packagingDir "DevCert.cer"
Export-Certificate -Cert $cert -FilePath $cerPath | Out-Null
Import-Certificate -FilePath $cerPath -CertStoreLocation "Cert:\LocalMachine\TrustedPeople" -ErrorAction SilentlyContinue | Out-Null
Write-Host "Dev certificate ready and trusted (TrustedPeople)."

# --- 2. Build the sparse .msix from the manifest ---
# Find the newest SDK bin that actually contains the tools. The version lives
# in the PARENT directory name (e.g. ...\bin\10.0.26100.0\x64), so sort the
# version dirs by [version] -- sorting the leaf "x64" dirs by Name is wrong
# (every leaf is named "x64") and picks an arbitrary SDK that may lack the tools.
$sdkRoot = "${env:ProgramFiles(x86)}\Windows Kits\10\bin"
$makeAppx = $null
$signTool = $null
if (Test-Path $sdkRoot) {
    $versionDirs = Get-ChildItem $sdkRoot -Directory |
        Where-Object { $_.Name -match '^\d+\.\d+\.\d+\.\d+$' } |
        Sort-Object { [version]$_.Name } -Descending
    foreach ($v in $versionDirs) {
        $candidate = Join-Path $v.FullName "x64\makeappx.exe"
        if (Test-Path $candidate) {
            $makeAppx = $candidate
            $signTool = Join-Path $v.FullName "x64\signtool.exe"
            break
        }
    }
}
if (-not $makeAppx) {
    throw "makeappx.exe not found under $sdkRoot\<version>\x64. Install the Windows 10/11 SDK (with the signing/packaging tools)."
}
Write-Host "Using SDK tools: $makeAppx"

$msixPath = Join-Path $packagingDir "RightClickWslPath.msix"

# Sparse package: pack only the manifest (and Assets if present); binaries are
# referenced externally at registration time.
$packLayout = Join-Path $env:TEMP "rcwp_sparse_layout"
if (Test-Path $packLayout) { Remove-Item $packLayout -Recurse -Force }
New-Item -ItemType Directory -Path $packLayout | Out-Null
Copy-Item (Join-Path $packagingDir "AppxManifest.xml") $packLayout
if (Test-Path (Join-Path $packagingDir "Assets")) {
    Copy-Item (Join-Path $packagingDir "Assets") (Join-Path $packLayout "Assets") -Recurse
}

Write-Host "Packing sparse MSIX..."
& $makeAppx pack /d $packLayout /p $msixPath /nv /o
if ($LASTEXITCODE -ne 0) { throw "makeappx pack failed ($LASTEXITCODE)." }

Write-Host "Signing sparse MSIX..."
& $signTool sign /fd SHA256 /a /s "My" /n "RightClickWslPath Dev" $msixPath
if ($LASTEXITCODE -ne 0) { throw "signtool sign failed ($LASTEXITCODE)." }

# --- 3. Register with the external binary location ---
Write-Host "Registering sparse package (external location: $BuildDir)..."
Add-AppxPackage -Path $msixPath -ExternalLocation $BuildDir
Write-Host "Modern (Win11) context-menu handler registered."

# --- 4. Classic HKCU shell verbs (Win10 / Show more options) ---
if (-not $SkipClassic) {
    & (Join-Path $packagingDir "install-classic.ps1") -CliExe $cliExe
}

Write-Host ""
Write-Host "Done. Restart Explorer if the entry does not appear immediately:"
Write-Host "  Stop-Process -Name explorer -Force"
