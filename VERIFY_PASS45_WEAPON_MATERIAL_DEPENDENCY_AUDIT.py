#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCProductionWeaponRuntimeValidationSubsystem.cpp"
VALIDATOR_CMD = ROOT / "OsterConflict" / "VALIDATE_PRODUCTION_MODELS_UE58.cmd"
STEIN_REIMPORT = ROOT / "OsterConflict" / "Scripts" / "pass45_reimport_stein_weapon_materials.py"
STEIN_CMD = ROOT / "OsterConflict" / "PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd"
STEIN_TRY = ROOT / "OsterConflict" / "TRY_PASS45_STEIN_WEAPON_MATERIALS_UE58.cmd"
PRODUCTION_IMPORT = ROOT / "OsterConflict" / "Scripts" / "import_production_vehicle_assets.py"
PRODUCTION_FRESH = ROOT / "OsterConflict" / "Scripts" / "verify_production_vehicle_fresh_load.py"
PRODUCTION_TRY = ROOT / "OsterConflict" / "TRY_PRODUCTION_VEHICLES_UE58.cmd"
START_HERE = ROOT / "START_HERE.cmd"
UPROJECT = ROOT / "OsterConflict" / "OsterConflict.uproject"
CONTENT = ROOT / "OsterConflict" / "Content"
RAW_STEIN = CONTENT / "Raw" / "R13" / "Weapons" / "SteinClassicWeapons" / "WeaponsPack"
RUN_ALL = ROOT / "RUN_ALL_VERIFY.py"
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
stein_reimport = read(STEIN_REIMPORT)
stein_cmd = read(STEIN_CMD)
stein_try = read(STEIN_TRY)
production_import = read(PRODUCTION_IMPORT)
production_fresh = read(PRODUCTION_FRESH)
production_try = read(PRODUCTION_TRY)
start_here = read(START_HERE)
uproject = read(UPROJECT)
run_all = read(RUN_ALL)

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
    req(bool(companions), f"Stein weapon has no companion material assets to inspect: {folder}")

    raw_directory = RAW_STEIN / folder
    req(raw_directory.is_dir(), f"Stein authored source directory missing: {raw_directory.relative_to(ROOT)}")
    req(bool(list(raw_directory.glob("SKM_*.fbx"))), f"Stein FBX source missing: {folder}")
    req(bool(list(raw_directory.glob("*.png"))), f"Stein authored PNG texture source missing: {folder}")

req((CONTENT / "AK-47" / "Mesh" / "SKM_AK-47.uasset").is_file(),
    "AK-47 canonical mesh missing from current content")
req((CONTENT / "R13" / "Weapons" / "rocketlauncherModern.uasset").is_file(),
    "anti-armor launcher canonical mesh missing from current content")

# Stein corrective import must import external authored PNG files before the FBX and version the local cache.
for needle in (
    'IMPORT_CONTRACT_REVISION = "PASS45_STEIN_MATERIAL_CLOSURE_20260826_R1"',
    'source_dir.glob("*.png")',
    "import_source_textures(source_dir, destination)",
    "import_stein_fbx(fbx_source, destination)",
    "get_material_used_textures",
    "BasicShapeMaterial",
    "DefaultMaterial",
    "WorldGridMaterial",
    "_defaultMat",
    "PASS45_STEIN_AUTHORED_DEPENDENCIES=PASS",
    "STATUS=EDITOR_IMPORT_VALIDATED_RUNTIME_VISUAL_PENDING",
):
    req(needle in stein_reimport, f"Stein material reimport contract missing: {needle}")

texture_call = stein_reimport.find("imported_textures = import_source_textures(source_dir, destination)")
fbx_call = stein_reimport.find("import_stein_fbx(fbx_source, destination)")
req(texture_call >= 0 and fbx_call >= 0 and texture_call < fbx_call,
    "Stein corrective import must import authored PNG textures before the FBX")

for needle in (
    "-run=pythonscript",
    "pass45_reimport_stein_weapon_materials.py",
    "PASS45_STEIN_MATERIAL_CLOSURE_20260826_R1",
    "PASS45_STEIN_AUTHORED_DEPENDENCIES=PASS",
    "STATUS=EDITOR_IMPORT_VALIDATED_RUNTIME_VISUAL_PENDING",
):
    req(needle in stein_cmd, f"UE 5.8 Stein reimport command gate missing: {needle}")

for needle in (
    "PASS45_STEIN_MATERIAL_CLOSURE_20260826_R1",
    "PASS45_STEIN_AUTHORED_DEPENDENCIES=PASS",
    "PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd",
    "cache is missing or stale",
):
    req(needle in stein_try, f"Stein cached freshness gate missing: {needle}")

# BTR current production intake must not treat stale uasset existence as authored-material freshness.
for needle in (
    'IMPORT_CONTRACT_REVISION = "PASS45_MATERIAL_CLOSURE_20260826_R1"',
    "from generate_btr4_game_visual import build_btr4_glb",
    "BTR_GENERATED_SOURCE",
    "build_btr4_glb(BTR_GENERATED_SOURCE)",
    "authored_external_visual",
    "BTR4_AUTHORED_MATERIAL=M_BTR4_OC_Authored",
    "import_glb_combined(BTR_GENERATED_SOURCE, BTR_DEST, BTR_NAME)",
    "import_btr_fbx(BTR_SOURCE, BTR_TEXTURE_DIR, BTR_DEST, BTR_NAME)",
):
    req(needle in production_import, f"production/BTR import freshness contract missing: {needle}")

for needle in (
    'IMPORT_CONTRACT_REVISION = "PASS45_MATERIAL_CLOSURE_20260826_R1"',
    "_defaultmat",
    "SOURCE_KIND=BTR4:",
    "M_BTR4_OC_Authored",
    "Repository-safe BTR4 fallback was imported",
):
    req(needle in production_fresh, f"production fresh-load BTR material gate missing: {needle}")

for needle in (
    "REQUIRED_REVISION=PASS45_MATERIAL_CLOSURE_20260826_R1",
    "production_import_success.txt",
    "production_fresh_load_success.txt",
    "Existing .uasset files are not sufficient proof of current materials.",
    "FRESH_LOADED=/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus",
):
    req(needle in production_try, f"normal production intake freshness gate missing: {needle}")

# START_HERE is the user launcher. Both normal diagnostic and strict acceptance must prepare current material state.
for needle in (
    ":prepare_materials_optional",
    ":prepare_materials_strict",
    "TRY_PRODUCTION_VEHICLES_UE58.cmd",
    "TRY_PASS45_STEIN_WEAPON_MATERIALS_UE58.cmd",
    "IMPORT_PRODUCTION_VEHICLES_UE58.cmd",
    "PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd",
    "Rendered runtime appearance remains the final authority.",
):
    req(needle in start_here, f"START_HERE material preparation contract missing: {needle}")

req('"PythonScriptPlugin"' in uproject and '"Enabled": true' in uproject,
    "PythonScriptPlugin must remain enabled for the UE 5.8 Stein reimport commandlet")
req("VERIFY_PASS45_WEAPON_MATERIAL_DEPENDENCY_AUDIT.py" in run_all,
    "RUN_ALL_VERIFY.py must execute the Pass45 weapon dependency audit")

# The cumulative UE validator must run the corrective import and gate every repository-available weapon.
for needle in (
    "PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd",
    "call \"%STEIN_REIMPORT_CMD%\"",
    "every repository-available canonical weapon passed mesh + authored material + runtime material dependency checks",
    "CONTENT GAP: Remington 870 exact production payload is absent; it is not counted READY.",
    "CONTENT GAP: M249 exact production payload is absent; it is not counted READY.",
    'call :require_weapon_pass "AK-47"',
    'call :require_weapon_pass "MP5"',
    'call :require_weapon_pass "M1911"',
    'call :require_weapon_pass "M700"',
    'call :require_weapon_pass "M14"',
    'call :require_weapon_pass "MAC-10"',
    'call :require_weapon_pass "TEC-9"',
    'call :require_weapon_pass "Lever Action .45-70"',
    'call :require_weapon_pass "Anti-Armor Launcher"',
):
    req(needle in validator_cmd, f"available-weapon acceptance contract missing: {needle}")

# These two remain explicit CONTENT GAP when their production payload is absent. Do not fake READY.
production_gaps = []
for label, relative_path in (
    ("Remington870", Path("Production/Weapons/Remington870/SM_Remington870.uasset")),
    ("M249", Path("Production/Weapons/M249/SM_M249.uasset")),
):
    asset = CONTENT / relative_path
    if not asset.is_file():
        production_gaps.append(label)

req("weapon_runtime_validation.txt" in validator_cmd,
    "local UE production-model validation no longer exposes the weapon runtime report")
req("R14_PRODUCTION_WEAPONS=PASS" not in validator_cmd,
    "cumulative validator must not require an all-classes READY sentinel when explicit production CONTENT GAP assets are absent")

if errors:
    print("PASS45 WEAPON MATERIAL DEPENDENCY AUDIT: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 WEAPON MATERIAL DEPENDENCY AUDIT: PASS")
print("- runtime report exposes exact weapon mesh/material/runtime-material/used-texture chains")
print("- BasicShape/Default/WorldGrid/_defaultMat aliases are hard placeholder failures")
print("- Stein corrective path imports committed authored PNG textures before reimporting each runtime FBX")
print("- BTR current intake reimports on material-contract revision and uses authored repository fallback when local FBX is absent")
print("- stale uasset existence alone can no longer suppress production reimport")
print("- START_HERE prepares current material state before normal/compat/strict runtime routes")
print("- cumulative UE validator gates every repository-available canonical weapon instead of conflating absent payload with white-material failure")
if production_gaps:
    print("- explicit production CONTENT GAP (not READY):", ", ".join(production_gaps))
else:
    print("- Remington870 and M249 production payloads exist and must pass the same runtime dependency gate")
print("STATUS: SOURCE CONTRACT ONLY; UE editor reimport + local UE 5.8 rendered rack/BTR appearance remain authoritative")
