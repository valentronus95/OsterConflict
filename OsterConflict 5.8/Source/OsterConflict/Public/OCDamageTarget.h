#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCDamageTarget.generated.h"

class UOCHealthComponent;
class UStaticMeshComponent;

UCLASS()
class OSTERCONFLICT_API AOCDamageTarget : public AActor
{
    GENERATED_BODY()

public:
    AOCDamageTarget();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> TargetMesh;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UOCHealthComponent> HealthComponent;

    UFUNCTION()
    void HandleDeath();
};
