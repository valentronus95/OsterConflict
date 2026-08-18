#include "OCR13WeaponVariantSpawnSubsystem.h"

#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"
#include "OCWorldSectorOster.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

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

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) SpawnBundledVariants(*World);
        }), 1.10f, false);
}

void UOCR13WeaponVariantSpawnSubsystem::SpawnBundledVariants(UWorld& World)
{
    // Frontend-only sessions intentionally do not create the Oster world sector. That keeps menu boot free of gameplay actors.
    bool bGameplayWorldReady = false;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        if (*It)
        {
            bGameplayWorldReady = true;
            break;
        }
    }
    if (!bGameplayWorldReady) return;

    struct FVariantSeed
    {
        TSubclassOf<AOCWeaponBase> WeaponClass;
        FVector Location;
    };

    const FVariantSeed Seeds[] =
    {
        { AOCWeapon_M14::StaticClass(), AOCWorldSectorOster::MuseumAnchor() + FVector(5200.0f, -3200.0f, 80.0f) },
        { AOCWeapon_LeverAction::StaticClass(), AOCWorldSectorOster::ParkAnchor() + FVector(-5000.0f, 4200.0f, 80.0f) },
        { AOCWeapon_MAC10::StaticClass(), AOCWorldSectorOster::StadiumAnchor() + FVector(4200.0f, 4500.0f, 80.0f) },
        { AOCWeapon_Tec9::StaticClass(), AOCWorldSectorOster::CollegeAnchor() + FVector(-3600.0f, -4000.0f, 80.0f) },
    };

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    int32 Spawned = 0;
    for (const FVariantSeed& Seed : Seeds)
    {
        AOCWeaponBase* Weapon = World.SpawnActor<AOCWeaponBase>(
            Seed.WeaponClass, Seed.Location, FRotator::ZeroRotator, SpawnParams);
        if (!Weapon) continue;

        Weapon->DropToWorldServer(Seed.Location, FRotator::ZeroRotator);
        ++Spawned;
    }

    UE_LOG(LogTemp, Display, TEXT("R13 bundled weapon variants spawned as pickups: %d/4"), Spawned);
}
