#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCTestArena.generated.h"

class UPointLightComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class OSTERCONFLICT_API AOCTestArena : public AActor
{
    GENERATED_BODY()

public:
    AOCTestArena();

private:
    UPROPERTY()
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> Floor;

    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> WallNorth;

    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> WallSouth;

    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> WallEast;

    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> WallWest;

    UPROPERTY()
    TObjectPtr<UPointLightComponent> ArenaLight;
};
