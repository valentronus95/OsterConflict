from pathlib import Path

ROOT = Path(__file__).resolve().parent
CPP = ROOT / 'OsterConflict/Source/OsterConflict/Private/OCParkHardscapeAuthoredUpgradeSubsystem.cpp'
HEADER = ROOT / 'OsterConflict/Source/OsterConflict/Public/OCParkHardscapeAuthoredUpgradeSubsystem.h'
WORLD_H = ROOT / 'OsterConflict/Source/OsterConflict/Public/OCWorldSectorOster.h'
WORLD = ROOT / 'OsterConflict/Source/OsterConflict/Private/OCWorldSectorOster.cpp'
RUN_ALL = ROOT / 'RUN_ALL_VERIFY.py'
PLANE = ROOT / 'OsterConflict/Content/AdvancedVillagePack/Meshes/SM_Plane_1x1.uasset'
CONCRETE = ROOT / 'OsterConflict/Content/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Concrete_1_Inst.uasset'


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f'FAIL: {message}')


for path, label in [
    (CPP, 'hardscape upgrader cpp'),
    (HEADER, 'hardscape upgrader header'),
    (WORLD_H, 'world header'),
    (WORLD, 'world source'),
    (PLANE, 'tracked SM_Plane_1x1 payload'),
    (CONCRETE, 'tracked M_Concrete_1_Inst payload'),
]:
    require(path.exists(), f'{label} missing')

cpp = CPP.read_text(encoding='utf-8')
header = HEADER.read_text(encoding='utf-8')
world_h = WORLD_H.read_text(encoding='utf-8')
world = WORLD.read_text(encoding='utf-8')
run_all = RUN_ALL.read_text(encoding='utf-8')

require('created directly by AOCWorldSectorOster' in header, 'direct primary-owner header contract missing')
require('ParkMemorialSurface and ParkSkateSurface' in header, 'header must state exact two-family scope')
require('ParkMemorialMonument and ParkSkateRamps remain separate content gaps' in header,
        'header must preserve monument/ramp content-gap truth')
require('primary_authoring=1 / normalization_bridge=0' in header,
        'header must retire normalization bridge ownership')

for owner, count in [
    ('ParkMemorialSurface', 1),
    ('ParkMemorialMonument', 1),
    ('ParkSkateSurface', 1),
    ('ParkSkateRamps', 2),
]:
    require(f'TObjectPtr<UInstancedStaticMeshComponent> {owner};' in world_h,
            f'{owner} primary UPROPERTY missing')
    require(f'{owner} = MakeISM(TEXT("{owner}"), TEXT("BlockAll"));' in world,
            f'{owner} primary component creation missing')
    require(f'TEXT("{owner}")' in cpp, f'{owner} runtime lookup missing')
    require(f'{owner}->GetInstanceCount() == {count}' in cpp,
            f'{owner} exact instance-count contract missing')

for needle in [
    'AddBox(ParkMemorialSurface, Park + FVector(-600, 200, 28), FVector(3100, 2500, 56));',
    'AddBox(ParkMemorialMonument, Park + FVector(-600, 200, 230), FVector(260, 260, 400));',
    'AddBox(ParkSkateSurface, Park + FVector(6100, -4100, 18), FVector(4300, 2600, 36));',
    'AddBoxRotated(ParkSkateRamps, Park + FVector(6100, -4100, 120)',
    'AddBoxRotated(ParkSkateRamps, Park + FVector(7400, -3500, 95)',
    'PASS45_PARK_PRIMARY_SEMANTIC_OWNERS_READY',
    'primary_authoring=1 normalization_bridge=0 remaining_content_gap_instances=3',
]:
    require(needle in world, f'primary semantic source contract missing: {needle}')

require('AddBox(ParkMemorialPlaza,' not in world, 'legacy mixed memorial source must stay empty')
require('AddBox(ParkSkateFitness,' not in world, 'legacy mixed skate source must stay empty')
require('AddBoxRotated(ParkSkateFitness,' not in world, 'legacy mixed skate ramp source must stay empty')

require('constexpr float HardscapeUpgradeDelaySeconds = 0.85f;' in cpp,
        'hardscape presentation delay contract changed unexpectedly')
require('if (ElapsedSeconds < HardscapeUpgradeDelaySeconds) return;' in cpp,
        'hardscape delay gate missing')
require('LegacyMemorial->GetInstanceCount() == 0' in cpp, 'legacy memorial bucket must be empty')
require('LegacySkate->GetInstanceCount() == 0' in cpp, 'legacy skate bucket must be empty')
require('primary_source_required=1 normalization_bridge=0' in cpp,
        'hardscape must fail closed when primary ownership is invalid')

require('/Game/AdvancedVillagePack/Meshes/SM_Plane_1x1.SM_Plane_1x1' in cpp,
        'authored hardscape plane path missing')
require('/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Concrete_1_Inst.M_Concrete_1_Inst' in cpp,
        'authored concrete material path missing')
require('BuildPlan(ParkMemorialSurface, SurfaceMesh, MemorialPlan, Failure)' in cpp,
        'memorial surface preflight missing')
require('BuildPlan(ParkSkateSurface, SurfaceMesh, SkatePlan, Failure)' in cpp,
        'skate surface preflight missing')
require('BuildPlan(ParkMemorialMonument' not in cpp,
        'memorial monument must not be authored by hardscape slice')
require('BuildPlan(ParkSkateRamps' not in cpp,
        'skate ramps must not be authored by hardscape slice')
require('bUntouchedContentGapsPreserved' in cpp,
        'monument/ramp untouched postcondition missing')
require('RestorePlan(SkatePlan);' in cpp and 'RestorePlan(MemorialPlan);' in cpp,
        'transaction rollback must restore both hardscape owners')
require('PASS45_AUTHORED_PARK_HARDSCAPE_READY' in cpp,
        'hardscape ready marker missing')
require('remaining_content_gap_instances=3' in cpp,
        'ready marker must keep monument + two ramps open')
require('primary_authoring=1 normalization_bridge=0' in cpp,
        'ready marker must reflect direct semantic ownership')
require('primary_authoring=0' not in cpp and 'migration_bridge_required=1' not in cpp,
        'obsolete bridge ownership must not survive')
require('gate_k_complete=1' not in cpp and 'runtime_acceptance=1' not in cpp,
        'hardscape source slice must never claim global/runtime acceptance')

require('VERIFY_PASS45_AUTHORED_PARK_HARDSCAPE.py' in run_all,
        'cumulative RUN_ALL_VERIFY integration missing')

print('PASS45 AUTHORED PARK HARDSCAPE: PASS')
