#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/NetSerialization.h"
#include "OCWeaponTypes.h"
#include "OCOrdnanceTypes.h"
#include "OCCharacter.generated.h"

class AOCAmmoBox;
class AOCArmedVehicleBase;
class AOCInteractableActor;
class AOCWeaponBase;
class AOCVehicleBase;
class UCameraComponent;
class UCameraShakeBase;
class UInputAction;
class UInputMappingContext;
class UOCHealthComponent;
class UOCCombatVisualComponent;
class UOCCharacterAudioComponent;
class UOCCharacterVisualComponent;
class USkeletalMeshComponent;
struct FInputActionValue;

UCLASS()
class OSTERCONFLICT_API AOCCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AOCCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void PawnClientRestart() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintPure, Category="Character")
    UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

    UFUNCTION(BlueprintPure, Category="Character")
    UOCHealthComponent* GetHealthComponent() const { return HealthComponent; }

    UFUNCTION(BlueprintPure, Category="Character|Trauma")
    UOCCombatVisualComponent* GetCombatVisualComponent() const { return CombatVisualComponent; }

    UFUNCTION(BlueprintPure, Category="Character|Audio")
    UOCCharacterAudioComponent* GetCharacterAudioComponent() const { return CharacterAudioComponent; }

    UFUNCTION(BlueprintPure, Category="Character|Visual")
    UOCCharacterVisualComponent* GetCharacterVisualComponent() const { return CharacterVisualComponent; }

    UFUNCTION(BlueprintPure, Category="Character|Visual")
    USkeletalMeshComponent* GetFirstPersonArms() const { return FirstPersonArms; }

    UFUNCTION(BlueprintPure, Category="Weapon")
    AOCWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }

    UFUNCTION(BlueprintPure, Category="Weapon|Inventory")
    AOCWeaponBase* GetPrimaryWeapon() const { return PrimaryWeapon; }

    UFUNCTION(BlueprintPure, Category="Weapon|Inventory")
    AOCWeaponBase* GetSecondaryWeapon() const { return SecondaryWeapon; }

    UFUNCTION(BlueprintPure, Category="Weapon|Inventory")
    EOCInventorySlot GetActiveWeaponSlot() const { return ActiveWeaponSlot; }

    UFUNCTION(BlueprintPure, Category="Weapon|Interaction")
    FString GetInteractionPrompt() const;

    UFUNCTION(BlueprintPure, Category="Combat")
    bool IsAiming() const { return bIsAiming; }

    UFUNCTION(BlueprintPure, Category="Movement")
    bool IsSprinting() const { return bIsSprinting; }

    UFUNCTION(BlueprintPure, Category="Movement")
    bool IsCrouchedOC() const { return bIsCrouched; }

    UFUNCTION(BlueprintPure, Category="Health|Downed")
    bool IsDowned() const;

    UFUNCTION(BlueprintPure, Category="Vehicle")
    bool IsInVehicle() const { return bInVehicle; }

    UFUNCTION(BlueprintPure, Category="Vehicle")
    bool IsVehicleGunner() const { return bVehicleGunner; }

    UFUNCTION(BlueprintPure, Category="Vehicle")
    AOCVehicleBase* GetCurrentVehicle() const { return CurrentVehicle; }

    /** Authority-only transition used by vehicle Pawns during possession changes. */
    void EnterVehicleServer(AOCVehicleBase* Vehicle);
    void ExitVehicleServer(const FVector& ExitLocation, const FRotator& ExitRotation);
    void EnterVehicleGunnerServer(AOCArmedVehicleBase* Vehicle, const FVector& GunnerCameraWorldLocation);
    void ExitVehicleGunnerServer(const FVector& ExitLocation, const FRotator& ExitRotation);

    UFUNCTION(BlueprintPure, Category="Health|Downed")
    float GetDownedTimeRemaining() const;

    UFUNCTION(BlueprintPure, Category="Health|Downed")
    float GetGiveUpProgress() const;

    UFUNCTION(BlueprintPure, Category="Health|Revive")
    float GetReviveProgress() const;

    UFUNCTION(BlueprintPure, Category="Health|Revive")
    FString GetReviveTargetName() const;

    UFUNCTION(BlueprintPure, Category="HUD")
    float GetCrosshairGap() const;

    UFUNCTION(BlueprintPure, Category="HUD")
    float GetHitMarkerAlpha() const;

    UFUNCTION(BlueprintPure, Category="HUD")
    bool WasLastHitFatal() const { return bLastHitFatal; }

    UFUNCTION(BlueprintPure, Category="HUD")
    float GetDamageIndicatorAlpha() const;

    /** S17B local-only preferences; safe to call after settings Apply/Save. */
    void ApplyLocalUserPreferences();
    void RefreshInputMappingsFromUserSettings();

    UFUNCTION(BlueprintPure, Category="HUD")
    float GetLastDamageDirectionDegrees() const { return LastDamageDirectionDegrees; }

    void NotifyDamageReceived(const FVector& DamageOrigin);

    /** Server-only ammo distribution entry point used by ammo boxes. Returns amount actually granted. */
    int32 AddAmmoFromBoxServer(EOCAmmoType AmmoType, int32 Amount);

    /** Server entry used by flash/stun gameplay actors. */
    void ApplyFlashEffectServer(float Intensity, float DurationSeconds);

    /** Authority-only helper used by S13 medic bots. */
    bool StartAIReviveServer(AOCCharacter* Target);
    void CancelAIReviveServer();

    UFUNCTION(BlueprintPure, Category="Grenade") EOCGrenadeType GetSelectedGrenadeType() const { return SelectedGrenadeType; }
    UFUNCTION(BlueprintPure, Category="Grenade") int32 GetSelectedGrenadeCount() const;
    UFUNCTION(BlueprintPure, Category="Engineer") EOCTrapPreset GetSelectedTrapPreset() const { return SelectedTrapPreset; }
    UFUNCTION(BlueprintPure, Category="Engineer") int32 GetTrapCount() const { return TrapCount; }
    UFUNCTION(BlueprintPure, Category="HUD") float GetFlashEffectAlpha() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    TObjectPtr<UCameraComponent> FirstPersonCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health")
    TObjectPtr<UOCHealthComponent> HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|Trauma")
    TObjectPtr<UOCCombatVisualComponent> CombatVisualComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Audio")
    TObjectPtr<UOCCharacterAudioComponent> CharacterAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|Visual")
    TObjectPtr<UOCCharacterVisualComponent> CharacterVisualComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|Visual")
    TObjectPtr<USkeletalMeshComponent> FirstPersonArms;

    UPROPERTY(EditDefaultsOnly, Category="Movement")
    float WalkSpeed = 450.0f;

    UPROPERTY(EditDefaultsOnly, Category="Movement")
    float SprintSpeed = 700.0f;

    UPROPERTY(EditDefaultsOnly, Category="Movement")
    float AimWalkSpeed = 320.0f;

    /** R7: explicit crouch speed; downed crawl overrides this while crouched. */
    UPROPERTY(EditDefaultsOnly, Category="Movement")
    float CrouchSpeed = 250.0f;

    UPROPERTY(EditDefaultsOnly, Category="Combat|ADS")
    float ADSFieldOfView = 70.0f;

    UPROPERTY(EditDefaultsOnly, Category="Combat|ADS")
    float ADSInterpSpeed = 14.0f;

    UPROPERTY(EditDefaultsOnly, Category="Combat|Recoil")
    float RecoilRecoveryDelay = 0.10f;

    UPROPERTY(EditDefaultsOnly, Category="Combat|Recoil")
    float RecoilRecoverySpeed = 8.5f;

    UPROPERTY(EditDefaultsOnly, Category="Combat|Feedback")
    TSubclassOf<UCameraShakeBase> FireCameraShakeClass;

    UPROPERTY(EditDefaultsOnly, Category="Weapon|Loadout")
    TSubclassOf<AOCWeaponBase> StarterPrimaryWeaponClass;

    UPROPERTY(EditDefaultsOnly, Category="Weapon|Loadout")
    TSubclassOf<AOCWeaponBase> StarterSecondaryWeaponClass;

    UPROPERTY(EditDefaultsOnly, Category="Weapon|Interaction", meta=(ClampMin="50.0"))
    float InteractionDistance = 260.0f;

    UPROPERTY(EditDefaultsOnly, Category="World|Interaction", meta=(ClampMin="100.0"))
    float WorldInteractionTraceDistance = 400.0f;

    UPROPERTY(EditDefaultsOnly, Category="Health|Downed", meta=(ClampMin="20.0"))
    float DownedCrawlSpeed = 95.0f;

    UPROPERTY(EditDefaultsOnly, Category="Health|Downed")
    float DownedCameraZ = 38.0f;

    UPROPERTY(EditDefaultsOnly, Category="Health|Downed", meta=(ClampMin="0.5"))
    float GiveUpHoldSeconds = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category="Health|Revive", meta=(ClampMin="50.0"))
    float ReviveDistance = 220.0f;

    UPROPERTY(EditDefaultsOnly, Category="Health|Revive", meta=(ClampMin="0.5"))
    float ReviveHoldSeconds = 3.0f;

    /** S05 prototype hook. S06 will turn this into team/class rules so only medic-capable roles revive. */
    UPROPERTY(EditDefaultsOnly, Category="Health|Revive")
    bool bHasMedicCapability = true;

    UPROPERTY(ReplicatedUsing=OnRep_Inventory, VisibleInstanceOnly, BlueprintReadOnly, Category="Weapon|Inventory")
    TObjectPtr<AOCWeaponBase> PrimaryWeapon;

    UPROPERTY(ReplicatedUsing=OnRep_Inventory, VisibleInstanceOnly, BlueprintReadOnly, Category="Weapon|Inventory")
    TObjectPtr<AOCWeaponBase> SecondaryWeapon;

    UPROPERTY(ReplicatedUsing=OnRep_CurrentWeapon, VisibleInstanceOnly, BlueprintReadOnly, Category="Weapon")
    TObjectPtr<AOCWeaponBase> CurrentWeapon;

    UPROPERTY(ReplicatedUsing=OnRep_Inventory, VisibleInstanceOnly, BlueprintReadOnly, Category="Weapon|Inventory")
    EOCInventorySlot ActiveWeaponSlot = EOCInventorySlot::None;

    UPROPERTY(ReplicatedUsing=OnRep_IsSprinting)
    bool bIsSprinting = false;

    UPROPERTY(ReplicatedUsing=OnRep_IsAiming)
    bool bIsAiming = false;

    UPROPERTY(ReplicatedUsing=OnRep_InVehicle, VisibleInstanceOnly, BlueprintReadOnly, Category="Vehicle")
    bool bInVehicle = false;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Vehicle")
    TObjectPtr<AOCVehicleBase> CurrentVehicle;

    UPROPERTY(ReplicatedUsing=OnRep_InVehicle, VisibleInstanceOnly, BlueprintReadOnly, Category="Vehicle")
    bool bVehicleGunner = false;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Health|Revive")
    TObjectPtr<AOCCharacter> ReviveTarget;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Health|Revive")
    double ReviveEndServerTime = 0.0;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Health|Downed")
    double GiveUpEndServerTime = 0.0;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Grenade") EOCGrenadeType SelectedGrenadeType = EOCGrenadeType::Fragmentation;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Grenade") int32 FragGrenades = 2;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Grenade") int32 SmokeGrenades = 2;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Grenade") int32 FlashGrenades = 2;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Engineer") EOCTrapPreset SelectedTrapPreset = EOCTrapPreset::ContactInfantry;
    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Engineer") int32 TrapCount = 4;

    UFUNCTION(Server, Reliable) void ServerThrowSelectedGrenade();
    UFUNCTION(Server, Reliable) void ServerCycleGrenadeType();
    UFUNCTION(Server, Reliable) void ServerDeploySelectedTrap();
    UFUNCTION(Server, Reliable) void ServerCycleTrapPreset();
    UFUNCTION(Client, Reliable) void ClientApplyFlash(float Intensity, float DurationSeconds);

    UFUNCTION(Server, Reliable)
    void ServerSetSprinting(bool bNewSprinting);

    UFUNCTION(Server, Reliable)
    void ServerSetAiming(bool bNewAiming);

    UFUNCTION(Server, Reliable)
    void ServerSetFireHeld(bool bHeld);

    UFUNCTION(Server, Unreliable)
    void ServerSetVehicleGunnerAim(float RelativeYaw, float RelativePitch);

    UFUNCTION(Server, Reliable)
    void ServerSetVehicleGunnerFireHeld(bool bHeld);

    UFUNCTION(Server, Reliable)
    void ServerReloadVehicleTurret();

    UFUNCTION(Server, Reliable)
    void ServerReload();

    UFUNCTION(Server, Reliable)
    void ServerCycleFireMode();

    UFUNCTION(Server, Reliable)
    void ServerInteract();

    UFUNCTION(Server, Reliable)
    void ServerCancelInteract();

    UFUNCTION(Server, Reliable)
    void ServerSetGiveUpHeld(bool bHeld);

    UFUNCTION(Server, Reliable)
    void ServerDropCurrentWeapon();

    UFUNCTION(Server, Reliable)
    void ServerEquipWeaponSlot(EOCInventorySlot RequestedSlot);

    UFUNCTION(Client, Unreliable)
    void ClientConfirmHit(bool bFatalHit);

    UFUNCTION(Client, Unreliable)
    void ClientDamageFeedback(FVector_NetQuantize DamageOrigin);

    UFUNCTION()
    void OnRep_CurrentWeapon();

    UFUNCTION()
    void OnRep_Inventory();

    UFUNCTION()
    void OnRep_IsSprinting();

    UFUNCTION()
    void OnRep_IsAiming();

    UFUNCTION()
    void OnRep_InVehicle();

    UFUNCTION()
    void HandleDowned();

    UFUNCTION()
    void HandleRevived();

    UFUNCTION()
    void HandleDeath();

private:
    UPROPERTY()
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY()
    TObjectPtr<UInputAction> MoveForwardAction;
    UPROPERTY()
    TObjectPtr<UInputAction> MoveBackwardAction;
    UPROPERTY()
    TObjectPtr<UInputAction> MoveRightAction;
    UPROPERTY()
    TObjectPtr<UInputAction> MoveLeftAction;
    UPROPERTY()
    TObjectPtr<UInputAction> TurnAction;
    UPROPERTY()
    TObjectPtr<UInputAction> LookUpAction;
    UPROPERTY()
    TObjectPtr<UInputAction> JumpAction;
    UPROPERTY()
    TObjectPtr<UInputAction> SprintAction;
    UPROPERTY()
    TObjectPtr<UInputAction> CrouchAction;
    UPROPERTY()
    TObjectPtr<UInputAction> FireAction;
    UPROPERTY()
    TObjectPtr<UInputAction> AimAction;
    UPROPERTY()
    TObjectPtr<UInputAction> ReloadAction;
    UPROPERTY()
    TObjectPtr<UInputAction> FireModeAction;
    UPROPERTY()
    TObjectPtr<UInputAction> InteractAction;
    UPROPERTY()
    TObjectPtr<UInputAction> DropWeaponAction;
    UPROPERTY()
    TObjectPtr<UInputAction> PrimaryWeaponAction;
    UPROPERTY()
    TObjectPtr<UInputAction> SecondaryWeaponAction;
    UPROPERTY() TObjectPtr<UInputAction> ThrowGrenadeAction;
    UPROPERTY() TObjectPtr<UInputAction> CycleGrenadeAction;
    UPROPERTY() TObjectPtr<UInputAction> DeployTrapAction;
    UPROPERTY() TObjectPtr<UInputAction> CycleTrapAction;

    FTimerHandle ServerFireTimerHandle;
    FTimerHandle LocalFireFeedbackTimerHandle;
    FTimerHandle ReviveTimerHandle;
    FTimerHandle GiveUpTimerHandle;

    bool bServerFireHeld = false;
    bool bLocalFireHeld = false;
    bool bLastHitFatal = false;
    float DefaultFieldOfView = 90.0f;
    FVector StandingCameraRelativeLocation = FVector(0.0f, 0.0f, 64.0f);
    float CurrentRecoilPitchOffset = 0.0f;
    float CurrentRecoilYawOffset = 0.0f;
    double LastLocalShotTime = -1000.0;
    double LastHitConfirmTime = -1000.0;
    double LastDamageFeedbackTime = -1000.0;
    float LastDamageDirectionDegrees = 0.0f;
    float LocalVehicleGunnerYaw = 0.0f;
    float LocalVehicleGunnerPitch = 0.0f;
    double FlashEffectEndLocalTime = 0.0;
    float FlashEffectPeakAlpha = 0.0f;
    float FlashEffectDuration = 0.0f;

    void ConfigureEnhancedInput();
    void MoveForward(const FInputActionValue& Value);
    void MoveBackward(const FInputActionValue& Value);
    void MoveRight(const FInputActionValue& Value);
    void MoveLeft(const FInputActionValue& Value);
    void Turn(const FInputActionValue& Value);
    void LookUp(const FInputActionValue& Value);
    void JumpPressed();
    void JumpReleased();
    void StartSprint();
    void StopSprint();
    void ToggleCrouch();
    void StartAim();
    void StopAim();
    void FirePressed();
    void FireReleased();
    void ReloadPressed();
    void CycleFireModePressed();
    void InteractPressed();
    void InteractReleased();
    void DropWeaponPressed();
    void EquipPrimaryPressed();
    void EquipSecondaryPressed();
    void ThrowGrenadePressed();
    void CycleGrenadePressed();
    void DeployTrapPressed();
    void CycleTrapPressed();

    void StartLocalFireFeedback();
    void StopLocalFireFeedback();
    void ApplyLocalShotFeedback();
    void RecoverLocalRecoil(float DeltaSeconds);
    void ServerHandleFirePulse();
    void StopServerFireTimer();
    void ApplyMovementSpeed();

    void SpawnStarterLoadoutServer();
    AOCWeaponBase* SpawnInventoryWeaponServer(TSubclassOf<AOCWeaponBase> WeaponClass);
    void EquipSlotServer(EOCInventorySlot Slot);
    void PickupWeaponServer(AOCWeaponBase* Weapon);
    void DropWeaponServer(AOCWeaponBase* Weapon);
    void RefreshWeaponPresentation();
    AOCWeaponBase* FindClosestWorldWeapon(float& OutDistance) const;
    AOCAmmoBox* FindClosestAmmoBox(float& OutDistance) const;
    AOCInteractableActor* FindFocusedWorldInteractable(float& OutDistance) const;
    AOCVehicleBase* FindFocusedVehicle(float& OutDistance) const;
    AOCCharacter* FindClosestDownedCharacter(float& OutDistance) const;
    bool CanReviveTargetServer(const AOCCharacter* Target) const;
    void StartReviveServer(AOCCharacter* Target);
    void CancelReviveServer();
    void CompleteReviveServer();
    void StartGiveUpServer();
    void CancelGiveUpServer();
    void CompleteGiveUpServer();
    void ApplyLifeStatePresentation();
    void ApplyVehicleOccupancyPresentation();
    void UpdateVehicleGunnerView();
    double GetSynchronizedServerTime() const;
};
