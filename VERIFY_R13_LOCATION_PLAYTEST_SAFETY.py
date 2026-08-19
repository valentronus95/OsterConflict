from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict/Source/OsterConflict"

compact = (SRC / "Private/OCR13CompactOsterSubsystem.cpp").read_text(encoding="utf-8")
respawn_h = (SRC / "Public/OCR13MuseumRespawnSubsystem.h").read_text(encoding="utf-8")
respawn_cpp = (SRC / "Private/OCR13MuseumRespawnSubsystem.cpp").read_text(encoding="utf-8")
frontend = (SRC / "Private/OCR13FrontendMenuSubsystem.cpp").read_text(encoding="utf-8")
game_mode_h = (SRC / "Public/OCGameMode.h").read_text(encoding="utf-8")
game_mode = (SRC / "Private/OCGameMode.cpp").read_text(encoding="utf-8")
legacy_museum = (SRC / "Private/OCR13MuseumStadiumPhotoFidelitySubsystem.cpp").read_text(encoding="utf-8")
final_museum = (SRC / "Private/OCR137MuseumPhotoModelSubsystem.cpp").read_text(encoding="utf-8")

apply_start = legacy_museum.find("void UOCR13MuseumStadiumPhotoFidelitySubsystem::ApplyPhotoFidelity")
apply_end = legacy_museum.find("void UOCR13MuseumStadiumPhotoFidelitySubsystem::SuppressLegacyMuseumPresentation", apply_start)
legacy_apply = legacy_museum[apply_start:apply_end] if apply_start >= 0 and apply_end > apply_start else ""

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
    "frontend local test is explicitly bot-free": "?Bots=0?Population=1?BotFill=0?MaxPlayers=16?R13Gameplay=1?LocationTest=1" in frontend,
    "frontend identifies clean location inspection": "Огляд локації • без ботів і автотранспорту" in frontend,
    "GameMode exposes location-test state": "bool IsLocationTestMode() const { return bLocationTestMode; }" in game_mode_h,
    "GameMode parses LocationTest option": "ParseOption(Options, TEXT(\"LocationTest\"))" in game_mode,
    "location test disables requested bots": "RequestedBotCount = 0;" in game_mode,
    "location test disables bot fill": "bAutoFillBots = false;" in game_mode,
    "location test keeps one human target": "TargetPopulation = 1;" in game_mode,
    "vehicle fleets are gated by location test": "if (!bLocationTestMode)" in game_mode and "SpawnCivilianVehicleFleet();" in game_mode and "SpawnCombatVehicleFleet();" in game_mode,
    "legacy R13.6 photo pass still builds stadium": "BuildStadium(World);" in legacy_apply,
    "legacy R13.6 photo pass no longer builds museum": bool(legacy_apply) and "BuildMuseum(World);" not in legacy_apply,
    "R13.7 final museum owns canonical anchor": "AOCWorldSectorOster::MuseumAnchor()" in final_museum,
    "R13.7 final museum has unique runtime tag": 'Model->Tags.Add(TEXT("R137_MuseumPhotoModel"));' in final_museum,
}

failed = [name for name, ok in checks.items() if not ok]
for name, ok in checks.items():
    print(("PASS" if ok else "FAIL") + ": " + name)

if failed:
    raise SystemExit("R13 location playtest safety verification failed: " + ", ".join(failed))

print("PASS: location inspection is bot/vehicle-free, retained roads have solid support, humans respawn beside the museum, and R13.7 exclusively owns the museum presentation.")
