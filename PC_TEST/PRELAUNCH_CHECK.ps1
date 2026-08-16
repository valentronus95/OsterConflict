param(
    [Parameter(Mandatory=$true)][string]$UERoot,
    [Parameter(Mandatory=$true)][string]$ProjectRoot,
    [Parameter(Mandatory=$true)][string]$OutDir
)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$report = New-Object System.Collections.Generic.List[string]
$warnings = New-Object System.Collections.Generic.List[string]
function Pass([string]$m){$report.Add("PASS  $m"); Write-Host "PASS  $m" -ForegroundColor Green}
function Warn([string]$m){$report.Add("WARN  $m");$warnings.Add($m); Write-Warning $m}
function Fail([string]$m){$report.Add("FAIL  $m"); throw $m}

$UERoot=(Resolve-Path $UERoot).Path
$ProjectRoot=(Resolve-Path $ProjectRoot).Path
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

if([Environment]::OSVersion.Platform -ne [PlatformID]::Win32NT){ Fail 'Windows x64 is required for this R8 kit.' }
Pass "Windows: $([Environment]::OSVersion.VersionString)"

if($ProjectRoot.Length -gt 120){ Warn "Project path is long ($($ProjectRoot.Length) chars). Prefer D:\\OsterTest\\..." } else { Pass "Project path length: $($ProjectRoot.Length)" }
if($ProjectRoot -match 'OneDrive|Dropbox|Google Drive'){ Warn 'Project is inside a sync folder. Move it to a short local SSD path before compiling.' } else { Pass 'Project is not in a common sync-folder path.' }

$buildVersion=Join-Path $UERoot 'Engine\Build\Build.version'
if(!(Test-Path $buildVersion)){ Fail "Build.version missing: $buildVersion" }
$v=Get-Content $buildVersion -Raw | ConvertFrom-Json
if($v.MajorVersion -ne 5 -or $v.MinorVersion -ne 8){ Fail "UE 5.8 required; found $($v.MajorVersion).$($v.MinorVersion).$($v.PatchVersion)" }
Pass "UE version $($v.MajorVersion).$($v.MinorVersion).$($v.PatchVersion)"

$installedBuild = Test-Path (Join-Path $UERoot 'Engine\Build\InstalledBuild.txt')
foreach($rel in @('Engine\Build\BatchFiles\Build.bat','Engine\Build\BatchFiles\RunUAT.bat','Engine\Binaries\Win64\UnrealEditor-Cmd.exe')){
    $p=Join-Path $UERoot $rel
    if(!(Test-Path $p)){ Fail "Required UE 5.8 path missing: $p" }
}
if($installedBuild){
    Pass 'Launcher/installed UE 5.8 detected; source-only RunUBT.bat is not required.'
}else{
    foreach($rel in @('Engine\Build\BatchFiles\RunUBT.bat','Engine\Source')){
        $p=Join-Path $UERoot $rel
        if(!(Test-Path $p)){ Fail "Required UE source-build path missing: $p" }
    }
    Pass 'UE source-build tools and Engine\Source are present.'
}

$prereqDir=Join-Path $UERoot 'Engine\Extras\Redist\en-us'
if(Test-Path $prereqDir){ Pass 'UE prerequisite installer folder present.' } else { Warn "UE prerequisites folder not found: $prereqDir" }

$vswhere="${Env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if(!(Test-Path $vswhere)){ Fail 'vswhere.exe not found. Install Visual Studio 2022/2026 with Game development with C++.' }
$vsJson=& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -format json
if(!$vsJson){ Fail 'Visual Studio C++ toolchain not found.' }
$vs=($vsJson | ConvertFrom-Json)[0]
$vsVersion=[version]$vs.installationVersion
if($vsVersion.Major -eq 17 -and $vsVersion -lt [version]'17.14.0.0'){ Fail "UE 5.8 requires Visual Studio 2022 17.14+; found $vsVersion" }
if($vsVersion.Major -lt 17){ Fail "Unsupported Visual Studio version: $vsVersion" }
Pass "Visual Studio $vsVersion at $($vs.installationPath)"

# Windows SDK: Epic's UE 5.8 toolchain table lists 10.0.22621.0 minimum.
$sdkRoot=(Get-ItemProperty 'HKLM:\SOFTWARE\Microsoft\Windows Kits\Installed Roots' -ErrorAction SilentlyContinue).KitsRoot10
if($sdkRoot){
    $sdkVers=Get-ChildItem (Join-Path $sdkRoot 'Include') -Directory -ErrorAction SilentlyContinue | ForEach-Object { try{[version]$_.Name}catch{} } | Sort-Object -Descending
    $sdk=$sdkVers | Select-Object -First 1
    if(!$sdk){ Warn 'Could not enumerate Windows SDK versions.' }
    elseif($sdk -lt [version]'10.0.22621.0'){ Fail "Windows SDK 10.0.22621.0+ required for UE 5.8 project gate; found $sdk" }
    else { Pass "Windows SDK $sdk" }
}else{ Warn 'Windows Kits registry root not found; UBT will perform authoritative SDK detection.' }

$drive=[System.IO.DriveInfo]::new((Split-Path -Qualifier $ProjectRoot))
$freeGB=[math]::Round($drive.AvailableFreeSpace/1GB,1)
if($freeGB -lt 30){ Fail "Only $freeGB GB free on project drive. Need at least 30 GB to attempt this test safely." }
elseif($freeGB -lt 60){ Warn "Only $freeGB GB free on project drive. 60+ GB is strongly preferred for compile/cook/package evidence." }
else{ Pass "Free disk on project drive: $freeGB GB" }

$cs=Get-CimInstance Win32_ComputerSystem
$os=Get-CimInstance Win32_OperatingSystem
$cpu=Get-CimInstance Win32_Processor | Select-Object -First 1
$gpu=Get-CimInstance Win32_VideoController | Select-Object Name,DriverVersion,AdapterRAM
$ramGB=[math]::Round($cs.TotalPhysicalMemory/1GB,1)
if($ramGB -lt 16){ Warn "RAM: $ramGB GB. Build may be painful; 32 GB is the UE recommended development target." } else { Pass "RAM: $ramGB GB" }
$report.Add("INFO  CPU: $($cpu.Name)")
foreach($g in $gpu){ $report.Add("INFO  GPU: $($g.Name) driver=$($g.DriverVersion)") }

$portInUse=Get-NetUDPEndpoint -LocalPort 7777 -ErrorAction SilentlyContinue
if($portInUse){ Warn 'UDP port 7777 is already bound. Close the other server/app before packaged smoke.' } else { Pass 'UDP port 7777 is free.' }

# Capture diagnostic evidence without blocking the build if dxdiag itself fails.
try { Start-Process -FilePath 'dxdiag.exe' -ArgumentList '/whql:off','/t',(Join-Path $OutDir 'DXDIAG.txt') -Wait -WindowStyle Hidden } catch { Warn "dxdiag capture failed: $($_.Exception.Message)" }
$system=[pscustomobject]@{
    timestamp=(Get-Date).ToString('o'); ue="$($v.MajorVersion).$($v.MinorVersion).$($v.PatchVersion)";
    visual_studio="$vsVersion"; project_root=$ProjectRoot; ue_root=$UERoot; free_disk_gb=$freeGB;
    ram_gb=$ramGB; cpu=$cpu.Name; gpu=$gpu
}
$system | ConvertTo-Json -Depth 5 | Set-Content -Encoding UTF8 (Join-Path $OutDir 'SYSTEM_INFO.json')
$report | Set-Content -Encoding UTF8 (Join-Path $OutDir 'PRELAUNCH_REPORT.txt')
if($warnings.Count){ Write-Host "Prelaunch completed with $($warnings.Count) warning(s)." -ForegroundColor Yellow } else { Write-Host 'Prelaunch completed with no warnings.' -ForegroundColor Green }
exit 0
