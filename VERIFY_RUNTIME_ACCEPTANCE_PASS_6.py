#!/usr/bin/env python3
"""Source-level contracts for runtime acceptance pass 6.

This verifier deliberately does not claim visual/runtime verification. It protects the code paths
that the UE runtime playtest must exercise: museum spawn/rack, StaticMesh weapon presentation,
barrel-bound muzzle FX, compact HUD, production vehicles, inert proxies, grass density and menu travel isolation.
"""

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private"
PUB = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public"


def read(path: Path) -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise AssertionError(f"{label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise AssertionError(f"{label}: forbidden {needle!r}")


def require_regex(text: str, pattern: str, label: str) -> None:
    if not re.search(pattern, text, flags=re.MULTILINE | re.DOTALL):
        raise AssertionError(f"{label}: pattern not found: {pattern}")


def check_museum_spawn_and_rack() -> None:
    text = read(SRC / "OCTeamSpawnPoint.cpp")
    require(text, "AOCWorldSectorOster::MuseumAnchor()", "museum spawn")
    require(text, "SpawnRuntimeBaseWeaponRack", "museum weapon rack")
    require(text, "FVector::DistSquared2D(GetActorLocation(), Museum)", "legacy spawn relocation")

    expected_classes = [
        "AOCWeapon_AssaultRifle::StaticClass()",
        "AOCWeapon_SMG::StaticClass()",
        "AOCWeapon_Pistol::StaticClass()",
        "AOCWeapon_Sniper::StaticClass()",
        "AOCWeapon_Shotgun::StaticClass()",
        "AOCWeapon_LMG::StaticClass()",
        "AOCWeapon_M14::StaticClass()",
        "AOCWeapon_Mac10::StaticClass()",
        "AOCWeapon_Tec9::StaticClass()",
        "AOCWeapon_LeverAction::StaticClass()",
        "AOCAntiArmorLauncher::StaticClass()",
    ]
    for weapon_class in expected_classes:
        require(text, weapon_class, "complete museum weapon rack")


def check_legacy_base_cleanup() -> None:
    header = read(PUB / "OCRuntimeAcceptancePass6Subsystem.h")
    text = read(SRC / "OCRuntimeAcceptancePass6Subsystem.cpp")
    require(header, "RemoveLegacyGameplayBaseInstances", "pass6 cleanup owner")
    require(text, "FVector(-104000.0f, -92000.0f, 0.0f)", "legacy team-one base center")
    require(text, "FVector( 104000.0f,  92000.0f, 0.0f)", "legacy team-two base center")
    require(text, "Component->RemoveInstance(InstanceIndex)", "legacy base instance removal")
    require(text, "NormalizeProductionStaticWeapons", "static weapon normalization")
    require(text, "FQuat::FindBetweenNormals(NativeForward, FVector::ForwardVector)", "weapon long-axis normalization")
    require(text, "OC_Pass6AxisNormalized", "weapon normalization idempotency")


def check_first_person_static_mesh_support() -> None:
    header = read(PUB / "OCFirstPersonWeaponPresentationSubsystem.h")
    text = read(SRC / "OCFirstPersonWeaponPresentationSubsystem.cpp")
    require(header, "UPrimitiveComponent* FindProductionWeaponVisual", "mesh-agnostic FP visual lookup")
    require(header, "FindProductionSkeletalWeaponVisual", "animation-only skeletal path")
    require(text, "Weapon.GetComponents<UPrimitiveComponent>", "StaticMesh-aware FP lookup")
    require(text, "Cast<UStaticMeshComponent>(ProductionVisual)", "StaticMesh FP acceptance path")
    require(text, "Weapon->SetActorRelativeLocation(WeaponLocation)", "mesh-agnostic FP pose")
    require(text, "Weapon->SetActorRelativeRotation(WeaponRotation)", "mesh-agnostic FP rotation")


def check_muzzle_and_tracer() -> None:
    text = read(SRC / "OCTransientVisualFX.cpp")
    require(text, "ResolveFiringWeapon", "actual firing-weapon resolution")
    require(text, "Character->GetCurrentWeapon()", "current weapon binding")
    require(text, "OC_ProductionWeaponVisual", "production muzzle component")
    require(text, "TryResolveSocketMuzzle", "authored muzzle socket path")
    require(text, "TryResolveBoundsMuzzle", "bounds muzzle fallback")
    require(text, "const FVector VisualMuzzle = ResolveWeaponMuzzle", "muzzle FX rebasing")
    require(text, "const FVector VisualStart = ResolveWeaponMuzzle", "tracer FX rebasing")
    require_regex(text, r"FMath::Min\(DistanceToEnd,\s*900\.0f\)", "short tracer contract")


def check_compact_hud() -> None:
    minimap = read(SRC / "OCMinimapSubsystem.cpp")
    chat = read(SRC / "OCRuntimeChatSubsystem.cpp")
    require(minimap, "constexpr float MinimapOuterSize = 184.0f", "compact minimap")
    require(minimap, "constexpr float MinimapInnerSize = 172.0f", "compact minimap inner")
    require(chat, "FVector2D(360.0f, 190.0f)", "compact chat")
    require(chat, "Messages.Num() - 5", "compact chat history")
    require(chat, "EKeys::Y", "team chat hotkey")
    require(chat, "EKeys::U", "global chat hotkey")


def check_vehicle_production_shells() -> None:
    btr = read(SRC / "OCBTR.cpp")
    pickup = read(SRC / "OCPickupGunTruck.cpp")

    for text, label in ((btr, "BTR"), (pickup, "pickup")):
        require(text, "SetCollisionEnabled(ECollisionEnabled::NoCollision)", f"{label} inert proxy collision")
        require(text, "SetGenerateOverlapEvents(false)", f"{label} inert proxy overlaps")
        require(text, "SetCanEverAffectNavigation(false)", f"{label} inert proxy nav")
        require(text, "SetCastShadow(false)", f"{label} inert proxy shadow")

    require(btr, "/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus", "BTR-4 production shell")
    require(pickup, "/Game/Production/Weapons/M2/SM_M2_Browning", "M2 production asset")
    require(pickup, "ResolveLongAxisToForward", "M2 axis normalization")
    require(pickup, "MuzzlePoint->SetRelativeLocation(FVector(82.5f", "M2 muzzle at visual front bound")
    require(pickup, "DisableVisualProxy(TurretBaseMesh)", "primitive turret proxy disabled")
    require(pickup, "DisableVisualProxy(BarrelMesh)", "primitive barrel proxy disabled")


def check_foliage_density() -> None:
    text = read(SRC / "OCDenseGroundFoliageSubsystem.cpp")
    require(text, "constexpr float GridStep = 900.0f", "denser grass grid")
    require(text, "RandomStream.RandRange(4, 6)", "denser grass clumps")
    require(text, "SetCollisionEnabled(ECollisionEnabled::NoCollision)", "foliage no collision")


def check_menu_travel_isolation() -> None:
    text = read(SRC / "OCR13UIViewportStabilizerSubsystem.cpp")
    require(text, "SetWorldRenderingSuppressed(false)", "persistent viewport rendering kept enabled")
    forbid(text, "SetWorldRenderingSuppressed(bFrontendMenu)", "no frontend rendering toggle")
    require(text, "const bool bStartupShell = !bHasGameplayPawn", "pawn-less travel shell isolation")
    require(text, "R13_MenuWorldBlocker", "opaque travel blocker")
    require(text, "R13_MenuBackground", "approved travel background")


def main() -> int:
    checks = [
        check_museum_spawn_and_rack,
        check_legacy_base_cleanup,
        check_first_person_static_mesh_support,
        check_muzzle_and_tracer,
        check_compact_hud,
        check_vehicle_production_shells,
        check_foliage_density,
        check_menu_travel_isolation,
    ]

    failed = []
    for check in checks:
        try:
            check()
            print(f"PASS {check.__name__}")
        except AssertionError as exc:
            failed.append(str(exc))
            print(f"FAIL {check.__name__}: {exc}", file=sys.stderr)

    if failed:
        print(f"\nRuntime acceptance pass 6 SOURCE CONTRACTS FAILED: {len(failed)}", file=sys.stderr)
        return 1

    print(f"\nRuntime acceptance pass 6 SOURCE CONTRACTS PASS: {len(checks)}/{len(checks)}")
    print("Runtime visual approval is still required; this script does not mark acceptance items VERIFIED.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
