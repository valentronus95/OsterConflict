from pathlib import Path

ROOT = Path(__file__).resolve().parent
CPP = ROOT / 'OsterConflict/Source/OsterConflict/Private/OCParkHardscapeAuthoredUpgradeSubsystem.cpp'
HEADER = ROOT / 'OsterConflict/Source/OsterConflict/Public/OCParkHardscapeAuthoredUpgradeSubsystem.h'
NORMALIZER = ROOT / 'OsterConflict/Source/OsterConflict/Private/OCParkSemanticOwnerNormalizationSubsystem.cpp'
RUN_ALL = ROOT / 'RUN_ALL_VERIFY.py'
PLANE = ROOT / 'OsterConflict/Content/AdvancedVillagePack/Meshes/SM_Plane_1x1.uasset'
CONCRETE = ROOT / 'OsterConflict/Content/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Concrete_1_Inst.uasset'


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f'FAIL: {message}')


require(CPP.exists(), 'hardscape upgrader cpp missing')
require(HEADER.exists(), 'hardscape upgrader header missing')
require(NORMALIZER.exists(), 'semantic owner normalizer missing')
require(PLANE.exists(), 'tracked SM_Plane_1x1 payload missing')
require(CONCRETE.exists(), 'tracked M_Concrete_1_Inst payload missing')

cpp = CPP.read_text(encoding='utf-8')
header = HEADER.read_text(encoding='utf-8')
normalizer = NORMALIZER.read_text(encoding='utf-8')
run_all = RUN_ALL.read_text(encoding='utf-8')

require('UOCParkHardscapeAuthoredUpgradeSubsystem' in header, 'world subsystem class contract missing')
require('ParkMemorialSurface and ParkSkateSurface' in header, 'header must state exact two-family scope')
require('ParkMemorialMonument and ParkSkateRamps remain separate content gaps' in header,
        'header must preserve monument/ramp content-gap truth')

require('constexpr float NormalizationDelaySeconds = 0.35f;' in normalizer,
        'semantic owner normalization delay contract changed unexpectedly')
require('constexpr float HardscapeUpgradeDelaySeconds = 0.85f;' in cpp,
        'hardscape upgrade must run after semantic owner normalization')
require('if (ElapsedSeconds < HardscapeUpgradeDelaySeconds) return;' in cpp,
        'hardscape delay gate missing')

for owner, count in [
    ('ParkMemorialSurface', 1),
    ('ParkMemorialMonument', 1),
    ('ParkSkateSurface', 1),
    ('ParkSkateRamps', 2),
]:
    require(f'TEXT("{owner}")' in cpp, f'{owner} owner lookup missing')
    require(f'{owner}->GetInstanceCount() == {count}' in cpp,
            f'{owner} exact instance-count contract missing')

require('LegacyMemorial->GetInstanceCount() == 0' in cpp, 'legacy memorial bucket must be empty')
require('LegacySkate->GetInstanceCount() == 0' in cpp, 'legacy skate bucket must be empty')
require('normalization_required=1' in cpp, 'hardscape must fail closed when normalization is incomplete')

require('/Game/AdvancedVillagePack/Meshes/SM_Plane_1x1.SM_Plane_1x1' in cpp,
        'authored hardscape plane path missing')
require('/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Concrete_1_Inst.M_Concrete_1_Inst' in cpp,
        'authored concrete material path missing')
require('IsEngineCube(SourceMesh)' in cpp, 'source Cube guard missing')
require('SourceTopZ' in cpp and 'NewTopOffsetZ' in cpp,
        'source top-surface preservation math missing')
require('SourceSize.X / NewNativeSize.X' in cpp and 'SourceSize.Y / NewNativeSize.Y' in cpp,
        'XY footprint fit missing')

require('BuildPlan(ParkMemorialSurface, SurfaceMesh, MemorialPlan, Failure)' in cpp,
        'memorial surface preflight missing')
require('BuildPlan(ParkSkateSurface, SurfaceMesh, SkatePlan, Failure)' in cpp,
        'skate surface preflight missing')
require('BuildPlan(ParkMemorialMonument' not in cpp,
        'memorial monument must not be authored by hardscape slice')
require('BuildPlan(ParkSkateRamps' not in cpp,
        'skate ramps must not be authored by hardscape slice')

require('MemorialMonumentMeshBefore' in cpp and 'SkateRampsMeshBefore' in cpp,
        'untouched content-gap mesh snapshots missing')
require('bUntouchedContentGapsPreserved' in cpp,
        'monument/ramp untouched postcondition missing')
require('RestorePlan(SkatePlan);' in cpp and 'RestorePlan(MemorialPlan);' in cpp,
        'transaction rollback must restore both hardscape owners')
require('PASS45_AUTHORED_PARK_HARDSCAPE_CONTENT_GAP' in cpp,
        'hardscape content-gap marker missing')
require('PASS45_AUTHORED_PARK_HARDSCAPE_FAIL' in cpp,
        'hardscape fail marker missing')
require('PASS45_AUTHORED_PARK_HARDSCAPE_READY' in cpp,
        'hardscape ready marker missing')
require('remaining_content_gap_instances=3' in cpp,
        'ready marker must keep monument + two ramps open')
require('gate_k_complete=0 runtime_acceptance=0' in cpp,
        'ready/fail truth must not promote Gate K/runtime')
require('gate_k_complete=1' not in cpp and 'runtime_acceptance=1' not in cpp,
        'hardscape source slice must never claim global/runtime acceptance')

require('VERIFY_PASS45_AUTHORED_PARK_HARDSCAPE.py' in run_all,
        'cumulative RUN_ALL_VERIFY integration missing')

print('PASS45 AUTHORED PARK HARDSCAPE: PASS')
