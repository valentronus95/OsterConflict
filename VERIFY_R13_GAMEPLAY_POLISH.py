from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parent
PROJECT = ROOT / "OsterConflict"

required = [
    ROOT / ".gitignore",
    ROOT / "R13_DOWNLOAD_AND_IMPORT_CONTENT.cmd",
    PROJECT / "Config" / "DefaultGame.ini",
    PROJECT / "Source" / "OsterConflict" / "Public" / "OCVehicleBase.h",
    PROJECT / "Source" / "OsterConflict" / "Public" / "OCUIRuntimePolishSubsystem.h",
    PROJECT / "Source" / "OsterConflict" / "Private" / "OCUIRuntimePolishSubsystem.cpp",
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
packaging = (PROJECT / "Config" / "DefaultGame.ini").read_text(encoding="utf-8")
vehicle = (PROJECT / "Source" / "OsterConflict" / "Public" / "OCVehicleBase.h").read_text(encoding="utf-8")
ui_h = (PROJECT / "Source" / "OsterConflict" / "Public" / "OCUIRuntimePolishSubsystem.h").read_text(encoding="utf-8")
ui_cpp = (PROJECT / "Source" / "OsterConflict" / "Private" / "OCUIRuntimePolishSubsystem.cpp").read_text(encoding="utf-8")
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
    ("Oster museum is the requested menu source", "Будинок Солонини, Остер.JPG" in download),
    ("museum accessibility subsystem is world-scoped", "UWorldSubsystem" in access_h and "OsterConflict_Runtime" in access_cpp),
    ("museum entrance step ordering is repaired", "-2240.0f + Step * 120.0f" in access_cpp and "UpdateInstanceTransform" in access_cpp),
    ("AR broken vertical recoil is disabled for R13", "T.RecoilPitchMin = 0.0f; T.RecoilPitchMax = 0.0f;" in weapon_variants),
    ("AR retains horizontal shot feedback", "T.RecoilYawMax = 0.28f;" in weapon_variants),
    ("generated localization output stays out of Git changes", "OsterConflict/Content/Localization/Game/" in ignore),
    ("generated cook open-order logs stay out of Git changes", "OsterConflict/Build/**/FileOpenOrder/*.log" in ignore),
]

failed = [name for name, ok in checks if not ok]
if failed:
    print("R13 gameplay polish verification: FAIL")
    print("Failed checks:", *failed, sep="\n - ")
    sys.exit(1)

print("R13 gameplay polish verification: PASS")
print(f"Checked {len(required)} required files and {len(checks)} gameplay/presentation regression markers.")
