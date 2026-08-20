#include "OCR141MuseumWindowReplacementSubsystem.h"

#include "OCBreakableWindow.h"
#include "OCGameMode.h"
#include "OCMuseumBreakableWindow.h"
#include "OCWorldSectorOster.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    constexpr float R141WindowReplacementDelaySeconds = 6.0f;
    constexpr float MuseumWindowRadiusCm = 2800.0f;
}

bool UOCR141MuseumWindowReplacementSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR141MuseumWindowReplacementSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (InWorld.GetNetMode() == NM_Client) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ReplaceMuseumWindows(*World);
        }), R141WindowReplacementDelaySeconds, false);
}

void UOCR141MuseumWindowReplacementSubsystem::ReplaceMuseumWindows(UWorld& World) const
{
    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    const float RadiusSq = FMath::Square(MuseumWindowRadiusCm);

    TArray<TWeakObjectPtr<AOCBreakableWindow>> PrototypeWindows;
    for (TActorIterator<AOCBreakableWindow> It(&World); It; ++It)
    {
        AOCBreakableWindow* Window = *It;
        if (!Window || Cast<AOCMuseumBreakableWindow>(Window)) continue;
        if (FVector::DistSquared2D(Window->GetActorLocation(), Museum) > RadiusSq) continue;
        if (!Window->ActorHasTag(TEXT("R138_MuseumInteractive")) &&
            !Window->ActorHasTag(TEXT("R140_MuseumInteractive")))
        {
            continue;
        }

        // Never heal a window that somehow got shot during the short construction delay.
        if (Window->IsBroken()) continue;
        PrototypeWindows.Add(Window);
    }

    int32 Replaced = 0;
    for (const TWeakObjectPtr<AOCBreakableWindow>& WeakWindow : PrototypeWindows)
    {
        AOCBreakableWindow* Prototype = WeakWindow.Get();
        if (!Prototype) continue;

        FActorSpawnParameters Params;
        Params.Owner = Prototype->GetOwner();
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AOCMuseumBreakableWindow* Styled = World.SpawnActor<AOCMuseumBreakableWindow>(
            AOCMuseumBreakableWindow::StaticClass(), Prototype->GetActorTransform(), Params);
        if (!Styled) continue;

        for (const FName Tag : Prototype->Tags)
        {
            Styled->Tags.AddUnique(Tag);
        }
        Styled->Tags.AddUnique(TEXT("R141_MuseumStyledWindow"));
        Prototype->Destroy();
        ++Replaced;
    }

    UE_LOG(LogTemp, Display,
        TEXT("R14.1 museum windows: replaced %d prototype panes with photo-styled breakable glass/frame actors."),
        Replaced);
}
