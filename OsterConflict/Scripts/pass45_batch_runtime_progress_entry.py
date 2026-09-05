#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path

import pass45_batch_runtime_progress as progress

progress.ORCHESTRATOR = Path(__file__).with_name("pass45_batch_runtime_runtimefix.py")

if __name__ == "__main__":
    raise SystemExit(progress.main())
