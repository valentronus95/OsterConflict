param(
    [Parameter(Mandatory=$true)][string]$UERoot,
    [ValidateSet('Compile','Full')][string]$Mode='Full',
    [switch]$CleanProject
)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest

$KitRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$ProjectRoot = Join-Path $KitRoot 'OsterConflict'
$Project = Join-Path $ProjectRoot 'OsterConflict.uproject'
$Stamp = Get-Date -Format 'yyyyMMdd_HHmmss'
$ResultsRoot = Join-Path $PSScriptRoot "TEST_RESULTS\$Stamp"
$LogsRoot = Join-Path $ResultsRoot 'Logs'
$PackageRoot = Join-Path $ResultsRoot 'Package'
New-Item -ItemType Directory -Force -Path $ResultsRoot,$LogsRoot,$PackageRoot | Out-Null

if($CleanProject){
    foreach($dir in @('Binaries','Intermediate','Saved','DerivedDataCache')){
        $candidate=Join-Path $ProjectRoot $dir
        if(Test-Path $candidate){ Remove-Item $candidate -Recurse -Force -ErrorAction Stop }
    }
}

$Summary = New-Object System.Collections.Generic.List[string]
$StageResults = @()
$CurrentStage='init'
$CurrentLog=''
$FailureMessage=''

function Add-Summary([string]$Text){ $Summary.Add($Text); Write-Host $Text }
function Add-Stage([string]$Name,[string]$Status,[string]$Detail=''){
    $script:StageResults += [pscustomobject]@{Stage=$Name;Status=$Status;Detail=$Detail}
    Add-Summary ("{0,-32} {1} {2}" -f $Name,$Status,$Detail)
}
function Resolve-Required([string]$Path,[string]$Label){
    if(-not(Test-Path $Path)){ throw "$Label not found: $Path" }
    return (Resolve-Path $Path).Path
}
function Run-Logged([string]$Name,[string]$Exe,[string[]]$ArgumentList,[string]$LogName){
    $script:CurrentStage=$Name
    $log=Join-Path $LogsRoot $LogName
    $script:CurrentLog=$log
    Write-Host "`n========== $Name ==========" -ForegroundColor Cyan
    & $Exe @ArgumentList 2>&1 | Tee-Object -FilePath $log
    $rc=$LASTEXITCODE
    if($rc -ne 0){
        Add-Stage $Name 'FAIL' "exit=$rc; log=$LogName"
        throw "$Name failed with exit code $rc"
    }
    Add-Stage $Name 'PASS' "log=$LogName"
}

try {
    $UERoot=(Resolve-Path $UERoot).Path
    $BuildVersion=Resolve-Required (Join-Path $UERoot 'Engine\Build\Build.version') 'Build.version'
    $BuildBat=Resolve-Required (Join-Path $UERoot 'Engine\Build\BatchFiles\Build.bat') 'Build.bat'
    $RunUAT=Resolve-Required (Join-Path $UERoot 'Engine\Build\BatchFiles\RunUAT.bat') 'RunUAT.bat'
    $EditorCmd=Resolve-Required (Join-Path $UERoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe') 'UnrealEditor-Cmd.exe'
    $InstalledBuild = Test-Path (Join-Path $UERoot 'Engine\Build\InstalledBuild.txt')
    Resolve-Required $Project 'OsterConflict.uproject' | Out-Null

    $v=Get-Content $BuildVersion -Raw | ConvertFrom-Json
    if($v.MajorVersion -ne 5 -or $v.MinorVersion -ne 8){ throw "UE 5.8 required, found $($v.MajorVersion).$($v.MinorVersion).$($v.PatchVersion)" }
    Add-Stage 'UE version' 'PASS' "$($v.MajorVersion).$($v.MinorVersion).$($v.PatchVersion)"

    Run-Logged 'R8 prelaunch check' 'powershell.exe' @('-NoProfile','-ExecutionPolicy','Bypass','-File',(Join-Path $PSScriptRoot 'PRELAUNCH_CHECK.ps1'),'-UERoot',$UERoot,'-ProjectRoot',$ProjectRoot,'-OutDir',$ResultsRoot) '00A_PrelaunchCheck.log'

    $vswhere="${Env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if(Test-Path $vswhere){
        $vs=& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if(-not $vs){ throw 'Visual Studio C++ toolchain not found. Install Game development with C++ + Windows SDK.' }
        Add-Stage 'Visual Studio C++' 'PASS' $vs
    } else { Add-Stage 'Visual Studio detection' 'WARN' 'vswhere not found; UBT will perform authoritative toolchain detection.' }

    # Capture environment before build.
    @(
        "Timestamp=$Stamp",
        "UE_ROOT=$UERoot",
        "UE=$($v.MajorVersion).$($v.MinorVersion).$($v.PatchVersion)",
        "Project=$Project",
        "Computer=$env:COMPUTERNAME",
        "Windows=$([Environment]::OSVersion.VersionString)",
        "Mode=$Mode"
    ) | Set-Content -Encoding UTF8 (Join-Path $ResultsRoot 'ENVIRONMENT.txt')

    # Optional source verifiers. They are useful but not required to prove UBT success.
    $python=Get-Command python -ErrorAction SilentlyContinue
    if($python){
        $script:CurrentStage='Static verifier suite'
        $verifyLog=Join-Path $LogsRoot '00_StaticVerify.log'
        Push-Location $KitRoot
        try { & python (Join-Path $KitRoot 'RUN_ALL_VERIFY.py') 2>&1 | Tee-Object -FilePath $verifyLog; $rc=$LASTEXITCODE } finally { Pop-Location }
        if($rc -eq 0){Add-Stage 'Static verifier suite' 'PASS' '00_StaticVerify.log'} else {Add-Stage 'Static verifier suite' 'WARN' "exit=$rc; compile continues"}
    } else { Add-Stage 'Static verifier suite' 'SKIP' 'Python not found; UE compile continues.' }

    # Existing Windows preflight queries UBT targets and checks MSVC.
    Run-Logged 'S18C toolchain preflight' 'powershell.exe' @('-NoProfile','-ExecutionPolicy','Bypass','-File',(Join-Path $ProjectRoot 'Scripts\S18C\WINDOWS_TOOLCHAIN_PREFLIGHT.ps1'),'-UERoot',$UERoot) '01_ToolchainPreflight.log'

    # Project files. Source engines expose GenerateProjectFiles.bat at root; some layouts expose it in BatchFiles.
    $GPF=Join-Path $UERoot 'GenerateProjectFiles.bat'
    if(-not(Test-Path $GPF)){ $GPF=Join-Path $UERoot 'Engine\Build\BatchFiles\GenerateProjectFiles.bat' }
    if(Test-Path $GPF){ Run-Logged 'Generate project files' $GPF @("-project=$Project",'-game','-engine') '02_GenerateProjectFiles.log' }
    else { Add-Stage 'Generate project files' 'WARN' 'GenerateProjectFiles.bat not found; continuing with direct UBT.' }

    # Explicit compiles. Launcher installs do not ship every dedicated-server artifact, so use Build.bat
    # (present in both Launcher and source engines) and validate the normal Game target on installed builds.
    Run-Logged 'Compile Editor' $BuildBat @('OsterConflictEditor','Win64','Development',"-Project=$Project",'-WaitMutex','-NoHotReloadFromIDE','-UTF8Output') '03_CompileEditor.log'
    Run-Logged 'Compile Game' $BuildBat @('OsterConflict','Win64','Development',"-Project=$Project",'-WaitMutex','-UTF8Output') '04_CompileGame.log'
    if($InstalledBuild){
        Add-Stage 'Compile Dedicated Server' 'SKIP' 'Launcher/installed UE detected; use listen-server test path.'
    } else {
        Run-Logged 'Compile Client' $BuildBat @('OsterConflictClient','Win64','Development',"-Project=$Project",'-WaitMutex','-UTF8Output') '04B_CompileClient.log'
        Run-Logged 'Compile Server' $BuildBat @('OsterConflictServer','Win64','Development',"-Project=$Project",'-WaitMutex','-UTF8Output') '05_CompileServer.log'
    }

    if($Mode -eq 'Full'){
        # Localization gather/compile. Does not claim English translation coverage by itself.
        Run-Logged 'Localization GatherText' 'powershell.exe' @('-NoProfile','-ExecutionPolicy','Bypass','-File',(Join-Path $ProjectRoot 'Scripts\S19C\RUN_LOCALIZATION_GATHER.ps1'),'-UE_ROOT',$UERoot) '06_LocalizationGather.log'

        # Create project-owned runtime map before automation/package.
        Run-Logged 'Create release map' $EditorCmd @($Project,'-run=pythonscript',"-script=$(Join-Path $ProjectRoot 'Scripts\S18B\CREATE_RELEASE_MAP.py')",'-unattended','-nop4','-NullRHI','-NoSplash','-UTF8Output') '07_CreateReleaseMap.log'
        $MapFile=Join-Path $ProjectRoot 'Content\Maps\OsterConflict_Runtime.umap'
        if(-not(Test-Path $MapFile)){ throw "Release map not created: $MapFile" }
        Add-Stage 'Release map exists' 'PASS' $MapFile

        $AutomationReportRoot=Join-Path $ResultsRoot 'AutomationReports'
        Run-Logged 'Automation OsterConflict.Release' $EditorCmd @($Project,'-unattended','-nop4','-NullRHI','-NoSplash','-UTF8Output','-ExecCmds=Automation RunTest OsterConflict.Release;Quit',"-ReportExportPath=$AutomationReportRoot") '08_Automation.log'
        $AutomationIndex=Join-Path $AutomationReportRoot 'index.json'
        if(-not(Test-Path $AutomationIndex)){ throw "Automation command returned success but report index.json is missing: $AutomationIndex" }
        Add-Stage 'Automation report exists' 'PASS' $AutomationIndex

        $ClientArchive=Join-Path $PackageRoot 'Client'
        New-Item -ItemType Directory -Force -Path $ClientArchive | Out-Null

        if($InstalledBuild){
            Run-Logged 'Package Game' $RunUAT @('BuildCookRun',"-project=$Project",'-noP4','-platform=Win64','-clientconfig=Development','-target=OsterConflict','-build','-cook','-stage','-pak','-package','-prereqs','-archive',"-archivedirectory=$ClientArchive",'-map=/Game/Maps/OsterConflict_Runtime','-utf8output') '09_PackageGame.log'
            Add-Stage 'Package Dedicated Server' 'SKIP' 'Launcher/installed UE detected.'
            Add-Stage 'Dedicated server smoke' 'SKIP' 'Use START_HERE option 4 for the listen-server gameplay smoke test.'
        } else {
            $ServerArchive=Join-Path $PackageRoot 'Server'
            New-Item -ItemType Directory -Force -Path $ServerArchive | Out-Null
            Run-Logged 'Package Client' $RunUAT @('BuildCookRun',"-project=$Project",'-noP4','-platform=Win64','-clientconfig=Development','-target=OsterConflictClient','-build','-cook','-stage','-pak','-package','-prereqs','-archive',"-archivedirectory=$ClientArchive",'-map=/Game/Maps/OsterConflict_Runtime','-utf8output') '09_PackageClient.log'
            Run-Logged 'Package Server' $RunUAT @('BuildCookRun',"-project=$Project",'-noP4','-server','-noclient','-serverplatform=Win64','-serverconfig=Development','-target=OsterConflictServer','-build','-cook','-stage','-pak','-package','-archive',"-archivedirectory=$ServerArchive",'-map=/Game/Maps/OsterConflict_Runtime','-utf8output') '10_PackageServer.log'
            if($python){
                Run-Logged 'Post-build audit' 'python' @((Join-Path $ProjectRoot 'Scripts\S18B\POST_BUILD_AUDIT.py'),'--client',$ClientArchive,'--server',$ServerArchive,'--out',$PackageRoot) '11_PostBuildAudit.log'
            } else { Add-Stage 'Post-build audit' 'SKIP' 'Python not found.' }
            Run-Logged 'Packaged server + 2 clients' 'powershell.exe' @('-NoProfile','-ExecutionPolicy','Bypass','-File',(Join-Path $ProjectRoot 'Scripts\S18B\SMOKE_LOCAL.ps1'),'-BuildRoot',$PackageRoot,'-WarmupSeconds','12','-PlaySeconds','30') '12_PackagedSmoke.log'
        }
    }

    Add-Summary ''
    Add-Summary ("RESULT: PASS for Mode=$Mode; every requested stage completed.")
}
catch {
    $FailureMessage=$_.Exception.Message
    Add-Summary ''
    Add-Summary "RESULT: FAIL at stage [$CurrentStage]"
    Add-Summary "ERROR: $FailureMessage"
    Add-Summary 'Do not repair files manually. Send the TEST_RESULTS zip/logs for root-cause analysis.'
    $Analyzer = Join-Path $ProjectRoot 'Scripts\S18C\ANALYZE_BUILD_LOG.py'
    $Py = Get-Command python -ErrorAction SilentlyContinue
    if($Py -and (Test-Path $Analyzer)){
        $EvidenceLog = $null
        if($CurrentLog -and (Test-Path $CurrentLog)){ $EvidenceLog = Get-Item $CurrentLog }
        if(-not $EvidenceLog){ $EvidenceLog = Get-ChildItem $LogsRoot -Filter '*.log' -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1 }
        if($EvidenceLog){
            try { & python $Analyzer $EvidenceLog.FullName 2>&1 | Set-Content -Encoding UTF8 (Join-Path $ResultsRoot 'ROOT_CAUSE_HINT.txt') } catch {}
        }
    }
}
finally {
    $StageResults | ConvertTo-Json -Depth 4 | Set-Content -Encoding UTF8 (Join-Path $ResultsRoot 'STAGES.json')
    $Summary | Set-Content -Encoding UTF8 (Join-Path $ResultsRoot 'SUMMARY.txt')
    $result=[pscustomobject]@{
        timestamp=$Stamp; mode=$Mode; ue_root=$UERoot; project=$Project;
        success=[string]::IsNullOrEmpty($FailureMessage); failed_stage=$CurrentStage; error=$FailureMessage;
        stages=$StageResults
    }
    $result | ConvertTo-Json -Depth 6 | Set-Content -Encoding UTF8 (Join-Path $ResultsRoot 'RESULT.json')

    # Copy project Saved logs if present, then zip evidence.
    $SavedLogs=Join-Path $ProjectRoot 'Saved\Logs'
    if(Test-Path $SavedLogs){ Copy-Item $SavedLogs (Join-Path $ResultsRoot 'ProjectSavedLogs') -Recurse -Force -ErrorAction SilentlyContinue }
    $Zip=Join-Path (Split-Path $ResultsRoot -Parent) ("OSTER_UE58_TEST_RESULTS_$Stamp.zip")
    try { Compress-Archive -Path (Join-Path $ResultsRoot '*') -DestinationPath $Zip -Force; Write-Host "Evidence ZIP: $Zip" -ForegroundColor Yellow } catch { Write-Warning "Could not create evidence zip: $($_.Exception.Message)" }
}

if([string]::IsNullOrEmpty($FailureMessage)){ exit 0 } else { exit 1 }
