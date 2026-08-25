from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "Source" / "OsterConflict"
PRIVATE = SOURCE / "Private"
PUBLIC = SOURCE / "Public"

failures: list[str] = []


def require(condition: bool, message: str) -> None:
    if not condition:
        failures.append(message)


def read(path: Path) -> str:
    require(path.is_file(), f"missing required file: {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8") if path.is_file() else ""


def absent(path: Path, label: str) -> None:
    require(not path.exists(), f"retired {label} resurrected: {path.relative_to(ROOT)}")


required_files = [
    SOURCE / "OsterConflict.Build.cs",
    PUBLIC / "OCWeaponBase.h",
    PUBLIC / "OCR137MuseumPhotoModelSubsystem.h",
    PRIVATE / "OCR137MuseumPhotoModelSubsystem.cpp",
    PUBLIC / "OCR138MuseumInteractiveArchitectureSubsystem.h",
    PRIVATE / "OCR138MuseumInteractiveArchitectureSubsystem.cpp",
    PUBLIC / "OCMuseumDoubleDoor.h",
    PRIVATE / "OCMuseumDoubleDoor.cpp",
    PUBLIC / "OCR139MuseumMainDoorReplacementSubsystem.h",
    PRIVATE / "OCR139MuseumMainDoorReplacementSubsystem.cpp",
    PUBLIC / "OCMuseumServiceDoubleDoor.h",
    PRIVATE / "OCMuseumServiceDoubleDoor.cpp",
    PUBLIC / "OCR140MuseumFacadeDetailSubsystem.h",
    PRIVATE / "OCR140MuseumFacadeDetailSubsystem.cpp",
    PUBLIC / "OCMuseumBreakableWindow.h",
    PRIVATE / "OCMuseumBreakableWindow.cpp",
    PUBLIC / "OCR142MuseumEntranceDetailSubsystem.h",
    PRIVATE / "OCR142MuseumEntranceDetailSubsystem.cpp",
    PUBLIC / "OCR143MuseumSiteVegetationSubsystem.h",
    PRIVATE / "OCR143MuseumSiteVegetationSubsystem.cpp",
    PUBLIC / "OCR144MuseumRearExteriorDetailSubsystem.h",
    PRIVATE / "OCR144MuseumRearExteriorDetailSubsystem.cpp",
    PUBLIC / "OCR145MuseumTreeLayoutSubsystem.h",
    PRIVATE / "OCR145MuseumTreeLayoutSubsystem.cpp",
    PUBLIC / "OCR138MuseumRuntimeValidationSubsystem.h",
    PRIVATE / "OCR138MuseumRuntimeValidationSubsystem.cpp",
    PUBLIC / "OCLandmarkStartupCoordinatorSubsystem.h",
    PRIVATE / "OCLandmarkStartupCoordinatorSubsystem.cpp",
    ROOT / "VALIDATE_MUSEUM_UE58.cmd",
]
for path in required_files:
    read(path)

# Pass45 physically retired the temporary R14.1 prototype-window replacement stage.
absent(PUBLIC / "OCR141MuseumWindowReplacementSubsystem.h", "R14.1 Museum window replacement header")
absent(PRIVATE / "OCR141MuseumWindowReplacementSubsystem.cpp", "R14.1 Museum window replacement source")

build_rules = read(SOURCE / "OsterConflict.Build.cs")
weapon_base = read(PUBLIC / "OCWeaponBase.h")
r137 = read(PRIVATE / "OCR137MuseumPhotoModelSubsystem.cpp")
architecture_h = read(PUBLIC / "OCR138MuseumInteractiveArchitectureSubsystem.h")
architecture = read(PRIVATE / "OCR138MuseumInteractiveArchitectureSubsystem.cpp")
main_door = read(PRIVATE / "OCMuseumDoubleDoor.cpp")
main_door_replacement = read(PRIVATE / "OCR139MuseumMainDoorReplacementSubsystem.cpp")
service_door = read(PRIVATE / "OCMuseumServiceDoubleDoor.cpp")
facade = read(PRIVATE / "OCR140MuseumFacadeDetailSubsystem.cpp")
window = read(PRIVATE / "OCMuseumBreakableWindow.cpp")
entrance = read(PRIVATE / "OCR142MuseumEntranceDetailSubsystem.cpp")
vegetation = read(PRIVATE / "OCR143MuseumSiteVegetationSubsystem.cpp")
rear = read(PRIVATE / "OCR144MuseumRearExteriorDetailSubsystem.cpp")
trees = read(PRIVATE / "OCR145MuseumTreeLayoutSubsystem.cpp")
validation = read(PRIVATE / "OCR138MuseumRuntimeValidationSubsystem.cpp")
startup = read(PRIVATE / "OCLandmarkStartupCoordinatorSubsystem.cpp")
windows_launcher = read(ROOT / "VALIDATE_MUSEUM_UE58.cmd")

# UE 5.8 compile contracts learned from factual Windows builds.
require("bUseUnity = false;" in build_rules,
        "OsterConflict unity build must stay disabled until file-local helper symbols are uniquely namespaced")
require("GetActorRelativeLocation() const" in weapon_base,
        "weapon base lost UE 5.8 relative-location getter required by first-person presentation")
require("GetActorRelativeRotation() const" in weapon_base,
        "weapon base lost UE 5.8 relative-rotation getter required by first-person presentation")
require("WeaponRoot->GetRelativeLocation()" in weapon_base,
        "weapon relative-location getter no longer reads the authoritative weapon root")
require("WeaponRoot->GetRelativeRotation()" in weapon_base,
        "weapon relative-rotation getter no longer reads the authoritative weapon root")

# Pass45 Museum ownership. R13.7 is the only visible exterior shell.
for token in [
    "PASS45_MUSEUM_R137_PRIMARY_EXTERIOR_READY",
    "visible_shell_owner=R137",
    "static_glass=0",
    "prototype_doors=0",
    "prototype_trees=0",
    "prototype_service_gable=0",
    "breakable actor owns visible glass",
]:
    require(token in r137, f"R13.7 single-visible-owner contract missing: {token}")
for forbidden in [
    "R137Museum_RedTimberGable",
    "R137Museum_Pine01",
    "R137Museum_Pine03",
    "R137Museum_Deciduous01",
    "FVector(-62.0f, -672.0f, 205.0f)",
    "FVector(965.0f, -155.0f, 385.0f)",
]:
    require(forbidden not in r137, f"R13.7 obsolete prototype ownership returned: {forbidden}")

# R13.8 owns hidden interaction collision plus final styled breakable glass, never a second visible shell.
for token in [
    'TEXT("R138_MuseumInteractionCollision")',
    'TEXT("MuseumInteractionCollision")',
    'TEXT("Section:%s")',
    "Component->SetVisibility(false, true)",
    "Component->SetHiddenInGame(true, true)",
    "AOCMuseumBreakableWindow",
    "BuildInteractionCollisionArchitecture",
    "ReleaseR137StructuralCollision",
    "PASS45_MUSEUM_R137_VISIBLE_OWNER_PRESERVED",
    "PASS45_MUSEUM_R138_COLLISION_ONLY_READY",
    "PASS45_MUSEUM_INTERACTIVE_OPENINGS_READY",
    "final_window_class=1",
    "prototype_doors=0",
    "PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_READY",
    "visible_owner=R137",
    "interaction_owner=R138",
    "visible_shell_duplication=0",
]:
    require(token in architecture_h + architecture, f"R13.8 collision/glass ownership contract missing: {token}")
for forbidden in [
    "R138_MuseumHighFidelityArchitecture",
    "MuseumStructural",
    "BuildSegmentedArchitecture",
    "SuppressSolidPrototype",
    "MakeMuseumMID",
    "SpawnMuseumDoor",
]:
    require(forbidden not in architecture_h + architecture,
            f"old visible/prototype R13.8 ownership returned: {forbidden}")

# Current final door owners are direct; no R13.8 prototype door replacement chain exists.
require("AOCMuseumDoubleDoor" in main_door_replacement,
        "main entrance owner no longer uses final museum double door")
require('TEXT("MuseumMainDoubleDoor")' in main_door_replacement,
        "main double-door tag missing")
require('TEXT("MuseumServiceDoubleDoor")' in facade,
        "final service double-door tag missing")
require("SpawnMuseumDoor" not in architecture,
        "R13.8 generic prototype door actor returned")

# Interactive window actor is glass/debris only. R13.7 supplies visible frame/grille geometry.
for token in [
    "Glass_Window.Glass_Window",
    "PASS45_MUSEUM_WINDOW_GLASS_ONLY_READY",
    "visible_frame_owner=R137",
    "interactive_frame_visible=0",
    "Component->SetVisibility(false, true)",
    "Component->SetHiddenInGame(true, true)",
]:
    require(token in window, f"Museum glass-only window contract missing: {token}")
require('TEXT("MuseumUpperGableWindow")' in facade,
        "upper service-gable breakable window missing")
require("AOCMuseumBreakableWindow" in facade,
        "upper gable window is not created directly as final styled glass")

# R14.0 is additive final detail only; R14.5 is the sole current Museum tree author.
for token in [
    "PASS45_MUSEUM_R140_DETAIL_ONLY_READY",
    "late_r137_suppression=0",
    "instance_removal=0",
]:
    require(token in facade, f"R14.0 detail-only contract missing: {token}")
for forbidden in ["SuppressIncorrectR137GableAndCanopy", "WrongCanopyTarget", "RemoveInstance("]:
    require(forbidden not in facade, f"R14.0 late mutation returned: {forbidden}")
require('TEXT("R143_MuseumSiteVegetation")' in vegetation, "museum low-vegetation layer missing")
require('TEXT("R144_MuseumRearExteriorDetail")' in rear, "museum rear-exterior layer missing")
require('TEXT("R145_MuseumPhotoTreeLayout")' in trees, "museum photo-oriented tree layer missing")
for token in [
    "PASS45_MUSEUM_TREE_SINGLE_OWNER_READY",
    "owner=R145",
    "r137_tree_pass=0",
    "late_hide=0",
    "const FTreeSeed Seeds[]",
]:
    require(token in trees, f"R14.5 single tree-owner contract missing: {token}")
require("HideR137MuseumTrees" not in trees,
        "R14.5 again hides a temporary R13.7 tree pass instead of owning trees directly")
require(trees.count("FVector(") >= 14, "R14.5 tree layout became unexpectedly sparse")

# Runtime object naming contracts. Dynamic component/MID owners keep unique UObject names.
for path, text in [
    (PRIVATE / "OCR138MuseumInteractiveArchitectureSubsystem.cpp", architecture),
    (PRIVATE / "OCR140MuseumFacadeDetailSubsystem.cpp", facade),
    (PRIVATE / "OCR142MuseumEntranceDetailSubsystem.cpp", entrance),
    (PRIVATE / "OCR143MuseumSiteVegetationSubsystem.cpp", vegetation),
    (PRIVATE / "OCR144MuseumRearExteriorDetailSubsystem.cpp", rear),
    (PRIVATE / "OCR145MuseumTreeLayoutSubsystem.cpp", trees),
]:
    require("MakeUniqueObjectName" in text, f"runtime unique UObject naming missing in {path.name}")
require("MakeUniqueObjectName" in main_door, "main museum door MID names are not unique")
require("MakeUniqueObjectName" in service_door, "service museum door MID names are not unique")
require("MakeUniqueObjectName" in window, "museum window MID name is not unique")

# Startup ownership is coordinated explicitly. Historical stage timers are cleared before direct current-stage calls.
for token in [
    "Timers.ClearAllTimersForObject(Stage)",
    "PASS45_LANDMARK_STARTUP_COORDINATED_READY",
    "R138_collision_glass",
    "R139_R140_doors_facade",
    "R142_R145_details",
    "window_replacement_stage=0",
    "legacy_core_recovery=0",
    "destructive_visibility_rebuild=0",
]:
    require(token in startup, f"Pass45 Museum startup coordinator contract missing: {token}")
require("OCR141MuseumWindowReplacementSubsystem" not in startup,
        "startup coordinator resurrected retired R14.1 window replacement")

# Remaining fallback delays are still monotonic when a stage runs outside the coordinator, but R14.1 is gone.
def delay(text: str, symbol: str) -> float:
    match = re.search(rf"constexpr\s+float\s+{re.escape(symbol)}\s*=\s*([0-9.]+)f", text)
    require(match is not None, f"delay symbol missing: {symbol}")
    return float(match.group(1)) if match else -1.0

layer_delays = [
    delay(architecture, "R138MuseumDelaySeconds"),
    delay(main_door_replacement, "R139MuseumMainDoorDelaySeconds"),
    delay(facade, "R140FacadeDelaySeconds"),
    delay(entrance, "R142EntranceDetailDelaySeconds"),
    delay(vegetation, "R143SiteVegetationDelaySeconds"),
    delay(rear, "R144RearDetailDelaySeconds"),
    delay(trees, "R145TreeLayoutDelaySeconds"),
]
require(layer_delays == sorted(layer_delays), f"remaining Museum fallback delays are out of order: {layer_delays}")
validation_delay = delay(validation, "R138MuseumValidationDelaySeconds")
require(validation_delay > max(layer_delays), "Museum runtime validator runs before the final fallback detail layer")

# R14.4 cylinder orientation contract: UE basic Cylinder is Z-axis aligned; horizontal X gutters require pitch=90.
require(rear.count("FRotator(90.0f, 0.0f, 0.0f)") >= 4,
        "R14.4 horizontal gutter/elbow cylinders are not rotated from Z onto X")
require("FRotator(0.0f, 90.0f, 0.0f)" not in rear,
        "R14.4 contains yaw-only cylinder rotation that leaves the cylinder vertical")

# Runtime validation now proves the current single-owner state, not the deleted R13.8 visible shell/R14.1 replacement chain.
for token in [
    'Actor->ActorHasTag(TEXT("R137_MuseumPhotoModel"))',
    'Actor->ActorHasTag(TEXT("R138_MuseumInteractionCollision"))',
    'Component->ComponentHasTag(TEXT("MuseumInteractionCollision"))',
    "R137VisibleExteriorActors == 1",
    "CollisionActors == 1",
    "CollisionSections >= 30",
    "VisibleCollisionSections == 0",
    "FacadeDetailActors == 1",
    "EntranceDetailActors == 1",
    "SiteVegetationActors == 1",
    "RearExteriorActors == 1",
    "TreeLayoutActors == 1",
    "MainDoorActors == 1",
    "ServiceDoorActors == 1",
    "PrototypeServiceDoors == 0",
    "PrototypeMainDoorLeaves == 0",
    "StyledMuseumWindows == BreakableWindows",
    "PrototypeMuseumWindows == 0",
    "UpperGableWindows == 1",
    "InitiallyBrokenWindows == 0",
    "PASS45_MUSEUM_RUNTIME_SINGLE_OWNER_READY",
    "PASS45_MUSEUM_RUNTIME_SINGLE_OWNER_FAIL",
]:
    require(token in validation, f"current Museum runtime validation contract missing: {token}")
require("R138_MuseumHighFidelityArchitecture" not in validation,
        "runtime validator still requires deleted visible R13.8 architecture")

# Windows validation launcher remains interactive: build, Sandbox runtime, parse current validation evidence.
require("OsterConflictEditor Win64 Development" in windows_launcher,
        "Museum Windows launcher no longer builds the editor")
require("/Game/Maps/OsterConflict_Runtime?Mode=Sandbox" in windows_launcher,
        "Museum Windows launcher lost Sandbox runtime map")
require('R14.5 museum validation PASS' in windows_launcher,
        "Museum Windows launcher lost current validation PASS marker check")
require("necho " not in windows_launcher.lower(), "Museum Windows launcher contains mistyped necho command")
object_collision_pos = windows_launcher.find("Cannot replace existing object of a different class")
fatal_pos = windows_launcher.find('findstr /I /C:"Fatal error"')
require(object_collision_pos >= 0, "Museum Windows launcher lost UObject collision check")
require(fatal_pos >= 0, "Museum Windows launcher lost fatal-error check")
require(object_collision_pos < fatal_pos,
        "specific UObject collision check must run before generic fatal-error check")

# Photo-fidelity guardrails. Do not fabricate the unreadable historical wall inscription.
require("historical inscription" not in facade.lower(),
        "facade code claims an unverified historical inscription")
require('FromString(TEXT("30"))' in facade, "verified address number 30 detail missing")

if failures:
    print("Museum source contracts FAILED:")
    for failure in failures:
        print(f" - {failure}")
    sys.exit(1)

print("Museum source contracts PASS")
print(f"Current fallback layer delays: {layer_delays}; validation={validation_delay}")
print("- R13.7 is the sole visible exterior; R13.8 is hidden collision + final breakable glass")
print("- retired R14.1 replacement stage stays physically absent")
print("- R14.0 is additive-only and R14.5 is the sole Museum tree owner")
print("- runtime validator proves one visible owner, hidden collision and final door/window state")
print("STATUS: SOURCE CONTRACT ONLY; factual UE 5.8 runtime remains authoritative")
