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
print("- production materials survive base tint bypass and are revalidated before/after possession")
print("- null/default/BasicShape BTR material slots fail closed instead of rendering a white vehicle")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 rendered possession validation remains authoritative")
