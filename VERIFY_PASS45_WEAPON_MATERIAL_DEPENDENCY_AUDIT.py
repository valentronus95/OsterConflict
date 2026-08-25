#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCProductionWeaponRuntimeValidationSubsystem.cpp"
VALIDATOR_CMD = ROOT / "OsterConflict" / "VALIDATE_PRODUCTION_MODELS_UE58.cmd"
CONTENT = ROOT / "OsterConflict" / "Content"
errors = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


cpp = read(CPP)
validator_cmd = read(VALIDATOR_CMD)

# Pass45 requires the exact chain: weapon -> mesh -> slot -> material -> texture dependencies -> runtime material.
for needle in (
    "GetUsedTextures(",
    "authoredMaterial=%s",
    "runtimeMaterial=%s",
    "textureCount=%d",
    "textures=%s",
    "PASS45 dependency contract: weapon class -> exact mesh -> material slot -> material asset -> used texture dependencies -> runtime material",
    "PASS45_WEAPON_DEPENDENCY_REPORT=PASS",
    "dependency_report=1",
):
    req(needle in cpp, f"weapon runtime dependency contract missing: {needle}")

# Imported/default material aliases are not accepted merely because they are project assets.
for needle in (
    "BasicShapeMaterial",
    "DefaultMaterial",
    "WorldGridMaterial",
    "_defaultMat",
):
    req(needle in cpp, f"placeholder material rejection missing: {needle}")

# Existing repository content that can support Pass45 must remain visible to the audit.
stein_expectations = {
    "MP5": "SKM_MP5.uasset",
    "1911": "SKM_1911.uasset",
    "M700": "SKM_M700.uasset",
    "M14": "SKM_M14.uasset",
    "Mac10": "SKM_Mac10.uasset",
    "Tec9": "SKM_Tec9.uasset",
    "LeverAction": "SKM_LeverAction.uasset",
}

for folder, mesh_name in stein_expectations.items():
    directory = CONTENT / "R13" / "Weapons" / "Stein" / folder
    req(directory.is_dir(), f"existing Stein weapon directory missing: {directory.relative_to(ROOT)}")
    req((directory / mesh_name).is_file(), f"existing Stein mesh missing: {(directory / mesh_name).relative_to(ROOT)}")
    companions = [
        path for path in directory.glob("*.uasset")
        if path.name != mesh_name and not path.name.startswith("SKM_")
    ]
    req(bool(companions), f"Stein weapon has no companion material/texture assets to inspect: {folder}")

req((CONTENT / "AK-47" / "Mesh" / "SKM_AK-47.uasset").is_file(),
    "AK-47 canonical mesh missing from current content")
req((CONTENT / "R13" / "Weapons" / "rocketlauncherModern.uasset").is_file(),
    "anti-armor launcher canonical mesh missing from current content")

# These two remain explicit CONTENT GAP when their production payload is absent. Do not fake READY.
production_gaps = []
for label, relative_path in (
    ("Remington870", Path("Production/Weapons/Remington870/SM_Remington870.uasset")),
    ("M249", Path("Production/Weapons/M249/SM_M249.uasset")),
):
    asset = CONTENT / relative_path
    if not asset.is_file():
        production_gaps.append(label)

req("R14_PRODUCTION_WEAPONS=PASS" in validator_cmd,
    "local UE production-model validation no longer requires the production weapon runtime sentinel")
req("weapon_runtime_validation.txt" in validator_cmd,
    "local UE production-model validation no longer exposes the weapon runtime report")

if errors:
    print("PASS45 WEAPON MATERIAL DEPENDENCY AUDIT: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 WEAPON MATERIAL DEPENDENCY AUDIT: PASS")
print("- runtime report exposes exact weapon mesh/material/runtime-material/used-texture chains")
print("- BasicShape/Default/WorldGrid/_defaultMat aliases are hard placeholder failures")
print("- current Stein/AK/launcher repository assets remain available for factual UE material inspection")
if production_gaps:
    print("- explicit production CONTENT GAP:", ", ".join(production_gaps))
else:
    print("- Remington870 and M249 production payloads exist and must pass the same runtime dependency gate")
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 build/runtime report remains authoritative")
