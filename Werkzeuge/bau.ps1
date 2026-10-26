param([string]$Config = "G2A MD Release", [string]$Copy = "")
# Baut die Coop-DLL (Kern\GothicCoop.vcxproj) mit den Visual Studio 2019 Build Tools.
#   -Config "G2A MD Release"   Gothic 2 Die Nacht des Raben
#   -Config "G1 MD Release"    Gothic 1 (auch Othello)
#   -Copy <Datei>              die fertige DLL dorthin kopieren (relativ zum Repo, z. B. Bau\Gothic2Coop.dll)
# Alle Konfigurationen linken nach Kern\Bin\GothicCoop.dll - darum vorher entfernen und danach pruefen, dass sie neu ist.
$root = Split-Path -Parent $PSScriptRoot
$msb = $env:MSBUILD
if (-not $msb) { $msb = "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild.exe" }
Set-Location (Join-Path $root "Kern")
$dll = "Bin\GothicCoop.dll"
if (Test-Path $dll) { Remove-Item $dll -Force }
$start = Get-Date
$out = & $msb GothicCoop.vcxproj /m /nologo /v:m "/p:Configuration=$Config" /p:Platform=Win32 /p:PreBuildEventUseInBuild=False /p:PostBuildEventUseInBuild=False 2>&1
$err = $out | Select-String -Pattern ": (fatal )?error " | ForEach-Object { $_.Line } | Sort-Object -Unique
"[$Config] Fehler: " + $err.Count
$err | Select-Object -First 60
if (-not (Test-Path $dll) -or (Get-Item $dll).LastWriteTime -lt $start) {
    "[$Config] $dll nicht neu gelinkt"
    exit 1
}
if ($err.Count -eq 0 -and $Copy) {
    $target = Join-Path $root $Copy
    New-Item -ItemType Directory -Force (Split-Path $target) | Out-Null
    Copy-Item $dll $target -Force
    "kopiert nach $target"
}
