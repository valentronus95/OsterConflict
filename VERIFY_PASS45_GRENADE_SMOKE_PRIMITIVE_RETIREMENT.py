#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
GRENADE = SRC / "Private" / "OCGrenadeProjectile.cpp"
CHARACTER = SRC / "Private" / "OCCharacter.cpp"
VISUAL_TYPES = SRC / "Public" / "OCCharacterVisualTypes.h"
SMOKE = SRC / "Private" / "OCSmokeCloud.cpp"
SMOKE_H = SRC / "Public" / "OCSmokeCloud.h"
SMOKE_ASSET = ROOT / "OsterConflict" / "Content" / "PotaVFX_Smoke" / "VFX" / "System" / "ColorSmoke" / "NS_SmokeGradient_Loop.uasset"
FRAG_VFX_ASSET = ROOT / "OsterConflict" / "Content" / "Fire_EXP_Vol01_Free" / "Niagara" / "EXP" / "NS_Sub_EXP_Small_002.uasset"
FRAG_IDENTITY_MAT = ROOT / "OsterConflict" / "Content" / "R13" / "Weapons" / "green.uasset"
SMOKE_IDENTITY_MAT = ROOT / "OsterConflict" / "Content" / "R13" / "Weapons" / "greyLight.uasset"
FLASH_IDENTITY_MAT = ROOT / "OsterConflict" / "Content" / "R13" / "Weapons" / "sand.uasset"
BUILD = SRC / "OsterConflict.Build.cs"
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


grenade = read(GRENADE)
character = read(CHARACTER)
visual_types = read(VISUAL_TYPES)
smoke = read(SMOKE)
smoke_h = read(SMOKE_H)
build = read(BUILD)
tz = read(TZ)

# The visible grenade must use the tracked repository mesh, never an Engine sphere/cube stand-in.
req('/Game/R13/Weapons/grenade.grenade' in grenade,
    'tracked R13 grenade visual is not the current production presentation source')
req('/Engine/BasicShapes/Sphere.Sphere' not in grenade,
    'rejected Engine sphere grenade visual returned')
req('PASS45_GRENADE_PRODUCTION_VISUAL_READY' in grenade,
    'grenade production visual READY marker missing')
req('PASS45_GRENADE_PRODUCTION_VISUAL_FAIL' in grenade and 'primitive_visible=0' in grenade,
    'grenade production visual does not fail closed with primitive hidden')
req('OC_ProductionGrenadeVisual' in grenade,
    'production grenade component is not explicitly tagged')

# Until exact per-type bodies are committed, the shared real mesh must still be readable by type through tracked
# authored materials. This is identity assistance, not permission to claim the exact grenade-body content gap closed.
for material_path, object_path, label in (
    (FRAG_IDENTITY_MAT, '/Game/R13/Weapons/green.green', 'fragmentation'),
    (SMOKE_IDENTITY_MAT, '/Game/R13/Weapons/greyLight.greyLight', 'smoke'),
    (FLASH_IDENTITY_MAT, '/Game/R13/Weapons/sand.sand', 'flash'),
):
    req(material_path.is_file(), f'{label} grenade identity material is not committed: {material_path.relative_to(ROOT)}')
    req(object_path in grenade, f'{label} authored identity material is not wired into grenade presentation')
req('GetPass45GrenadeIdentityMaterialPath' in grenade,
    'grenade presentation lost replicated type-to-authored-material selection')
req('GrenadeMesh->EmptyOverrideMaterials();' in grenade and 'GrenadeMesh->SetMaterial(' in grenade,
    'grenade type refresh does not deterministically replace stale material overrides')
req('PASS45_GRENADE_TYPE_IDENTITY_MATERIAL_READY' in grenade and 'type_distinguishable=1' in grenade,
    'authored grenade type identity cannot prove a readable successful state')
req('PASS45_GRENADE_TYPE_IDENTITY_MATERIAL_FAIL' in grenade and 'type_distinguishable=0' in grenade,
    'missing grenade identity material is not fail-visible')
req('shared_generic_body=1' in grenade and 'exact_type_body=0' in grenade and 'type_specific_content_gap=1' in grenade,
    'shared grenade body is falsely promoted to exact frag/smoke/flash content closure')

# Fragmentation detonation presentation must use the committed authored Niagara donor and replicate from the factual
# server detonation. A missing donor must fail visibly instead of falling back to primitive geometry.
frag_vfx = '/Game/Fire_EXP_Vol01_Free/Niagara/EXP/NS_Sub_EXP_Small_002.NS_Sub_EXP_Small_002'
req(FRAG_VFX_ASSET.is_file(),
    'authored frag Niagara payload referenced by runtime code is not committed in the repository')
req(frag_vfx in grenade,
    'committed frag Niagara donor is not wired into AOCGrenadeProjectile')
req('MulticastDetonationVFX(GrenadeType, GetActorLocation());' in grenade,
    'factual fragmentation detonation no longer emits the replicated VFX presentation event')
req('Type != EOCGrenadeType::Fragmentation' in grenade,
    'frag detonation multicast is not restricted to fragmentation presentation')
req('UNiagaraFunctionLibrary::SpawnSystemAtLocation' in grenade,
    'fragmentation presentation no longer spawns authored Niagara at the factual detonation location')
req('PASS45_FRAG_EXPLOSION_VFX_DONOR_WIRED' in grenade and 'authored_niagara=1' in grenade,
    'frag authored Niagara integration has no source-visible success evidence')
req('PASS45_FRAG_EXPLOSION_VFX_LOAD_FAIL' in grenade and 'runtime_acceptance=0' in grenade,
    'frag Niagara load failure is not fail-visible')

# Throwing is authoritative and transactional.
throw_start = character.find('void AOCCharacter::ServerThrowSelectedGrenade_Implementation()')
throw_end = character.find('void AOCCharacter::ServerCycleGrenadeType_Implementation()', throw_start)
throw_block = character[throw_start:throw_end] if throw_start >= 0 and throw_end > throw_start else ''
req(bool(throw_block), 'authoritative grenade throw implementation missing')
req('FCollisionShape::MakeSphere' in throw_block and 'SweepSingleByChannel' in throw_block,
    'grenade throw no longer performs bounded collision clearance sweep')
req('OverlapBlockingTestByChannel' in throw_block,
    'grenade throw no longer validates final spawn location overlap')
req('ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding' in throw_block,
    'grenade projectile can force-spawn into blocking geometry again')
req('PASS45_GRENADE_SAFE_SPAWN_REJECTED' in throw_block and 'inventory_consumed=0' in throw_block,
    'unsafe grenade spawn rejection is not fail-visible/non-consuming')
req('PASS45_GRENADE_SPAWN_FAIL' in throw_block and 'inventory_consumed=0' in throw_block,
    'projectile spawn failure can become an invisible inventory loss')
req('PASS45_GRENADE_THROW_COMMIT_READY' in throw_block and 'inventory_committed_after_spawn=1' in throw_block,
    'successful throw does not expose transactional inventory commit evidence')
req('GetVelocity()' in throw_block and 'inherited_velocity=1' in throw_block,
    'grenade throw no longer inherits player movement velocity')
req('EOCCharacterActionEvent::GrenadeThrow' in throw_block,
    'successful grenade throw does not emit the presentation action event')
req('PASS45_GRENADE_THROW_PRESENTATION_BRIDGE_READY' in throw_block and 'second_gameplay_timer=0' in throw_block,
    'grenade throw presentation bridge is missing or invents a second gameplay timer')
req('GrenadeThrow' in visual_types,
    'character action enum has no grenade throw presentation event')

spawn_pos = throw_block.find('SpawnActor<AOCGrenadeProjectile>')
commit_pos = throw_block.find('--(*Count)')
req(spawn_pos >= 0 and commit_pos > spawn_pos,
    'grenade inventory is decremented before factual projectile spawn success')
req(throw_block.count('--(*Count)') == 1,
    'grenade throw owns multiple inventory decrement paths')

# The imported authored Niagara donor is now the sole smoke presentation owner.
smoke_asset = '/Game/PotaVFX_Smoke/VFX/System/ColorSmoke/NS_SmokeGradient_Loop.NS_SmokeGradient_Loop'
req(SMOKE_ASSET.is_file(),
    'authored smoke Niagara payload referenced by runtime code is not committed in the repository')
req(smoke_asset in smoke,
    'imported authored smoke Niagara system is not wired into AOCSmokeCloud')
req('UNiagaraComponent' in smoke and 'UNiagaraSystem' in smoke,
    'smoke runtime does not own/load the authored Niagara presentation')
req('PASS45_SMOKE_VFX_DONOR_WIRED' in smoke and 'authored_niagara=1' in smoke,
    'authored smoke Niagara integration does not emit source-visible wiring evidence')
req('PASS45_SMOKE_VFX_RUNTIME_READY' in smoke and 'runtime_loaded=1' in smoke and 'manual_visual_acceptance=0' in smoke,
    'smoke runtime cannot prove factual Niagara load/activation without falsely claiming manual visual acceptance')
req('PASS45_SMOKE_VFX_LOAD_FAIL' in smoke and 'primitive_visible=0' in smoke,
    'smoke VFX load failure is not visibly fail-closed')
req('PASS45_SMOKE_VFX_CONTENT_GAP' not in smoke,
    'obsolete smoke content-gap marker remains in the runtime implementation')
for stale in (
    '/Engine/BasicShapes/Sphere.Sphere',
    'SmokePuff_',
    'UStaticMeshComponent',
):
    req(stale not in smoke, f'rejected primitive smoke presentation returned: {stale}')
req('TObjectPtr<UNiagaraComponent> SmokeVFX' in smoke_h,
    'smoke header does not expose the single Niagara presentation owner')
req('primitive sphere/cube substitute' in smoke_h.lower(),
    'smoke header does not preserve fail-closed primitive-retirement truth')
req('SmokeHalfHeightCm' in smoke_h and 'GetSmokeHalfHeightCm' in smoke_h,
    'smoke gameplay volume lost its explicit finite vertical bound')

# Gameplay occlusion must grow with the smoke instead of becoming a full-radius invisible wall at detonation.
# This is intentionally query-time math rather than a per-frame Tick owner; exact Niagara synchronization stays
# pending until local UE 5.8 visual acceptance.
req('SmokeExpansionSeconds' in smoke_h and 'GetSmokeExpansionSeconds' in smoke_h,
    'smoke gameplay volume has no explicit expansion duration')
req('PrimaryActorTick.bCanEverTick = false' in smoke,
    'smoke expansion introduced a per-frame actor Tick')
for needle in (
    'GetGameTimeSinceCreation()',
    'SafeExpansionSeconds',
    'ExpansionAlpha',
    'EffectiveRadiusCm',
    'EffectiveHalfHeightCm',
    'SmokeRadiusCm * ExpansionAlpha',
    'SmokeHalfHeightCm * ExpansionAlpha',
    'FMath::Abs(Delta.Z) > EffectiveHalfHeightCm',
    'FMath::Square(EffectiveRadiusCm)',
):
    req(needle in smoke, f'smoke bounded expansion contract missing: {needle}')
req('gameplay_volume_expands=1' in smoke and 'expansion_s=' in smoke,
    'smoke runtime evidence does not expose expanding gameplay-volume truth')
req('exact_visual_sync=0' in smoke,
    'smoke source falsely claims exact Niagara/gameplay expansion synchronization')
req('FVector2D(Delta.X, Delta.Y).SizeSquared()' in smoke,
    'smoke finite-volume horizontal radius check is missing')
req('finite_volume=1' in smoke and 'half_height_cm=' in smoke,
    'smoke runtime evidence does not expose finite 3D gameplay-volume truth')
req('"Niagara"' in build,
    'OsterConflict module does not declare the Niagara dependency required by authored smoke VFX')

# Canonical runtime authority must preserve both the later source truth and the latest factual runtime rejection.
for needle in (
    'PASS45_GRENADE_PRODUCTION_VISUAL_READY',
    'PASS45_GRENADE_THROW_COMMIT_READY',
    '/Game/PotaVFX_Smoke/VFX/System/ColorSmoke/NS_SmokeGradient_Loop',
    'PASS45_SMOKE_VFX_RUNTIME_READY',
    'exact_visual_sync=0',
    'manual_visual_acceptance=0',
    'RUNTIME REJECTED 2026-08-27',
):
    req(needle in tz, f'canonical Pass45 TZ lost grenade/smoke/runtime truth: {needle}')

# Old 2026-08-26 source-state wording may remain only as historical context, never as the current checklist truth.
for stale_current_claim in (
    'authored growing smoke VFX with useful visual sight blocking remains **CONTENT GAP**',
    'Repository authored smoke/Niagara content is currently not proven present',
    'because no accepted authored smoke particle/Niagara payload is currently present',
):
    req(stale_current_claim not in tz,
        f'canonical Pass45 TZ still presents superseded smoke source state as current: {stale_current_claim}')
req('Smoke Niagara source content is now committed/wired' in tz,
    'item 24 no longer distinguishes committed smoke source integration from pending UE visual acceptance')
req('distinct authored flash-grenade world VFX remains **CONTENT GAP**' in tz,
    'canonical item 24 lost the remaining flash-world-VFX content gap')

if errors:
    print('PASS45 GRENADE/SMOKE PRIMITIVE RETIREMENT + THROW SEMANTICS: FAIL')
    for error in errors:
        print('[FAIL]', error)
    raise SystemExit(1)

print('PASS45 GRENADE/SMOKE PRIMITIVE RETIREMENT + THROW SEMANTICS: PASS')
print('- tracked R13 grenade mesh replaces the visible Engine sphere fail-closed')
print('- frag/smoke/flash share a real body but use distinct tracked authored identity materials')
print('- committed Fire_EXP Niagara donor is guarded for replicated fragmentation detonation presentation')
print('- exact per-type grenade bodies remain an explicit content gap')
print('- authoritative throw uses swept/overlap-checked spawn clearance')
print('- grenade inventory commits only after factual projectile spawn success')
print('- successful throw emits a presentation event without a second gameplay timer')
print('- primitive smoke-ball presentation remains physically retired')
print('- committed PotaVFX Niagara smoke donor is wired as the sole visible smoke owner')
print('- smoke gameplay occlusion is finite and expands by query-time game age without an actor Tick')
print('- canonical TZ now matches committed smoke source truth without upgrading UE visual acceptance')
print('- exact Niagara/gameplay expansion synchronization and manual visual acceptance remain pending')
print('STATUS: SOURCE-INTEGRATED; exact grenade bodies + flash world VFX + local UE 5.8 visual acceptance remain pending')