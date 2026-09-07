#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCProductionCharacterAssetsSubsystem.generated.h"

class AOCCharacter;
class UAnimSequence;
class UOCCharacterVisualProfile;
class USkeletalMesh;
class UStaticMesh;
struct FStreamableHandle;

/**
 * Runtime bridge for the restored QuantumCharacter pack.
 * GAME RECOVERY owns its package preload before first possession so character spawn never performs disk-backed
 * synchronous presentation loads on the game thread.
 */
UCLASS()
class OSTERCONFLICT_API UOCProductionCharacterAssetsSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return bInitialized && bEligible && !bPreloadComplete; }
    virtual bool IsTickableWhenPaused() const override { return true; }

    bool IsCharacterAssetsReady() const { return bInitialized && (!bEligible || bPreloadComplete); }
    float GetCharacterAssetsProgress() const;

private:
    void BeginPreload();
    void BuildProfiles();
    void ApplyToCharacters();
    void ApplyGear(AOCCharacter& Character);
    void ApplyAnimation(AOCCharacter& Character);

    TSharedPtr<FStreamableHandle> PreloadHandle;

    UPROPERTY(Transient) TObjectPtr<UOCCharacterVisualProfile> UAProfile;
    UPROPERTY(Transient) TObjectPtr<UOCCharacterVisualProfile> MaskedProfile;
    UPROPERTY(Transient) TObjectPtr<UOCCharacterVisualProfile> RangersProfile;
    UPROPERTY(Transient) TObjectPtr<UOCCharacterVisualProfile> InsurgentsProfile;

    UPROPERTY(Transient) TObjectPtr<USkeletalMesh> VestMesh;
    UPROPERTY(Transient) TObjectPtr<USkeletalMesh> DropsMesh;
    UPROPERTY(Transient) TObjectPtr<USkeletalMesh> HolsterMesh;
    UPROPERTY(Transient) TObjectPtr<UStaticMesh> CapMesh;

    UPROPERTY(Transient) TObjectPtr<UAnimSequence> IdleAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> WalkAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> RunAnimation;
    UPROPERTY(Transient) TObjectPtr<UAnimSequence> FallAnimation;

    TMap<TWeakObjectPtr<AOCCharacter>, uint8> AnimationStateByCharacter;
    FTimerHandle RefreshTimer;
    bool bInitialized = false;
    bool bEligible = false;
    bool bPreloadRequested = false;
    bool bPreloadComplete = false;
};
