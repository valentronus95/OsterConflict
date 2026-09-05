#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"
HEADER = SRC / "Public" / "OCCharacterVisualComponent.h"
CPP = SRC / "Private" / "OCCharacterVisualComponent.cpp"
TZ = ROOT / "PASS45_RUNTIME_RECOVERY_TZ.md"

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
tz = read(TZ)

# The diagnostic proxy implementation may remain available for isolated developer use, but Pass45 production
# must never opt into visible Engine BasicShape character geometry by default.
req("bool bEnableSourceOnlyProxy = false;" in header,
    "character source-only BasicShape proxy is not fail-closed by default")
req("production gameplay fails closed" in header,
    "character proxy default has no explicit Pass45 fail-closed rationale")
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

req("Stable 60 FPS is not permission to ship primitive visuals." in tz,
    "canonical Pass45 Gate K anti-prototype rule is missing")
req("no visible production BasicShape/proxy content" in tz,
    "canonical Pass45 Gate K no-visible-proxy rule is missing")

if errors:
    print("PASS45 CHARACTER PROXY RETIREMENT: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 CHARACTER PROXY RETIREMENT: PASS")
print("- source-only character BasicShape proxies remain diagnostic-only")
print("- production runtime default is fail-closed: bEnableSourceOnlyProxy=false")
print("- missing production body/arms cannot silently promote primitive geometry to accepted presentation")
print("STATUS: SOURCE GUARDED; local UE 5.8 character visual acceptance remains required")
