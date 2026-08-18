#include "OCR13BotMobilitySubsystem.h"

#include "OCAIController.h"
#include "OCBotCharacter.h"
#include "OCCapturePoint.h"
#include "OCHealthComponent.h"
#include "OCPlayerState.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"

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

    TArray<AOCBotCharacter*> MobileBots;
    for (TActorIterator<AOCBotCharacter> It(World); It; ++It)
    {
        AOCBotCharacter* Bot = *It;
        if (!Bot || Bot->IsInVehicle() || Bot->IsDowned()) continue;
        if (!Bot->GetHealthComponent() || !Bot->GetHealthComponent()->IsAlive()) continue;
        MobileBots.Add(Bot);
    }

    for (AOCBotCharacter* Bot : MobileBots)
    {
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

        FVector BaseDirection = BestPoint->GetActorLocation() - Bot->GetActorLocation();
        BaseDirection.Z = 0.0f;
        if (!BaseDirection.Normalize()) continue;

        // The old fallback sent every bot to the exact same point, producing a single-file parade.
        // Give each bot a stable lateral approach lane around the objective before local separation is applied.
        const FVector Right(-BaseDirection.Y, BaseDirection.X, 0.0f);
        const uint32 StableHash = GetTypeHash(BotState->GetPlayerName());
        const int32 LaneIndex = static_cast<int32>(StableHash % 7u) - 3;
        const float LaneOffsetCm = static_cast<float>(LaneIndex) * 185.0f;
        const FVector ApproachPoint = BestPoint->GetActorLocation() + Right * LaneOffsetCm;

        FVector DesiredDirection = ApproachPoint - Bot->GetActorLocation();
        DesiredDirection.Z = 0.0f;
        if (!DesiredDirection.Normalize()) continue;

        // Cheap source-map avoidance for the no-NavMesh fallback. Fifteen bots means O(n^2) here is tiny,
        // while a 3.5 m personal-space radius is enough to stop overlapping bodies and rail-like columns.
        FVector Separation = FVector::ZeroVector;
        constexpr float SeparationRadiusCm = 350.0f;
        for (AOCBotCharacter* Other : MobileBots)
        {
            if (!Other || Other == Bot) continue;
            const AOCPlayerState* OtherState = Other->GetPlayerState<AOCPlayerState>();
            if (!OtherState || OtherState->GetTeamId() != BotState->GetTeamId()) continue;

            FVector Away = Bot->GetActorLocation() - Other->GetActorLocation();
            Away.Z = 0.0f;
            const float Distance = Away.Size();
            if (Distance <= KINDA_SMALL_NUMBER || Distance >= SeparationRadiusCm) continue;

            Away /= Distance;
            Separation += Away * (1.0f - Distance / SeparationRadiusCm);
        }

        FVector FinalDirection = DesiredDirection + Separation * 1.25f;
        FinalDirection.Z = 0.0f;
        if (!FinalDirection.Normalize()) FinalDirection = DesiredDirection;

        // Character movement consumes input every frame; this fixes the old 0.2 s "single pulse" fallback.
        Bot->AddMovementInput(FinalDirection, 1.0f, true);
    }
}

TStatId UOCR13BotMobilitySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13BotMobilitySubsystem, STATGROUP_Tickables);
}
