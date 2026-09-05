#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
BASE_H = SRC / "Public" / "OCWeaponBase.h"
BASE_CPP = SRC / "Private" / "OCWeaponBase.cpp"
VARIANTS = SRC / "Private" / "OCWeaponVariants.cpp"
LAUNCHER = SRC / "Private" / "OCAntiArmorLauncher.cpp"
FALLBACK = SRC / "Private" / "OCRealWeaponFallbackSubsystem.cpp"
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
runtime_evidence = read(RUNTIME_EVIDENCE)
tz = read(TZ)

# Keep the old source composite only as hidden collision/debug history while migration is incomplete.
req('BuildSourceOnlyWeaponVisual();' in base_cpp,
    'source primitive history disappeared without a replacement collision migration contract')
req('/Engine/BasicShapes/Cube.Cube' in base_cpp,
    'expected current hidden physics/source primitive contract changed unexpectedly')
req('USceneComponent* GetWeaponVisualRoot() const { return WeaponRoot; }' in base_h,
    'real fallback has no stable unscaled visual root accessor')

# Concrete variants must hide source primitives before any production load can fail.
for function_name in ('ApplySkeletalProductionWeapon', 'ApplyStaticProductionWeapon'):
    start = variants.find(function_name + '(AOCWeaponBase* Owner')
    req(start >= 0, f'missing production helper: {function_name}')

skeletal_start = variants.find('UPrimitiveComponent* ApplySkeletalProductionWeapon')
skeletal_load = variants.find('LoadObject<USkeletalMesh>', skeletal_start)
skeletal_hide = variants.find('HideStaticWeaponFallback(Owner);', skeletal_start)
req(skeletal_start >= 0 and skeletal_hide > skeletal_start and skeletal_load > skeletal_hide,
    'skeletal production path does not hide primitives before LoadObject')

static_def = variants.find('UStaticMeshComponent* ApplyStaticProductionWeapon', variants.find('UPrimitiveComponent* ApplySkeletalProductionWeapon') + 1)
static_hide = variants.find('HideStaticWeaponFallback(Owner);', static_def)
static_load = variants.find('LoadObject<UStaticMesh>', static_def)
req(static_def >= 0 and static_hide > static_def and static_load > static_hide,
    'static production path does not hide primitives before LoadObject')

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

# Launcher is a hard current runtime rejection: hide the source tube before exact production load.
launcher_begin = launcher.find('void AOCAntiArmorLauncher::BeginPlay()')
launcher_hide = launcher.find('Component->SetVisibility(false, true);', launcher_begin)
launcher_load = launcher.find('LoadObject<UStaticMesh>', launcher_begin)
req(launcher_begin >= 0 and launcher_hide > launcher_begin and launcher_load > launcher_hide,
    'launcher does not hide primitive geometry before production LoadObject')
for needle in (
    'PASS45_LAUNCHER_PRODUCTION_VISUAL_FAIL weapon=OC_RPG1',
    'primitive_visible=0 runtime_acceptance=0',
    'PASS45_LAUNCHER_PRODUCTION_VISUAL_READY weapon=OC_RPG1',
):
    req(needle in launcher, f'launcher fail-closed primitive contract missing: {needle}')

# 2026-08-27 runtime regression: AK/MP5 could show briefly and then disappear because a tag-only production
# component suppressed real fallback after BasicShape retirement. A production visual must own a real assigned mesh.
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

# Runtime real fallback must keep the primitive invisible, attach the real mesh to unscaled WeaponRoot,
# and preserve the physical root collision authority needed by pickup/drop physics.
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

# Runtime acceptance must consume this evidence, otherwise a source-only green check could hide a broken rack again.
for needle in (
    'PASS45_PRIMITIVE_WEAPON_RUNTIME_READY',
    'PASS45_VISIBLE_PRIMITIVE_WEAPON_FAIL',
):
    req(needle in runtime_evidence, f'strict runtime evidence gate missing primitive marker: {needle}')

# The compact canonical TZ intentionally stores policy/current state, not every historical
# runtime marker. Concrete primitive evidence is owned by source + the runtime evidence verifier above.
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
print('- concrete weapon variants hide source BasicShape geometry before production loading can fail')
print('- production visual acceptance requires an assigned static/skeletal mesh, never only a component tag')
print('- AK exact static sibling and MP5 tracked real-SMG fallback prevent invisible actors when exact production fails')
print('- launcher fails closed with primitive_visible=0')
print('- real fallbacks attach to the unscaled visual root while preserving physics-root collision authority')
print('- strict runtime evidence requires zero visible BasicShape rack weapons')
print('STATUS: SOURCE-CODED; local UE 5.8 rendered acceptance remains pending')
