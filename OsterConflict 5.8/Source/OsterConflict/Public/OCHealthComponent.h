#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OCHealthComponent.generated.h"

class AController;

UENUM(BlueprintType)
enum class EOCLifeState : uint8
{
    Alive,
    Downed,
    Dead
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOCHealthChangedSignature, float, NewHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOCDownedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOCRevivedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOCDeathSignature);

UCLASS(ClassGroup=(OsterConflict), meta=(BlueprintSpawnableComponent))
class OSTERCONFLICT_API UOCHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOCHealthComponent();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintPure, Category="Health")
    float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure, Category="Health")
    float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintPure, Category="Health")
    float GetHealthNormalized() const;

    UFUNCTION(BlueprintPure, Category="Health")
    EOCLifeState GetLifeState() const { return LifeState; }

    UFUNCTION(BlueprintPure, Category="Health")
    bool IsAlive() const { return LifeState == EOCLifeState::Alive; }

    UFUNCTION(BlueprintPure, Category="Health")
    bool IsDowned() const { return LifeState == EOCLifeState::Downed; }

    UFUNCTION(BlueprintPure, Category="Health")
    bool IsDead() const { return LifeState == EOCLifeState::Dead; }

    UFUNCTION(BlueprintPure, Category="Health|Downed")
    float GetDownedTimeRemaining() const;

    UFUNCTION(BlueprintPure, Category="Health")
    AController* GetLastDamageInstigator() const { return LastDamageInstigator.Get(); }

    /** Authority-only revive. Returns true only when the target was actually revived. */
    bool ReviveServer(AController* ReviverController);

    /** Authority-only final death, used by bleed-out and voluntary give-up. */
    void GiveUpServer();

    /** Sandbox/admin helper. Authority only; restores alive state and full health. */
    void RestoreFullServer();

    UPROPERTY(BlueprintAssignable, Category="Health")
    FOCHealthChangedSignature OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category="Health")
    FOCDownedSignature OnDowned;

    UPROPERTY(BlueprintAssignable, Category="Health")
    FOCRevivedSignature OnRevived;

    UPROPERTY(BlueprintAssignable, Category="Health")
    FOCDeathSignature OnDeath;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health", meta=(ClampMin="1.0"))
    float MaxHealth = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health|Regeneration", meta=(ClampMin="0.0"))
    float RegenDelay = 5.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health|Regeneration", meta=(ClampMin="0.0"))
    float RegenPerSecond = 25.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health|Downed", meta=(ClampMin="5.0"))
    float DownedDuration = 60.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health|Downed", meta=(ClampMin="1.0", ClampMax="100.0"))
    float ReviveHealthPercent = 35.0f;

    /** Damage this large bypasses Downed and kills immediately. Intended for prototype heavy explosions. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health|Downed", meta=(ClampMin="1.0"))
    float InstantDeathDamageThreshold = 180.0f;

    /** Any single hit at or above this value finishes a player who is already Downed. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health|Downed", meta=(ClampMin="1.0"))
    float DownedFinishDamageThreshold = 10.0f;

    UPROPERTY(ReplicatedUsing=OnRep_CurrentHealth, VisibleInstanceOnly, BlueprintReadOnly, Category="Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(ReplicatedUsing=OnRep_LifeState, VisibleInstanceOnly, BlueprintReadOnly, Category="Health")
    EOCLifeState LifeState = EOCLifeState::Alive;

    /** Server-world timestamp, synchronized through GameState for client countdown rendering. */
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Health|Downed")
    double DownedEndServerTime = 0.0;

    UFUNCTION()
    void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
        AController* InstigatedBy, AActor* DamageCauser);

    UFUNCTION()
    void OnRep_CurrentHealth(float PreviousHealth);

    UFUNCTION()
    void OnRep_LifeState(EOCLifeState PreviousState);

private:
    FTimerHandle RegenTimerHandle;
    FTimerHandle BleedOutTimerHandle;
    TWeakObjectPtr<AController> LastDamageInstigator;

    void ScheduleRegeneration();
    void RegenerationStep();
    void EnterDownedServer();
    void CompleteDeathServer();
    double GetSynchronizedServerTime() const;
};
