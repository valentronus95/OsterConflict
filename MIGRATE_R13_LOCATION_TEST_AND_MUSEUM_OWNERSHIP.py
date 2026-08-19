from pathlib import Path

ROOT = Path(__file__).resolve().parent
SRC = ROOT / "OsterConflict" / "Source" / "OsterConflict"


def patch(path: Path, old: str, new: str, label: str) -> None:
    text = path.read_text(encoding="utf-8")
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"PATCH FAIL {label}: expected exact block once, found {count}")
    path.write_text(text.replace(old, new, 1), encoding="utf-8")
    print(f"PATCH PASS: {label}")


# 1) Human-facing local test is now explicitly a clean location-inspection session.
frontend = SRC / "Private" / "OCR13FrontendMenuSubsystem.cpp"
patch(
    frontend,
    'StatusText->SetText(NSLOCTEXT("OCR13Frontend", "LocalStatus", "Conquest • 15 ботів • локальний сервер"));',
    'StatusText->SetText(NSLOCTEXT("OCR13Frontend", "LocalStatus", "Огляд локації • без ботів і автотранспорту"));',
    "frontend local status",
)
patch(
    frontend,
    'PC->ConsoleCommand(TEXT("open /Game/Maps/OsterConflict_Runtime?listen?Mode=Conquest?Bots=15?Population=16?BotFill=1?MaxPlayers=16?R13Gameplay=1"));',
    'PC->ConsoleCommand(TEXT("open /Game/Maps/OsterConflict_Runtime?listen?Mode=Conquest?Bots=0?Population=1?BotFill=0?MaxPlayers=16?R13Gameplay=1?LocationTest=1"));',
    "frontend location-test travel",
)

# 2) GameMode owns the test-mode contract instead of relying on spawned actors to clean themselves up later.
gm_h = SRC / "Public" / "OCGameMode.h"
patch(
    gm_h,
    '    bool IsFrontendOnlySession() const { return bFrontendOnlySession; }\n',
    '    bool IsFrontendOnlySession() const { return bFrontendOnlySession; }\n'
    '    /** Location-first inspection session: no bots or vehicle fleets, used while rebuilding Oster. */\n'
    '    bool IsLocationTestMode() const { return bLocationTestMode; }\n',
    "GameMode location-test accessor",
)
patch(
    gm_h,
    '    bool bFrontendOnlySession = false;\n',
    '    bool bFrontendOnlySession = false;\n'
    '    /** Temporary location-first playtest profile requested by ?LocationTest=1. */\n'
    '    bool bLocationTestMode = false;\n',
    "GameMode location-test state",
)

gm = SRC / "Private" / "OCGameMode.cpp"
patch(
    gm,
    '    const FString RequestedMode = UGameplayStatics::ParseOption(Options, TEXT("Mode"));\n'
    '    bSandboxMode = RequestedMode.Equals(TEXT("Sandbox"), ESearchCase::IgnoreCase) || RequestedMode.Equals(TEXT("Test"), ESearchCase::IgnoreCase);\n',
    '    const FString RequestedMode = UGameplayStatics::ParseOption(Options, TEXT("Mode"));\n'
    '    bSandboxMode = RequestedMode.Equals(TEXT("Sandbox"), ESearchCase::IgnoreCase) || RequestedMode.Equals(TEXT("Test"), ESearchCase::IgnoreCase);\n'
    '    const FString LocationTestOption = UGameplayStatics::ParseOption(Options, TEXT("LocationTest"));\n'
    '    bLocationTestMode = LocationTestOption.Equals(TEXT("1")) ||\n'
    '        LocationTestOption.Equals(TEXT("true"), ESearchCase::IgnoreCase);\n',
    "GameMode parse LocationTest",
)
patch(
    gm,
    '    SpawnOsterCenterSector();\n'
    '    SpawnCivilianVehicleFleet();\n'
    '    SpawnCombatVehicleFleet();\n',
    '    SpawnOsterCenterSector();\n'
    '    if (!bLocationTestMode)\n'
    '    {\n'
    '        SpawnCivilianVehicleFleet();\n'
    '        SpawnCombatVehicleFleet();\n'
    '    }\n'
    '    else\n'
    '    {\n'
    '        RequestedBotCount = 0;\n'
    '        TargetPopulation = 1;\n'
    '        bAutoFillBots = false;\n'
    '        UE_LOG(LogTemp, Display, TEXT("R13 location test: bot population and vehicle fleets suppressed for map inspection."));\n'
    '    }\n',
    "GameMode suppress vehicles and bots in location test",
)

# 3) R13.7 is the sole museum owner. R13.6 remains as the stadium photo-fidelity pass only.
legacy_photo = SRC / "Private" / "OCR13MuseumStadiumPhotoFidelitySubsystem.cpp"
patch(
    legacy_photo,
    'void UOCR13MuseumStadiumPhotoFidelitySubsystem::ApplyPhotoFidelity(UWorld& World)\n'
    '{\n'
    '    SuppressLegacyMuseumPresentation(World);\n'
    '    SuppressLegacyStadiumPresentation(World);\n'
    '    BuildMuseum(World);\n'
    '    BuildStadium(World);\n'
    '}\n',
    'void UOCR13MuseumStadiumPhotoFidelitySubsystem::ApplyPhotoFidelity(UWorld& World)\n'
    '{\n'
    '    // R13.7 owns MuseumAnchor exclusively. This legacy pass now touches only the adjacent stadium,\n'
    '    // preventing an obsolete museum from being built and then replaced less than a second later.\n'
    '    SuppressLegacyStadiumPresentation(World);\n'
    '    BuildStadium(World);\n'
    '}\n',
    "R13.6 stadium-only ownership",
)

# 4) The source branch intentionally has no committed menu music asset. Keep the menu silent rather than embedding
# a dangling dynamic cook path. Import tooling can re-enable music once a real asset is committed.
audio = SRC / "Private" / "OCR13FrontendAudioSubsystem.cpp"
old_audio = '''    USoundBase* MenuSound = LoadObject<USoundBase>(nullptr,
        TEXT("/Game/R13/Audio/menu_ambient.menu_ambient"));
    if (!MenuSound)
    {
        if (!bWarnedMissingMusic)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("R13 frontend audio: /Game/R13/Audio/menu_ambient is not imported yet; menu remains silent instead of leaking combat audio."));
            bWarnedMissingMusic = true;
        }
        return;
    }
'''
new_audio = '''    // No menu-music .uasset is committed on the source/location branch yet. Do not keep a dangling /Game path:
    // the frontend stays intentionally silent while combat remains isolated behind explicit match start.
    USoundBase* MenuSound = nullptr;
    if (!MenuSound)
    {
        if (!bWarnedMissingMusic)
        {
            UE_LOG(LogTemp, Display,
                TEXT("R13 frontend audio: source location branch has no bundled menu music; frontend remains intentionally silent."));
            bWarnedMissingMusic = true;
        }
        return;
    }
'''
patch(audio, old_audio, new_audio, "remove dangling menu-audio cook path")

# 5) Keep regression verifiers aligned with the intentional location-first test contract.
r11 = ROOT / "VERIFY_R11_VISUAL_FOUNDATION.py"
patch(
    r11,
    "req('?listen?Mode=Conquest?Bots=15?Population=16?BotFill=1?MaxPlayers=16?R13Gameplay=1' in frontend and\n"
    "    'StartLocalGameplay()' in frontend,\n"
    "    'frontend START action owns explicit local listen-server travel')\n",
    "req('?listen?Mode=Conquest?Bots=0?Population=1?BotFill=0?MaxPlayers=16?R13Gameplay=1?LocationTest=1' in frontend and\n"
    "    'StartLocalGameplay()' in frontend,\n"
    "    'frontend START action owns explicit bot-free location-test listen-server travel')\n",
    "R11 location-test travel verifier",
)

audio_verify = ROOT / "VERIFY_R13_6_FRONTEND_AUDIO_LAYOUT.py"
patch(
    audio_verify,
    '    "/Game/R13/Audio/menu_ambient.menu_ambient",\n',
    '    "source location branch has no bundled menu music",\n',
    "frontend audio verifier optional music marker",
)

print("R13 location-test + museum ownership migration complete")
# Triggered after workflow definition exists on branch.
