#include "OCVehicleSpawnPoint.h"

#include "OCVehicleBase.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

AOCVehicleSpawnPoint::AOCVehicleSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);
    VehicleClass = AOCCivilianVehicle::StaticClass();
}

void AOCVehicleSpawnPoint::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
    {
        SpawnVehicleServer();
    }
}

void AOCVehicleSpawnPoint::ConfigureRuntime(EOCCivilianVehicleStyle NewStyle, float NewRespawnDelaySeconds)
{
    VehicleStyle = NewStyle;
    RespawnDelaySeconds = FMath::Max(5.0f, NewRespawnDelaySeconds);
    if (AOCCivilianVehicle* Civilian = Cast<AOCCivilianVehicle>(CurrentVehicle))
    {
        Civilian->SetVehicleStyleServer(VehicleStyle);
    }
}

void AOCVehicleSpawnPoint::ConfigureRespawnDelayRuntime(float NewRespawnDelaySeconds)
{
    RespawnDelaySeconds = FMath::Max(5.0f, NewRespawnDelaySeconds);
}

void AOCVehicleSpawnPoint::ResetForRoundServer()
{
    if (!HasAuthority()) return;
    GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
    if (CurrentVehicle)
    {
        CurrentVehicle->OnVehicleWrecked.RemoveDynamic(this, &AOCVehicleSpawnPoint::HandleVehicleWrecked);
        CurrentVehicle->OnDestroyed.RemoveDynamic(this, &AOCVehicleSpawnPoint::HandleVehicleDestroyed);
        CurrentVehicle->Destroy();
        CurrentVehicle = nullptr;
    }
    SpawnVehicleServer();
}

void AOCVehicleSpawnPoint::SpawnVehicleServer()
{
    if (!HasAuthority() || CurrentVehicle || !VehicleClass || !GetWorld())
    {
        return;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    AOCVehicleBase* Vehicle = GetWorld()->SpawnActor<AOCVehicleBase>(VehicleClass, GetActorTransform(), Params);
    if (!Vehicle)
    {
        GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AOCVehicleSpawnPoint::SpawnVehicleServer, 5.0f, false);
        return;
    }

    CurrentVehicle = Vehicle;
    if (AOCCivilianVehicle* Civilian = Cast<AOCCivilianVehicle>(Vehicle))
    {
        Civilian->SetVehicleStyleServer(VehicleStyle);
    }
    Vehicle->OnVehicleWrecked.AddDynamic(this, &AOCVehicleSpawnPoint::HandleVehicleWrecked);
    Vehicle->OnDestroyed.AddDynamic(this, &AOCVehicleSpawnPoint::HandleVehicleDestroyed);
}

void AOCVehicleSpawnPoint::HandleVehicleWrecked(AActor* WreckedActor)
{
    if (!HasAuthority() || WreckedActor != CurrentVehicle)
    {
        return;
    }
    CurrentVehicle = nullptr;
    GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AOCVehicleSpawnPoint::SpawnVehicleServer,
        RespawnDelaySeconds, false);
}

void AOCVehicleSpawnPoint::HandleVehicleDestroyed(AActor* DestroyedActor)
{
    if (!HasAuthority() || DestroyedActor != CurrentVehicle)
    {
        return;
    }
    CurrentVehicle = nullptr;
    GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AOCVehicleSpawnPoint::SpawnVehicleServer,
        RespawnDelaySeconds, false);
}
