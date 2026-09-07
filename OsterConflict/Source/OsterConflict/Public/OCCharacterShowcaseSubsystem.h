#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCCharacterShowcaseSubsystem.generated.h"

class ASkeletalMeshActor;
class AOCWeaponBase;

/**
 * Lightweight local model-inspection stand.
 * Spawns five non-AI skeletal mannequins at the selected team's base and loads only the five
 * explicitly required CHARACTER_SKIN entries instead of preloading the whole local/Fab manifest.
 */
UCLASS()
class OSTERCONFLICT_API UOCCharacterShowcaseSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    void TrySpawnShowcase();
    void ClearShowcase();
    void BlockShowcase(const FString& Reason, int32 CompatibleCount = INDEX_NONE);

    bool bShowcaseReady = false;
    bool bShowcaseBlocked = false;
    FTimerHandle RetryTimer;
    TArray<TWeakObjectPtr<ASkeletalMeshActor>> Mannequins;
    TArray<TWeakObjectPtr<AOCWeaponBase>> DisplayWeapons;
};
