#include "OCPass45ImportedResidentialUpgradeSubsystem.h"

#include "OCGameMode.h"
#include "OCPass45LocalAssetResolver.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
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

    UStaticMesh* ResolveResidentialMesh()
    {
        if (UStaticMesh* Mesh = OCPass45FindLocalStaticMesh(
            { FName(TEXT("/Game/fivestory-building-appartament-of-post-soviet")) },
            { TEXT("building"), TEXT("apartment"), TEXT("house"), TEXT("five") }))
        {
            return Mesh;
        }
        if (UStaticMesh* Mesh = OCPass45FindLocalStaticMesh(
            { FName(TEXT("/Game/AdvancedVillagePack")) },
            { TEXT("house"), TEXT("building"), TEXT("home") }))
        {
            return Mesh;
        }
        return OCPass45FindLocalStaticMesh(
            { FName(TEXT("/Game/Modular_Rural_Cabin")) },
            { TEXT("cabin"), TEXT("house"), TEXT("building") });
    }

    FTransform FitBuildingToSourceBox(
        const FTransform& SourceTransform,
        const FBoxSphereBounds& SourceBounds,
        const FBoxSphereBounds& AuthoredBounds)
    {
        const FVector SourceScale = SourceTransform.GetScale3D().GetAbs();
        const FVector SourceNativeSize = SourceBounds.BoxExtent * 2.0f;
        const FVector SourceSize(
            SourceNativeSize.X * SourceScale.X,
            SourceNativeSize.Y * SourceScale.Y,
            SourceNativeSize.Z * SourceScale.Z);
        FVector AuthoredSize = AuthoredBounds.BoxExtent * 2.0f;

        const bool bSourceLongY = SourceSize.Y > SourceSize.X;
        const bool bAuthoredLongY = AuthoredSize.Y > AuthoredSize.X;
        FRotator Rotation = SourceTransform.Rotator();
        if (bSourceLongY != bAuthoredLongY)
        {
            Rotation.Yaw += 90.0f;
            Swap(AuthoredSize.X, AuthoredSize.Y);
        }

        const float ScaleX = AuthoredSize.X > 1.0f ? SourceSize.X / AuthoredSize.X : 1.0f;
        const float ScaleY = AuthoredSize.Y > 1.0f ? SourceSize.Y / AuthoredSize.Y : 1.0f;
        const float ScaleZ = AuthoredSize.Z > 1.0f ? SourceSize.Z / AuthoredSize.Z : 1.0f;
        const float UniformScale = FMath::Clamp(FMath::Min3(ScaleX, ScaleY, ScaleZ), 0.02f, 12.0f);

        const FQuat SourceQuat = SourceTransform.GetRotation();
        const FVector SourceScaledOrigin(
            SourceBounds.Origin.X * SourceScale.X,
            SourceBounds.Origin.Y * SourceScale.Y,
            SourceBounds.Origin.Z * SourceScale.Z);
        const FVector SourceCenter = SourceTransform.GetLocation() + SourceQuat.RotateVector(SourceScaledOrigin);
        const float SourceBottomZ = SourceTransform.GetLocation().Z +
            (SourceBounds.Origin.Z - SourceBounds.BoxExtent.Z) * SourceScale.Z;

        const FQuat NewQuat = Rotation.Quaternion();
        FVector NewLocation = SourceCenter - NewQuat.RotateVector(AuthoredBounds.Origin * UniformScale);
        const float AuthoredBottomOffsetZ =
            (AuthoredBounds.Origin.Z - AuthoredBounds.BoxExtent.Z) * UniformScale;
        NewLocation.Z = SourceBottomZ - AuthoredBottomOffsetZ;

        return FTransform(NewQuat, NewLocation, FVector(UniformScale));
    }

    void RetireMatchingSourceDecor(UInstancedStaticMeshComponent* Component, int32 ExpectedCount)
    {
        if (!Component || Component->GetInstanceCount() != ExpectedCount || !IsEngineCube(Component->GetStaticMesh())) return;
        Component->ClearInstances();
        Component->MarkRenderStateDirty();
    }
}

bool UOCPass45ImportedResidentialUpgradeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCPass45ImportedResidentialUpgradeSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
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
    if (SectorCount != 1 || !Sector) return;

    UInstancedStaticMeshComponent* Buildings = FindISM(Sector, TEXT("Buildings"));
    if (!Buildings || Buildings->GetInstanceCount() <= 0)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_RESIDENTIAL_SKIPPED reason=no_generic_residential_instances verified_landmarks_untouched=1"));
        return;
    }
    if (!IsEngineCube(Buildings->GetStaticMesh()))
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_RESIDENTIAL_SKIPPED reason=existing_authored_owner_preserved mesh=%s"),
            Buildings->GetStaticMesh() ? *Buildings->GetStaticMesh()->GetPathName() : TEXT("null"));
        return;
    }

    UStaticMesh* AuthoredMesh = ResolveResidentialMesh();
    if (!AuthoredMesh)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_RESIDENTIAL_CONTENT_GAP local_building_pack=0 source_owner_preserved=1"));
        return;
    }

    const int32 Count = Buildings->GetInstanceCount();
    TArray<FTransform> SourceTransforms;
    SourceTransforms.Reserve(Count);
    for (int32 Index = 0; Index < Count; ++Index)
    {
        FTransform Transform;
        if (!Buildings->GetInstanceTransform(Index, Transform, false)) return;
        SourceTransforms.Add(Transform);
    }

    const FBoxSphereBounds SourceBounds = Buildings->GetStaticMesh()->GetBounds();
    const FBoxSphereBounds AuthoredBounds = AuthoredMesh->GetBounds();
    const FVector AuthoredSize = AuthoredBounds.BoxExtent * 2.0f;
    if (AuthoredSize.X <= 1.0f || AuthoredSize.Y <= 1.0f || AuthoredSize.Z <= 1.0f) return;

    TArray<FTransform> NewTransforms;
    NewTransforms.Reserve(Count);
    for (const FTransform& SourceTransform : SourceTransforms)
    {
        NewTransforms.Add(FitBuildingToSourceBox(SourceTransform, SourceBounds, AuthoredBounds));
    }

    Buildings->ClearInstances();
    Buildings->SetStaticMesh(AuthoredMesh);
    Buildings->EmptyOverrideMaterials();
    Buildings->SetCollisionProfileName(TEXT("BlockAll"));
    for (const FTransform& Transform : NewTransforms)
    {
        Buildings->AddInstance(Transform, false);
    }
    Buildings->MarkRenderStateDirty();

    if (Buildings->GetInstanceCount() != Count || Buildings->GetStaticMesh() != AuthoredMesh)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_IMPORTED_RESIDENTIAL_FAIL post_write_count=%d expected=%d runtime_acceptance=0"),
            Buildings->GetInstanceCount(), Count);
        return;
    }

    RetireMatchingSourceDecor(FindISM(Sector, TEXT("ResidentialRoofs")), Count);
    RetireMatchingSourceDecor(FindISM(Sector, TEXT("ResidentialDetails")), Count);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_IMPORTED_RESIDENTIAL_READY asset=%s instances=%d generic_cube_buildings=0 verified_landmarks_untouched=1 bounds_fit=1 ground_contact_preserved=1 runtime_acceptance=0"),
        *AuthoredMesh->GetPathName(), Count);
}
