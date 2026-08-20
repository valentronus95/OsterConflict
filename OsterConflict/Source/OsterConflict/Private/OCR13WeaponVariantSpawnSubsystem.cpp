#include "OCR13WeaponVariantSpawnSubsystem.h"

#include "OCGameMode.h"
#include "OCTeamSpawnPoint.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"
#include "OCWorldSectorOster.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

namespace
{
    constexpr int32 MaxSpawnAttempts = 20;
    constexpr float SpawnRetryDelaySeconds = 0.50f;
    constexpr int32 WeaponTestCount = 10;
}

bool UOCR13WeaponVariantSpawnSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13WeaponVariantSpawnSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (InWorld.GetNetMode() == NM_Client) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    ScheduleSpawnAttempt(InWorld, 0.25f);
}

void UOCR13WeaponVariantSpawnSubsystem::ScheduleSpawnAttempt(UWorld& World, const float DelaySeconds)
{
    if (bSpawnComplete || SpawnAttemptCount >= MaxSpawnAttempts) return;

    TWeakObjectPtr<UWorld> WeakWorld(&World);
    FTimerHandle Timer;
    World.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* RetryWorld = WeakWorld.Get()) TrySpawnBundledVariants(*RetryWorld);
        }), FMath::Max(0.05f, DelaySeconds), false);
}

void UOCR13WeaponVariantSpawnSubsystem::TrySpawnBundledVariants(UWorld& World)
{
    if (bSpawnComplete) return;
    ++SpawnAttemptCount;

    bool bGameplayWorldReady = false;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        if (*It)
        {
            bGameplayWorldReady = true;
            break;
        }
    }

    if (!bGameplayWorldReady)
    {
        if (SpawnAttemptCount < MaxSpawnAttempts)
        {
            ScheduleSpawnAttempt(World, SpawnRetryDelaySeconds);
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("R13 weapon test rack: Oster world sector was not ready after %d attempts."),
                SpawnAttemptCount);
        }
        return;
    }

    // Put the complete test rack beside the actual Team One base spawn. The compact layout can move that spawn,
    // so derive the location from the live actor instead of scattering test weapons across unrelated landmarks.
    FVector BaseSpawn(-64000.0f, 44000.0f, 160.0f);
    for (TActorIterator<AOCTeamSpawnPoint> It(&World); It; ++It)
    {
        AOCTeamSpawnPoint* Spawn = *It;
        if (Spawn && Spawn->IsBaseSpawn() && Spawn->GetTeamId() == EOCTeam::TeamOne)
        {
            BaseSpawn = Spawn->GetActorLocation();
            break;
        }
    }

    const TSubclassOf<AOCWeaponBase> Classes[WeaponTestCount] =
    {
        AOCWeapon_AssaultRifle::StaticClass(),
        AOCWeapon_SMG::StaticClass(),
        AOCWeapon_Pistol::StaticClass(),
        AOCWeapon_Sniper::StaticClass(),
        AOCWeapon_Shotgun::StaticClass(),
        AOCWeapon_LMG::StaticClass(),
        AOCWeapon_M14::StaticClass(),
        AOCWeapon_LeverAction::StaticClass(),
        AOCWeapon_MAC10::StaticClass(),
        AOCWeapon_Tec9::StaticClass(),
    };

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    TArray<AOCWeaponBase*> SpawnedWeapons;
    SpawnedWeapons.Reserve(WeaponTestCount);

    bool bSpawnedAll = true;
    for (int32 Index = 0; Index < WeaponTestCount; ++Index)
    {
        // Two clean rows of five, starting about 4 m to the side/front of the spawn point.
        const int32 Row = Index / 5;
        const int32 Column = Index % 5;
        const FVector Location = BaseSpawn + FVector(
            420.0f + static_cast<float>(Row) * 230.0f,
            -480.0f + static_cast<float>(Column) * 240.0f,
            120.0f);

        AOCWeaponBase* Weapon = World.SpawnActor<AOCWeaponBase>(
            Classes[Index], Location, FRotator::ZeroRotator, SpawnParams);
        if (!Weapon)
        {
            bSpawnedAll = false;
            break;
        }

        Weapon->DropToWorldServer(Location, FRotator::ZeroRotator);
        SpawnedWeapons.Add(Weapon);
    }

    if (!bSpawnedAll || SpawnedWeapons.Num() != WeaponTestCount)
    {
        for (AOCWeaponBase* Weapon : SpawnedWeapons)
        {
            if (IsValid(Weapon)) Weapon->Destroy();
        }

        if (SpawnAttemptCount < MaxSpawnAttempts)
        {
            ScheduleSpawnAttempt(World, SpawnRetryDelaySeconds);
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("R13 weapon test rack: could not create all %d pickups after %d attempts."),
                WeaponTestCount, SpawnAttemptCount);
        }
        return;
    }

    bSpawnComplete = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13 weapon test rack spawned beside Team One base: %d/%d weapons after %d attempt(s)."),
        SpawnedWeapons.Num(), WeaponTestCount, SpawnAttemptCount);
}
