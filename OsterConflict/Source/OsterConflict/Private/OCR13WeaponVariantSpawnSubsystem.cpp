#include "OCR13WeaponVariantSpawnSubsystem.h"

#include "OCGameMode.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

namespace
{
    // Deployment can legitimately stay open for a while. Keep retrying for roughly one minute so the rack is
    // derived from the player's real deployed pawn instead of a guessed team-base coordinate.
    constexpr int32 MaxSpawnAttempts = 180;
    constexpr float SpawnRetryDelaySeconds = 0.35f;
    constexpr int32 WeaponTestCount = 10;
}

bool UOCR13WeaponVariantSpawnSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13WeaponVariantSpawnSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (InWorld.GetNetMode() == NM_Client) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    ScheduleSpawnAttempt(InWorld, 0.25f);
}

void UOCR13WeaponVariantSpawnSubsystem::ScheduleSpawnAttempt(UWorld& World, const float DelaySeconds)
{
    if (bSpawnComplete || SpawnAttemptCount >= MaxSpawnAttempts) return;

    TWeakObjectPtr<UWorld> WeakWorld(&World);
    FTimerHandle Timer;
    World.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* RetryWorld = WeakWorld.Get()) TrySpawnBundledVariants(*RetryWorld);
        }), FMath::Max(0.05f, DelaySeconds), false);
}

void UOCR13WeaponVariantSpawnSubsystem::TrySpawnBundledVariants(UWorld& World)
{
    if (bSpawnComplete) return;
    ++SpawnAttemptCount;

    // A controller exists while the deployment panel is open, but the pawn exists only after the selected team /
    // squad / role / spawn has actually deployed. Anchor the test rack to that real pawn. This guarantees that the
    // weapons appear beside the place where the tester actually materializes, including forward-spawn selections.
    APlayerController* PlayerController = World.GetFirstPlayerController();
    APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        if (SpawnAttemptCount < MaxSpawnAttempts)
        {
            ScheduleSpawnAttempt(World, SpawnRetryDelaySeconds);
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("R13 weapon test rack: no deployed player pawn after %d attempts."), SpawnAttemptCount);
        }
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
        AOCWeapon_MAC10::StaticClass(),
        AOCWeapon_Tec9::StaticClass(),
    };

    const FVector PawnLocation = PlayerPawn->GetActorLocation();
    const FVector Forward = PlayerPawn->GetActorForwardVector().GetSafeNormal2D();
    const FVector Right = PlayerPawn->GetActorRightVector().GetSafeNormal2D();
    const FVector RackOrigin = PawnLocation + Forward * 360.0f;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    TArray<AOCWeaponBase*> SpawnedWeapons;
    SpawnedWeapons.Reserve(WeaponTestCount);

    bool bSpawnedAll = true;
    for (int32 Index = 0; Index < WeaponTestCount; ++Index)
    {
        // Two rows of five, roughly 3.6 m in front of the deployed pawn and spread laterally so pickups do not overlap.
        const int32 Row = Index / 5;
        const int32 Column = Index % 5;
        const float LateralCm = (static_cast<float>(Column) - 2.0f) * 145.0f;
        const float ForwardCm = static_cast<float>(Row) * 170.0f;
        const FVector Location = RackOrigin + Right * LateralCm + Forward * ForwardCm + FVector(0.0f, 0.0f, 90.0f);

        AOCWeaponBase* Weapon = World.SpawnActor<AOCWeaponBase>(
            Classes[Index], Location, PlayerPawn->GetActorRotation(), SpawnParams);
        if (!Weapon)
        {
            bSpawnedAll = false;
            break;
        }

        Weapon->Tags.Add(FName(TEXT("R13_WeaponTestRack")));
        Weapon->DropToWorldServer(Location, PlayerPawn->GetActorRotation());
        SpawnedWeapons.Add(Weapon);
    }

    if (!bSpawnedAll || SpawnedWeapons.Num() != WeaponTestCount)
    {
        for (AOCWeaponBase* Weapon : SpawnedWeapons)
        {
            if (IsValid(Weapon)) Weapon->Destroy();
        }

        if (SpawnAttemptCount < MaxSpawnAttempts)
        {
            ScheduleSpawnAttempt(World, SpawnRetryDelaySeconds);
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("R13 weapon test rack: could not create all %d pickups after %d attempts."),
                WeaponTestCount, SpawnAttemptCount);
        }
        return;
    }

    bSpawnComplete = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13 weapon test rack spawned beside deployed player: %d/%d weapons after %d attempt(s)."),
        SpawnedWeapons.Num(), WeaponTestCount, SpawnAttemptCount);
}
