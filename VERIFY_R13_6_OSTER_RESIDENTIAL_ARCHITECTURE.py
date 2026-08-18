from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
H = SRC / "Public" / "OCR13OsterResidentialArchitectureSubsystem.h"
CPP = SRC / "Private" / "OCR13OsterResidentialArchitectureSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.6 OSTER RESIDENTIAL ARCHITECTURE VERIFY FAIL: " + message)


for path in (H, CPP):
    if not path.is_file():
        fail(f"missing source: {path.relative_to(ROOT)}")

h = H.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")

includes = [line.strip() for line in h.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must remain final architecture header include")

for token in [
    "ArchitectureDelaySeconds = 1.95f",
    "R13_House01",
    "R13_House02",
    "R12_House01",
    "R12_House02",
    "Component->SetVisibility(false, true)",
    "Component->SetHiddenInGame(true, true)",
    "Do not disable collision",
    "IsReservedLandmarkArea",
    "AOCWorldSectorOster::MuseumAnchor()",
    "AOCWorldSectorOster::ParkAnchor()",
    "AOCWorldSectorOster::CollegeAnchor()",
    "AOCWorldSectorOster::StadiumAnchor()",
    "/Game/Modular_Rural_Cabin/Meshes/Modular/Roof_Both_Ends_4m.Roof_Both_Ends_4m",
    "/Game/Modular_Rural_Cabin/Meshes/Modular/Porch_4x4m.Porch_4x4m",
    "/Game/Modular_Rural_Cabin/Meshes/Modular/Door_01.Door_01",
    "/Game/Modular_Rural_Cabin/Materials/Instances/Metal_Roof.Metal_Roof",
    "R13_OsterBrickRed",
    "R13_OsterBrickOrange",
    "R13_OsterBrickBrown",
    "R13_OsterBrickMuted",
    "R13_OsterHousePlinth",
    "R13_OsterGreyPitchedRoofs",
    "R13_OsterHouseWindowTrim",
    "R13_OsterHouseWindowGlass",
    "Bounds.Origin.Z - Bounds.BoxExtent.Z",
    "AddFittedRoof",
    "AddFittedGroundProp",
    "HideLegacyHouseExtras",
    "GameMode->IsFrontendOnlySession()",
    "legacy collision retained invisibly",
]:
    if token not in cpp:
        fail(f"architecture marker missing: {token}")

for forbidden in [
    "SetCollisionEnabled(ECollisionEnabled::NoCollision);\n                Component->SetVisibility(false",
    "Component->DestroyComponent()",
    "SM_House_Var01.SM_House_Var01",
    "SM_House_Var02.SM_House_Var02",
]:
    if forbidden in cpp:
        fail(f"unsafe/generic architecture marker present: {forbidden}")

print("R13.6 OSTER RESIDENTIAL ARCHITECTURE VERIFY: PASS")
print("Checks generic prefab visual suppression with collision retained, low brick Oster house presentation, grey real pitched roofs, restrained facade detail and landmark exclusions.")
