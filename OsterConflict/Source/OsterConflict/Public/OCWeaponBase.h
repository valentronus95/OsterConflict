#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/NetSerialization.h"
#include "OCWeaponTypes.h"
#include "OCTraumaTypes.h"
#include "OCAudioTypes.h"
#include "OCWeaponBase.generated.h"

class AOCCharacter;
class UOCWeaponDefinition;
class UStaticMeshComponent;
class UOCWeaponAudioComponent;
class UOCWeaponAudioProfile;

UCLASS()
class OSTERCONFLICT_API AOCWeaponBase : public AActor
{
    GENERATED_BODY()

public:
    AOCWeaponBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual bool TryFireServer(AOCCharacter* Shooter, const FVector& TraceOrigin, const FVector& TraceDirection,
        bool bAiming, bool bMoving, FHitResult& OutHit, bool& bOutDamagedActor, bool& bOutFatalHit);
    bool BeginReloadServer();
    void CancelReloadServer();
    bool CycleFireModeServer();

    void EquipToCharacterServer(AOCCharacter* NewOwnerCharacter);
    void StoreInInventoryServer(AOCCharacter* NewOwnerCharacter);
    void DropToWorldServer(const FVector& DropLocation, const FRotator& DropRotation);
    int32 AddReserveAmmoServer(int32 Amount);
    bool InstallAttachmentServer(EOCAttachmentSlot Slot, FName AttachmentId);

    /** Called locally by Character after inventory replication. */
    void ApplyInventoryPresentation(bool bActive, USceneComponent* ActiveAttachParent);

    /**
     * Resolve the rendered muzzle from the active production visual while retaining camera-origin aim reconciliation.
     * This deliberately uses the visible production component bounds instead of the camera as a muzzle surrogate.
     * Asset-specific sockets can supersede this bounds fallback later without changing the firing contract.
     */
    FVector ResolvePresentationMuzzleOrigin(const FVector& AimOrigin, const FVector& AimDirection) const
    {
        const FVector Direction = AimDirection.GetSafeNormal();
        if (Direction.IsNearlyZero())
        {
            return AimOrigin;
        }

        TArray<UPrimitiveComponent*> Components;
        GetComponents<UPrimitiveComponent>(Components);
        const FName ProductionTag(TEXT("OC_ProductionWeaponVisual"));
        for (UPrimitiveComponent* Component : Components)
        {
            if (!Component || !Component->ComponentHasTag(ProductionTag) || !Component->IsVisible())
            {
                continue;
            }

            const FBoxSphereBounds Bounds = Component->Bounds;
            const float ForwardExtent =
                FMath::Abs(Direction.X) * Bounds.BoxExtent.X +
                FMath::Abs(Direction.Y) * Bounds.BoxExtent.Y +
                FMath::Abs(Direction.Z) * Bounds.BoxExtent.Z;
            return Bounds.Origin + Direction * FMath::Max(2.0f, ForwardExtent);
        }

        return AimOrigin;
    }

    /**
     * AActor exposes relative-transform setters but no matching getters in UE 5.8.
     * The physical WeaponMesh is the current actor root so player drops can replicate rigid-body motion. The
     * WeaponRoot branch intentionally remains as a compatibility fallback for any legacy or authored variant that
     * still makes the visual root authoritative. This keeps first-person presentation and drop physics in one truth.
     */
    FVector GetActorRelativeLocation() const
    {
        if (const USceneComponent* Root = GetRootComponent())
        {
            if (Root != WeaponRoot)
            {
                return Root->GetRelativeLocation();
            }
        }
        return WeaponRoot ? WeaponRoot->GetRelativeLocation() : GetActorLocation();
    }

    FRotator GetActorRelativeRotation() const
    {
        if (const USceneComponent* Root = GetRootComponent())
        {
            if (Root != WeaponRoot)
            {
                return Root->GetRelativeRotation();
            }
        }
        return WeaponRoot ? WeaponRoot->GetRelativeRotation() : GetActorRotation();
    }

    UFUNCTION(BlueprintPure, Category="Weapon")
    int32 GetAmmoInMagazine() const { return AmmoInMagazine; }

    UFUNCTION(BlueprintPure, Category="Weapon")
    int32 GetReserveAmmo() const { return ReserveAmmo; }

    UFUNCTION(BlueprintPure, Category="Weapon")
    int32 GetMagazineSize() const;

    UFUNCTION(BlueprintPure, Category="Weapon")
    int32 GetMaxReserveAmmo() const { return Tuning.MaxReserveAmmo; }

    UFUNCTION(BlueprintPure, Category="Weapon")
    float GetFireInterval() const;

    UFUNCTION(BlueprintPure, Category="Weapon")
    float GetReloadDuration() const { return FMath::Max(0.05f, Tuning.ReloadDuration); }

    UFUNCTION(BlueprintPure, Category="Weapon")
    EOCFireMode GetCurrentFireMode() const { return CurrentFireMode; }

    UFUNCTION(BlueprintPure, Category="Weapon|Action")
    EOCWeaponActionType GetWeaponActionType() const { return Tuning.ActionType; }

    UFUNCTION(BlueprintPure, Category="Weapon|FireMode")
    bool SupportsFireMode(EOCFireMode Mode) const
    {
        switch (Mode)
        {
            case EOCFireMode::SemiAutomatic: return Tuning.bSupportsSemiAutomatic;
            case EOCFireMode::Burst3: return Tuning.bSupportsBurst3;
            case EOCFireMode::Automatic: return Tuning.bSupportsAutomatic;
            default: return false;
        }
    }

    UFUNCTION(BlueprintPure, Category="Weapon")
    bool IsReloading() const { return bIsReloading; }

    UFUNCTION(BlueprintPure, Category="Weapon")
    float GetHipSpreadDegrees() const { return Tuning.HipSpreadDegrees; }

    UFUNCTION(BlueprintPure, Category="Weapon")
    float GetADSSpreadDegrees() const { return Tuning.ADSSpreadDegrees; }

    UFUNCTION(BlueprintPure, Category="Weapon|Recoil")
    float GetRecoilPitchMin() const;

    UFUNCTION(BlueprintPure, Category="Weapon|Recoil")
    float GetRecoilPitchMax() const;

    UFUNCTION(BlueprintPure, Category="Weapon|Recoil")
    float GetRecoilYawMax() const;

    /** Local owner-only recoil magnitude sourced exclusively from server-confirmed shot multicast events. */
    UFUNCTION(BlueprintPure, Category="Weapon|Recoil")
    float GetConfirmedLocalRecoilPitchOffset() const { return ConfirmedLocalRecoilPitchOffset; }

    UFUNCTION(BlueprintPure, Category="Weapon|Inventory")
    EOCInventorySlot GetPreferredSlot() const { return Tuning.PreferredSlot; }

    UFUNCTION(BlueprintPure, Category="Weapon|Inventory")
    EOCAmmoType GetAmmoType() const { return Tuning.AmmoType; }

    UFUNCTION(BlueprintPure, Category="Weapon|Inventory")
    EOCWeaponClass GetWeaponClass() const { return Tuning.WeaponClass; }

    UFUNCTION(BlueprintPure, Category="Weapon|Inventory")
    FString GetWeaponDisplayName() const { return Tuning.DisplayName; }

    UFUNCTION(BlueprintPure, Category="Weapon|Inventory")
    FName GetWeaponId() const { return Tuning.WeaponId; }

    UFUNCTION(BlueprintPure, Category="Weapon|Inventory")
    bool IsWorldPickup() const { return bIsWorldPickup; }

    UFUNCTION(BlueprintPure, Category="Weapon|Attachments")
    FString GetAttachmentSummary() const;

    UFUNCTION(BlueprintPure, Category="Weapon|Audio")
    bool IsSuppressed() const;

    UFUNCTION(BlueprintPure, Category="Weapon|Audio")
    UOCWeaponAudioComponent* GetWeaponAudioComponent() const { return WeaponAudioComponent; }

protected:
    /** Stable visual attach point. This intentionally remains separate from the physics root. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
    TObjectPtr<USceneComponent> WeaponRoot;

    /** Hidden/source collision body; this is the intended physics authority for dropped weapons. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
    TObjectPtr<UStaticMeshComponent> WeaponMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon|Audio")
    TObjectPtr<UOCWeaponAudioComponent> WeaponAudioComponent;

    /** Optional per-class profile when no WeaponDefinition overrides it. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Audio")
    TObjectPtr<UOCWeaponAudioProfile> DefaultAudioProfile;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Data")
    TObjectPtr<UOCWeaponDefinition> WeaponDefinition;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Data")
    FOCWeaponTuning Tuning;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Weapon|Ammo")
    int32 AmmoInMagazine = 30;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Weapon|Ammo")
    int32 ReserveAmmo = 120;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Weapon|State")
    bool bIsReloading = false;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Weapon|FireMode")
    EOCFireMode CurrentFireMode = EOCFireMode::Automatic;

    UPROPERTY(ReplicatedUsing=OnRep_WorldPickup, VisibleInstanceOnly, BlueprintReadOnly, Category="Weapon|Inventory")
    bool bIsWorldPickup = false;

    UPROPERTY(ReplicatedUsing=OnRep_Attachments, VisibleInstanceOnly, BlueprintReadOnly, Category="Weapon|Attachments")
    TArray<FOCWeaponAttachmentState> Attachments;

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastFireTraceFX(FVector_NetQuantize TraceStart, FVector_NetQuantize TraceEnd, bool bHit);

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastImpactFX(FVector_NetQuantize ImpactLocation, FVector_NetQuantizeNormal ImpactNormal, EOCImpactSurface SurfaceType);

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastShotAudio(FVector_NetQuantize ShotOrigin, FVector_NetQuantize TraceEnd, bool bSuppressed, bool bSupersonic,
        EOCAcousticEnvironment Environment, int32 EventSeed);

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastWeaponStateAudio(EOCWeaponAudioEvent Event, FVector_NetQuantize SourceLocation, int32 EventSeed);

    UFUNCTION(BlueprintImplementableEvent, Category="Weapon|Impact")
    void BP_PlayImpactFX(FVector ImpactLocation, FVector ImpactNormal, EOCImpactSurface SurfaceType);

    UFUNCTION()
    void OnRep_WorldPickup();

    UFUNCTION()
    void OnRep_Attachments();

    void ConfigureBuiltInTuning(const FOCWeaponTuning& NewTuning);

private:
    double LastServerFireTime = -1000.0;
    double LastServerDryFireTime = -1000.0;
    int32 ServerAudioEventCounter = 0;
    FTimerHandle ReloadTimerHandle;

    /** Client-local, confirmed-shot recoil state. It is driven by server-accepted shot multicast, never held input. */
    double LastConfirmedLocalShotTime = -1000.0;
    float ConfirmedLocalRecoilPitchOffset = 0.0f;
    float ConfirmedLocalRecoilYawOffset = 0.0f;
    float ConfirmedRecoilRecoveryDelay = 0.10f;
    float ConfirmedRecoilRecoverySpeed = 8.5f;

    UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> SourceVisualParts;
    void BuildSourceOnlyWeaponVisual();

    void ApplyConfirmedLocalShotRecoil();
    void RecoverConfirmedLocalShotRecoil(float DeltaSeconds);
    float CalculateSpreadDegrees(bool bAiming, bool bMoving) const;
    float GetRecoilMultiplier() const;
    float GetADSSpreadMultiplier() const;
    float GetDamageMultiplier() const;
    bool HasAttachment(FName AttachmentId) const;
    void FinishReloadServer();
    void ApplyDefinitionIfAssigned();
    void ApplyWorldPickupPresentation();
};
