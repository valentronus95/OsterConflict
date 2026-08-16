#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "OCBotTypes.h"
#include "OCTeamTypes.h"
#include "OCAIController.generated.h"

class AOCBotCharacter;
class AOCCapturePoint;
class AOCCharacter;
class AOCVehicleBase;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UENUM()
enum class EOCBotBrainState : uint8
{
    Objective,
    Combat,
    Revive,
    Vehicle
};

struct FOCBotRuntimeTuning
{
    float ThinkInterval = 0.20f;
    float SightRadius = 6500.0f;
    float LoseSightRadius = 7800.0f;
    float PeripheralVisionDegrees = 80.0f;
    float ReactionSeconds = 0.28f;
    float AimErrorDegrees = 1.20f;
    float PreferredCombatRange = 2600.0f;
    float MaxCombatRange = 7000.0f;
    float CoverSearchRadius = 850.0f;
    float ReviveSearchRadius = 1600.0f;
    float VehicleSeekDistance = 1800.0f;
    float VehicleUseObjectiveDistance = 5200.0f;
};

/**
 * S13 source-only bot brain.
 * AI Perception handles sight events, NavMesh handles path finding, and the high-level decision loop remains in C++
 * so this milestone does not depend on BehaviorTree/Blackboard assets that cannot be authored without Unreal Editor.
 */
UCLASS()
class OSTERCONFLICT_API AOCAIController : public AAIController
{
    GENERATED_BODY()

public:
    AOCAIController();

    virtual void OnPossess(APawn* InPawn) override;
    virtual void Tick(float DeltaSeconds) override;

    void AssignBotIdentityServer(EOCTeam Team, EOCPlayerRole BotRole, EOCBotDifficulty Difficulty, int32 BotIndex);
    void SetDifficultyServer(EOCBotDifficulty Difficulty);

    UFUNCTION(BlueprintPure, Category="AI")
    EOCBotDifficulty GetBotDifficulty() const { return BotDifficulty; }

    UFUNCTION(BlueprintPure, Category="AI")
    EOCBotBrainState GetBrainState() const { return BrainState; }

private:
    UPROPERTY(VisibleAnywhere, Category="AI")
    TObjectPtr<UAIPerceptionComponent> BotPerception;

    UPROPERTY()
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UPROPERTY()
    TObjectPtr<AOCCharacter> CombatTarget;

    UPROPERTY()
    TObjectPtr<AOCCharacter> ReviveTarget;

    UPROPERTY()
    TObjectPtr<AOCCapturePoint> ObjectiveTarget;

    EOCBotDifficulty BotDifficulty = EOCBotDifficulty::Normal;
    EOCBotBrainState BrainState = EOCBotBrainState::Objective;
    FOCBotRuntimeTuning Tuning;
    double NextThinkTime = 0.0;
    double TargetAcquiredTime = -1000.0;
    double NextCoverSearchTime = 0.0;
    double NextDoorCheckTime = 0.0;
    double NextVehicleSearchTime = 0.0;
    FVector CachedVehicleObjective = FVector::ZeroVector;

    UFUNCTION()
    void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    void RunDecisionLoop();
    void UpdateInfantryBrain(AOCBotCharacter* Bot);
    void UpdateVehicleBrain(AOCVehicleBase* Vehicle);
    AOCCharacter* FindBestEnemy(const AOCBotCharacter* Bot) const;
    AOCCharacter* FindReviveCandidate(const AOCBotCharacter* Bot) const;
    AOCCapturePoint* ChooseObjective(const AOCBotCharacter* Bot) const;
    AOCVehicleBase* FindUsefulVehicle(const AOCBotCharacter* Bot, const FVector& ObjectiveLocation) const;
    bool HasClearLineOfSightTo(const AOCCharacter* Bot, const AOCCharacter* Target) const;
    bool FindSimpleCoverPoint(const AOCBotCharacter* Bot, const AOCCharacter* Enemy, FVector& OutPoint) const;
    void EngageEnemy(AOCBotCharacter* Bot, AOCCharacter* Enemy);
    void MoveTowardObjective(AOCBotCharacter* Bot, AOCCapturePoint* Objective);
    void TryUseDoorAhead(AOCBotCharacter* Bot);
    void MoveBotTo(AOCBotCharacter* Bot, const FVector& Destination, float AcceptanceRadius);
    void StopCombat();
    EOCTeam GetBotTeam() const;
    static FOCBotRuntimeTuning MakeTuning(EOCBotDifficulty Difficulty);
};
