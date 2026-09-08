#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS38 VERIFY FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


def require(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise SystemExit(f"PASS38 VERIFY FAIL: {label}: missing {needle!r}")


def forbid(text: str, needle: str, label: str) -> None:
    if needle in text:
        raise SystemExit(f"PASS38 VERIFY FAIL: {label}: forbidden {needle!r}")


def absent(path: Path, label: str) -> None:
    if path.exists():
        raise SystemExit(f"PASS38 VERIFY FAIL: stale {label} resurrected: {path.relative_to(ROOT)}")


for path, label in (
    (SRC / "Public" / "OCMuseumVisibilityPass37Subsystem.h", "Museum visibility/rebuild guard"),
    (SRC / "Private" / "OCMuseumVisibilityPass37Subsystem.cpp", "Museum visibility/rebuild guard"),
    (SRC / "Public" / "OCMuseumCoreRecoverySubsystem.h", "Museum core recovery owner"),
    (SRC / "Private" / "OCMuseumCoreRecoverySubsystem.cpp", "Museum core recovery owner"),
    (SRC / "Public" / "OCWeaponPalettePass37Subsystem.h", "weapon palette compatibility owner"),
    (SRC / "Private" / "OCWeaponPalettePass37Subsystem.cpp", "weapon palette compatibility owner"),
):
    absent(path, label)

fallback_h = read(SRC / "Public" / "OCRealWeaponFallbackSubsystem.h")
fallback = read(SRC / "Private" / "OCRealWeaponFallbackSubsystem.cpp")
game_h = read(SRC / "Public" / "OCGameMode.h")
runtime_safe = read(SRC / "Private" / "OCGameModeRuntimeSafe.cpp")
bot_policy = read(SRC / "Private" / "OCBotPopulationPolicySubsystem.cpp")
startup = read(SRC / "Private" / "OCLandmarkStartupCoordinatorSubsystem.cpp")
fp_h = read(SRC / "Public" / "OCFirstPersonWeaponPresentationSubsystem.h")
fp = read(SRC / "Private" / "OCFirstPersonWeaponPresentationSubsystem.cpp")
weapon_anim_h = read(SRC / "Public" / "OCWeaponAnimationProfiles.h")
weapon_anim = read(SRC / "Private" / "OCWeaponAnimationProfiles.cpp")
perf_h = read(SRC / "Public" / "OCPerformanceSampleSubsystem.h")
perf = read(SRC / "Private" / "OCPerformanceSampleSubsystem.cpp")
launcher = read(ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd")
acceptance = read(ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd")
evidence = read(ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py")

for needle in (
    "GAME_RECOVERY_WORLD_PREP_BEGIN",
    "pre_spawn=1 tick_when_paused=1 staged_materialization=1",
    "timing_probe=1",
    "GAME_RECOVERY_WORLD_PREP_TIMERS_CANCELLED",
    "duplicate_startup_timers=0",
    "GAME_RECOVERY_WORLD_PREP_STAGE_TIMING",
    "GAME_RECOVERY_WORLD_READY",
    "pre_spawn=1",
    "post_spawn_landmark_materialization=0",
    "total_prep_ms=",
    "slowest_stage_ms=",
):
    require(startup, needle, "staged landmark startup")

# Weapon audit is finite, truth-only and never substitutes a nearby look-alike mesh for exact identity.
for needle in (
    "int32 RefreshPassCount = 0",
    "MaxRefreshPasses = 12",
    "ClearTimer(RefreshTimer)",
    "PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP",
    "PASS36_WEAPON_MATERIAL_AUDIT_READY",
    "PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP",
    "PASS45_GENERIC_WEAPON_FALLBACK_RETIRED",
    "permanent_scan=0",
    "generic_substitution=0",
):
    require(fallback_h + fallback, needle, "bounded truth-only exact-identity weapon scan")
for forbidden in (
    "UMaterialInstanceDynamic::Create",
    "Component->SetMaterial(Slot",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED",
    "reason=material_gap_audited",
    "/Game/AK-47/Mesh/SM_AK-47.SM_AK-47",
    "R13 real SMG temporary MP5 fallback",
):
    forbid(fallback, forbidden, "retired weapon repair/substitution")

# First-person hands/ADS/weapon action presentation must never discover animation packages with a blocking
# LoadObject during world begin play or first shot/reload/manual action. Profile data owns the authored paths;
# the presentation subsystem preloads them asynchronously and first-use code resolves resident objects only.
for needle in (
    "OCAppendWeaponAnimationAssetPaths",
    "FSoftObjectPath",
):
    require(weapon_anim_h + weapon_anim, needle, "profile-owned animation preload paths")
for needle in (
    "PresentationAnimationPreloadHandle",
    "RequestPresentationAnimationPreload",
    "CompletePresentationAnimationPreload",
    "RequestAsyncLoad(",
    "FStreamableManager::AsyncLoadHighPriority",
    "ResolveResidentAnimation",
    "ResolveObject()",
    "PASS45_FP_ANIMATION_ASYNC_PRELOAD_BEGIN",
    "PASS45_FP_ANIMATION_ASYNC_PRELOAD_READY",
    "sync_load=0",
    "first_use_sync_load=0",
):
    require(fp_h + fp, needle, "non-blocking first-person animation presentation")
forbid(fp, "LoadObject<", "first-person presentation may not synchronously load animation packages")

# Normal local/listen gameplay owns filler bots again, but frontend/travel stays light until the human host exists.
for needle in (
    "int32 TargetPopulation = 16",
    "bool bAutoFillBots = true",
    "RestoreExpectedLocalBotFill",
    "Humans replace bots first",
):
    require(game_h, needle, "approved filler-bot defaults and human priority")
for needle in (
    "PASS44_LOCAL_BOT_AUTOFILL_DEFERRED_READY",
    "background_ai_load_before_human=0",
    "restore_owner=OCBotPopulationPolicySubsystem",
):
    require(runtime_safe, needle, "light frontend/travel staging")
for needle in (
    "IsFrontendOnlySession()",
    "IsSandboxMode()",
    "NM_DedicatedServer",
    "GetHumanPlayerCount() <= 0",
    "RestoreExpectedLocalBotFill()",
    "GAME_RECOVERY_BOT_FILL_RESTORED",
):
    require(bot_policy, needle, "post-host filler-bot restoration")

# Visual/content world preparation remains acceptance evidence, not permission to spawn a human pawn.
for needle in (
    "GAME_RECOVERY_SPAWN_GATE_VISUAL_FAIL_SOFT",
    "gameplay_spawn_release=1",
    "acceptance_preserved=1",
    "fail_closed=0",
    "visual_gate_blocks_spawn=0",
):
    require(runtime_safe, needle, "fail-soft human spawn gate")
forbid(runtime_safe, "player_spawned=0 fail_closed=1", "visual world-prep failure may not strand deployment")

for needle in (
    "bRecoveryRuntimeContractLogged",
    "ValidatePass45RecoveryRuntimeContract",
):
    require(perf_h, needle, "thermal/display sampler state")
for needle in (
    '#include "HAL/IConsoleManager.h"',
    'FindConsoleVariable(TEXT("t.MaxFPS"))',
    'MaxFpsVariable->GetFloat()',
    'FMath::IsNearlyEqual(RuntimeMaxFps, 60.0f, 0.5f)',
    'PASS45_THERMAL_CAP_RUNTIME_READY',
    'PASS45_THERMAL_CAP_RUNTIME_FAIL',
    'quality_mutation=0 render_scale_mutation=0',
):
    require(perf, needle, "actual runtime thermal cap evidence")

launcher_parts = launcher.split(":quick_normal_game", 1)
if len(launcher_parts) != 2:
    raise SystemExit("PASS38 VERIFY FAIL: canonical launcher is missing explicit quick-normal section")
strict_launcher, quick_launcher = launcher_parts
require(launcher, 'set "QUALITY_CMDS=t.MaxFPS 60,', "shared 60 FPS quality command")
for needle in ('-fullscreen', '-ExecCmds="%QUALITY_CMDS%"'):
    require(strict_launcher, needle, "strict recovery launcher request")
forbid(strict_launcher, '-windowed', "strict recovery route must not force windowed mode")
require(quick_launcher, '-windowed', "quick normal route must remain desktop-recoverable")
require(quick_launcher, '-ExecCmds="%QUALITY_CMDS%"', "quick normal route must reuse shared 60 FPS quality commands")

for needle in (
    'require(gameplay, "PASS45_THERMAL_CAP_RUNTIME_READY"',
    'forbid(gameplay, "PASS45_THERMAL_CAP_RUNTIME_FAIL"',
    '"THERMAL_CAP_RUNTIME_CONTRACT=PASS\\n"',
):
    require(evidence, needle, "strict thermal runtime evidence")

for marker in (
    "PASS38_MUSEUM_REBUILD_BUDGET_READY",
    "PASS38_MUSEUM_REBUILD_BUDGET_FAIL",
    "PASS38_WEAPON_PALETTE_SCAN_STOPPED",
    "PASS44_WEAPON_PALETTE_MUTATION_DISABLED",
    "PASS37_MUSEUM_VISIBLE_CORE_READY",
    "PASS29_MAIN_START_DIRECT_HOST_QUEUED",
    "PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED",
    "PASS44_LOCAL_BOT_AUTOFILL_DISABLED_READY",
):
    forbid(acceptance, marker, f"stale acceptance marker {marker}")
for marker in (
    "GAME_RECOVERY_WORLD_READY",
    "PASS45_SECONDARY_MENU_HOST_SETUP_QUEUED",
    "PASS14_MAIN_START_OPENS_SERVER_SETUP",
    "PASS45_SECONDARY_MENU_HOST_TRAVEL_EXECUTE",
    "GAME_RECOVERY_BOT_FILL_RESTORED",
    "PASS36_WEAPON_MATERIAL_AUDIT_READY",
    "PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP",
    "PASS14_PERF_30FPS_READY",
):
    require(acceptance, marker, f"current runtime acceptance marker {marker}")

print("RUNTIME RUNAWAY / HEAT PASS 38/45 FORWARD-PORTED SOURCE CONTRACT PASS")
print("- destructive Museum recovery and obsolete palette owner remain physically deleted")
print("- landmark startup is staged and remains factual acceptance evidence")
print("- weapon material audit is finite; generic look-alike weapon substitution stays retired")
print("- first-person/ADS/weapon-action animations are asynchronously preloaded; first use performs resident-only resolution")
print("- local/listen filler bots restore only after a human host exists; frontend/Sandbox/dedicated paths remain isolated")
print("- visual world-preparation failures remain logged but cannot strand the human without a pawn")
print("- strict recovery remains fullscreen; quick normal is windowed and both reuse the 60 FPS quality command")
print("- low-FPS/thermal recovery never lowers render scale to disguise the problem")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime remains authoritative")
