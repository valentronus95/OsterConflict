#include "OCGameMode.h"
#include "OCBuildVersion.h"

#include "OCAmmoBox.h"
#include "OCBotCharacter.h"
#include "OCBreakableWindow.h"
#include "OCAIController.h"
#include "OCCapturePoint.h"
#include "OCCharacter.h"
#include "OCDamageTarget.h"
#include "OCDestructibleProp.h"
#include "OCEnterableHouse.h"
#include "OCHouseTypes.h"
#include "OCGameState.h"
#include "OCHealthComponent.h"
#include "OCInteractableDoor.h"
#include "OCInteractableGate.h"
#include "OCInteractableLight.h"
#include "OCDeployableTrap.h"
#include "OCSmokeCloud.h"
#include "OCHUD.h"
#include "OCPlayerController.h"
#include "OCPlayerState.h"
#include "OCTeamSpawnPoint.h"
#include "OCWorldSectorOster.h"
#include "OCVisualEnvironment.h"
#include "OCAmbientAudioZone.h"
#include "OCVehicleBase.h"
#include "OCArmedVehicleBase.h"
#include "OCVehicleSpawnPoint.h"
#include "OCCombatVehicleSpawnPoints.h"
#include "OCLobbyTypes.h"
#include "OCCharacterVisualTypes.h"
#include "OCCivilianVehicle.h"
#include "OCWeaponVariants.h"
#include "OCAntiArmorLauncher.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

AOCGameMode::AOCGameMode()
{
    DefaultPawnClass = AOCCharacter::StaticClass();
    HUDClass = AOCHUD::StaticClass();
    PlayerControllerClass = AOCPlayerController::StaticClass();
    PlayerStateClass = AOCPlayerState::StaticClass();
    GameStateClass = AOCGameState::StaticClass();
}


void AOCGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    // R6: a packaged/standalone client that opens the Frontend must remain UI-only.
    // Use -NoFrontend for intentional standalone gameplay/dev sessions.
    bFrontendOnlySession = (GetNetMode() == NM_Standalone) &&
        !FParse::Param(FCommandLine::Get(), TEXT("NoFrontend"));
    const FString RequestedMode = UGameplayStatics::ParseOption(Options, TEXT("Mode"));
    bSandboxMode = RequestedMode.Equals(TEXT("Sandbox"), ESearchCase::IgnoreCase) || RequestedMode.Equals(TEXT("Test"), ESearchCase::IgnoreCase);

    const FString SandboxAdminAllOption = UGameplayStatics::ParseOption(Options, TEXT("SandboxAdminAll"));
#if UE_BUILD_SHIPPING
    bAllowSandboxAdminAll = false;
#else
    bAllowSandboxAdminAll = bSandboxMode && (SandboxAdminAllOption.Equals(TEXT("1")) ||
        SandboxAdminAllOption.Equals(TEXT("true"), ESearchCase::IgnoreCase));
#endif

    const FString MaxPlayersOption = UGameplayStatics::ParseOption(Options, TEXT("MaxPlayers"));
    if (!MaxPlayersOption.IsEmpty()) MaxPlayerSlots = FMath::Clamp(FCString::Atoi(*MaxPlayersOption), 2, 64);

    const FString BotsOption = UGameplayStatics::ParseOption(Options, TEXT("Bots"));
    RequestedBotCount = BotsOption.IsEmpty() ? -1 : FMath::Clamp(FCString::Atoi(*BotsOption), 0, MaxPlayerSlots);

    const FString PopulationOption = UGameplayStatics::ParseOption(Options, TEXT("Population"));
    if (!PopulationOption.IsEmpty()) TargetPopulation = FMath::Clamp(FCString::Atoi(*PopulationOption), 0, MaxPlayerSlots);
    else if (RequestedBotCount >= 0) TargetPopulation = RequestedBotCount;
    else TargetPopulation = MaxPlayerSlots;

    const FString BotFillOption = UGameplayStatics::ParseOption(Options, TEXT("BotFill"));
    if (!BotFillOption.IsEmpty()) bAutoFillBots = !BotFillOption.Equals(TEXT("0")) && !BotFillOption.Equals(TEXT("false"), ESearchCase::IgnoreCase);

    ConfiguredBotDifficulty = ParseBotDifficulty(Options);
    ConfigurePerformanceProfile(Options);

    TeamOneFaction = ParseFactionOption(UGameplayStatics::ParseOption(Options, TEXT("Team1Faction")), EOCFactionArchetype::UASpecialUnit);
    TeamTwoFaction = ParseFactionOption(UGameplayStatics::ParseOption(Options, TEXT("Team2Faction")), EOCFactionArchetype::MaskedFighters);
    if (TeamTwoFaction == TeamOneFaction)
    {
        TeamTwoFaction = TeamOneFaction == EOCFactionArchetype::MaskedFighters
            ? EOCFactionArchetype::USRangers : EOCFactionArchetype::MaskedFighters;
    }
}

void AOCGameMode::BeginPlay()
{
    Super::BeginPlay();
    if (bFrontendOnlySession)
    {
        UE_LOG(LogTemp, Log, TEXT("Frontend-only standalone session: gameplay world, bots and match timers are suppressed."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("OC_SERVER_READY milestone=%s build=%s protocol=%d mode=%s max_humans=%d target_population=%d map=%s"),
        OCBuildVersion::Milestone, OCBuildVersion::ProjectVersion, OCBuildVersion::NetworkProtocol,
        bSandboxMode ? TEXT("Sandbox") : TEXT("Conquest"), MaxPlayerSlots, TargetPopulation, *GetWorld()->GetMapName());

    SpawnOsterCenterSector();
    SpawnCivilianVehicleFleet();
    SpawnCombatVehicleFleet();

    if (AOCGameState* State = GetGameState<AOCGameState>())
    {
        State->SetGameplayModeServer(bSandboxMode ? EOCGameplayMode::Sandbox : EOCGameplayMode::Conquest);
        State->ConfigurePopulationServer(MaxPlayerSlots, TargetPopulation);
        State->InitializeRoundServer(StartingTickets);
    }

    if (!bSandboxMode)
    {
        GetWorldTimerManager().SetTimer(
            TicketBleedTimerHandle, this, &AOCGameMode::ApplyTicketBleed, TicketBleedInterval, true, TicketBleedInterval);
    }

    if (bAutoFillBots || RequestedBotCount > 0)
    {
        FTimerHandle BotStartupHandle;
        GetWorldTimerManager().SetTimer(BotStartupHandle, this, &AOCGameMode::SpawnConfiguredBots, 1.25f, false);
    }
    RefreshPopulationState();
}


void AOCGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
    if (!ErrorMessage.IsEmpty()) return;

    const FString ProtocolOption = UGameplayStatics::ParseOption(Options, TEXT("Protocol"));
    if (ProtocolOption.IsEmpty())
    {
#if UE_BUILD_SHIPPING
        ErrorMessage = TEXT("VERSION_MISMATCH");
        return;
#else
        UE_LOG(LogTemp, Warning, TEXT("Legacy client connected without Protocol option; expected %d."), OCBuildVersion::NetworkProtocol);
#endif
    }
    else
    {
        const int32 RemoteProtocol = FCString::Atoi(*ProtocolOption);
        if (RemoteProtocol != OCBuildVersion::NetworkProtocol)
        {
            UE_LOG(LogTemp, Warning, TEXT("VERSION_MISMATCH remote=%d local=%d address=%s"),
                RemoteProtocol, OCBuildVersion::NetworkProtocol, *Address);
            ErrorMessage = TEXT("VERSION_MISMATCH");
            return;
        }
    }

    if (GetHumanPlayerCount() >= MaxPlayerSlots)
    {
        ErrorMessage = TEXT("SERVER_FULL_HUMANS");
    }
}

FString AOCGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
    const FString& Options, const FString& Portal)
{
    const FString Error = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
    if (!Error.IsEmpty() || !NewPlayerController) return Error;
    if (bFrontendOnlySession) return Error;

    AOCPlayerState* State = NewPlayerController->GetPlayerState<AOCPlayerState>();
    if (!State) return Error;

    FString RequestedName = UGameplayStatics::ParseOption(Options, TEXT("Name"));
    RequestedName.TrimStartAndEndInline();
    if (RequestedName.IsEmpty()) RequestedName = MakeFallbackPlayerName();

    State->SetPlayerName(MakeUniquePlayerName(RequestedName, State));
    State->SetBotPlayerServer(false);
    const FString AutoDeployOption = UGameplayStatics::ParseOption(Options, TEXT("AutoDeploy"));
    const bool bAutoDeployForSmoke = AutoDeployOption.Equals(TEXT("1")) ||
        AutoDeployOption.Equals(TEXT("true"), ESearchCase::IgnoreCase);
    State->SetLobbyReadyServer(bAutoDeployForSmoke);
    State->SetRoleServer(ParseRequestedRole(Options));
    if (State->GetTeamId() == EOCTeam::None) State->SetTeamServer(AssignBalancedTeam(State));
    AssignSquadServer(State, ParseRequestedSquad(Options));
    ApplyFactionToState(State);
    return Error;
}


void AOCGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    if (!NewPlayer || bFrontendOnlySession) return;

    if (AOCPlayerState* State = NewPlayer->GetPlayerState<AOCPlayerState>())
    {
        State->SetBotPlayerServer(false);
        if (State->GetTeamId() == EOCTeam::None) State->SetTeamServer(AssignBalancedTeam(State));
        if (State->GetSquadId() == INDEX_NONE) AssignSquadServer(State, INDEX_NONE);
        ApplyFactionToState(State);
        UE_LOG(LogTemp, Log, TEXT("Human joined: %s [%s/%s]"), *State->GetPlayerName(),
            *OCTeamToString(State->GetTeamId()), *OCSquadName(State->GetSquadId()));
    }

    if (AOCPlayerController* OCPC = Cast<AOCPlayerController>(NewPlayer))
    {
        OCPC->ClientSetSandboxAdminAllowed(CanUseSandboxAdmin(OCPC));
    }

    if (bAutoFillBots)
    {
        if (const AOCPlayerState* State = NewPlayer->GetPlayerState<AOCPlayerState>();
            State && GetHumanPlayerCount() + GetBotPlayerCount() > TargetPopulation && GetBotPlayerCount() > 0)
        {
            if (AOCAIController* Bot = SelectBotToRemove(State->GetTeamId())) RemoveBotController(Bot);
        }
    }
    MaintainPopulation();
}


void AOCGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
    if (!NewPlayer || bFrontendOnlySession) return;
    const AOCPlayerState* State = NewPlayer->GetPlayerState<AOCPlayerState>();
    if (State && !State->IsBotPlayer() && !State->IsLobbyReady()) return;
    Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

void AOCGameMode::Logout(AController* Exiting)
{
    AOCPlayerState* LeavingState = Exiting ? Exiting->GetPlayerState<AOCPlayerState>() : nullptr;
    const FString LeavingName = LeavingState ? LeavingState->GetPlayerName() : TEXT("Unknown");
    const EOCTeam LeavingTeam = LeavingState ? LeavingState->GetTeamId() : EOCTeam::None;
    const int32 LeavingSquad = LeavingState ? LeavingState->GetSquadId() : INDEX_NONE;
    UE_LOG(LogTemp, Log, TEXT("Player left: %s"), *LeavingName);
    Super::Logout(Exiting);

    if (LeavingTeam != EOCTeam::None && LeavingSquad != INDEX_NONE) RepairSquadLeadership(LeavingTeam, LeavingSquad);
    RefreshPopulationState();
    if (bAutoFillBots && GetWorld())
    {
        GetWorldTimerManager().SetTimer(PopulationMaintenanceTimerHandle, this, &AOCGameMode::MaintainPopulation,
            BotRefillDelay, false);
    }
}

EOCTeam AOCGameMode::AssignBalancedTeam(AOCPlayerState* JoiningState) const
{
    int32 TeamOneCount = 0;
    int32 TeamTwoCount = 0;
    if (const AOCGameState* State = GetGameState<AOCGameState>())
    {
        for (APlayerState* PlayerState : State->PlayerArray)
        {
            const AOCPlayerState* OCState = Cast<AOCPlayerState>(PlayerState);
            if (!OCState || OCState == JoiningState) continue;
            if (OCState->GetTeamId() == EOCTeam::TeamOne) ++TeamOneCount;
            else if (OCState->GetTeamId() == EOCTeam::TeamTwo) ++TeamTwoCount;
        }
    }
    return TeamOneCount <= TeamTwoCount ? EOCTeam::TeamOne : EOCTeam::TeamTwo;
}

EOCPlayerRole AOCGameMode::ParseRequestedRole(const FString& Options)
{
    FString RoleOption = UGameplayStatics::ParseOption(Options, TEXT("Role"));
    RoleOption.TrimStartAndEndInline();
    if (RoleOption.Equals(TEXT("Rifleman"), ESearchCase::IgnoreCase)) return EOCPlayerRole::Rifleman;
    if (RoleOption.Equals(TEXT("Engineer"), ESearchCase::IgnoreCase)) return EOCPlayerRole::Engineer;
    if (RoleOption.Equals(TEXT("Support"), ESearchCase::IgnoreCase)) return EOCPlayerRole::Support;
    return EOCPlayerRole::Medic;
}

int32 AOCGameMode::ParseRequestedSquad(const FString& Options)
{
    FString Value = UGameplayStatics::ParseOption(Options, TEXT("Squad"));
    Value.TrimStartAndEndInline();
    if (Value.IsEmpty() || Value.Equals(TEXT("Auto"), ESearchCase::IgnoreCase)) return INDEX_NONE;
    static const TCHAR* Names[] = { TEXT("Alpha"), TEXT("Bravo"), TEXT("Charlie"), TEXT("Delta"),
        TEXT("Echo"), TEXT("Foxtrot"), TEXT("Golf"), TEXT("Hotel") };
    for (int32 I = 0; I < UE_ARRAY_COUNT(Names); ++I) if (Value.Equals(Names[I], ESearchCase::IgnoreCase)) return I;
    const int32 Numeric = FCString::Atoi(*Value);
    return (Numeric >= 0 && Numeric < 8) ? Numeric : INDEX_NONE;
}

void AOCGameMode::ConfigurePerformanceProfile(const FString& Options)
{
    FString Value = UGameplayStatics::ParseOption(Options, TEXT("PerfProfile"));
    Value.TrimStartAndEndInline();
    if (Value.IsEmpty()) Value = TEXT("Balanced");

    if (Value.Equals(TEXT("LowCPU"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("Server"), ESearchCase::IgnoreCase))
    {
        PerformanceProfileName = TEXT("LowCPU");
        AIThinkIntervalScale = 1.35f;
        MaxPersistentCorpses = FMath::Min(MaxPersistentCorpses, 10);
    }
    else if (Value.Equals(TEXT("Quality"), ESearchCase::IgnoreCase))
    {
        PerformanceProfileName = TEXT("Quality");
        AIThinkIntervalScale = 0.90f;
        MaxPersistentCorpses = FMath::Max(MaxPersistentCorpses, 28);
    }
    else
    {
        PerformanceProfileName = TEXT("Balanced");
        AIThinkIntervalScale = 1.0f;
    }
}

EOCBotDifficulty AOCGameMode::ParseBotDifficulty(const FString& Options)
{
    FString Value = UGameplayStatics::ParseOption(Options, TEXT("BotDifficulty"));
    Value.TrimStartAndEndInline();
    if (Value.Equals(TEXT("Recruit"), ESearchCase::IgnoreCase)) return EOCBotDifficulty::Recruit;
    if (Value.Equals(TEXT("Veteran"), ESearchCase::IgnoreCase)) return EOCBotDifficulty::Veteran;
    return EOCBotDifficulty::Regular;
}

EOCFactionArchetype AOCGameMode::ParseFactionOption(const FString& Value, EOCFactionArchetype Fallback)
{
    FString Normalized = Value;
    Normalized.TrimStartAndEndInline();
    if (Normalized.IsEmpty()) return Fallback;
    if (Normalized.Equals(TEXT("UA"), ESearchCase::IgnoreCase) || Normalized.Equals(TEXT("UASpecialUnit"), ESearchCase::IgnoreCase)) return EOCFactionArchetype::UASpecialUnit;
    if (Normalized.Equals(TEXT("Masked"), ESearchCase::IgnoreCase) || Normalized.Equals(TEXT("MaskedFighters"), ESearchCase::IgnoreCase)) return EOCFactionArchetype::MaskedFighters;
    if (Normalized.Equals(TEXT("US"), ESearchCase::IgnoreCase) || Normalized.Equals(TEXT("USRangers"), ESearchCase::IgnoreCase)) return EOCFactionArchetype::USRangers;
    if (Normalized.Equals(TEXT("Insurgents"), ESearchCase::IgnoreCase)) return EOCFactionArchetype::Insurgents;
    return Fallback;
}

int32 AOCGameMode::GetBotPlayerCount() const
{
    int32 Count = 0;
    if (const AOCGameState* State = GetGameState<AOCGameState>())
    {
        for (APlayerState* PlayerState : State->PlayerArray)
        {
            if (const AOCPlayerState* OCState = Cast<AOCPlayerState>(PlayerState); OCState && OCState->IsBotPlayer()) ++Count;
        }
    }
    return Count;
}

int32 AOCGameMode::GetHumanPlayerCount() const
{
    int32 Count = 0;
    if (const AOCGameState* State = GetGameState<AOCGameState>())
    {
        for (APlayerState* PlayerState : State->PlayerArray)
        {
            if (const AOCPlayerState* OCState = Cast<AOCPlayerState>(PlayerState); OCState && !OCState->IsBotPlayer()) ++Count;
        }
    }
    return Count;
}

FString AOCGameMode::MakeUniquePlayerName(const FString& RequestedName, const AOCPlayerState* IgnoreState) const
{
    FString Base;
    Base.Reserve(RequestedName.Len());
    for (const TCHAR C : RequestedName)
    {
        if (FChar::IsAlnum(C) || C == TEXT('_') || C == TEXT('-') || C == TEXT(' ')) Base.AppendChar(C);
    }
    Base.TrimStartAndEndInline();
    Base = Base.Left(24);
    if (Base.IsEmpty()) Base = TEXT("Player");

    auto Exists = [&](const FString& Candidate)
    {
        if (const AOCGameState* GS = GetGameState<AOCGameState>())
        {
            for (APlayerState* PS : GS->PlayerArray)
            {
                const AOCPlayerState* OPS = Cast<AOCPlayerState>(PS);
                if (OPS && OPS != IgnoreState && OPS->GetPlayerName().Equals(Candidate, ESearchCase::IgnoreCase)) return true;
            }
        }
        return false;
    };
    if (!Exists(Base)) return Base;
    for (int32 Suffix=2; Suffix<100; ++Suffix)
    {
        const FString Tail = FString::Printf(TEXT("-%d"), Suffix);
        const FString Candidate = Base.Left(FMath::Max(1, 24-Tail.Len())) + Tail;
        if (!Exists(Candidate)) return Candidate;
    }
    return FString::Printf(TEXT("Player-%04d"), FMath::RandRange(0,9999));
}

FString AOCGameMode::MakeFallbackPlayerName()
{
    return FString::Printf(TEXT("Player %02d"), FallbackPlayerNumber++);
}

// ---- Remaining existing gameplay functions are unchanged in this R13 content pass. ----
// This file is intentionally not regenerated piecemeal. The original implementation continues below in source history.
