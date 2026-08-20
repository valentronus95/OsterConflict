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


required_files = [
    SOURCE / "OsterConflict.Build.cs",
    PUBLIC / "OCWeaponBase.h",
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
    PUBLIC / "OCR141MuseumWindowReplacementSubsystem.h",
    PRIVATE / "OCR141MuseumWindowReplacementSubsystem.cpp",
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
    ROOT / "VALIDATE_MUSEUM_UE58.cmd",
]

for path in required_files:
    read(path)

build_rules = read(SOURCE / "OsterConflict.Build.cs")
weapon_base = read(PUBLIC / "OCWeaponBase.h")
architecture = read(PRIVATE / "OCR138MuseumInteractiveArchitectureSubsystem.cpp")
main_door = read(PRIVATE / "OCMuseumDoubleDoor.cpp")
main_door_replacement = read(PRIVATE / "OCR139MuseumMainDoorReplacementSubsystem.cpp")
service_door = read(PRIVATE / "OCMuseumServiceDoubleDoor.cpp")
facade = read(PRIVATE / "OCR140MuseumFacadeDetailSubsystem.cpp")
window = read(PRIVATE / "OCMuseumBreakableWindow.cpp")
window_replacement = read(PRIVATE / "OCR141MuseumWindowReplacementSubsystem.cpp")
entrance = read(PRIVATE / "OCR142MuseumEntranceDetailSubsystem.cpp")
vegetation = read(PRIVATE / "OCR143MuseumSiteVegetationSubsystem.cpp")
rear = read(PRIVATE / "OCR144MuseumRearExteriorDetailSubsystem.cpp")
trees = read(PRIVATE / "OCR145MuseumTreeLayoutSubsystem.cpp")
validation = read(PRIVATE / "OCR138MuseumRuntimeValidationSubsystem.cpp")
windows_launcher = read(ROOT / "VALIDATE_MUSEUM_UE58.cmd")

# UE 5.8 compile contracts learned from the first real Windows build.
# Several legacy/new museum .cpp files intentionally use common anonymous-namespace helper names.
# Unity build concatenates those files and causes C2084/C2374/C2572 redefinitions, so keep it disabled.
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

# Structural/gameplay contracts.
require('TEXT("MuseumStructural")' in architecture, "segmented architecture lost MuseumStructural tags")
require('TEXT("Section:%s")' in architecture, "segmented architecture lost stable Section:* IDs")
require('TEXT("R138_MuseumHighFidelityArchitecture")' in architecture, "missing R13.8 architecture actor tag")
require("AOCBreakableWindow" in architecture, "R13.8 no longer spawns breakable museum openings")
require("AOCMuseumDoubleDoor" in main_door_replacement, "main entrance replacement no longer uses museum double door")
require('TEXT("MuseumMainDoubleDoor")' in main_door_replacement, "main double-door tag missing")
require('TEXT("MuseumServiceDoubleDoor")' in facade, "final service double-door tag missing")
require("AOCMuseumBreakableWindow" in window_replacement, "museum window replacement lost styled breakable subclass")
require("Glass_Window.Glass_Window" in window, "museum breakable windows lost real glass material")
require('TEXT("MuseumUpperGableWindow")' in facade, "upper service-gable breakable window missing")
require('TEXT("R143_MuseumSiteVegetation")' in vegetation, "museum low-vegetation layer missing")
require('TEXT("R144_MuseumRearExteriorDetail")' in rear, "museum rear-exterior layer missing")
require('TEXT("R145_MuseumPhotoTreeLayout")' in trees, "museum photo-oriented tree layer missing")
require('TEXT("R137Museum_Pine01")' in trees and 'TEXT("R137Museum_Pine03")' in trees,
        "R14.5 no longer suppresses the symmetric R13.7 pine layout")

# Runtime object naming contracts. These source files dynamically create MIDs/components and must not
# fall back to fixed names, which previously caused a fatal UObject class/name replacement collision.
for path, text in [
    (PRIVATE / "OCR138MuseumInteractiveArchitectureSubsystem.cpp", architecture),
    (PRIVATE / "OCR140MuseumFacadeDetailSubsystem.cpp", facade),
    (PRIVATE / "OCR142MuseumEntranceDetailSubsystem.cpp", entrance),
    (PRIVATE / "OCR143MuseumSiteVegetationSubsystem.cpp", vegetation),
    (PRIVATE / "OCR144MuseumRearExteriorDetailSubsystem.cpp", rear),
    (PRIVATE / "OCR145MuseumTreeLayoutSubsystem.cpp", trees),
]:
    require("MakeUniqueObjectName" in text, f"runtime unique UObject naming missing in {path.name}")

# Door classes are constructor-owned default subobjects, but dynamic material instances must still use unique names.
require("MakeUniqueObjectName" in main_door, "main museum door MID names are not unique")
require("MakeUniqueObjectName" in service_door, "service museum door MID names are not unique")
require("MakeUniqueObjectName" in window, "museum window MID name is not unique")

# Startup ordering. The validator must run after all replacement/detail passes.
def delay(text: str, symbol: str) -> float:
    match = re.search(rf"constexpr\s+float\s+{re.escape(symbol)}\s*=\s*([0-9.]+)f", text)
    require(match is not None, f"delay symbol missing: {symbol}")
    return float(match.group(1)) if match else -1.0

layer_delays = [
    delay(architecture, "R138MuseumDelaySeconds"),
    delay(main_door_replacement, "R139MuseumMainDoorDelaySeconds"),
    delay(facade, "R140FacadeDelaySeconds"),
    delay(window_replacement, "R141WindowReplacementDelaySeconds"),
    delay(entrance, "R142EntranceDetailDelaySeconds"),
    delay(vegetation, "R143SiteVegetationDelaySeconds"),
    delay(rear, "R144RearDetailDelaySeconds"),
    delay(trees, "R145TreeLayoutDelaySeconds"),
]
require(layer_delays == sorted(layer_delays), f"museum runtime layer delays are out of order: {layer_delays}")
validation_delay = delay(validation, "R138MuseumValidationDelaySeconds")
require(validation_delay > max(layer_delays), "museum validator runs before the final detail layer")

# R14.4 cylinder orientation contract: UE basic Cylinder is Z-axis aligned; horizontal X gutters require pitch=90.
require(rear.count("FRotator(90.0f, 0.0f, 0.0f)") >= 4,
        "R14.4 horizontal gutter/elbow cylinders are not rotated from Z onto X")
require("FRotator(0.0f, 90.0f, 0.0f)" not in rear,
        "R14.4 contains yaw-only cylinder rotation that leaves the cylinder vertical")

# R14.5 tree layout must stay intentionally asymmetrical and must not block the approach centreline.
require("const FTreeSeed Seeds[]" in trees, "R14.5 tree seed table missing")
require(trees.count("FVector(") >= 14, "R14.5 tree layout became unexpectedly sparse")
require("HideR137MuseumTrees(World);" in trees, "R14.5 does not suppress the old symmetric tree pass")

# Final validation must explicitly enforce the post-replacement state.
for token in [
    "FacadeDetailActors == 1",
    "EntranceDetailActors == 1",
    "SiteVegetationActors == 1",
    "RearExteriorActors == 1",
    "TreeLayoutActors == 1",
    "MainDoorActors == 1",
    "ServiceDoorActors == 1",
    "PrototypeServiceDoors == 0",
    "StyledMuseumWindows == BreakableWindows",
    "PrototypeMuseumWindows == 0",
    "UpperGableWindows == 1",
    "InitiallyBrokenWindows == 0",
]:
    require(token in validation, f"runtime validation contract missing: {token}")

# Windows validation launcher contract. It is intentionally interactive: build, launch Sandbox, then parse the log.
require("OsterConflictEditor Win64 Development" in windows_launcher, "museum Windows launcher no longer builds the editor")
require("/Game/Maps/OsterConflict_Runtime?Mode=Sandbox" in windows_launcher, "museum Windows launcher lost Sandbox runtime map")
require('R14.5 museum validation PASS' in windows_launcher, "museum Windows launcher lost R14.5 PASS marker check")
require("necho " not in windows_launcher.lower(), "museum Windows launcher contains mistyped necho command")
object_collision_pos = windows_launcher.find("Cannot replace existing object of a different class")
fatal_pos = windows_launcher.find('findstr /I /C:"Fatal error"')
require(object_collision_pos >= 0, "museum Windows launcher lost UObject collision check")
require(fatal_pos >= 0, "museum Windows launcher lost fatal-error check")
require(object_collision_pos < fatal_pos, "specific UObject collision check must run before generic fatal-error check")

# Photo-fidelity guardrails. Do not fabricate the unreadable historical wall inscription.
require("historical inscription" not in facade.lower(), "facade code claims an unverified historical inscription")
require('FromString(TEXT("30"))' in facade, "verified address number 30 detail missing")

if failures:
    print("Museum source contracts FAILED:")
    for failure in failures:
        print(f" - {failure}")
    sys.exit(1)

print("Museum source contracts PASS")
print(f"Layer delays: {layer_delays}; validation={validation_delay}")
