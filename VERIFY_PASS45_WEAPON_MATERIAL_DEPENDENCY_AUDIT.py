#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCProductionWeaponRuntimeValidationSubsystem.cpp"
VALIDATOR_CMD = ROOT / "OsterConflict" / "VALIDATE_PRODUCTION_MODELS_UE58.cmd"
STRICT_MATERIAL = ROOT / "OsterConflict" / "RUN_PASS45_STRICT_MATERIAL_GATE.cmd"
STEIN_REIMPORT = ROOT / "OsterConflict" / "Scripts" / "pass45_reimport_stein_weapon_materials.py"
STEIN_FRESH = ROOT / "OsterConflict" / "Scripts" / "verify_stein_weapon_materials_fresh_load.py"
STEIN_CMD = ROOT / "OsterConflict" / "PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd"
STEIN_TRY = ROOT / "OsterConflict" / "TRY_PASS45_STEIN_WEAPON_MATERIALS_UE58.cmd"
PRODUCTION_IMPORT = ROOT / "OsterConflict" / "Scripts" / "import_production_vehicle_assets.py"
PRODUCTION_IMPORT_CMD = ROOT / "OsterConflict" / "IMPORT_PRODUCTION_VEHICLES_UE58.cmd"
PRODUCTION_FRESH = ROOT / "OsterConflict" / "Scripts" / "verify_production_vehicle_fresh_load.py"
PRODUCTION_TRY = ROOT / "OsterConflict" / "TRY_PRODUCTION_VEHICLES_UE58.cmd"
START_HERE = ROOT / "START_HERE.cmd"
FULL_ACCEPTANCE = ROOT / "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd"
PLAYFLOW = ROOT / "RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd"
NORMAL = ROOT / "RUN_R14_CURRENT_GAMEPLAY.cmd"
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
strict_material = read(STRICT_MATERIAL)
stein_reimport = read(STEIN_REIMPORT)
stein_fresh = read(STEIN_FRESH)
stein_cmd = read(STEIN_CMD)
stein_try = read(STEIN_TRY)
production_import = read(PRODUCTION_IMPORT)
production_import_cmd = read(PRODUCTION_IMPORT_CMD)
production_fresh = read(PRODUCTION_FRESH)
production_try = read(PRODUCTION_TRY)
start_here = read(START_HERE)
full_acceptance = read(FULL_ACCEPTANCE)
playflow = read(PLAYFLOW)
normal = read(NORMAL)
uproject = read(UPROJECT)
run_all = read(RUN_ALL)

# Gate F is required-available, not fictional 11/11 exact-production readiness.
# Exact payload when present is mandatory. Missing exact payload may use an explicit real authored fallback,
# which must pass the same material/texture dependency checks and remains CONTENT GAP, never production READY.
for needle in (
    "GetUsedTextures(",
    "IsPlaceholderTexture(",
    "textureDependency=%s",
    "OutTextureDependencyGaps",
    "RealFallbackWeaponVisualTag",
    "FindRealFallbackStaticMeshComponent",
    "PASS45 dependency contract: weapon class -> exact mesh OR explicit real fallback -> material slot -> material asset -> used texture dependencies -> runtime material",
    "PASS45_REQUIRED_AVAILABLE_WEAPONS=PASS",
    "PASS45_AUTHORED_WEAPON_MATERIALS=PASS",
    "PASS45_WEAPON_DEPENDENCY_REPORT=PASS",
    "PASS45_EXACT_PRODUCTION_CONTENT_GAPS=",
    "PASS45_REQUIRED_AVAILABLE_WEAPON_VISUALS_VALIDATED_READY",
    "PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL",
):
    req(needle in cpp, f"required-available weapon runtime dependency contract missing: {needle}")

for needle in (
    "BasicShapeMaterial",
    "DefaultMaterial",
    "WorldGridMaterial",
    "_defaultMat",
    "DefaultTexture",
    "WhiteSquareTexture",
):
    req(needle in cpp, f"placeholder material/texture rejection missing: {needle}")

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
    req((directory / mesh_name).is_file(), f"existing Stein mesh missing: {(directory / mesh_name).relative_to(ROOT)}")
    raw_directory = RAW_STEIN / folder
    req(bool(list(raw_directory.glob("SKM_*.fbx"))), f"Stein FBX source missing: {folder}")
    req(bool(list(raw_directory.glob("*.png"))), f"Stein authored PNG texture source missing: {folder}")

req((CONTENT / "AK-47" / "Mesh" / "SKM_AK-47.uasset").is_file(), "AK-47 canonical mesh missing")
req((CONTENT / "R13" / "Weapons" / "rocketlauncherModern.uasset").is_file(), "anti-armor launcher canonical mesh missing")

# Factual UE 5.8 evidence on 2026-08-26 proved two separate problems. R2 still reached validation with zero
# discovered textures, and the old launcher allowed a raw -1 commandlet result to slip through IF ERRORLEVEL 1.
# R3 therefore always owns every Stein mesh slot with a deterministic material built from committed PNG source,
# treats same-process texture enumeration as advisory, and requires an independent fresh UE process before PASS.
for needle in (
    'IMPORT_CONTRACT_REVISION = "PASS45_STEIN_MATERIAL_CLOSURE_20260826_R3"',
    'source_dir.glob("*.png")',
    "import_source_textures(source_dir, destination)",
    "import_stein_fbx(fbx_source, destination)",
    "create_explicit_authored_material",
    "R3_ALWAYS_EXPLICIT",
    "MaterialExpressionTextureSample",
    "MP_BASE_COLOR",
    "recompile_material",
    "get_num_material_expressions",
    "PASS45_STEIN_AUTHORED_GRAPH=PASS",
    "PASS45_STEIN_UE58_EXPLICIT_BINDING=READY",
    "STATUS=EDITOR_GRAPH_AUTHORED_FRESH_LOAD_PENDING",
):
    req(needle in stein_reimport, f"Stein material reimport R3 contract missing: {needle}")
texture_call = stein_reimport.find("imported_textures = import_source_textures(source_dir, destination)")
fbx_call = stein_reimport.find("import_stein_fbx(fbx_source, destination)")
material_call = stein_reimport.find("material, material_path = create_explicit_authored_material")
binding_call = stein_reimport.find("slot_count = bind_material_to_mesh_slots")
req(texture_call >= 0 and fbx_call > texture_call, "Stein PNG textures must import before the FBX")
req(material_call > fbx_call, "Stein R3 explicit authored material must be created after FBX import")
req(binding_call > material_call, "Stein R3 explicit material must own mesh slots before authoring validation")

for needle in (
    'IMPORT_CONTRACT_REVISION = "PASS45_STEIN_MATERIAL_CLOSURE_20260826_R3"',
    "get_material_used_textures",
    "get_used_textures",
    "PASS45_STEIN_AUTHORED_DEPENDENCIES=PASS",
    "PASS45_STEIN_FRESH_LOAD=READY",
    "PASS45_STEIN_UE58_EXPLICIT_BINDING=READY",
    "STATUS=FRESH_LOAD_VALIDATED_RUNTIME_VISUAL_PENDING",
    "still exposes zero used textures after fresh load",
):
    req(needle in stein_fresh, f"Stein R3 fresh-load dependency gate missing: {needle}")

for needle in (
    "-run=pythonscript",
    "pass45_reimport_stein_weapon_materials.py",
    "verify_stein_weapon_materials_fresh_load.py",
    "pass45_stein_material_fresh_load_success.txt",
    "PASS45_STEIN_MATERIAL_CLOSURE_20260826_R3",
    "PASS45_STEIN_AUTHORED_GRAPH=PASS",
    "PASS45_STEIN_AUTHORED_DEPENDENCIES=PASS",
    "PASS45_STEIN_FRESH_LOAD=READY",
    "Never propagate that raw value",
):
    req(needle in stein_cmd, f"UE 5.8 Stein R3 authoring/fresh-load wrapper missing: {needle}")
for needle in (
    "PASS45_STEIN_MATERIAL_CLOSURE_20260826_R3",
    "pass45_stein_material_fresh_load_success.txt",
    "PASS45_STEIN_AUTHORED_DEPENDENCIES=PASS",
    "PASS45_STEIN_FRESH_LOAD=READY",
    "cache is missing or stale",
):
    req(needle in stein_try, f"Stein R3 freshness wrapper missing: {needle}")

# PASS45 item 30: BTR intake is R3 revision-based. Canonical source is +X forward internally,
# with internal +Z up exported through an explicit glTF +Y-up root transform.
for needle in (
    'IMPORT_CONTRACT_REVISION = "PASS45_BTR_GLTF_Y_UP_20260827_R3"',
    "from generate_btr4_game_visual import build_btr4_glb",
    "BTR_GENERATED_SOURCE",
    "build_btr4_glb(BTR_GENERATED_SOURCE)",
    "authored_external_visual_canonical_plus_x",
    "BTR4_AUTHORED_MATERIAL=M_BTR4_OC_Authored",
    "BTR4_FORWARD_AXIS=+X",
    "BTR4_GLTF_UP_AXIS=+Y",
    "BTR4_INTERNAL_UP_AXIS=+Z",
):
    req(needle in production_import, f"production/BTR R3 import contract missing: {needle}")
req("PASS45_BTR_AXIS_OPTIC_20260827_R2" not in production_import,
    "sideways BTR R2 import revision returned")
req("SOURCE_KIND=BTR4:local_user_fbx" not in production_import,
    "uncalibrated local BTR FBX was re-promoted to canonical runtime intake")

for needle in (
    'IMPORT_CONTRACT_REVISION = "PASS45_BTR_GLTF_Y_UP_20260827_R3"',
    "M_BTR4_OC_Authored",
    "SOURCE_KIND=BTR4:",
    "BTR4_FORWARD_AXIS=+X",
    "BTR4_GLTF_UP_AXIS=+Y",
    "BTR4_INTERNAL_UP_AXIS=+Z",
    "authored_external_visual_canonical_plus_x",
    "AUTHORED_MATERIALS_READY",
):
    req(needle in production_fresh, f"production fresh-load material/axis gate missing: {needle}")
req("PASS45_BTR_AXIS_OPTIC_20260827_R2" not in production_fresh,
    "production fresh-load regressed to BTR R2")

for needle in (
    "REQUIRED_REVISION=PASS45_BTR_GLTF_Y_UP_20260827_R3",
    "Existing .uasset files are not sufficient proof of current materials/orientation.",
    "BTR4_FORWARD_AXIS=+X",
    "BTR4_GLTF_UP_AXIS=+Y",
    "BTR4_INTERNAL_UP_AXIS=+Z",
    "production_fresh_load_success.txt",
):
    req(needle in production_try, f"normal production freshness material/axis gate missing: {needle}")
req("PASS45_BTR_AXIS_OPTIC_20260827_R2" not in production_try,
    "normal production freshness gate regressed to BTR R2")

for needle in (
    "REQUIRED_REVISION=PASS45_BTR_GLTF_Y_UP_20260827_R3",
    "PASS45_NONZERO_COMMANDLET_DEFERRED_TO_FRESH_LOAD",
    "if not exist \"%SUCCESS_SENTINEL%\"",
    "IMPORT_CONTRACT_REVISION=%REQUIRED_REVISION%",
    "BTR4_FORWARD_AXIS=+X",
    "BTR4_GLTF_UP_AXIS=+Y",
    "BTR4_INTERNAL_UP_AXIS=+Z",
    "Reopening imported production assets in a fresh UE process",
    "verify_production_vehicle_fresh_load.py",
    "production_fresh_load_success.txt",
    "fresh UE process could not validate imported production models",
):
    req(needle in production_import_cmd, f"production import commandlet/fresh-load R3 contract missing: {needle}")
req("PASS45_BTR_AXIS_OPTIC_20260827_R2" not in production_import_cmd,
    "strict production command regressed to BTR R2")

# One user launcher, one strict material/import owner chain. Quick option 1 must not run heavy import work.
for needle in (
    ":prepare_materials_strict",
    "PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd",
    "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd",
    "STEIN_STRICT_RC=!ERRORLEVEL!",
    'if not "!STEIN_STRICT_RC!"=="0"',
    'set "OC_QUICK_NORMAL=1"',
):
    req(needle in start_here, f"START_HERE strict/quick material route missing: {needle}")
for forbidden in (
    ":prepare_materials_optional",
    "TRY_PRODUCTION_VEHICLES_UE58.cmd",
    "TRY_PASS45_STEIN_WEAPON_MATERIALS_UE58.cmd",
    "IMPORT_PRODUCTION_VEHICLES_UE58.cmd",
):
    req(forbidden not in start_here, f"START_HERE quick path reintroduced heavy material/import work: {forbidden}")
req("IMPORT_PRODUCTION_VEHICLES_UE58.cmd" in normal and 'call "%PRODUCTION_IMPORT%"' in normal,
    "CURRENT_GAMEPLAY must remain the single strict production vehicle intake owner")
req('if /I "%OC_QUICK_NORMAL%"=="1" goto quick_normal_game' in normal,
    "CURRENT_GAMEPLAY quick preflight bypass is missing")
normal_parts = normal.split(':quick_normal_game', 1)
req(len(normal_parts) == 2, "CURRENT_GAMEPLAY is missing explicit quick-normal section")
if len(normal_parts) == 2:
    strict_normal, quick_normal = normal_parts
    req('call "%PRODUCTION_IMPORT%"' in strict_normal,
        "strict CURRENT_GAMEPLAY lost production vehicle intake")
    req('call "%PRODUCTION_IMPORT%"' not in quick_normal,
        "quick CURRENT_GAMEPLAY must not invoke production vehicle intake")
    req("verify_required_weapon_assets.py" not in quick_normal,
        "quick CURRENT_GAMEPLAY must not invoke weapon commandlet preflight")
req("RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd" in full_acceptance,
    "full acceptance wrapper no longer delegates to playflow/performance")
req("RUN_R14_CURRENT_GAMEPLAY.cmd" in playflow,
    "playflow wrapper no longer delegates to the single gameplay launcher")

for needle in (
    "PASS45_REQUIRED_AVAILABLE_WEAPONS=PASS",
    "PASS45_AUTHORED_WEAPON_MATERIALS=PASS",
    "PASS45_WEAPON_DEPENDENCY_REPORT=PASS",
    "PASS45_EXACT_PRODUCTION_CONTENT_GAPS=",
    "PASS45_REQUIRED_AVAILABLE_WEAPON_VISUALS_VALIDATED_READY",
    "PASS45_REQUIRED_AVAILABLE_WEAPON_RUNTIME_FAIL",
):
    req(needle in strict_material, f"strict material gate missing required-available contract: {needle}")
req("R14_PRODUCTION_WEAPONS=PASS" not in strict_material,
    "strict material gate resurrected all-classes exact-production readiness")

req('"PythonScriptPlugin"' in uproject and '"Enabled": true' in uproject,
    "PythonScriptPlugin must remain enabled for UE 5.8 material import")
req("VERIFY_PASS45_WEAPON_MATERIAL_DEPENDENCY_AUDIT.py" in run_all,
    "RUN_ALL_VERIFY.py must execute this Pass45 material audit")

for needle in (
    "PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd",
    "every repository-available canonical weapon passed mesh + authored material + runtime material dependency checks",
    "CONTENT GAP: Remington 870 exact production payload is absent; it is not counted READY.",
    "CONTENT GAP: M249 exact production payload is absent; it is not counted READY.",
):
    req(needle in validator_cmd, f"standalone available-exact validator missing: {needle}")
req("R14_PRODUCTION_WEAPONS=PASS" not in validator_cmd,
    "standalone validator must not require absent exact payloads to become READY")

production_gaps = []
for label, relative_path in (
    ("Remington870", Path("Production/Weapons/Remington870/SM_Remington870.uasset")),
    ("M249", Path("Production/Weapons/M249/SM_M249.uasset")),
):
    if not (CONTENT / relative_path).is_file():
        production_gaps.append(label)

if errors:
    print("PASS45 WEAPON MATERIAL DEPENDENCY AUDIT: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 WEAPON MATERIAL DEPENDENCY AUDIT: PASS")
print("- exact production and explicit real fallback are distinguished; fallback never impersonates production")
print("- every accepted visual requires non-placeholder material and used-texture dependencies")
print("- Stein R3 always owns its runtime mesh slots and requires independent fresh-load dependency proof")
print("- negative UnrealEditor-Cmd failures cannot slip through strict acceptance as PASS")
print("- production Interchange non-zero diagnostics may continue only through current sentinel + independent fresh-load verification")
print("- BTR R3 intake is revisioned, authored-material guarded, canonical +X-forward and explicit glTF +Y-up")
print("- quick normal skips heavy imports; option 2 retains one strict gameplay/material/import owner chain")
if production_gaps:
    print("- explicit exact-production CONTENT GAP (not READY):", ", ".join(production_gaps))
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 import, build and rendered runtime remain authoritative")
