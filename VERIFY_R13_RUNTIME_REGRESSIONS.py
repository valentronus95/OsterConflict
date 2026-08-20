from pathlib import Path

ROOT = Path(__file__).resolve().parent
PRIVATE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private"

PLAYER = PRIVATE / "OCPlayerController.cpp"
LIGHTING = PRIVATE / "OCVisualEnvironment.cpp"
PICKUP = PRIVATE / "OCPickupGunTruck.cpp"


def fail(message: str) -> None:
    raise SystemExit(f"R13 RUNTIME REGRESSION VERIFY FAIL: {message}")


for path in (PLAYER, LIGHTING, PICKUP):
    if not path.is_file():
        fail(f"missing source file: {path.relative_to(ROOT)}")

player = PLAYER.read_text(encoding="utf-8")
for token in (
    "ResetIgnoreMoveInput();",
    "ResetIgnoreLookInput();",
    "PlayerInput->FlushPressedKeys();",
    "SetInputMode(FInputModeGameOnly());",
):
    if token not in player:
        fail(f"Escape/input regression guard missing: {token}")

lighting = LIGHTING.read_text(encoding="utf-8")
for token in (
    "SunLight->SetLightColor(FLinearColor::White);",
    "SkyAtmosphere->SetRayleighScattering(",
    "SkyAtmosphere->SetMieScattering(",
    "SkyAtmosphere->SetGroundAlbedo(FColor(96, 96, 96));",
    "HeightFog->SetFogDensity(0.0f);",
    "HeightFog->SetFogMaxOpacity(0.0f);",
):
    if token not in lighting:
        fail(f"neutral-lighting regression guard missing: {token}")

pickup = PICKUP.read_text(encoding="utf-8")
for token in (
    "InteriorCamera->SetRelativeLocation(FVector(28.0f, -45.0f, 88.0f));",
    "InteriorCamera->SetFieldOfView(92.0f);",
    "Windshield->SetVisibility(false, true);",
):
    if token not in pickup:
        fail(f"pickup first-person camera regression guard missing: {token}")

if "InteriorCamera->SetRelativeLocation(FVector(82.0f, -45.0f, 79.0f));" in pickup:
    fail("obsolete pickup camera location returned")

print("R13 RUNTIME REGRESSION VERIFY: PASS")
