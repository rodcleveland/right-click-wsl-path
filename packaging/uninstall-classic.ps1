<#
.SYNOPSIS
  Removes the classic "Copy as WSL path" HKCU shell verbs installed by
  install-classic.ps1.
#>
$ErrorActionPreference = "Continue"
$verb = "CopyAsWslPath"

$keys = @("*", "Directory", "Directory\Background", "Drive")
foreach ($k in $keys) {
    $shellKey = "HKCU:\Software\Classes\$k\shell\$verb"
    if (Test-Path $shellKey) {
        Remove-Item -Path $shellKey -Recurse -Force
        Write-Host "Removed classic verb for: $k"
    }
}
Write-Host "Classic HKCU shell verbs removed."
