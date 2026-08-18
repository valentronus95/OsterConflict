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
        "VERIFY_R13_OSTER_PROP_ART.py",
        "VERIFY_R13_RUNTIME_REGRESSIONS.py",
        "VERIFY_R13_PC_TEST_HARDENING.py",
        "VERIFY_R13_GAMEPLAY_POLISH.py",
        "VERIFY_R13_STEIN_WEAPONS.py",
    ]:
        run_verifier(ROOT / name)

    print("ALL SOURCE + R13 RUNTIME REGRESSIONS + R13 OSTER PROP ART + R13 STEIN WEAPONS + R13 GAMEPLAY POLISH + R13 PC HARDENING + R11 VISUAL + R10 CXX + R9 UHT + R8 UE5.8 TARGET + R7 LOGIC/PHYSICS VERIFIERS: PASS")
finally:
    restore_local_generated_dirs(moved_generated)
