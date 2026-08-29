#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OCCharacterVisualTypes.h"
#include "OCCharacterVisualComponent.generated.h"

class AOCCharacter;
class UOCCharacterVisualProfile;
class USkeletalMeshComponent;
class UStaticMeshComponent;

/**
 * Presentation-only character art layer. Gameplay identity remains in PlayerState.
 * Profiles can replace source-only proxies with production skeletal meshes/AnimBPs later.
 */
UCLASS(ClassGroup=(OsterConflict), meta=(BlueprintSpawnableComponent))
class OSTERCONFLICT_API UOCCharacterVisualComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOCCharacterVisualComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void InitializeFirstPersonArms(USkeletalMeshComponent* InFirstPersonArms);
    void RefreshPresentation(bool bForce = false);

    /** Runtime art bridge used by restored production packs without changing gameplay identity. */
    void SetRuntimeProfiles(UOCCharacterVisualProfile* InUA, UOCCharacterVisualProfile* InMasked,
        UOCCharacterVisualProfile* InRangers, UOCCharacterVisualProfile* InInsurgents)
    {
        if (UASpecialUnitProfile == InUA && MaskedFightersProfile == InMasked &&
            USRangersProfile == InRangers && InsurgentsProfile == InInsurgents)
        {
            return;
        }

        UASpecialUnitProfile = InUA;
        MaskedFightersProfile = InMasked;
        USRangersProfile = InRangers;
        InsurgentsProfile = InInsurgents;
        RefreshPresentation(true);
    }

    UFUNCTION(BlueprintPure, Category="Character|Visual") FOCCharacterAppearance GetAppearance() const { return CurrentAppearance; }
    UFUNCTION(BlueprintPure, Category="Character|Visual") EOCFactionArchetype GetFaction() const { return CurrentAppearance.Faction; }

    /** Authority-only one-shot animation event. The multicast is cosmetic and intentionally unreliable. */
    void BroadcastActionServer(EOCCharacterActionEvent Event);

    UFUNCTION(BlueprintImplementableEvent, Category="Character|Visual")
    void BP_OnAppearanceApplied(const FOCCharacterAppearance& Appearance, UOCCharacterVisualProfile* Profile);

    UFUNCTION(BlueprintImplementableEvent, Category="Character|Animation")
    void BP_OnCharacterAction(EOCCharacterActionEvent Event, int32 EventSeed);

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Profiles") TObjectPtr<UOCCharacterVisualProfile> UASpecialUnitProfile;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Profiles") TObjectPtr<UOCCharacterVisualProfile> MaskedFightersProfile;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Profiles") TObjectPtr<UOCCharacterVisualProfile> USRangersProfile;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Profiles") TObjectPtr<UOCCharacterVisualProfile> InsurgentsProfile;

    // Pass45 Gate K: production gameplay fails closed when production character content is unavailable.
    // Source-only Engine BasicShape proxies may be explicitly enabled for isolated developer diagnostics,
    // but they must never be the default runtime presentation path.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Proxy") bool bEnableSourceOnlyProxy = false;

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastCharacterAction(EOCCharacterActionEvent Event, int32 EventSeed);

private:
    TWeakObjectPtr<AOCCharacter> CharacterOwner;
    TWeakObjectPtr<USkeletalMeshComponent> FirstPersonArms;
    FOCCharacterAppearance CurrentAppearance;
    EOCFactionArchetype LastAppliedFaction = EOCFactionArchetype::UASpecialUnit;
    int32 LastAppliedSeed = 0;
    int32 ActionSequence = 0;
    bool bHasAppliedPresentation = false;
    TArray<TObjectPtr<UStaticMeshComponent>> ThirdPersonProxyParts;
    TArray<TObjectPtr<UStaticMeshComponent>> FirstPersonProxyParts;

    UOCCharacterVisualProfile* GetProfile(EOCFactionArchetype Faction) const;
    FOCCharacterAppearance BuildAppearance() const;
    void ApplyProfile(UOCCharacterVisualProfile* Profile);
    void BuildSourceOnlyProxy();
    void UpdateSourceOnlyProxy(bool bShowProxy);
    void ApplyProxyTint(EOCFactionArchetype Faction);
};
