from pathlib import Path
import re
import sys

base = Path(__file__).resolve().parent
root = base / "OsterConflict"

required = [
    "OsterConflict.uproject",
    "Source/OsterConflict/Public/OCHealthComponent.h",
    "Source/OsterConflict/Private/OCHealthComponent.cpp",
    "Source/OsterConflict/Public/OCCharacter.h",
    "Source/OsterConflict/Private/OCCharacter.cpp",
    "Source/OsterConflict/Public/OCPlayerState.h",
    "Source/OsterConflict/Private/OCPlayerState.cpp",
    "Source/OsterConflict/Private/OCHUD.cpp",
    "Docs/SESSION_05_README_UA.md",
    "Docs/DOWNED_MEDIC_ARCHITECTURE_S05.md",
    "Docs/S05_TEST_MATRIX.md",
    "Docs/ROADMAP_SESSIONS.md",
]

missing = [item for item in required if not (root / item).exists()]
if missing:
    print("S05 structural verification: FAIL")
    print("Missing:", *missing, sep="\n - ")
    sys.exit(1)

checks = {
    "life state enum": ("Source/OsterConflict/Public/OCHealthComponent.h", "enum class EOCLifeState"),
    "downed state": ("Source/OsterConflict/Public/OCHealthComponent.h", "Downed"),
    "60 second duration": ("Source/OsterConflict/Public/OCHealthComponent.h", "float DownedDuration = 60.0f"),
    "life state replication": ("Source/OsterConflict/Private/OCHealthComponent.cpp", "DOREPLIFETIME(UOCHealthComponent, LifeState)"),
    "downed timestamp replication": ("Source/OsterConflict/Private/OCHealthComponent.cpp", "DOREPLIFETIME(UOCHealthComponent, DownedEndServerTime)"),
    "server synchronized time": ("Source/OsterConflict/Private/OCHealthComponent.cpp", "GetServerWorldTimeSeconds"),
    "server bleedout": ("Source/OsterConflict/Private/OCHealthComponent.cpp", "BleedOutTimerHandle"),
    "server revive": ("Source/OsterConflict/Private/OCHealthComponent.cpp", "ReviveServer"),
    "give up server": ("Source/OsterConflict/Private/OCHealthComponent.cpp", "GiveUpServer"),
    "crawl speed": ("Source/OsterConflict/Public/OCCharacter.h", "DownedCrawlSpeed = 95.0f"),
    "medic capability": ("Source/OsterConflict/Public/OCCharacter.h", "bHasMedicCapability"),
    "revive distance": ("Source/OsterConflict/Public/OCCharacter.h", "ReviveDistance = 220.0f"),
    "revive hold": ("Source/OsterConflict/Public/OCCharacter.h", "ReviveHoldSeconds = 3.0f"),
    "give up hold": ("Source/OsterConflict/Public/OCCharacter.h", "GiveUpHoldSeconds = 2.0f"),
    "revive target replication": ("Source/OsterConflict/Private/OCCharacter.cpp", "DOREPLIFETIME(AOCCharacter, ReviveTarget)"),
    "give up replication": ("Source/OsterConflict/Private/OCCharacter.cpp", "DOREPLIFETIME(AOCCharacter, GiveUpEndServerTime)"),
    "continuous revive validation": ("Source/OsterConflict/Private/OCCharacter.cpp", "HasAuthority() && ReviveTarget && !CanReviveTargetServer"),
    "revive line of sight": ("Source/OsterConflict/Private/OCCharacter.cpp", "OCReviveLineOfSight"),
    "revive interaction": ("Source/OsterConflict/Private/OCCharacter.cpp", "StartReviveServer"),
    "cancel revive RPC": ("Source/OsterConflict/Private/OCCharacter.cpp", "ServerCancelInteract_Implementation"),
    "give up RPC": ("Source/OsterConflict/Private/OCCharacter.cpp", "ServerSetGiveUpHeld_Implementation"),
    "downed handler": ("Source/OsterConflict/Private/OCCharacter.cpp", "HandleDowned"),
    "revived handler": ("Source/OsterConflict/Private/OCCharacter.cpp", "HandleRevived"),
    "revive stats": ("Source/OsterConflict/Private/OCPlayerState.cpp", "RegisterRevive"),
    "revives replicate": ("Source/OsterConflict/Private/OCPlayerState.cpp", "DOREPLIFETIME(AOCPlayerState, Revives)"),
    "downed HUD": ("Source/OsterConflict/Private/OCHUD.cpp", 'TEXT("DOWNED")'),
    "bleedout HUD": ("Source/OsterConflict/Private/OCHUD.cpp", "BLEED OUT"),
    "giveup HUD": ("Source/OsterConflict/Private/OCHUD.cpp", "HOLD SPACE TO GIVE UP"),
    "revive HUD": ("Source/OsterConflict/Private/OCHUD.cpp", "REVIVING"),
    "scoreboard revive column": ("Source/OsterConflict/Private/OCHUD.cpp", "GetRevives()"),
}

for name, (rel, marker) in checks.items():
    text = (root / rel).read_text(encoding="utf-8")
    if marker not in text:
        print(f"S05 structural verification: FAIL — {name}")
        sys.exit(1)

# Lightweight delimiter sanity across source/build scripts.
for path in (root / "Source").rglob("*"):
    if path.suffix not in {".h", ".cpp", ".cs"}:
        continue
    text = path.read_text(encoding="utf-8", errors="ignore")
    for left, right in (("{", "}"), ("(", ")"), ("[", "]")):
        if text.count(left) != text.count(right):
            print(f"S05 structural verification: FAIL — delimiter mismatch in {path}")
            sys.exit(1)

# Unreal Header Tool include rule: *.generated.h must be the final #include in reflected headers.
for path in (root / "Source/OsterConflict/Public").glob("*.h"):
    text = path.read_text(encoding="utf-8")
    if "UCLASS" not in text and "USTRUCT" not in text and "UENUM" not in text:
        continue
    include_lines = [line.strip() for line in text.splitlines() if line.strip().startswith("#include")]
    generated = [line for line in include_lines if ".generated.h\"" in line]
    if generated and include_lines[-1] != generated[-1]:
        print(f"S05 structural verification: FAIL — generated.h is not the final include in {path.name}")
        sys.exit(1)

# Server RPC declarations in OCCharacter must have _Implementation definitions.
character_h = (root / "Source/OsterConflict/Public/OCCharacter.h").read_text(encoding="utf-8")
character_cpp = (root / "Source/OsterConflict/Private/OCCharacter.cpp").read_text(encoding="utf-8")
rpc_names = re.findall(r'UFUNCTION\(Server,[^)]*\)\s*\n\s*void\s+(\w+)\s*\(', character_h)
for rpc in rpc_names:
    marker = f"AOCCharacter::{rpc}_Implementation("
    if marker not in character_cpp:
        print(f"S05 structural verification: FAIL — missing RPC implementation for {rpc}")
        sys.exit(1)

# S05 must not contain build artifacts.
for forbidden in ("Binaries", "Intermediate", "Saved", ".vs"):
    if (root / forbidden).exists():
        print(f"S05 structural verification: FAIL — forbidden build artifact folder: {forbidden}")
        sys.exit(1)

# Catch the malformed duplicate block opener that existed in an earlier HUD snapshot.
hud = (root / "Source/OsterConflict/Private/OCHUD.cpp").read_text(encoding="utf-8")
if re.search(r'if\s*\(!State\)\s*\{\s*\{', hud, flags=re.S):
    print("S05 structural verification: FAIL — malformed duplicate HUD brace")
    sys.exit(1)

print("S05 structural verification: PASS")
print(f"Checked {len(required)} required files, {len(checks)} S05 markers and {len(rpc_names)} Character server RPCs.")
