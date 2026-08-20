from pathlib import Path

ROOT = Path(__file__).resolve().parent
HEADER = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCR13RoadsideInfrastructureSubsystem.h"
SOURCE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCR13RoadsideInfrastructureSubsystem.cpp"

REQUIRED_ASSETS = [
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Power_Pole_1.uasset",
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Power_Pole_Addons.uasset",
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Power_Pole_Light.uasset",
]

REQUIRED_TOKENS = [
    'FindISM(Sector, TEXT("Roads"))',
    'Power_Pole_1.Power_Pole_1',
    'Power_Pole_Addons.Power_Pole_Addons',
    'Power_Pole_Light.Power_Pole_Light',
    'AddRoadsidePoles',
    'MinPoleSpacingCm = 5200.0f',
    'MaxPoleSpacingCm = 6400.0f',
    'MaxPolesPerRoad = 28',
    'const bool bLongAxisX = SizeX >= SizeY',
    'IsInsideKrushelnytskaSlice(Location)',
    'Component->SetCollisionEnabled(ECollisionEnabled::NoCollision)',
    'Component->SetCanEverAffectNavigation(false)',
    'InfrastructureRoot->SetActorEnableCollision(false)',
    'if (GameMode->IsFrontendOnlySession()) return;',
    'roads/navigation unchanged',
]

FORBIDDEN_TOKENS = [
    'SetCollisionProfileName(TEXT("BlockAll"))',
    'SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics)',
    'Roads->AddInstance',
    'Roads->RemoveInstance',
    'Roads->ClearInstances',
    'SetStaticMesh(PoleMesh)',
    'Pole_Cables.Pole_Cables',
]


def fail(message: str) -> None:
    raise SystemExit(f"R13.5 ROADSIDE INFRASTRUCTURE VERIFY FAIL: {message}")


if not HEADER.is_file():
    fail(f"missing header: {HEADER.relative_to(ROOT)}")
if not SOURCE.is_file():
    fail(f"missing source: {SOURCE.relative_to(ROOT)}")

text = SOURCE.read_text(encoding="utf-8")
for token in REQUIRED_TOKENS:
    if token not in text:
        fail(f"missing source guard/token: {token}")
for token in FORBIDDEN_TOKENS:
    if token in text:
        fail(f"roadside dressing must remain visual-only and must not mutate authored roads: {token}")

for asset in REQUIRED_ASSETS:
    if not asset.is_file():
        fail(f"missing committed roadside asset: {asset.relative_to(ROOT)}")

print("R13.5 ROADSIDE INFRASTRUCTURE VERIFY: PASS")
print("Checks authored-road anchoring, bundled utility-pole meshes, Krushelnytska exclusion and zero collision/navigation mutation.")
