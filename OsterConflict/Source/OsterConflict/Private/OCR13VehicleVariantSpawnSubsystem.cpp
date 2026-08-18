#include "OCR13VehicleVariantSpawnSubsystem.h"

#include "OCCivilianVehicle.h"
#include "OCVehicleSpawnPoint.h"
#include "OCWorldSectorOster.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

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

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) SpawnBundledVehicleVariants(*World);
        }), 1.35f, false);
}

void UOCR13VehicleVariantSpawnSubsystem::SpawnBundledVehicleVariants(UWorld& World)
{
    bool bGameplayWorldReady = false;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        if (*It)
        {
            bGameplayWorldReady = true;
            break;
        }
    }
    if (!bGameplayWorldReady) return;

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

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    int32 Spawned = 0;
    for (const FTruckSeed& Seed : Seeds)
    {
        AOCVehicleSpawnPoint* SpawnPoint = World.SpawnActor<AOCVehicleSpawnPoint>(
            AOCVehicleSpawnPoint::StaticClass(), Seed.Location, FRotator(0.0f, Seed.Yaw, 0.0f), SpawnParams);
        if (!SpawnPoint) continue;

        SpawnPoint->ConfigureRuntime(EOCCivilianVehicleStyle::BoxTruck, Seed.RespawnDelay);
        ++Spawned;
    }

    UE_LOG(LogTemp, Display, TEXT("R13 bundled vehicle variants spawned: box trucks=%d/2"), Spawned);
}
