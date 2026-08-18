#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OCTeamTypes.h"
#include "OCGameplayMode.h"
#include "OCBotTypes.h"
#include "OCLobbyTypes.h"
#include "OCCharacterVisualTypes.h"
#include "OCGameMode.generated.h"

class AOCCharacter;
class AOCCapturePoint;
class AOCPlayerState;
class AOCAIController;
class AOCPlayerController;
class AOCDestructibleProp;

UCLASS()
class OSTERCONFLICT_API AOCGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AOCGameMode();

    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void BeginPlay() override;
    virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
        FString& ErrorMessage) override;
    virtual void RestartPlayer(AController* NewPlayer) override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
        const FString& Options, const FString& Portal = TEXT("")) override;

    void HandleCharacterDeath(AOCCharacter* DeadCharacter, AController* KillerController);
    void HandleCapturePointOwnerChanged(AOCCapturePoint* Point, EOCTeam PreviousOwner, EOCTeam NewOwner);
    bool CanDealDamage(const AController* InstigatorController, const AActor* VictimActor) const;
    bool IsSandboxMode() const { return bSandboxMode; }
    /** True only for the standalone UI shell that must never expose the live gameplay world behind the frontend. */
    bool IsFrontendOnlySession() const { return bFrontendOnlySession; }
    /** Server-owned Sandbox admin policy. Sandbox mode by itself never grants admin rights. */
    bool CanUseSandboxAdmin(const AController* Controller) const;

    /** S13/S14 bot management. Bots are filler: humans always have slot priority. */
    void SpawnDebugBots(int32 Count);
    void RemoveAllBots();
    void MaintainPopulation();
    EOCBotDifficulty GetConfiguredBotDifficulty() const { return ConfiguredBotDifficulty; }
    int32 GetHumanPlayerCount() const;
    int32 GetBotPlayerCount() const;
    int32 GetMaxPlayerSlots() const { return MaxPlayerSlots; }
    int32 GetTargetPopulation() const { return TargetPopulation; }
    float GetAIThinkIntervalScale() const { return AIThinkIntervalScale; }
    FString GetPerformanceProfileName() const { return PerformanceProfileName; }
    FString BuildPerformanceSnapshot() const;
    EOCFactionArchetype GetFactionForTeam(EOCTeam Team) const { return Team == EOCTeam::TeamTwo ? TeamTwoFaction : TeamOneFaction; }

    /** S14/R13 squad/chat/deployment backend. */
    bool RequestSquadChange(AOCPlayerState* State, int32 RequestedSquadId);
    bool RequestRoleChange(AOCPlayerState* State, EOCPlayerRole RequestedRole);
    bool RequestTeamChange(AOCPlayerState* State, EOCTeam RequestedTeam);
    void RouteChatMessage(AOCPlayerController* Sender, EOCChatChannel Channel, const FString& Message);
    bool SubmitSquadOrder(AOCPlayerController* Sender, EOCSquadOrderType Type, FName ObjectiveId,
        const FVector& RequestedLocation);
    bool GetSquadOrderFor(EOCTeam Team, int32 SquadId, FOCSquadOrder& OutOrder) const;
    FString MakeUniquePlayerName(const FString& RequestedName, const AOCPlayerState* IgnoreState = nullptr) const;

protected:
    UPROPERTY(EditDefaultsOnly, Category="Respawn") float RespawnDelay = 3.0f;
    UPROPERTY(EditDefaultsOnly, Category="Respawn|Corpse", meta=(ClampMin="3.0")) float CorpseLifetimeSeconds = 30.0f;
    UPROPERTY(EditDefaultsOnly, Category="Respawn|Corpse", meta=(ClampMin="1", ClampMax="64")) int32 MaxPersistentCorpses = 20;
    UPROPERTY(EditDefaultsOnly, Category="Match", meta=(ClampMin="1")) int32 StartingTickets = 200;
    UPROPERTY(EditDefaultsOnly, Category="Match", meta=(ClampMin="1.0")) float TicketBleedInterval = 5.0f;
    UPROPERTY(EditDefaultsOnly, Category="Match") bool bFriendlyFire = false;
    UPROPERTY(EditDefaultsOnly, Category="Match", meta=(ClampMin="2.0")) float RoundEndDuration = 8.0f;

    UPROPERTY(EditDefaultsOnly, Category="Population", meta=(ClampMin="2", ClampMax="64")) int32 MaxPlayerSlots = 16;
    UPROPERTY(EditDefaultsOnly, Category="Population", meta=(ClampMin="0", ClampMax="64")) int32 TargetPopulation = 16;
    UPROPERTY(EditDefaultsOnly, Category="Population") bool bAutoFillBots = true;
    UPROPERTY(EditDefaultsOnly, Category="Population", meta=(ClampMin="0.5")) float BotRefillDelay = 3.0f;
    UPROPERTY(EditDefaultsOnly, Category="Squad", meta=(ClampMin="2", ClampMax="8")) int32 MaxSquadSize = 4;
    UPROPERTY(EditDefaultsOnly, Category="Character|Faction") EOCFactionArchetype TeamOneFaction = EOCFactionArchetype::UASpecialUnit;
    UPROPERTY(EditDefaultsOnly, Category="Character|Faction") EOCFactionArchetype TeamTwoFaction = EOCFactionArchetype::MaskedFighters;

private:
    int32 FallbackPlayerNumber = 1;
    bool bSandboxMode = false;
    /** Standalone Frontend is UI-only; it must not spawn match world/bots behind the menu. */
    bool bFrontendOnlySession = false;
    /** Explicit development/test server switch. Ignored in Shipping builds. */
    bool bAllowSandboxAdminAll = false;
    int32 RequestedBotCount = -1;
    EOCBotDifficulty ConfiguredBotDifficulty = EOCBotDifficulty::Normal;
    FString PerformanceProfileName = TEXT("Balanced");
    float AIThinkIntervalScale = 1.0f;
    int32 NextBotIndex = 1;
    FTimerHandle TicketBleedTimerHandle;
    FTimerHandle RoundRestartTimerHandle;
    FTimerHandle PopulationMaintenanceTimerHandle;
    TMap<int32, FOCSquadOrder> SquadOrders;
    TArray<TWeakObjectPtr<AOCCharacter>> CorpseQueue;

    void SpawnOsterCenterSector();
    void SpawnCivilianVehicleFleet();
    void SpawnCombatVehicleFleet();
    void RespawnController(AController* ControllerToRespawn);
    void SpawnConfiguredBots();
    bool SpawnSingleBot(EOCTeam Team, EOCPlayerRole BotRole);
    void RestartBotController(AOCAIController* BotController);
    void RemoveBotController(AOCAIController* BotController);
    AOCAIController* SelectBotToRemove(EOCTeam PreferredTeam) const;
    void RefreshPopulationState();
    void RegisterCorpse(AOCCharacter* DeadCharacter);

    static EOCBotDifficulty ParseBotDifficulty(const FString& Options);
    void ConfigurePerformanceProfile(const FString& Options);
    static EOCFactionArchetype ParseFactionOption(const FString& Value, EOCFactionArchetype Fallback);
    void ApplyFactionToState(AOCPlayerState* State) const;
    FString MakeFallbackPlayerName();
    EOCTeam AssignBalancedTeam(AOCPlayerState* JoiningState) const;
    static EOCPlayerRole ParseRequestedRole(const FString& Options);
    static int32 ParseRequestedSquad(const FString& Options);
    int32 ChooseBestSquad(EOCTeam Team) const;
    bool IsSquadFull(EOCTeam Team, int32 SquadId, const AOCPlayerState* IgnoreState = nullptr) const;
    void AssignSquadServer(AOCPlayerState* State, int32 RequestedSquadId);
    void RepairSquadLeadership(EOCTeam Team, int32 SquadId);
    static int32 MakeSquadKey(EOCTeam Team, int32 SquadId);

    bool FindBestSpawnTransform(AController* ControllerToSpawn, FTransform& OutTransform) const;
    void ApplyTicketBleed();
    void CheckForRoundEnd();
    void RestartPrototypeRound();
};
