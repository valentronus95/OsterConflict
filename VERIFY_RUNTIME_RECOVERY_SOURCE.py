#!/usr/bin/env python3
"""Structural guard for current GAME_RECOVERY/PASS45 source ownership.

This proves source contracts only. It must never be reported as an Unreal Engine build,
playtest, visual acceptance or runtime acceptance.
"""

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent

# GitHub Windows runners can inherit a legacy console encoding. Keep verifier diagnostics deterministic instead of
# crashing while printing a Unicode path/marker, which is a particularly ridiculous way for a source guard to fail.
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")

CHECKS = {
    "current authority": (
        ROOT / "AGENTS.md",
        [
            "latest explicit user requirement and latest user-observed runtime evidence",
            "PR #94 was merged on 2026-09-07",
            "current GAME_RECOVERY/PASS45 continuation work proceeds on `main`",
            "No historical verifier may require a runtime-rejected owner/fallback back into production",
            "stale verifiers are updated/demoted/deleted",
            "Normal local/listen gameplay may fill empty match population with bots",
        ],
    ),
    "GAME_RECOVERY acceptance": (
        ROOT / "GAME_RECOVERY.md",
        [
            "Definition of Done",
            "UE 5.8 runtime",
            "не повертати старі proxy/заглушки як production-рішення",
        ],
    ),
    "deployment deferred possession": (
        ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13DeploymentFlowSubsystem.cpp",
        [
            "UIReadyDeployKeepOpenUntilSpawn",
            "PASS45_DEPLOY_REQUEST_EXECUTE",
            "PASS45_DEPLOY_DIRECT_READY",
            "slate_callback=0",
            "possession_confirmed=1",
        ],
    ),
    "filler-bot policy": (
        ROOT / "OsterConflict/Source/OsterConflict/Private/OCBotPopulationPolicySubsystem.cpp",
        [
            "IsFrontendOnlySession",
            "IsSandboxMode",
            "NM_DedicatedServer",
            "GetHumanPlayerCount() <= 0",
            "RestoreExpectedLocalBotFill",
            "GAME_RECOVERY_BOT_FILL_RESTORED",
        ],
    ),
    "generic weapon fallback retirement": (
        ROOT / "OsterConflict/Source/OsterConflict/Private/OCRealWeaponFallbackSubsystem.cpp",
        [
            "PASS45_GENERIC_WEAPON_FALLBACK_RETIRED",
            "generic_substitution=0",
            "exact_visual_owner=imported_bridge",
            "primitive_cleanup=1",
            "PASS45_PRIMITIVE_WEAPON_VISUAL_RETIRED",
            "PASS45_VISIBLE_PRIMITIVE_WEAPON_FAIL",
            "PASS45_PRIMITIVE_WEAPON_RUNTIME_READY",
            "Wrong-identity replacement is intentionally forbidden.",
        ],
    ),
    "vehicle exit input recovery": (
        ROOT / "OsterConflict/Source/OsterConflict/Private/OCVehicleExitInputRecoverySubsystem.cpp",
        ["ClearAllMappings", "ResetIgnoreMoveInput", "ResetIgnoreLookInput", "FInputModeGameOnly", "UIApplyLocalPreferences"],
    ),
    "tactical map contract": (
        ROOT / "OsterConflict/Source/OsterConflict/Private/OCTacticalMapSubsystem.cpp",
        [
            "EKeys::M",
            "EKeys::V",
            "UnmapKey",
            "M reserved for map; DeployTrap moved to V for this pawn.",
            "FInputModeGameOnly",
        ],
    ),
}

failures: list[str] = []

for label, (path, needles) in CHECKS.items():
    if not path.is_file():
        failures.append(f"{label}: missing {path.relative_to(ROOT)}")
        continue
    text = path.read_text(encoding="utf-8", errors="replace")
    for needle in needles:
        if needle not in text:
            failures.append(f"{label}: missing marker {needle!r} in {path.relative_to(ROOT)}")

# Current source guard must also keep rejected production shortcuts physically absent.
weapon_fallback = ROOT / "OsterConflict/Source/OsterConflict/Private/OCRealWeaponFallbackSubsystem.cpp"
if weapon_fallback.is_file():
    text = weapon_fallback.read_text(encoding="utf-8", errors="replace")
    for forbidden in (
        "/Game/AK-47/Mesh/SM_AK-47.SM_AK-47",
        "R13 real SMG temporary MP5 fallback",
        "GAME_RECOVERY_REAL_WEAPON_FALLBACK_PRELOAD_BEGIN",
        "PASS45_REAL_WEAPON_FALLBACK_READY",
        "LoadObject<",
    ):
        if forbidden in text:
            failures.append(f"generic weapon fallback retirement: forbidden stale behavior {forbidden!r}")

# A source verifier must never accidentally bless runtime status. Keep the active runtime-sensitive ledger rows open
# if those rows still exist; source/CI success is not permission to mark them VERIFIED/DONE.
ledger_path = ROOT / "OSTER_CONFLICT_WORK_LEDGER.md"
if ledger_path.is_file():
    ledger = ledger_path.read_text(encoding="utf-8", errors="replace")
    for required_open_id in (
        "GAME-WEAPONS-001",
        "VIS-FP-001",
        "ASSET-BTR-001",
        "ASSET-CHARACTER-001",
        "GAME-VEHICLE-INPUT-001",
        "UI-TACTICAL-MAP-001",
    ):
        matching = [line for line in ledger.splitlines() if f"| {required_open_id} |" in line]
        if matching and ("| VERIFIED |" in matching[0] or "| DONE |" in matching[0]):
            failures.append(f"ledger: {required_open_id} is incorrectly runtime-closed")

if failures:
    print("RUNTIME RECOVERY SOURCE GUARD: FAIL")
    for failure in failures:
        print(f" - {failure}")
    print("This is a SOURCE-ONLY failure. It is not an UE runtime result.")
    sys.exit(1)

print("RUNTIME RECOVERY SOURCE GUARD: PASS")
print("Current main authority, deferred deployment, filler bots, primitive retirement and input/tactical recovery contracts are present.")
print("SOURCE-ONLY PASS. UE 5.8 build/playtest is still required before runtime acceptance.")
