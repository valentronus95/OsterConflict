#!/usr/bin/env python3
from __future__ import annotations

import os
import subprocess
from pathlib import Path

import pass45_batch_runtime as base

_original_subprocess_run = subprocess.run
_original_stage_run = base.run
_runtime_user_aborted = False
STRICT_STAGE_TIMEOUT_SECONDS = 270


def _is_game_runtime(command) -> bool:
    if not isinstance(command, (list, tuple)):
        return False
    lowered = [str(item).lower() for item in command]
    return (
        any(item.endswith("unrealeditor.exe") for item in lowered)
        and "-game" in lowered
        and "-frontend" in lowered
    )


def _patched_subprocess_run(*popenargs, **kwargs):
    global _runtime_user_aborted

    command = kwargs.get("args")
    positional = list(popenargs)
    if command is None and positional:
        command = positional[0]

    if _is_game_runtime(command):
        rewritten: list[str] = []
        for raw_item in command:
            item = str(raw_item)
            lowered_item = item.lower()
            if lowered_item == "-fullscreen":
                continue
            if lowered_item.startswith("-resx="):
                rewritten.append("-ResX=1280")
                continue
            if lowered_item.startswith("-resy="):
                rewritten.append("-ResY=720")
                continue
            rewritten.append(item)

        lowered = [item.lower() for item in rewritten]
        if "-windowed" not in lowered:
            rewritten.append("-windowed")
        if not any(item.lower().startswith("-resx=") for item in rewritten):
            rewritten.append("-ResX=1280")
        if not any(item.lower().startswith("-resy=") for item in rewritten):
            rewritten.append("-ResY=720")
        if not any(item.lower().startswith("-winx=") for item in rewritten):
            rewritten.append("-WinX=40")
        if not any(item.lower().startswith("-winy=") for item in rewritten):
            rewritten.append("-WinY=40")

        # Runtime acceptance currently values a responsive Windows message pump over RHI-thread A/B purity.
        # Keep the diagnostic/game window actually usable while the remaining startup owners are being retired.
        if "-norhithread" not in [item.lower() for item in rewritten]:
            rewritten.append("-norhithread")
        if "-nosplash" not in [item.lower() for item in rewritten]:
            rewritten.append("-nosplash")

        print(
            "[RUNTIME WINDOW] Стабільне вікно 1280x720: Alt+Tab/minimize/close мають працювати; "
            "startup splash вимкнено; RHI thread тимчасово OFF для цього пакетного runtime.",
            flush=True,
        )

        if "args" in kwargs:
            kwargs["args"] = rewritten
        elif positional:
            positional[0] = rewritten
            popenargs = tuple(positional)

        completed = _original_subprocess_run(*popenargs, **kwargs)
        normalized = base.normalize_windows_returncode(int(completed.returncode))
        _runtime_user_aborted = normalized == -1073741510
        if _runtime_user_aborted:
            print(
                "[RUNTIME STOP] Windows code -1073741510 (0xC000013A): runtime був примусово закритий/перерваний користувачем.",
                flush=True,
            )
        return completed

    return _original_subprocess_run(*popenargs, **kwargs)


def _terminate_stage_tree(proc: subprocess.Popen) -> None:
    if proc.poll() is not None:
        return
    if os.name == "nt":
        _original_subprocess_run(
            ["taskkill", "/PID", str(proc.pid), "/T", "/F"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=False,
        )
    else:
        proc.terminate()
        try:
            proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()
    try:
        proc.wait(timeout=10)
    except subprocess.TimeoutExpired:
        pass


def _strict_stage_run(stage):
    stage.log_path.parent.mkdir(parents=True, exist_ok=True)
    print(f"[RUN ] {stage.label}")
    try:
        with stage.log_path.open("wb") as output:
            proc = subprocess.Popen(
                stage.command,
                cwd=base.ROOT,
                stdout=output,
                stderr=subprocess.STDOUT,
            )
            try:
                raw_rc = proc.wait(timeout=STRICT_STAGE_TIMEOUT_SECONDS)
                stage.rc = base.normalize_windows_returncode(int(raw_rc))
            except subprocess.TimeoutExpired:
                _terminate_stage_tree(proc)
                stage.rc = 124
                output.write(
                    (
                        f"\n[STOP] Strict material gate exceeded {STRICT_STAGE_TIMEOUT_SECONDS}s. "
                        "Only this diagnostic process tree was terminated; batch runtime continues.\n"
                    ).encode("utf-8")
                )
                output.flush()
    except Exception as exc:
        stage.rc = 126
        stage.log_path.write_text(f"{type(exc).__name__}: {exc}\n", encoding="utf-8")

    stage.issues = base.extract_issues(stage.log_path)
    print(f"[{'PASS' if stage.rc == 0 else 'FAIL'}] {stage.label} code={stage.rc}")
    return stage


def _failure_evidence(stage) -> list[str]:
    evidence = list(stage.issues or [])
    if evidence:
        return evidence[:12]
    text = base.decode(stage.log_path)
    return [line.strip() for line in text.splitlines()[-12:] if line.strip()]


def _patched_stage_run(stage):
    if stage.name == "strict_material_gate":
        result = _strict_stage_run(stage)
    else:
        result = _original_stage_run(stage)

    if result.rc != 0:
        print(f"[DETAIL] {result.label} failure evidence:", flush=True)
        for line in _failure_evidence(result):
            print(f"   > {line}", flush=True)
    return result


def _previous_failure_evidence(path: Path, stage_name: str) -> list[str]:
    text = base.decode(path)
    if not text:
        return []

    lines = [line.strip() for line in text.splitlines() if line.strip()]
    selected: list[str] = []
    if stage_name == "all_local_assets":
        markers = ("UNBOUND_WEAPON", "LOCAL_WEAPON_NORMALIZE", "[STOP]", "[ERROR]")
    elif stage_name == "remington870":
        markers = (
            "PASS45_REMINGTON870_PRODUCTION_IMPORT_FAIL",
            "LogPython: Error:",
            "Python script executed with errors",
            "[STOP]",
        )
    else:
        markers = (
            "PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL",
            "PASS45_WEAPON_PRODUCTION_VISUAL_GAP",
            "PASS45_PRODUCTION_VEHICLE_",
            "Fatal error:",
            "Unhandled Exception:",
            "LogTemp: Error:",
            "[STOP]",
        )

    for line in lines:
        if any(marker.lower() in line.lower() for marker in markers) and line not in selected:
            selected.append(line)
            if len(selected) >= 12:
                break

    if selected:
        return selected
    return [line for line in lines[-12:]]


def _print_previous_failure_evidence() -> None:
    candidates = (
        ("all_local_assets", "ALL local/Fab assets", base.LOG_ROOT / "all_local_assets.log"),
        ("remington870", "Remington 870", base.LOG_ROOT / "remington870.log"),
        ("strict_material_gate", "Strict material gate", base.LOG_ROOT / "strict_material_gate.log"),
    )
    printed_header = False
    for stage_name, label, path in candidates:
        evidence = _previous_failure_evidence(path, stage_name)
        if not evidence:
            continue
        if not printed_header:
            print("[PREVIOUS RUN] Діагностика вже наявних PASS45 логів перед новим запуском:", flush=True)
            printed_header = True
        print(f"[PREVIOUS] {label}:", flush=True)
        for line in evidence:
            print(f"   > {line}", flush=True)
    if printed_header:
        print("[PREVIOUS RUN] Старі логи прочитані; далі запускається новий актуальний цикл.\n", flush=True)


# pass45_batch_runtime imports the same subprocess module object, so patching it here changes only
# this shim process. The canonical orchestrator source remains a single source of truth.
subprocess.run = _patched_subprocess_run
base.subprocess.run = _patched_subprocess_run
base.run = _patched_stage_run

if __name__ == "__main__":
    _print_previous_failure_evidence()
    raise SystemExit(base.main())
