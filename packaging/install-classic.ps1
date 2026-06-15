<#
.SYNOPSIS
  Installs the classic "Copy as WSL path" HKCU shell verbs that invoke the CLI
  helper. Used on Windows 10, and on Windows 11 under "Show more options".

.DESCRIPTION
  Registers the verb for the four target types:
    *                     -> files
    Directory             -> folders
    Directory\Background   -> folder background (uses %V = the open folder)
    Drive                 -> drives

  Per the plan (KTD4), classic shell verbs invoke the command once per selected
  item, so multi-select copies per-item rather than newline-joined. The modern
  Win11 handler aggregates the full selection.

.PARAMETER CliExe
  Absolute path to wslpathcopy.exe.
#>
param(
    [Parameter(Mandatory = $true)]
    [string]$CliExe
)

$ErrorActionPreference = "Stop"
$CliExe = (Resolve-Path $CliExe).Path
$label = "Copy as WSL path"
$verb = "CopyAsWslPath"

# itemTypes maps the HKCU\Software\Classes subkey to the command argument token.
# %1 = the clicked item's path; %V = the background folder path.
$itemTypes = @(
    @{ Key = "*";                    Arg = "%1" },
    @{ Key = "Directory";            Arg = "%1" },
    @{ Key = "Directory\Background"; Arg = "%V" },
    @{ Key = "Drive";                Arg = "%1" }
)

foreach ($t in $itemTypes) {
    $shellKey = "HKCU:\Software\Classes\$($t.Key)\shell\$verb"
    $cmdKey   = "$shellKey\command"

    New-Item -Path $shellKey -Force | Out-Null
    Set-ItemProperty -Path $shellKey -Name "(Default)" -Value $label
    # Icon: point at the CLI exe (no custom icon resource yet).
    Set-ItemProperty -Path $shellKey -Name "Icon" -Value "`"$CliExe`",0"

    New-Item -Path $cmdKey -Force | Out-Null
    Set-ItemProperty -Path $cmdKey -Name "(Default)" -Value "`"$CliExe`" `"$($t.Arg)`""

    Write-Host "Registered classic verb for: $($t.Key)"
}

Write-Host "Classic HKCU shell verbs installed."
