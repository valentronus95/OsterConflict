from pathlib import Path
import os
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
PROJECT = ROOT / "OsterConflict"
HOLD = ROOT / ".source_verify_hold"
GENERATED_DIRS = ("Binaries", "Intermediate", "Saved", "DerivedDataCache")
ENV = os.environ.copy()
ENV["PYTHONUTF8"] = "1"
ENV["PYTHONIOENCODING"] = "utf-8"


def run_verifier(path: Path) -> None:
    print(f"===== {path.name} =====")
    result = subprocess.run([sys.executable, str(path)], cwd=ROOT, env=ENV)
    if result.returncode:
        raise SystemExit(result.returncode)


def tracked_generated_paths() -> list[str]:
    try:
        result = subprocess.run(
            ["git", "ls-files"], cwd=ROOT, text=True, capture_output=True, check=True
        )
    except (OSError, subprocess.CalledProcessError) as exc:
        raise SystemExit(f"SOURCE VERIFY FAIL: cannot query git index: {exc}")

    prefixes = tuple(f"OsterConflict/{name}/" for name in GENERATED_DIRS)
    return [line for line in result.stdout.splitlines() if line.replace("\\", "/").startswith(prefixes)]


def recover_stale_hold() -> None:
    if not HOLD.exists():
        return
    for name in GENERATED_DIRS:
        held = HOLD / name
        live = PROJECT / name
        if not held.exists():
            continue
        if live.exists():
            raise SystemExit(
                f"SOURCE VERIFY FAIL: both live and held generated directories exist for {name}; "
                f"inspect {HOLD} before continuing"
            )
        try:
            held.rename(live)
        except OSError as exc:
            raise SystemExit(
                f"SOURCE VERIFY FAIL: cannot restore generated directory {name}: {exc}"
            ) from None
    try:
        HOLD.rmdir()
    except OSError:
        pass


def restore_partial_hold(moved: list[str]) -> None:
    for name in reversed(moved):
        held = HOLD / name
        live = PROJECT / name
        if not held.exists() or live.exists():
            continue
        try:
            held.rename(live)
        except OSError:
            pass
    if HOLD.exists():
        try:
            HOLD.rmdir()
        except OSError:
            pass


def hide_local_generated_dirs() -> list[str]:
    offenders = tracked_generated_paths()
    if offenders:
        print("SOURCE VERIFY FAIL: generated UE output is tracked by Git:")
        for path in offenders[:50]:
            print(f" - {path}")
        raise SystemExit(1)

    moved: list[str] = []
    try:
        for name in GENERATED_DIRS:
            live = PROJECT / name
            if not live.exists():
                continue
            HOLD.mkdir(exist_ok=True)
            held = HOLD / name
            if held.exists():
                raise RuntimeError(f"hold path already exists: {held}")
            live.rename(held)
            moved.append(name)
    except (OSError, RuntimeError) as exc:
        restore_partial_hold(moved)
        raise SystemExit(
            "SOURCE VERIFY FAIL: cannot temporarily isolate Unreal generated folders. "
            "Close OsterConflict, Unreal Editor and any local/dedicated server, then run validation again. "
            f"Details: {exc}"
        ) from None

    if moved:
        print("Local UE generated directories temporarily excluded from source-only checks: " + ", ".join(moved))
    return moved


def restore_local_generated_dirs(moved: list[str]) -> None:
    for name in reversed(moved):
        held = HOLD / name
        live = PROJECT / name
        if not held.exists():
            continue
        if live.exists():
            raise RuntimeError(f"cannot restore {name}: destination already exists: {live}")
        held.rename(live)
    if HOLD.exists():
        try:
            HOLD.rmdir()
        except OSError:
            pass


recover_stale_hold()
moved_generated = hide_local_generated_dirs()
try:
    ordered = [
        "S04", "S05", "S06", "S07", "S08", "S09", "S10", "S11", "S12", "S13",
        "S14A", "S14B", "S15A", "S15B", "S16A", "S16B", "S16C", "S17A", "S17B",
        "S18A", "S18B", "S18C",
    ]
    for tag in ordered:
        verifier = ROOT / f"VERIFY_{tag}.py"
        if verifier.exists():
            run_verifier(verifier)

    for name in [
        "VERIFY_S18C_HARDENING_R1.py",
        "VERIFY_S19C_SOURCE.py",
        "VERIFY_R6_LAUNCH_KIT.py",
        "VERIFY_R7_LOGIC_PHYSICS.py",
        "VERIFY_R8_UE58_TARGETS.py",
        "VERIFY_R9_UHT_TRAUMA.py",
        "VERIFY_R10_CXX_BATCH_FIX.py",
        "VERIFY_R11_VISUAL_FOUNDATION.py",
        "VERIFY_R13_LOCATION_FIRST_S01.py",
        "VERIFY_R13_LOCATION_FIRST_S01_VEGETATION_DATA.py",
        "VERIFY_R13_LOCATION_PLAYTEST_SAFETY.py",
        "VERIFY_R13_FRONTEND_MENU_GUARD.py",
        "VERIFY_R13_DYNAMIC_COOK_PATHS.py",
        "VERIFY_R13_LFS_LAUNCH_GATE.py",
        "VERIFY_R13_TICKABLE_SUBSYSTEM_LINKAGE.py",
        "VERIFY_R13_1_STABILIZATION.py",
        "VERIFY_R13_3_DEPLOYMENT_SPAWN.py",
        "VERIFY_R13_3_DEPLOYMENT_RECONCILIATION.py",
        "VERIFY_R13_4_ENVIRONMENT_DRESSING.py",
        "VERIFY_R13_4_FOLIAGE_DIVERSITY.py",
        "VERIFY_R13_4_VISUAL_BATCH_CONSOLIDATION.py",
        "VERIFY_R13_6_MAP_PLACEMENT_REPAIR.py",
        "VERIFY_R13_6_VERIFIED_OSTER_GEOGRAPHY.py",
        "VERIFY_R13_6_OSTER_RESIDENTIAL_ARCHITECTURE.py",
        "VERIFY_R13_6_MUSEUM_STADIUM_PHOTO_FIDELITY.py",
        "VERIFY_R13_7_MUSEUM_REPLACEMENT.py",
        "VERIFY_R13_4_RESIDENTIAL_INFILL.py",
        "VERIFY_R13_4_RESIDENTIAL_INFILL_FENCES.py",
        "VERIFY_R13_4_ENTERABLE_HOUSE_POPULATION.py",
        "VERIFY_R13_5_LANDMARK_SITE_DRESSING.py",
        "VERIFY_R13_5_CENTRAL_PARK_DRESSING.py",
        "VERIFY_R13_5_CENTRAL_PARK_CANOPY.py",
        "VERIFY_R13_5_ROADSIDE_INFRASTRUCTURE.py",
        "VERIFY_R13_5_COLLEGE_STADIUM_VISUALS.py",
        "VERIFY_R13_5_MUSEUM_PROTECTION.py",
        "VERIFY_R13_5_VISUAL_ASSET_PATHS.py",
        "VERIFY_R13_OSTER_PROP_ART.py",
        "VERIFY_R13_PARK_FURNITURE.py",
        "VERIFY_R13_LANDMARK_WINDOWS.py",
        "VERIFY_R13_LANDMARK_ROOFS.py",
        "VERIFY_R13_MUSEUM_CHIMNEYS.py",
        "VERIFY_R13_RUNTIME_REGRESSIONS.py",
        "VERIFY_R13_RUNTIME_SPAWN_BRIDGES.py",
        "VERIFY_R13_PC_TEST_HARDENING.py",
        "VERIFY_R13_GAMEPLAY_POLISH.py",
        "VERIFY_R13_6_VEHICLE_GAMEPLAY_REPAIR.py",
        "VERIFY_R13_6_TACTICAL_MAP.py",
        "VERIFY_R13_6_FRONTEND_AUDIO_LAYOUT.py",
        "VERIFY_R13_6_FRAME_RATE_GUARD.py",
        "VERIFY_R13_STEIN_WEAPONS.py",
    ]:
        run_verifier(ROOT / name)

    print("ALL SOURCE + R13 LOCATION PLAYTEST SAFETY + R13.7 MUSEUM REPLACEMENT + R13 LOCATION-FIRST S01 + S01 VEGETATION DATA + R13.6 MAP PLACEMENT + VERIFIED OSTER GEOGRAPHY + OSTER RESIDENTIAL ARCHITECTURE + MUSEUM/STADIUM PHOTO FIDELITY + VEHICLE GAMEPLAY REPAIR + TACTICAL MAP + FRONTEND AUDIO/LAYOUT + FRAME-RATE THERMAL GUARD + R13.5 VISUAL ASSET PATHS + MUSEUM PROTECTION + COLLEGE/STADIUM VISUALS + CENTRAL PARK CANOPY + LANDMARK SITE DRESSING + CENTRAL PARK DRESSING + ROADSIDE INFRASTRUCTURE + R13.4 ENTERABLE HOUSE POPULATION + INFILL FENCES + RESIDENTIAL INFILL + VISUAL BATCH CONSOLIDATION + ENVIRONMENT DRESSING + FOLIAGE DIVERSITY + R13.3 DEPLOYMENT RECONCILIATION + DEPLOYMENT/SPAWN + R13.1 STABILIZATION + R13 FRONTEND MENU GUARD + R13 DYNAMIC COOK PATHS + R13 LFS LAUNCH GATE + R13 TICKABLE SUBSYSTEM LINKAGE + R13 RUNTIME SPAWN BRIDGES + R13 MUSEUM CHIMNEYS + R13 LANDMARK ROOFS + R13 LANDMARK WINDOWS + R13 PARK FURNITURE + R13 RUNTIME REGRESSIONS + R13 OSTER PROP ART + R13 STEIN WEAPONS + R13 GAMEPLAY POLISH + R13 PC HARDENING + R11 VISUAL + R10 CXX + R9 UHT + R8 UE5.8 TARGET + R7 LOGIC/PHYSICS VERIFIERS: PASS")
finally:
    restore_local_generated_dirs(moved_generated)
