#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
CPP = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCProductionWeaponRuntimeValidationSubsystem.cpp"
VALIDATOR_CMD = ROOT / "OsterConflict" / "VALIDATE_PRODUCTION_MODELS_UE58.cmd"
STRICT_MATERIAL = ROOT / "OsterConflict" / "RUN_PASS45_STRICT_MATERIAL_GATE.cmd"
STEIN_REIMPORT = ROOT / "OsterConflict" / "Scripts" / "pass45_reimport_stein_weapon_materials.py"
STEIN_CMD = ROOT / "OsterConflict" / "PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd"
STEIN_TRY = ROOT / "OsterConflict" / "TRY_PASS45_STEIN_WEAPON_MATERIALS_UE58.cmd"
PRODUCTION_IMPORT = ROOT / "OsterConflict" / "Scripts" / "import_production_vehicle_assets.py"
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
stein_cmd = read(STEIN_CMD)
stein_try = read(STEIN_TRY)
production_import = read(PRODUCTION_IMPORT)
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

# Existing exact content that Pass45 can repair must remain present in the repository.
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

# Texture-first Stein import is the correction for the white/default runtime weapons.
for needle in (
    'IMPORT_CONTRACT_REVISION = "PASS45_STEIN_MATERIAL_CLOSURE_20260826_R1"',
    'source_dir.glob("*.png")',
    "import_source_textures(source_dir, destination)",
    "import_stein_fbx(fbx_source, destination)",
    "get_material_used_textures",
    "PASS45_STEIN_AUTHORED_DEPENDENCIES=PASS",
    "STATUS=EDITOR_IMPORT_VALIDATED_RUNTIME_VISUAL_PENDING",
):
    req(needle in stein_reimport, f"Stein material reimport contract missing: {needle}")
texture_call = stein_reimport.find("imported_textures = import_source_textures(source_dir, destination)")
fbx_call = stein_reimport.find("import_stein_fbx(fbx_source, destination)")
req(texture_call >= 0 and fbx_call > texture_call, "Stein PNG textures must import before the FBX")

for needle in (
    "-run=pythonscript",
    "pass45_reimport_stein_weapon_materials.py",
    "PASS45_STEIN_MATERIAL_CLOSURE_20260826_R1",
    "PASS45_STEIN_AUTHORED_DEPENDENCIES=PASS",
):
    req(needle in stein_cmd, f"UE 5.8 Stein reimport wrapper missing: {needle}")
for needle in (
    "PASS45_STEIN_MATERIAL_CLOSURE_20260826_R1",
    "PASS45_STEIN_AUTHORED_DEPENDENCIES=PASS",
    "cache is missing or stale",
):
    req(needle in stein_try, f"Stein freshness wrapper missing: {needle}")

# BTR intake is revision-based and has one canonical local-FBX-or-authored-GLB resolver.
for needle in (
    'IMPORT_CONTRACT_REVISION = "PASS45_MATERIAL_CLOSURE_20260826_R1"',
    "from generate_btr4_game_visual import build_btr4_glb",
    "BTR_GENERATED_SOURCE",
    "build_btr4_glb(BTR_GENERATED_SOURCE)",
    "authored_external_visual",
    "BTR4_AUTHORED_MATERIAL=M_BTR4_OC_Authored",
):
    req(needle in production_import, f"production/BTR import contract missing: {needle}")
for needle in (
    'IMPORT_CONTRACT_REVISION = "PASS45_MATERIAL_CLOSURE_20260826_R1"',
    "M_BTR4_OC_Authored",
    "SOURCE_KIND=BTR4:",
    "AUTHORED_MATERIALS_READY",
):
    req(needle in production_fresh, f"production fresh-load material gate missing: {needle}")
for needle in (
    "REQUIRED_REVISION=PASS45_MATERIAL_CLOSURE_20260826_R1",
    "Existing .uasset files are not sufficient proof of current materials.",
    "production_fresh_load_success.txt",
):
    req(needle in production_try, f"normal production freshness gate missing: {needle}")

# One user launcher, one strict vehicle-import owner. START_HERE prepares Stein; CURRENT_GAMEPLAY owns strict vehicle intake.
for needle in (
    ":prepare_materials_optional",
    ":prepare_materials_strict",
    "TRY_PRODUCTION_VEHICLES_UE58.cmd",
    "TRY_PASS45_STEIN_WEAPON_MATERIALS_UE58.cmd",
    "PASS45_REIMPORT_STEIN_WEAPON_MATERIALS_UE58.cmd",
    "RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd",
):
    req(needle in start_here, f"START_HERE Pass45 material/full-test route missing: {needle}")
req("IMPORT_PRODUCTION_VEHICLES_UE58.cmd" not in start_here,
    "START_HERE reintroduced duplicate strict production vehicle import")
req("IMPORT_PRODUCTION_VEHICLES_UE58.cmd" in normal and 'call "%PRODUCTION_IMPORT%"' in normal,
    "CURRENT_GAMEPLAY must remain the single strict production vehicle intake owner")
req("RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd" in full_acceptance,
    "full acceptance wrapper no longer delegates to playflow/performance")
req("RUN_R14_CURRENT_GAMEPLAY.cmd" in playflow,
    "playflow wrapper no longer delegates to the single gameplay launcher")

# Strict post-run material gate must accept explicit exact-content gaps only when the required available visuals pass.
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

# The standalone cumulative production validator still checks every exact repository asset that physically exists.
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
print("- Stein corrective import is texture-first and revisioned")
print("- BTR current intake is revisioned and has repository-authored fallback")
print("- START_HERE/full-test chain has one gameplay launch and one strict vehicle-import owner")
if production_gaps:
    print("- explicit exact-production CONTENT GAP (not READY):", ", ".join(production_gaps))
print("STATUS: SOURCE CONTRACT ONLY; local UE 5.8 import, build and rendered runtime remain authoritative")
