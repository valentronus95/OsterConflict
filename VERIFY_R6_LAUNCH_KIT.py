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

# START_HERE is now intentionally a small user-facing launcher, not the historical R8/R11 test-kit menu.
req('OSTER CONFLICT - ГОЛОВНИЙ ЗАПУСК' in start,
    'START_HERE identifies current Oster Conflict launcher')
req('choice /C 1230' in start,
    'START_HERE exposes normal game, technical test, editor and exit')
req('call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"' in start,
    'START_HERE normal-game route uses current R14 gameplay launcher')
req('call "%~dp0RUN_R14_MAIN_SANDBOX_TEST.cmd"' in start,
    'START_HERE technical route uses current R14 sandbox launcher')
req('UnrealEditor.exe' in start and 'OsterConflict.uproject' in start,
    'START_HERE editor route is explicit')
req('& goto menu' not in start,
    'START_HERE does not detach goto from IF blocks with ampersand chaining')
req(start.count('goto menu') >= 3 and start.count('call "%~dp0') >= 2,
    'START_HERE dispatches its active actions through explicit conditional blocks')

# Normal playtest must never silently run stale source or a proxy-content acceptance build.
for marker in [
    'git branch --show-current',
    'git fetch origin main',
    'git rev-parse HEAD',
    'git rev-parse origin/main',
    'Local main is not current GitHub main.',
    'Building current OsterConflictEditor',
    'IMPORT_PRODUCTION_VEHICLES_UE58.cmd',
    'Importing and validating REAL production HMMWV + M2 Browning + BTR-4 assets',
    'The game will not launch with civilian pickup/proxy turret/proxy BTR geometry pretending to be final assets.',
    '/Game/Maps/OsterConflict_Runtime',
    '-Frontend',
]:
    req(marker in normal, f'current normal-game launcher marker: {marker}')

# Full production importer remains the only normal-game asset gate.
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
