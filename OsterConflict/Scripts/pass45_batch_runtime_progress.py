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
HEARTBEAT_SECONDS = 10.0
QUIET_WARNING_SECONDS = 90.0


def _fmt(seconds: float) -> str:
    total = max(0, int(seconds))
    hours, rem = divmod(total, 3600)
    minutes, secs = divmod(rem, 60)
    return f"{hours:02d}:{minutes:02d}:{secs:02d}"


def _pct(completed: int) -> int:
    if TOTAL_STEPS <= 0:
        return 100
    return min(100, max(0, int((completed * 100) / TOTAL_STEPS)))


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
    last_heartbeat = batch_started
    last_quiet_warning = 0.0
    completed = 0
    current_step = 0
    current_label = "initialization"
    runtime_active = False
    stream_closed = False

    print(f"[PROGRESS] 0% (0/{TOTAL_STEPS}) START total_elapsed=00:00:00", flush=True)
    print("[INFO] Кожні 10 секунд буде [WORKING]: це означає, що процес живий і не закритий.", flush=True)

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

    def mark_started(label: str) -> None:
        nonlocal current_step, current_label, current_started, runtime_active
        current_step = min(TOTAL_STEPS, completed + 1)
        current_label = label
        current_started = time.monotonic()
        runtime_active = label == "Gameplay runtime"
        print(
            f"[PROGRESS] {_pct(completed)}% step {current_step}/{TOTAL_STEPS} START: {current_label}",
            flush=True,
        )

    def mark_completed() -> None:
        nonlocal completed, runtime_active
        if current_step <= 0 or completed >= current_step:
            return
        completed = current_step
        runtime_active = False
        now = time.monotonic()
        print(
            f"[PROGRESS] {_pct(completed)}% ({completed}/{TOTAL_STEPS}) completed "
            f"step_elapsed={_fmt(now - current_started)} total_elapsed={_fmt(now - batch_started)}",
            flush=True,
        )

    try:
        while True:
            try:
                item = events.get(timeout=1.0)
            except queue.Empty:
                item = "__NO_EVENT__"

            now = time.monotonic()

            if item is None:
                stream_closed = True
            elif item != "__NO_EVENT__":
                line = item
                last_child_output = now
                print(line, flush=True)

                if line.startswith("[RUN ] "):
                    mark_started(line[len("[RUN ] "):].strip())
                elif line.startswith("[RUNTIME] Current-source build"):
                    if current_step > completed:
                        mark_completed()
                    mark_started("Gameplay runtime")
                elif line.startswith("[POST]"):
                    if runtime_active and current_step > completed:
                        mark_completed()
                elif current_step > completed and (line.startswith("[PASS] ") or line.startswith("[FAIL] ")):
                    if current_label and current_label in line:
                        mark_completed()

            if proc.poll() is None and now - last_heartbeat >= HEARTBEAT_SECONDS:
                step = current_step if current_step > 0 else min(TOTAL_STEPS, completed + 1)
                step_elapsed = now - current_started if current_step > 0 else now - batch_started
                print(
                    f"[WORKING] {_pct(completed)}% step {step}/{TOTAL_STEPS} {current_label} "
                    f"elapsed={_fmt(step_elapsed)} total={_fmt(now - batch_started)} process_alive=1",
                    flush=True,
                )
                last_heartbeat = now

                quiet_for = now - last_child_output
                if quiet_for >= QUIET_WARNING_SECONDS and now - last_quiet_warning >= QUIET_WARNING_SECONDS:
                    print(
                        f"[WARN] Немає нового текстового виводу {_fmt(quiet_for)}, але процес ЖИВИЙ. "
                        "Не закривайте вікно лише через тишу.",
                        flush=True,
                    )
                    last_quiet_warning = now

            if stream_closed and proc.poll() is not None:
                break

        rc = int(proc.wait())
    except KeyboardInterrupt:
        print("\n[STOP] Тест перервано користувачем. Завершую дочірній процес...", flush=True)
        try:
            proc.terminate()
            proc.wait(timeout=8)
        except Exception:
            try:
                proc.kill()
            except Exception:
                pass
        return 130

    total_elapsed = time.monotonic() - batch_started
    if completed >= TOTAL_STEPS:
        print(
            f"[PROGRESS] 100% ({TOTAL_STEPS}/{TOTAL_STEPS}) FINISHED "
            f"total_elapsed={_fmt(total_elapsed)} process_alive=0 exit_code={rc}",
            flush=True,
        )
    else:
        print(
            f"[STOPPED] {_pct(completed)}% ({completed}/{TOTAL_STEPS}) pipeline ended early "
            f"total_elapsed={_fmt(total_elapsed)} process_alive=0 exit_code={rc}",
            flush=True,
        )
    return rc


if __name__ == "__main__":
    raise SystemExit(main())
