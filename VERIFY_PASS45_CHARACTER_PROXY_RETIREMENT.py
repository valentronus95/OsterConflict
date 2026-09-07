#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
HEADER = SRC / "Public" / "OCCharacterVisualComponent.h"
CPP = SRC / "Private" / "OCCharacterVisualComponent.cpp"
GAME_RECOVERY = ROOT / "GAME_RECOVERY.md"

errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


header = read(HEADER)
cpp = read(CPP)
game_recovery = read(GAME_RECOVERY)

# The diagnostic proxy implementation may remain available for isolated developer use, but production
# must never opt into visible Engine BasicShape character geometry by default.
req("bool bEnableSourceOnlyProxy = false;" in header,
    "character source-only BasicShape proxy is not fail-closed by default")
req("production gameplay fails closed" in header,
    "character proxy default has no explicit fail-closed rationale")
req("if (bEnableSourceOnlyProxy) BuildSourceOnlyProxy();" in cpp,
    "source proxy construction is no longer explicitly gated")
req("UpdateSourceOnlyProxy(!bHasProductionBody);" in cpp,
    "production-body/proxy visibility ownership changed unexpectedly")

# Keep the diagnostic implementation recognizable so a future refactor cannot silently make it unconditional.
for needle in (
    "/Engine/BasicShapes/Cube.Cube",
    "/Engine/BasicShapes/Sphere.Sphere",
    "/Engine/BasicShapes/Cylinder.Cylinder",
    "BuildSourceOnlyProxy",
    "ThirdPersonProxyParts",
    "FirstPersonProxyParts",
):
    req(needle in cpp, f"diagnostic proxy implementation signature missing: {needle}")

# GAME_RECOVERY.md is now the canonical task owner. Its production rules explicitly reject visible proxy/helper
# geometry and allow fallback only as fail-closed diagnostics, never as the normal picture shown to the player.
for needle in (
    "не повертати старі proxy/заглушки як production-рішення",
    "жодних синіх дисків, білих proxy, detached parts або debug shapes",
    "fallback дозволений лише як fail-closed діагностика",
):
    req(needle in game_recovery,
        f"canonical GAME_RECOVERY no-visible-proxy rule is missing: {needle}")

if errors:
    print("PASS45 CHARACTER PROXY RETIREMENT: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 CHARACTER PROXY RETIREMENT: PASS")
print("- source-only character BasicShape proxies remain diagnostic-only")
print("- production runtime default is fail-closed: bEnableSourceOnlyProxy=false")
print("- missing production body/arms cannot silently promote primitive geometry to accepted presentation")
print("- GAME_RECOVERY.md is the canonical current no-visible-proxy contract")
print("STATUS: SOURCE GUARDED; local UE 5.8 character visual acceptance remains required")
