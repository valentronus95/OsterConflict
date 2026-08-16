#include "OCR13WholeOsterArtSubsystem.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    bool IsNoCollisionProxy(const FName Name)
    {
        static const TSet<FName> NoCollision =
        {
            TEXT("ResidentialDetails"), TEXT("TreeCrowns"), TEXT("SovietPoplarCrowns"),
            TEXT("BirchCrowns"), TEXT("PineCrowns"), TEXT("GrassMown"), TEXT("GrassRough"), TEXT("GrassWetland")
        };
        return NoCollision.Contains(Name);
    }

    bool IsWholeOsterProxy(const FName Name)
    {
        static const TSet<FName> WholeOster =
        {
            TEXT("Sidewalks"), TEXT("Buildings"), TEXT("ResidentialRoofs"), TEXT("ResidentialDetails"),
            TEXT("Fences"), TEXT("WoodFences"), TEXT("MetalFences"), TEXT("LightSheetFences"),
            TEXT("TreeTrunks"), TEXT("TreeCrowns"), TEXT("SovietPoplarTrunks"), TEXT("SovietPoplarCrowns"),
            TEXT("BirchTrunks"), TEXT("BirchCrowns"), TEXT("PineTrunks"), TEXT("PineCrowns"),
            TEXT("GrassMown"), TEXT("GrassRough"), TEXT("GrassWetland")
        };
        return WholeOster.Contains(Name);
    }

    bool IsRejectedFantasySliceProp(const FName Name)
    {
        const FString Value = Name.ToString();
        return Value.StartsWith(TEXT("R12_Fence")) || Value == TEXT("R12_StreetLights");
    }
}

bool UOCR13WholeOsterArtSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    if (!World) return false;
    return World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE;
}

void UOCR13WholeOsterArtSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    // R12's visual slice is created after GameMode BeginPlay. Run after it so the whole-city proxy families can be
    // restored and the rejected fantasy props hidden deterministically.
    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ApplyWholeOsterBridge(*World);
        }), 0.70f, false);
}

void UOCR13WholeOsterArtSubsystem::ApplyWholeOsterBridge(UWorld& World)
{
    int32 RestoredFamilies = 0;
    int32 HiddenFantasyFamilies = 0;

    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        const bool bWorldSector = Actor->GetClass()->GetName().Contains(TEXT("OCWorldSectorOster"));
        TInlineComponentArray<UActorComponent*> Components;
        Actor->GetComponents(Components);

        for (UActorComponent* RawComponent : Components)
        {
            UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(RawComponent);
            if (!Primitive) continue;
            const FName Name = Primitive->GetFName();

            if (bWorldSector && IsWholeOsterProxy(Name))
            {
                Primitive->SetVisibility(true, true);
                Primitive->SetCollisionEnabled(IsNoCollisionProxy(Name)
                    ? ECollisionEnabled::NoCollision
                    : ECollisionEnabled::QueryAndPhysics);
                ++RestoredFamilies;
            }

            if (IsRejectedFantasySliceProp(Name))
            {
                Primitive->SetVisibility(false, true);
                Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                ++HiddenFantasyFamilies;
            }
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13 whole-Oster bridge: restored %d city proxy families; hid %d rejected fantasy fence/lamp families."),
        RestoredFamilies, HiddenFantasyFamilies);
}
