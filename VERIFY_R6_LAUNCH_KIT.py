from pathlib import Path
import re
ROOT=Path(__file__).resolve().parent
P=ROOT/'OsterConflict'
checks=[]
def req(cond,msg):
    if not cond: raise SystemExit('R6 VERIFY FAIL: '+msg)
    checks.append(msg)

def text(rel): return (ROOT/rel).read_text(encoding='utf-8',errors='ignore')
def ptext(rel): return (P/rel).read_text(encoding='utf-8',errors='ignore')

val=text('PC_TEST/RUN_UE58_PC_VALIDATION.ps1')
pre=text('PC_TEST/PRELAUNCH_CHECK.ps1')
launch=text('PC_TEST/LAUNCH_LATEST_LOCAL_SESSION.ps1')
smoke=ptext('Scripts/S18B/SMOKE_LOCAL.ps1')
engine=ptext('Config/DefaultEngine.ini')
ui=ptext('Source/OsterConflict/Private/OCGameUIRootWidget.cpp')
pc=ptext('Source/OsterConflict/Private/OCPlayerController.cpp')
gm=ptext('Source/OsterConflict/Private/OCGameMode.cpp')
playerh=ptext('Source/OsterConflict/Public/OCPlayerUserSettings.h')
audioh=ptext('Source/OsterConflict/Public/OCAudioUserSettings.h')
gather=ptext('Config/Localization/Game_Gather.ini')
gameini=ptext('Config/DefaultGame.ini')
preflight=ptext('Scripts/S18C/WINDOWS_TOOLCHAIN_PREFLIGHT.ps1')

# R5 launch fixes retained.
pos_full=val.find("if($Mode -eq 'Full'){")
req(pos_full>0,'Full-mode guard exists')
for marker in ['Localization GatherText','Create release map','Automation OsterConflict.Release','Package Client','Package Server','Packaged server + 2 clients']:
    req(val.find(marker)>pos_full,f'{marker} guarded by Full mode')
req('python not found in PATH (required by audit scripts)' not in preflight,'Python not mandatory for UBT preflight')
req('Get-NetUDPEndpoint -LocalPort 7777' in pre,'prelaunch uses UDP 7777')
req('Get-NetTCPConnection -LocalPort 7777' not in pre,'prelaunch does not use TCP game-port probe')
req('ResourceName=Game.locres' in gather,'localization resource name explicit')
req('uk-UA/Game.locres' in ptext('Scripts/S19C/RUN_LOCALIZATION_GATHER.ps1'),'native LocRes output checked')
req('+CulturesToGenerate=uk-UA' in gather and '+CulturesToGenerate=en' in gather,'localization gather appends both uk-UA and en cultures')
req('\nCulturesToGenerate=uk-UA' not in gather and '\nCulturesToGenerate=en' not in gather,'localization cultures do not use resetting bare array assignments')
req('+SearchDirectoryPaths=Source' in gather,'localization source search path uses explicit array append')
req('+FileNameFilters=*.h' in gather and '+FileNameFilters=*.cpp' in gather,'localization gathers both C++ headers and sources')
req('\nFileNameFilters=' not in gather,'localization filename filters do not use resetting bare array assignments')
req('[Internationalization]' in gameini and '+LocalizationPaths=%GAMEDIR%Content/Localization/Game' in gameini,'runtime game localization path configured')
req('InternationalizationPreset=All' in gameini,'packaging includes full internationalization support')
req('+CulturesToStage=uk-UA' in gameini and '+CulturesToStage=en' in gameini,'packaging stages uk-UA and en localization')
req(("& $RunUBT '-Mode=QueryTargets' \"-Project=$ProjectPath\"" in preflight) or ("& $BuildBat '-Mode=QueryTargets' \"-Project=$ProjectPath\"" in preflight),'UBT target query uses robust precomputed project path arguments')
for rel in ['PC_TEST/PRELAUNCH_CHECK.ps1','PC_TEST/RUN_UE58_PC_VALIDATION.ps1','OsterConflict/Scripts/S18C/WINDOWS_TOOLCHAIN_PREFLIGHT.ps1']:
    wt=text(rel) if rel.startswith('PC_TEST/') else ptext(rel[len('OsterConflict/'):])
    req('$env:ProgramFiles(x86)' not in wt and '${Env:ProgramFiles(x86)}' in wt,f'{rel} uses brace-safe ProgramFiles(x86) environment syntax')

# Project-owned packaged startup map.
req('GameDefaultMap=/Game/Maps/OsterConflict_Runtime' in engine,'client default map is project-owned runtime map')
req('ServerDefaultMap=/Game/Maps/OsterConflict_Runtime' in engine,'server default map is project-owned runtime map')
req('GameDefaultMap=/Engine/Maps/Entry' not in engine,'packaged client no longer depends on Engine Entry')

# Responsive source UMG root.
for m in ['Components/ScaleBox.h','Components/SizeBox.h','EStretch::ScaleToFit','SetWidthOverride(1600.0f)','SetHeightOverride(900.0f)','WidgetTree->RootWidget = ViewportScale']:
    req(m in ui,f'responsive UMG marker: {m}')
req('virtual TSharedRef<SWidget> RebuildWidget() override;' in ptext('Source/OsterConflict/Public/OCGameUIRootWidget.h'),'native UI declares RebuildWidget override')
req('UOCGameUIRootWidget::RebuildWidget()' in ui and 'BuildWidgetTree();' in ui and 'return Super::RebuildWidget();' in ui,'native UI builds WidgetTree before underlying Slate widget')
req('if (!RootCanvas) BuildWidgetTree();' not in ui,'NativeConstruct no longer creates widget tree too late')
req(re.search(r'PlacePanel\(DeploymentPanel\s*,\s*FVector2D\(120\s*,\s*80\)\s*,\s*FVector2D\(1360\s*,\s*690\)', ui) is not None,'deployment remains authored on reference canvas')

# Frontend shell cannot close into a suppressed standalone world.
req('GetNetMode() == NM_Standalone' in pc and 'FParse::Param(FCommandLine::Get(), TEXT("NoFrontend"))' in pc,'standalone Frontend close guard exists')
req('Frontend-only standalone session' in gm and 'bFrontendOnlySession' in gm,'frontend-only gameplay suppression retained')
req('bStandaloneShell' in ui and 'FrontendCloseButton->SetVisibility' in ui,'standalone frontend hides non-actionable close control')
req('FrontendFocusOrder = { FrontendConnectButton, FrontendLocalButton, FrontendSettingsButton };' in ui,'gamepad frontend focus order excludes hidden close control')

# Runtime server + client join evidence.
req('OC_SERVER_READY' in gm,'server emits readiness marker')
req('Human joined: %s' in gm,'server emits human join marker')
req('-ABSLOG=' in smoke,'smoke writes explicit absolute logs')
req("'OC_SERVER_READY'" in smoke,'smoke requires server readiness marker')
req("'Human joined: SmokeAlpha'" in smoke,'smoke requires SmokeAlpha server join')
req("'Human joined: SmokeBravo'" in smoke,'smoke requires SmokeBravo server join')
req('Protocol=18?Name=SmokeAlpha' in smoke and 'Protocol=18?Name=SmokeBravo' in smoke,'smoke clients pass protocol 18')
req('Fatal error:|Assertion failed:|LowLevelFatalError|Unhandled Exception' in smoke,'smoke scans exact logs for crash/fatal markers')
req('Get-NetUDPEndpoint -LocalPort $Port' in smoke,'smoke waits for UDP bind')
req('OC_SERVER_READY' in launch and '-ABSLOG=' in launch,'manual launch waits for ready marker and writes explicit logs')
req('/Game/Maps/OsterConflict_Runtime -Frontend' in launch,'manual client launches project-owned runtime map in Frontend shell')

# User settings semantics follow persistence-oriented GameUserSettings config class flags.
expected='UCLASS(Config=GameUserSettings, ConfigDoNotCheckDefaults, BlueprintType)'
req(expected in playerh,'player preferences use ConfigDoNotCheckDefaults')
req(expected in audioh,'audio preferences use ConfigDoNotCheckDefaults')
req('DefaultConfig' not in playerh and 'DefaultConfig' not in audioh,'runtime user preferences are not DefaultConfig classes')
playercpp=ptext('Source/OsterConflict/Private/OCPlayerUserSettings.cpp')
req('LastUsername' in playerh and 'LastServerAddress' in playerh and 'SetFrontendIdentity' in playerh,'frontend username/address persistence contract declared')
req('LastUsername and LastServerAddress deliberately survive Reset Defaults' in playercpp,'Reset Defaults preserves frontend identity/endpoint')
req('SetFrontendIdentity(CleanName, CleanAddress)' in pc,'validated Direct Connect identity/address persist before travel')
req('FrontendPrefs->GetSavedUsername()' in ui and 'FrontendPrefs->GetLastServerAddress()' in ui,'Frontend restores persisted username/address')

# Batch wrappers must preserve the PowerShell/Unreal result through pause.
for rel in ['RUN_COMPILE_ONLY.cmd','RUN_CLEAN_FULL_TEST.cmd','RUN_LOCAL_GAME_AFTER_BUILD.cmd','STOP_LOCAL_SERVER.cmd']:
    wt=text(rel)
    req('set RC=%ERRORLEVEL%' in wt and 'exit /b %RC%' in wt,f'{rel} preserves failure exit code')

# Kit identity and wrappers.
req(('R8 TARGET RULES FIX' in text('START_HERE.cmd')) or ('R11 VISUAL FOUNDATION' in text('START_HERE.cmd')),'START_HERE identifies current launch kit')
stopps=text('PC_TEST/STOP_LOCAL_SERVER.ps1')
req("$p.ProcessName -like 'OsterConflict*'" in stopps and 'Refusing to stop it' in stopps,'STOP_LOCAL_SERVER refuses PID reuse by unrelated processes')
start=text('START_HERE.cmd')
req('& goto menu' not in start,'START_HERE does not detach goto from IF conditions with ampersand chaining')
req(start.count('goto menu') >= 6 and start.count('call "%~dp0') >= 5,'START_HERE dispatches each action through explicit conditional blocks')
req("'R8 prelaunch check'" in val,'validator stage label identifies current R8 kit')
req('R8 kit' in pre,'prelaunch failure text identifies current R8 kit')
readme=text('PC_TEST/README_PC_TEST_UA.md')
req('Запустити `START_HERE.cmd`' in readme and '**1. Compile only**' in readme,'PC test README routes first run through START_HERE compile-only')
req('AutomationReports/index.json' in readme and 'AutomationReports\\index.json' in text('FIRST_RUN_README_UA.txt'),'README documents required Automation report evidence')
req("$AutomationIndex=Join-Path $AutomationReportRoot 'index.json'" in val,'automation validation requires exported index.json evidence')
req("Add-Stage 'Automation report exists' 'PASS' $AutomationIndex" in val,'automation report evidence is recorded as a validation stage')
for rel in ['START_HERE.cmd','RUN_COMPILE_ONLY.cmd','RUN_PC_TEST.cmd','RUN_CLEAN_FULL_TEST.cmd','RUN_LOCAL_GAME_AFTER_BUILD.cmd','STOP_LOCAL_SERVER.cmd','R7_LOGIC_PHYSICS_FINDINGS.md']:
    req((ROOT/rel).exists(),rel)

req("'S16A','S16B','S16C'" in text('RUN_ALL_VERIFY.py'),'root regression runner includes S16B fence/vegetation verifier')
source_runner=ptext('Scripts/RUN_ALL_SOURCE_VERIFIERS.py')
req('S18C_HARDENING_R1' in source_runner and 'S19C_SOURCE' in source_runner,'internal source verifier runner includes hardening R1 and S19C source gates')
netmatrix=ptext('Scripts/S18C/NETWORK_EMULATION_MATRIX.ps1')
req('Protocol=18' in netmatrix,'network emulation clients use current protocol contract')
req('NetEmulation.PktLag' in netmatrix and 'NetEmulation.PktLoss' in netmatrix and 'NetEmulation.PktJitter' in netmatrix,'network emulation matrix retains lag/loss/jitter commands')
postaudit=ptext('Scripts/S18B/POST_BUILD_AUDIT.py')
collect=ptext('Scripts/S18B/COLLECT_RC_LOGS.ps1')
builds18b=ptext('Scripts/S18B/BuildS18B.ps1')
req('fallback_token' not in postaudit and "'OsterConflict.exe'" in postaudit,'post-build audit cannot mistake arbitrary helper client/server executables for the game')
req("Label='BuildRoot'" in collect and "Label='SavedLogs'" in collect and '$relative=' in collect,'RC log collector preserves source buckets/relative paths')
req('$global:LASTEXITCODE = 0' in builds18b and '$rc=$LASTEXITCODE' in builds18b,'historical S18B Step does not inherit stale external exit code')
req("'-prereqs'" in builds18b,'historical S18B client package includes prerequisites')

# No generated/cached build dirs in shipped source tree.
for bad in ['Binaries','Intermediate','Saved','DerivedDataCache']:
    req(not (P/bad).exists(),f'no source archive {bad}')

print(f'R6 BASELINE / R8 LAUNCH REGRESSION VERIFY: PASS ({len(checks)} checks)')
