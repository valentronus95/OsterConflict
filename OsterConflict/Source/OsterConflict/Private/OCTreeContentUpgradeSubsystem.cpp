#include "OCTreeContentUpgradeSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    const TCHAR* DeciduousTreePath =
        TEXT("/Game/KiteDemo/Environments/Trees/HillTree_02/HillTree_02.HillTree_02");
    const TCHAR* PineTreePath =
        TEXT("/Game/KiteDemo/Environments/Trees/ScotsPine_01/ScotsPine_01.ScotsPine_01");
    const TCHAR* TallPineTreePath =
        TEXT("/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01");

    UInstancedStaticMeshComponent* FindTreeISM(AActor* Actor, const FName Name)
    {
        if (!Actor) return nullptr;
        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return Component;
        }
        return nullptr;
    }

    bool UpgradeTreeFamily(
        UInstancedStaticMeshComponent* Component,
        UStaticMesh* NewMesh,
        int32& OutInstances,
        FString& OutFailure)
    {
        OutInstances = 0;
        if (!Component || !NewMesh)
        {
            OutFailure = TEXT("component_or_imported_mesh_missing");
            return false;
        }

        UStaticMesh* OldMesh = Component->GetStaticMesh();
        if (!OldMesh)
        {
            OutFailure = TEXT("source_tree_mesh_missing");
            return false;
        }

        OutInstances = Component->GetInstanceCount();
        if (OldMesh == NewMesh)
        {
            Component->EmptyOverrideMaterials();
            return true;
        }

        const FBoxSphereBounds OldBounds = OldMesh->GetBounds();
        const FBoxSphereBounds NewBounds = NewMesh->GetBounds();
        const FVector OldSize = OldBounds.BoxExtent * 2.0f;
        const FVector NewSize = NewBounds.BoxExtent * 2.0f;
        const float OldWidth = FMath::Max(OldSize.X, OldSize.Y);
        const float NewWidth = FMath::Max(NewSize.X, NewSize.Y);
        if (OldSize.Z <= 10.0f || NewSize.Z <= 10.0f || OldWidth <= 10.0f || NewWidth <= 10.0f)
        {
            OutFailure = TEXT("tree_bounds_invalid");
            return false;
        }

        TArray<FTransform> RemappedTransforms;
        RemappedTransforms.Reserve(OutInstances);
        for (int32 Index = 0; Index < OutInstances; ++Index)
        {
            FTransform OldTransform;
            if (!Component->GetInstanceTransform(Index, OldTransform, false))
            {
                OutFailure = FString::Printf(TEXT("instance_transform_read_failed_%d"), Index);
                return false;
            }

            const FVector OldScale = OldTransform.GetScale3D().GetAbs();
            const float DesiredHeight = OldSize.Z * OldScale.Z;
            const float DesiredWidth = OldWidth * FMath::Max(OldScale.X, OldScale.Y);
            const float NewScaleZ = FMath::Clamp(DesiredHeight / NewSize.Z, 0.05f, 20.0f);
            const float NewScaleXY = FMath::Clamp(DesiredWidth / NewWidth, 0.05f, 20.0f);

            const float OldBottomZ = OldTransform.GetLocation().Z +
                (OldBounds.Origin.Z - OldBounds.BoxExtent.Z) * OldScale.Z;
            FVector NewLocation = OldTransform.GetLocation();
            NewLocation.Z = OldBottomZ -
                (NewBounds.Origin.Z - NewBounds.BoxExtent.Z) * NewScaleZ;

            RemappedTransforms.Add(FTransform(
                OldTransform.GetRotation(),
                NewLocation,
                FVector(NewScaleXY, NewScaleXY, NewScaleZ)));
        }

        Component->SetStaticMesh(NewMesh);
        Component->EmptyOverrideMaterials();
        for (int32 Index = 0; Index < RemappedTransforms.Num(); ++Index)
        {
            if (!Component->UpdateInstanceTransform(Index, RemappedTransforms[Index], false, false, true))
            {
                OutFailure = FString::Printf(TEXT("instance_transform_write_failed_%d"), Index);
                return false;
            }
        }
        Component->MarkRenderStateDirty();

        if (Component->GetStaticMesh() != NewMesh || Component->GetInstanceCount() != OutInstances)
        {
            OutFailure = TEXT("tree_upgrade_postcondition_failed");
            return false;
        }
        return true;
    }
}

bool UOCTreeContentUpgradeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCTreeContentUpgradeSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (!InWorld.IsGameWorld()) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    AOCWorldSectorOster* Sector = nullptr;
    int32 SectorCount = 0;
    for (TActorIterator<AOCWorldSectorOster> It(&InWorld); It; ++It)
    {
        Sector = *It;
        ++SectorCount;
    }
    if (SectorCount != 1 || !Sector)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_REGIONAL_TREE_INTAKE_FAIL reason=oster_sector_count_%d runtime_acceptance=0"),
            SectorCount);
        return;
    }

    UStaticMesh* DeciduousMesh = LoadObject<UStaticMesh>(nullptr, DeciduousTreePath);
    UStaticMesh* PineMesh = LoadObject<UStaticMesh>(nullptr, PineTreePath);
    UStaticMesh* TallPineMesh = LoadObject<UStaticMesh>(nullptr, TallPineTreePath);
    if (!DeciduousMesh || !PineMesh || !TallPineMesh)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_REGIONAL_TREE_INTAKE_FAIL reason=imported_tree_asset_load deciduous=%d pine=%d tall_pine=%d runtime_acceptance=0"),
            DeciduousMesh ? 1 : 0,
            PineMesh ? 1 : 0,
            TallPineMesh ? 1 : 0);
        return;
    }

    struct FFamily
    {
        const FName ComponentName;
        UStaticMesh* Mesh;
        const TCHAR* Label;
    };
    const FFamily Families[] =
    {
        { TEXT("AuthoredDeciduousTrees"), DeciduousMesh, TEXT("mixed_deciduous") },
        { TEXT("AuthoredPine01Trees"), PineMesh, TEXT("scots_pine") },
        { TEXT("AuthoredPine03Trees"), TallPineMesh, TEXT("scots_pine_tall") },
    };

    int32 TotalInstances = 0;
    for (const FFamily& Family : Families)
    {
        int32 Instances = 0;
        FString Failure;
        UInstancedStaticMeshComponent* Component = FindTreeISM(Sector, Family.ComponentName);
        if (!UpgradeTreeFamily(Component, Family.Mesh, Instances, Failure))
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_REGIONAL_TREE_INTAKE_FAIL family=%s reason=%s runtime_acceptance=0"),
                Family.Label,
                *Failure);
            return;
        }
        TotalInstances += Instances;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_REGIONAL_TREE_INTAKE_WIRED deciduous=HillTree_02 pine=ScotsPine_01 tall_pine=ScotsPineTall_01 families=3 instances=%d placement_preserved=1 ground_base_preserved=1 height_preserved=1 imported_materials=1 runtime_acceptance=0"),
        TotalInstances);
}
