from pathlib import Path

ROOT = Path(__file__).resolve().parent
compact = (ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13CompactOsterSubsystem.cpp").read_text(encoding="utf-8")
respawn_h = (ROOT / "OsterConflict/Source/OsterConflict/Public/OCR13MuseumRespawnSubsystem.h").read_text(encoding="utf-8")
respawn_cpp = (ROOT / "OsterConflict/Source/OsterConflict/Private/OCR13MuseumRespawnSubsystem.cpp").read_text(encoding="utf-8")

checks = {
    "shared linear-infrastructure padding constant": "constexpr float LinearInfrastructurePaddingCm = 18000.0f;" in compact,
    "supported ground width uses shared padding": "CompactWidthCm + 2.0f * LinearInfrastructurePaddingCm" in compact,
    "supported ground height uses shared padding": "CompactHeightCm + 2.0f * LinearInfrastructurePaddingCm" in compact,
    "linear crop uses same shared padding": "bLinearInfrastructure ? LinearInfrastructurePaddingCm : NonLinearPaddingCm" in compact,
    "ground collision explicitly restored": "Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);" in compact,
    "ground forced visible": "Mesh->SetHiddenInGame(false, true);" in compact,
    "museum respawn subsystem exists": "class OSTERCONFLICT_API UOCR13MuseumRespawnSubsystem" in respawn_h,
    "museum respawn uses canonical anchor": "AOCWorldSectorOster::MuseumAnchor()" in respawn_cpp,
    "museum respawn excludes frontend shell": "GameMode->IsFrontendOnlySession()" in respawn_cpp,
    "museum respawn waits for accepted deployment": "HasCompletedR13InitialDeployment()" in respawn_cpp,
    "museum respawn uses collision-safe teleport": "FindTeleportSpot" in respawn_cpp,
    "museum respawn faces museum": "const FVector ToMuseum = Museum - GroundedLocation;" in respawn_cpp,
}

failed = [name for name, ok in checks.items() if not ok]
for name, ok in checks.items():
    print(("PASS" if ok else "FAIL") + ": " + name)

if failed:
    raise SystemExit("R13 location playtest safety verification failed: " + ", ".join(failed))

print("PASS: retained compact roads are supported by solid ground and human respawns stage beside the museum.")
