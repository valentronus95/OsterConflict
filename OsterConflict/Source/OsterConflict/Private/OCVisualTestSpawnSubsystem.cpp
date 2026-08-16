#include "OCVisualTestSpawnSubsystem.h"

#include "OCBTR.h"
#include "OCCombatVehicleSpawnPoints.h"
#include "OCPickupGunTruck.h"
#include "OCTeamSpawnPoint.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

bool UOCVisualTestSpawnSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    if (!World) return false;
    return World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE;
}

void UOCVisualTestSpawnSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (!FParse::Param(FCommandLine::Get(), TEXT("R12VisualSlice"))) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    // GameMode creates spawn points and combat vehicle spawn points during BeginPlay. Give that source-owned setup
    // a fraction of a second, then move only the QA routing actors. Normal game launches never enter this path.
    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) RepositionVisualTestContent(*World);
        }), 0.45f, false);
}

void UOCVisualTestSpawnSubsystem::RepositionVisualTestContent(UWorld& World)
{
    // Krushelnytska R12 slice spans roughly Y=-12k..+15k around X=-3.4k.
    // Base spawns are deliberately placed at opposite ends for a short, immediately readable test lane.
    int32 TeamOneBaseIndex = 0;
    int32 TeamTwoBaseIndex = 0;
    for (TActorIterator<AOCTeamSpawnPoint> It(&World); It; ++It)
    {
        AOCTeamSpawnPoint* Point = *It;
        if (!Point || !Point->IsBaseSpawn()) continue;

        const bool bOriginalTeamOneSide = Point->GetActorLocation().X < 0.0f;
        const int32 LocalIndex = bOriginalTeamOneSide ? TeamOneBaseIndex++ : TeamTwoBaseIndex++;
        const FVector Target = bOriginalTeamOneSide
            ? FVector(-3900.0f + LocalIndex * 900.0f, -10500.0f, 90.0f)
            : FVector(-3900.0f + LocalIndex * 900.0f, 13500.0f, 90.0f);
        Point->SetActorLocationAndRotation(Target,
            FRotator(0.0f, bOriginalTeamOneSide ? 90.0f : -90.0f, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
    }

    auto MoveBTR = [](AOCBTR* Vehicle)
    {
        if (!Vehicle) return;
        const bool bTeamOneSide = Vehicle->GetActorLocation().X < 0.0f;
        const FVector Target = bTeamOneSide ? FVector(-3400.0f, -7600.0f, 190.0f) : FVector(-3400.0f, 10600.0f, 190.0f);
        Vehicle->SetActorLocationAndRotation(Target,
            FRotator(0.0f, bTeamOneSide ? 90.0f : -90.0f, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
    };
    for (TActorIterator<AOCBTR> It(&World); It; ++It) MoveBTR(*It);

    auto MoveGunTruck = [](AOCPickupGunTruck* Vehicle)
    {
        if (!Vehicle) return;
        const bool bTeamOneSide = Vehicle->GetActorLocation().X < 0.0f;
        const FVector Target = bTeamOneSide ? FVector(-4300.0f, -6900.0f, 180.0f) : FVector(-2500.0f, 9900.0f, 180.0f);
        Vehicle->SetActorLocationAndRotation(Target,
            FRotator(0.0f, bTeamOneSide ? 90.0f : -90.0f, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
    };
    for (TActorIterator<AOCPickupGunTruck> It(&World); It; ++It) MoveGunTruck(*It);

    // Move the persistent combat spawn points as well, so destroyed QA vehicles respawn back inside the slice.
    for (TActorIterator<AOCBTRSpawnPoint> It(&World); It; ++It)
    {
        AOCBTRSpawnPoint* Spawn = *It;
        if (!Spawn) continue;
        const bool bTeamOneSide = Spawn->GetActorLocation().X < 0.0f;
        Spawn->SetActorLocationAndRotation(
            bTeamOneSide ? FVector(-3400.0f, -7600.0f, 190.0f) : FVector(-3400.0f, 10600.0f, 190.0f),
            FRotator(0.0f, bTeamOneSide ? 90.0f : -90.0f, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
    }
    for (TActorIterator<AOCPickupGunTruckSpawnPoint> It(&World); It; ++It)
    {
        AOCPickupGunTruckSpawnPoint* Spawn = *It;
        if (!Spawn) continue;
        const bool bTeamOneSide = Spawn->GetActorLocation().X < 0.0f;
        Spawn->SetActorLocationAndRotation(
            bTeamOneSide ? FVector(-4300.0f, -6900.0f, 180.0f) : FVector(-2500.0f, 9900.0f, 180.0f),
            FRotator(0.0f, bTeamOneSide ? 90.0f : -90.0f, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
    }

    UE_LOG(LogTemp, Display, TEXT("R12.2 visual-test routing: base spawns and combat vehicles moved onto Krushelnytska slice."));
}
