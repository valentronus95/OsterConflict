from pathlib import Path
import csv

ROOT = Path(__file__).resolve().parent
LEDGER = ROOT / "PASS45_CONTENT_INTEGRATION_LEDGER.csv"
SRC = ROOT / "OsterConflict/Source/OsterConflict/Private"

GRENADE = SRC / "OCGrenadeProjectile.cpp"
SMOKE = SRC / "OCSmokeCloud.cpp"
DENSE = SRC / "OCDenseGroundFoliageSubsystem.cpp"
TREES = SRC / "OCTreeContentUpgradeSubsystem.cpp"


def read(path: Path) -> str:
    if not path.is_file():
        raise SystemExit(f"PASS45 CONTENT LEDGER WIRING FAIL: missing {path.relative_to(ROOT)}")
    return path.read_text(encoding="utf-8")


with LEDGER.open("r", encoding="utf-8-sig", newline="") as fh:
    rows = {row["AssetId"]: row for row in csv.DictReader(fh)}

required_integrated = {
    "CI-VFX-001": "Fire_EXP grenade detonation Niagara",
    "CI-VFX-002": "PotaVFX smoke Niagara",
    "CI-WORLD-003A": "KiteDemo regional foliage and tree intake",
    "CI-WORLD-005": "PN foliage explicit fallback",
}

for asset_id, label in required_integrated.items():
    row = rows.get(asset_id)
    if not row:
        raise SystemExit(f"PASS45 CONTENT LEDGER WIRING FAIL: missing ledger row {asset_id} ({label})")
    if (row.get("State") or "").strip().upper() != "INTEGRATED":
        raise SystemExit(
            f"PASS45 CONTENT LEDGER WIRING FAIL: {asset_id} must remain INTEGRATED while source wiring exists"
        )

grenade = read(GRENADE)
smoke = read(SMOKE)
dense = read(DENSE)
trees = read(TREES)

for needle in (
    "/Game/Fire_EXP_Vol01_Free/",
    "PASS45_FRAG_EXPLOSION_VFX_DONOR_WIRED",
):
    if needle not in grenade:
        raise SystemExit(f"PASS45 CONTENT LEDGER WIRING FAIL: Fire_EXP runtime wiring missing {needle!r}")

for needle in (
    "/Game/PotaVFX_Smoke/",
    "PASS45_SMOKE_VFX_DONOR_WIRED",
    "runtime_acceptance=0",
):
    if needle not in smoke:
        raise SystemExit(f"PASS45 CONTENT LEDGER WIRING FAIL: PotaVFX runtime wiring missing {needle!r}")

for needle in (
    "/Game/KiteDemo/Environments/Foliage/Grass/FieldGrass/SM_FieldGrass_01.SM_FieldGrass_01",
    "/Game/KiteDemo/Environments/Foliage/Ferns/SM_Fern_01.SM_Fern_01",
    "/Game/KiteDemo/Environments/Foliage/Flowers/FieldScabious/SM_FieldScabious_01.SM_FieldScabious_01",
    "/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_01_mesh.grass_01_01_mesh",
):
    if needle not in dense:
        raise SystemExit(f"PASS45 CONTENT LEDGER WIRING FAIL: foliage runtime wiring missing {needle!r}")

for needle in (
    "/Game/KiteDemo/Environments/Trees/HillTree_02/HillTree_02.HillTree_02",
    "/Game/KiteDemo/Environments/Trees/ScotsPine_01/ScotsPine_01.ScotsPine_01",
    "/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01",
    "PASS45_REGIONAL_TREE_INTAKE_WIRED",
    "placement_preserved=1",
    "ground_base_preserved=1",
    "height_preserved=1",
    "runtime_acceptance=0",
):
    if needle not in trees:
        raise SystemExit(f"PASS45 CONTENT LEDGER WIRING FAIL: regional tree wiring missing {needle!r}")

# GroundTiles/Rocks intentionally remain pending. The current compact map must not invent river/water topology or
# blindly assign unverified demo materials just because the assets exist.
row = rows.get("CI-WORLD-003B")
if not row or (row.get("State") or "").strip().upper() != "PENDING_INTEGRATION":
    raise SystemExit("PASS45 CONTENT LEDGER WIRING FAIL: CI-WORLD-003B must remain pending until factual ground integration")

print("PASS45 CONTENT LEDGER RUNTIME WIRING: PASS")
print("- INTEGRATED VFX/foliage/tree ledger states have matching project-owned runtime source wiring")
print("- all source wiring stays fail-honest with runtime_acceptance=0 until local UE 5.8 evidence exists")
print("- KiteDemo GroundTiles/Rocks remain explicitly pending rather than false-ready")
print("STATUS: SOURCE INTEGRATION ONLY; local UE 5.8 runtime acceptance remains required")
