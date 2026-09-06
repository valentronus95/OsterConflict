#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
HEADER = SRC / "Public" / "OCGrenadeProjectile.h"
CPP = SRC / "Private" / "OCGrenadeProjectile.cpp"
PRELOAD_HEADER = SRC / "Public" / "OCPass45ImportedGrenadeVisualSubsystem.h"
PRELOAD_CPP = SRC / "Private" / "OCPass45ImportedGrenadeVisualSubsystem.cpp"
SMOKE_CPP = SRC / "Private" / "OCSmokeCloud.cpp"
DEPLOY_CPP = SRC / "Private" / "OCDeploymentLoadingSubsystem.cpp"
HUD_CPP = SRC / "Private" / "OCHUD.cpp"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


header = read(HEADER)
cpp = read(CPP)
preload_header = read(PRELOAD_HEADER)
preload_cpp = read(PRELOAD_CPP)
smoke_cpp = read(SMOKE_CPP)
deploy_cpp = read(DEPLOY_CPP)
hud_cpp = read(HUD_CPP)

req('ReplicatedUsing=OnRep_GrenadeType' in header,
    'GrenadeType is not replication-notify driven')
req('void OnRep_GrenadeType();' in header,
    'grenade type replication notify declaration missing')
req('void RefreshGrenadePresentation();' in header,
    'single grenade presentation refresh owner missing')
req('DOREPLIFETIME(AOCGrenadeProjectile, GrenadeType);' in cpp,
    'GrenadeType replication registration missing')

begin_start = cpp.find('void AOCGrenadeProjectile::BeginPlay()')
begin_end = cpp.find('void AOCGrenadeProjectile::RefreshGrenadePresentation()', begin_start)
begin_block = cpp[begin_start:begin_end] if begin_start >= 0 and begin_end > begin_start else ''
req('RefreshGrenadePresentation();' in begin_block,
    'BeginPlay does not build presentation from the current replicated grenade type')

init_start = cpp.find('void AOCGrenadeProjectile::InitializeGrenadeServer(')
init_end = cpp.find('void AOCGrenadeProjectile::MulticastDetonationVFX_Implementation', init_start)
init_block = cpp[init_start:init_end] if init_start >= 0 and init_end > init_start else ''
assign_pos = init_block.find('GrenadeType = NewType;')
refresh_pos = init_block.find('RefreshGrenadePresentation();')
req(assign_pos >= 0 and refresh_pos > assign_pos,
    'authority does not refresh grenade presentation after assigning the factual type')
req('ForceNetUpdate();' in init_block,
    'authoritative grenade type assignment no longer forces timely replication')

onrep_start = cpp.find('void AOCGrenadeProjectile::OnRep_GrenadeType()')
onrep_end = cpp.find('void AOCGrenadeProjectile::GetLifetimeReplicatedProps', onrep_start)
onrep_block = cpp[onrep_start:onrep_end] if onrep_start >= 0 and onrep_end > onrep_start else ''
req('RefreshGrenadePresentation();' in onrep_block,
    'clients do not refresh grenade presentation when GrenadeType replicates')

req('PASS45_GRENADE_PRESENTATION_TYPE_REFRESH' in cpp,
    'grenade type presentation lifecycle has no fail-visible runtime evidence')
req('replicated_type_refresh=1' in cpp,
    'grenade presentation runtime evidence does not expose replication-driven refresh')
req('shared_generic_body=1' in cpp and 'type_specific_content_gap=1' in cpp,
    'current generic grenade body is no longer represented fail-honestly')
req('type_specific_content_ready=1' not in cpp,
    'source falsely claims type-specific grenade body content is ready')
req('/Engine/BasicShapes/' not in cpp,
    'grenade presentation lifecycle resurrected an Engine BasicShape visual')

# GAME_RECOVERY first-use hitch closure: package loads must finish before deployment releases the player.
req('RequestAsyncLoad' in preload_cpp,
    'grenade presentation owner does not async-preload packages')
req('IsGrenadePresentationReady() const' in preload_header,
    'grenade preloader does not expose factual readiness')
req('GetGrenadePresentationProgress() const' in preload_header,
    'grenade preloader does not expose loading progress')
for required_path in (
    '/Game/R13/Weapons/grenade.grenade',
    '/Game/R13/Weapons/green.green',
    '/Game/R13/Weapons/greyLight.greyLight',
    '/Game/R13/Weapons/sand.sand',
    '/Game/Fire_EXP_Vol01_Free/Niagara/EXP/NS_Sub_EXP_Small_002.NS_Sub_EXP_Small_002',
    '/Game/PotaVFX_Smoke/VFX/System/ColorSmoke/NS_SmokeGradient_Loop.NS_SmokeGradient_Loop',
):
    req(required_path in preload_cpp,
        f'pre-spawn grenade preload lost required asset: {required_path}')

req('LoadObject<' not in cpp,
    'fragmentation/flash grenade throw or detonation path still contains blocking LoadObject')
req('LoadObject<' not in smoke_cpp,
    'smoke grenade first detonation still contains blocking LoadObject')
req('.GetAsset()' not in preload_cpp,
    'Fab grenade resolver still contains synchronous FAssetData::GetAsset')
req('.ResolveObject()' in cpp and '.ResolveObject()' in preload_cpp and '.ResolveObject()' in smoke_cpp,
    'grenade first-use path is not lookup-only after preload')
req('wait_for_grenades=1' in deploy_cpp and 'IsGrenadePresentationReady()' in deploy_cpp,
    'deployment can release the player before grenade preload is ready')
req('grenade_assets_ready_before_spawn=1' in deploy_cpp,
    'deployment completion log does not prove grenade assets were ready before spawn')
req('mandatory_assets=6' in preload_cpp and 'tracked_assets=6' in preload_cpp,
    'grenade preloader does not account for frag and smoke Niagara payloads')

# One detonation and bounded fragmentation VFX lifetime.
req('bool bDetonated = false;' in header,
    'grenade projectile has no duplicate detonation guard')
req('if (!HasAuthority() || bDetonated) return;' in cpp and 'bDetonated = true;' in cpp,
    'server detonation is not guarded as a one-shot event')
req('one_explosion_event=1' in cpp,
    'detonation log does not expose one-shot semantics')
req('DeactivateImmediate();' in cpp and 'DestroyComponent();' in cpp,
    'fragmentation Niagara has no forced cleanup for looping donor VFX')
req('Pass45FragExplosionCleanupSeconds' in cpp,
    'fragmentation VFX cleanup lifetime is not explicit')
req('Pass45FragExplosionVisualScale' in cpp,
    'fragmentation visual scale is not explicitly separated from damage radius')
req('EOCWorldAudioEvent::ExplosionLarge' in cpp,
    'fragmentation grenade still uses only the small explosion audio event')
req('sync_package_loads=0' in smoke_cpp,
    'smoke VFX source does not expose lookup-only first-use evidence')

# HUD was already wired; preserve it rather than rebuilding a second grenade HUD owner.
req('GetSelectedGrenadeType()' in hud_cpp and 'GetSelectedGrenadeCount()' in hud_cpp,
    'HUD no longer exposes selected grenade type/count')
req('F THROW' in hud_cpp,
    'HUD no longer exposes grenade throw control')

if errors:
    print('PASS45 GRENADE TYPE PRESENTATION + GAME RECOVERY FIRST USE: FAIL')
    for error in errors:
        print('[FAIL]', error)
    raise SystemExit(1)

print('PASS45 GRENADE TYPE PRESENTATION + GAME RECOVERY FIRST USE: PASS')
print('- replicated grenade type presentation remains intact and fail-honest')
print('- tracked grenade mesh/material/frag+smoke Niagara plus discovered Fab grenade meshes are async-preloaded before deployment release')
print('- frag/smoke throw and detonation paths contain no blocking LoadObject/GetAsset package load')
print('- HUD retains selected grenade type/count and throw control')
print('- server detonation is one-shot; fragmentation Niagara has an explicit forced cleanup')
print('STATUS: SOURCE/PRELOAD CLOSED; factual UE 5.8 first-throw/explosion acceptance remains required')