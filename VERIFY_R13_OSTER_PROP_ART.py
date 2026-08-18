from pathlib import Path

ROOT = Path(__file__).resolve().parent
PROP_SOURCE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCR13OsterPropArtSubsystem.cpp"
METAL_SOURCE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCR13MetalFenceBridgeSubsystem.cpp"
METAL_HEADER = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Public" / "OCR13MetalFenceBridgeSubsystem.h"

REQUIRED_ASSETS = [
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Fence_Old_1_2m.uasset",
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Fence_Old_2_2m.uasset",
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Fence_Old_3_2m.uasset",
    ROOT / "OsterConflict" / "Content" / "Modular_Rural_Cabin" / "Meshes" / "Props" / "Power_Pole_Light.uasset",
    ROOT / "OsterConflict" / "Content" / "Scene_RoadsideConstruction" / "Assets" / "MS" / "3D" / "Urb_Roa_Sheet_Metal_Rusty_01" / "SM_Urb_Roa_Sheet_Metal_Rusty_01.uasset",
    ROOT / "OsterConflict" / "Content" / "Scene_RoadsideConstruction" / "Assets" / "MS" / "3D" / "Urb_Roa_Sheet_Metal_Rusty_02" / "SM_Urb_Roa_Sheet_Metal_Rusty_02.uasset",
    ROOT / "OsterConflict" / "Content" / "Scene_RoadsideConstruction" / "Assets" / "MS" / "3D" / "Urb_Roa_Sheet_Metal_Rusty_03" / "SM_Urb_Roa_Sheet_Metal_Rusty_03.uasset",
]

PROP_SOURCE_TOKENS = [
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
    'Root->SetMobility(EComponentMobility::Static)',
    'IsUsableVerticalFencePanel',
    'ClearFamilies',
    'bool AddFenceModules',
    'bool AddVerticalPropReplacements',
    'Target->ClearInstances()',
    'HideProxyIfFullyReplaced',
    'const bool bWoodComplete = AddFenceModules',
    'const bool bLightSheetComplete = AddFenceModules',
    'const bool bPowerPoleLightComplete = AddVerticalPropReplacements',
    'HideProxyIfFullyReplaced(WoodProxy, bWoodComplete, WoodAdded)',
    'HideProxyIfFullyReplaced(LightSheetProxy, bLightSheetComplete, LightSheetAdded)',
    'HideProxyIfFullyReplaced(StreetLightProxy, bPowerPoleLightComplete, PowerPoleLightAdded)',
    'source family hides only after complete replacement',
]

METAL_SOURCE_TOKENS = [
    'FindISM(WorldSector, TEXT("MetalFences"))',
    '/Engine/BasicShapes/Cube.Cube',
    'R13_MetalFencePickets',
    'R13_MetalFenceRails',
    'BuildOpenMetalFence',
    'PicketCount = FMath::Clamp',
    'Proxy->SetVisibility(false, true)',
    'retained hidden semantic proxy collision',
]


def fail(message: str) -> None:
    raise SystemExit(f"R13 OSTER PROP ART VERIFY FAIL: {message}")


for source in (PROP_SOURCE, METAL_SOURCE, METAL_HEADER):
    if not source.is_file():
        fail(f"missing source file: {source.relative_to(ROOT)}")

prop_text = PROP_SOURCE.read_text(encoding="utf-8")
for token in PROP_SOURCE_TOKENS:
    if token not in prop_text:
        fail(f"missing prop-art source guard/token: {token}")

if 'HideProxyIfReplaced(' in prop_text:
    fail("prop bridge must not hide a whole source family after only partial replacement")

metal_text = METAL_SOURCE.read_text(encoding="utf-8")
for token in METAL_SOURCE_TOKENS:
    if token not in metal_text:
        fail(f"missing metal-fence source guard/token: {token}")

if 'Proxy->SetCollisionEnabled' in metal_text:
    fail("MetalFences proxy collision must remain enabled when its visual slab is hidden")

for asset in REQUIRED_ASSETS:
    if not asset.is_file():
        fail(f"missing committed asset: {asset.relative_to(ROOT)}")

print("R13 OSTER PROP ART VERIFY: PASS")
