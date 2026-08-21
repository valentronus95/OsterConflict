from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
CPP = ROOT / "OsterConflict/Source/OsterConflict/Private/OCTacticalMapSubsystem.cpp"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


text = CPP.read_text(encoding="utf-8")

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
