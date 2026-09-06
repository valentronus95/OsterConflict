from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"

btr_h = (SRC / "Public" / "OCBTR.h").read_text(encoding="utf-8", errors="replace")
btr_cpp = (SRC / "Private" / "OCBTR.cpp").read_text(encoding="utf-8", errors="replace")
base_cpp = (SRC / "Private" / "OCVehicleBase.cpp").read_text(encoding="utf-8", errors="replace")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit("PASS45 BTR4 MATERIAL STATE VERIFY FAIL: " + message)
    print("PASS:", message)


require("ValidateProductionBTR4MaterialState" in btr_h,
        "BTR declares dedicated production material-state validation")
for signature in (
    "virtual void PossessedBy(AController* NewController) override;",
    "virtual void UnPossessed() override;",
    "virtual void PawnClientRestart() override;",
):
    require(signature in btr_h, f"BTR lifecycle hook present: {signature}")

require('AssetPath.StartsWith(TEXT("/Game/Production/"))' in base_cpp and "continue;" in base_cpp,
        "base vehicle legacy tint bypasses production meshes")

for phase in ("ApplyVehicleStyle", "PossessedBy", "UnPossessed", "PawnClientRestart"):
    require(f'ValidateProductionBTR4MaterialState(TEXT("{phase}"))' in btr_cpp,
            f"BTR revalidates material state at {phase}")

# Exact BTR-4 is the only presentation owner. If its production asset is missing/invalid, all old
# cube/cylinder hull/turret proxies must stay retired instead of masquerading as a second BTR visual.
for marker in (
    "PASS45_BTR4_PRODUCTION_VISUAL_GAP",
    "blockout_substitution=0",
    "primitive_hull_visible=0",
    "primitive_turret_visible=0",
):
    require(marker in btr_cpp, f"BTR production-shell fail-closed marker present: {marker}")
require("DisableVisualProxy(Chassis);" in btr_cpp,
        "missing BTR production shell hides the legacy chassis visual")
require("DisableVisualProxy(TurretBaseMesh);" in btr_cpp and "DisableVisualProxy(BarrelMesh);" in btr_cpp,
        "missing BTR production shell hides primitive turret/barrel visuals")

# GAME_RECOVERY point 10: the production BTR must never perform a first-use blocking LoadObject.
# Async streamable loading owns the package request; presentation only resolves an already-loaded object.
require("struct FStreamableHandle;" in btr_h and "TSharedPtr<FStreamableHandle> ProductionVisualLoadHandle;" in btr_h,
        "BTR owns an async production-visual streamable handle")
require("void HandleProductionVisualLoaded();" in btr_h,
        "BTR declares async production-visual completion callback")
require("RequestAsyncLoad" in btr_cpp and "FStreamableDelegate::CreateUObject" in btr_cpp,
        "BTR production shell uses asynchronous streamable loading")
require("FSoftObjectPath(ProductionBTR4Path).ResolveObject()" in btr_cpp,
        "BTR presentation resolves only an already-loaded production object")
require("LoadObject<UStaticMesh>" not in btr_cpp,
        "BTR runtime presentation contains no blocking UStaticMesh LoadObject")
for marker in (
    "GAME_RECOVERY_BTR4_ASYNC_LOAD_BEGIN",
    "GAME_RECOVERY_BTR4_ASYNC_LOAD_READY",
    "GAME_RECOVERY_BTR4_ASYNC_LOAD_FAIL",
    "sync_runtime_loads=0",
):
    require(marker in btr_cpp, f"BTR async load source marker present: {marker}")

require('MeshPath.StartsWith(TEXT("/Game/Production/Vehicles/BTR4/"))' in btr_cpp,
        "material guard is scoped to production BTR-4 mesh")
require('MaterialPath.StartsWith(TEXT("/Game/Production/Vehicles/BTR4/"))' in btr_cpp,
        "accepted BTR materials must come from production BTR-4 content")
require('/Engine/EngineMaterials/DefaultMaterial' in btr_cpp,
        "DefaultMaterial is explicitly rejected")
require('/Engine/BasicShapes/BasicShapeMaterial' in btr_cpp,
        "BasicShapeMaterial is explicitly rejected")
require('PASS45_BTR4_MATERIAL_STATE_READY' in btr_cpp,
        "valid authored BTR material state emits READY marker")
require('PASS45_BTR4_MATERIAL_STATE_FAIL' in btr_cpp and 'production_visible=0' in btr_cpp,
        "invalid BTR material state emits hard FAIL marker")
require('Chassis->SetVisibility(false, true);' in btr_cpp and 'Chassis->SetHiddenInGame(true, true);' in btr_cpp,
        "invalid production BTR fails closed instead of rendering white/default")
require('Chassis->SetVisibility(true, true);' in btr_cpp and 'Chassis->SetHiddenInGame(false, true);' in btr_cpp,
        "valid authored BTR is restored visible")

print("PASS45 BTR4 MATERIAL STATE VERIFY PASS")
print("- exact BTR-4 production shell is the single visual owner; blockout substitution is retired")
print("- production shell package loading is async; runtime ApplyVehicleStyle has no blocking LoadObject")
print("- production materials survive base tint bypass and are revalidated before/after possession")
print("- null/default/BasicShape BTR material slots fail closed instead of rendering a white vehicle")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 rendered possession/performance validation remains authoritative")
