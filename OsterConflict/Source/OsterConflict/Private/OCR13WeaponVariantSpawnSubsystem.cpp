#include "OCR13WeaponVariantSpawnSubsystem.h"

#include "OCGameMode.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"
#include "OCWorldSectorOster.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

namespace
{
    constexpr int32 MaxSpawnAttempts = 20;
    constexpr float SpawnRetryDelaySeconds = 0.50f;
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

    // Frontend-only sessions intentionally do not create the Oster world sector or gameplay pickups.
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

    bool bGameplayWorldReady = false;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        if (*It)
        {
            bGameplayWorldReady = true;
            break;
        }
    }

    if (!bGameplayWorldReady)
    {
        if (SpawnAttemptCount < MaxSpawnAttempts)
        {
            ScheduleSpawnAttempt(World, SpawnRetryDelaySeconds);
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("R13 bundled weapon variants: Oster world sector was not ready after %d attempts; no extra pickups created."),
                SpawnAttemptCount);
        }
        return;
    }

    struct FVariantSeed
    {
        TSubclassOf<AOCWeaponBase> WeaponClass;
        FVector Location;
    };

    const FVariantSeed Seeds[] =
    {
        { AOCWeapon_M14::StaticClass(), AOCWorldSectorOster::MuseumAnchor() + FVector(5200.0f, -3200.0f, 80.0f) },
        { AOCWeapon_LeverAction::StaticClass(), AOCWorldSectorOster::ParkAnchor() + FVector(-5000.0f, 4200.0f, 80.0f) },
        { AOCWeapon_MAC10::StaticClass(), AOCWorldSectorOster::StadiumAnchor() + FVector(4200.0f, 4500.0f, 80.0f) },
        { AOCWeapon_Tec9::StaticClass(), AOCWorldSectorOster::CollegeAnchor() + FVector(-3600.0f, -4000.0f, 80.0f) },
    };

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    TArray<AOCWeaponBase*> SpawnedWeapons;
    SpawnedWeapons.Reserve(UE_ARRAY_COUNT(Seeds));

    bool bSpawnedAll = true;
    for (const FVariantSeed& Seed : Seeds)
    {
        AOCWeaponBase* Weapon = World.SpawnActor<AOCWeaponBase>(
            Seed.WeaponClass, Seed.Location, FRotator::ZeroRotator, SpawnParams);
        if (!Weapon)
        {
            bSpawnedAll = false;
            break;
        }

        Weapon->DropToWorldServer(Seed.Location, FRotator::ZeroRotator);
        SpawnedWeapons.Add(Weapon);
    }

    if (!bSpawnedAll || SpawnedWeapons.Num() != UE_ARRAY_COUNT(Seeds))
    {
        // Do not leave a partial set behind and then duplicate successful seeds on the retry.
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
                TEXT("R13 bundled weapon variants: could not create all four pickup variants after %d attempts."),
                SpawnAttemptCount);
        }
        return;
    }

    bSpawnComplete = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13 bundled weapon variants spawned as pickups: 4/4 after %d attempt(s)"), SpawnAttemptCount);
}
