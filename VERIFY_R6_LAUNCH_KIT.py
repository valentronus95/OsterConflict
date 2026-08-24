from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
P = ROOT / 'OsterConflict'
checks = []


def req(condition, message):
    if not condition:
        raise SystemExit('R6 VERIFY FAIL: ' + message)
    checks.append(message)


def text(rel):
    return (ROOT / rel).read_text(encoding='utf-8', errors='ignore')


def ptext(rel):
    return (P / rel).read_text(encoding='utf-8', errors='ignore')


engine = ptext('Config/DefaultEngine.ini')
ui = ptext('Source/OsterConflict/Private/OCGameUIRootWidget.cpp')
pc = ptext('Source/OsterConflict/Private/OCPlayerController.cpp')
gm = ptext('Source/OsterConflict/Private/OCGameMode.cpp')
start = text('START_HERE.cmd')
normal = text('RUN_R14_CURRENT_GAMEPLAY.cmd')
playflow = text('RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd')
production_import = ptext('IMPORT_PRODUCTION_VEHICLES_UE58.cmd')
source_recovery = ptext('Scripts/prepare_local_production_sources.ps1')
all_verify = text('RUN_ALL_VERIFY.py')

req('GameDefaultMap=/Game/Maps/OsterConflict_Runtime' in engine, 'client default map is project-owned runtime map')
req('ServerDefaultMap=/Game/Maps/OsterConflict_Runtime' in engine, 'server default map is project-owned runtime map')
req('GlobalDefaultGameMode=/Script/OsterConflict.OCGameModeRuntimeSafe' in engine,
    'Pass 44 runtime-safe GameMode is the client default')
req('GameDefaultMap=/Engine/Maps/Entry' not in engine, 'packaged client does not regress to Engine Entry')

for marker in [
    'Components/ScaleBox.h', 'Components/SizeBox.h', 'EStretch::ScaleToFit',
    'SetWidthOverride(1600.0f)', 'SetHeightOverride(900.0f)', 'WidgetTree->RootWidget = ViewportScale',
]:
    req(marker in ui, f'responsive UMG marker: {marker}')
req('virtual TSharedRef<SWidget> RebuildWidget() override;' in ptext('Source/OsterConflict/Public/OCGameUIRootWidget.h'),
    'native UI declares RebuildWidget override')
req('UOCGameUIRootWidget::RebuildWidget()' in ui and 'BuildWidgetTree();' in ui,
    'native UI builds WidgetTree before Slate rebuild')
req(re.search(r'PlacePanel\(DeploymentPanel\s*,\s*FVector2D\(120\s*,\s*80\)\s*,\s*FVector2D\(1360\s*,\s*690\)', ui) is not None,
    'deployment remains authored on reference canvas')

req('GetNetMode() == NM_Standalone' in pc and 'FParse::Param(FCommandLine::Get(), TEXT("NoFrontend"))' in pc,
    'standalone Frontend close guard exists')
req('Frontend-only standalone session' in gm and 'bFrontendOnlySession' in gm,
    'frontend-only gameplay suppression retained')

req('OSTER CONFLICT - ГОЛОВНИЙ ЗАПУСК' in start, 'START_HERE identifies current Oster Conflict launcher')
req('choice /C 1230' in start, 'START_HERE exposes normal game, full runtime test, editor and exit')
req('call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"' in start,
    'START_HERE normal-game route uses current R14 gameplay launcher')
req('call "%~dp0RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"' in start,
    'START_HERE full-test route uses current playflow acceptance wrapper')
req('RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd' not in start,
    'internal Pass 21 runtime acceptance is not exposed in START_HERE')
req('RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd' not in start,
    'focused Pass 15 runtime acceptance is not exposed in START_HERE')
req('RUN_R14_MAIN_SANDBOX_TEST.cmd' not in start,
    'technical sandbox is not exposed in START_HERE')
req('UnrealEditor.exe' in start and 'OsterConflict.uproject' in start and '-d3d11' in start,
    'START_HERE editor route uses current safe D3D11 renderer')
req('& goto menu' not in start, 'START_HERE does not detach goto from IF blocks')
req(start.count('goto menu') >= 3 and start.count('call "%~dp0') >= 2,
    'START_HERE dispatches active actions through explicit blocks')

for marker in [
    'RUN_R14_CURRENT_GAMEPLAY.cmd', 'PASS29_MAIN_START_DIRECT_HOST_QUEUED',
    'PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE', 'PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY',
    'PASS44_COMPACT_PLAYABLE_AREA_READY', 'PASS14_PERF_SAMPLE', 'PASS14_PERF_30FPS_READY',
]:
    req(marker in playflow, f'full runtime playflow wrapper marker: {marker}')

# Main and explicit runtime-fix branches can be tested without stale-source ambiguity.
req('set "FETCH_BRANCH=main"' in normal and 'set "REMOTE_REF=origin/main"' in normal,
    'main normal-game route verifies origin/main')
req('findstr /B /I /C:"fix/runtime-acceptance-"' in normal and
    '/C:"fix/runtime-map-spawn-fps-assets-"' in normal and
    'set "REMOTE_REF=origin/%CURRENT_BRANCH%"' in normal,
    'current runtime-fix branch route exists for pre-merge UE testing')
for marker in [
    'git branch --show-current', 'git fetch origin "%FETCH_BRANCH%"', 'git rev-parse HEAD',
    'git rev-parse "%REMOTE_REF%"', 'Local %CURRENT_BRANCH% is not current GitHub %REMOTE_REF%.',
    'Building current OsterConflictEditor',
    'Opening every required REAL/playable weapon visual in a fresh UE process',
    'IMPORT_PRODUCTION_VEHICLES_UE58.cmd', 'if "%IS_ACCEPTANCE%"=="1" (',
    '[3/4] STRICT ACCEPTANCE: importing and validating REAL production HMMWV + M2 Browning + BTR-4 assets',
    '[3/4] NORMAL GAME: optional production model intake is handled by START_HERE before this launcher.',
    'Missing exact production models remain visible content gaps; no proxy is called production-ready.',
    '/Game/Maps/OsterConflict_Runtime', '-Frontend', '-d3d11',
]:
    req(marker in normal, f'current normal-game launcher marker: {marker}')

strict_stage = normal.find('[3/4] STRICT ACCEPTANCE')
acceptance_gate = normal.rfind('if "%IS_ACCEPTANCE%"=="1" (', 0, strict_stage)
import_call = normal.find('call "%PRODUCTION_IMPORT%"', strict_stage)
normal_else = normal.find(') else (', strict_stage)
req(strict_stage >= 0 and acceptance_gate >= 0 and import_call >= 0 and normal_else >= 0 and
    acceptance_gate < strict_stage < import_call < normal_else,
    'production vehicle ingest stays inside strict acceptance branch')

# Exact local source names now belong to the importer/source-recovery owner, not duplicated gameplay text.
for marker in [
    'import_production_vehicle_assets.py', 'production_import_success.txt',
    '/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA', '/Game/Production/Weapons/M2/SM_M2_Browning',
    '/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus',
    'set "HMMWV_IMPORTED=0"', 'set "M2_IMPORTED=0"', 'set "BTR_IMPORTED=0"',
]:
    req(marker in production_import, f'production import gate marker: {marker}')
for marker in [
    'ukrainian_hmmwv_mk_19.glb', 'm2_50cal_machinegun_cc0.glb', 'BTR4_Bucephalus.fbx',
    'Find-BtrFbxInNamedArchive', 'Available models may still be imported independently',
]:
    req(marker in source_recovery, f'production source recovery marker: {marker}')

for tag in ['S16A', 'S16B', 'S16C', 'S18C_HARDENING_R1', 'S19C_SOURCE', 'R11_VISUAL_FOUNDATION',
            'VERIFY_RUNTIME_MAP_SPAWN_FPS_ASSETS_PASS_44.py']:
    req(tag in all_verify, f'root regression runner includes {tag}')

for rel in [
    'START_HERE.cmd', 'RUN_R14_CURRENT_GAMEPLAY.cmd', 'RUN_R14_MAIN_SANDBOX_TEST.cmd',
    'RUN_COMPILE_ONLY.cmd', 'RUN_PC_TEST.cmd', 'STOP_LOCAL_SERVER.cmd',
]:
    req((ROOT / rel).exists(), rel)

for bad in ['Binaries', 'Intermediate', 'Saved', 'DerivedDataCache']:
    req(not (P / bad).exists(), f'no source archive {bad}')

print(f'R6 / PASS 44 CURRENT R14 LAUNCH REGRESSION VERIFY: PASS ({len(checks)} checks)')
