#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCCivilianVehicle.h"
#include "OCVehicleSpawnPoint.generated.h"

class AOCVehicleBase;
class USceneComponent;

/** Persistent vehicle respawner. Destroyed wrecks remain for their configured lifetime; a fresh vehicle returns later. */
UCLASS()
class OSTERCONFLICT_API AOCVehicleSpawnPoint : public AActor
{
    GENERATED_BODY()

public:
    AOCVehicleSpawnPoint();
    virtual void BeginPlay() override;

    void ConfigureRuntime(EOCCivilianVehicleStyle NewStyle, float NewRespawnDelaySeconds);
    void ConfigureRespawnDelayRuntime(float NewRespawnDelaySeconds);
    /** Authority-only round reset: remove current vehicle/timers and spawn a fresh vehicle at this seed. */
    void ResetForRoundServer();

private:
    UPROPERTY() TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY() TObjectPtr<AOCVehicleBase> CurrentVehicle;

protected:
    UPROPERTY(EditAnywhere, Category="Vehicle")
    TSubclassOf<AOCVehicleBase> VehicleClass;

    UPROPERTY(EditAnywhere, Category="Vehicle")
    EOCCivilianVehicleStyle VehicleStyle = EOCCivilianVehicleStyle::Wagon;

    UPROPERTY(EditAnywhere, Category="Vehicle", meta=(ClampMin="5.0"))
    float RespawnDelaySeconds = 32.0f;

private:
    FTimerHandle RespawnTimerHandle;

    void SpawnVehicleServer();

    UFUNCTION()
    void HandleVehicleWrecked(AActor* WreckedActor);

    UFUNCTION()
    void HandleVehicleDestroyed(AActor* DestroyedActor);
};
