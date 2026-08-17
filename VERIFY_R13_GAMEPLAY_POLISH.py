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
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCCharacterVisualComponent.cpp",
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCCivilianVehicle.cpp",
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCPickupGunTruck.cpp",
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCBTR.cpp",
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCVisualEnvironment.cpp",
    PROJECT / "Source" / "OsterConflict" / "Public" / "OCR13AccessibilitySubsystem.h",
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCR13AccessibilitySubsystem.cpp",
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
character_visual = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCCharacterVisualComponent.cpp").read_text(encoding="utf-8")
civilian = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCCivilianVehicle.cpp").read_text(encoding="utf-8")
pickup = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCPickupGunTruck.cpp").read_text(encoding="utf-8")
btr = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCBTR.cpp").read_text(encoding="utf-8")
environment = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCVisualEnvironment.cpp").read_text(encoding="utf-8")
access_h = (PROJECT / "Source" / "OsterConflict" / "Public" / "OCR13AccessibilitySubsystem.h").read_text(encoding="utf-8")
access_cpp = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCR13AccessibilitySubsystem.cpp").read_text(encoding="utf-8")
weapon_variants = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCWeaponVariants.cpp").read_text(encoding="utf-8")

checks = [
    ("vehicles default to third-person camera", "bool bFirstPersonCamera = false;" in vehicle),
    ("R13 dynamic art is always cooked", '+DirectoriesToAlwaysCook=(Path="/Game/R13")' in packaging),
    ("Fab AK path is always cooked", '+DirectoriesToAlwaysCook=(Path="/Game/AK-47")' in packaging),
    ("deployment identity is not overwritten by polish layer", "SetText(Left, 1" not in ui_cpp),
    ("deployment spawn selection is not overwritten by polish layer", "SetText(Spawn, 1" not in ui_cpp),
    ("deployment start action is explicitly named", "ПОЧАТИ ГРУ" in ui_cpp),
    ("pause menu has leave-game action", "LeaveCurrentSession" in ui_h and "LeaveCurrentSession" in ui_cpp),
    ("pause leave button disconnects through controller", "PC->DisconnectFromServer();" in ui_cpp),
    ("pause menu explains Escape resume", "ESC = ПРОДОВЖИТИ" in ui_cpp),
    ("player-facing frontend has five top-level actions", all(marker in ui_cpp for marker in ["MainStart", "MainLocal", "MainNetwork", "MainSettings", "MainQuit"])),
    ("frontend uses full-screen Oster museum backdrop", "FullscreenMenuBackground" in ui_h and "/Game/R13/UI/Oster_Menu_BG.Oster_Menu_BG" in ui_cpp and "1600.0f, 900.0f" in ui_cpp),
    ("listen gameplay test starts through frontend", " -Frontend " in listen_test and "-NoFrontend" not in listen_test),
    ("driver turret mapping no longer steals free-look", 'ContextName == TEXT("IMC_DriverTurretRuntime")' in ui_cpp and "RemoveMappingContext(Context)" in ui_cpp),
    ("primitive first-person proxy hands are hidden", "Never expose primitive debug arms/hands in first person" in character_visual and "Part->SetVisibility(false, true);" in character_visual),
    ("civilian road speed is capped near 90 km/h", "MaxForwardSpeedKmh = 90.0f;" in civilian and "DriveForce = 1200000.0f;" in civilian),
    ("pickup road speed is 90 km/h", "MaxForwardSpeedKmh = 90.0f;" in pickup and "DriveForce = 1600000.0f;" in pickup),
    ("BTR clears 40 km/h with stronger steering", "MaxForwardSpeedKmh = 65.0f;" in btr and "SteeringTorque = 310000000.0f;" in btr),
    ("daylight atmosphere no longer uses amber-heavy scattering", "SetRayleighScatteringScale(1.0f)" in environment and "SetMieScatteringScale(0.004f)" in environment and "SetLightColor(FLinearColor::White)" in environment),
    ("Oster museum is the requested menu source", "Будинок Солонини, Остер.JPG" in download),
    ("content import creates current-state stamp", "R13_MUSEUM_WEAPONS_V2" in download),
    ("content importer rejects missing required assets", "runtime-required assets are missing" in import_script and "expected_assets" in import_script),
    ("listen launcher delegates to strict readiness gate", "CHECK_R13_LAUNCH_READY.ps1" in listen_test and "READY_RC" in listen_test),
    ("readiness gate refuses stale content", "R13 GAMEPLAY LAUNCH BLOCKED: REQUIRED ART IS MISSING OR STALE" in launch_ready and "R13_MUSEUM_WEAPONS_V2" in launch_ready),
    ("readiness gate refuses stale C++ module", "R13 GAMEPLAY LAUNCH BLOCKED: C++ BUILD IS STALE" in launch_ready and "LastWriteTimeUtc" in launch_ready),
    ("readiness gate checks weapon uassets", "WeaponRoot" in launch_ready and ".uasset" in launch_ready),
    ("readiness gate checks museum background uasset", "Oster_Menu_BG.uasset" in launch_ready),
    ("museum accessibility subsystem is world-scoped", "UWorldSubsystem" in access_h and "OsterConflict_Runtime" in access_cpp),
    ("museum entrance step ordering is repaired", "-2240.0f + Step * 120.0f" in access_cpp and "UpdateInstanceTransform" in access_cpp),
    ("AR broken vertical recoil is disabled for R13", "T.RecoilPitchMin = 0.0f; T.RecoilPitchMax = 0.0f;" in weapon_variants),
    ("AR retains horizontal shot feedback", "T.RecoilYawMax = 0.28f;" in weapon_variants),
    ("generated localization output stays out of Git changes", "OsterConflict/Content/Localization/Game/" in ignore),
    ("generated cook open-order logs stay out of Git changes", "OsterConflict/Build/**/FileOpenOrder/*.log" in ignore),
    ("local content state stays out of Git changes", "OsterConflict/Content/Raw/R13/R13_IMPORT_STATE.txt" in ignore),
]

failed = [name for name, ok in checks if not ok]
if failed:
    print("R13 gameplay polish verification: FAIL")
    print("Failed checks:", *failed, sep="\n - ")
    sys.exit(1)

print("R13 gameplay polish verification: PASS")
print(f"Checked {len(required)} required files and {len(checks)} gameplay/presentation regression markers.")
