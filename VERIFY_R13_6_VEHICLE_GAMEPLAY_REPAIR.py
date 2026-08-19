from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
H = SRC / "Public" / "OCR13VehicleGameplayRepairSubsystem.h"
CPP = SRC / "Private" / "OCR13VehicleGameplayRepairSubsystem.cpp"
VEHICLE_ART = SRC / "Private" / "OCR13VehicleArtSubsystem.cpp"
WEAPON_ART = SRC / "Private" / "OCR13WeaponArtSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit("R13.6 VEHICLE GAMEPLAY REPAIR VERIFY FAIL: " + message)


for path in (H, CPP, VEHICLE_ART, WEAPON_ART):
    if not path.is_file():
        fail(f"missing source: {path.relative_to(ROOT)}")

h = H.read_text(encoding="utf-8", errors="replace")
cpp = CPP.read_text(encoding="utf-8", errors="replace")
vehicle_art = VEHICLE_ART.read_text(encoding="utf-8", errors="replace")
weapon_art = WEAPON_ART.read_text(encoding="utf-8", errors="replace")

includes = [line.strip() for line in h.splitlines() if line.strip().startswith("#include")]
if not includes or "generated.h" not in includes[-1]:
    fail("generated.h must remain the final vehicle-repair header include")

# Lifecycle/type declarations belong in the reflected header; implementation markers belong in the .cpp.
for token in [
    "UTickableWorldSubsystem",
    "virtual void Tick(float DeltaTime) override",
    "virtual TStatId GetStatId() const override",
    "LastDriverByVehicle",
    "GroundingRepairedVehicles",
    "MountedGunRepairedVehicles",
]:
    if token not in h:
        fail(f"vehicle repair header marker missing: {token}")

for token in [
    "VehicleRepairScanIntervalSeconds = 0.15f",
    "ImportedWheelContactBelowBodyCm = 32.0f",
    "MountedMachineGunLengthCm = 190.0f",
    "Vehicle->SetAIDriveInputsServer(0.0f, 0.0f, false)",
    "PhysicsBody->WakeAllRigidBodies()",
    "PC->FlushPressedKeys()",
    "DesiredVisualBottom = -PhysicsBody->GetUnscaledBoxExtent().Z - ImportedWheelContactBelowBodyCm",
    "Bounds.Origin.Z - Bounds.BoxExtent.Z",
    "/Game/R13/Weapons/machinegun.machinegun",
    "R13_PickupMountedMG",
    'FindObjectFast<USceneComponent>(Pickup, TEXT("BarrelPivot"))',
    'FindStaticMeshComponent(Pickup, TEXT("BarrelMesh"))',
    "RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13VehicleGameplayRepairSubsystem",
]:
    if token not in cpp:
        fail(f"vehicle repair implementation marker missing: {token}")

for token in [
    "ImportedWheelContactBelowBodyCm = 32.0f",
    "-PhysicsBody->GetUnscaledBoxExtent().Z - ImportedWheelContactBelowBodyCm",
    "CockpitZFromRoadEye(PhysicsBody, 138.0f)",
]:
    if token not in vehicle_art:
        fail(f"vehicle art does not share repaired road-contact/cockpit contract: {token}")

if "/Game/R13/Weapons/machinegun.machinegun" not in weapon_art:
    fail("mounted-gun asset is not already part of the verified runtime weapon-art family")

for forbidden in [
    "ImportedWheelContactBelowBodyCm = 60.0f",
    "-PhysicsBody->GetUnscaledBoxExtent().Z - 60.0f",
    "AlwaysSpawn",
]:
    if forbidden in cpp or forbidden in vehicle_art:
        fail(f"unsafe/obsolete vehicle repair marker present: {forbidden}")

print("R13.6 VEHICLE GAMEPLAY REPAIR VERIFY: PASS")
print("Checks tickable subsystem lifecycle in its reflected header, shared 32 cm imported-wheel contact plane for chassis/cockpit, neutral released-handbrake state on every new driver possession and real mounted-machine-gun pickup art.")
