#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCLocalInboxWeaponOverrideSubsystem.generated.h"

class AActor;
class AOCWeaponBase;

/** Runtime visual override for user-supplied weapon families from models_game_OC. */
UCLASS()
class OSTERCONFLICT_API UOCLocalInboxWeaponOverrideSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

private:
    void HandleActorSpawned(AActor* Actor);
    void ApplyLocalVisual(AOCWeaponBase* Weapon);
    bool ResolveVisualForWeapon(AOCWeaponBase* Weapon, FString& OutObjectPath, float& OutDesiredLengthCm,
        FString& OutCategory) const;

    FDelegateHandle ActorSpawnedHandle;
};
