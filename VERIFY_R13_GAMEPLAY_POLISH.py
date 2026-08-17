from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent
PROJECT = ROOT / "OsterConflict"

required = [
    ROOT / ".gitignore",
    ROOT / "R13_DOWNLOAD_AND_IMPORT_CONTENT.cmd",
    ROOT / "RUN_R11_LISTEN_TEST.cmd",
    ROOT / "PC_TEST" / "CHECK_R13_LAUNCH_READY.ps1",
    PROJECT / "Scripts" / "R13" / "IMPORT_R13_CONTENT.py",
    PROJECT / "Config" / "DefaultGame.ini",
    PROJECT / "Source" / "OsterConflict" / "Public" / "OCVehicleBase.h",
    PROJECT / "Source" / "OsterConflict" / "Public" / "OCUIRuntimePolishSubsystem.h",
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCUIRuntimePolishSubsystem.cpp",
    PROJECT / "Source" / "OsterConflict" / "Public" / "OCR13FrontendMenuSubsystem.h",
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCR13FrontendMenuSubsystem.cpp",
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCCharacterVisualComponent.cpp",
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCCivilianVehicle.cpp",
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCPickupGunTruck.cpp",
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCBTR.cpp",
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCVisualEnvironment.cpp",
    PROJECT / "Source" / "OsterConflict" / "Public" / "OCR13AccessibilitySubsystem.h",
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCR13AccessibilitySubsystem.cpp",
    PROJECT / "Source" / "OsterConflict" / "Public" / "OCR13VehicleArtSubsystem.h",
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCR13VehicleArtSubsystem.cpp",
    PROJECT / "Source" / "OsterConflict" / "Public" / "OCR13WholeOsterArtSubsystem.h",
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCR13WholeOsterArtSubsystem.cpp",
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCWeaponVariants.cpp",
]
missing = [str(p.relative_to(ROOT)) for p in required if not p.exists()]
if missing:
    print("R13 gameplay polish verification: FAIL")
    print("Missing:", *missing, sep="\n - ")
    sys.exit(1)

ignore = (ROOT / ".gitignore").read_text(encoding="utf-8")
download = (ROOT / "R13_DOWNLOAD_AND_IMPORT_CONTENT.cmd").read_text(encoding="utf-8")
listen_test = (ROOT / "RUN_R11_LISTEN_TEST.cmd").read_text(encoding="utf-8")
launch_ready = (ROOT / "PC_TEST" / "CHECK_R13_LAUNCH_READY.ps1").read_text(encoding="utf-8")
import_script = (PROJECT / "Scripts" / "R13" / "IMPORT_R13_CONTENT.py").read_text(encoding="utf-8")
packaging = (PROJECT / "Config" / "DefaultGame.ini").read_text(encoding="utf-8")
vehicle = (PROJECT / "Source" / "OsterConflict" / "Public" / "OCVehicleBase.h").read_text(encoding="utf-8")
ui_h = (PROJECT / "Source" / "OsterConflict" / "Public" / "OCUIRuntimePolishSubsystem.h").read_text(encoding="utf-8")
ui_cpp = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCUIRuntimePolishSubsystem.cpp").read_text(encoding="utf-8")
frontend_h = (PROJECT / "Source" / "OsterConflict" / "Public" / "OCR13FrontendMenuSubsystem.h").read_text(encoding="utf-8")
frontend_cpp = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCR13FrontendMenuSubsystem.cpp").read_text(encoding="utf-8")
character_visual = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCCharacterVisualComponent.cpp").read_text(encoding="utf-8")
civilian = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCCivilianVehicle.cpp").read_text(encoding="utf-8")
pickup = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCPickupGunTruck.cpp").read_text(encoding="utf-8")
btr = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCBTR.cpp").read_text(encoding="utf-8")
environment = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCVisualEnvironment.cpp").read_text(encoding="utf-8")
vehicle_art = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCR13VehicleArtSubsystem.cpp").read_text(encoding="utf-8")
whole_oster_art = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCR13WholeOsterArtSubsystem.cpp").read_text(encoding="utf-8")
access_h = (PROJECT / "Source" / "OsterConflict" / "Public" / "OCR13AccessibilitySubsystem.h").read_text(encoding="utf-8")
access_cpp = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCR13AccessibilitySubsystem.cpp").read_text(encoding="utf-8")
weapon_variants = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCWeaponVariants.cpp").read_text(encoding="utf-8")

checks = [
    ("vehicles default to third-person camera", "bool bFirstPersonCamera = false;" in vehicle),
    ("R13 dynamic art is always cooked", '+DirectoriesToAlwaysCook=(Path="/Game/R13")' in packaging),
    ("Fab AK path is always cooked", '+DirectoriesToAlwaysCook=(Path="/Game/AK-47")' in packaging),
    ("AdvancedVillage environment art is always cooked", '+DirectoriesToAlwaysCook=(Path="/Game/AdvancedVillagePack")' in packaging),
    ("VehicleVarietyPack art is always cooked", '+DirectoriesToAlwaysCook=(Path="/Game/VehicleVarietyPack")' in packaging),
    ("PN foliage grass is always cooked", '+DirectoriesToAlwaysCook=(Path="/Game/PN_FoliageCollection")' in packaging),
    ("deployment identity is not overwritten by polish layer", "SetText(Left, 1" not in ui_cpp),
    ("deployment spawn selection is not overwritten by polish layer", "SetText(Spawn, 1" not in ui_cpp),
    ("deployment start action is explicitly named", "ПОЧАТИ ГРУ" in ui_cpp),
    ("pause menu has leave-game action", "LeaveCurrentSession" in ui_h and "LeaveCurrentSession" in ui_cpp),
    ("pause leave button disconnects through controller", "PC->DisconnectFromServer();" in ui_cpp),
    ("dedicated frontend detaches legacy panel", "LegacyFrontend->RemoveFromParent();" in frontend_cpp and "SuppressLegacyFrontendLayers" in frontend_cpp),
    ("dedicated frontend owns independent canvas composition", all(marker in frontend_cpp for marker in ["R13_MenuWorldBlocker", "R13_MenuBackground", "R13_MenuShade", "R13_MenuPanel", "R13FrontendFillCanvas"])),
    ("dedicated menu art sits below deployment panel", "R13FrontendFillCanvas(Canvas->AddChildToCanvas(Blocker), 70)" in frontend_cpp and "R13FrontendFillCanvas(Canvas->AddChildToCanvas(Background), 71)" in frontend_cpp),
    ("approved main menu has split OSTER CONFLICT hierarchy", all(marker in frontend_cpp for marker in ["BrandOster", "BrandConflict", 'FName(TEXT("Light"))', 'FName(TEXT("Bold"))', "LetterSpacing"])),
    ("approved main menu removes opaque swamp panel", "FLinearColor(0.0f, 0.0f, 0.0f, 0.0f)" in frontend_cpp and "bMainPage ? FMargin(0.0f)" in frontend_cpp),
    ("approved main menu uses local left-side feather", "GradientStrips" in frontend_cpp and "R13FrontendPlaceGradientStrip" in frontend_cpp and "GradientVisibility" in frontend_cpp),
    ("approved main menu does not globally dim background", "const ESlateVisibility ShadeVisibility = bDimGameplay" in frontend_cpp),
    ("approved main menu buttons stay narrow and restrained", "SetWidthOverride(430.0f)" in frontend_cpp and "0.24f" in frontend_cpp and "0.34f" in frontend_cpp),
    ("dedicated frontend exposes five top-level actions", all(marker in frontend_cpp for marker in ["MainStart", "MainLocal", "MainNetwork", "MainSettings", "MainQuit"])),
    ("dedicated frontend owns press delegates", all(marker in frontend_cpp for marker in ["OnPrimaryClicked", "OnSecondaryClicked", "OnNetworkClicked", "OnSettingsClicked", "OnQuitClicked", "OnPressed.AddDynamic"])),
    ("dedicated frontend forces UI-only mouse/input mode", "bShowMouseCursor = true" in frontend_cpp and "bEnableClickEvents = true" in frontend_cpp and "FInputModeUIOnly" in frontend_cpp and "SetInputMode(Mode)" in frontend_cpp and "SetWidgetToFocus" in frontend_cpp),
    ("dedicated frontend restores game-only input on launch", "FInputModeGameOnly" in frontend_cpp and "bShowMouseCursor = false" in frontend_cpp and "ReleaseMenuInput" in frontend_cpp),
    ("pause menu owns explicit continue action", "PauseContinue" in frontend_cpp and "bPauseMenuActive" in frontend_cpp and "ПРОДОВЖИТИ ГРУ" in frontend_cpp),
    ("pause presentation dims gameplay without main backdrop", "SetPresentationVisibility(true, false, true)" in frontend_cpp and "0.54f" in frontend_cpp),
    ("dedicated frontend supports local and network pages", "ПОЧАТИ ЛОКАЛЬНУ ГРУ" in frontend_cpp and "ПІДКЛЮЧИТИСЯ" in frontend_cpp and "IP:порт сервера" in frontend_cpp),
    ("frontend uses opaque full-screen Oster backdrop", "/Game/R13/UI/Oster_Menu_BG.Oster_Menu_BG" in frontend_cpp and "R13_MenuWorldBlocker" in frontend_cpp and "SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f))" in frontend_cpp and "SetOffsets(FMargin(0.0f))" in frontend_cpp),
    ("custom menu art is normalized to opaque RGB before import", "Format24bppRgb" in download and "Normalizing custom artwork to an opaque PNG" in download),
    ("importer prefers normalized opaque menu source", "NORMALIZED_MENU_SOURCE" in import_script and "if NORMALIZED_MENU_SOURCE.exists()" in import_script),
    ("listen gameplay test starts through frontend", " -Frontend " in listen_test and "-NoFrontend" not in listen_test),
    ("driver turret mapping no longer steals free-look", 'ContextName == TEXT("IMC_DriverTurretRuntime")' in ui_cpp and "RemoveMappingContext(Context)" in ui_cpp),
    ("primitive first-person proxy hands are hidden", "Never expose primitive debug arms/hands in first person" in character_visual and "Part->SetVisibility(false, true);" in character_visual),
    ("civilian road speed is capped near 90 km/h", "MaxForwardSpeedKmh = 90.0f;" in civilian and "DriveForce = 1200000.0f;" in civilian),
    ("pickup road speed is 90 km/h", "MaxForwardSpeedKmh = 90.0f;" in pickup and "DriveForce = 1600000.0f;" in pickup),
    ("BTR clears 40 km/h with stronger steering", "MaxForwardSpeedKmh = 65.0f;" in btr and "SteeringTorque = 310000000.0f;" in btr),
    ("real road vehicle meshes replace cube chassis", all(marker in vehicle_art for marker in ["SM_Pickup.SM_Pickup", "SM_Hatchback.SM_Hatchback", "SM_SUV.SM_SUV", "FitMeshToPhysicsBody", "HideProxyParts"])),
    ("road vehicle first person is a cockpit view", all(marker in vehicle_art for marker in ["RepairDriverCockpit", "InteriorCamera->SetRelativeLocation", "Dashboard", "SteeringWheel", "SetFieldOfView(82.0f)"])),
    ("BTR is not disguised with a civilian placeholder", "if (Cast<AOCBTR>(Vehicle))" in vehicle_art and "correctly licensed military vehicle" in vehicle_art),
    ("whole Oster uses real house meshes", "SM_House_Var01.SM_House_Var01" in whole_oster_art and "AddHouseReplacements" in whole_oster_art),
    ("whole Oster uses real tree meshes", "SM_Tree_Var01.SM_Tree_Var01" in whole_oster_art and "AddTreeReplacements" in whole_oster_art),
    ("whole Oster uses dense real grass meshes", "SM_GrassPatch_Var01.SM_GrassPatch_Var01" in whole_oster_art and "grass_01_01_mesh.grass_01_01_mesh" in whole_oster_art and "constexpr float Fractions[] = { -0.42f, -0.21f, 0.0f, 0.21f, 0.42f }" in whole_oster_art and "GrassCount > 0" in whole_oster_art),
    ("whole Oster hides replaced primitive families", 'TEXT("Buildings")' in whole_oster_art and 'TEXT("TreeCrowns")' in whole_oster_art and 'TEXT("GrassMown")' in whole_oster_art),
    ("daylight atmosphere uses explicit Earth-like scattering", all(marker in environment for marker in ["SetRayleighScatteringScale(1.0f)", "SetRayleighScattering(FLinearColor(0.005802f, 0.013558f, 0.033100f))", "SetMieScatteringScale(1.0f)", "SetMieScattering(FLinearColor(0.003996f, 0.003996f, 0.003996f))", "SetLightColor(FLinearColor::White)"])),
    ("Oster museum fallback source remains documented", "Будинок Солонини, Остер.JPG" in download),
    ("content import creates current-state stamp", "R13_STEIN_WEAPONS_V3" in download),
    ("content importer rejects missing required assets", "runtime-required assets are missing" in import_script and "expected_assets" in import_script),
    ("listen launcher delegates to strict readiness gate", "CHECK_R13_LAUNCH_READY.ps1" in listen_test and "READY_RC" in listen_test),
    ("readiness gate refuses stale content", "R13 GAMEPLAY LAUNCH BLOCKED: REQUIRED ART IS MISSING OR STALE" in launch_ready and "R13_MUSEUM_WEAPONS_V2" in launch_ready),
    ("readiness gate refuses stale C++ module", "R13 GAMEPLAY LAUNCH BLOCKED: C++ BUILD IS STALE" in launch_ready and "LastWriteTimeUtc" in launch_ready),
    ("readiness gate checks weapon uassets", "WeaponRoot" in launch_ready and ".uasset" in launch_ready),
    ("readiness gate checks menu background uasset", "Oster_Menu_BG.uasset" in launch_ready),
    ("museum accessibility subsystem is world-scoped", "UWorldSubsystem" in access_h and "OsterConflict_Runtime" in access_cpp),
    ("museum entrance step ordering is repaired", "-2240.0f + Step * 120.0f" in access_cpp and "UpdateInstanceTransform" in access_cpp),
    ("AR broken vertical recoil is disabled for R13", "T.RecoilPitchMin = 0.0f; T.RecoilPitchMax = 0.0f;" in weapon_variants),
    ("AR retains horizontal shot feedback", "T.RecoilYawMax = 0.28f;" in weapon_variants),
    ("generated localization output stays out of Git changes", "OsterConflict/Content/Localization/Game/" in ignore),
    ("generated cook open-order logs stay out of Git changes", "OsterConflict/Build/**/FileOpenOrder/*.log" in ignore),
    ("local content state stays out of Git changes", "OsterConflict/Content/Raw/R13/R13_IMPORT_STATE.txt" in ignore),
    ("R13 import diagnostic log stays out of Git changes", "PC_TEST/R13_IMPORT_LAST.log" in ignore),
]

failed = [name for name, ok in checks if not ok]
if failed:
    print("R13 gameplay polish verification: FAIL")
    print("Failed checks:", *failed, sep="\n - ")
    sys.exit(1)

print("R13 gameplay polish verification: PASS")
print(f"Checked {len(required)} required files and {len(checks)} gameplay/presentation regression markers.")
