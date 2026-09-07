#!/usr/bin/env python3
from __future__ import annotations

import os
import queue
import subprocess
import sys
import threading
import time
from pathlib import Path

SCRIPT = Path(__file__).resolve()
PROJECT_DIR = SCRIPT.parents[1]
ROOT = PROJECT_DIR.parent
ORCHESTRATOR = SCRIPT.with_name("pass45_batch_runtime.py")
LOG_ROOT = ROOT / "Logs" / "PASS45_BATCH"
GAME_LOG = ROOT / "Logs" / "R14_CURRENT_GAMEPLAY.log"
MATERIAL_LOG = ROOT / "Logs" / "PASS45_STRICT_MATERIAL_GATE.log"

TOTAL_STEPS = 14
REFRESH_SECONDS = 1.0
QUIET_WARNING_SECONDS = 90.0
STALLED_ABORT_SECONDS = 300.0
MAX_LABEL = 52

STAGE_ACTIVITY_FILES: dict[str, tuple[Path, ...]] = {
    "Current-source OsterConflictEditor C++ build": (LOG_ROOT / "final_build.log",),
    "ALL local/Fab assets: prepare + import + runtime bindings": (LOG_ROOT / "all_local_assets.log",),
    "Stein weapon materials + fresh-load": (LOG_ROOT / "stein_materials.log",),
    "M700/Lever manual-action audio + fresh-load": (LOG_ROOT / "manual_action_audio.log",),
    "Remington 870 skeletal pump + fresh-load": (LOG_ROOT / "remington870.log",),
    "HMMWV + M2 + BTR-4 production intake": (LOG_ROOT / "production_vehicles.log",),
    "Every required weapon opens in fresh UE": (
        LOG_ROOT / "required_weapon_assets.log",
        LOG_ROOT / "required_weapon_assets_ue.log",
    ),
    "Strict authored material/dependency gate": (
        LOG_ROOT / "strict_material_gate.log",
        MATERIAL_LOG,
    ),
    "Gameplay runtime": (GAME_LOG,),
    "Gate K final-world visual truth": (LOG_ROOT / "post_gate_k.log",),
    "Interaction/material evidence": (LOG_ROOT / "post_runtime_evidence.log",),
    "M700 / Remington / Lever runtime evidence": (LOG_ROOT / "post_manual_action_runtime.log",),
    "Grenade throw animation runtime evidence": (LOG_ROOT / "post_grenade_throw_runtime.log",),
    "Flash grenade authored VFX runtime evidence": (LOG_ROOT / "post_flash_vfx_runtime.log",),
}


def _fmt(seconds: float) -> str:
    total = max(0, int(seconds))
    minutes, secs = divmod(total, 60)
    if minutes < 60:
        return f"{minutes:02d}:{secs:02d}"
    hours, minutes = divmod(minutes, 60)
    return f"{hours:02d}:{minutes:02d}:{secs:02d}"


def _pct(completed: int) -> int:
    if TOTAL_STEPS <= 0:
        return 100
    return min(100, max(0, int((completed * 100) / TOTAL_STEPS)))


def _short_label(label: str) -> str:
    label = " ".join(label.split())
    if len(label) <= MAX_LABEL:
        return label
    return label[: MAX_LABEL - 3] + "..."


def _reader(pipe, out: queue.Queue[str | None]) -> None:
    try:
        for raw in iter(pipe.readline, ""):
            out.put(raw.rstrip("\r\n"))
    finally:
        out.put(None)


def _activity_signature(paths: tuple[Path, ...]) -> tuple[tuple[str, int, int], ...]:
    values: list[tuple[str, int, int]] = []
    for path in paths:
        try:
            stat = path.stat()
            values.append((str(path), int(stat.st_size), int(stat.st_mtime_ns)))
        except OSError:
            values.append((str(path), -1, -1))
    return tuple(values)


def _terminate_process_tree(proc: subprocess.Popen[str]) -> None:
    if proc.poll() is not None:
        return
    if os.name == "nt":
        try:
            subprocess.run(
                ["taskkill", "/PID", str(proc.pid), "/T", "/F"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                check=False,
            )
        except Exception:
            pass
    if proc.poll() is None:
        try:
            proc.terminate()
            proc.wait(timeout=5)
        except Exception:
            try:
                proc.kill()
            except Exception:
                pass


def main() -> int:
    if not ORCHESTRATOR.is_file():
        print(f"[STOP] PASS45 orchestrator missing: {ORCHESTRATOR}", flush=True)
        return 91

    env = os.environ.copy()
    env["PYTHONIOENCODING"] = "utf-8"
    env["PYTHONUNBUFFERED"] = "1"

    command = [sys.executable, "-u", str(ORCHESTRATOR)]
    batch_started = time.monotonic()
    current_started = batch_started
    last_activity = batch_started
    completed = 0
    current_step = 0
    current_label = ""
    current_activity_paths: tuple[Path, ...] = ()
    current_activity_signature: tuple[tuple[str, int, int], ...] = ()
    runtime_active = False
    stream_closed = False
    last_status_width = 0
    last_refresh = 0.0
    forced_rc: int | None = None

    def clear_status() -> None:
        nonlocal last_status_width
        if last_status_width <= 0:
            return
        sys.stdout.write("\r" + (" " * last_status_width) + "\r")
        sys.stdout.flush()
        last_status_width = 0

    def refresh_activity(now: float) -> None:
        nonlocal current_activity_signature, last_activity
        if not current_activity_paths:
            return
        signature = _activity_signature(current_activity_paths)
        if signature != current_activity_signature:
            current_activity_signature = signature
            last_activity = now

    def draw_status(force: bool = False) -> None:
        nonlocal last_status_width, last_refresh
        if current_step <= completed or current_step <= 0:
            return
        now = time.monotonic()
        if not force and now - last_refresh < REFRESH_SECONDS:
            return
        refresh_activity(now)
        quiet_for = now - last_activity
        state = "ПРАЦЮЄ"
        if quiet_for >= QUIET_WARNING_SECONDS:
            state = f"НЕМАЄ НОВИХ ДАНИХ {_fmt(quiet_for)}"
        text = (
            f"[{_pct(completed):3d}%] {_short_label(current_label)} | "
            f"{_fmt(now - current_started)} | {state}"
        )
        padded = text.ljust(max(last_status_width, len(text)))
        sys.stdout.write("\r" + padded)
        sys.stdout.flush()
        last_status_width = len(padded)
        last_refresh = now

    def print_child(line: str) -> None:
        nonlocal last_activity
        last_activity = time.monotonic()
        clear_status()
        print(line, flush=True)
        draw_status(force=True)

    def mark_started(label: str) -> None:
        nonlocal current_step, current_label, current_started, runtime_active, last_refresh
        nonlocal current_activity_paths, current_activity_signature, last_activity
        current_step = min(TOTAL_STEPS, completed + 1)
        current_label = label
        current_started = time.monotonic()
        last_activity = current_started
        runtime_active = label == "Gameplay runtime"
        current_activity_paths = STAGE_ACTIVITY_FILES.get(label, ())
        current_activity_signature = _activity_signature(current_activity_paths)
        last_refresh = 0.0
        draw_status(force=True)

    def mark_completed() -> None:
        nonlocal completed, runtime_active
        if current_step <= 0 or completed >= current_step:
            return
        completed = current_step
        runtime_active = False
        clear_status()

    print("[INFO] Відсоток і таймер оновлюються В ОДНОМУ рядку. Нові рядки не дублюються.", flush=True)
    print("[INFO] Якщо stage-лог реально не змінюється 5 хвилин, preflight автоматично зупиняється замість безкінечного зависання.", flush=True)

    try:
        proc = subprocess.Popen(
            command,
            cwd=ROOT,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            stdin=None,
            text=True,
            encoding="utf-8",
            errors="replace",
            bufsize=1,
        )
    except Exception as exc:
        print(f"[STOP] Не вдалося запустити PASS45 orchestrator: {type(exc).__name__}: {exc}", flush=True)
        return 92

    assert proc.stdout is not None
    events: queue.Queue[str | None] = queue.Queue()
    threading.Thread(target=_reader, args=(proc.stdout, events), daemon=True).start()

    try:
        while True:
            try:
                item = events.get(timeout=0.25)
            except queue.Empty:
                item = "__NO_EVENT__"

            now = time.monotonic()

            if item is None:
                stream_closed = True
            elif item != "__NO_EVENT__":
                line = item
                last_activity = now

                if line.startswith("[RUN ] "):
                    mark_started(line[len("[RUN ] "):].strip())
                    continue

                if line.startswith("[RUNTIME] Current-source build"):
                    if current_step > completed:
                        mark_completed()
                    clear_status()
                    print(line, flush=True)
                    mark_started("Gameplay runtime")
                    continue

                if line.startswith("[POST]"):
                    if runtime_active and current_step > completed:
                        mark_completed()
                    clear_status()
                    print(line, flush=True)
                    continue

                if line.startswith("[PASS] ") or line.startswith("[FAIL] "):
                    if current_step > completed and current_label and current_label in line:
                        clear_status()
                        print(line, flush=True)
                        mark_completed()
                        continue

                    # A post verifier can fail before run() if its file is missing. Count that as one step.
                    if line.startswith("[FAIL] ") and current_step <= completed and completed < TOTAL_STEPS:
                        completed += 1
                    print_child(line)
                    continue

                # Keep the real orchestrator messages, but never leave a stale progress line above them.
                print_child(line)

            if proc.poll() is None:
                draw_status()
                now = time.monotonic()
                refresh_activity(now)
                stalled_for = now - last_activity
                if (
                    not runtime_active
                    and current_step > completed
                    and current_step > 0
                    and stalled_for >= STALLED_ABORT_SECONDS
                ):
                    clear_status()
                    print(
                        f"[STOP] {_short_label(current_label)} не дав жодного нового stdout/log evidence "
                        f"за {_fmt(stalled_for)}. Вважаю stage завислим і завершую його дерево процесів.",
                        flush=True,
                    )
                    forced_rc = 124
                    _terminate_process_tree(proc)
                    break

            if stream_closed and proc.poll() is not None:
                break

        if forced_rc is None:
            rc = int(proc.wait())
        else:
            try:
                proc.wait(timeout=5)
            except Exception:
                pass
            rc = forced_rc
    except KeyboardInterrupt:
        clear_status()
        print("[STOP] Тест перервано користувачем. Завершую все дерево дочірніх процесів...", flush=True)
        _terminate_process_tree(proc)
        return 130

    clear_status()
    total_elapsed = time.monotonic() - batch_started
    if completed >= TOTAL_STEPS:
        print(f"[100%] ГОТОВО | час {_fmt(total_elapsed)} | exit_code={rc}", flush=True)
    else:
        print(f"[{_pct(completed)}%] ЗУПИНЕНО | час {_fmt(total_elapsed)} | exit_code={rc}", flush=True)
    return rc


if __name__ == "__main__":
    raise SystemExit(main())
