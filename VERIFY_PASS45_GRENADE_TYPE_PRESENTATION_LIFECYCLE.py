#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
HEADER = SRC / "Public" / "OCGrenadeProjectile.h"
CPP = SRC / "Private" / "OCGrenadeProjectile.cpp"

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

if errors:
    print('PASS45 GRENADE TYPE PRESENTATION LIFECYCLE: FAIL')
    for error in errors:
        print('[FAIL]', error)
    raise SystemExit(1)

print('PASS45 GRENADE TYPE PRESENTATION LIFECYCLE: PASS')
print('- authoritative type assignment refreshes presentation after the factual type is known')
print('- replicated GrenadeType refreshes presentation on clients through OnRep')
print('- current shared grenade body remains an explicit type-specific CONTENT GAP')
print('- no BasicShape or false type-specific READY path is accepted')
print('STATUS: SOURCE-LIFECYCLE CLOSED; distinct frag/smoke/flash authored bodies remain runtime/content work')
