#include "OCProductionVehicleVisualGuardSubsystem.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

namespace
{
    constexpr float ValidationDelaySeconds = 1.00f;

    bool IsPlaceholderMaterial(const UMaterialInterface* Material)
    {
        if (!Material) return true;
        const FString Path = Material->GetPathName();
        return Path.Contains(TEXT("BasicShapeMaterial"), ESearchCase::IgnoreCase) ||
            Path.Contains(TEXT("DefaultMaterial"), ESearchCase::IgnoreCase) ||
            Path.Contains(TEXT("WorldGridMaterial"), ESearchCase::IgnoreCase);
    }
}

bool UOCProductionVehicleVisualGuardSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCProductionVehicleVisualGuardSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    InWorld.GetTimerManager().SetTimer(
        ValidationTimer,
        this,
        &UOCProductionVehicleVisualGuardSubsystem::ValidateProductionVisuals,
        ValidationDelaySeconds,
        false);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_PRODUCTION_VEHICLE_VALIDATION_SCHEDULED delay_s=%.2f mutation=0 polling=0"),
        ValidationDelaySeconds);
}

void UOCProductionVehicleVisualGuardSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimer);
    }
    Super::Deinitialize();
}

void UOCProductionVehicleVisualGuardSubsystem::ValidateProductionVisuals()
{
    UWorld* World = GetWorld();
    if (!World) return;

    int32 ProductionComponents = 0;
    int32 UnexpectedOverrides = 0;
    int32 PlaceholderOrMissingSlots = 0;
    bool bHMMWV = false;
    bool bM2 = false;
    bool bBTR4 = false;

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        TInlineComponentArray<UStaticMeshComponent*> MeshComponents;
        Actor->GetComponents(MeshComponents);
        for (UStaticMeshComponent* Component : MeshComponents)
        {
            UStaticMesh* Mesh = Component ? Component->GetStaticMesh() : nullptr;
            if (!Component || !Mesh) continue;

            const FString AssetPath = Mesh->GetPathName();
            if (!AssetPath.StartsWith(TEXT("/Game/Production/"))) continue;

            ++ProductionComponents;
            bHMMWV |= AssetPath.Contains(TEXT("/Vehicles/HMMWV/SM_HMMWV_UA"));
            bM2 |= AssetPath.Contains(TEXT("/Weapons/M2/SM_M2_Browning"));
            bBTR4 |= AssetPath.Contains(TEXT("/Vehicles/BTR4/SM_BTR4_Bucephalus"));

            const int32 MaterialCount = Mesh->GetStaticMaterials().Num();
            if (MaterialCount <= 0)
            {
                ++PlaceholderOrMissingSlots;
                continue;
            }

            for (int32 Slot = 0; Slot < MaterialCount; ++Slot)
            {
                UMaterialInterface* AuthoredMaterial = Mesh->GetMaterial(Slot);
                UMaterialInterface* RuntimeMaterial = Component->GetMaterial(Slot);

                if (RuntimeMaterial != AuthoredMaterial)
                {
                    ++UnexpectedOverrides;
                }
                if (IsPlaceholderMaterial(AuthoredMaterial))
                {
                    ++PlaceholderOrMissingSlots;
                }
            }
        }
    }

    if (UnexpectedOverrides > 0)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_PRODUCTION_VEHICLE_MATERIAL_OVERRIDE_FAIL components=%d unexpected_overrides=%d mutation=0 primary_source_fix_required=1"),
            ProductionComponents,
            UnexpectedOverrides);
    }

    if (PlaceholderOrMissingSlots > 0)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_PRODUCTION_VEHICLE_MATERIAL_GAP components=%d missing_or_placeholder_slots=%d mutation=0 exact_material_ready=0"),
            ProductionComponents,
            PlaceholderOrMissingSlots);
    }

    if (bHMMWV && bM2 && bBTR4 && UnexpectedOverrides == 0 && PlaceholderOrMissingSlots == 0)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_PRODUCTION_VEHICLE_VISUALS_VALIDATED_READY hmmwv=1 m2=1 btr4=1 authored_materials=1 validation_only=1 mutation=0 polling=0"));
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS45_PRODUCTION_VEHICLE_CONTENT_GAP hmmwv=%d m2=%d btr4=%d components=%d overrides=%d material_gaps=%d validation_only=1"),
            bHMMWV ? 1 : 0,
            bM2 ? 1 : 0,
            bBTR4 ? 1 : 0,
            ProductionComponents,
            UnexpectedOverrides,
            PlaceholderOrMissingSlots);
    }
}
