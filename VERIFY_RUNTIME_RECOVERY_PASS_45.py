#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
PRIVATE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private"
errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def require(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


tz = read(ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md")
recovery_tz = read(ROOT / "GAME_RECOVERY.md")
launcher = read(ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd")
startup = read(PRIVATE / "OCLandmarkStartupCoordinatorSubsystem.cpp")

require("RUNTIME REJECTED" in tz and "RUNTIME ACCEPTANCE DEFERRED" in tz,
        "Pass45 runtime rejection/deferred acceptance truth was lost")
for flag in ("runtime_acceptance=0", "merge_permitted=0"):
    require(flag in tz, f"Pass45 factual acceptance flag missing: {flag}")

require("fix/pass45-runtime-rejection-material-closure-20260826" in recovery_tz,
        "GAME_RECOVERY single working branch contract is missing")
require("Definition of Done" in recovery_tz and "UE 5.8 runtime" in recovery_tz,
        "GAME_RECOVERY no longer requires factual UE 5.8 runtime acceptance")

for token in ("-d3d11", "-sm5", "-nohdr", "-nosplash"):
    require(token in launcher, f"normal launcher renderer token missing: {token}")
require('set "RHI_FLAGS=-d3d11 -sm5 -nohdr -nosplash"' in launcher,
        "normal DX11/SM5 RHI-thread baseline is missing")
require('if /I "%OC_RHI_COMPAT%"=="1"' in launcher and "-norhithread" in launcher,
        "explicit no-RHI-thread compatibility route is missing")
require("-d3d12" not in launcher.lower() and "-sm6" not in launcher.lower(),
        "normal recovery route must not force D3D12/SM6")
require("t.MaxFPS 60" in launcher,
        "normal recovery route lost the 60 FPS thermal guard")

parts = launcher.split(":quick_normal_game", 1)
require(len(parts) == 2, "quick-normal launcher section is missing")
if len(parts) == 2:
    quick = parts[1]
    require("-windowed" in quick.lower(), "quick-normal route is not windowed")
    res_x = re.search(r"-ResX=(\d+)", quick, re.IGNORECASE)
    res_y = re.search(r"-ResY=(\d+)", quick, re.IGNORECASE)
    require(res_x is not None and int(res_x.group(1)) >= 1280,
            "quick-normal horizontal resolution is below 1280")
    require(res_y is not None and int(res_y.group(1)) >= 720,
            "quick-normal vertical resolution is below 720")

for token in (
    "CancelHistoricalStageTimers",
    "GAME_RECOVERY_WORLD_PREP_TIMERS_CANCELLED",
    "UOCGameRecoveryStadiumActivationSubsystem",
    "IsStadiumPresentationReady",
    "GAME_RECOVERY_WORLD_READY",
    "stadium_ready=1",
    "post_spawn_landmark_materialization=0",
):
    require(token in startup, f"current staged landmark startup contract missing: {token}")

delegated = (
    "VERIFY_SLATE_RENDER_TARGET_STARTUP_PASS_43.py",
    "VERIFY_DX11_SM5_RENDER_TARGET_PASS_23.py",
    "VERIFY_PASS45_GRENADE_TYPE_PRESENTATION_LIFECYCLE.py",
    "VERIFY_PASS45_GRENADE_SMOKE_PRIMITIVE_RETIREMENT.py",
    "VERIFY_PASS45_BTR4_MATERIAL_STATE.py",
    "VERIFY_PASS45_BTR4_AXIS_REMOTE_OPTIC.py",
    "VERIFY_PASS45_HMMWV_M2_HIERARCHY.py",
    "VERIFY_GAME_RECOVERY_STADIUM_PRELOAD.py",
    "VERIFY_PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RETIREMENT.py",
    "VERIFY_PASS45_PRIMITIVE_WEAPON_RETIREMENT.py",
)

for name in delegated:
    script = ROOT / name
    if not script.is_file():
        errors.append(f"delegated current source gate missing: {name}")
        continue
    print(f"[GAME_RECOVERY] running {name}")
    result = subprocess.run([sys.executable, str(script)], cwd=ROOT, check=False)
    if result.returncode != 0:
        errors.append(f"delegated current source gate failed: {name} rc={result.returncode}")

if errors:
    print("RUNTIME RECOVERY PASS 45: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("RUNTIME RECOVERY PASS 45: PASS")
print("- current recovery gate delegates specialized source contracts instead of duplicating stale assertions")
print("- DX11/SM5 startup, grenades, production vehicles, stadium, weapon proxy retirement and reference-driven map rules are guarded")
print("- staged landmark readiness uses current GAME_RECOVERY markers")
print("STATUS: SOURCE CONTRACT ONLY; factual UE 5.8 runtime remains authoritative")
