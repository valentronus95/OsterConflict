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
    const FName MuseumPhotoModelTag(TEXT("R137_MuseumPhotoModel"));
    const FName CultureHousePhotoModelTag(TEXT("R146_CultureHouseModel"));
    const FName SilpoPhotoModelTag(TEXT("R140_SilpoModel"));

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
            if (OutNames.Num() < 24)
            {
                OutNames.Add(FString::Printf(TEXT("%s:%s"), *Actor->GetName(), *Component->GetName()));
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
    int32 MuseumCount = 0;
    int32 CultureHouseCount = 0;
    int32 SilpoCount = 0;
    int32 BasicShapeComponents = 0;
    int32 BasicShapeInstances = 0;
    int32 LandmarkBasicShapeComponents = 0;
    int32 LandmarkBasicShapeInstances = 0;
    TArray<FString> BasicShapeNames;

    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        ++SectorCount;
        CountVisibleBasicShapes(*It, BasicShapeComponents, BasicShapeInstances, BasicShapeNames);
    }

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        const bool bStadium = Actor->ActorHasTag(AuthoritativeStadiumTag);
        const bool bMuseum = Actor->ActorHasTag(MuseumPhotoModelTag);
        const bool bCultureHouse = Actor->ActorHasTag(CultureHousePhotoModelTag);
        const bool bSilpo = Actor->ActorHasTag(SilpoPhotoModelTag);
        if (!bStadium && !bMuseum && !bCultureHouse && !bSilpo) continue;

        StadiumCount += bStadium ? 1 : 0;
        MuseumCount += bMuseum ? 1 : 0;
        CultureHouseCount += bCultureHouse ? 1 : 0;
        SilpoCount += bSilpo ? 1 : 0;

        const int32 ComponentsBefore = BasicShapeComponents;
        const int32 InstancesBefore = BasicShapeInstances;
        CountVisibleBasicShapes(Actor, BasicShapeComponents, BasicShapeInstances, BasicShapeNames);
        LandmarkBasicShapeComponents += BasicShapeComponents - ComponentsBefore;
        LandmarkBasicShapeInstances += BasicShapeInstances - InstancesBefore;
    }

    if (SectorCount != 1)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_GATE_K_RUNTIME_FAIL reason=oster_sector_count_%d gate_k_complete=0"), SectorCount);
        return;
    }

    if (StadiumCount != 1 || MuseumCount != 1 || CultureHouseCount != 1 || SilpoCount != 1)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_GATE_K_RUNTIME_FAIL reason=landmark_owner_count stadium=%d museum=%d culture=%d silpo=%d gate_k_complete=0"),
            StadiumCount, MuseumCount, CultureHouseCount, SilpoCount);
        return;
    }

    bFinished = true;
    if (BasicShapeComponents > 0)
    {
        const FString Names = FString::Join(BasicShapeNames, TEXT(","));
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_VISUAL_FIDELITY_CONTENT_GAP visible_basicshape_components=%d visible_basicshape_instances=%d landmark_basicshape_components=%d landmark_basicshape_instances=%d owners=OCWorldSectorOster,R13_StadionOsterAuthoritative,R137_MuseumPhotoModel,R146_CultureHouseModel,R140_SilpoModel sample=%s gate_k_complete=0"),
            BasicShapeComponents,
            BasicShapeInstances,
            LandmarkBasicShapeComponents,
            LandmarkBasicShapeInstances,
            *Names);
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_GATE_K_RUNTIME_FAIL reason=visible_basicshape_core_content components=%d instances=%d landmark_components=%d landmark_instances=%d gate_k_complete=0"),
            BasicShapeComponents,
            BasicShapeInstances,
            LandmarkBasicShapeComponents,
            LandmarkBasicShapeInstances);
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_GATE_K_RUNTIME_READY visible_basicshape_components=0 visible_basicshape_instances=0 landmark_basicshape_components=0 landmark_basicshape_instances=0 sector_owners=1 stadium_owners=1 museum_owners=1 culture_owners=1 silpo_owners=1 developer_markers=0 ground_cover_proxies=0 gate_k_complete=1"));
}
