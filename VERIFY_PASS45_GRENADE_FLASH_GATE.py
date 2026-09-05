#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
GRENADE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCGrenadeProjectile.cpp"
RUNTIME_GATE = ROOT / "VERIFY_PASS45_GRENADE_FLASH_RUNTIME.py"
LAUNCHER = ROOT / "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 FLASH GRENADE GATE: FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8", errors="replace")


grenade = read(GRENADE)
runtime_gate = read(RUNTIME_GATE)
launcher = read(LAUNCHER)
errors: list[str] = []


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


# Gameplay flash semantics remain present, but current source has no accepted dedicated world-VFX implementation.
req("ApplyFlashServer();" in grenade and "void AOCGrenadeProjectile::ApplyFlashServer()" in grenade,
    "flash gameplay/LOS semantics disappeared while presentation work remains open")
req("PASS45_FLASH_WORLD_VFX_RUNTIME_READY" not in grenade,
    "grenade source falsely emits flash world-VFX runtime READY without an accepted authored implementation")

# The standalone runtime gate must require future factual authored evidence and reject fail-visible content/load errors.
for needle in (
    "PASS45_FLASH_WORLD_VFX_RUNTIME_READY",
    "PASS45_FLASH_WORLD_VFX_CONTENT_GAP",
    "PASS45_FLASH_WORLD_VFX_LOAD_FAIL",
    "PASS45_FLASH_WORLD_VFX_RUNTIME_FAIL",
    "distinct authored flash-grenade world VFX runtime evidence",
):
    req(needle in runtime_gate, f"flash runtime gate missing: {needle}")

for needle in (
    "FLASH_VFX_VERIFY",
    "VERIFY_PASS45_GRENADE_FLASH_RUNTIME.py",
    "Verifying distinct authored flash-grenade world VFX evidence",
    "FLASH_VFX_RC",
    "Gameplay flash semantics alone cannot satisfy item 24",
):
    req(needle in launcher, f"main runtime launcher lost flash VFX gate wiring: {needle}")

if errors:
    print("PASS45 FLASH GRENADE GATE: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 FLASH GRENADE GATE: PASS")
print("- flash gameplay/LOS semantics remain intact")
print("- current source does not fake a dedicated flash world-VFX READY marker")
print("- final automated acceptance now requires distinct authored flash world-VFX runtime evidence")
print("STATUS: FAIL-HONEST SOURCE GATE; authored flash presentation content and direct UE 5.8 visual acceptance remain pending")
