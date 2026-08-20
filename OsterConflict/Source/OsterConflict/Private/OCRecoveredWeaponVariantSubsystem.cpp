#include "OCRecoveredWeaponVariantSubsystem.h"

#include "OCGameMode.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"
#include "OCWorldSectorOster.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

bool UOCRecoveredWeaponVariantSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCRecoveredWeaponVariantSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_Client) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    InWorld.GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (WeakWorld.IsValid()) SpawnSandboxRack();
        }));
}

void UOCRecoveredWeaponVariantSubsystem::SpawnSandboxRack()
{
    UWorld* World = GetWorld();
    AOCGameMode* GameMode = World ? World->GetAuthGameMode<AOCGameMode>() : nullptr;
    if (!World || !GameMode || GameMode->IsFrontendOnlySession() || !GameMode->IsSandboxMode()) return;

    for (TActorIterator<AOCWeaponBase> It(World); It; ++It)
    {
        if (It->ActorHasTag(FName(TEXT("OC_R13VariantRack")))) return;
    }

    const TSubclassOf<AOCWeaponBase> Classes[] =
    {
        AOCWeapon_M14::StaticClass(),
        AOCWeapon_Mac10::StaticClass(),
        AOCWeapon_Tec9::StaticClass(),
        AOCWeapon_LeverAction::StaticClass()
    };

    const FVector Anchor = AOCWorldSectorOster::MuseumAnchor() + FVector(650.0f, 420.0f, 45.0f);
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    for (int32 Index = 0; Index < UE_ARRAY_COUNT(Classes); ++Index)
    {
        const FVector Location = Anchor + FVector(0.0f, Index * 95.0f, 0.0f);
        if (AOCWeaponBase* Weapon = World->SpawnActor<AOCWeaponBase>(Classes[Index], Location, FRotator::ZeroRotator, Params))
        {
            Weapon->Tags.Add(FName(TEXT("OC_R13VariantRack")));
            Weapon->DropToWorldServer(Location, FRotator::ZeroRotator);
        }
    }

    UE_LOG(LogTemp, Display, TEXT("Sandbox restored-weapon rack spawned: M14, MAC-10, TEC-9, Lever Action."));
}
