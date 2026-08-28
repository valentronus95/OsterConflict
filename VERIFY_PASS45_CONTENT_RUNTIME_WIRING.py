#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"

SMOKE_CPP = SRC / "Private" / "OCSmokeCloud.cpp"
SMOKE_H = SRC / "Public" / "OCSmokeCloud.h"
DENSE = SRC / "Private" / "OCDenseGroundFoliageSubsystem.cpp"
BUILD = SRC / "OsterConflict.Build.cs"

ASSETS = {
    "smoke_niagara": ROOT / "OsterConflict/Content/PotaVFX_Smoke/VFX/System/ColorSmoke/NS_SmokeGradient_Loop.uasset",
    "field_grass": ROOT / "OsterConflict/Content/KiteDemo/Environments/Foliage/Grass/FieldGrass/SM_FieldGrass_01.uasset",
    "fern": ROOT / "OsterConflict/Content/KiteDemo/Environments/Foliage/Ferns/SM_Fern_01.uasset",
    "field_flower": ROOT / "OsterConflict/Content/KiteDemo/Environments/Foliage/Flowers/FieldScabious/SM_FieldScabious_01.uasset",
}

errors: list[str] = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def req(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


smoke = read(SMOKE_CPP)
smoke_h = read(SMOKE_H)
dense = read(DENSE)
build = read(BUILD)

for label, path in ASSETS.items():
    req(path.is_file(), f"tracked intake asset missing: {label}: {path.relative_to(ROOT)}")

# Smoke: imported authored Niagara is wired to the existing AOCSmokeCloud gameplay owner.
smoke_path = "/Game/PotaVFX_Smoke/VFX/System/ColorSmoke/NS_SmokeGradient_Loop.NS_SmokeGradient_Loop"
req(smoke_path in smoke, "PotaVFX authored smoke donor is not selected by AOCSmokeCloud")
req("TObjectPtr<UNiagaraComponent> SmokeVFX" in smoke_h, "AOCSmokeCloud has no single Niagara presentation component")
req("PASS45_SMOKE_VFX_DONOR_WIRED" in smoke and "authored_niagara=1" in smoke,
    "smoke source wiring is not fail-visible")
req("PASS45_SMOKE_VFX_LOAD_FAIL" in smoke and "primitive_visible=0" in smoke,
    "smoke load failure can regress to a fake primitive presentation")
req("PASS45_SMOKE_VFX_CONTENT_GAP" not in smoke, "obsolete smoke content-gap implementation remains active")
req('"Niagara"' in build, "Niagara module dependency is missing")

# Block0 ground foliage: imported KiteDemo assets are preferred inside the existing single dense-foliage owner.
for needle in (
    "/Game/KiteDemo/Environments/Foliage/Grass/FieldGrass/SM_FieldGrass_01.SM_FieldGrass_01",
    "/Game/KiteDemo/Environments/Foliage/Ferns/SM_Fern_01.SM_Fern_01",
    "/Game/KiteDemo/Environments/Foliage/Flowers/FieldScabious/SM_FieldScabious_01.SM_FieldScabious_01",
    "content_intake=KiteDemo",
    "full_playable_bounds=1",
    "candidate_surface_guard=1",
):
    req(needle in dense, f"Block0 imported foliage runtime selection missing: {needle}")

# Existing performance and ownership rules must survive content intake.
for needle in (
    "FullGridStepCm = 1000.0f",
    "LowCPUGridStepCm = 1500.0f",
    "FullCellsPerBatch = 32",
    "LowCPUCellsPerBatch = 48",
    "Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);",
    "Component->SetCastShadow(false);",
    "bool IsMaintainedCivicZone(const FVector& Point)",
    "const float PlantChance = bMaintained ? 0.03f : 0.12f;",
    "const float FlowerChance = bMaintained ? 0.008f : 0.025f;",
):
    req(needle in dense, f"content intake regressed Block0 foliage contract: {needle}")

if errors:
    print("PASS45 CONTENT RUNTIME WIRING: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 CONTENT RUNTIME WIRING: PASS")
print("- imported PotaVFX smoke Niagara is source-wired without primitive fallback")
print("- imported KiteDemo field grass, fern and field flower are selected by the existing Block0 foliage owner")
print("- full-map bounds, candidate surface guards and LowCPU budgets remain intact")
print("STATUS: SOURCE-INTEGRATED; UE 5.8 visual/runtime acceptance remains pending")
