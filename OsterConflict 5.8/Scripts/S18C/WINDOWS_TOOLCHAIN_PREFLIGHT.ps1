param([string]$UERoot=$env:UE_ROOT)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
function Die([string]$m){ throw "[S18C] $m" }
if([string]::IsNullOrWhiteSpace($UERoot)){Die 'UE_ROOT is not set. Point it at the UE 5.8 installation root.'}
$UERoot=(Resolve-Path $UERoot).Path
$BuildVersion=Join-Path $UERoot 'Engine\Build\Build.version'
$BuildBat=Join-Path $UERoot 'Engine\Build\BatchFiles\Build.bat'
$RunUAT=Join-Path $UERoot 'Engine\Build\BatchFiles\RunUAT.bat'
$EditorCmd=Join-Path $UERoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
foreach($p in @($BuildVersion,$BuildBat,$RunUAT,$EditorCmd)){if(-not(Test-Path $p)){Die "missing $p"}}
$v=Get-Content $BuildVersion -Raw | ConvertFrom-Json
if($v.MajorVersion -ne 5 -or $v.MinorVersion -ne 8){Die "Expected UE 5.8, found $($v.MajorVersion).$($v.MinorVersion)"}
Write-Host "UE: $($v.MajorVersion).$($v.MinorVersion).$($v.PatchVersion)" -ForegroundColor Green
# Python is optional for source audit helpers; UBT/MSVC compilation does not require it.
$py=Get-Command python -ErrorAction SilentlyContinue
if($py){ Write-Host 'Python: available (optional audit helpers enabled)' -ForegroundColor Green }
else { Write-Warning 'Python not found; optional audit helpers will be skipped by the outer test harness.' }
$vswhere="${Env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if(Test-Path $vswhere){
  $vs=& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
  if($vs){Write-Host "MSVC toolchain: $vs" -ForegroundColor Green}else{Die 'Visual Studio/MSVC C++ workload not found'}
}else{Write-Warning 'vswhere.exe not found; UBT will perform the authoritative compiler check.'}
Write-Host 'Querying UBT targets...' -ForegroundColor Cyan
$ProjectRoot=(Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$ProjectPath=Join-Path $ProjectRoot 'OsterConflict.uproject'
& $BuildBat '-Mode=QueryTargets' "-Project=$ProjectPath" | Out-Host
if($LASTEXITCODE -ne 0){ Write-Warning "Target query via Build.bat returned $LASTEXITCODE; compile stage remains authoritative." }
Write-Host 'WINDOWS_TOOLCHAIN_PREFLIGHT: PASS' -ForegroundColor Green
