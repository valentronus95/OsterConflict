#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCLocalInboxWeaponOverrideSubsystem.generated.h"

class AActor;
class AOCWeaponBase;
struct FStreamableHandle;

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
    void ApplyResidentLocalVisual(AOCWeaponBase* Weapon, const FString& ObjectPath,
        float DesiredLengthCm, const FString& Category);
    void CompleteLocalVisualPreload(TWeakObjectPtr<AOCWeaponBase> WeakWeapon, FString ObjectPath,
        float DesiredLengthCm, FString Category);
    bool ResolveVisualForWeapon(AOCWeaponBase* Weapon, FString& OutObjectPath, float& OutDesiredLengthCm,
        FString& OutCategory) const;

    FDelegateHandle ActorSpawnedHandle;
    TArray<TSharedPtr<FStreamableHandle>> ResidentPreloadHandles;
};
