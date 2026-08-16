#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "OCTeamTypes.h"
#include "OCCharacterVisualTypes.h"
#include "OCPlayerState.generated.h"

UCLASS()
class OSTERCONFLICT_API AOCPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    AOCPlayerState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintPure, Category="Score") int32 GetKills() const { return Kills; }
    UFUNCTION(BlueprintPure, Category="Score") int32 GetDeaths() const { return Deaths; }
    UFUNCTION(BlueprintPure, Category="Score") int32 GetRevives() const { return Revives; }
    UFUNCTION(BlueprintPure, Category="Network") int32 GetPingMs() const;

    UFUNCTION(BlueprintPure, Category="Team") EOCTeam GetTeamId() const { return TeamId; }
    UFUNCTION(BlueprintPure, Category="Team") EOCPlayerRole GetPlayerRole() const { return PlayerRole; }
    UFUNCTION(BlueprintPure, Category="Team") bool IsMedic() const { return PlayerRole == EOCPlayerRole::Medic; }
    UFUNCTION(BlueprintPure, Category="Team") bool IsEngineer() const { return PlayerRole == EOCPlayerRole::Engineer; }

    UFUNCTION(BlueprintPure, Category="Lobby") bool IsBotPlayer() const { return bBotPlayer; }
    UFUNCTION(BlueprintPure, Category="Lobby") int32 GetSquadId() const { return SquadId; }
    UFUNCTION(BlueprintPure, Category="Lobby") bool IsSquadLeader() const { return bSquadLeader; }
    UFUNCTION(BlueprintPure, Category="Lobby") bool IsLobbyReady() const { return bLobbyReady; }

    UFUNCTION(BlueprintPure, Category="Character|Visual") EOCFactionArchetype GetFactionArchetype() const { return FactionArchetype; }
    UFUNCTION(BlueprintPure, Category="Character|Visual") int32 GetAppearanceSeed() const { return AppearanceSeed; }

    /** Authority-only identity/team/squad state. */
    void SetTeamServer(EOCTeam NewTeam);
    void SetRoleServer(EOCPlayerRole NewRole);
    void SetBotPlayerServer(bool bNewBot);
    void SetSquadServer(int32 NewSquadId, bool bLeader);
    void SetSquadLeaderServer(bool bLeader);
    void SetLobbyReadyServer(bool bReady);
    void SetFactionServer(EOCFactionArchetype NewFaction, int32 NewAppearanceSeed);

    void RegisterKill(int32 ScoreAward = 100);
    void RegisterDeath();
    void RegisterRevive(int32 ScoreAward = 50);
    /** Authority-only score/KD/R reset between rounds. Identity/team/squad remain intact. */
    void ResetRoundStatsServer();

protected:
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Score") int32 Kills = 0;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Score") int32 Deaths = 0;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Score") int32 Revives = 0;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Team") EOCTeam TeamId = EOCTeam::None;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Team") EOCPlayerRole PlayerRole = EOCPlayerRole::Medic;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Lobby") bool bBotPlayer = false;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Lobby") int32 SquadId = INDEX_NONE;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Lobby") bool bSquadLeader = false;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Lobby") bool bLobbyReady = false;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Character|Visual") EOCFactionArchetype FactionArchetype = EOCFactionArchetype::UASpecialUnit;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Character|Visual") int32 AppearanceSeed = 1;
};
