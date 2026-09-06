#include "OCPass45ImportedDoorVisualSubsystem.h"

#include "OCGameMode.h"
#include "OCInteractableDoor.h"
#include "OCPass45LocalAssetResolver.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    const FName LocalDoorVisualTag(TEXT("OC_PASS45_LOCAL_IMPORTED_DOOR"));
    constexpr int32 MaxRefreshPasses = 6;

    UStaticMeshComponent* FindMeshComponent(AActor* Owner, const FName ComponentName)
    {
        if (!Owner) return nullptr;
        TInlineComponentArray<UStaticMeshComponent*> Components;
        Owner->GetComponents(Components);
        for (UStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == ComponentName)
            {
                return Component;
            }
        }
        return nullptr;
    }

    bool HasImportedDoorVisual(const AActor* Owner)
    {
        if (!Owner) return false;
        TInlineComponentArray<UStaticMeshComponent*> Components;
        Owner->GetComponents(Components);
        for (const UStaticMeshComponent* Component : Components)
        {
            if (Component && Component->ComponentHasTag(LocalDoorVisualTag)) return true;
        }
        return false;
    }

    bool IsEngineCube(const UStaticMesh* Mesh)
    {
        return Mesh && Mesh->GetPathName().Contains(TEXT("/Engine/BasicShapes/Cube"), ESearchCase::IgnoreCase);
    }

    UStaticMesh* ResolveExactDoorLeaf()
    {
        const TArray<FString> DoorTokens =
        {
            TEXT("doorleaf"), TEXT("door_leaf"), TEXT("door-panel"), TEXT("door_panel"), TEXT("door")
        };

        const TArray<TArray<FName>> Roots =
        {
            { FName(TEXT("/Game/fivestory-building-appartament-of-post-soviet")) },
            { FName(TEXT("/Game/AdvancedVillagePack")) },
            { FName(TEXT("/Game/Modular_Rural_Cabin")) },
            { FName(TEXT("/Game/Deko_MatrixDemo")) }
        };

        for (const TArray<FName>& Root : Roots)
        {
            if (UStaticMesh* Mesh = OCPass45FindLocalStaticMeshStrict(Root, DoorTokens)) return Mesh;
        }
        return nullptr;
    }

    FVector SafeDivide(const FVector& Value, const FVector& Divisor)
    {
        return FVector(
            FMath::Abs(Divisor.X) > KINDA_SMALL_NUMBER ? Value.X / Divisor.X : Value.X,
            FMath::Abs(Divisor.Y) > KINDA_SMALL_NUMBER ? Value.Y / Divisor.Y : Value.Y,
            FMath::Abs(Divisor.Z) > KINDA_SMALL_NUMBER ? Value.Z / Divisor.Z : Value.Z);
    }

    bool ApplyDoorLeafVisual(AOCInteractableDoor* Door, UStaticMesh* AuthoredMesh)
    {
        if (!Door || !AuthoredMesh || HasImportedDoorVisual(Door)) return false;

        UStaticMeshComponent* DoorLeaf = FindMeshComponent(Door, FName(TEXT("DoorLeaf")));
        if (!DoorLeaf || !DoorLeaf->GetStaticMesh()) return false;

        // Never override another authored door owner. This bridge only retires the old BasicShape leaf render.
        if (!IsEngineCube(DoorLeaf->GetStaticMesh())) return false;

        const FBoxSphereBounds SourceBounds = DoorLeaf->GetStaticMesh()->GetBounds();
        const FVector SourceScale = DoorLeaf->GetRelativeScale3D().GetAbs();
        const FVector SourceNativeSize = SourceBounds.BoxExtent * 2.0f;
        const float TargetWidthCm = SourceNativeSize.X * SourceScale.X;
        const float TargetHeightCm = SourceNativeSize.Z * SourceScale.Z;

        const FBoxSphereBounds AuthoredBounds = AuthoredMesh->GetBounds();
        const FVector AuthoredSize = AuthoredBounds.BoxExtent * 2.0f;
        const float AuthoredWidthCm = FMath::Max(AuthoredSize.X, AuthoredSize.Y);
        const float AuthoredHeightCm = AuthoredSize.Z;
        if (TargetWidthCm <= 1.0f || TargetHeightCm <= 1.0f ||
            AuthoredWidthCm <= 1.0f || AuthoredHeightCm <= 1.0f)
        {
            return false;
        }

        const float UniformScale = FMath::Clamp(
            FMath::Min(TargetWidthCm / AuthoredWidthCm, TargetHeightCm / AuthoredHeightCm),
            0.02f, 12.0f);
        const float AxisYaw = AuthoredSize.Y > AuthoredSize.X ? 90.0f : 0.0f;
        const FQuat AxisCorrection = FRotator(0.0f, AxisYaw, 0.0f).Quaternion();

        UStaticMeshComponent* Visual = NewObject<UStaticMeshComponent>(
            Door,
            MakeUniqueObjectName(Door, UStaticMeshComponent::StaticClass(), FName(TEXT("Pass45ImportedDoorVisual"))));
        if (!Visual) return false;

        Visual->SetupAttachment(DoorLeaf);
        Visual->SetStaticMesh(AuthoredMesh);
        Visual->SetAbsolute(false, false, true);
        const FVector DesiredCenterOffset = -AxisCorrection.RotateVector(AuthoredBounds.Origin) * UniformScale;
        Visual->SetRelativeLocation(SafeDivide(DesiredCenterOffset, SourceScale));
        Visual->SetRelativeRotation(AxisCorrection.Rotator());
        Visual->SetRelativeScale3D(FVector(UniformScale));
        Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Visual->SetGenerateOverlapEvents(false);
        Visual->SetCanEverAffectNavigation(false);
        Visual->SetCastShadow(true);
        Visual->ComponentTags.AddUnique(LocalDoorVisualTag);
        Door->AddInstanceComponent(Visual);
        Visual->RegisterComponent();

        // Keep the original leaf as the replicated hinge/collision owner, but never render its cube anymore.
        DoorLeaf->SetVisibility(false, false);
        DoorLeaf->SetHiddenInGame(true, false);
        DoorLeaf->SetCastShadow(false);
        if (UStaticMeshComponent* Handle = FindMeshComponent(Door, FName(TEXT("Handle"))))
        {
            if (IsEngineCube(Handle->GetStaticMesh()))
            {
                Handle->SetVisibility(false, false);
                Handle->SetHiddenInGame(true, false);
                Handle->SetCastShadow(false);
            }
        }

        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_DOOR_VISUAL_READY actor=%s asset=%s target_width_cm=%.1f target_height_cm=%.1f authored_leaf=1 replicated_hinge_preserved=1 collision_owner_preserved=1 second_interaction_system=0 runtime_acceptance=0"),
            *Door->GetName(), *AuthoredMesh->GetPathName(), TargetWidthCm, TargetHeightCm);
        return true;
    }
}

bool UOCPass45ImportedDoorVisualSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCPass45ImportedDoorVisualSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    InWorld.GetTimerManager().SetTimer(
        RefreshTimer,
        this,
        &UOCPass45ImportedDoorVisualSubsystem::RefreshDoorVisuals,
        0.50f,
        true,
        0.25f);
}

void UOCPass45ImportedDoorVisualSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(RefreshTimer);
    RefreshPass = 0;
    Super::Deinitialize();
}

void UOCPass45ImportedDoorVisualSubsystem::RefreshDoorVisuals()
{
    UWorld* World = GetWorld();
    if (!World) return;
    ++RefreshPass;

    UStaticMesh* DoorMesh = ResolveExactDoorLeaf();
    int32 DoorCount = 0;
    int32 Applied = 0;
    for (TActorIterator<AOCInteractableDoor> It(World); It; ++It)
    {
        AOCInteractableDoor* Door = *It;
        if (!Door || Door->IsActorBeingDestroyed()) continue;
        ++DoorCount;
        if (DoorMesh && ApplyDoorLeafVisual(Door, DoorMesh)) ++Applied;
    }

    if (!DoorMesh && RefreshPass == 1)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_DOOR_CONTENT_GAP exact_local_door_mesh=0 existing_interaction_preserved=1 cube_collision_owner_preserved=1"));
    }
    if (Applied > 0)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_DOOR_VISUAL_PASS pass=%d doors=%d applied=%d second_interaction_system=0"),
            RefreshPass, DoorCount, Applied);
    }

    if (RefreshPass >= MaxRefreshPasses)
    {
        World->GetTimerManager().ClearTimer(RefreshTimer);
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_IMPORTED_DOOR_VISUAL_STOPPED passes=%d permanent_scan=0 runtime_acceptance=0"),
            RefreshPass);
    }
}
