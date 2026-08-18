#include "OCR13VehicleVariantSpawnSubsystem.h"

#include "OCCivilianVehicle.h"
#include "OCGameMode.h"
#include "OCVehicleSpawnPoint.h"
#include "OCWorldSectorOster.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

namespace
{
    constexpr int32 MaxSpawnAttempts = 20;
    constexpr float SpawnRetryDelaySeconds = 0.50f;
}

bool UOCR13VehicleVariantSpawnSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13VehicleVariantSpawnSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (InWorld.GetNetMode() == NM_Client) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    // The standalone frontend intentionally owns no gameplay sector or world vehicles.
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    ScheduleSpawnAttempt(InWorld, 0.25f);
}

void UOCR13VehicleVariantSpawnSubsystem::ScheduleSpawnAttempt(UWorld& World, const float DelaySeconds)
{
    if (bSpawnComplete || SpawnAttemptCount >= MaxSpawnAttempts) return;

    TWeakObjectPtr<UWorld> WeakWorld(&World);
    FTimerHandle Timer;
    World.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* RetryWorld = WeakWorld.Get()) TrySpawnBundledVehicleVariants(*RetryWorld);
        }), FMath::Max(0.05f, DelaySeconds), false);
}

void UOCR13VehicleVariantSpawnSubsystem::TrySpawnBundledVehicleVariants(UWorld& World)
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
                TEXT("R13 bundled vehicle variants: Oster world sector was not ready after %d attempts; no BoxTruck spawn points created."),
                SpawnAttemptCount);
        }
        return;
    }

    struct FTruckSeed
    {
        FVector Location;
        float Yaw;
        float RespawnDelay;
    };

    // Two box trucks reuse roads already present in the source-owned Oster layout and stay away from capture centers.
    const FTruckSeed Seeds[] =
    {
        { AOCWorldSectorOster::CollegeAnchor() + FVector(-11800.0f, 7800.0f, 165.0f), 92.0f, 46.0f },
        { AOCWorldSectorOster::MuseumAnchor() + FVector(13200.0f, -8400.0f, 165.0f), 182.0f, 48.0f },
    };

    struct FPendingTruckSpawn
    {
        AOCVehicleSpawnPoint* SpawnPoint = nullptr;
        FTransform Transform;
    };

    TArray<FPendingTruckSpawn> Pending;
    Pending.Reserve(UE_ARRAY_COUNT(Seeds));

    bool bPreparedAll = true;
    for (const FTruckSeed& Seed : Seeds)
    {
        const FTransform SpawnTransform(FRotator(0.0f, Seed.Yaw, 0.0f), Seed.Location);
        AOCVehicleSpawnPoint* SpawnPoint = World.SpawnActorDeferred<AOCVehicleSpawnPoint>(
            AOCVehicleSpawnPoint::StaticClass(), SpawnTransform, nullptr, nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn, ESpawnActorScaleMethod::MultiplyWithRoot);
        if (!SpawnPoint)
        {
            bPreparedAll = false;
            break;
        }

        // Configure before BeginPlay so the spawn point never emits a default-style vehicle for one frame.
        SpawnPoint->ConfigureRuntime(EOCCivilianVehicleStyle::BoxTruck, Seed.RespawnDelay);
        Pending.Add({ SpawnPoint, SpawnTransform });
    }

    if (!bPreparedAll || Pending.Num() != UE_ARRAY_COUNT(Seeds))
    {
        for (FPendingTruckSpawn& Item : Pending)
        {
            if (IsValid(Item.SpawnPoint)) Item.SpawnPoint->Destroy();
        }

        if (SpawnAttemptCount < MaxSpawnAttempts)
        {
            ScheduleSpawnAttempt(World, SpawnRetryDelaySeconds);
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("R13 bundled vehicle variants: could not prepare both BoxTruck spawn points after %d attempts."),
                SpawnAttemptCount);
        }
        return;
    }

    for (FPendingTruckSpawn& Item : Pending)
    {
        UGameplayStatics::FinishSpawningActor(
            Item.SpawnPoint, Item.Transform, ESpawnActorScaleMethod::MultiplyWithRoot);
    }

    bSpawnComplete = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13 bundled vehicle variants spawned: box trucks=2/2 after %d attempt(s)"), SpawnAttemptCount);
}
