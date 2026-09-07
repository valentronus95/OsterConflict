#include "OCPass45AuthoredFenceUpgradeSubsystem.h"

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
    constexpr float UpgradeDelaySeconds = 1.05f;
    constexpr float OwnerResolutionTimeoutSeconds = 6.0f;
    constexpr int32 MaxSegmentsPerSourceFence = 96;

    const TCHAR* FenceMeshPaths[] =
    {
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Meshes/SM_Fence_1_A.SM_Fence_1_A"),
        TEXT("/Game/Street_Props_Pack_V1/Mesh/SM_Fence_1.SM_Fence_1")
    };

    UInstancedStaticMeshComponent* FindISM(AOCWorldSectorOster* Sector, const FName Name)
    {
        if (!Sector) return nullptr;
        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Sector->GetComponents(Components);
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

    UStaticMesh* LoadFirstAvailableFence(FString& OutPath)
    {
        for (const TCHAR* Path : FenceMeshPaths)
        {
            if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Path))
            {
                OutPath = Path;
                return Mesh;
            }
        }
        OutPath.Reset();
        return nullptr;
    }

    struct FFenceSnapshot
    {
        UStaticMesh* Mesh = nullptr;
        TArray<FTransform> Instances;
        TArray<UMaterialInterface*> Materials;
    };

    bool CaptureFenceOwner(UInstancedStaticMeshComponent* Component, FFenceSnapshot& OutSnapshot, FString& OutFailure)
    {
        if (!Component)
        {
            OutFailure = TEXT("fences_owner_missing");
            return false;
        }
        if (Component->GetInstanceCount() <= 0)
        {
            OutFailure = TEXT("fences_owner_empty");
            return false;
        }
        if (!IsEngineCube(Component->GetStaticMesh()))
        {
            OutFailure = FString::Printf(TEXT("fences_source_mesh_not_cube_%s"),
                Component->GetStaticMesh() ? *Component->GetStaticMesh()->GetPathName() : TEXT("null"));
            return false;
        }

        OutSnapshot.Mesh = Component->GetStaticMesh();
        OutSnapshot.Instances.Reserve(Component->GetInstanceCount());
        for (int32 Index = 0; Index < Component->GetInstanceCount(); ++Index)
        {
            FTransform Transform;
            if (!Component->GetInstanceTransform(Index, Transform, false))
            {
                OutFailure = FString::Printf(TEXT("fence_transform_read_failed_%d"), Index);
                return false;
            }
            OutSnapshot.Instances.Add(Transform);
        }

        const int32 MaterialCount = Component->GetNumMaterials();
        OutSnapshot.Materials.Reserve(MaterialCount);
        for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
        {
            OutSnapshot.Materials.Add(Component->GetMaterial(MaterialIndex));
        }
        return true;
    }

    void RestoreFenceOwner(UInstancedStaticMeshComponent* Component, const FFenceSnapshot& Snapshot)
    {
        if (!Component || !Snapshot.Mesh) return;
        Component->ClearInstances();
        Component->SetStaticMesh(Snapshot.Mesh);
        Component->EmptyOverrideMaterials();
        for (int32 MaterialIndex = 0; MaterialIndex < Snapshot.Materials.Num(); ++MaterialIndex)
        {
            Component->SetMaterial(MaterialIndex, Snapshot.Materials[MaterialIndex]);
        }
        for (const FTransform& Transform : Snapshot.Instances)
        {
            Component->AddInstance(Transform, false);
        }
        Component->MarkRenderStateDirty();
    }

    bool BuildFenceSegments(
        const FTransform& SourceTransform,
        const FBoxSphereBounds& SourceBounds,
        const FBoxSphereBounds& AuthoredBounds,
        TArray<FTransform>& OutTransforms)
    {
        const FVector SourceScale = SourceTransform.GetScale3D().GetAbs();
        const FVector SourceNativeSize = SourceBounds.BoxExtent * 2.0f;
        const FVector SourceSize(
            SourceNativeSize.X * SourceScale.X,
            SourceNativeSize.Y * SourceScale.Y,
            SourceNativeSize.Z * SourceScale.Z);
        const FVector AuthoredSize = AuthoredBounds.BoxExtent * 2.0f;
        if (SourceSize.X <= 1.0f || SourceSize.Y <= 1.0f || SourceSize.Z <= 1.0f ||
            AuthoredSize.X <= 1.0f || AuthoredSize.Y <= 1.0f || AuthoredSize.Z <= 1.0f)
        {
            return false;
        }

        const bool bSourceLongAxisY = SourceSize.Y > SourceSize.X;
        const bool bAuthoredLongAxisY = AuthoredSize.Y > AuthoredSize.X;
        const float SourceLength = FMath::Max(SourceSize.X, SourceSize.Y);
        const float SourceWidth = FMath::Min(SourceSize.X, SourceSize.Y);
        const float AuthoredLength = FMath::Max(AuthoredSize.X, AuthoredSize.Y);
        const float AuthoredWidth = FMath::Min(AuthoredSize.X, AuthoredSize.Y);

        const float HeightScale = FMath::Clamp(SourceSize.Z / AuthoredSize.Z, 0.05f, 8.0f);
        const float NaturalSegmentLength = AuthoredLength * HeightScale;
        if (NaturalSegmentLength <= 1.0f) return false;

        const int32 SegmentCount = FMath::Clamp(
            FMath::CeilToInt(SourceLength / NaturalSegmentLength), 1, MaxSegmentsPerSourceFence);
        const float CellLength = SourceLength / static_cast<float>(SegmentCount);
        const float LongAxisScale = FMath::Clamp(CellLength / AuthoredLength, 0.02f, 8.0f);
        const float WidthScale = AuthoredWidth > 1.0f
            ? FMath::Clamp(SourceWidth / AuthoredWidth, HeightScale * 0.45f, HeightScale * 2.25f)
            : HeightScale;

        FRotator NewRotation = SourceTransform.Rotator();
        if (bSourceLongAxisY != bAuthoredLongAxisY)
        {
            NewRotation.Yaw += 90.0f;
        }
        const FQuat NewQuat = NewRotation.Quaternion();
        const FQuat SourceQuat = SourceTransform.GetRotation();
        const FVector SourceAxisLocal = bSourceLongAxisY ? FVector::YAxisVector : FVector::XAxisVector;
        const FVector SourceAxis = SourceQuat.RotateVector(SourceAxisLocal).GetSafeNormal();

        const FVector SourceScaledOrigin(
            SourceBounds.Origin.X * SourceScale.X,
            SourceBounds.Origin.Y * SourceScale.Y,
            SourceBounds.Origin.Z * SourceScale.Z);
        const FVector SourceCenter = SourceTransform.GetLocation() + SourceQuat.RotateVector(SourceScaledOrigin);
        const float SourceBottomZ = SourceTransform.GetLocation().Z +
            (SourceBounds.Origin.Z - SourceBounds.BoxExtent.Z) * SourceScale.Z;

        FVector NewScale;
        if (bAuthoredLongAxisY)
        {
            NewScale = FVector(WidthScale, LongAxisScale, HeightScale);
        }
        else
        {
            NewScale = FVector(LongAxisScale, WidthScale, HeightScale);
        }

        const FVector NewScaledOrigin(
            AuthoredBounds.Origin.X * NewScale.X,
            AuthoredBounds.Origin.Y * NewScale.Y,
            AuthoredBounds.Origin.Z * NewScale.Z);
        const float AuthoredBottomOffsetZ =
            (AuthoredBounds.Origin.Z - AuthoredBounds.BoxExtent.Z) * NewScale.Z;

        for (int32 SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex)
        {
            const float Along = -0.5f * SourceLength +
                (static_cast<float>(SegmentIndex) + 0.5f) * CellLength;
            const FVector DesiredCenter = SourceCenter + SourceAxis * Along;
            FVector NewLocation = DesiredCenter - NewQuat.RotateVector(NewScaledOrigin);
            NewLocation.Z = SourceBottomZ - AuthoredBottomOffsetZ;
            OutTransforms.Add(FTransform(NewQuat, NewLocation, NewScale));
        }
        return true;
    }

    bool ApplyFenceCutover(
        UInstancedStaticMeshComponent* Component,
        UStaticMesh* AuthoredMesh,
        const FFenceSnapshot& Snapshot,
        int32& OutAuthoredSegmentCount)
    {
        if (!Component || !AuthoredMesh || !Snapshot.Mesh) return false;

        const FBoxSphereBounds SourceBounds = Snapshot.Mesh->GetBounds();
        const FBoxSphereBounds AuthoredBounds = AuthoredMesh->GetBounds();
        TArray<FTransform> AuthoredTransforms;
        for (const FTransform& SourceTransform : Snapshot.Instances)
        {
            if (!BuildFenceSegments(SourceTransform, SourceBounds, AuthoredBounds, AuthoredTransforms))
            {
                return false;
            }
        }
        if (AuthoredTransforms.Num() < Snapshot.Instances.Num()) return false;

        Component->ClearInstances();
        Component->SetStaticMesh(AuthoredMesh);
        Component->EmptyOverrideMaterials();
        Component->SetCollisionProfileName(TEXT("BlockAll"));
        for (const FTransform& Transform : AuthoredTransforms)
        {
            if (Component->AddInstance(Transform, false) == INDEX_NONE)
            {
                RestoreFenceOwner(Component, Snapshot);
                return false;
            }
        }
        Component->MarkRenderStateDirty();

        if (Component->GetStaticMesh() != AuthoredMesh ||
            Component->GetInstanceCount() != AuthoredTransforms.Num())
        {
            RestoreFenceOwner(Component, Snapshot);
            return false;
        }

        OutAuthoredSegmentCount = AuthoredTransforms.Num();
        return true;
    }
}

bool UOCPass45AuthoredFenceUpgradeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCPass45AuthoredFenceUpgradeSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCPass45AuthoredFenceUpgradeSubsystem, STATGROUP_Tickables);
}

void UOCPass45AuthoredFenceUpgradeSubsystem::Tick(float DeltaTime)
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
    if (ElapsedSeconds < UpgradeDelaySeconds) return;

    AOCWorldSectorOster* Sector = nullptr;
    int32 SectorCount = 0;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        Sector = *It;
        ++SectorCount;
    }
    if (SectorCount != 1 || !Sector)
    {
        if (ElapsedSeconds < OwnerResolutionTimeoutSeconds) return;
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_FENCE_UPGRADE_FAIL reason=oster_sector_count_%d runtime_acceptance=0"),
            SectorCount);
        return;
    }

    UInstancedStaticMeshComponent* Fences = FindISM(Sector, TEXT("Fences"));
    FFenceSnapshot Snapshot;
    FString Failure;
    if (!CaptureFenceOwner(Fences, Snapshot, Failure))
    {
        if (ElapsedSeconds < OwnerResolutionTimeoutSeconds) return;
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_FENCE_UPGRADE_FAIL reason=%s runtime_acceptance=0"), *Failure);
        return;
    }

    FString FencePath;
    UStaticMesh* FenceMesh = LoadFirstAvailableFence(FencePath);
    if (!FenceMesh)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_FENCE_CONTENT_GAP owner=Fences expected=SM_Fence_1_A_or_SM_Fence_1 runtime_acceptance=0"));
        return;
    }

    int32 SegmentCount = 0;
    if (!ApplyFenceCutover(Fences, FenceMesh, Snapshot, SegmentCount))
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_AUTHORED_FENCE_UPGRADE_FAIL reason=fence_cutover_write_failed rollback=1 runtime_acceptance=0"));
        return;
    }

    bFinished = true;
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_AUTHORED_FENCE_UPGRADE_READY owner=Fences authored_mesh=%s source_runs=%d authored_segments=%d basicshape_fences=0 source_footprint_preserved=1 source_ground_contact_preserved=1 tiled_segments=1 wood_metal_light_sheet_owners_untouched=1 runtime_acceptance=0"),
        *FencePath, Snapshot.Instances.Num(), SegmentCount);
}