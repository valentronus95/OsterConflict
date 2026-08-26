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
    else TargetPopulation = 0;

    const FString BotFillOption = UGameplayStatics::ParseOption(Options, TEXT("BotFill"));
    if (!BotFillOption.IsEmpty())
    {
        bAutoFillBots = !BotFillOption.Equals(TEXT("0")) && !BotFillOption.Equals(TEXT("false"), ESearchCase::IgnoreCase);
        // Explicit BotFill without Bots/Population is a real opt-in request for a filled server.
        if (bAutoFillBots && PopulationOption.IsEmpty() && RequestedBotCount < 0) TargetPopulation = MaxPlayerSlots;
    }

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

    // S18C hardening: explicit protocol mismatch is rejected before deployment.
    // Missing Protocol is tolerated only outside Shipping for compatibility with older source-only/dev launch paths.
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

    // Bots never reserve a hard human slot. A real player may join a bot-filled server as long as the human cap is not reached.
    if (GetHumanPlayerCount() >= MaxPlayerSlots)
    {
        ErrorMessage = TEXT("SERVER_FULL_HUMANS");
    }
}

FString AOCGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
    const FString& Options, const FString& Portal)
{
    const FString Error = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
    if (!Error.IsEmpty() || !NewPlayerController)
    {
        return Error;
    }
    if (bFrontendOnlySession)
    {
        return Error;
    }

    AOCPlayerState* State = NewPlayerController->GetPlayerState<AOCPlayerState>();
    if (!State)
    {
        return Error;
    }

    FString RequestedName = UGameplayStatics::ParseOption(Options, TEXT("Name"));
    RequestedName.TrimStartAndEndInline();
    if (RequestedName.IsEmpty())
    {
        RequestedName = MakeFallbackPlayerName();
    }

    State->SetPlayerName(MakeUniquePlayerName(RequestedName, State));
    State->SetBotPlayerServer(false);
    const FString AutoDeployOption = UGameplayStatics::ParseOption(Options, TEXT("AutoDeploy"));
    const bool bAutoDeployForSmoke = AutoDeployOption.Equals(TEXT("1")) ||
        AutoDeployOption.Equals(TEXT("true"), ESearchCase::IgnoreCase);
    State->SetLobbyReadyServer(bAutoDeployForSmoke);
    State->SetRoleServer(ParseRequestedRole(Options));
    if (State->GetTeamId() == EOCTeam::None)
    {
        State->SetTeamServer(AssignBalancedTeam(State));
    }
    AssignSquadServer(State, ParseRequestedSquad(Options));
    ApplyFactionToState(State);
    return Error;
}


void AOCGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    if (!NewPlayer) return;
    if (bFrontendOnlySession) return;

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

    // Human priority: replace a filler bot from the joining human's team first when possible, preserving team parity.
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
    if (State && !State->IsBotPlayer() && !State->IsLobbyReady())
    {
        // S17A: humans stay controller-only while choosing team/squad/role/spawn in Deployment UI.
        return;
    }
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
            if (!OCState || OCState == JoiningState)
            {
                continue;
            }
            if (OCState->GetTeamId() == EOCTeam::TeamOne)
            {
                ++TeamOneCount;
            }
            else if (OCState->GetTeamId() == EOCTeam::TeamTwo)
            {
                ++TeamTwoCount;
            }
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
        MaxPersistentCorpses = FMath::Min(MaxPersistentCorpses, 20);
    }
    else
    {
        PerformanceProfileName = TEXT("Balanced");
        AIThinkIntervalScale = 1.0f;
        MaxPersistentCorpses = FMath::Min(MaxPersistentCorpses, 16);
    }
    UE_LOG(LogTemp, Log, TEXT("S18A performance profile: %s | AI think x%.2f | corpse cap %d"),
        *PerformanceProfileName, AIThinkIntervalScale, MaxPersistentCorpses);
}

FString AOCGameMode::BuildPerformanceSnapshot() const
{
    UWorld* World = GetWorld();
    if (!World) return TEXT("PERF SNAPSHOT: no world");

    auto CountClass = [World](UClass* Class) -> int32
    {
        int32 Count = 0;
        for (TActorIterator<AActor> It(World, Class); It; ++It) ++Count;
        return Count;
    };

    return FString::Printf(TEXT("PERF [%s] Humans=%d Bots=%d Characters=%d Vehicles=%d Capture=%d Doors=%d Windows=%d Destructibles=%d AmbientZones=%d CorpseBudget=%d AIThinkScale=%.2f"),
        *PerformanceProfileName,
        GetHumanPlayerCount(), GetBotPlayerCount(),
        CountClass(AOCCharacter::StaticClass()), CountClass(AOCVehicleBase::StaticClass()),
        CountClass(AOCCapturePoint::StaticClass()), CountClass(AOCInteractableDoor::StaticClass()),
        CountClass(AOCBreakableWindow::StaticClass()), CountClass(AOCDestructibleProp::StaticClass()),
        CountClass(AOCAmbientAudioZone::StaticClass()), MaxPersistentCorpses, AIThinkIntervalScale);
}

EOCBotDifficulty AOCGameMode::ParseBotDifficulty(const FString& Options)
{
    FString Value = UGameplayStatics::ParseOption(Options, TEXT("BotDifficulty"));
    Value.TrimStartAndEndInline();
    if (Value.Equals(TEXT("Easy"), ESearchCase::IgnoreCase)) return EOCBotDifficulty::Easy;
    if (Value.Equals(TEXT("Hard"), ESearchCase::IgnoreCase)) return EOCBotDifficulty::Hard;
    if (Value.Equals(TEXT("Veteran"), ESearchCase::IgnoreCase)) return EOCBotDifficulty::Veteran;
    return EOCBotDifficulty::Normal;
}


EOCFactionArchetype AOCGameMode::ParseFactionOption(const FString& InValue, EOCFactionArchetype Fallback)
{
    FString Value = InValue;
    Value.TrimStartAndEndInline();
    if (Value.IsEmpty()) return Fallback;
    if (Value.Equals(TEXT("UA"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("UASpecial"), ESearchCase::IgnoreCase) ||
        Value.Equals(TEXT("UASpecialUnit"), ESearchCase::IgnoreCase)) return EOCFactionArchetype::UASpecialUnit;
    if (Value.Equals(TEXT("Masked"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("MaskedFighters"), ESearchCase::IgnoreCase))
        return EOCFactionArchetype::MaskedFighters;
    if (Value.Equals(TEXT("Rangers"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("USRangers"), ESearchCase::IgnoreCase))
        return EOCFactionArchetype::USRangers;
    if (Value.Equals(TEXT("Insurgents"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("Rebels"), ESearchCase::IgnoreCase))
        return EOCFactionArchetype::Insurgents;
    return Fallback;
}

void AOCGameMode::ApplyFactionToState(AOCPlayerState* State) const
{
    if (!State || !HasAuthority() || State->GetTeamId() == EOCTeam::None) return;
    const EOCFactionArchetype Faction = GetFactionForTeam(State->GetTeamId());
    const uint32 Hash = HashCombine(GetTypeHash(State->GetPlayerName()), static_cast<uint32>(State->GetPlayerId() + 1));
    const int32 AppearanceSeed = 1 + static_cast<int32>(Hash % 2000000000u);
    State->SetFactionServer(Faction, AppearanceSeed);
}


void AOCGameMode::SpawnConfiguredBots()
{
    if (!HasAuthority()) return;
    if (bAutoFillBots) MaintainPopulation();
    else if (RequestedBotCount > 0) SpawnDebugBots(RequestedBotCount);
}

void AOCGameMode::SpawnDebugBots(int32 Count)
{
    if (!HasAuthority()) return;
    Count = FMath::Clamp(Count, 0, FMath::Max(0, MaxPlayerSlots - GetHumanPlayerCount() - GetBotPlayerCount()));
    for (int32 I = 0; I < Count; ++I)
    {
        int32 TeamOne = 0, TeamTwo = 0;
        if (const AOCGameState* State = GetGameState<AOCGameState>())
        {
            for (APlayerState* PS : State->PlayerArray)
            {
                const AOCPlayerState* OPS = Cast<AOCPlayerState>(PS); if (!OPS) continue;
                if (OPS->GetTeamId() == EOCTeam::TeamOne) ++TeamOne; else if (OPS->GetTeamId() == EOCTeam::TeamTwo) ++TeamTwo;
            }
        }
        const EOCTeam Team = TeamOne <= TeamTwo ? EOCTeam::TeamOne : EOCTeam::TeamTwo;
        const int32 RoleCycle = (NextBotIndex - 1) % 6;
        const EOCPlayerRole BotRole = (RoleCycle == 0) ? EOCPlayerRole::Medic : (RoleCycle == 1) ? EOCPlayerRole::Engineer :
            (RoleCycle == 2) ? EOCPlayerRole::Support : EOCPlayerRole::Rifleman;
        if (!SpawnSingleBot(Team, BotRole)) break;
    }
    RefreshPopulationState();
}

bool AOCGameMode::SpawnSingleBot(EOCTeam Team, EOCPlayerRole BotRole)
{
    if (!HasAuthority() || !GetWorld() || Team==EOCTeam::None) return false;
    FActorSpawnParameters Params; Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AOCAIController* BotController=GetWorld()->SpawnActor<AOCAIController>(AOCAIController::StaticClass(),FVector::ZeroVector,FRotator::ZeroRotator,Params);
    if(!BotController)return false;
    BotController->AssignBotIdentityServer(Team, BotRole, ConfiguredBotDifficulty, NextBotIndex++);
    if (AOCPlayerState* BotState = BotController->GetPlayerState<AOCPlayerState>())
    {
        BotState->SetBotPlayerServer(true);
        AssignSquadServer(BotState, INDEX_NONE);
        ApplyFactionToState(BotState);
    }
    RestartBotController(BotController);
    if(!BotController->GetPawn()){BotController->Destroy();return false;}
    UE_LOG(LogTemp,Log,TEXT("Spawned AI bot %s [%s/%s]"),*BotController->GetPlayerState<APlayerState>()->GetPlayerName(),*OCTeamToString(Team),*OCBotDifficultyToString(ConfiguredBotDifficulty));
    return true;
}

void AOCGameMode::RestartBotController(AOCAIController* BotController)
{
    if(!BotController||!GetWorld())return;

    // A round restart can reach an AI while it is still alive or driving a vehicle.
    // Return the driver to its hidden character first, then remove that old pawn before spawning a fresh bot pawn.
    if (APawn* ExistingPawn = BotController->GetPawn())
    {
        if (AOCVehicleBase* Vehicle = Cast<AOCVehicleBase>(ExistingPawn))
        {
            Vehicle->ForceExitDriverServer();
            ExistingPawn = BotController->GetPawn();
        }
        if (ExistingPawn)
        {
            BotController->UnPossess();
            ExistingPawn->Destroy();
        }
    }

    FTransform SpawnTransform;
    if(!FindBestSpawnTransform(BotController,SpawnTransform))
    {
        const AOCPlayerState* State=BotController->GetPlayerState<AOCPlayerState>();
        const bool bTeamTwo=State&&State->GetTeamId()==EOCTeam::TeamTwo;
        SpawnTransform=FTransform(FRotator(0,bTeamTwo?180.0f:0.0f,0),bTeamTwo?FVector(2800,0,120):FVector(-2800,0,120));
    }
    FActorSpawnParameters Params; Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    AOCBotCharacter* Pawn=GetWorld()->SpawnActor<AOCBotCharacter>(AOCBotCharacter::StaticClass(),SpawnTransform,Params);
    if(Pawn)BotController->Possess(Pawn);
}

void AOCGameMode::RemoveBotController(AOCAIController* Bot)
{
    if (!HasAuthority() || !Bot) return;
    AOCPlayerState* State = Bot->GetPlayerState<AOCPlayerState>();
    const EOCTeam Team = State ? State->GetTeamId() : EOCTeam::None;
    const int32 Squad = State ? State->GetSquadId() : INDEX_NONE;
    if (State) State->SetSquadServer(INDEX_NONE, false);
    APawn* Pawn = Bot->GetPawn();
    if (AOCVehicleBase* Vehicle = Cast<AOCVehicleBase>(Pawn))
    {
        Vehicle->ForceExitDriverServer();
        Pawn = Bot->GetPawn();
    }
    if (Pawn)
    {
        Bot->UnPossess();
        Pawn->Destroy();
    }
    Bot->Destroy();
    if (Team != EOCTeam::None && Squad != INDEX_NONE) RepairSquadLeadership(Team, Squad);
}

void AOCGameMode::RemoveAllBots()
{
    if (!HasAuthority() || !GetWorld()) return;
    TArray<AOCAIController*> Bots;
    for (TActorIterator<AOCAIController> It(GetWorld()); It; ++It) Bots.Add(*It);
    for (AOCAIController* Bot : Bots) RemoveBotController(Bot);
    RefreshPopulationState();
}

int32 AOCGameMode::GetHumanPlayerCount() const
{
    int32 Count = 0;
    if (!GetWorld()) return Count;
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        const APlayerController* PC = It->Get();
        if (PC && !PC->IsActorBeingDestroyed()) ++Count;
    }
    return Count;
}

int32 AOCGameMode::GetBotPlayerCount() const
{
    int32 Count = 0;
    if (!GetWorld()) return Count;
    for (TActorIterator<AOCAIController> It(GetWorld()); It; ++It) if (*It && !It->IsActorBeingDestroyed()) ++Count;
    return Count;
}

void AOCGameMode::RefreshPopulationState()
{
    if (AOCGameState* State = GetGameState<AOCGameState>())
    {
        State->ConfigurePopulationServer(MaxPlayerSlots, TargetPopulation);
        State->SetPopulationCountsServer(GetHumanPlayerCount(), GetBotPlayerCount());
    }
}

AOCAIController* AOCGameMode::SelectBotToRemove(EOCTeam PreferredTeam) const
{
    if (!GetWorld()) return nullptr;
    AOCAIController* Best = nullptr;
    float BestScore = -TNumericLimits<float>::Max();
    for (TActorIterator<AOCAIController> It(GetWorld()); It; ++It)
    {
        AOCAIController* Bot = *It; if (!Bot) continue;
        const AOCPlayerState* State = Bot->GetPlayerState<AOCPlayerState>();
        float Score = 0.0f;
        if (State && State->GetTeamId() == PreferredTeam) Score += 1000.0f;
        APawn* Pawn = Bot->GetPawn();
        if (!Pawn) Score += 500.0f;
        if (const AOCCharacter* Character = Cast<AOCCharacter>(Pawn); Character && Character->IsDowned()) Score += 350.0f;
        if (Cast<AOCVehicleBase>(Pawn)) Score -= 250.0f; // avoid popping an active driver unless necessary
        if (State) Score -= State->GetScore() * 0.01f;
        if (!Best || Score > BestScore) { Best = Bot; BestScore = Score; }
    }
    return Best;
}

void AOCGameMode::MaintainPopulation()
{
    if (!HasAuthority()) return;
    const int32 Humans = GetHumanPlayerCount();
    if (Humans >= MaxPlayerSlots)
    {
        while (GetBotPlayerCount() > 0) RemoveBotController(SelectBotToRemove(EOCTeam::None));
        RefreshPopulationState();
        return;
    }

    const int32 DesiredBots = bAutoFillBots ? FMath::Clamp(TargetPopulation - Humans, 0, MaxPlayerSlots - Humans) : GetBotPlayerCount();
    while (GetBotPlayerCount() > DesiredBots)
    {
        // Prefer replacing a bot from the team of the newest/most numerous humans to preserve team parity.
        const EOCTeam Preferred = AssignBalancedTeam(nullptr) == EOCTeam::TeamOne ? EOCTeam::TeamTwo : EOCTeam::TeamOne;
        AOCAIController* Bot = SelectBotToRemove(Preferred);
        if (!Bot) break;
        RemoveBotController(Bot);
    }
    if (bAutoFillBots && GetBotPlayerCount() < DesiredBots) SpawnDebugBots(DesiredBots - GetBotPlayerCount());
    RefreshPopulationState();
}


int32 AOCGameMode::MakeSquadKey(EOCTeam Team, int32 SquadId)
{
    return (Team == EOCTeam::TeamTwo ? 100 : 0) + FMath::Max(0, SquadId);
}

bool AOCGameMode::IsSquadFull(EOCTeam Team, int32 SquadId, const AOCPlayerState* IgnoreState) const
{
    if (Team == EOCTeam::None || SquadId < 0) return true;
    int32 Count = 0;
    if (const AOCGameState* State = GetGameState<AOCGameState>())
    {
        for (APlayerState* PS : State->PlayerArray)
        {
            const AOCPlayerState* OPS = Cast<AOCPlayerState>(PS);
            if (!OPS || OPS == IgnoreState) continue;
            if (OPS->GetTeamId() == Team && OPS->GetSquadId() == SquadId) ++Count;
        }
    }
    return Count >= MaxSquadSize;
}

int32 AOCGameMode::ChooseBestSquad(EOCTeam Team) const
{
    int32 BestSquad = 0;
    int32 BestCount = TNumericLimits<int32>::Max();
    for (int32 Squad = 0; Squad < 8; ++Squad)
    {
        int32 Count = 0;
        if (const AOCGameState* State = GetGameState<AOCGameState>())
        {
            for (APlayerState* PS : State->PlayerArray)
            {
                const AOCPlayerState* OPS = Cast<AOCPlayerState>(PS);
                if (OPS && OPS->GetTeamId() == Team && OPS->GetSquadId() == Squad) ++Count;
            }
        }
        if (Count < MaxSquadSize && Count < BestCount) { BestCount = Count; BestSquad = Squad; }
    }
    return BestSquad;
}

void AOCGameMode::AssignSquadServer(AOCPlayerState* State, int32 RequestedSquadId)
{
    if (!HasAuthority() || !State || State->GetTeamId() == EOCTeam::None) return;
    const EOCTeam Team = State->GetTeamId();
    int32 Squad = RequestedSquadId;
    if (Squad < 0 || Squad >= 8 || IsSquadFull(Team, Squad, State)) Squad = ChooseBestSquad(Team);

    bool bHasLeader = false;
    AOCPlayerState* BotLeader = nullptr;
    if (const AOCGameState* GS = GetGameState<AOCGameState>())
    {
        for (APlayerState* PS : GS->PlayerArray)
        {
            AOCPlayerState* OPS = Cast<AOCPlayerState>(PS); if (!OPS || OPS == State) continue;
            if (OPS->GetTeamId() == Team && OPS->GetSquadId() == Squad && OPS->IsSquadLeader())
            {
                bHasLeader = true;
                if (OPS->IsBotPlayer()) BotLeader = OPS;
                break;
            }
        }
    }
    const bool bPromoteHumanOverBot = !State->IsBotPlayer() && BotLeader;
    if (bPromoteHumanOverBot) BotLeader->SetSquadLeaderServer(false);
    State->SetSquadServer(Squad, !bHasLeader || bPromoteHumanOverBot);
}

void AOCGameMode::RepairSquadLeadership(EOCTeam Team, int32 SquadId)
{
    if (!HasAuthority() || SquadId < 0) return;
    TArray<AOCPlayerState*> Members;
    if (AOCGameState* GS = GetGameState<AOCGameState>())
    {
        for (APlayerState* PS : GS->PlayerArray)
        {
            AOCPlayerState* OPS = Cast<AOCPlayerState>(PS);
            if (OPS && OPS->GetTeamId() == Team && OPS->GetSquadId() == SquadId) Members.Add(OPS);
        }
    }
    if (Members.Num() == 0) { SquadOrders.Remove(MakeSquadKey(Team, SquadId)); return; }
    for (AOCPlayerState* Member : Members) if (Member && Member->IsSquadLeader()) return;
    Members.Sort([](const AOCPlayerState& A, const AOCPlayerState& B)
    {
        if (A.IsBotPlayer() != B.IsBotPlayer()) return !A.IsBotPlayer();
        return A.GetScore() > B.GetScore();
    });
    Members[0]->SetSquadLeaderServer(true);
}

bool AOCGameMode::RequestTeamChange(AOCPlayerState* State, EOCTeam RequestedTeam)
{
    if (!HasAuthority() || !State || RequestedTeam == EOCTeam::None) return false;
    if (State->GetTeamId() == RequestedTeam) return true;
    if (State->IsLobbyReady()) return false; // no mid-life team hopping in S17A

    int32 RequestedHumans = 0;
    int32 OtherHumans = 0;
    const EOCTeam OtherTeam = RequestedTeam == EOCTeam::TeamOne ? EOCTeam::TeamTwo : EOCTeam::TeamOne;
    if (const AOCGameState* GS = GetGameState<AOCGameState>())
    {
        for (APlayerState* PS : GS->PlayerArray)
        {
            const AOCPlayerState* OPS = Cast<AOCPlayerState>(PS);
            if (!OPS || OPS == State || OPS->IsBotPlayer()) continue;
            if (OPS->GetTeamId() == RequestedTeam) ++RequestedHumans;
            else if (OPS->GetTeamId() == OtherTeam) ++OtherHumans;
        }
    }
    if (RequestedHumans > OtherHumans) return false;

    const EOCTeam OldTeam = State->GetTeamId();
    const int32 OldSquad = State->GetSquadId();
    State->SetTeamServer(RequestedTeam);
    State->SetLobbyReadyServer(false);
    State->SetSquadServer(INDEX_NONE, false);
    if (OldTeam != EOCTeam::None && OldSquad >= 0) RepairSquadLeadership(OldTeam, OldSquad);
    AssignSquadServer(State, INDEX_NONE);
    ApplyFactionToState(State);
    return true;
}

bool AOCGameMode::RequestSquadChange(AOCPlayerState* State, int32 RequestedSquadId)
{
    if (!HasAuthority() || !State || State->GetTeamId() == EOCTeam::None) return false;
    if (RequestedSquadId < 0 || RequestedSquadId >= 8) RequestedSquadId = ChooseBestSquad(State->GetTeamId());
    if (IsSquadFull(State->GetTeamId(), RequestedSquadId, State)) return false;
    const int32 OldSquad = State->GetSquadId();
    const EOCTeam Team = State->GetTeamId();
    State->SetSquadServer(RequestedSquadId, false);
    RepairSquadLeadership(Team, OldSquad);
    RepairSquadLeadership(Team, RequestedSquadId);
    return true;
}

void AOCGameMode::RouteChatMessage(AOCPlayerController* Sender, EOCChatChannel Channel, const FString& Message)
{
    if (!HasAuthority() || !Sender || Message.IsEmpty() || !GetWorld()) return;
    const AOCPlayerState* SenderState = Sender->GetPlayerState<AOCPlayerState>();
    if (!SenderState) return;

    FOCChatMessage Chat;
    Chat.SenderName = SenderState->GetPlayerName();
    Chat.Message = Message.Left(120);
    Chat.Channel = Channel;
    Chat.Team = SenderState->GetTeamId();
    Chat.SquadId = SenderState->GetSquadId();
    Chat.ServerTime = GetWorld()->GetTimeSeconds();

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        AOCPlayerController* Recipient = Cast<AOCPlayerController>(It->Get()); if (!Recipient) continue;
        const AOCPlayerState* RecipientState = Recipient->GetPlayerState<AOCPlayerState>(); if (!RecipientState) continue;
        bool bDeliver = Channel == EOCChatChannel::Global;
        if (Channel == EOCChatChannel::Team) bDeliver = RecipientState->GetTeamId() == Chat.Team;
        if (Channel == EOCChatChannel::Squad) bDeliver = RecipientState->GetTeamId() == Chat.Team && RecipientState->GetSquadId() == Chat.SquadId;
        if (bDeliver) Recipient->ClientReceiveChat(Chat);
    }
}

bool AOCGameMode::SubmitSquadOrder(AOCPlayerController* Sender, EOCSquadOrderType Type, FName ObjectiveId,
    const FVector& RequestedLocation)
{
    if (!HasAuthority() || !Sender || !GetWorld()) return false;
    const AOCPlayerState* State = Sender->GetPlayerState<AOCPlayerState>();
    if (!State || !State->IsSquadLeader() || State->GetSquadId() < 0) return false;

    FOCSquadOrder Order;
    Order.Type = Type;
    Order.ObjectiveId = ObjectiveId;
    Order.IssuerName = State->GetPlayerName();
    Order.ServerTime = GetWorld()->GetTimeSeconds();
    Order.WorldLocation = Sender->GetPawn() ? Sender->GetPawn()->GetActorLocation() : FVector::ZeroVector;

    if (Type == EOCSquadOrderType::AttackObjective || Type == EOCSquadOrderType::DefendObjective)
    {
        AOCCapturePoint* Found = nullptr;
        for (TActorIterator<AOCCapturePoint> It(GetWorld()); It; ++It)
            if (It->GetPointId() == ObjectiveId) { Found = *It; break; }
        if (!Found) return false;
        Order.WorldLocation = Found->GetActorLocation();
    }
    else if (Type == EOCSquadOrderType::Move)
    {
        if (!Sender->GetPawn() || FVector::DistSquared(Sender->GetPawn()->GetActorLocation(), RequestedLocation) > FMath::Square(5000.0f)) return false;
        Order.WorldLocation = RequestedLocation;
    }
    else if (Type != EOCSquadOrderType::Regroup) return false;

    SquadOrders.Add(MakeSquadKey(State->GetTeamId(), State->GetSquadId()), Order);
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        AOCPlayerController* Recipient = Cast<AOCPlayerController>(It->Get()); if (!Recipient) continue;
        const AOCPlayerState* RState = Recipient->GetPlayerState<AOCPlayerState>();
        if (RState && RState->GetTeamId() == State->GetTeamId() && RState->GetSquadId() == State->GetSquadId())
            Recipient->ClientReceiveSquadOrder(Order);
    }
    return true;
}

bool AOCGameMode::GetSquadOrderFor(EOCTeam Team, int32 SquadId, FOCSquadOrder& OutOrder) const
{
    if (const FOCSquadOrder* Order = SquadOrders.Find(MakeSquadKey(Team, SquadId)))
    {
        OutOrder = *Order;
        return Order->IsActive();
    }
    return false;
}

void AOCGameMode::RestartPlayer(AController* NewPlayer)
{
    if (!NewPlayer)
    {
        return;
    }

    const AOCGameState* MatchState = GetGameState<AOCGameState>();
    if (MatchState && MatchState->GetOCMatchPhase() == EOCMatchPhase::Ended)
    {
        return;
    }

    if (AOCAIController* BotController = Cast<AOCAIController>(NewPlayer))
    {
        RestartBotController(BotController);
        return;
    }

    FTransform SpawnTransform;
    if (FindBestSpawnTransform(NewPlayer, SpawnTransform))
    {
        RestartPlayerAtTransform(NewPlayer, SpawnTransform);
        return;
    }

    const AOCPlayerState* State = NewPlayer->GetPlayerState<AOCPlayerState>();
    const bool bTeamTwo = State && State->GetTeamId() == EOCTeam::TeamTwo;
    const FVector FallbackLocation = bTeamTwo ? FVector(2800.0f, 0.0f, 120.0f) : FVector(-2800.0f, 0.0f, 120.0f);
    const FRotator FallbackRotation(0.0f, bTeamTwo ? 180.0f : 0.0f, 0.0f);
    RestartPlayerAtTransform(NewPlayer, FTransform(FallbackRotation, FallbackLocation));
}

bool AOCGameMode::FindBestSpawnTransform(AController* ControllerToSpawn, FTransform& OutTransform) const
{
    if (!ControllerToSpawn || !GetWorld()) return false;

    const AOCPlayerState* SpawnState = ControllerToSpawn->GetPlayerState<AOCPlayerState>();
    const EOCTeam Team = SpawnState ? SpawnState->GetTeamId() : EOCTeam::None;
    if (Team == EOCTeam::None) return false;

    FName Requested = TEXT("AUTO");
    if (const AOCPlayerController* HumanPC = Cast<AOCPlayerController>(ControllerToSpawn))
        Requested = HumanPC->GetRequestedDeploymentSpawn();

    auto MatchesRequest = [Requested](const AOCTeamSpawnPoint* Point)
    {
        if (!Point) return false;
        const FString R = Requested.ToString().ToUpper();
        if (R == TEXT("AUTO")) return true;
        if (R == TEXT("BASE")) return Point->IsBaseSpawn();
        return !Point->IsBaseSpawn() && Point->GetLinkedCapturePointId().ToString().Equals(R, ESearchCase::IgnoreCase);
    };

    auto FindSafest = [&](bool bHonorRequest) -> const AOCTeamSpawnPoint*
    {
        const AOCTeamSpawnPoint* BestPoint = nullptr;
        double BestSafetyScore = -1.0;
        for (TActorIterator<AOCTeamSpawnPoint> It(GetWorld()); It; ++It)
        {
            const AOCTeamSpawnPoint* Point = *It;
            if (!Point || !Point->IsAvailableForTeam(Team) || (bHonorRequest && !MatchesRequest(Point))) continue;

            double NearestEnemySq = TNumericLimits<double>::Max();
            bool bFoundEnemy = false;
            for (TActorIterator<AOCCharacter> CharacterIt(GetWorld()); CharacterIt; ++CharacterIt)
            {
                const AOCCharacter* Character = *CharacterIt;
                const AOCPlayerState* CharacterState = Character ? Character->GetPlayerState<AOCPlayerState>() : nullptr;
                if (!Character || !CharacterState || CharacterState->GetTeamId() == Team || CharacterState->GetTeamId() == EOCTeam::None ||
                    !Character->GetHealthComponent() || !Character->GetHealthComponent()->IsAlive()) continue;
                bFoundEnemy = true;
                NearestEnemySq = FMath::Min(NearestEnemySq,
                    static_cast<double>(FVector::DistSquared(Point->GetActorLocation(), Character->GetActorLocation())));
            }
            const double SafetyScore = bFoundEnemy ? NearestEnemySq : TNumericLimits<double>::Max() * 0.5;
            if (!BestPoint || SafetyScore > BestSafetyScore) { BestPoint = Point; BestSafetyScore = SafetyScore; }
        }
        return BestPoint;
    };

    // Requested forward point may be unavailable because it is neutral/contested/lost. In that case we safely fall back.
    const AOCTeamSpawnPoint* BestPoint = FindSafest(Requested != FName(TEXT("AUTO")));
    if (!BestPoint) BestPoint = FindSafest(false);
    if (!BestPoint) return false;

    OutTransform = BestPoint->GetActorTransform();
    OutTransform.AddToTranslation(FVector(0.0f, 0.0f, 80.0f));
    return true;
}

void AOCGameMode::HandleCharacterDeath(AOCCharacter* DeadCharacter, AController* KillerController)
{
    if (!DeadCharacter || !HasAuthority())
    {
        return;
    }

    AController* DeadController = DeadCharacter->GetController();
    AOCPlayerState* VictimState = DeadCharacter->GetPlayerState<AOCPlayerState>();
    AOCPlayerState* KillerState = KillerController ? KillerController->GetPlayerState<AOCPlayerState>() : nullptr;

    if (VictimState)
    {
        VictimState->RegisterDeath();
        if (!bSandboxMode)
        {
            if (AOCGameState* MatchState = GetGameState<AOCGameState>()) MatchState->RemoveTicketsServer(VictimState->GetTeamId(), 1);
        }
    }

    if (KillerState && KillerState != VictimState && VictimState && KillerState->GetTeamId() != VictimState->GetTeamId())
    {
        KillerState->RegisterKill(100);
    }

    if (!bSandboxMode) CheckForRoundEnd();

    RegisterCorpse(DeadCharacter);

    if (!DeadController)
    {
        DeadCharacter->SetLifeSpan(FMath::Max(CorpseLifetimeSeconds, 3.0f));
        return;
    }

    DeadCharacter->DetachFromControllerPendingDestroy();
    // Respawn timing and corpse cleanup are intentionally independent in S14B.
    DeadCharacter->SetLifeSpan(FMath::Max(CorpseLifetimeSeconds, RespawnDelay + 1.0f));

    const AOCGameState* MatchState = GetGameState<AOCGameState>();
    if (MatchState && MatchState->GetOCMatchPhase() == EOCMatchPhase::Ended)
    {
        return;
    }

    FTimerDelegate RespawnDelegate;
    RespawnDelegate.BindUObject(this, &AOCGameMode::RespawnController, DeadController);
    FTimerHandle RespawnTimer;
    GetWorldTimerManager().SetTimer(RespawnTimer, RespawnDelegate, RespawnDelay, false);
}

void AOCGameMode::RegisterCorpse(AOCCharacter* DeadCharacter)
{
    if (!HasAuthority() || !DeadCharacter) return;

    CorpseQueue.RemoveAll([](const TWeakObjectPtr<AOCCharacter>& Item) { return !Item.IsValid(); });
    CorpseQueue.Add(DeadCharacter);

    while (CorpseQueue.Num() > MaxPersistentCorpses)
    {
        TWeakObjectPtr<AOCCharacter> Oldest = CorpseQueue[0];
        CorpseQueue.RemoveAt(0);
        if (Oldest.IsValid()) Oldest->Destroy();
    }
}

void AOCGameMode::RespawnController(AController* ControllerToRespawn)
{
    const AOCGameState* MatchState = GetGameState<AOCGameState>();
    if (IsValid(ControllerToRespawn) && (!MatchState || MatchState->GetOCMatchPhase() != EOCMatchPhase::Ended))
    {
        RestartPlayer(ControllerToRespawn);
    }
}

void AOCGameMode::HandleCapturePointOwnerChanged(AOCCapturePoint* Point, EOCTeam PreviousOwner, EOCTeam NewOwner)
{
    if (!HasAuthority() || !Point)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Capture point %s owner: %s -> %s"), *Point->GetPointId().ToString(),
        *OCTeamToString(PreviousOwner), *OCTeamToString(NewOwner));
}

bool AOCGameMode::CanUseSandboxAdmin(const AController* Controller) const
{
#if UE_BUILD_SHIPPING
    return false;
#else
    if (!bSandboxMode || !Controller) return false;
    if (bAllowSandboxAdminAll) return true;
    // A listen/local host may administer its own local Sandbox. Remote clients on a dedicated server
    // require the server-owned SandboxAdminAll test switch until a persistent account/admin identity exists.
    const APlayerController* PlayerController = Cast<APlayerController>(Controller);
    return PlayerController && GetNetMode() != NM_DedicatedServer && PlayerController->IsLocalController();
#endif
}

bool AOCGameMode::CanDealDamage(const AController* InstigatorController, const AActor* VictimActor) const
{
    if (bSandboxMode && VictimActor)
    {
        const APawn* SandboxVictimPawn = Cast<APawn>(VictimActor);
        const AOCPlayerController* SandboxVictimPC = SandboxVictimPawn ? Cast<AOCPlayerController>(SandboxVictimPawn->GetController()) : nullptr;
        if (SandboxVictimPC && SandboxVictimPC->IsSandboxGodMode())
        {
            return false;
        }
    }
    if (bFriendlyFire || !InstigatorController || !VictimActor)
    {
        return true;
    }

    const AOCPlayerState* InstigatorState = InstigatorController->GetPlayerState<AOCPlayerState>();
    if (const AOCArmedVehicleBase* ArmedVehicle = Cast<AOCArmedVehicleBase>(VictimActor))
    {
        if (InstigatorState && ArmedVehicle->GetOccupantTeam() != EOCTeam::None &&
            InstigatorState->GetTeamId() == ArmedVehicle->GetOccupantTeam())
        {
            return false;
        }
    }

    const APawn* VictimPawn = Cast<APawn>(VictimActor);
    const AController* VictimController = VictimPawn ? VictimPawn->GetController() : nullptr;
    if (!VictimController || VictimController == InstigatorController)
    {
        return true;
    }

    const AOCPlayerState* VictimState = VictimController->GetPlayerState<AOCPlayerState>();
    if (!InstigatorState || !VictimState || InstigatorState->GetTeamId() == EOCTeam::None || VictimState->GetTeamId() == EOCTeam::None)
    {
        return true;
    }

    return InstigatorState->GetTeamId() != VictimState->GetTeamId();
}

void AOCGameMode::ApplyTicketBleed()
{
    if (bSandboxMode) return;
    if (!HasAuthority())
    {
        return;
    }

    AOCGameState* MatchState = GetGameState<AOCGameState>();
    if (!MatchState || MatchState->GetOCMatchPhase() != EOCMatchPhase::InProgress)
    {
        return;
    }

    int32 TeamOnePoints = 0;
    int32 TeamTwoPoints = 0;
    for (TActorIterator<AOCCapturePoint> It(GetWorld()); It; ++It)
    {
        const AOCCapturePoint* Point = *It;
        if (!Point) continue;
        if (Point->GetOwnerTeam() == EOCTeam::TeamOne) ++TeamOnePoints;
        else if (Point->GetOwnerTeam() == EOCTeam::TeamTwo) ++TeamTwoPoints;
    }

    if (TeamOnePoints > TeamTwoPoints)
    {
        MatchState->RemoveTicketsServer(EOCTeam::TeamTwo, TeamOnePoints - TeamTwoPoints);
    }
    else if (TeamTwoPoints > TeamOnePoints)
    {
        MatchState->RemoveTicketsServer(EOCTeam::TeamOne, TeamTwoPoints - TeamOnePoints);
    }

    CheckForRoundEnd();
}

void AOCGameMode::CheckForRoundEnd()
{
    if (bSandboxMode) return;
    AOCGameState* MatchState = GetGameState<AOCGameState>();
    if (!MatchState || MatchState->GetOCMatchPhase() != EOCMatchPhase::InProgress)
    {
        return;
    }

    const int32 TeamOne = MatchState->GetTickets(EOCTeam::TeamOne);
    const int32 TeamTwo = MatchState->GetTickets(EOCTeam::TeamTwo);
    if (TeamOne <= 0 || TeamTwo <= 0)
    {
        EOCTeam Winner = EOCTeam::None;
        if (TeamOne > TeamTwo) Winner = EOCTeam::TeamOne;
        else if (TeamTwo > TeamOne) Winner = EOCTeam::TeamTwo;
        MatchState->FinishRoundServer(Winner);
        if (!GetWorldTimerManager().IsTimerActive(RoundRestartTimerHandle))
        {
            GetWorldTimerManager().SetTimer(RoundRestartTimerHandle, this, &AOCGameMode::RestartPrototypeRound,
                RoundEndDuration, false);
        }
    }
}

void AOCGameMode::RestartPrototypeRound()
{
    if (!HasAuthority())
    {
        return;
    }

    for (TActorIterator<AOCCapturePoint> It(GetWorld()); It; ++It) if (AOCCapturePoint* Point = *It) Point->ResetPointServer();
    for (TActorIterator<AOCInteractableDoor> It(GetWorld()); It; ++It) if (AOCInteractableDoor* Door = *It) Door->ResetServer();
    for (TActorIterator<AOCInteractableGate> It(GetWorld()); It; ++It) if (AOCInteractableGate* Gate = *It) Gate->ResetServer();
    for (TActorIterator<AOCInteractableLight> It(GetWorld()); It; ++It) if (AOCInteractableLight* Light = *It) Light->ResetServer();
    for (TActorIterator<AOCBreakableWindow> It(GetWorld()); It; ++It) if (AOCBreakableWindow* Window = *It) Window->ResetServer();
    for (TActorIterator<AOCDestructibleProp> It(GetWorld()); It; ++It) if (AOCDestructibleProp* Prop = *It) Prop->ResetServer();

    TArray<AActor*> TransientRoundActors;
    for (TActorIterator<AOCSmokeCloud> It(GetWorld()); It; ++It) if (AOCSmokeCloud* Smoke = *It) TransientRoundActors.Add(Smoke);
    for (TActorIterator<AOCDeployableTrap> It(GetWorld()); It; ++It) if (AOCDeployableTrap* Trap = *It) TransientRoundActors.Add(Trap);
    for (AActor* Actor : TransientRoundActors) if (IsValid(Actor)) Actor->Destroy();

    // Clear vehicle-seat ownership before pawns are destroyed/restarted.
    for (TActorIterator<AOCArmedVehicleBase> It(GetWorld()); It; ++It) if (AOCArmedVehicleBase* Vehicle = *It) Vehicle->ForceExitGunnerServer();
    for (TActorIterator<AOCVehicleBase> It(GetWorld()); It; ++It) if (AOCVehicleBase* Vehicle = *It) Vehicle->ForceExitDriverServer();

    TArray<AOCVehicleBase*> RoundVehicles;
    for (TActorIterator<AOCVehicleBase> It(GetWorld()); It; ++It) if (AOCVehicleBase* Vehicle = *It) RoundVehicles.Add(Vehicle);
    for (AOCVehicleBase* Vehicle : RoundVehicles) if (IsValid(Vehicle)) Vehicle->Destroy();
    for (TActorIterator<AOCVehicleSpawnPoint> It(GetWorld()); It; ++It) if (AOCVehicleSpawnPoint* SpawnPoint = *It) SpawnPoint->ResetForRoundServer();

    if (AOCGameState* MatchState = GetGameState<AOCGameState>())
    {
        MatchState->InitializeRoundServer(StartingTickets);
        for (APlayerState* RawState : MatchState->PlayerArray)
        {
            if (AOCPlayerState* State = Cast<AOCPlayerState>(RawState)) State->ResetRoundStatsServer();
        }
    }

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC)
        {
            continue;
        }
        if (APawn* ExistingPawn = PC->GetPawn())
        {
            if (AOCVehicleBase* Vehicle = Cast<AOCVehicleBase>(ExistingPawn))
            {
                Vehicle->ForceExitDriverServer();
                ExistingPawn = PC->GetPawn();
            }
            if (ExistingPawn)
            {
                PC->UnPossess();
                ExistingPawn->Destroy();
            }
        }
        RestartPlayer(PC);
    }

    TArray<AOCAIController*> BotControllers;
    for (TActorIterator<AOCAIController> It(GetWorld()); It; ++It)
    {
        if (AOCAIController* Bot = *It) BotControllers.Add(Bot);
    }
    for (AOCAIController* Bot : BotControllers)
    {
        RestartBotController(Bot);
    }
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

void AOCGameMode::SpawnOsterCenterSector()
{
    if (!HasAuthority())
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // R11: the release map is intentionally blank, so provide a source-owned daylight/atmosphere rig first.
    GetWorld()->SpawnActor<AOCVisualEnvironment>(AOCVisualEnvironment::StaticClass(), FTransform::Identity, SpawnParams);

    GetWorld()->SpawnActor<AOCWorldSectorOster>(AOCWorldSectorOster::StaticClass(), FTransform::Identity, SpawnParams);

    // Pass45: the old S08 enterable house was explicitly gameplay-authored rather than tied to a
    // verified residence. Do not spawn it in normal runtime. The class remains available for future
    // location-specific authored use, while the existing anchor may still seed non-visual ambient audio.
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_GENERIC_ENTERABLE_HOUSE_RETIRED normal_runtime_spawn=0 reference_required=1"));

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    const FVector Stadium = AOCWorldSectorOster::StadiumAnchor();
    const FVector Park = AOCWorldSectorOster::ParkAnchor();
    const FVector College = AOCWorldSectorOster::CollegeAnchor();

    // S15B ambient topology. These are client-local presentation zones; final Sound/MetaSound profiles are assigned in Content.
    struct FAmbientZoneSeed { FVector Location; FVector Extent; float MinInterval; float MaxInterval; float Radius; };
    const FAmbientZoneSeed AmbientSeeds[] =
    {
        { Museum, FVector(18000,15000,7000), 7.0f, 16.0f, 2100.0f },
        { Park, FVector(26000,24000,7000), 4.0f, 11.0f, 2600.0f },
        { College, FVector(17000,16000,7000), 8.0f, 18.0f, 2200.0f },
        { AOCWorldSectorOster::KrushelnytskaEnterableHouseAnchor(), FVector(24000,30000,6500), 5.0f, 14.0f, 2400.0f }
    };
    for (const FAmbientZoneSeed& Seed : AmbientSeeds)
    {
        if (AOCAmbientAudioZone* Zone = GetWorld()->SpawnActor<AOCAmbientAudioZone>(AOCAmbientAudioZone::StaticClass(), Seed.Location, FRotator::ZeroRotator, SpawnParams))
        {
            Zone->ConfigureRuntime(Seed.Extent, Seed.MinInterval, Seed.MaxInterval, Seed.Radius);
        }
    }

    // Pass 44: keep the regression firing lane inside the compact central battlefield instead of the retired south-east edge.
    const FVector TargetLocations[] =
    {
        Museum + FVector(-9000.0f, 9000.0f, 180.0f), Museum + FVector(-6500.0f, 9000.0f, 180.0f),
        Museum + FVector(-4000.0f, 9000.0f, 180.0f), Museum + FVector(-1500.0f, 9000.0f, 180.0f),
        Museum + FVector(1000.0f, 9000.0f, 180.0f)
    };
    for (const FVector& Location : TargetLocations)
    {
        GetWorld()->SpawnActor<AOCDamageTarget>(AOCDamageTarget::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    }

    // S14B destruction lane remains useful, but its actors are now authored beside the compact firing lane.
    struct FDestructibleSeed { EOCImpactSurface Surface; FVector Location; FVector Scale; float Durability; };
    const FDestructibleSeed DestructionSeeds[] =
    {
        { EOCImpactSurface::Wood, Museum + FVector(-9000.0f, 11500.0f, 80.0f), FVector(1.6f,0.22f,1.1f), 65.0f },
        { EOCImpactSurface::Metal, Museum + FVector(-6500.0f, 11500.0f, 80.0f), FVector(1.2f,0.18f,1.2f), 160.0f },
        { EOCImpactSurface::Masonry, Museum + FVector(-4000.0f, 11500.0f, 90.0f), FVector(1.8f,0.28f,1.4f), 220.0f }
    };
    for (const FDestructibleSeed& Seed : DestructionSeeds)
    {
        if (AOCDestructibleProp* Prop = GetWorld()->SpawnActor<AOCDestructibleProp>(
            AOCDestructibleProp::StaticClass(), Seed.Location, FRotator::ZeroRotator, SpawnParams))
        {
            Prop->ConfigureRuntime(Seed.Surface, Seed.Durability, Seed.Scale);
        }
    }

    struct FPickupSeed { TSubclassOf<AOCWeaponBase> Class; FVector Location; };
    const FPickupSeed WeaponSeeds[] =
    {
        { AOCWeapon_AssaultRifle::StaticClass(), Museum + FVector(-4200.0f, -5200.0f, 80.0f) },
        { AOCWeapon_SMG::StaticClass(), Stadium + FVector(0.0f, -6500.0f, 80.0f) },
        { AOCWeapon_Sniper::StaticClass(), Park + FVector(6500.0f, 5000.0f, 80.0f) },
        { AOCWeapon_Shotgun::StaticClass(), College + FVector(4800.0f, -2600.0f, 80.0f) },
        { AOCWeapon_LMG::StaticClass(), College + FVector(-5000.0f, 5200.0f, 80.0f) },
        { AOCWeapon_Pistol::StaticClass(), Museum + FVector(3500.0f, 4200.0f, 80.0f) }
    };

    int32 WeaponIndex = 0;
    for (const FPickupSeed& Seed : WeaponSeeds)
    {
        AOCWeaponBase* Weapon = GetWorld()->SpawnActor<AOCWeaponBase>(Seed.Class, Seed.Location, FRotator::ZeroRotator, SpawnParams);
        if (Weapon)
        {
            Weapon->DropToWorldServer(Seed.Location, FRotator::ZeroRotator);
            if (WeaponIndex == 0)
            {
                Weapon->InstallAttachmentServer(EOCAttachmentSlot::Optic, FName(TEXT("RedDot")));
                Weapon->InstallAttachmentServer(EOCAttachmentSlot::Underbarrel, FName(TEXT("VerticalGrip")));
            }
            else if (WeaponIndex == 1)
            {
                Weapon->InstallAttachmentServer(EOCAttachmentSlot::Muzzle, FName(TEXT("Suppressor")));
            }
            else if (WeaponIndex == 4)
            {
                Weapon->InstallAttachmentServer(EOCAttachmentSlot::Magazine, FName(TEXT("ExtendedMag")));
            }
        }
        ++WeaponIndex;
    }

    const FVector AmmoLocations[] =
    {
        Museum + FVector(-3000.0f, -4500.0f, 60.0f),
        Park + FVector(0.0f, -4500.0f, 60.0f),
        College + FVector(3600.0f, 4300.0f, 60.0f)
    };
    for (const FVector& Location : AmmoLocations)
    {
        GetWorld()->SpawnActor<AOCAmmoBox>(AOCAmmoBox::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    }

    // S07 objective layout uses the real-city-inspired sectors instead of the old 80 m test arena.
    struct FObjectiveSeed { const TCHAR* Id; FVector Location; float Radius; float CaptureSeconds; };
    const FObjectiveSeed ObjectiveSeeds[] =
    {
        { TEXT("A"), (Museum + Stadium) * 0.5f + FVector(0.0f, 0.0f, 20.0f), 1100.0f, 14.0f },
        { TEXT("B"), Park + FVector(0.0f, 0.0f, 20.0f), 1250.0f, 15.0f },
        { TEXT("C"), College + FVector(0.0f, 2400.0f, 20.0f), 1100.0f, 14.0f }
    };
    for (const FObjectiveSeed& Seed : ObjectiveSeeds)
    {
        AOCCapturePoint* Point = GetWorld()->SpawnActor<AOCCapturePoint>(AOCCapturePoint::StaticClass(), Seed.Location,
            FRotator::ZeroRotator, SpawnParams);
        if (Point)
        {
            Point->ConfigureServer(FName(Seed.Id), Seed.Radius, Seed.CaptureSeconds);
        }
    }

    struct FSpawnSeed { EOCTeam Team; FVector Location; float Yaw; bool bBase; const TCHAR* LinkedPoint; };
    const FSpawnSeed SpawnSeeds[] =
    {
        // Pass 44: BASE actors are authored beside Museum from frame zero. Primary/secondary identity is
        // resolved by AOCTeamSpawnPoint without any dependency on the retired ±920 m map edges.
        { EOCTeam::TeamOne, Museum + FVector(-1400.0f, -2400.0f, 40.0f), 60.0f, true, TEXT("") },
        { EOCTeam::TeamOne, Museum + FVector(-2300.0f, -3100.0f, 40.0f), 60.0f, true, TEXT("") },
        { EOCTeam::TeamTwo, Museum + FVector(1400.0f, -2400.0f, 40.0f), 120.0f, true, TEXT("") },
        { EOCTeam::TeamTwo, Museum + FVector(2300.0f, -3100.0f, 40.0f), 120.0f, true, TEXT("") },

        // Forward spawn candidates are intentionally outside the capture radius and require point ownership.
        { EOCTeam::TeamOne, Museum + FVector(-7500.0f, -1500.0f, 40.0f), 15.0f, false, TEXT("A") },
        { EOCTeam::TeamTwo, Stadium + FVector(7600.0f, 2000.0f, 40.0f), 195.0f, false, TEXT("A") },
        { EOCTeam::TeamOne, Park + FVector(-8500.0f, -5200.0f, 40.0f), 20.0f, false, TEXT("B") },
        { EOCTeam::TeamTwo, Park + FVector(8500.0f, 5200.0f, 40.0f), 200.0f, false, TEXT("B") },
        { EOCTeam::TeamOne, College + FVector(-7500.0f, -5000.0f, 40.0f), 10.0f, false, TEXT("C") },
        { EOCTeam::TeamTwo, College + FVector(7600.0f, 5500.0f, 40.0f), 190.0f, false, TEXT("C") }
    };
    for (const FSpawnSeed& Seed : SpawnSeeds)
    {
        AOCTeamSpawnPoint* Point = GetWorld()->SpawnActor<AOCTeamSpawnPoint>(AOCTeamSpawnPoint::StaticClass(), Seed.Location,
            FRotator(0.0f, Seed.Yaw, 0.0f), SpawnParams);
        if (Point)
        {
            Point->ConfigureServer(Seed.Team, Seed.bBase, Seed.bBase ? NAME_None : FName(Seed.LinkedPoint));
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS44_RUNTIME_GAMEPLAY_SEEDS_COMPACT_READY firing_lane=compact destruction_lane=compact base_seeds=museum old_edge_base_seeds=0"));
}


void AOCGameMode::SpawnCivilianVehicleFleet()
{
    if (!HasAuthority() || !GetWorld())
    {
        return;
    }

    struct FVehicleSeed
    {
        FVector Location;
        float Yaw;
        EOCCivilianVehicleStyle Style;
        float RespawnDelay;
    };

    // S10 distributed civilian mobility. All Pass 44 seeds are inside the compact central road graph.
    const FVehicleSeed Seeds[] =
    {
        { FVector(-47000.0f,  9000.0f, 145.0f),   0.0f, EOCCivilianVehicleStyle::Wagon,     34.0f },
        { FVector(-25500.0f, 17000.0f, 145.0f), 180.0f, EOCCivilianVehicleStyle::Sedan,     34.0f },
        { FVector(-33500.0f, 36000.0f, 145.0f),  92.0f, EOCCivilianVehicleStyle::Hatchback, 30.0f },
        { FVector( -2500.0f, 47000.0f, 145.0f),  84.0f, EOCCivilianVehicleStyle::Wagon,     36.0f },
        { FVector( 11000.0f, -9000.0f, 145.0f),   0.0f, EOCCivilianVehicleStyle::Sedan,     32.0f },
        { FVector( 15000.0f, -9000.0f, 145.0f), 180.0f, EOCCivilianVehicleStyle::Hatchback, 32.0f },
        { FVector(-58500.0f, 67500.0f, 145.0f),   0.0f, EOCCivilianVehicleStyle::Wagon,     38.0f },
        { FVector(-26000.0f,  9200.0f, 145.0f), 180.0f, EOCCivilianVehicleStyle::Sedan,     34.0f }
    };

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    for (const FVehicleSeed& Seed : Seeds)
    {
        AOCVehicleSpawnPoint* SpawnPoint = GetWorld()->SpawnActor<AOCVehicleSpawnPoint>(
            AOCVehicleSpawnPoint::StaticClass(), Seed.Location, FRotator(0.0f, Seed.Yaw, 0.0f), Params);
        if (SpawnPoint)
        {
            SpawnPoint->ConfigureRuntime(Seed.Style, Seed.RespawnDelay);
        }
    }
}


void AOCGameMode::SpawnCombatVehicleFleet()
{
    if (!HasAuthority() || !GetWorld())
    {
        return;
    }

    struct FCombatSeed
    {
        bool bBTR;
        FVector Location;
        float Yaw;
        float RespawnDelay;
    };

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    const FVector Stadium = AOCWorldSectorOster::StadiumAnchor();

    // Pass 44: production pickup/HMMWV and BTR spawn points are inside the compact Museum/Stadium core,
    // close enough to be actually inspected during the next runtime instead of existing beyond the retired map edge.
    const FCombatSeed Seeds[] =
    {
        { false, Museum + FVector(-8500.0f, -7000.0f, 180.0f),  45.0f, 52.0f },
        { true,  Museum + FVector(-13500.0f, -6500.0f, 190.0f), 35.0f, 78.0f },
        { false, Museum + FVector( 8500.0f, -7000.0f, 180.0f), 135.0f, 52.0f },
        { true,  Stadium + FVector(-2500.0f, 5500.0f, 190.0f), 200.0f, 78.0f }
    };

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    for (const FCombatSeed& Seed : Seeds)
    {
        AOCVehicleSpawnPoint* SpawnPoint = nullptr;
        if (Seed.bBTR)
        {
            SpawnPoint = GetWorld()->SpawnActor<AOCBTRSpawnPoint>(AOCBTRSpawnPoint::StaticClass(),
                Seed.Location, FRotator(0.0f, Seed.Yaw, 0.0f), Params);
        }
        else
        {
            SpawnPoint = GetWorld()->SpawnActor<AOCPickupGunTruckSpawnPoint>(AOCPickupGunTruckSpawnPoint::StaticClass(),
                Seed.Location, FRotator(0.0f, Seed.Yaw, 0.0f), Params);
        }
        if (SpawnPoint)
        {
            SpawnPoint->ConfigureRespawnDelayRuntime(Seed.RespawnDelay);
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS44_COMBAT_VEHICLE_SEEDS_COMPACT_READY vehicles=4 museum_stadium_core=1 old_edge_vehicle_seeds=0"));
}
