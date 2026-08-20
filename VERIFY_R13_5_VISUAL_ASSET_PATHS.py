from pathlib import Path

ROOT = Path(__file__).resolve().parent
CONTENT = ROOT / "OsterConflict" / "Content"


def fail(message: str) -> None:
    raise SystemExit("R13.5 VISUAL ASSET PATH VERIFY FAIL: " + message)


# Runtime LoadObject paths added/relied on by the consolidated environment pass.
# Verify repository paths directly so typos fail before a long UE compile/runtime test.
required_assets = [
    # Structural village houses retained invisibly for collision/placement metadata + deciduous trees.
    "AdvancedVillagePack/Meshes/SM_House_Var01.uasset",
    "AdvancedVillagePack/Meshes/SM_House_Var02.uasset",
    "AdvancedVillagePack/Meshes/SM_House_Var01_Extra01.uasset",
    "AdvancedVillagePack/Meshes/SM_House_Var01_Extra02.uasset",
    "AdvancedVillagePack/Meshes/SM_House_Var01_Extra03.uasset",
    "AdvancedVillagePack/Meshes/SM_House_Var01_Extra04.uasset",
    "AdvancedVillagePack/Meshes/SM_House_Var01_Extra05.uasset",
    "AdvancedVillagePack/Meshes/SM_House_Var01_Extra06.uasset",
    "AdvancedVillagePack/Meshes/SM_House_Var01_Extra07.uasset",
    "AdvancedVillagePack/Meshes/SM_House_Var01_Extra08.uasset",
    "AdvancedVillagePack/Meshes/SM_House_Var02_Extra.uasset",
    "AdvancedVillagePack/Meshes/SM_Tree_Var01.uasset",
    "AdvancedVillagePack/Meshes/SM_Tree_Var03.uasset",
    "AdvancedVillagePack/Meshes/SM_Tree_Var04.uasset",
    "AdvancedVillagePack/Meshes/SM_Tree_Var05.uasset",

    # PN grass used by whole-city environment dressing + Krushelnytska.
    "PN_FoliageCollection/Meshes/grassMesh/grass_01_01_mesh.uasset",
    "PN_FoliageCollection/Meshes/grassMesh/grass_01_02_mesh.uasset",
    "PN_FoliageCollection/Meshes/grassMesh/grass_01_03_mesh.uasset",

    # Rural foliage species/detail.
    "Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_01.uasset",
    "Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_02.uasset",
    "Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_03.uasset",
    "Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_04.uasset",
    "Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_05.uasset",
    "Modular_Rural_Cabin/Meshes/Foliage/Shrubs_1.uasset",
    "Modular_Rural_Cabin/Meshes/Foliage/Shrubs_1_Single.uasset",
    "Modular_Rural_Cabin/Meshes/Foliage/Bush_1.uasset",
    "Modular_Rural_Cabin/Meshes/Foliage/Cat_Tail.uasset",
    "Modular_Rural_Cabin/Meshes/Foliage/Cat_Tail_2.uasset",
    "Modular_Rural_Cabin/Meshes/Foliage/Flower_Patch_1.uasset",
    "Modular_Rural_Cabin/Meshes/Foliage/Grass_Patch_Long.uasset",

    # Rural props / utilities / seating.
    "Modular_Rural_Cabin/Meshes/Props/Side_Shed.uasset",
    "Modular_Rural_Cabin/Meshes/Props/Outhouse_House.uasset",
    "Modular_Rural_Cabin/Meshes/Props/Wheel_Barrow.uasset",
    "Modular_Rural_Cabin/Meshes/Props/Pallet.uasset",
    "Modular_Rural_Cabin/Meshes/Props/Tire.uasset",
    "Modular_Rural_Cabin/Meshes/Props/Utility_Box_1a.uasset",
    "Modular_Rural_Cabin/Meshes/Props/Power_Pole_1.uasset",
    "Modular_Rural_Cabin/Meshes/Props/Power_Pole_Addons.uasset",
    "Modular_Rural_Cabin/Meshes/Props/Power_Pole_Light.uasset",
    "Modular_Rural_Cabin/Meshes/Props/Old_Planks_Plank_1.uasset",
    "Modular_Rural_Cabin/Meshes/Props/Old_Planks_Plank_2.uasset",
    "Modular_Rural_Cabin/Meshes/Props/Old_Planks_Plank_3.uasset",

    # Residential fence families.
    "Modular_Rural_Cabin/Meshes/Props/Fence_Old_1_2m.uasset",
    "Modular_Rural_Cabin/Meshes/Props/Fence_Old_2_2m.uasset",
    "Modular_Rural_Cabin/Meshes/Props/Fence_Old_3_2m.uasset",
    "Scene_RoadsideConstruction/Assets/MS/3D/Urb_Roa_Sheet_Metal_Rusty_01/SM_Urb_Roa_Sheet_Metal_Rusty_01.uasset",
    "Scene_RoadsideConstruction/Assets/MS/3D/Urb_Roa_Sheet_Metal_Rusty_02/SM_Urb_Roa_Sheet_Metal_Rusty_02.uasset",
    "Scene_RoadsideConstruction/Assets/MS/3D/Urb_Roa_Sheet_Metal_Rusty_03/SM_Urb_Roa_Sheet_Metal_Rusty_03.uasset",

    # Oster residential roof/door/porch + enterable-house/environment materials.
    "Modular_Rural_Cabin/Meshes/Modular/Roof_Both_Ends_4m.uasset",
    "Modular_Rural_Cabin/Meshes/Modular/Porch_4x4m.uasset",
    "Modular_Rural_Cabin/Meshes/Modular/Door_01.uasset",
    "Modular_Rural_Cabin/Materials/Instances/Metal_Roof.uasset",
    "Modular_Rural_Cabin/Materials/Instances/Diorama_Ground.uasset",
    "Modular_Rural_Cabin/Materials/Instances/Glass_Window.uasset",
    "Modular_Rural_Cabin/Materials/Instances/Wood_Old.uasset",
    "Modular_Rural_Cabin/Materials/Instances/Wood_Planks_Painted_Blue.uasset",
    "Modular_Rural_Cabin/Materials/Instances/Wood_Planks_Painted_Green.uasset",
    "Modular_Rural_Cabin/Materials/Instances/Wood_Planks_Painted_Yellow.uasset",
    "Modular_Rural_Cabin/Materials/Instances/Wood_Planks_Painted_Red.uasset",

    # Landmark path + road/sidewalk art.
    "TileableForestRoad/Meshes/SM_Forest_Path.uasset",
    "Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Street_01/SM_Urb_Roa_Street_01.uasset",
    "Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Sidewalk_01/SM_Urb_Roa_Sidewalk_01.uasset",
]

missing = [rel for rel in required_assets if not (CONTENT / rel).is_file()]
if missing:
    fail("missing committed runtime assets: " + ", ".join(missing))

# Also make sure every visual source still refers to the intended asset families instead of silently drifting
# back to old prefab presentation for things for which explicit R13 art now exists.
source_dir = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private"
source_markers = {
    "OCR13MuseumReferenceSubsystem.cpp": [
        "Wood_Planks_Painted_Blue.Wood_Planks_Painted_Blue",
        "Glass_Window.Glass_Window",
        "SM_Pine_Tree_01.SM_Pine_Tree_01",
    ],
    "OCR13MuseumStadiumPhotoFidelitySubsystem.cpp": [
        "Roof_Both_Ends_4m.Roof_Both_Ends_4m",
        "Metal_Roof.Metal_Roof",
        "Wood_Planks_Painted_Blue.Wood_Planks_Painted_Blue",
        "Glass_Window.Glass_Window",
        "SM_Pine_Tree_01.SM_Pine_Tree_01",
        "SM_Pine_Tree_03.SM_Pine_Tree_03",
        "SM_Pine_Tree_05.SM_Pine_Tree_05",
        "SM_Tree_Var01.SM_Tree_Var01",
        "SM_Tree_Var04.SM_Tree_Var04",
        "R13_MuseumPhotoBrickBody",
        "R13_StadiumPhotoOpenGrass",
    ],
    "OCR13GroundSurfaceSubsystem.cpp": [
        "/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial",
        "R13_MatteOsterGround",
        "matte non-water city floor",
    ],
    "OCR13EnvironmentDressingSubsystem.cpp": [
        "SM_House_Var01_Extra%02d.SM_House_Var01_Extra%02d",
        "SM_House_Var02_Extra.SM_House_Var02_Extra",
    ],
    "OCR13OsterResidentialArchitectureSubsystem.cpp": [
        "Roof_Both_Ends_4m.Roof_Both_Ends_4m",
        "Porch_4x4m.Porch_4x4m",
        "Door_01.Door_01",
        "Metal_Roof.Metal_Roof",
        "R13_OsterBrickRed",
        "legacy collision retained invisibly",
    ],
    "OCR13EnterableHouseArtSubsystem.cpp": [
        "Roof_Both_Ends_4m.Roof_Both_Ends_4m",
        "Metal_Roof.Metal_Roof",
    ],
    "OCR13ResidentialYardSubsystem.cpp": [
        "Side_Shed.Side_Shed",
        "Outhouse_House.Outhouse_House",
        "Wheel_Barrow.Wheel_Barrow",
    ],
    "OCR13ResidentialInfillFenceSubsystem.cpp": [
        "Fence_Old_1_2m.Fence_Old_1_2m",
        "Fence_Old_2_2m.Fence_Old_2_2m",
        "Fence_Old_3_2m.Fence_Old_3_2m",
        "SM_Urb_Roa_Sheet_Metal_Rusty_01",
        "SM_Urb_Roa_Sheet_Metal_Rusty_02",
        "SM_Urb_Roa_Sheet_Metal_Rusty_03",
    ],
    "OCR13KrushelnytskaInfrastructureSubsystem.cpp": [
        "Power_Pole_1.Power_Pole_1",
        "Power_Pole_Addons.Power_Pole_Addons",
    ],
    "OCR13CentralParkCanopySubsystem.cpp": [
        "SM_Pine_Tree_03.SM_Pine_Tree_03",
        "SM_Pine_Tree_05.SM_Pine_Tree_05",
    ],
    "OCR13CollegeFacadeSubsystem.cpp": ["Glass_Window.Glass_Window"],
    "OCR13StadiumSurfaceSubsystem.cpp": ["Old_Planks_Plank_1.Old_Planks_Plank_1"],
}

for filename, markers in source_markers.items():
    path = source_dir / filename
    if not path.is_file():
        fail(f"missing visual source: {filename}")
    text = path.read_text(encoding="utf-8", errors="replace")
    for marker in markers:
        if marker not in text:
            fail(f"{filename} lost expected asset reference: {marker}")

if "Diorama_Ground.Diorama_Ground" in (source_dir / "OCR13GroundSurfaceSubsystem.cpp").read_text(encoding="utf-8", errors="replace"):
    fail("broad city ground regressed to the wet-looking diorama material")

print("R13.5 VISUAL ASSET PATH VERIFY: PASS")
print(f"Checked {len(required_assets)} committed mesh/material assets and {len(source_markers)} visual source integrations.")
