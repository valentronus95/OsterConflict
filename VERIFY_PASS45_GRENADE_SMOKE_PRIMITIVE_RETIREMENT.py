#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
GRENADE = SRC / "Private" / "OCGrenadeProjectile.cpp"
CHARACTER = SRC / "Private" / "OCCharacter.cpp"
VISUAL_TYPES = SRC / "Public" / "OCCharacterVisualTypes.h"
SMOKE = SRC / "Private" / "OCSmokeCloud.cpp"
SMOKE_H = SRC / "Public" / "OCSmokeCloud.h"
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

# Throwing is authoritative and transactional. Human beings apparently need a verifier to confirm that a grenade
# should not vanish from inventory when the server failed to create it, so here we are.
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

# Runtime evidence rejected the fake smoke-ball cluster. No visible BasicShape substitute is allowed.
for stale in (
    '/Engine/BasicShapes/Sphere.Sphere',
    'SmokePuff_',
    'UStaticMeshComponent',
):
    req(stale not in smoke, f'rejected primitive smoke presentation returned: {stale}')
req('PASS45_SMOKE_VFX_CONTENT_GAP' in smoke,
    'missing authored smoke VFX is not fail-visible')
req('authored_vfx=0' in smoke and 'primitive_visible=0' in smoke and 'runtime_acceptance=0' in smoke,
    'smoke content gap can impersonate runtime acceptance')
req('TArray<TObjectPtr<UStaticMeshComponent>> Puffs' not in smoke_h,
    'primitive smoke puff ownership still exists in header')
req('gameplay smoke volume' in smoke_h.lower() and 'particle/Niagara' in smoke_h,
    'smoke header does not separate gameplay volume from authored VFX truth')

# Canonical TZ must preserve the distinction: source progress is real, authored visual/runtime acceptance is separate.
for needle in (
    'PASS45_GRENADE_PRODUCTION_VISUAL_READY',
    'PASS45_GRENADE_THROW_COMMIT_READY',
    'PASS45_SMOKE_VFX_CONTENT_GAP',
    'primitive grenade/smoke',
    'RUNTIME REJECTED 2026-08-26',
):
    req(needle in tz, f'canonical Pass45 TZ lost grenade/smoke status: {needle}')

if errors:
    print('PASS45 GRENADE/SMOKE PRIMITIVE RETIREMENT + THROW SEMANTICS: FAIL')
    for error in errors:
        print('[FAIL]', error)
    raise SystemExit(1)

print('PASS45 GRENADE/SMOKE PRIMITIVE RETIREMENT + THROW SEMANTICS: PASS')
print('- tracked R13 grenade mesh replaces the visible Engine sphere fail-closed')
print('- authoritative throw uses swept/overlap-checked spawn clearance')
print('- grenade inventory commits only after factual projectile spawn success')
print('- successful throw emits a presentation event without a second gameplay timer')
print('- primitive smoke-ball cluster is physically retired from rendering')
print('- smoke gameplay volume remains available while authored VFX stays explicit CONTENT GAP')
print('STATUS: SOURCE-CODED; local UE 5.8 throw feel/animation and authored smoke VFX remain pending')
