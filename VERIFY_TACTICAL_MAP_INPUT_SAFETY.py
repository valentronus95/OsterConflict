from pathlib import Path
import re
import runpy

ROOT = Path(__file__).resolve().parent
MAP_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCTacticalMapSubsystem.cpp"
CHARACTER_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCCharacter.cpp"
HUD_CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCHUD.cpp"
VISUAL_VERIFY = ROOT / "VERIFY_TACTICAL_MAP_VISUAL_DESIGN.py"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


text = MAP_CPP.read_text(encoding="utf-8")
character = CHARACTER_CPP.read_text(encoding="utf-8")
hud = HUD_CPP.read_text(encoding="utf-8")

# Canonical key ownership: M belongs to Tactical Map; DeployTrap belongs to V everywhere.
require("MapKey(MapToggleAction, EKeys::M)" in text,
        "Tactical Map must keep the canonical M mapping")
require("DefaultMappingContext->MapKey(DeployTrapAction, EKeys::V);" in character,
        "DeployTrap default mapping must be V")
require("DefaultMappingContext->MapKey(DeployTrapAction, EKeys::M);" not in character,
        "DeployTrap must not reclaim Tactical Map key M")
require("V DEPLOY" in hud,
        "HUD must advertise V as the DeployTrap key")
require("M DEPLOY" not in hud,
        "HUD must not advertise M as the DeployTrap key")

# Drag must keep receiving mouse movement until LMB release, even when the cursor crosses the map edge.
require("CaptureMouse(TakeWidget())" in text,
        "LMB tactical-map drag must capture the mouse")
require(text.count("ReleaseMouseCapture()") >= 2,
        "tactical-map drag must release mouse capture on normal and defensive exit paths")

# Tactical Map owns the movement/look ignore lock it creates. When another UI displaces the map,
# those locks must be removed even if that other UI keeps its own cursor/input mode.
match = re.search(
    r"void\s+UOCTacticalMapSubsystem::CloseMap\([^)]*\)\s*\{(?P<body>.*?)\n\}",
    text,
    re.S,
)
require(match is not None, "CloseMap implementation not found")
body = match.group("body")
reset_move = body.find("PlayerController.ResetIgnoreMoveInput();")
reset_look = body.find("PlayerController.ResetIgnoreLookInput();")
early_return = body.find("if (!bRestoreGameplayInput) return;")
require(reset_move >= 0 and reset_look >= 0 and early_return >= 0,
        "CloseMap input recovery markers are incomplete")
require(reset_move < early_return and reset_look < early_return,
        "map-owned movement/look locks must be cleared before blocking-UI early return")

print("Tactical Map input safety contract: PASS")

# Keep the existing protected workflow as the entry point while making the production visual
# contract mandatory for every Tactical Map CI run.
require(VISUAL_VERIFY.exists(), "Tactical Map production visual verifier is missing")
runpy.run_path(str(VISUAL_VERIFY), run_name="__tactical_map_visual_contract__")
