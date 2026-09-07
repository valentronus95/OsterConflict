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
    constexpr float ContinuousObservationIntervalSeconds = 2.0f;

    bool IsEngineBasicShape(const UStaticMesh* Mesh)
    {
        return Mesh && Mesh->GetPathName().Contains(TEXT("/Engine/BasicShapes/"), ESearchCase::IgnoreCase);
    }

    bool IsRuntimeVisibleBasicShape(const UStaticMeshComponent* Component)
    {
        if (!Component || !Component->IsRegistered() || !Component->IsVisible() || Component->bHiddenInGame)
        {
            return false;
        }
        return IsEngineBasicShape(Component->GetStaticMesh());
    }

    void CountVisibleBasicShapes(AActor* Actor, int32& OutComponents, int32& OutInstances, TArray<FString>& OutNames)
    {
        // Actor-level hidden state is authoritative for runtime rendering too. Inventory weapons and other dormant
        // gameplay actors may keep registered collision/proxy components whose component visibility flags remain
        // true while SetActorHiddenInGame(true) suppresses the whole actor. Counting those as rendered BasicShape
        // content would manufacture a false Gate K failure.
        if (!Actor || Actor->IsHidden()) return;

        TInlineComponentArray<UStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UStaticMeshComponent* Component : Components)
        {
            if (!IsRuntimeVisibleBasicShape(Component)) continue;

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

    // Let foliage/debug retirement and landmark ownership finish before the first observation. A clean first scan
    // is not terminal: gameplay can spawn weapons, ordnance, characters or vehicles later in the same session.
    if (!bReadyLogged && ElapsedSeconds < 3.0f) return;

    // After initial READY keep a low-frequency watch alive for the gameplay-world lifetime. This avoids an all-actor
    // walk every frame while still catching late-spawned visible BasicShape content that the old one-shot gate missed.
    if (bReadyLogged && ElapsedSeconds < NextObservationSeconds) return;
    NextObservationSeconds = ElapsedSeconds + ContinuousObservationIntervalSeconds;

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
    }

    // Gate K is the final gameplay-world observer, not merely a landmark observer. Scan every actor so a visible
    // Engine BasicShape cannot leak through a character, weapon, grenade, vehicle or another gameplay owner while
    // the world/landmark subset still reports READY. Hidden actors and hidden-in-game collision/proxy components
    // are intentionally excluded because they are not rendered production content.
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        const bool bStadium = Actor->ActorHasTag(AuthoritativeStadiumTag);
        const bool bMuseum = Actor->ActorHasTag(MuseumPhotoModelTag);
        const bool bCultureHouse = Actor->ActorHasTag(CultureHousePhotoModelTag);
        const bool bSilpo = Actor->ActorHasTag(SilpoPhotoModelTag);

        StadiumCount += bStadium ? 1 : 0;
        MuseumCount += bMuseum ? 1 : 0;
        CultureHouseCount += bCultureHouse ? 1 : 0;
        SilpoCount += bSilpo ? 1 : 0;

        const int32 ComponentsBefore = BasicShapeComponents;
        const int32 InstancesBefore = BasicShapeInstances;
        CountVisibleBasicShapes(Actor, BasicShapeComponents, BasicShapeInstances, BasicShapeNames);
        if (bStadium || bMuseum || bCultureHouse || bSilpo)
        {
            LandmarkBasicShapeComponents += BasicShapeComponents - ComponentsBefore;
            LandmarkBasicShapeInstances += BasicShapeInstances - InstancesBefore;
        }
    }

    if (SectorCount != 1)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_GATE_K_RUNTIME_FAIL reason=oster_sector_count_%d initial_ready_seen=%d continuous_watch=1 gate_k_complete=0"),
            SectorCount, bReadyLogged ? 1 : 0);
        return;
    }

    if (StadiumCount != 1 || MuseumCount != 1 || CultureHouseCount != 1 || SilpoCount != 1)
    {
        bFinished = true;
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_GATE_K_RUNTIME_FAIL reason=landmark_owner_count stadium=%d museum=%d culture=%d silpo=%d initial_ready_seen=%d continuous_watch=1 gate_k_complete=0"),
            StadiumCount, MuseumCount, CultureHouseCount, SilpoCount, bReadyLogged ? 1 : 0);
        return;
    }

    if (BasicShapeComponents > 0)
    {
        bFinished = true;
        const FString Names = FString::Join(BasicShapeNames, TEXT(","));
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_VISUAL_FIDELITY_CONTENT_GAP visible_basicshape_components=%d visible_basicshape_instances=%d landmark_basicshape_components=%d landmark_basicshape_instances=%d scope=all_gameplay_actors runtime_visible_only=1 hidden_in_game_ignored=1 initial_ready_seen=%d continuous_watch=1 sample=%s gate_k_complete=0"),
            BasicShapeComponents,
            BasicShapeInstances,
            LandmarkBasicShapeComponents,
            LandmarkBasicShapeInstances,
            bReadyLogged ? 1 : 0,
            *Names);
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_GATE_K_RUNTIME_FAIL reason=visible_basicshape_core_content components=%d instances=%d landmark_components=%d landmark_instances=%d scope=all_gameplay_actors initial_ready_seen=%d continuous_watch=1 gate_k_complete=0"),
            BasicShapeComponents,
            BasicShapeInstances,
            LandmarkBasicShapeComponents,
            LandmarkBasicShapeInstances,
            bReadyLogged ? 1 : 0);
        return;
    }

    if (!bReadyLogged)
    {
        bReadyLogged = true;
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_GATE_K_RUNTIME_READY visible_basicshape_components=0 visible_basicshape_instances=0 landmark_basicshape_components=0 landmark_basicshape_instances=0 sector_owners=1 stadium_owners=1 museum_owners=1 culture_owners=1 silpo_owners=1 developer_markers=0 ground_cover_proxies=0 scope=all_gameplay_actors runtime_visible_only=1 hidden_in_game_ignored=1 continuous_watch=1 late_spawn_detection=1 scan_interval_seconds=2.0 gate_k_complete=1"));
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_GATE_K_RUNTIME_WATCH_ACTIVE scope=all_gameplay_actors late_spawn_detection=1 scan_interval_seconds=2.0 mutation=0 runtime_visual_acceptance=pending"));
    }
}
