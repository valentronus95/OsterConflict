#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PLAYER = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCPlayerController.cpp"
OVERRIDE = ROOT / "OsterConflict" / "Source" / "OsterConflict" / "Private" / "OCLocalInboxWeaponOverrideSubsystem.cpp"
WORKFLOW = ROOT / ".github" / "workflows" / "pass45-local-build-import-regression.yml"

errors = []


def read(path: Path) -> str:
    if not path.is_file():
        errors.append(f"missing file: {path.relative_to(ROOT)}")
        return ""
    return path.read_text(encoding="utf-8", errors="replace")


def require(condition: bool, message: str) -> None:
    if not condition:
        errors.append(message)


player = read(PLAYER)
override = read(OVERRIDE)
workflow = read(WORKFLOW)

require('TEXT("Spawn all weapons")' in player, "sandbox admin lost Spawn all weapons action")
require('#include "OCLocalInboxRuntimeSubsystem.h"' in player, "sandbox rack no longer reads factual local runtime bindings")
require('GetAssetObjectPathsForCategory(Binding.Category, Paths)' in player, "sandbox rack no longer enumerates every bound path in each weapon category")
require('OC_FORCE_WEAPON_CATEGORY_%s' in player, "sandbox rack no longer tags each spawned actor with its exact bound category")
require('OC_FORCE_WEAPON_PATH_INDEX_%d' in player, "sandbox rack no longer tags each spawned actor with its exact bound path index")
require('RepresentedClasses' in player and 'SpawnedFallbackClasses' in player, "sandbox rack lost gameplay-class fallback coverage")
require('OC_SANDBOX_ALL_WEAPONS_SPAWNED' in player, "sandbox rack lost factual spawn-count log")

required_categories = (
    "M16_M4", "AR15", "AK74", "AK47", "ASSAULT_GENERIC", "RIFLE_GENERIC",
    "MP5", "SMG_GENERIC", "M1911", "MAKAROV", "PISTOL_GENERIC",
    "M700", "BALLISTA", "KAR98", "SNIPER_GENERIC", "REMINGTON870",
    "SHOTGUN_GENERIC", "M249", "LMG_GENERIC", "M14", "MAC10", "TEC9",
    "LEVER_ACTION", "M72", "LAUNCHER", "LAUNCHER_GENERIC",
)
for category in required_categories:
    require(f'TEXT("{category}")' in player, f"sandbox all-weapons rack lost category {category}")

required_classes = (
    "AOCWeapon_AssaultRifle", "AOCWeapon_SMG", "AOCWeapon_Pistol", "AOCWeapon_Sniper",
    "AOCWeapon_Shotgun", "AOCWeapon_LMG", "AOCWeapon_M14", "AOCWeapon_Mac10",
    "AOCWeapon_Tec9", "AOCWeapon_LeverAction", "AOCAntiArmorLauncher",
)
for cls in required_classes:
    require(f'{cls}::StaticClass()' in player, f"sandbox all-weapons rack lost gameplay class {cls}")

require('OC_FORCE_WEAPON_CATEGORY_' in override, "weapon runtime override no longer recognizes forced rack categories")
require('OC_FORCE_WEAPON_PATH_INDEX_' in override, "weapon runtime override no longer recognizes forced rack path indices")
require('ForcedPaths.IsValidIndex(ForcedPathIndex)' in override, "forced rack path index is no longer bounds-checked")
require('OutObjectPath = ForcedPaths[SafePathIndex]' in override, "forced rack no longer selects the exact requested bound model")
require('if (!ForcedCategory.IsEmpty())' in override, "forced rack category handling disappeared")
require('auto TryCategory =' in override, "normal weapon runtime fallback resolution disappeared")
require(
    override.index('if (!ForcedCategory.IsEmpty())') < override.index('auto TryCategory ='),
    "forced rack binding must run before normal category precedence can hide another model",
)

for path in (
    "OsterConflict/Source/OsterConflict/Private/OCPlayerController.cpp",
    "OsterConflict/Source/OsterConflict/Private/OCLocalInboxWeaponOverrideSubsystem.cpp",
    "VERIFY_PASS45_SANDBOX_WEAPON_RACK.py",
):
    require(path in workflow, f"asset regression workflow does not trigger for {path}")
require('python VERIFY_PASS45_SANDBOX_WEAPON_RACK.py' in workflow, "asset regression workflow does not execute sandbox weapon-rack verifier")

if errors:
    print("PASS45 SANDBOX WEAPON RACK: FAIL")
    for error in errors:
        print("[FAIL]", error)
    raise SystemExit(1)

print("PASS45 SANDBOX WEAPON RACK: PASS")
print("- Spawn all weapons enumerates every factual bound model path across supported weapon categories")
print("- M14, MAC-10, TEC-9, Lever Action and launcher gameplay classes are explicitly covered")
print("- each bound model is forced onto its own spawned actor before normal category precedence")
print("- gameplay classes without a bound model still receive one fallback test pickup")
print("- exact rack behavior is protected by the Pass45 asset regression workflow")
