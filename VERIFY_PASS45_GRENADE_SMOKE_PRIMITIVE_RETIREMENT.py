#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
GRENADE = SRC / "Private" / "OCGrenadeProjectile.cpp"
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

# Canonical TZ must preserve the distinction: source primitive retirement is progress, real VFX is still open.
for needle in (
    'PASS45_GRENADE_PRODUCTION_VISUAL_READY',
    'PASS45_SMOKE_VFX_CONTENT_GAP',
    'primitive grenade/smoke',
    'RUNTIME REJECTED 2026-08-26',
):
    req(needle in tz, f'canonical Pass45 TZ lost grenade/smoke status: {needle}')

if errors:
    print('PASS45 GRENADE/SMOKE PRIMITIVE RETIREMENT: FAIL')
    for error in errors:
        print('[FAIL]', error)
    raise SystemExit(1)

print('PASS45 GRENADE/SMOKE PRIMITIVE RETIREMENT: PASS')
print('- tracked R13 grenade mesh replaces the visible Engine sphere fail-closed')
print('- primitive smoke-ball cluster is physically retired from rendering')
print('- smoke gameplay volume remains available while authored VFX stays explicit CONTENT GAP')
print('STATUS: SOURCE-CODED; local UE 5.8 grenade appearance/throw feel and authored smoke VFX remain pending')
