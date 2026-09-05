#!/usr/bin/env python3
from __future__ import annotations

import os
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass, field
from datetime import datetime
from pathlib import Path

SCRIPT = Path(__file__).resolve()
PROJECT_DIR = SCRIPT.parents[1]
ROOT = PROJECT_DIR.parent
PROJECT = PROJECT_DIR / "OsterConflict.uproject"
LOG_ROOT = ROOT / "Logs" / "PASS45_BATCH"
REPORT = ROOT / "Logs" / "PASS45_BATCH_RUNTIME_REPORT.txt"
GAME_LOG = ROOT / "Logs" / "R14_CURRENT_GAMEPLAY.log"
MATERIAL_LOG = ROOT / "Logs" / "PASS45_STRICT_MATERIAL_GATE.log"
WEAPON_REPORT = PROJECT_DIR / "Saved" / "AutomationReports" / "ProductionModels" / "weapon_runtime_validation.txt"
WEAPON_SENTINEL = PROJECT_DIR / "Saved" / "AutomationReports" / "ProductionModels" / "required_weapon_asset_preflight_success.txt"

REQUIRED_RUNTIME_MARKERS = (
    "PASS29_MAIN_START_DIRECT_HOST_QUEUED",
    "PASS29_STATIC_FRONTEND_HOST_TRAVEL_EXECUTE",
    "PASS14_HOST_TRAVEL_BEGIN",
    "PASS14_FRONTEND_TRAVEL_HANDOFF_READY",
    "PASS44_LOCAL_BOT_AUTOFILL_DISABLED_READY",
    "PASS44_PRIMARY_WORLD_COMPACT_AUTHORING_READY",
    "PASS44_RUNTIME_GAMEPLAY_SEEDS_COMPACT_READY",
    "PASS44_BASE_ROLE_COORDINATE_INDEPENDENT_READY",
    "PASS44_COMBAT_VEHICLE_SEEDS_COMPACT_READY",
    "PASS44_COMPACT_PLAYABLE_AREA_READY",
    "PASS44_TACTICAL_MAP_COMPACT_BOUNDS_READY",
    "PASS44_ACTUAL_PAWN_MUSEUM_BASE_READY",
    "PASS45_BLOCK0_PRETICK_GROUND_READY",
    "PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_READY",
    "PASS45_REGIONAL_TREE_INTAKE_WIRED",
    "PASS45_LANDMARK_STARTUP_COORDINATED_READY",
    "PASS45_MUSEUM_R137_VISIBLE_OWNER_PRESERVED",
    "PASS45_MUSEUM_R138_COLLISION_ONLY_READY",
    "PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_READY",
    "PASS45_MUSEUM_LAYER_VALIDATION_READY",
    "PASS14_FOLIAGE_BUDGET_READY",
    "PASS30_MUSEUM_WINDOW_FRAME_CLEAN_READY",
    "PASS37_MUSEUM_VISIBLE_BASES_READY",
    "PASS42_BASE_RACK_GROUNDED_READY",
    "PASS45_REQUIRED_AVAILABLE_WEAPONS_READY",
    "PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY",
    "PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY",
    "PASS45_HMMWV_PROPORTIONAL_VISUAL_READY",
    "PASS45_BTR4_PROPORTIONAL_VISUAL_READY",
    "PASS45_M2_MOUNT_ALIGNMENT_READY",
    "PASS45_M2_GUNNER_PITCH_CONTRACT_READY",
    "PASS45_VEHICLE_ENTER_TRANSFORM_READY",
    "PASS45_VEHICLE_EXIT_TRANSFORM_READY",
    "PASS45_GUNNER_EXIT_TRANSFORM_READY",
    "PASS35_TACTICAL_PLAYER_MARKER_FOREGROUND",
    "PASS31_GAMEPLAY_INPUT_READY",
    "PASS41_INPUT_RECOVERY_POLL_BUDGET_READY",
    "PASS36_LOWCPU_FOLIAGE_SCOPE_READY",
    "PASS36_LOWCPU_FOLIAGE_RUNTIME_READY",
    "PASS36_WEAPON_MATERIAL_AUDIT_READY",
    "PASS38_WEAPON_FALLBACK_SCAN_STOPPED",
    "PASS39_GRAPHICS_QUALITY_PROFILE_READY",
    "PASS39_MINIMAP_UPDATE_BUDGET_READY",
    "PASS39_FP_LOCAL_PAWN_FAST_PATH_READY",
    "PASS39_PERF_SAMPLER_IDLE_READY",
    "PASS40_UI_STABILIZER_BUDGET_READY",
    "PASS40_DEPLOYMENT_PRESENTATION_BUDGET_READY",
    "PASS14_PERF_SAMPLE",
    "PASS14_PERF_30FPS_READY",
)

FAIL_PATTERNS = (
    "PASS7_PRODUCTION_VEHICLE_RUNTIME_FAIL",
    "PASS45_BLOCK0_PRETICK_GROUND_FAIL",
    "PASS45_BLOCK0_PRETICK_GROUND_CONTENT_GAP",
    "PASS45_BLOCK0_SPATIAL_GRASS_COVERAGE_FAIL",
    "PASS45_REGIONAL_TREE_INTAKE_FAIL",
    "PASS44_ACTUAL_PAWN_MUSEUM_BASE_FAIL",
    "PASS44_COMPACT_PLAYABLE_AREA_FAIL",
    "PASS37_BASE_DEPLOYMENT_RECOVERY_FAIL",
    "PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_FAIL",
    "PASS45_MUSEUM_R138_COLLISION_ONLY_FAIL",
    "PASS45_MUSEUM_LAYER_VALIDATION_FAIL",
    "PASS45_LANDMARK_SEPARATION_VALIDATION_FAIL",
    "PASS42_BASE_RACK_GROUNDING_INCOMPLETE",
    "PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL",
    "PASS44_WEAPON_RACK_AUTHORED_MATERIAL_GAP",
    "PASS45_PRODUCTION_VEHICLE_MATERIAL_OVERRIDE_FAIL",
    "PASS45_PRODUCTION_VEHICLE_MATERIAL_GAP",
    "PASS45_PRODUCTION_VEHICLE_CONTENT_GAP",
    "PASS45_VEHICLE_ENTER_TRANSFORM_FAIL",
    "PASS45_VEHICLE_EXIT_TRANSFORM_FAIL",
    "PASS45_GUNNER_EXIT_TRANSFORM_FAIL",
    "PASS38_WEAPON_FALLBACK_SCAN_BOUNDED_STOP",
    "PASS15_EMERGENCY_PERF_PROFILE_APPLIED",
    "PASS10_FOLIAGE_RUNTIME_FAIL",
    "PASS14_PERF_BELOW_TARGET",
)

INTERESTING = re.compile(
    r"(?:\[STOP\]|\[ERROR\]|Traceback|RuntimeError|PASS45_[A-Z0-9_]*(?:FAIL|GAP)|"
    r"PASS7_[A-Z0-9_]*FAIL|error(?:\s+code|\s*:)|failed)",
    re.IGNORECASE,
)


@dataclass
class Stage:
    name: str
    label: str
    command: list[str]
    log_path: Path
    rc: int | None = None
    issues: list[str] = field(default_factory=list)


def decode(path: Path) -> str:
    if not path.is_file():
        return ""
    raw = path.read_bytes()
    for encoding in ("utf-8", "utf-8-sig", "cp1251", "cp866"):
        try:
            return raw.decode(encoding)
        except UnicodeDecodeError:
            pass
    return raw.decode("utf-8", errors="replace")


def extract_issues(path: Path, limit: int = 16) -> list[str]:
    text = decode(path)
    found: list[str] = []
    for raw in text.splitlines():
        line = raw.strip()
        if line and INTERESTING.search(line) and line not in found:
            found.append(line)
        if len(found) >= limit:
            break
    if not found and text:
        found = [x.strip() for x in text.splitlines()[-8:] if x.strip()]
    return found[:limit]


def run(stage: Stage) -> Stage:
    stage.log_path.parent.mkdir(parents=True, exist_ok=True)
    print(f"[RUN ] {stage.label}")
    try:
        with stage.log_path.open("wb") as output:
            proc = subprocess.run(stage.command, cwd=ROOT, stdout=output, stderr=subprocess.STDOUT, check=False)
        stage.rc = int(proc.returncode)
    except Exception as exc:
        stage.rc = 126
        stage.log_path.write_text(f"{type(exc).__name__}: {exc}\n", encoding="utf-8")
    stage.issues = extract_issues(stage.log_path)
    print(f"[{'PASS' if stage.rc == 0 else 'FAIL'}] {stage.label} code={stage.rc}")
    return stage


def git(*args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["git", "-C", str(ROOT), *args], capture_output=True, text=True,
        encoding="utf-8", errors="replace", check=False
    )


def cmd_call(path: Path) -> list[str]:
    return ["cmd.exe", "/d", "/s", "/c", f'call "{path}"']


def resolve_engine() -> tuple[Path, Path, Path]:
    roots: list[Path] = []
    if os.environ.get("ProgramFiles"):
        roots.append(Path(os.environ["ProgramFiles"]) / "Epic Games" / "UE_5.8")
    roots.append(Path(r"C:\Program Files\Epic Games\UE_5.8"))
    for root in roots:
        editor = root / "Engine" / "Binaries" / "Win64" / "UnrealEditor.exe"
        editor_cmd = root / "Engine" / "Binaries" / "Win64" / "UnrealEditor-Cmd.exe"
        build = root / "Engine" / "Build" / "BatchFiles" / "Build.bat"
        if editor.is_file() and editor_cmd.is_file() and build.is_file():
            return editor, editor_cmd, build
    raise RuntimeError("UE 5.8 editor/commandlet/build tools not found")


def tracked_changes() -> list[str]:
    cp = git("status", "--porcelain", "--untracked-files=no")
    return [line for line in cp.stdout.splitlines() if line.strip()]


def report(branch: str, head_before: str, remote_head: str, preflight: list[Stage], runtime_rc: int | None,
           missing: list[str], runtime_failures: list[str], post: list[Stage], dirty_before: list[str],
           dirty_after: list[str], formal_blockers: list[str]) -> None:
    lines = [
        "OSTER CONFLICT - PASS45 BATCH RUNTIME REPORT",
        f"generated={datetime.now().isoformat(timespec='seconds')}",
        f"branch={branch}", f"head_before={head_before}", f"remote_head={remote_head}",
        f"head_after={git('rev-parse', 'HEAD').stdout.strip()}", "", "PREFLIGHT",
    ]
    for stage in preflight:
        lines.append(f"{stage.name}: {'PASS' if stage.rc == 0 else 'FAIL'} code={stage.rc} log={stage.log_path}")
        lines.extend(f"  > {item}" for item in stage.issues)
    lines += ["", f"preflight_failure_count={sum(stage.rc != 0 for stage in preflight)}", ""]
    if runtime_rc is None:
        lines.append("RUNTIME: NOT STARTED - preflight blockers exist")
    else:
        lines += [f"RUNTIME: exit_code={runtime_rc} log={GAME_LOG}", f"runtime_missing_marker_count={len(missing)}"]
        lines.extend(f"  MISSING {item}" for item in missing)
        lines.append(f"runtime_fail_line_count={len(runtime_failures)}")
        lines.extend(f"  FAIL {item}" for item in runtime_failures)
    lines += ["", "POST-RUN VERIFIERS"]
    if not post:
        lines.append("  NOT RUN")
    for stage in post:
        lines.append(f"{stage.name}: {'PASS' if stage.rc == 0 else 'FAIL'} code={stage.rc} log={stage.log_path}")
        lines.extend(f"  > {item}" for item in stage.issues)
    lines += ["", "TRACKED CHANGES BEFORE"]
    lines.extend(f"  {item}" for item in dirty_before or ["NONE"])
    lines += ["", "TRACKED CHANGES AFTER"]
    lines.extend(f"  {item}" for item in dirty_after or ["NONE"])
    lines += ["", "FORMAL ACCEPTANCE BLOCKERS"]
    lines.extend(f"  {item}" for item in formal_blockers or ["NONE"])
    if any(stage.rc != 0 for stage in preflight):
        status = "PREFLIGHT_FAIL"
    elif runtime_rc not in (None, 0) or missing or runtime_failures or any(stage.rc != 0 for stage in post):
        status = "RUNTIME_OR_POSTCHECK_FAIL"
    elif formal_blockers:
        status = "DIAGNOSTIC_PASS_FORMAL_ACCEPTANCE_BLOCKED"
    else:
        status = "AUTOMATED_PASS_VISUAL_ACCEPTANCE_PENDING"
    lines += ["", f"STATUS={status}"]
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    os.system("chcp 65001 >nul 2>nul")
    LOG_ROOT.mkdir(parents=True, exist_ok=True)
    print("=" * 68)
    print("OSTER CONFLICT - PASS45 ПАКЕТНИЙ RUNTIME-ТЕСТ")
    print("=" * 68)
    print("Один запуск збирає ВСІ проблеми. На першій помилці більше не зупиняємось.")
    print("Локальні Changes не видаляються, не stash і не reset.")
    print()

    if not PROJECT.is_file() or shutil.which("git") is None:
        print("[STOP] Project or Git is missing.")
        return 2

    branch = git("branch", "--show-current").stdout.strip()
    head_before = git("rev-parse", "HEAD").stdout.strip()
    fetch = git("fetch", "origin", branch)
    remote_head = git("rev-parse", f"origin/{branch}").stdout.strip()
    if not branch or not head_before or fetch.returncode != 0 or not remote_head:
        print("[STOP] Не вдалося визначити/оновити current branch.")
        return 3
    if head_before.lower() != remote_head.lower():
        print("[STOP] Локальна гілка відстає від GitHub. Fetch origin -> Pull origin.")
        print(f"Local : {head_before}\nGitHub: {remote_head}")
        return 4

    dirty_before = tracked_changes()
    formal_blockers: list[str] = []
    if dirty_before:
        formal_blockers.append(f"tracked_changes_before={len(dirty_before)}")
        print(f"[WARN] Tracked Changes: {len(dirty_before)}. Не чіпаю їх; formal acceptance буде BLOCKED.")
    else:
        print("[PASS] Exact tracked worktree matches current HEAD.")

    try:
        editor, editor_cmd, build_bat = resolve_engine()
    except Exception as exc:
        print(f"[STOP] {exc}")
        return 5

    wrapper_specs = (
        ("all_local_assets", "ALL local/Fab assets: prepare + import + runtime bindings", PROJECT_DIR / "IMPORT_ALL_LOCAL_INBOX_UE58.cmd"),
        ("stein_materials", "Stein weapon materials + fresh-load", PROJECT_DIR / "PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd"),
        ("manual_action_audio", "M700/Lever manual-action audio + fresh-load", PROJECT_DIR / "PASS45_IMPORT_MANUAL_ACTION_AUDIO_UE58.cmd"),
        ("remington870", "Remington 870 skeletal pump + fresh-load", PROJECT_DIR / "PASS45_IMPORT_REMINGTON870_PRODUCTION_UE58.cmd"),
        ("production_vehicles", "HMMWV + M2 + BTR-4 production intake", PROJECT_DIR / "IMPORT_PRODUCTION_VEHICLES_UE58.cmd"),
    )
    preflight: list[Stage] = []
    for name, label, path in wrapper_specs:
        command = cmd_call(path) if path.is_file() else ["cmd.exe", "/d", "/c", "exit", "127"]
        preflight.append(Stage(name, label, command, LOG_ROOT / f"{name}.log"))

    build_cmd = ["cmd.exe", "/d", "/s", "/c", f'call "{build_bat}" OsterConflictEditor Win64 Development -Project="{PROJECT}" -WaitMutex']
    preflight.append(Stage("final_build", "Final OsterConflictEditor C++ build", build_cmd, LOG_ROOT / "final_build.log"))

    try:
        WEAPON_SENTINEL.unlink(missing_ok=True)
    except OSError:
        pass
    weapon_verify = PROJECT_DIR / "Scripts" / "verify_required_weapon_assets.py"
    weapon_cmd = [str(editor_cmd), str(PROJECT), "-run=pythonscript", f"-script={weapon_verify}", "-unattended", "-nop4", "-nosplash", "-nullrhi", "-stdout", "-FullStdOutLogOutput", "-UTF8Output", f"-abslog={LOG_ROOT / 'required_weapon_assets_ue.log'}"]
    preflight.append(Stage("required_weapon_assets", "Every required weapon opens in fresh UE", weapon_cmd, LOG_ROOT / "required_weapon_assets.log"))

    material_gate = PROJECT_DIR / "RUN_PASS45_STRICT_MATERIAL_GATE.cmd"
    preflight.append(Stage("strict_material_gate", "Strict authored material/dependency gate", cmd_call(material_gate) if material_gate.is_file() else ["cmd.exe", "/d", "/c", "exit", "127"], LOG_ROOT / "strict_material_gate.log"))

    print("\n[PREFLIGHT] Проганяю ВСІ незалежні етапи...")
    for stage in preflight:
        run(stage)
        if stage.name == "required_weapon_assets" and stage.rc == 0 and not WEAPON_SENTINEL.is_file():
            stage.rc = 91
            stage.issues.append("required weapon success sentinel missing")
            print("[FAIL] Required weapon sentinel missing")

    if git("rev-parse", "HEAD").stdout.strip().lower() != head_before.lower():
        formal_blockers.append("head_changed_during_preflight=1")
    dirty_mid = tracked_changes()
    if dirty_mid != dirty_before:
        formal_blockers.append("tracked_worktree_changed_during_preflight=1")

    blockers = [stage for stage in preflight if stage.rc != 0]
    if blockers:
        report(branch, head_before, remote_head, preflight, None, [], [], [], dirty_before, dirty_mid, formal_blockers)
        print("\n" + "=" * 68)
        print(f"[STOP] Знайдено {len(blockers)} preflight проблем. Гру поки не запускаю.")
        for stage in blockers:
            print(f" - {stage.label}: code={stage.rc}")
            for item in stage.issues[:3]:
                print(f"   > {item}")
        print(f"ЄДИНИЙ ЗВІТ: {REPORT}")
        return 20

    print("\n[RUNTIME] Preflight чистий. Запускаю ОДИН gameplay runtime.")
    print("START -> TEAM/SQUAD/ROLE -> BASE -> У БІЙ. F10 -> Spawn all weapons.")
    print("Перевір HMMWV/BTR/M2, зброю, побудь у gameplay >=20 сек і нормально вийди.")
    try:
        GAME_LOG.unlink(missing_ok=True)
    except OSError:
        pass
    runtime_cmd = [str(editor), str(PROJECT), "/Game/Maps/OsterConflict_Runtime", "-game", "-Frontend", "-d3d11", "-sm5", "-nohdr", "-NoScreenMessages", "-log", f"-abslog={GAME_LOG}", "-fullscreen", "-ResX=1600", "-ResY=900", "-ExecCmds=t.MaxFPS 60", "-culture=uk-UA"]
    runtime_rc = int(subprocess.run(runtime_cmd, cwd=ROOT, check=False).returncode)
    game_text = decode(GAME_LOG)
    missing = [marker for marker in REQUIRED_RUNTIME_MARKERS if marker not in game_text]
    if "PASS45_INITIAL_BASE_DEPLOYMENT_" not in game_text:
        missing.append("PASS45_INITIAL_BASE_DEPLOYMENT_*")
    runtime_failures: list[str] = []
    for raw in game_text.splitlines():
        line = raw.strip()
        if not line:
            continue
        if any(pattern in line for pattern in FAIL_PATTERNS) or re.search(r"\bPASS(?:\d+|45)_[A-Z0-9_]*_FAIL\b", line):
            if line not in runtime_failures:
                runtime_failures.append(line)
    input_lines = "\n".join(line for line in game_text.splitlines() if "PASS31_GAMEPLAY_INPUT_READY" in line)
    if input_lines and "moveIgnored=0 lookIgnored=0" not in input_lines:
        runtime_failures.append("PASS31_GAMEPLAY_INPUT_READY did not prove moveIgnored=0 lookIgnored=0")

    post_specs = (
        ("gate_k", "Gate K final-world visual truth", [sys.executable, str(ROOT / "VERIFY_PASS45_GATE_K_RUNTIME_LOG.py"), str(GAME_LOG)]),
        ("runtime_evidence", "Interaction/material evidence", [sys.executable, str(ROOT / "VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py"), str(GAME_LOG), str(MATERIAL_LOG), str(WEAPON_REPORT)]),
        ("manual_action_runtime", "M700 / Remington / Lever runtime evidence", [sys.executable, str(ROOT / "VERIFY_PASS45_MANUAL_ACTION_RUNTIME.py"), str(GAME_LOG)]),
        ("grenade_throw_runtime", "Grenade throw animation runtime evidence", [sys.executable, str(ROOT / "VERIFY_PASS45_GRENADE_THROW_ANIMATION_RUNTIME.py"), str(GAME_LOG)]),
        ("flash_vfx_runtime", "Flash grenade authored VFX runtime evidence", [sys.executable, str(ROOT / "VERIFY_PASS45_GRENADE_FLASH_RUNTIME.py"), str(GAME_LOG)]),
    )
    post: list[Stage] = []
    print("\n[POST] Проганяю ВСІ post-runtime verifier-и...")
    for name, label, command in post_specs:
        stage = Stage(name, label, command, LOG_ROOT / f"post_{name}.log")
        if not Path(command[1]).is_file():
            stage.rc = 127
            stage.log_path.write_text(f"missing verifier: {command[1]}\n", encoding="utf-8")
            stage.issues = [f"missing verifier: {command[1]}"]
            print(f"[FAIL] {label} code=127")
        else:
            run(stage)
        post.append(stage)

    dirty_after = tracked_changes()
    head_after = git("rev-parse", "HEAD").stdout.strip()
    if head_after.lower() != head_before.lower():
        formal_blockers.append("head_changed_during_runtime=1")
    if dirty_after:
        formal_blockers.append(f"tracked_changes_after={len(dirty_after)}")
    if dirty_after != dirty_before:
        formal_blockers.append("tracked_worktree_changed_during_test=1")

    report(branch, head_before, remote_head, preflight, runtime_rc, missing, runtime_failures, post, dirty_before, dirty_after, formal_blockers)
    post_failures = [stage for stage in post if stage.rc != 0]
    failed = runtime_rc != 0 or bool(missing or runtime_failures or post_failures)

    print("\n" + "=" * 68)
    print("PASS45 ПАКЕТНИЙ РЕЗУЛЬТАТ")
    print(f"Runtime exit: {runtime_rc}")
    print(f"Missing READY: {len(missing)}")
    print(f"Runtime FAIL: {len(runtime_failures)}")
    print(f"Post verifier FAIL: {len(post_failures)}")
    print(f"Formal blockers: {len(formal_blockers)}")
    print(f"ЄДИНИЙ ЗВІТ: {REPORT}")
    if failed:
        print("[FAIL] Усі знайдені проблеми вже зібрані одним списком у звіті.")
        return 30
    if formal_blockers:
        print("[DIAGNOSTIC PASS] Runtime чистий, formal acceptance заблокований tracked Changes.")
        return 31
    print("[PASS] Автоматичні gates пройдено. Лишається пряма візуальна перевірка.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
