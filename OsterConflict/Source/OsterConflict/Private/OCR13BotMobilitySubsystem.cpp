#include "OCR13BotMobilitySubsystem.h"

#include "OCAIController.h"
#include "OCBotCharacter.h"
#include "OCCapturePoint.h"
#include "OCHealthComponent.h"
#include "OCPlayerState.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"

bool UOCR13BotMobilitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13BotMobilitySubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || DeltaTime <= 0.0f) return;

    // If proper navigation exists, do nothing. The normal OCAIController MoveTo path is better than this fallback.
    if (UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
    {
        if (NavSystem->GetDefaultNavDataInstance(FNavigationSystem::DontCreate)) return;
    }

    for (TActorIterator<AOCBotCharacter> It(World); It; ++It)
    {
        AOCBotCharacter* Bot = *It;
        if (!Bot || Bot->IsInVehicle() || Bot->IsDowned()) continue;
        if (!Bot->GetHealthComponent() || !Bot->GetHealthComponent()->IsAlive()) continue;

        AOCAIController* AI = Cast<AOCAIController>(Bot->GetController());
        if (!AI || AI->GetBrainState() != EOCBotBrainState::Objective) continue;

        const AOCPlayerState* BotState = Bot->GetPlayerState<AOCPlayerState>();
        if (!BotState) continue;

        AOCCapturePoint* BestPoint = nullptr;
        float BestDistanceSq = TNumericLimits<float>::Max();
        for (TActorIterator<AOCCapturePoint> PointIt(World); PointIt; ++PointIt)
        {
            AOCCapturePoint* Point = *PointIt;
            if (!Point) continue;
            // Prefer neutral/enemy objectives so squads actually leave spawn and create a battle line.
            if (Point->GetOwnerTeam() == BotState->GetTeamId() && !Point->IsContested()) continue;
            const float DistanceSq = FVector::DistSquared2D(Bot->GetActorLocation(), Point->GetActorLocation());
            if (DistanceSq < BestDistanceSq)
            {
                BestDistanceSq = DistanceSq;
                BestPoint = Point;
            }
        }

        if (!BestPoint)
        {
            for (TActorIterator<AOCCapturePoint> PointIt(World); PointIt; ++PointIt)
            {
                AOCCapturePoint* Point = *PointIt;
                if (!Point) continue;
                const float DistanceSq = FVector::DistSquared2D(Bot->GetActorLocation(), Point->GetActorLocation());
                if (DistanceSq < BestDistanceSq)
                {
                    BestDistanceSq = DistanceSq;
                    BestPoint = Point;
                }
            }
        }

        if (!BestPoint || BestDistanceSq < FMath::Square(450.0f)) continue;
        FVector Direction = BestPoint->GetActorLocation() - Bot->GetActorLocation();
        Direction.Z = 0.0f;
        if (!Direction.Normalize()) continue;

        // Character movement consumes input every frame; this fixes the old 0.2 s "single pulse" fallback.
        Bot->AddMovementInput(Direction, 1.0f, true);
    }
}

TStatId UOCR13BotMobilitySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13BotMobilitySubsystem, STATGROUP_Tickables);
}
