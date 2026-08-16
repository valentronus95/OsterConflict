#include "OCVisualTestSpawnSubsystem.h"

#include "OCBTR.h"
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

    // GameMode creates spawn points and combat vehicles during BeginPlay. Give that setup a fraction of a second,
    // then redirect only this explicit QA launch. Normal game launches never enter this path.
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
    // The production bases are over a kilometre from the current art slice. Replace only the base spawn actors
    // for -R12VisualSlice so clicking BASE puts both teams at opposite ends of the short Krushelnytska QA lane.
    TArray<AOCTeamSpawnPoint*> OldBaseSpawns;
    for (TActorIterator<AOCTeamSpawnPoint> It(&World); It; ++It)
    {
        if (AOCTeamSpawnPoint* Point = *It; Point && Point->IsBaseSpawn()) OldBaseSpawns.Add(Point);
    }
    for (AOCTeamSpawnPoint* Point : OldBaseSpawns)
    {
        if (IsValid(Point)) Point->Destroy();
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    const FVector TeamOneBases[] =
    {
        FVector(-3900.0f, -10500.0f, 90.0f),
        FVector(-3000.0f, -10500.0f, 90.0f)
    };
    const FVector TeamTwoBases[] =
    {
        FVector(-3900.0f, 13500.0f, 90.0f),
        FVector(-3000.0f, 13500.0f, 90.0f)
    };

    for (const FVector& Location : TeamOneBases)
    {
        if (AOCTeamSpawnPoint* Point = World.SpawnActor<AOCTeamSpawnPoint>(AOCTeamSpawnPoint::StaticClass(),
            Location, FRotator(0.0f, 90.0f, 0.0f), SpawnParams))
        {
            Point->ConfigureServer(EOCTeam::TeamOne, true, NAME_None);
        }
    }
    for (const FVector& Location : TeamTwoBases)
    {
        if (AOCTeamSpawnPoint* Point = World.SpawnActor<AOCTeamSpawnPoint>(AOCTeamSpawnPoint::StaticClass(),
            Location, FRotator(0.0f, -90.0f, 0.0f), SpawnParams))
        {
            Point->ConfigureServer(EOCTeam::TeamTwo, true, NAME_None);
        }
    }

    // The current BTR and gun-truck prototypes are also moved into the slice so vehicle QA starts in seconds,
    // not after a several-hundred-metre run. Their production spawn-point geography is left untouched.
    for (TActorIterator<AOCBTR> It(&World); It; ++It)
    {
        AOCBTR* Vehicle = *It;
        if (!Vehicle) continue;
        const bool bTeamOneSide = Vehicle->GetActorLocation().X < 0.0f;
        const FVector Target = bTeamOneSide ? FVector(-3400.0f, -7600.0f, 190.0f) : FVector(-3400.0f, 10600.0f, 190.0f);
        Vehicle->SetActorLocationAndRotation(Target,
            FRotator(0.0f, bTeamOneSide ? 90.0f : -90.0f, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
    }

    for (TActorIterator<AOCPickupGunTruck> It(&World); It; ++It)
    {
        AOCPickupGunTruck* Vehicle = *It;
        if (!Vehicle) continue;
        const bool bTeamOneSide = Vehicle->GetActorLocation().X < 0.0f;
        const FVector Target = bTeamOneSide ? FVector(-4300.0f, -6900.0f, 180.0f) : FVector(-2500.0f, 9900.0f, 180.0f);
        Vehicle->SetActorLocationAndRotation(Target,
            FRotator(0.0f, bTeamOneSide ? 90.0f : -90.0f, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
    }

    UE_LOG(LogTemp, Display, TEXT("R12.2 visual-test routing: QA base spawns and combat vehicles placed on Krushelnytska slice."));
}
