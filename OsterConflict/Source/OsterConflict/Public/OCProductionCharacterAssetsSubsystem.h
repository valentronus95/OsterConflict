#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCProductionCharacterAssetsSubsystem.generated.h"

class AOCCharacter;
class UOCCharacterVisualProfile;
class USkeletalMesh;
class UStaticMesh;

/**
 * Runtime bridge for the restored QuantumCharacter pack.
 * Keeps gameplay/faction state authoritative while replacing source-only visual proxies.
 */
UCLASS()
class OSTERCONFLICT_API UOCProductionCharacterAssetsSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void BuildProfiles();
    void ApplyToCharacters();
    void ApplyGear(AOCCharacter& Character);

    UPROPERTY(Transient) TObjectPtr<UOCCharacterVisualProfile> UAProfile;
    UPROPERTY(Transient) TObjectPtr<UOCCharacterVisualProfile> MaskedProfile;
    UPROPERTY(Transient) TObjectPtr<UOCCharacterVisualProfile> RangersProfile;
    UPROPERTY(Transient) TObjectPtr<UOCCharacterVisualProfile> InsurgentsProfile;

    UPROPERTY(Transient) TObjectPtr<USkeletalMesh> VestMesh;
    UPROPERTY(Transient) TObjectPtr<USkeletalMesh> DropsMesh;
    UPROPERTY(Transient) TObjectPtr<USkeletalMesh> HolsterMesh;
    UPROPERTY(Transient) TObjectPtr<UStaticMesh> CapMesh;

    FTimerHandle RefreshTimer;
};
