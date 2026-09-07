#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
BASE_H = SRC / "Public" / "OCWeaponBase.h"
BASE_CPP = SRC / "Private" / "OCWeaponBase.cpp"
VARIANTS = SRC / "Private" / "OCWeaponVariants.cpp"
LAUNCHER = SRC / "Private" / "OCAntiArmorLauncher.cpp"
FALLBACK = SRC / "Private" / "OCRealWeaponFallbackSubsystem.cpp"
LOCAL_BRIDGE = SRC / "Private" / "OCPass45ImportedWeaponBridgeSubsystem.cpp"
RUNTIME_EVIDENCE = ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"
TZ = ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


def forbid(text: str, needle: str, message: str) -> None:
    if needle in text:
        errors.append(message)


base_h = read(BASE_H)
base_cpp = read(BASE_CPP)
variants = read(VARIANTS)
launcher = read(LAUNCHER)
fallback = read(FALLBACK)
local_bridge = read(LOCAL_BRIDGE)
runtime_evidence = read(RUNTIME_EVIDENCE)
tz = read(TZ)

# GAME_RECOVERY cutover: the root BasicShape remains only as invisible physics authority. The historical decorative
# Cube/Cylinder/material composite must never be rebuilt during BeginPlay and no blocking asset load is allowed here.
req('BuildSourceOnlyWeaponVisual();' in base_cpp,
    'base weapon no longer applies the source-visual retirement contract during BeginPlay')
req('/Engine/BasicShapes/Cube.Cube' in base_cpp,
    'hidden root physics mesh contract changed unexpectedly')
forbid(base_cpp, 'LoadObject<',
    'base weapon BeginPlay regained blocking LoadObject')
for forbidden in (
    '/Engine/BasicShapes/Cylinder.Cylinder',
    '/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial',
    'UMaterialInstanceDynamic::Create',
    'SourceVisualParts.Add(',
):
    forbid(base_cpp, forbidden, f'retired decorative source weapon composite returned: {forbidden}')
for needle in (
    'WeaponMesh->SetVisibility(false, false);',
    'WeaponMesh->SetHiddenInGame(true, false);',
    'GAME_RECOVERY_SOURCE_WEAPON_COMPOSITE_RETIRED',
    'decorative_parts=0 blocking_asset_loads=0 primitive_visible=0 collision_authority_preserved=1',
):
    req(needle in base_cpp, f'base weapon source-composite retirement contract missing: {needle}')
req('USceneComponent* GetWeaponVisualRoot() const { return WeaponRoot; }' in base_h,
    'real fallback has no stable unscaled visual root accessor')

# Concrete variants must hide source primitives before resident production resolution. GAME_RECOVERY forbids
# blocking package loads in BeginPlay; async owners can later populate the exact production visual.
for function_name in ('ApplySkeletalProductionWeapon', 'ApplyStaticProductionWeapon'):
    start = variants.find(function_name + '(AOCWeaponBase* Owner')
    req(start >= 0, f'missing production helper: {function_name}')

skeletal_start = variants.find('UPrimitiveComponent* ApplySkeletalProductionWeapon')
skeletal_resolve = variants.find('FSoftObjectPath(AssetPath).ResolveObject()', skeletal_start)
skeletal_hide = variants.find('HideStaticWeaponFallback(Owner);', skeletal_start)
req(skeletal_start >= 0 and skeletal_hide > skeletal_start and skeletal_resolve > skeletal_hide,
    'skeletal production path does not hide primitives before resident ResolveObject')

static_def = variants.find('UStaticMeshComponent* ApplyStaticProductionWeapon', variants.find('UPrimitiveComponent* ApplySkeletalProductionWeapon') + 1)
static_hide = variants.find('HideStaticWeaponFallback(Owner);', static_def)
static_resolve = variants.find('FSoftObjectPath(AssetPath).ResolveObject()', static_def)
req(static_def >= 0 and static_hide > static_def and static_resolve > static_hide,
    'static production path does not hide primitives before resident ResolveObject')
forbid(variants, 'LoadObject<',
    'weapon variant BeginPlay regained blocking LoadObject')

for needle in (
    'Component->SetVisibility(false, true);',
    'Component->SetHiddenInGame(true, true);',
    'PASS45_WEAPON_PRODUCTION_VISUAL_GAP weapon=AK-47 primitive_visible=0',
    'PASS45_WEAPON_PRODUCTION_VISUAL_GAP weapon=Remington870 primitive_visible=0 real_fallback_pending=1',
    'PASS45_WEAPON_PRODUCTION_VISUAL_GAP weapon=M249 primitive_visible=0 real_fallback_pending=1',
    'PASS45_WEAPON_PRODUCTION_VISUAL_GAP weapon=MAC10 primitive_visible=0',
):
    req(needle in variants, f'variant fail-closed primitive contract missing: {needle}')

for stale in (
    'keeping source-only fallback',
    'keeping shotgun fallback visual',
    'keeping LMG fallback visual',
):
    forbid(variants, stale, f'visible source fallback wording returned: {stale}')

for needle in (
    'int32 HideSourceProxyVisuals(AOCWeaponBase& Weapon)',
    'Component->ComponentHasTag(ProductionVisualTag)',
    'Component->SetVisibility(false, false);',
    'Component->SetHiddenInGame(true, false);',
    'const int32 HiddenSourceProxyVisuals = HideSourceProxyVisuals(Weapon);',
    'source_proxy_visuals_hidden=%d',
    'physics_root_preserved=1',
):
    req(needle in local_bridge, f'local imported bridge source-proxy retirement missing: {needle}')

# Launcher owns its exact asset asynchronously and keeps the source primitive hidden throughout preload/failure.
launcher_begin = launcher.find('void AOCAntiArmorLauncher::BeginPlay()')
launcher_hide = launcher.find('Component->SetVisibility(false, true);', launcher_begin)
launcher_preload = launcher.find('RequestAsyncLoad(', launcher_begin)
req(launcher_begin >= 0 and launcher_hide > launcher_begin and launcher_preload > launcher_hide,
    'launcher does not hide primitive geometry before async production preload')
forbid(launcher, 'LoadObject<', 'launcher regained blocking production LoadObject')
for needle in (
    'FSoftObjectPath(ProductionLauncherPath)',
    'CompleteProductionVisualPreload',
    'FSoftObjectPath(ProductionLauncherPath).ResolveObject()',
    'GAME_RECOVERY_LAUNCHER_PRELOAD_BEGIN weapon=OC_RPG1',
    'GAME_RECOVERY_LAUNCHER_PRELOAD_GAP weapon=OC_RPG1',
    'PASS45_LAUNCHER_PRODUCTION_VISUAL_FAIL weapon=OC_RPG1',
    'PASS45_LAUNCHER_PRODUCTION_VISUAL_READY weapon=OC_RPG1',
    'primitive_visible=0',
    'resident_until_end_play=1',
    'async_preloaded=1 resident_asset=1 sync_load=0',
):
    req(needle in launcher, f'launcher async fail-closed primitive contract missing: {needle}')

for needle in (
    '#include "Components/SkeletalMeshComponent.h"',
    'Component->ComponentHasTag(ProductionVisualTag) &&',
    'IsValid(Component->GetStaticMesh())',
    'IsValid(Component->GetSkeletalMeshAsset())',
    '/Game/AK-47/Mesh/SM_AK-47.SM_AK-47',
    'committed AK-47 static sibling',
    'Name.Equals(TEXT("MP5"), ESearchCase::IgnoreCase)',
    'R13 real SMG temporary MP5 fallback',
):
    req(needle in fallback, f'2026-08-27 renderable weapon fallback guard missing: {needle}')
forbid(
    fallback,
    'if (IsValid(Component) && Component->ComponentHasTag(ProductionVisualTag)) return true;',
    'tag-only production visual acceptance returned; invisible tagged weapon can suppress fallback again')
forbid(fallback, 'LoadObject<',
    'real weapon fallback regained blocking LoadObject')
for needle in (
    'RequestAsyncLoad(',
    'ResolveResidentStaticMesh',
    'GAME_RECOVERY_REAL_WEAPON_FALLBACK_PRELOAD_BEGIN',
    'GAME_RECOVERY_REAL_WEAPON_FALLBACK_PRELOAD_READY',
    'sync_load=0',
):
    req(needle in fallback, f'real fallback async preload contract missing: {needle}')

for needle in (
    'MeshPath.Contains(TEXT("/Engine/BasicShapes/")',
    'HideRejectedPrimitiveVisuals(*Weapon);',
    'PASS45_PRIMITIVE_WEAPON_VISUAL_RETIRED',
    'PASS45_VISIBLE_PRIMITIVE_WEAPON_FAIL',
    'PASS45_PRIMITIVE_WEAPON_RUNTIME_READY',
    'USceneComponent* PhysicsRoot = Weapon.GetRootComponent();',
    'USceneComponent* VisualRoot = Weapon.GetWeaponVisualRoot();',
    'if (Existing != PhysicsRoot)',
    'Visual->SetupAttachment(VisualRoot);',
    'PASS45_REAL_WEAPON_FALLBACK_READY',
    'primitive_visible=0 visual_root_unscaled=1 physics_root_preserved=1',
):
    req(needle in fallback, f'real fallback primitive retirement contract missing: {needle}')

for needle in (
    'PASS45_PRIMITIVE_WEAPON_RUNTIME_READY',
    'PASS45_VISIBLE_PRIMITIVE_WEAPON_FAIL',
):
    req(needle in runtime_evidence, f'strict runtime evidence gate missing primitive marker: {needle}')

for needle in (
    'RUNTIME REJECTED',
    '22/36 = 61.1% complete, 38.9% remaining',
    'runtime_acceptance=0',
    'item16_checked=0',
    'merge_permitted=0',
    'Batch first, not micro-task first',
    'A historical verifier never outranks newer runtime truth or a newer user requirement.',
):
    req(needle in tz, f'canonical Pass45 TZ lost current execution truth: {needle}')

if errors:
    print('PASS45 PRIMITIVE WEAPON RETIREMENT: FAIL')
    for error in errors:
        print('[FAIL]', error)
    raise SystemExit(1)

print('PASS45 PRIMITIVE WEAPON RETIREMENT: PASS')
print('- base weapon keeps only an invisible physics root; decorative Cube/Cylinder/material runtime composite is retired')
print('- concrete weapon variants hide source BasicShape geometry before resident production resolution and never LoadObject during BeginPlay')
print('- local imported production bridge hides old source proxy rendering while preserving the physics root')
print('- real fallback safety meshes preload asynchronously and refresh only uses resident assets')
print('- production visual acceptance requires an assigned static/skeletal mesh, never only a component tag')
print('- launcher exact production mesh async-preloads while primitive geometry remains hidden')
print('- real fallbacks attach to the unscaled visual root while preserving physics-root collision authority')
print('- strict runtime evidence requires zero visible BasicShape rack weapons')
print('STATUS: SOURCE-CODED; local UE 5.8 rendered acceptance remains pending')
