#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCRecoveredEnvironmentSubsystem.generated.h"

class AActor;
class UInstancedStaticMeshComponent;
class UStaticMesh;
class USceneComponent;

/** Places restored environment meshes into the compact R13 runtime as visual-only content. */
UCLASS()
class OSTERCONFLICT_API UOCRecoveredEnvironmentSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void Populate(UWorld& World);
    static UInstancedStaticMeshComponent* CreateVisualISM(AActor* Owner, USceneComponent* Root,
        UStaticMesh* Mesh, FName Name);
    static void AddFittedInstance(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        const FVector& Location, const FVector& DesiredSizeCm, float YawDegrees);

    bool bPopulated = false;
};