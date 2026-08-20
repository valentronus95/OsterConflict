#include "OCRecoveredWeaponVariantSubsystem.h"

#include "OCGameMode.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

namespace
{
    constexpr int32 MaxSpawnAttempts = 180;
    constexpr float SpawnRetryDelaySeconds = 0.35f;
    constexpr int32 WeaponTestCount = 10;
    const FName TestRackTag(TEXT("OC_TestWeaponRack"));
}

bool UOCRecoveredWeaponVariantSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCRecoveredWeaponVariantSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

#if UE_BUILD_SHIPPING
    return;
#else
    if (InWorld.GetNetMode() == NM_Client) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    ScheduleSpawnAttempt(0.25f);
#endif
}

void UOCRecoveredWeaponVariantSubsystem::ScheduleSpawnAttempt(const float DelaySeconds)
{
#if UE_BUILD_SHIPPING
    return;
#else
    UWorld* World = GetWorld();
    if (!World || bSpawnComplete || SpawnAttemptCount >= MaxSpawnAttempts) return;

    FTimerHandle Timer;
    World->GetTimerManager().SetTimer(
        Timer,
        FTimerDelegate::CreateWeakLambda(this, [this]() { TrySpawnTestRack(); }),
        FMath::Max(0.05f, DelaySeconds),
        false);
#endif
}

void UOCRecoveredWeaponVariantSubsystem::TrySpawnTestRack()
{
#if UE_BUILD_SHIPPING
    return;
#else
    UWorld* World = GetWorld();
    AOCGameMode* GameMode = World ? World->GetAuthGameMode<AOCGameMode>() : nullptr;
    if (!World || !GameMode || GameMode->IsFrontendOnlySession() || bSpawnComplete) return;

    ++SpawnAttemptCount;

    for (TActorIterator<AOCWeaponBase> It(World); It; ++It)
    {
        if (It->ActorHasTag(TestRackTag))
        {
            bSpawnComplete = true;
            return;
        }
    }

    APlayerController* PlayerController = World->GetFirstPlayerController();
    APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        if (SpawnAttemptCount < MaxSpawnAttempts) ScheduleSpawnAttempt(SpawnRetryDelaySeconds);
        else UE_LOG(LogTemp, Warning, TEXT("Test weapon rack: no deployed player pawn after %d attempts."), SpawnAttemptCount);
        return;
    }

    const TSubclassOf<AOCWeaponBase> Classes[WeaponTestCount] =
    {
        AOCWeapon_AssaultRifle::StaticClass(),
        AOCWeapon_SMG::StaticClass(),
        AOCWeapon_Pistol::StaticClass(),
        AOCWeapon_Sniper::StaticClass(),
        AOCWeapon_Shotgun::StaticClass(),
        AOCWeapon_LMG::StaticClass(),
        AOCWeapon_M14::StaticClass(),
        AOCWeapon_LeverAction::StaticClass(),
        AOCWeapon_Mac10::StaticClass(),
        AOCWeapon_Tec9::StaticClass(),
    };

    const FVector PawnLocation = PlayerPawn->GetActorLocation();
    const FVector Forward = PlayerPawn->GetActorForwardVector().GetSafeNormal2D();
    const FVector Right = PlayerPawn->GetActorRightVector().GetSafeNormal2D();
    const FVector RackOrigin = PawnLocation + Forward * 360.0f;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    TArray<AOCWeaponBase*> SpawnedWeapons;
    SpawnedWeapons.Reserve(WeaponTestCount);

    bool bSpawnedAll = true;
    for (int32 Index = 0; Index < WeaponTestCount; ++Index)
    {
        const int32 Row = Index / 5;
        const int32 Column = Index % 5;
        const float LateralCm = (static_cast<float>(Column) - 2.0f) * 145.0f;
        const float ForwardCm = static_cast<float>(Row) * 170.0f;
        const FVector Location = RackOrigin + Right * LateralCm + Forward * ForwardCm + FVector(0.0f, 0.0f, 90.0f);

        AOCWeaponBase* Weapon = World->SpawnActor<AOCWeaponBase>(
            Classes[Index], Location, PlayerPawn->GetActorRotation(), Params);
        if (!Weapon)
        {
            bSpawnedAll = false;
            break;
        }

        Weapon->Tags.Add(TestRackTag);
        Weapon->DropToWorldServer(Location, PlayerPawn->GetActorRotation());
        SpawnedWeapons.Add(Weapon);
    }

    if (!bSpawnedAll || SpawnedWeapons.Num() != WeaponTestCount)
    {
        for (AOCWeaponBase* Weapon : SpawnedWeapons)
        {
            if (IsValid(Weapon)) Weapon->Destroy();
        }

        if (SpawnAttemptCount < MaxSpawnAttempts) ScheduleSpawnAttempt(SpawnRetryDelaySeconds);
        else UE_LOG(LogTemp, Warning,
            TEXT("Test weapon rack: could not create all %d pickups after %d attempts."), WeaponTestCount, SpawnAttemptCount);
        return;
    }

    bSpawnComplete = true;
    UE_LOG(LogTemp, Display,
        TEXT("Test weapon rack spawned beside deployed player: %d/%d weapons after %d attempt(s)."),
        SpawnedWeapons.Num(), WeaponTestCount, SpawnAttemptCount);
#endif
}
