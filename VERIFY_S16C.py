from pathlib import Path

ROOT = Path(__file__).parent / "OsterConflict"
required = [
    ROOT / "Source/OsterConflict/Public/OCCharacterVisualTypes.h",
    ROOT / "Source/OsterConflict/Public/OCCharacterVisualProfile.h",
    ROOT / "Source/OsterConflict/Private/OCCharacterVisualProfile.cpp",
    ROOT / "Source/OsterConflict/Public/OCCharacterVisualComponent.h",
    ROOT / "Source/OsterConflict/Private/OCCharacterVisualComponent.cpp",
    ROOT / "Source/OsterConflict/Public/OCCharacterAnimInstance.h",
    ROOT / "Source/OsterConflict/Private/OCCharacterAnimInstance.cpp",
    ROOT / "Docs/SESSION_16C_README_UA.md",
    ROOT / "Docs/CHARACTER_ART_ANIMATION_ARCHITECTURE_S16C.md",
    ROOT / "Docs/CHARACTER_ASSET_MANIFEST_S16C.md",
    ROOT / "Docs/S16C_TEST_MATRIX.md",
]
for p in required:
    assert p.exists(), f"missing {p}"

files = {
    "types": ROOT / "Source/OsterConflict/Public/OCCharacterVisualTypes.h",
    "profile": ROOT / "Source/OsterConflict/Public/OCCharacterVisualProfile.h",
    "visual_h": ROOT / "Source/OsterConflict/Public/OCCharacterVisualComponent.h",
    "visual_c": ROOT / "Source/OsterConflict/Private/OCCharacterVisualComponent.cpp",
    "anim_h": ROOT / "Source/OsterConflict/Public/OCCharacterAnimInstance.h",
    "anim_c": ROOT / "Source/OsterConflict/Private/OCCharacterAnimInstance.cpp",
    "char_h": ROOT / "Source/OsterConflict/Public/OCCharacter.h",
    "char_c": ROOT / "Source/OsterConflict/Private/OCCharacter.cpp",
    "ps_h": ROOT / "Source/OsterConflict/Public/OCPlayerState.h",
    "ps_c": ROOT / "Source/OsterConflict/Private/OCPlayerState.cpp",
    "gm_h": ROOT / "Source/OsterConflict/Public/OCGameMode.h",
    "gm_c": ROOT / "Source/OsterConflict/Private/OCGameMode.cpp",
    "hud": ROOT / "Source/OsterConflict/Private/OCHUD.cpp",
}
text = "\n".join(p.read_text(encoding="utf-8") for p in files.values())
markers = [
    "EOCFactionArchetype", "UASpecialUnit", "MaskedFighters", "USRangers", "Insurgents",
    "FOCCharacterAppearance", "UOCCharacterVisualProfile", "ThirdPersonBodyMesh", "FirstPersonArmsMesh",
    "UOCCharacterVisualComponent", "BuildSourceOnlyProxy", "FirstPersonProxyParts", "DebugFactionColor",
    "UOCCharacterAnimInstance", "DirectionDegrees", "bReloading", "AimPitch", "bVehicleGunner",
    "FactionArchetype", "AppearanceSeed", "SetFactionServer", "Team1Faction", "Team2Faction",
    "ParseFactionOption", "ApplyFactionToState", "BroadcastActionServer", "EOCCharacterActionEvent::Fire",
    "EOCCharacterActionEvent::ReloadStart", "EOCCharacterActionEvent::ReviveStart", "EOCCharacterActionEvent::Death",
    "FACTION       %s",
]
for m in markers:
    assert m in text, f"missing marker: {m}"

# UHT include order: generated.h must be the final local include in headers that have it.
for p in (ROOT / "Source/OsterConflict/Public").glob("*.h"):
    lines = p.read_text(encoding="utf-8").splitlines()
    generated = [i for i, line in enumerate(lines) if "generated.h\"" in line]
    if generated:
        gi = generated[0]
        later_includes = [line for line in lines[gi+1:] if line.strip().startswith("#include")]
        assert not later_includes, f"include after generated.h in {p.name}: {later_includes}"

print(f"S16C structural verification: PASS\nChecked {len(required)} required files and {len(markers)} S16C markers.")
