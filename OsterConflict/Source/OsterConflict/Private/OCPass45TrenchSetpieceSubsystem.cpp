#include "OCPass45TrenchSetpieceSubsystem.h"

#include "OCGameMode.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    bool LooksLikeSandbag(const FString& RawText)
    {
        const FString Text = RawText.ToLower();
        return Text.Contains(TEXT("sandbag")) ||
            Text.Contains(TEXT("sand_bag")) ||
            Text.Contains(TEXT("sand-bag")) ||
            Text.Contains(TEXT("sand bag")) ||
            Text.Contains(TEXT("sandbags")) ||
            Text.Contains(TEXT("sand_bags")) ||
            Text.Contains(TEXT("sand-bags")) ||
            Text.Contains(TEXT("sand bags"));
    }

    bool IsSandbagActor(const AActor* Actor)
    {
        if (!IsValid(Actor)) return false;
        if (LooksLikeSandbag(Actor->GetName())) return true;
        for (const FName& Tag : Actor->Tags)
        {
            if (LooksLikeSandbag(Tag.ToString())) return true;
        }
        return false;
    }

    bool IsSandbagComponent(const UStaticMeshComponent* Component)
    {
        if (!IsValid(Component)) return false;
        if (LooksLikeSandbag(Component->GetName())) return true;
        const UStaticMesh* Mesh = Component->GetStaticMesh();
        return IsValid(Mesh) && (LooksLikeSandbag(Mesh->GetName()) || LooksLikeSandbag(Mesh->GetPathName()));
    }
}

bool UOCPass45TrenchSetpieceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCPass45TrenchSetpieceSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCPass45TrenchSetpieceSubsystem, STATGROUP_Tickables);
}

void UOCPass45TrenchSetpieceSubsystem::Tick(float DeltaTime)
{
    if (bFinished) return;

    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld()) return;
    if (!World->GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    // Retire existing sandbag actors/components once. Some authored/runtime sandbag actors are built from generic
    // Cube/static-mesh components, so component-only name matching leaves them visible. Actor name/tag matching owns
    // that case without turning this subsystem back into a permanent world watcher.
    int32 RetiredComponents = 0;
    int32 RetiredActors = 0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!IsValid(Actor) || Actor->IsActorBeingDestroyed()) continue;

        const bool bSandbagActor = IsSandbagActor(Actor);
        TInlineComponentArray<UStaticMeshComponent*> StaticMeshes;
        Actor->GetComponents(StaticMeshes);
        int32 ActorSandbagComponents = 0;
        for (UStaticMeshComponent* Component : StaticMeshes)
        {
            if (!bSandbagActor && !IsSandbagComponent(Component)) continue;

            ++ActorSandbagComponents;
            ++RetiredComponents;
            Component->SetVisibility(false, true);
            Component->SetHiddenInGame(true, true);
            Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Component->SetGenerateOverlapEvents(false);
            Component->SetCastShadow(false);
            Component->SetCanEverAffectNavigation(false);
            Component->SetComponentTickEnabled(false);
        }

        if (bSandbagActor || (ActorSandbagComponents > 0 && ActorSandbagComponents == StaticMeshes.Num()))
        {
            Actor->SetActorHiddenInGame(true);
            Actor->SetActorEnableCollision(false);
            Actor->SetActorTickEnabled(false);
            ++RetiredActors;
        }
    }

    bFinished = true;
    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_TRENCH_SETPIECE_RETIRED authored_spawn_instances=0 existing_sandbag_components_retired=%d existing_sandbag_actors_retired=%d actor_name_tag_matching=1 permanent_scan=0 runtime_acceptance=0"),
        RetiredComponents,
        RetiredActors);
}
