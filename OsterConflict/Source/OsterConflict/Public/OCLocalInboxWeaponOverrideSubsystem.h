#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCLocalInboxWeaponOverrideSubsystem.generated.h"

class AActor;
class AOCWeapon_AssaultRifle;

/** Runtime visual override for explicit user-supplied weapon families such as M16/M4. */
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
    void ApplyM16Visual(AOCWeapon_AssaultRifle* Weapon);

    FString M16ObjectPath;
    FDelegateHandle ActorSpawnedHandle;
};
