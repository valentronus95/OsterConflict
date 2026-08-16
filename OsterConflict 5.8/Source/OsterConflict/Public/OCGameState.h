#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "OCTeamTypes.h"
#include "OCGameplayMode.h"
#include "OCGameState.generated.h"

UCLASS()
class OSTERCONFLICT_API AOCGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AOCGameState();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintPure, Category="Match") int32 GetTickets(EOCTeam Team) const;
    UFUNCTION(BlueprintPure, Category="Match") EOCMatchPhase GetOCMatchPhase() const { return MatchPhase; }
    UFUNCTION(BlueprintPure, Category="Match") EOCTeam GetWinningTeam() const { return WinningTeam; }
    UFUNCTION(BlueprintPure, Category="Match") int32 GetStartingTickets() const { return StartingTickets; }
    UFUNCTION(BlueprintPure, Category="Match") EOCGameplayMode GetGameplayMode() const { return GameplayMode; }
    UFUNCTION(BlueprintPure, Category="Match") bool IsSandboxMode() const { return GameplayMode == EOCGameplayMode::Sandbox; }

    UFUNCTION(BlueprintPure, Category="Population") int32 GetHumanPlayerCount() const { return HumanPlayerCount; }
    UFUNCTION(BlueprintPure, Category="Population") int32 GetBotPlayerCount() const { return BotPlayerCount; }
    UFUNCTION(BlueprintPure, Category="Population") int32 GetMaxPlayerSlots() const { return MaxPlayerSlots; }
    UFUNCTION(BlueprintPure, Category="Population") int32 GetTargetPopulation() const { return TargetPopulation; }

    void SetGameplayModeServer(EOCGameplayMode NewMode);
    void ConfigurePopulationServer(int32 InMaxSlots, int32 InTargetPopulation);
    void SetPopulationCountsServer(int32 Humans, int32 Bots);
    void InitializeRoundServer(int32 InStartingTickets);
    int32 RemoveTicketsServer(EOCTeam Team, int32 Amount);
    void FinishRoundServer(EOCTeam Winner);

protected:
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Match") int32 TeamOneTickets = 0;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Match") int32 TeamTwoTickets = 0;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Match") int32 StartingTickets = 0;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Match") EOCMatchPhase MatchPhase = EOCMatchPhase::Waiting;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Match") EOCTeam WinningTeam = EOCTeam::None;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Match") EOCGameplayMode GameplayMode = EOCGameplayMode::Conquest;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Population") int32 HumanPlayerCount = 0;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Population") int32 BotPlayerCount = 0;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Population") int32 MaxPlayerSlots = 16;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Population") int32 TargetPopulation = 16;
};
