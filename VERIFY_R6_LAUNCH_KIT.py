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
production_import = ptext('IMPORT_PRODUCTION_VEHICLES_UE58.cmd')
all_verify = text('RUN_ALL_VERIFY.py')

# Project-owned startup maps remain explicit.
req('GameDefaultMap=/Game/Maps/OsterConflict_Runtime' in engine,
    'client default map is project-owned runtime map')
req('ServerDefaultMap=/Game/Maps/OsterConflict_Runtime' in engine,
    'server default map is project-owned runtime map')
req('GameDefaultMap=/Engine/Maps/Entry' not in engine,
    'packaged client does not regress to Engine Entry')

# Responsive source UMG root still owns the native 1600x900 reference canvas.
for marker in [
    'Components/ScaleBox.h',
    'Components/SizeBox.h',
    'EStretch::ScaleToFit',
    'SetWidthOverride(1600.0f)',
    'SetHeightOverride(900.0f)',
    'WidgetTree->RootWidget = ViewportScale',
]:
    req(marker in ui, f'responsive UMG marker: {marker}')
req('virtual TSharedRef<SWidget> RebuildWidget() override;' in
    ptext('Source/OsterConflict/Public/OCGameUIRootWidget.h'),
    'native UI declares RebuildWidget override')
req('UOCGameUIRootWidget::RebuildWidget()' in ui and 'BuildWidgetTree();' in ui,
    'native UI builds WidgetTree before Slate rebuild')
req(re.search(r'PlacePanel\(DeploymentPanel\s*,\s*FVector2D\(120\s*,\s*80\)\s*,\s*FVector2D\(1360\s*,\s*690\)', ui) is not None,
    'deployment remains authored on reference canvas')

# Frontend shell stays guarded from exposing a suppressed standalone world.
req('GetNetMode() == NM_Standalone' in pc and
    'FParse::Param(FCommandLine::Get(), TEXT("NoFrontend"))' in pc,
    'standalone Frontend close guard exists')
req('Frontend-only standalone session' in gm and 'bFrontendOnlySession' in gm,
    'frontend-only gameplay suppression retained')

# START_HERE is intentionally the only user-facing launcher.
req('OSTER CONFLICT - ГОЛОВНИЙ ЗАПУСК' in start,
    'START_HERE identifies current Oster Conflict launcher')
req('choice /C 1230' in start,
    'START_HERE exposes normal game, full runtime test, editor and exit')
req('call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"' in start,
    'START_HERE normal-game route uses current R14 gameplay launcher')
req('call "%~dp0RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd"' in start,
    'START_HERE full-test route uses current Pass 21 runtime acceptance wrapper')
req('RUN_R14_MAIN_SANDBOX_TEST.cmd' not in start,
    'technical sandbox is not exposed in the user-facing START_HERE menu')
req('UnrealEditor.exe' in start and 'OsterConflict.uproject' in start and '-d3d11' in start,
    'START_HERE editor route is explicit and uses the current safe D3D11 renderer')
req('& goto menu' not in start,
    'START_HERE does not detach goto from IF blocks with ampersand chaining')
req(start.count('goto menu') >= 3 and start.count('call "%~dp0') >= 2,
    'START_HERE dispatches its active actions through explicit conditional blocks')

# Normal playtest must never silently run stale source. Pass 20 separates playable normal launch from
# strict exact-production acceptance instead of making missing HMMWV/M2/BTR source binaries a menu blocker.
req('set "FETCH_BRANCH=main"' in normal and 'set "REMOTE_REF=origin/main"' in normal,
    'main normal-game route still verifies origin/main')
req('findstr /B /I /C:"fix/runtime-acceptance-"' in normal and
    'set "REMOTE_REF=origin/%CURRENT_BRANCH%"' in normal,
    'isolated runtime-acceptance branch route exists for pre-merge UE testing')
for marker in [
    'git branch --show-current',
    'git fetch origin "%FETCH_BRANCH%"',
    'git rev-parse HEAD',
    'git rev-parse "%REMOTE_REF%"',
    'Local %CURRENT_BRANCH% is not current GitHub %REMOTE_REF%.',
    'Building current OsterConflictEditor',
    'Opening every required REAL/playable weapon visual in a fresh UE process',
    'IMPORT_PRODUCTION_VEHICLES_UE58.cmd',
    'if "%IS_ACCEPTANCE%"=="1" (',
    '[3/4] STRICT ACCEPTANCE: importing and validating REAL production HMMWV + M2 Browning + BTR-4 assets',
    '[3/4] NORMAL GAME: skipping strict production vehicle intake.',
    'Exact HMMWV/M2/BTR production source files remain an open content gap',
    '/Game/Maps/OsterConflict_Runtime',
    '-Frontend',
    '-d3d11',
]:
    req(marker in normal, f'current normal-game launcher marker: {marker}')

strict_stage = normal.find('[3/4] STRICT ACCEPTANCE')
acceptance_gate = normal.rfind('if "%IS_ACCEPTANCE%"=="1" (', 0, strict_stage)
import_call = normal.find('call "%PRODUCTION_IMPORT%"', strict_stage)
normal_else = normal.find(') else (', strict_stage)
req(strict_stage >= 0 and acceptance_gate >= 0 and import_call >= 0 and normal_else >= 0 and
    acceptance_gate < strict_stage < import_call < normal_else,
    'production vehicle ingest stays inside strict acceptance branch')

# Full production importer remains the exact-art acceptance gate.
for marker in [
    'import_production_vehicle_assets.py',
    'production_import_success.txt',
    '/Game/Production/Vehicles/HMMWV/SM_HMMWV_UA',
    '/Game/Production/Weapons/M2/SM_M2_Browning',
    '/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus',
]:
    req(marker in production_import, f'production import gate marker: {marker}')

# Root verification runner retains the late-session coverage and hardened runtime checks.
for tag in ['S16A', 'S16B', 'S16C', 'S18C_HARDENING_R1', 'S19C_SOURCE', 'R11_VISUAL_FOUNDATION']:
    req(tag in all_verify, f'root regression runner includes {tag}')

# User-facing and diagnostic wrappers expected by the repository must still exist.
for rel in [
    'START_HERE.cmd',
    'RUN_R14_CURRENT_GAMEPLAY.cmd',
    'RUN_R14_MAIN_SANDBOX_TEST.cmd',
    'RUN_COMPILE_ONLY.cmd',
    'RUN_PC_TEST.cmd',
    'STOP_LOCAL_SERVER.cmd',
]:
    req((ROOT / rel).exists(), rel)

# Generated/cached build dirs are not committed into the project source tree.
for bad in ['Binaries', 'Intermediate', 'Saved', 'DerivedDataCache']:
    req(not (P / bad).exists(), f'no source archive {bad}')

print(f'R6 / CURRENT R14 LAUNCH REGRESSION VERIFY: PASS ({len(checks)} checks)')
