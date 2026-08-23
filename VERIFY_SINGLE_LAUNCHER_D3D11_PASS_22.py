#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
START = ROOT / "START_HERE.cmd"
NORMAL = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
RECOVERY = ROOT / "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"
LANDMARK = ROOT / "RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS22 VERIFY FAIL: missing {path.name}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS22 VERIFY FAIL: {label}: missing {needle!r}")


start = read(START)
normal = read(NORMAL)
recovery = read(RECOVERY)
landmark = read(LANDMARK)

# START_HERE is the only user-facing launcher and exposes only normal play, full runtime test and editor.
# Pass 21 remains an internal ownership acceptance helper; exposing it here would violate the single-launcher
# ownership contract. The user-facing full runtime test uses the focused R15 acceptance route.
for needle in (
    "1. ЗВИЧАЙНА ГРА",
    "2. ПОВНИЙ RUNTIME-ТЕСТ",
    "3. ВІДКРИТИ UNREAL EDITOR",
    'call "%~dp0RUN_R14_CURRENT_GAMEPLAY.cmd"',
    'call "%~dp0RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd"',
    "-d3d11",
):
    require(start, needle, "START_HERE canonical route")

if 'RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd' in start:
    raise SystemExit("PASS22 VERIFY FAIL: internal Pass 21 acceptance helper leaked into START_HERE")
if 'RUN_R14_MAIN_SANDBOX_TEST.cmd' in start:
    raise SystemExit("PASS22 VERIFY FAIL: technical sandbox leaked back into START_HERE user menu")

# The actual normal and acceptance game processes must use the same safe renderer.
for text, label in ((normal, "normal game"), (recovery, "focused runtime acceptance")):
    require(text, "-d3d11", f"{label} safe renderer")
    require(text, "D3D12", f"{label} crash rationale")
    if "-d3d12" in text.lower() or "-dx12" in text.lower():
        raise SystemExit(f"PASS22 VERIFY FAIL: {label} still forces D3D12")

# Pass 21 still exists as an internal acceptance wrapper and chains the focused recovery launcher.
require(landmark, "RUN_R15_RUNTIME_RECOVERY_ACCEPTANCE.cmd", "Pass 21 chains focused recovery")
require(landmark, "PASS21_LANDMARK_OWNERSHIP_READY", "Pass 21 runtime evidence")

# Dirty local content should be visible rather than silently ignored during a test.
require(normal, "[LOCAL CHANGE]", "normal game local-change visibility")
require(recovery, "[LOCAL CHANGE]", "acceptance local-change visibility")

print("SINGLE LAUNCHER / D3D11 PASS 22 SOURCE CONTRACT PASS")
print("- START_HERE is the only user-facing entry point")
print("- normal game and user-facing full runtime acceptance use DirectX 11 after the reproduced D3D12RHI startup crash")
print("- Pass 21 remains internal and technical sandbox is not exposed in START_HERE")
print("- local working-tree changes are printed during runtime tests")
print("STATUS: SOURCE CONTRACT ONLY; local UE runtime must confirm that the D3D12RHI crash is bypassed")
