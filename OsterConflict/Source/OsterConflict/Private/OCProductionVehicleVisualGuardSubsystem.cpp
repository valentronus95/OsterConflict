#include "OCProductionVehicleVisualGuardSubsystem.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

namespace
{
    constexpr int32 MaxAuditPasses = 12;
    constexpr float AuditIntervalSeconds = 0.50f;
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
        AuditTimer,
        this,
        &UOCProductionVehicleVisualGuardSubsystem::AuditProductionVisuals,
        AuditIntervalSeconds,
        true,
        0.10f);
}

void UOCProductionVehicleVisualGuardSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AuditTimer);
    }
    Super::Deinitialize();
}

void UOCProductionVehicleVisualGuardSubsystem::AuditProductionVisuals()
{
    UWorld* World = GetWorld();
    if (!World) return;

    ++AuditPass;
    int32 ProductionComponents = 0;
    int32 RestoredComponents = 0;
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

            bool bHasUnexpectedOverride = false;
            const int32 MaterialCount = Mesh->GetStaticMaterials().Num();
            for (int32 Slot = 0; Slot < MaterialCount; ++Slot)
            {
                if (Component->GetMaterial(Slot) != Mesh->GetMaterial(Slot))
                {
                    bHasUnexpectedOverride = true;
                    break;
                }
            }

            if (bHasUnexpectedOverride)
            {
                Component->EmptyOverrideMaterials();
                ++RestoredComponents;
            }
        }
    }

    if (RestoredComponents > 0)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PASS42_PRODUCTION_MATERIALS_RESTORED pass=%d components=%d restored=%d hmmwv=%d m2=%d btr4=%d"),
            AuditPass, ProductionComponents, RestoredComponents,
            bHMMWV ? 1 : 0, bM2 ? 1 : 0, bBTR4 ? 1 : 0);
    }

    if (bHMMWV && bM2 && bBTR4)
    {
        World->GetTimerManager().ClearTimer(AuditTimer);
        UE_LOG(LogTemp, Display,
            TEXT("PASS42_PRODUCTION_VEHICLE_VISUALS_READY hmmwv=1 m2=1 btr4=1 authored_materials=preserved polling=stopped"));
        return;
    }

    if (AuditPass >= MaxAuditPasses)
    {
        World->GetTimerManager().ClearTimer(AuditTimer);
        UE_LOG(LogTemp, Warning,
            TEXT("PASS42_PRODUCTION_VEHICLE_CONTENT_GAP hmmwv=%d m2=%d btr4=%d passes=%d polling=stopped"),
            bHMMWV ? 1 : 0, bM2 ? 1 : 0, bBTR4 ? 1 : 0, AuditPass);
    }
}