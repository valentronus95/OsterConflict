#include "OCR13ResidentialRealMeshSubsystem.h"

#include "OCGameMode.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float RestoreDelaySeconds = 2.20f;

    bool IsRealHouseFamily(const FName Name)
    {
        return Name == TEXT("R13_House01") || Name == TEXT("R13_House02");
    }

    bool IsRealHouseExtraFamily(const FName Name)
    {
        const FString Value = Name.ToString();
        return Value.StartsWith(TEXT("R13_House01Extra")) || Value == TEXT("R13_House02Extra");
    }

    bool IsProceduralReplacementFamily(const FName Name)
    {
        const FString Value = Name.ToString();
        return Value.StartsWith(TEXT("R13_OsterBrickHouse")) ||
            Name == TEXT("R13_OsterHousePlinths") ||
            Name == TEXT("R13_OsterHouseWindowTrim") ||
            Name == TEXT("R13_OsterHouseWindowGlass") ||
            Name == TEXT("R13_OsterGreyPitchedRoofs") ||
            Name == TEXT("R13_OsterHousePorches") ||
            Name == TEXT("R13_OsterHouseDoors");
    }
}

bool UOCR13ResidentialRealMeshSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13ResidentialRealMeshSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    // WholeOsterArt creates the complete AdvancedVillagePack houses at 0.80 s. EnvironmentDressing
    // adds their matching extras at 1.60 s. The legacy architecture pass currently hides both at
    // 1.95 s and replaces them with cubes. Run once after that pass and restore the real assets.
    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) RestoreRealResidentialMeshes(*World);
        }), RestoreDelaySeconds, false);
}

void UOCR13ResidentialRealMeshSubsystem::RestoreRealResidentialMeshes(UWorld& World)
{
    int32 RestoredHouseFamilies = 0;
    int32 RestoredExtraFamilies = 0;
    int32 HiddenProceduralFamilies = 0;

    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component) continue;
            const FName Name = Component->GetFName();

            if (IsRealHouseFamily(Name))
            {
                Component->SetVisibility(true, true);
                Component->SetHiddenInGame(false, true);
                // WholeOsterArt authored these families with BlockAll collision. Preserve that tested contract.
                Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                ++RestoredHouseFamilies;
                continue;
            }

            if (IsRealHouseExtraFamily(Name))
            {
                Component->SetVisibility(true, true);
                Component->SetHiddenInGame(false, true);
                ++RestoredExtraFamilies;
                continue;
            }

            if (IsProceduralReplacementFamily(Name))
            {
                Component->SetVisibility(false, true);
                Component->SetHiddenInGame(true, true);
                Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                ++HiddenProceduralFamilies;
            }
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13 residential real-mesh restore: real house families=%d, matching extra families=%d, procedural replacement families hidden=%d."),
        RestoredHouseFamilies, RestoredExtraFamilies, HiddenProceduralFamilies);
}
