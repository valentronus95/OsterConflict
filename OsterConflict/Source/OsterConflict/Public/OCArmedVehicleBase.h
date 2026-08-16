#pragma once

#include "CoreMinimal.h"
#include "OCVehicleBase.h"
#include "OCTeamTypes.h"
#include "OCArmedVehicleBase.generated.h"

class AOCCharacter;
class UDamageType;
class USceneComponent;
class UStaticMeshComponent;

/** S11 two-player armed vehicle foundation: driver drives, separate gunner aims/fires. */
UCLASS(Abstract)
class OSTERCONFLICT_API AOCArmedVehicleBase : public AOCVehicleBase
{
    GENERATED_BODY()

public:
    AOCArmedVehicleBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual bool TryEnterVehicleServer(AOCCharacter* Character) override;
    virtual FString GetSeatPrompt(const AOCCharacter* Character) const override;

    void SetGunnerAimServer(AOCCharacter* Requester, float RelativeYaw, float RelativePitch);
    void SetGunnerFireHeldServer(AOCCharacter* Requester, bool bHeld);
    void RequestGunnerReloadServer(AOCCharacter* Requester);
    bool ExitGunnerServer(AOCCharacter* Requester, bool bForced = false);
    void ForceExitGunnerServer();

    UFUNCTION(BlueprintPure, Category="Vehicle|Seats")
    AOCCharacter* GetGunnerCharacter() const { return GunnerCharacter; }

    UFUNCTION(BlueprintPure, Category="Vehicle|Seats")
    bool HasGunner() const { return GunnerCharacter != nullptr; }

    UFUNCTION(BlueprintPure, Category="Vehicle|Turret")
    float GetTurretYaw() const { return TurretYaw; }

    UFUNCTION(BlueprintPure, Category="Vehicle|Turret")
    float GetTurretPitch() const { return TurretPitch; }

    UFUNCTION(BlueprintPure, Category="Vehicle|Turret")
    float GetMaxTurretYawLimit() const { return MaxTurretYaw; }

    UFUNCTION(BlueprintPure, Category="Vehicle|Turret")
    float GetMinTurretPitchLimit() const { return MinTurretPitch; }

    UFUNCTION(BlueprintPure, Category="Vehicle|Turret")
    float GetMaxTurretPitchLimit() const { return MaxTurretPitch; }

    UFUNCTION(BlueprintPure, Category="Vehicle|Turret")
    int32 GetTurretAmmoInMagazine() const { return TurretAmmoInMagazine; }

    UFUNCTION(BlueprintPure, Category="Vehicle|Turret")
    int32 GetTurretReserveAmmo() const { return TurretReserveAmmo; }

    UFUNCTION(BlueprintPure, Category="Vehicle|Turret")
    bool IsTurretReloading() const { return bTurretReloading; }

    UFUNCTION(BlueprintPure, Category="Vehicle|Turret")
    FString GetTurretDisplayName() const { return TurretDisplayName; }

    UFUNCTION(BlueprintPure, Category="Vehicle|Seats")
    EOCTeam GetOccupantTeam() const { return OccupantTeam; }

    UFUNCTION(BlueprintPure, Category="Vehicle|Turret")
    FVector GetGunnerCameraWorldLocation() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Turret")
    TObjectPtr<USceneComponent> TurretPivot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Turret")
    TObjectPtr<UStaticMeshComponent> TurretBaseMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Turret")
    TObjectPtr<USceneComponent> BarrelPivot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Turret")
    TObjectPtr<UStaticMeshComponent> BarrelMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Turret")
    TObjectPtr<USceneComponent> MuzzlePoint;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Turret")
    FString TurretDisplayName = TEXT("VEHICLE MG");

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Turret")
    float TurretDamage = 28.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Turret")
    float TurretRoundsPerMinute = 650.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Turret")
    float TurretRangeCm = 12000.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Turret")
    float TurretSpreadDegrees = 0.65f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Turret")
    float MaxTurretYaw = 170.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Turret")
    float MinTurretPitch = -18.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Turret")
    float MaxTurretPitch = 46.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Turret")
    int32 TurretMagazineSize = 100;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Turret")
    int32 StartingTurretReserveAmmo = 400;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Turret")
    float TurretReloadSeconds = 4.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Turret")
    float GunnerEnterDistanceCm = 430.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Turret")
    TSubclassOf<UDamageType> TurretDamageTypeClass;

    UPROPERTY(ReplicatedUsing=OnRep_Gunner, VisibleInstanceOnly, BlueprintReadOnly, Category="Vehicle|Seats")
    TObjectPtr<AOCCharacter> GunnerCharacter;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Vehicle|Seats")
    EOCTeam OccupantTeam = EOCTeam::None;

    UPROPERTY(ReplicatedUsing=OnRep_TurretAim, VisibleInstanceOnly, BlueprintReadOnly, Category="Vehicle|Turret")
    float TurretYaw = 0.0f;

    UPROPERTY(ReplicatedUsing=OnRep_TurretAim, VisibleInstanceOnly, BlueprintReadOnly, Category="Vehicle|Turret")
    float TurretPitch = 0.0f;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Vehicle|Turret")
    int32 TurretAmmoInMagazine = 0;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Vehicle|Turret")
    int32 TurretReserveAmmo = 0;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Vehicle|Turret")
    bool bTurretReloading = false;

    virtual bool CanHullAcceptDamage(const FDamageEvent& DamageEvent) const;
    virtual float ModifyHullDamage(float DamageAmount, const FDamageEvent& DamageEvent) const;
    virtual void ApplyArmedVehicleStyle();
    virtual void OnVehicleEnteredWreckServer() override;

private:
    FTimerHandle TurretFireTimerHandle;
    FTimerHandle TurretReloadTimerHandle;
    bool bGunnerFireHeld = false;

    UFUNCTION()
    void OnRep_Gunner();

    UFUNCTION()
    void OnRep_TurretAim();

    void ApplyTurretPresentation();
    void BeginTurretFireServer();
    void StopTurretFireServer();
    void FireTurretShotServer();
    void FinishTurretReloadServer();
    bool CanGunnerOperateServer(const AOCCharacter* Requester) const;
    EOCTeam ResolveCharacterTeam(const AOCCharacter* Character) const;
};
