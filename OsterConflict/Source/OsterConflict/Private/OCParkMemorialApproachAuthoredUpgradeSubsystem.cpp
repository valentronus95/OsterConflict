#include "OCParkMemorialApproachAuthoredUpgradeSubsystem.h"

#include "OCGameMode.h"
#include "OCPlayerController.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"

namespace
{
    const TCHAR* AuthoredMemorialStepPath =
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Meshes/SM_Curb_1.SM_Curb_1");
    constexpr int32 ExpectedMemorialApproachInstances = 4;

    UInstancedStaticMeshComponent* FindISM(AActor* Actor, const FName Name)
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

    bool IsEngineCube(const UStaticMesh* Mesh)
    {
        return Mesh && Mesh->GetPathName().Contains(TEXT("/Engine/BasicShapes/Cube"), ESearchCase::IgnoreCase);
    }

    bool IsBasicShapeMaterial(const UMaterialInterface* Material)
    {
        return Material && Material->GetPathName().Contains(TEXT("/Engine/BasicShapes/"), ESearchCase::IgnoreCase);
    }

    bool UpgradeMemorialApproach(
        UInstancedStaticMeshComponent* Component,
        UStaticMesh* AuthoredMesh,
        int32& OutInstanceCount,
        FString& OutFailure)
    {
        OutInstanceCount = 0;
        if (!Component || !AuthoredMesh)
        {
            OutFailure = TEXT("park_memorial_approach_component_or_mesh_missing");
            return false;
        }

        if (Component->GetInstanceCount() != ExpectedMemorialApproachInstances)
        {
            OutFailure = FString::Printf(TEXT("park_memorial_approach_instance_count_%d_expected_%d"),
                Component->GetInstanceCount(), ExpectedMemorialApproachInstances);
            return false;
        }

        UStaticMesh* CurrentMesh = Component->GetStaticMesh();
        if (CurrentMesh == AuthoredMesh)
        {
            UMaterialInterface* RuntimeMaterial = Component->GetMaterial(0);
            if (!RuntimeMaterial || IsBasicShapeMaterial(RuntimeMaterial))
            {
                OutFailure = TEXT("park_memorial_approach_authored_material_not_ready");
                return false;
            }
            OutInstanceCount = Component->GetInstanceCount();
            return true;
        }

        if (!IsEngineCube(CurrentMesh))
        {
            OutFailure = FString::Printf(TEXT("park_memorial_approach_unexpected_source_mesh_%s"),
                CurrentMesh ? *CurrentMesh->GetPathName() : TEXT("null"));
            return false;
        }

        const FBoxSphereBounds OldBounds = CurrentMesh->GetBounds();
        const FBoxSphereBounds NewBounds = AuthoredMesh->GetBounds();
        const FVector NewNativeSize = NewBounds.BoxExtent * 2.0f;
        if (NewNativeSize.X <= 1.0f || NewNativeSize.Y <= 1.0f || NewNativeSize.Z <= 1.0f)
        {
            OutFailure = TEXT("park_memorial_approach_authored_bounds_invalid");
            return false;
        }

        TArray<FTransform> OldTransforms;
        OldTransforms.Reserve(ExpectedMemorialApproachInstances);
        for (int32 Index = 0; Index < Component->GetInstanceCount(); ++Index)
        {
            FTransform Transform;
            if (!Component->GetInstanceTransform(Index, Transform, false))
            {
                OutFailure = FString::Printf(TEXT("park_memorial_approach_source_transform_read_failed_%d"), Index);
                return false;
            }
            const FRotator Rotation = Transform.Rotator();
            if (FMath::Abs(Rotation.Pitch) > 0.1f || FMath::Abs(Rotation.Roll) > 0.1f)
            {
                OutFailure = FString::Printf(TEXT("park_memorial_approach_source_tilt_not_supported_%d"), Index);
                return false;
            }
            OldTransforms.Add(Transform);
        }

        const bool bAuthoredLongAxisY = NewNativeSize.Y > NewNativeSize.X;
        Component->SetStaticMesh(AuthoredMesh);
        Component->EmptyOverrideMaterials();

        for (int32 Index = 0; Index < OldTransforms.Num(); ++Index)
        {
            const FTransform& Old = OldTransforms[Index];
            const FVector OldScale = Old.GetScale3D().GetAbs();
            const FVector SourceSize(
                OldBounds.BoxExtent.X * 2.0f * OldScale.X,
                OldBounds.BoxExtent.Y * 2.0f * OldScale.Y,
                OldBounds.BoxExtent.Z * 2.0f * OldScale.Z);
            if (SourceSize.X <= 1.0f || SourceSize.Y <= 1.0f || SourceSize.Z <= 1.0f)
            {
                OutFailure = FString::Printf(TEXT("park_memorial_approach_source_bounds_invalid_%d"), Index);
                return false;
            }

            FRotator Rotation = Old.Rotator();
            FVector NewScale;
            if (bAuthoredLongAxisY)
            {
                Rotation.Yaw -= 90.0f;
                NewScale = FVector(
                    SourceSize.Y / NewNativeSize.X,
                    SourceSize.X / NewNativeSize.Y,
                    SourceSize.Z / NewNativeSize.Z);
            }
            else
            {
                NewScale = FVector(
                    SourceSize.X / NewNativeSize.X,
                    SourceSize.Y / NewNativeSize.Y,
                    SourceSize.Z / NewNativeSize.Z);
            }

            const FQuat SourceRotationQuat = Old.Rotator().Quaternion();
            const FVector OldScaledOrigin(
                OldBounds.Origin.X * OldScale.X,
                OldBounds.Origin.Y * OldScale.Y,
                OldBounds.Origin.Z * OldScale.Z);
            const FVector SourceCenter = Old.GetLocation() + SourceRotationQuat.RotateVector(OldScaledOrigin);
            const float SourceBottomZ = Old.GetLocation().Z +
                (OldBounds.Origin.Z - OldBounds.BoxExtent.Z) * OldScale.Z;

            const FQuat NewRotationQuat = Rotation.Quaternion();
            const FVector NewScaledOrigin(
                NewBounds.Origin.X * NewScale.X,
                NewBounds.Origin.Y * NewScale.Y,
                NewBounds.Origin.Z * NewScale.Z);
            FVector NewLocation = SourceCenter - NewRotationQuat.RotateVector(NewScaledOrigin);
            const float NewBottomOffsetZ =
                (NewBounds.Origin.Z - NewBounds.BoxExtent.Z) * NewScale.Z;
            NewLocation.Z = SourceBottomZ - NewBottomOffsetZ;

            if (!Component->UpdateInstanceTransform(
                Index,
                FTransform(NewRotationQuat, NewLocation, NewScale),
                false,
                false,
                true))
            {
                OutFailure = FString::Printf(TEXT("park_memorial_approach_authored_transform_write_failed_%d"), Index);
                return false;
            }
        }

        Component->MarkRenderStateDirty();
        UMaterialInterface* RuntimeMaterial = Component->GetMaterial(0);
        if (!RuntimeMaterial || IsBasicShapeMaterial(RuntimeMaterial) || Component->GetStaticMesh() != AuthoredMesh)
        {
            OutFailure = TEXT("park_memorial_approach_authored_postcondition_failed");
            return false;
        }

        OutInstanceCount = Component->GetInstanceCount();
        return OutInstanceCount == ExpectedMemorialApproachInstances;
    }
}

bool UOCParkMemorialApproachAuthoredUpgradeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCParkMemorialApproachAuthoredUpgradeSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCParkMemorialApproachAuthoredUpgradeSubsystem, STATGROUP_Tickables);
}

void UOCParkMemorialApproachAuthoredUpgradeSubsystem::Tick(float DeltaTime)
{
    if (bFinished) return;

    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld()) return;
    if (!World->GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController()) return;
    if (PC->IsFrontendMenuVisible() || PC->IsDeploymentPanelVisible() ||
        PC->IsSettingsVisible() || !PC->GetPawn())
    {
        ElapsedSeconds = 0.0f;
        return;
    }

    ElapsedSeconds += FMath::Max(0.0f, DeltaTime);
    if (ElapsedSeconds < 0.80f) return;

    AOCWorldSectorOster* Sector = nullptr;
    int32 SectorCount = 0;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        Sector = *It;
        ++SectorCount;
    }
    if (SectorCount != 1 || !Sector)
    {
        if (ElapsedSeconds < 5.0f) return;
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PARK_MEMORIAL_APPROACH_FAIL reason=oster_sector_count_%d gate_k_complete=0 runtime_acceptance=0"),
            SectorCount);
        return;
    }

    UStaticMesh* StepMesh = LoadObject<UStaticMesh>(nullptr, AuthoredMemorialStepPath);
    if (!StepMesh)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PARK_MEMORIAL_APPROACH_CONTENT_GAP step_mesh_loaded=0 expected=SM_Curb_1 family=ParkMemorialApproach gate_k_complete=0 runtime_acceptance=0"));
        return;
    }

    UInstancedStaticMeshComponent* ParkMemorialApproach = FindISM(Sector, TEXT("ParkMemorialApproach"));
    int32 ApproachInstances = 0;
    FString Failure;
    const bool bApproachReady = UpgradeMemorialApproach(ParkMemorialApproach, StepMesh, ApproachInstances, Failure);
    bFinished = true;

    if (!bApproachReady || ApproachInstances != ExpectedMemorialApproachInstances)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_PARK_MEMORIAL_APPROACH_FAIL family=ParkMemorialApproach ready=%d instances=%d expected=%d reason=%s gate_k_complete=0 runtime_acceptance=0"),
            bApproachReady ? 1 : 0,
            ApproachInstances,
            ExpectedMemorialApproachInstances,
            *Failure);
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_AUTHORED_PARK_MEMORIAL_APPROACH_READY step_mesh=SM_Curb_1 step_pack=Mega_Street_Props_Pack instances=4 semantic_owner=ParkMemorialApproach basicshape_meshes=0 basicshape_material_overrides=0 bounds_aware_box_fit=1 source_bottom_preserved=1 family_scope_exact=1 gate_k_complete=0 runtime_acceptance=0"));
}