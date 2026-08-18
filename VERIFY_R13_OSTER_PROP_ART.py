from pathlib import Path

ROOT = Path(__file__).resolve().parent
SOURCE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCR13OsterPropArtSubsystem.cpp"

REQUIRED_ASSETS = [
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Fence_Old_1_2m.uasset",
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Fence_Old_2_2m.uasset",
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Fence_Old_3_2m.uasset",
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Power_Pole_Light.uasset",
    ROOT / "OsterConflict" / "Content" / "Scene_RoadsideConstruction" / "Assets" / "MS" / "3D" / "Urb_Roa_Sheet_Metal_Rusty_01" / "SM_Urb_Roa_Sheet_Metal_Rusty_01.uasset",
    ROOT / "OsterConflict" / "Content" / "Scene_RoadsideConstruction" / "Assets" / "MS" / "3D" / "Urb_Roa_Sheet_Metal_Rusty_02" / "SM_Urb_Roa_Sheet_Metal_Rusty_02.uasset",
    ROOT / "OsterConflict" / "Content" / "Scene_RoadsideConstruction" / "Assets" / "MS" / "3D" / "Urb_Roa_Sheet_Metal_Rusty_03" / "SM_Urb_Roa_Sheet_Metal_Rusty_03.uasset",
]

REQUIRED_SOURCE_TOKENS = [
    'FindISM(WorldSector, TEXT("WoodFences"))',
    'FindISM(WorldSector, TEXT("LightSheetFences"))',
    'FindISMInWorld(World, TEXT("R12_StreetLights"))',
    'Fence_Old_1_2m.Fence_Old_1_2m',
    'Fence_Old_2_2m.Fence_Old_2_2m',
    'Fence_Old_3_2m.Fence_Old_3_2m',
    'Power_Pole_Light.Power_Pole_Light',
    'SM_Urb_Roa_Sheet_Metal_Rusty_01.SM_Urb_Roa_Sheet_Metal_Rusty_01',
    'SM_Urb_Roa_Sheet_Metal_Rusty_02.SM_Urb_Roa_Sheet_Metal_Rusty_02',
    'SM_Urb_Roa_Sheet_Metal_Rusty_03.SM_Urb_Roa_Sheet_Metal_Rusty_03',
    'IsUsableVerticalFencePanel',
    'AddVerticalPropReplacements',
    'HideProxyIfReplaced(WoodProxy, WoodAdded)',
    'HideProxyIfReplaced(LightSheetProxy, LightSheetAdded)',
    'HideProxyIfReplaced(StreetLightProxy, PowerPoleLightAdded)',
]


def fail(message: str) -> None:
    raise SystemExit(f"R13 OSTER PROP ART VERIFY FAIL: {message}")


if not SOURCE.is_file():
    fail(f"missing source file: {SOURCE.relative_to(ROOT)}")

source_text = SOURCE.read_text(encoding="utf-8")
for token in REQUIRED_SOURCE_TOKENS:
    if token not in source_text:
        fail(f"missing source guard/token: {token}")

for asset in REQUIRED_ASSETS:
    if not asset.is_file():
        fail(f"missing committed asset: {asset.relative_to(ROOT)}")

print("R13 OSTER PROP ART VERIFY: PASS")
