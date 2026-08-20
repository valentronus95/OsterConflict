#include "OCR13MapPlacementRepairSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float BenchRepairDelaySeconds = 0.95f;
    constexpr float FinalPlacementRepairDelaySeconds = 2.85f;

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

    bool IsCentralParkBenchProxy(const FTransform& Transform, const FVector& ParkAnchor)
    {
        const FVector Scale = Transform.GetScale3D().GetAbs();
        if (!FMath::IsNearlyEqual(Scale.X, 1.80f, 0.08f) ||
            !FMath::IsNearlyEqual(Scale.Y, 0.55f, 0.08f) ||
            !FMath::IsNearlyEqual(Scale.Z, 1.20f, 0.08f))
        {
            return false;
        }

        const FVector Delta = Transform.GetLocation() - ParkAnchor;
        return Delta.Size2D() <= 8000.0f && FMath::Abs(Delta.Z - 60.0f) <= 25.0f;
    }

    bool IsGenericVegetationFamily(const FName Name)
    {
        const FString Value = Name.ToString();
        return Value.StartsWith(TEXT("R13_Tree")) ||
            Value.StartsWith(TEXT("R13_Pine")) ||
            Value.StartsWith(TEXT("R13_CompanionTree")) ||
            Value.StartsWith(TEXT("R13_ExplicitPine")) ||
            Value.StartsWith(TEXT("R13_Shrub")) ||
            Value.StartsWith(TEXT("R13_WetlandReed"));
    }

    bool IsMisalignedPoleAttachment(const FName Name)
    {
        return Name == TEXT("R13_UtilityPoleAddons") ||
            Name == TEXT("R13_UtilityPoleLights") ||
            Name == TEXT("R13_KrushelnytskaPoleAddons") ||
            Name == TEXT("R13_KrushelnytskaPoleLights");
    }

    int32 GroundVegetationComponent(UInstancedStaticMeshComponent* Component)
    {
        if (!Component || !Component->GetStaticMesh() || Component->GetInstanceCount() <= 0) return 0;

        const FBoxSphereBounds Bounds = Component->GetStaticMesh()->GetBounds();
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        int32 Changed = 0;

        for (int32 Index = 0; Index < Component->GetInstanceCount(); ++Index)
        {
            FTransform Transform;
            if (!Component->GetInstanceTransform(Index, Transform, true)) continue;

            const FVector Scale = Transform.GetScale3D().GetAbs();
            FVector Location = Transform.GetLocation();
            const float GroundedZ = -LocalBottom * Scale.Z;
            if (FMath::IsNearlyEqual(Location.Z, GroundedZ, 1.0f)) continue;

            Location.Z = GroundedZ;
            Transform.SetLocation(Location);
            if (Component->UpdateInstanceTransform(Index, Transform, true, false, true)) ++Changed;
        }

        if (Changed > 0) Component->MarkRenderStateDirty();
        return Changed;
    }
}

bool UOCR13MapPlacementRepairSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13MapPlacementRepairSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);

    FTimerHandle BenchTimer;
    InWorld.GetTimerManager().SetTimer(BenchTimer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) RepairCentralParkBenchOrientation(*World);
        }), BenchRepairDelaySeconds, false);

    FTimerHandle FinalPlacementTimer;
    InWorld.GetTimerManager().SetTimer(FinalPlacementTimer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get())
            {
                RepairGeneratedVegetationGrounding(*World);
                SuppressMisalignedPoleAttachments(*World);
            }
        }), FinalPlacementRepairDelaySeconds, false);
}

void UOCR13MapPlacementRepairSubsystem::RepairCentralParkBenchOrientation(UWorld& World)
{
    AOCWorldSectorOster* Sector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        Sector = *It;
        if (Sector) break;
    }
    if (!Sector) return;

    UInstancedStaticMeshComponent* ParkDetails = FindISM(Sector, TEXT("ParkDetails"));
    if (!ParkDetails) return;

    const FVector ParkAnchor = AOCWorldSectorOster::ParkAnchor();
    int32 Reoriented = 0;
    for (int32 Index = 0; Index < ParkDetails->GetInstanceCount(); ++Index)
    {
        FTransform Transform;
        if (!ParkDetails->GetInstanceTransform(Index, Transform, true)) continue;
        if (!IsCentralParkBenchProxy(Transform, ParkAnchor)) continue;

        // ParkFurniture places the backrest along local +Y. The north row therefore keeps yaw 0 so its back points
        // away from the main alley; the south row rotates 180 degrees. Both seating rows now face the path.
        const FVector Delta = Transform.GetLocation() - ParkAnchor;
        const float FacingYaw = Delta.Y >= 0.0f ? 0.0f : 180.0f;
        Transform.SetRotation(FRotator(0.0f, FacingYaw, 0.0f).Quaternion());
        if (ParkDetails->UpdateInstanceTransform(Index, Transform, true, false, true)) ++Reoriented;
    }

    if (Reoriented > 0) ParkDetails->MarkRenderStateDirty();
    UE_LOG(LogTemp, Display,
        TEXT("R13.6 map placement repair: central-park bench proxies reoriented toward main alley=%d."), Reoriented);
}

void UOCR13MapPlacementRepairSubsystem::RepairGeneratedVegetationGrounding(UWorld& World)
{
    int32 GroundedInstances = 0;
    int32 TouchedComponents = 0;

    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component || !IsGenericVegetationFamily(Component->GetFName())) continue;
            const int32 Changed = GroundVegetationComponent(Component);
            if (Changed <= 0) continue;
            GroundedInstances += Changed;
            ++TouchedComponents;
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13.6 map placement repair: grounded vegetation instances=%d across components=%d using real mesh bounds."),
        GroundedInstances, TouchedComponents);
}

void UOCR13MapPlacementRepairSubsystem::SuppressMisalignedPoleAttachments(UWorld& World)
{
    int32 HiddenComponents = 0;
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component || !IsMisalignedPoleAttachment(Component->GetFName())) continue;
            Component->SetVisibility(false, true);
            Component->SetHiddenInGame(true, true);
            ++HiddenComponents;
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13.6 map placement repair: hidden %d unverified pole addon/light components; grounded pole bodies remain visible."),
        HiddenComponents);
}
