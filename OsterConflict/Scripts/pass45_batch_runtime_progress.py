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

TOTAL_STEPS = 14
REFRESH_SECONDS = 1.0
QUIET_WARNING_SECONDS = 90.0
MAX_LABEL = 52


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
    last_child_output = batch_started
    completed = 0
    current_step = 0
    current_label = ""
    runtime_active = False
    stream_closed = False
    last_status_width = 0
    last_refresh = 0.0

    def clear_status() -> None:
        nonlocal last_status_width
        if last_status_width <= 0:
            return
        sys.stdout.write("\r" + (" " * last_status_width) + "\r")
        sys.stdout.flush()
        last_status_width = 0

    def draw_status(force: bool = False) -> None:
        nonlocal last_status_width, last_refresh
        if current_step <= completed or current_step <= 0:
            return
        now = time.monotonic()
        if not force and now - last_refresh < REFRESH_SECONDS:
            return
        quiet_for = now - last_child_output
        state = "ПРАЦЮЄ"
        if quiet_for >= QUIET_WARNING_SECONDS:
            state = f"ПРОЦЕС ЖИВИЙ, тиша {_fmt(quiet_for)}"
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
        clear_status()
        print(line, flush=True)
        draw_status(force=True)

    def mark_started(label: str) -> None:
        nonlocal current_step, current_label, current_started, runtime_active, last_refresh
        current_step = min(TOTAL_STEPS, completed + 1)
        current_label = label
        current_started = time.monotonic()
        runtime_active = label == "Gameplay runtime"
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
                last_child_output = now

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

            if stream_closed and proc.poll() is not None:
                break

        rc = int(proc.wait())
    except KeyboardInterrupt:
        clear_status()
        print("[STOP] Тест перервано користувачем. Завершую дочірній процес...", flush=True)
        try:
            proc.terminate()
            proc.wait(timeout=8)
        except Exception:
            try:
                proc.kill()
            except Exception:
                pass
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
