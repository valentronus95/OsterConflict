#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCRecoveredEnvironmentSubsystem.generated.h"

class AActor;
class UInstancedStaticMeshComponent;
class UStaticMesh;
class USceneComponent;

/**
 * Places selected environment meshes that were restored to Content back into
 * the runtime Oster scene. The subsystem is deliberately visual-only and does
 * not replace authoritative movement, collision, mission or replication code.
 */
UCLASS()
class OSTERCONFLICT_API UOCRecoveredEnvironmentSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void TryPopulate(UWorld& World);
    void Populate(UWorld& World);

    static UInstancedStaticMeshComponent* CreateVisualISM(AActor* Owner, USceneComponent* Root,
        UStaticMesh* Mesh, const FName Name, bool bCollision);
    static void AddFittedInstance(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        const FVector& Location, const FVector& DesiredSizeCm, float YawDegrees);

    int32 AttachAttempts = 0;
    bool bPopulated = false;
};
