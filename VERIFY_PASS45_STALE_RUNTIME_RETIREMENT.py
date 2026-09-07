#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
errors = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


# This verifier has one job: keep factually retired runtime owners retired and ensure
# the current launcher/policy cannot silently resurrect rejected startup behaviour.
# Detailed Museum, vehicle, grenade, park and visual contracts have their own current verifiers.
retired_paths = [
    SRC / "Public" / "OCWorldProductionVisualsSubsystem.h",
    SRC / "Private" / "OCWorldProductionVisualsSubsystem.cpp",
    SRC / "Public" / "OCWorldAssetModelsSubsystem.h",
    SRC / "Private" / "OCWorldAssetModelsSubsystem.cpp",
    SRC / "Public" / "OCAssetModelDecorator.h",
    SRC / "Private" / "OCAssetModelDecorator.cpp",
    SRC / "Public" / "OCRecoveredEnvironmentSubsystem.h",
    SRC / "Private" / "OCRecoveredEnvironmentSubsystem.cpp",
    SRC / "Public" / "OCRecoveredBuildingDetailsSubsystem.h",
    SRC / "Private" / "OCRecoveredBuildingDetailsSubsystem.cpp",
    SRC / "Public" / "OCMuseumCoreRecoverySubsystem.h",
    SRC / "Private" / "OCMuseumCoreRecoverySubsystem.cpp",
    SRC / "Public" / "OCMuseumVisibilityPass37Subsystem.h",
    SRC / "Private" / "OCMuseumVisibilityPass37Subsystem.cpp",
    SRC / "Public" / "OCLandmarkShellOwnershipGuardSubsystem.h",
    SRC / "Private" / "OCLandmarkShellOwnershipGuardSubsystem.cpp",
    SRC / "Public" / "OCR137MuseumSiteReplacementSubsystem.h",
    SRC / "Private" / "OCR137MuseumSiteReplacementSubsystem.cpp",
    SRC / "Public" / "OCR13MuseumStadiumPhotoFidelitySubsystem.h",
    SRC / "Private" / "OCR13MuseumStadiumPhotoFidelitySubsystem.cpp",
    SRC / "Public" / "OCR141MuseumWindowReplacementSubsystem.h",
    SRC / "Private" / "OCR141MuseumWindowReplacementSubsystem.cpp",
    SRC / "Public" / "OCWeaponPalettePass37Subsystem.h",
    SRC / "Private" / "OCWeaponPalettePass37Subsystem.cpp",
    ROOT / "VERIFY_PASS45_COMPLETION_AUDIT.py",
    ROOT / "VERIFY_OSTER_WORLD_MODELS_PASS.py",
    ROOT / ".github" / "workflows" / "pass45-completion-audit.yml",
    ROOT / ".github" / "workflows" / "oster-world-models-pass.yml",
    ROOT / ".github" / "workflows" / "pass45-targeted-source-patch.yml",
    ROOT / ".github" / "workflows" / "pass45-vehicle-transform-trace-patch.yml",
    ROOT / ".github" / "workflows" / "pass45-museum-ownership-cleanup-patch.yml",
]
for path in retired_paths:
    req(not path.exists(), f"stale/rejected runtime contract resurrected: {path.relative_to(ROOT)}")

retired_class_names = (
    "OCWorldProductionVisualsSubsystem",
    "OCWorldAssetModelsSubsystem",
    "OCAssetModelDecorator",
    "OCRecoveredEnvironmentSubsystem",
    "OCRecoveredBuildingDetailsSubsystem",
    "OCMuseumCoreRecoverySubsystem",
    "OCMuseumVisibilityPass37Subsystem",
    "OCLandmarkShellOwnershipGuardSubsystem",
    "OCR137MuseumSiteReplacementSubsystem",
    "OCR13MuseumStadiumPhotoFidelitySubsystem",
    "OCR141MuseumWindowReplacementSubsystem",
    "OCWeaponPalettePass37Subsystem",
)
for path in list(SRC.rglob("*.cpp")) + list(SRC.rglob("*.h")):
    text = path.read_text(encoding="utf-8", errors="replace")
    for class_name in retired_class_names:
        req(class_name not in text,
            f"retired runtime class referenced by active source: {class_name} in {path.relative_to(ROOT)}")

startup = read(SRC / "Private" / "OCLandmarkStartupCoordinatorSubsystem.cpp")
launcher = read(ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd")
agents = read(ROOT / "AGENTS.md")

# Retired R14.1 Museum window stage must stay out of the active startup coordinator.
req("OCR141MuseumWindowReplacementSubsystem" not in startup,
    "retired R14.1 Museum window replacement stage returned to coordinator")
req("window_replacement_stage=0" in startup,
    "current startup coordinator no longer records retired window stage=0")

# Strict recovery remains fullscreen. Quick normal stays windowed so a failed startup cannot trap desktop focus.
launcher_parts = launcher.split(":quick_normal_game", 1)
req(len(launcher_parts) == 2, "canonical gameplay launcher is missing quick-normal split")
strict_launcher = launcher_parts[0] if launcher_parts else launcher
quick_launcher = launcher_parts[1] if len(launcher_parts) == 2 else ""
req(' -windowed ' not in strict_launcher.lower(), "strict gameplay launcher restored forced -windowed mode")
req("-fullscreen" in strict_launcher, "strict gameplay launcher no longer requests fullscreen recovery mode")
req("-windowed" in quick_launcher.lower(), "quick normal launcher is not desktop-recoverable windowed mode")

# Both launch paths consume one shared QUALITY_CMDS source of truth. Do not duplicate t.MaxFPS in each branch
# just to satisfy a substring test: prove the shared cap exists and both actual editor launches consume it.
req('set "QUALITY_CMDS=t.MaxFPS 60,' in strict_launcher,
    "shared thermal recovery 60 FPS cap is missing")
req('-ExecCmds="%QUALITY_CMDS%"' in strict_launcher,
    "strict gameplay launch does not consume shared QUALITY_CMDS")
req('-ExecCmds="%QUALITY_CMDS%"' in quick_launcher,
    "quick normal launch does not consume shared QUALITY_CMDS")
req("Thermal recovery cap: 60 FPS" in strict_launcher,
    "strict launcher lacks visible 60 FPS thermal recovery notice")

# Current root rules are semantic policy, not magic wording. Verify the actual binding rules that replaced
# historical phrases so production code is never distorted merely to make an obsolete string search green.
for needle in (
    "Superseded/rejected mutation owners are physically retired",
    "No historical verifier may require a runtime-rejected owner/fallback back into production",
    "stale verifiers are updated/demoted/deleted",
    "Never distort production code to satisfy obsolete tests",
):
    req(needle in agents, f"current stale-runtime retirement policy missing: {needle}")

if errors:
    print("PASS45 STALE RUNTIME RETIREMENT: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 STALE RUNTIME RETIREMENT: PASS")
print("- rejected historical runtime owners and temporary workflows remain physically retired")
print("- retired Museum window stage cannot return to the coordinated startup chain")
print("- strict recovery is fullscreen; quick normal is windowed")
print("- both launch modes consume one shared 60 FPS thermal cap")
print("- current root rules require physical retirement and stale-verifier cleanup without production-code distortion")
print("STATUS: SOURCE CONTRACT ONLY; local UE runtime remains authoritative")
