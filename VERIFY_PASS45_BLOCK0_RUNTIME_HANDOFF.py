#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
LAUNCHER = ROOT / "RUN_PASS45_BLOCK0_RUNTIME_ACCEPTANCE.cmd"
PLAN = ROOT / "PASS45_BLOCK_EXECUTION_PLAN.md"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.name}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


launcher = read(LAUNCHER)
plan = read(PLAN)

for needle in (
    "fix/pass45-runtime-rejection-material-closure-20260826",
    "git fetch origin",
    'git rev-parse "origin/%CURRENT_BRANCH%"',
    'if /I not "%TESTED_HEAD%"=="%REMOTE_HEAD%"',
    "git status --porcelain --untracked-files=all",
    "git lfs pull origin",
    "VERIFY_PASS45_BLOCK0_GROUND_FOUNDATION.py",
    "VERIFY_FOLIAGE_RUNTIME_PASS_10.py",
    "complete Block0 ground/grass/tree handoff contracts",
    "Block0 foliage/tree source gate failed. UE runtime test cancelled.",
    "OsterConflictEditor Win64 Development",
    "/Game/Maps/OsterConflict_Runtime?Mode=Sandbox?SandboxAdminAll=1?Bots=0?Population=0?BotFill=0?AutoDeploy=1?LocationTest=1",
    "-game -NoFrontend",
    "-d3d11 -sm5 -nohdr",
    '-ExecCmds="t.MaxFPS 60"',
    "PASS45_BLOCK0_PRETICK_GROUND_FAIL",
    "PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_FAIL",
    "PASS10_FOLIAGE_RUNTIME_FAIL",
    "PASS45_REGIONAL_TREE_INTAKE_FAIL",
    "PASS45_BLOCK0_PRETICK_GROUND_READY",
    "geometry_postcondition=1",
    "collision_enabled=1",
    "PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_READY",
    "PASS10_FOLIAGE_RUNTIME_READY",
    "PASS45_REGIONAL_TREE_INTAKE_WIRED",
    "Museum / central-sector ground context",
    "Central park",
    "College / urban lawn context",
    "Ordinary roadside / private-sector context",
    "Long sightline showing grass-ground LOD transition",
    "if %SHOT_COUNT% LSS 5",
    "PENDING_VISUAL_REVIEW",
    "This script never self-declares RUNTIME ACCEPTED",
    'if /I not "%REMOTE_AFTER%"=="%TESTED_HEAD%"',
    "Evidence is intentionally NOT pushed against a different source head",
    "git commit -m \"evidence(pass45): capture Block0 UE58 runtime for %HEAD_SHORT%\"",
    'git push origin "%CURRENT_BRANCH%"',
):
    req(needle in launcher, f"Block0 runtime handoff contract missing: {needle}")

# The expensive local UE build must not begin until both source authorities pass. The ground verifier owns the
# authored plane/spatial acceptance wiring; Pass10 owns final-candidate surface exclusions, maintained-vs-rough
# foliage policy and the imported regional-tree acceptance contract.
req(
    launcher.find('"%SOURCE_VERIFY%"') < launcher.find('"%FOLIAGE_SOURCE_VERIFY%"') < launcher.find("[BUILD]"),
    "complete Block0 source preflight must run both ground and foliage verifiers before UE build",
)

# The isolated Block0 route must not drag locked content blocks into the acceptance path.
for forbidden in (
    "IMPORT_PRODUCTION_VEHICLES_UE58.cmd",
    "verify_required_weapon_assets.py",
    "PASS7_PRODUCTION_VEHICLES_READY",
    "PASS45_REQUIRED_AVAILABLE_WEAPONS_READY",
    "PASS45_BTR4",
    "PASS45_HMMWV",
    "PASS45_ADS_ALIGNMENT",
):
    req(forbidden not in launcher,
        f"Block0 runtime handoff illegally depends on locked later-block content: {forbidden}")

# Evidence collection can be automated, acceptance cannot. Prevent accidental self-freeze/merge behavior.
for forbidden in (
    "RuntimeAcceptance=ACCEPTED",
    "RUNTIME ACCEPTED / FROZEN",
    "git checkout main",
    "git merge",
    "gh pr merge",
):
    req(forbidden not in launcher,
        f"Block0 handoff can falsely promote or merge acceptance state: {forbidden}")

for needle in (
    "Only **one content block may be ACTIVE at a time**.",
    "Ground + grass foundation | **ACTIVE**",
    "Block 0 cannot close from CI alone.",
    "Direct UE 5.8 screenshots from at least:",
):
    req(needle in plan, f"execution plan lost Block0 runtime authority: {needle}")

if errors:
    print("PASS45 BLOCK0 RUNTIME HANDOFF: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 BLOCK0 RUNTIME HANDOFF: PASS")
print("- one isolated launcher verifies current remote head, hydrates LFS and runs both Block0 source authorities before build")
print("- ground/spatial contracts plus Pass10 foliage surface/zoning/tree-intake contracts must pass before UE 5.8 starts")
print("- direct Sandbox/LocationTest launch avoids weapon/vehicle/landmark acceptance dependencies")
print("- runtime FAIL/READY markers are enforced before evidence collection")
print("- at least five exact-session screenshots are collected and tied to the tested source head")
print("- evidence may auto-commit/push only if GitHub still equals the tested head")
print("- visual evidence remains PENDING_VISUAL_REVIEW; the launcher cannot self-declare runtime acceptance")
