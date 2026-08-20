from pathlib import Path

ROOT = Path(__file__).resolve().parent
PUBLIC = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public"
PRIVATE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private"

VEHICLE_H = PUBLIC / "OCR13VehicleVariantSpawnSubsystem.h"
VEHICLE_CPP = PRIVATE / "OCR13VehicleVariantSpawnSubsystem.cpp"
WEAPON_H = PUBLIC / "OCR13WeaponVariantSpawnSubsystem.h"
WEAPON_CPP = PRIVATE / "OCR13WeaponVariantSpawnSubsystem.cpp"


def fail(message: str) -> None:
    raise SystemExit(f"R13 RUNTIME SPAWN BRIDGES VERIFY FAIL: {message}")


for path in (VEHICLE_H, VEHICLE_CPP, WEAPON_H, WEAPON_CPP):
    if not path.is_file():
        fail(f"missing source file: {path.relative_to(ROOT)}")

vehicle_h = VEHICLE_H.read_text(encoding="utf-8")
vehicle = VEHICLE_CPP.read_text(encoding="utf-8")
weapon_h = WEAPON_H.read_text(encoding="utf-8")
weapon = WEAPON_CPP.read_text(encoding="utf-8")

COMMON_HEADER_TOKENS = (
    "ScheduleSpawnAttempt(UWorld& World, float DelaySeconds)",
    "int32 SpawnAttemptCount = 0;",
    "bool bSpawnComplete = false;",
)
for token in COMMON_HEADER_TOKENS:
    if token not in vehicle_h:
        fail(f"vehicle retry state missing: {token}")
    if token not in weapon_h:
        fail(f"weapon retry state missing: {token}")

VEHICLE_TOKENS = (
    "MaxSpawnAttempts = 20",
    "SpawnRetryDelaySeconds = 0.50f",
    "GameMode->IsFrontendOnlySession()",
    "TrySpawnBundledVehicleVariants",
    "ScheduleSpawnAttempt(World, SpawnRetryDelaySeconds)",
    "SpawnActorDeferred<AOCVehicleSpawnPoint>",
    "ESpawnActorCollisionHandlingMethod::AlwaysSpawn, ESpawnActorScaleMethod::MultiplyWithRoot",
    "ConfigureRuntime(EOCCivilianVehicleStyle::BoxTruck",
    "Pending.Num() != UE_ARRAY_COUNT(Seeds)",
    "Item.SpawnPoint->Destroy()",
    "UGameplayStatics::FinishSpawningActor(",
    "Item.SpawnPoint, Item.Transform, ESpawnActorScaleMethod::MultiplyWithRoot",
    "bSpawnComplete = true;",
)
for token in VEHICLE_TOKENS:
    if token not in vehicle:
        fail(f"vehicle spawn hardening missing: {token}")

WEAPON_TOKENS = (
    "MaxSpawnAttempts = 20",
    "SpawnRetryDelaySeconds = 0.50f",
    "GameMode->IsFrontendOnlySession()",
    "TrySpawnBundledVariants",
    "ScheduleSpawnAttempt(World, SpawnRetryDelaySeconds)",
    "SpawnedWeapons.Reserve(UE_ARRAY_COUNT(Seeds))",
    "SpawnedWeapons.Num() != UE_ARRAY_COUNT(Seeds)",
    "if (IsValid(Weapon)) Weapon->Destroy();",
    "bSpawnComplete = true;",
)
for token in WEAPON_TOKENS:
    if token not in weapon:
        fail(f"weapon spawn hardening missing: {token}")

for obsolete in (
    "void UOCR13VehicleVariantSpawnSubsystem::SpawnBundledVehicleVariants",
    "void UOCR13WeaponVariantSpawnSubsystem::SpawnBundledVariants",
):
    if obsolete in vehicle or obsolete in weapon:
        fail(f"obsolete one-shot spawn entry point returned: {obsolete}")

print("R13 RUNTIME SPAWN BRIDGES VERIFY: PASS")
print("Checks bounded retries, frontend exclusion, explicit UE 5.8 deferred spawning, duplicate protection and partial-spawn rollback.")
