#include "OCR13AccessibilitySubsystem.h"

#include "OCWorldSectorOster.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

bool UOCR13AccessibilitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13AccessibilitySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    // GameMode creates the source-built Oster sector during BeginPlay, so run just after that spawn pass.
    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) RepairMuseumEntranceSteps(*World);
        }), 0.80f, false);
}

void UOCR13AccessibilitySubsystem::RepairMuseumEntranceSteps(UWorld& World)
{
    AOCWorldSectorOster* Sector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        Sector = *It;
        break;
    }
    if (!Sector) return;

    UInstancedStaticMeshComponent* LandmarkDetails = nullptr;
    TArray<UInstancedStaticMeshComponent*> Components;
    Sector->GetComponents<UInstancedStaticMeshComponent>(Components);
    for (UInstancedStaticMeshComponent* Component : Components)
    {
        if (Component && Component->GetFName() == TEXT("LandmarkDetails"))
        {
            LandmarkDetails = Component;
            break;
        }
    }
    if (!LandmarkDetails) return;

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    TArray<int32> StepIndices;
    StepIndices.SetNum(4);
    for (int32& Index : StepIndices) Index = INDEX_NONE;

    // The source generator accidentally put the highest/narrowest step farthest from the entrance.
    // Locate those exact four instances before moving anything, then reverse only their Y ordering.
    for (int32 Step = 0; Step < 4; ++Step)
    {
        const FVector ExpectedOld = Museum + FVector(1180.0f, -1880.0f - Step * 120.0f, 22.0f + Step * 20.0f);
        float BestDistanceSq = FMath::Square(12.0f);

        for (int32 InstanceIndex = 0; InstanceIndex < LandmarkDetails->GetInstanceCount(); ++InstanceIndex)
        {
            FTransform InstanceTransform;
            if (!LandmarkDetails->GetInstanceTransform(InstanceIndex, InstanceTransform, false)) continue;
            const float DistanceSq = FVector::DistSquared(InstanceTransform.GetLocation(), ExpectedOld);
            if (DistanceSq < BestDistanceSq)
            {
                BestDistanceSq = DistanceSq;
                StepIndices[Step] = InstanceIndex;
            }
        }
    }

    for (int32 Step = 0; Step < StepIndices.Num(); ++Step)
    {
        if (StepIndices[Step] == INDEX_NONE)
        {
            UE_LOG(LogTemp, Warning, TEXT("R13 accessibility: museum stair instance %d was not found."), Step);
            return;
        }
    }

    for (int32 Step = 0; Step < StepIndices.Num(); ++Step)
    {
        FTransform InstanceTransform;
        if (!LandmarkDetails->GetInstanceTransform(StepIndices[Step], InstanceTransform, false)) continue;
        InstanceTransform.SetLocation(Museum + FVector(1180.0f, -2240.0f + Step * 120.0f, 22.0f + Step * 20.0f));
        LandmarkDetails->UpdateInstanceTransform(
            StepIndices[Step], InstanceTransform, false, Step == StepIndices.Num() - 1, true);
    }

    UE_LOG(LogTemp, Display, TEXT("R13 accessibility: corrected Oster museum entrance step order."));
}
