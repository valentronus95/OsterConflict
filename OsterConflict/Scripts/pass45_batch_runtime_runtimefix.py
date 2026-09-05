#!/usr/bin/env python3
from __future__ import annotations

import subprocess
from pathlib import Path

import pass45_batch_runtime as base

_original_subprocess_run = subprocess.run
_runtime_user_aborted = False


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

        print(
            "[RUNTIME WINDOW] Запускаю у звичайному вікні 1280x720: його можна згорнути, "
            "перемкнути Alt+Tab і зробити скріншот.",
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


# pass45_batch_runtime imports the same subprocess module object, so patching it here changes only
# this shim process. The canonical orchestrator source remains a single source of truth.
subprocess.run = _patched_subprocess_run
base.subprocess.run = _patched_subprocess_run

if __name__ == "__main__":
    raise SystemExit(base.main())
