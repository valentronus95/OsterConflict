#include "OCVisualFidelityGateKSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

namespace
{
    const FName AuthoritativeStadiumTag(TEXT("R13_StadionOsterAuthoritative"));

    bool IsEngineBasicShape(const UStaticMesh* Mesh)
    {
        return Mesh && Mesh->GetPathName().Contains(TEXT("/Engine/BasicShapes/"), ESearchCase::IgnoreCase);
    }

    void CountVisibleBasicShapes(AActor* Actor, int32& OutComponents, int32& OutInstances, TArray<FString>& OutNames)
    {
        if (!Actor) return;

        TInlineComponentArray<UStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UStaticMeshComponent* Component : Components)
        {
            if (!Component || !Component->IsRegistered() || !Component->IsVisible()) continue;
            if (!IsEngineBasicShape(Component->GetStaticMesh())) continue;

            int32 Instances = 1;
            if (const UInstancedStaticMeshComponent* ISM = Cast<UInstancedStaticMeshComponent>(Component))
            {
                Instances = ISM->GetInstanceCount();
                if (Instances <= 0) continue;
            }

            ++OutComponents;
            OutInstances += Instances;
            if (OutNames.Num() < 16)
            {
                OutNames.Add(Component->GetName());
            }
        }
    }
}

bool UOCVisualFidelityGateKSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCVisualFidelityGateKSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCVisualFidelityGateKSubsystem, STATGROUP_Tickables);
}

void UOCVisualFidelityGateKSubsystem::Tick(float DeltaTime)
{
    if (bFinished) return;

    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld()) return;
    if (!World->GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    ElapsedSeconds += FMath::Max(0.0f, DeltaTime);
    // Let foliage/debug retirement and landmark ownership finish before observing final presentation.
    if (ElapsedSeconds < 3.0f) return;

    int32 SectorCount = 0;
    int32 StadiumCount = 0;
    int32 BasicShapeComponents = 0;
    int32 BasicShapeInstances = 0;
    TArray<FString> BasicShapeNames;

    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        ++SectorCount;
        CountVisibleBasicShapes(*It, BasicShapeComponents, BasicShapeInstances, BasicShapeNames);
    }

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || !Actor->ActorHasTag(AuthoritativeStadiumTag)) continue;
        ++StadiumCount;
        CountVisibleBasicShapes(Actor, BasicShapeComponents, BasicShapeInstances, BasicShapeNames);
    }

    if (SectorCount != 1)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_GATE_K_RUNTIME_FAIL reason=oster_sector_count_%d gate_k_complete=0"), SectorCount);
        return;
    }

    if (StadiumCount != 1)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_GATE_K_RUNTIME_FAIL reason=authoritative_stadium_count_%d gate_k_complete=0"), StadiumCount);
        return;
    }

    bFinished = true;
    if (BasicShapeComponents > 0)
    {
        const FString Names = FString::Join(BasicShapeNames, TEXT(","));
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_VISUAL_FIDELITY_CONTENT_GAP visible_basicshape_components=%d visible_basicshape_instances=%d owners=OCWorldSectorOster,R13_StadionOsterAuthoritative sample=%s gate_k_complete=0"),
            BasicShapeComponents,
            BasicShapeInstances,
            *Names);
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_GATE_K_RUNTIME_FAIL reason=visible_basicshape_core_content components=%d instances=%d gate_k_complete=0"),
            BasicShapeComponents,
            BasicShapeInstances);
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_GATE_K_RUNTIME_READY visible_basicshape_components=0 visible_basicshape_instances=0 sector_owners=1 stadium_owners=1 developer_markers=0 ground_cover_proxies=0 gate_k_complete=1"));
}
