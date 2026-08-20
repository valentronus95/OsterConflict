#include "OCHealthComponent.h"

#include "OCCharacter.h"
#include "OCGameMode.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

UOCHealthComponent::UOCHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UOCHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        if (Owner->HasAuthority())
        {
            CurrentHealth = MaxHealth;
            LifeState = EOCLifeState::Alive;
            DownedEndServerTime = 0.0;
            Owner->OnTakeAnyDamage.AddDynamic(this, &UOCHealthComponent::HandleTakeAnyDamage);
        }
    }
}

void UOCHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UOCHealthComponent, CurrentHealth);
    DOREPLIFETIME(UOCHealthComponent, LifeState);
    DOREPLIFETIME(UOCHealthComponent, DownedEndServerTime);
}

float UOCHealthComponent::GetHealthNormalized() const
{
    return MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;
}

double UOCHealthComponent::GetSynchronizedServerTime() const
{
    if (const UWorld* World = GetWorld())
    {
        if (const AGameStateBase* GameState = World->GetGameState<AGameStateBase>())
        {
            return GameState->GetServerWorldTimeSeconds();
        }
        return World->GetTimeSeconds();
    }
    return 0.0;
}

float UOCHealthComponent::GetDownedTimeRemaining() const
{
    if (!IsDowned() || DownedEndServerTime <= 0.0)
    {
        return 0.0f;
    }

    return FMath::Max(0.0f, static_cast<float>(DownedEndServerTime - GetSynchronizedServerTime()));
}

void UOCHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
    AController* InstigatedBy, AActor* DamageCauser)
{
    if (!DamagedActor || !DamagedActor->HasAuthority() || Damage <= 0.0f || IsDead())
    {
        return;
    }

    if (const AOCGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AOCGameMode>() : nullptr)
    {
        if (!GameMode->CanDealDamage(InstigatedBy, DamagedActor))
        {
            return;
        }
    }

    LastDamageInstigator = InstigatedBy;

    FVector DamageOrigin = DamagedActor->GetActorLocation();
    if (InstigatedBy && InstigatedBy->GetPawn())
    {
        DamageOrigin = InstigatedBy->GetPawn()->GetActorLocation();
    }
    else if (DamageCauser)
    {
        DamageOrigin = DamageCauser->GetActorLocation();
    }

    if (AOCCharacter* CharacterOwner = Cast<AOCCharacter>(DamagedActor))
    {
        CharacterOwner->NotifyDamageReceived(DamageOrigin);
    }

    if (IsDowned())
    {
        if (Damage >= DownedFinishDamageThreshold)
        {
            CompleteDeathServer();
        }
        return;
    }

    const float PreviousHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
    OnHealthChanged.Broadcast(CurrentHealth, CurrentHealth - PreviousHealth);

    if (CurrentHealth <= 0.0f)
    {
        GetWorld()->GetTimerManager().ClearTimer(RegenTimerHandle);
        if (Damage >= InstantDeathDamageThreshold)
        {
            CompleteDeathServer();
        }
        else
        {
            EnterDownedServer();
        }
        return;
    }

    ScheduleRegeneration();
}

void UOCHealthComponent::EnterDownedServer()
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || LifeState != EOCLifeState::Alive)
    {
        return;
    }

    LifeState = EOCLifeState::Downed;
    CurrentHealth = 0.0f;
    DownedEndServerTime = GetSynchronizedServerTime() + static_cast<double>(DownedDuration);
    GetWorld()->GetTimerManager().ClearTimer(RegenTimerHandle);
    GetWorld()->GetTimerManager().SetTimer(
        BleedOutTimerHandle, this, &UOCHealthComponent::CompleteDeathServer, DownedDuration, false);

    OnDowned.Broadcast();
    GetOwner()->ForceNetUpdate();
}

bool UOCHealthComponent::ReviveServer(AController* ReviverController)
{
    (void)ReviverController;

    if (!GetOwner() || !GetOwner()->HasAuthority() || !IsDowned())
    {
        return false;
    }

    GetWorld()->GetTimerManager().ClearTimer(BleedOutTimerHandle);
    LifeState = EOCLifeState::Alive;
    DownedEndServerTime = 0.0;

    const float PreviousHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(MaxHealth * (ReviveHealthPercent / 100.0f), 1.0f, MaxHealth);
    OnHealthChanged.Broadcast(CurrentHealth, CurrentHealth - PreviousHealth);
    OnRevived.Broadcast();
    ScheduleRegeneration();

    // Keep the original attacker attribution only until revival. New damage starts a new combat chain.
    LastDamageInstigator.Reset();
    GetOwner()->ForceNetUpdate();
    return true;
}

void UOCHealthComponent::RestoreFullServer()
{
    if (!GetOwner() || !GetOwner()->HasAuthority()) return;
    GetWorld()->GetTimerManager().ClearTimer(RegenTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(BleedOutTimerHandle);
    const float PreviousHealth = CurrentHealth;
    LifeState = EOCLifeState::Alive;
    DownedEndServerTime = 0.0;
    CurrentHealth = MaxHealth;
    LastDamageInstigator.Reset();
    OnHealthChanged.Broadcast(CurrentHealth, CurrentHealth - PreviousHealth);
    OnRevived.Broadcast();
    GetOwner()->ForceNetUpdate();
}

void UOCHealthComponent::GiveUpServer()
{
    if (GetOwner() && GetOwner()->HasAuthority() && IsDowned())
    {
        CompleteDeathServer();
    }
}

void UOCHealthComponent::CompleteDeathServer()
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || IsDead())
    {
        return;
    }

    GetWorld()->GetTimerManager().ClearTimer(RegenTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(BleedOutTimerHandle);
    LifeState = EOCLifeState::Dead;
    CurrentHealth = 0.0f;
    DownedEndServerTime = 0.0;
    OnDeath.Broadcast();
    GetOwner()->ForceNetUpdate();
}

void UOCHealthComponent::ScheduleRegeneration()
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || RegenPerSecond <= 0.0f || !IsAlive())
    {
        return;
    }

    FTimerManager& TimerManager = GetWorld()->GetTimerManager();
    TimerManager.ClearTimer(RegenTimerHandle);
    TimerManager.SetTimer(RegenTimerHandle, this, &UOCHealthComponent::RegenerationStep, 0.20f, true, RegenDelay);
}

void UOCHealthComponent::RegenerationStep()
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || !IsAlive())
    {
        GetWorld()->GetTimerManager().ClearTimer(RegenTimerHandle);
        return;
    }

    const float PreviousHealth = CurrentHealth;
    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + RegenPerSecond * 0.20f);
    OnHealthChanged.Broadcast(CurrentHealth, CurrentHealth - PreviousHealth);

    if (CurrentHealth >= MaxHealth)
    {
        GetWorld()->GetTimerManager().ClearTimer(RegenTimerHandle);
    }
}

void UOCHealthComponent::OnRep_CurrentHealth(float PreviousHealth)
{
    OnHealthChanged.Broadcast(CurrentHealth, CurrentHealth - PreviousHealth);
}

void UOCHealthComponent::OnRep_LifeState(EOCLifeState PreviousState)
{
    if (LifeState == PreviousState)
    {
        return;
    }

    switch (LifeState)
    {
        case EOCLifeState::Alive:
            OnRevived.Broadcast();
            break;
        case EOCLifeState::Downed:
            OnDowned.Broadcast();
            break;
        case EOCLifeState::Dead:
            OnDeath.Broadcast();
            break;
        default:
            break;
    }
}
