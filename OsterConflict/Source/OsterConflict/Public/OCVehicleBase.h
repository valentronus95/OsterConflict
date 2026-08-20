#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "OCVehicleBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOCVehicleWreckedSignature, AActor*, Vehicle);

class AOCCharacter;
class UBoxComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class USceneComponent;
class USpringArmComponent;
class UStaticMeshComponent;
class UOCVehicleAudioComponent;
struct FInputActionValue;

UENUM(BlueprintType)
enum class EOCVehicleDamageStage : uint8
{
    Intact,
    Damaged,
    Heavy,
    Critical,
    Wrecked
};

/**
 * S10 source-only drivable vehicle foundation.
 *
 * It intentionally uses a server-authoritative rigid-body + raycast suspension model so the prototype remains
 * playable without requiring a skeletal vehicle mesh or wheel bones. Final production vehicles can migrate to
 * Chaos Vehicles / Chaos Modular Vehicles once authored vehicle assets exist.
 */
UCLASS(Abstract)
class OSTERCONFLICT_API AOCVehicleBase : public APawn
{
    GENERATED_BODY()

public:
    AOCVehicleBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void PawnClientRestart() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual bool TryEnterVehicleServer(AOCCharacter* Character);
    void ForceExitDriverServer();

    /** Authority-only inputs used by S13 AI drivers; not exposed as a network RPC. */
    void SetAIDriveInputsServer(float NewThrottle, float NewSteering, bool bNewHandbrake);
    void AIRequestExitServer();

    UPROPERTY(BlueprintAssignable, Category="Vehicle|Damage")
    FOCVehicleWreckedSignature OnVehicleWrecked;

    UFUNCTION(BlueprintPure, Category="Vehicle")
    bool HasDriver() const { return DriverCharacter != nullptr; }

    UFUNCTION(BlueprintPure, Category="Vehicle")
    bool IsVehicleDestroyed() const { return bVehicleDestroyed; }

    UFUNCTION(BlueprintPure, Category="Vehicle")
    AOCCharacter* GetDriverCharacter() const { return DriverCharacter; }

    UFUNCTION(BlueprintPure, Category="Vehicle")
    float GetSpeedKmh() const;

    UFUNCTION(BlueprintPure, Category="Vehicle|Audio") float GetThrottleInput() const { return ThrottleInput; }
    UFUNCTION(BlueprintPure, Category="Vehicle|Audio") bool IsHandbrakeApplied() const { return bHandbrake; }
    UFUNCTION(BlueprintPure, Category="Vehicle|Audio") float GetMaxForwardSpeedKmh() const { return MaxForwardSpeedKmh; }

    UFUNCTION(BlueprintPure, Category="Vehicle")
    float GetVehicleHealthNormalized() const;

    UFUNCTION(BlueprintPure, Category="Vehicle")
    EOCVehicleDamageStage GetDamageStage() const { return DamageStage; }

    UFUNCTION(BlueprintPure, Category="Vehicle")
    bool IsFirstPersonCameraActive() const { return bFirstPersonCamera; }

    UFUNCTION(BlueprintPure, Category="Vehicle")
    FString GetDriverDisplayName() const;

    UFUNCTION(BlueprintPure, Category="Vehicle")
    virtual FString GetSeatPrompt(const AOCCharacter* Character) const;

    /** Engineer gameplay repair. Authority only; returns actual health restored. */
    float RepairVehicleServer(float Amount, AOCCharacter* Engineer);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Body")
    TObjectPtr<UBoxComponent> PhysicsBody;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Body")
    TObjectPtr<UStaticMeshComponent> Chassis;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Body")
    TObjectPtr<USceneComponent> InteriorRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Body")
    TObjectPtr<UStaticMeshComponent> Dashboard;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Body")
    TObjectPtr<UStaticMeshComponent> SteeringWheel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Body")
    TObjectPtr<UStaticMeshComponent> Windshield;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Body")
    TObjectPtr<UStaticMeshComponent> DriverDoor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Body")
    TObjectPtr<UStaticMeshComponent> PassengerDoor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Body")
    TObjectPtr<UStaticMeshComponent> FrontBumper;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Body")
    TObjectPtr<UStaticMeshComponent> RearBumper;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Body")
    TArray<TObjectPtr<UStaticMeshComponent>> WheelVisuals;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Camera")
    TObjectPtr<UCameraComponent> InteriorCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Camera")
    TObjectPtr<USpringArmComponent> ThirdPersonSpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Camera")
    TObjectPtr<UCameraComponent> ThirdPersonCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Audio")
    TObjectPtr<UOCVehicleAudioComponent> VehicleAudioComponent;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Physics")
    float VehicleMassKg = 1450.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Physics")
    float SuspensionTraceLengthCm = 78.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Physics")
    float WheelRadiusCm = 31.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Physics")
    float SpringStiffness = 11500.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Physics")
    float SuspensionDamping = 2100.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Physics")
    float DriveForce = 520000.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Physics")
    float ReverseForceScale = 0.62f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Physics")
    float RollingBrakeForce = 300000.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Physics")
    float HandbrakeForce = 1050000.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Physics")
    float LateralGrip = 7800.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Physics")
    float SteeringTorque = 78000000.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Physics")
    float AeroDrag = 0.18f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Physics")
    float MaxForwardSpeedKmh = 135.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Interaction")
    float EnterDistanceCm = 390.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Interaction")
    float MaxExitSpeedKmh = 14.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Damage")
    float MaxVehicleHealth = 450.0f;

    UPROPERTY(EditDefaultsOnly, Category="Vehicle|Damage")
    float WreckLifetimeSeconds = 24.0f;

    UPROPERTY(ReplicatedUsing=OnRep_Driver, VisibleInstanceOnly, BlueprintReadOnly, Category="Vehicle")
    TObjectPtr<AOCCharacter> DriverCharacter;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Vehicle|Input")
    float ThrottleInput = 0.0f;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Vehicle|Input")
    float SteeringInput = 0.0f;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category="Vehicle|Input")
    bool bHandbrake = false;

    UPROPERTY(ReplicatedUsing=OnRep_DamageState, VisibleInstanceOnly, BlueprintReadOnly, Category="Vehicle|Damage")
    float VehicleHealth = 450.0f;

    UPROPERTY(ReplicatedUsing=OnRep_DamageState, VisibleInstanceOnly, BlueprintReadOnly, Category="Vehicle|Damage")
    EOCVehicleDamageStage DamageStage = EOCVehicleDamageStage::Intact;

    UPROPERTY(ReplicatedUsing=OnRep_DamageState, VisibleInstanceOnly, BlueprintReadOnly, Category="Vehicle|Damage")
    bool bVehicleDestroyed = false;

    UFUNCTION(Server, Unreliable)
    void ServerSetDriveInputs(float NewThrottle, float NewSteering, bool bNewHandbrake);

    UFUNCTION(Server, Reliable)
    void ServerRequestExit();

    UFUNCTION()
    void OnRep_Driver();

    UFUNCTION()
    void OnRep_DamageState();

    UFUNCTION()
    void HandleChassisHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        FVector NormalImpulse, const FHitResult& Hit);

    virtual void ApplyVehicleStyle();
    /** Lets derived multi-axle prototypes add physical suspension contacts matching visible wheels. */
    void AddSuspensionPointLocal(const FVector& LocalPoint);
    void ClearSuspensionPointsLocal();
    FVector FindSafeExitLocationForCharacter(AOCCharacter* Character, float PreferredSideSign, bool bForced) const;
    virtual float GetCollisionDamageScale() const { return 1.0f; }
    virtual void OnVehicleEnteredWreckServer() {}

private:
    UPROPERTY() TObjectPtr<UInputMappingContext> VehicleMappingContext;
    UPROPERTY() TObjectPtr<UInputAction> DriveForwardAction;
    UPROPERTY() TObjectPtr<UInputAction> DriveReverseAction;
    UPROPERTY() TObjectPtr<UInputAction> SteerLeftAction;
    UPROPERTY() TObjectPtr<UInputAction> SteerRightAction;
    UPROPERTY() TObjectPtr<UInputAction> HandbrakeAction;
    UPROPERTY() TObjectPtr<UInputAction> ExitAction;
    UPROPERTY() TObjectPtr<UInputAction> ToggleCameraAction;
    UPROPERTY() TObjectPtr<UInputAction> FreeLookAction;
    UPROPERTY() TObjectPtr<UInputAction> LookXAction;
    UPROPERTY() TObjectPtr<UInputAction> LookYAction;

    TArray<FVector> SuspensionPointsLocal;
    FTimerHandle WreckDestroyTimerHandle;

    float LocalForward = 0.0f;
    float LocalReverse = 0.0f;
    float LocalLeft = 0.0f;
    float LocalRight = 0.0f;
    bool bLocalHandbrake = false;
    bool bFreeLookHeld = false;
    bool bFirstPersonCamera = false;
    float FreeLookYaw = 0.0f;
    float FreeLookPitch = 0.0f;
    float WheelSpinDegrees = 0.0f;

    void ConfigureVehicleInput();
    void DriveForward(const FInputActionValue& Value);
    void DriveForwardReleased(const FInputActionValue& Value);
    void DriveReverse(const FInputActionValue& Value);
    void DriveReverseReleased(const FInputActionValue& Value);
    void SteerLeft(const FInputActionValue& Value);
    void SteerLeftReleased(const FInputActionValue& Value);
    void SteerRight(const FInputActionValue& Value);
    void SteerRightReleased(const FInputActionValue& Value);
    void HandbrakePressed();
    void HandbrakeReleased();
    void ExitPressed();
    void ToggleCameraPressed();
    void FreeLookPressed();
    void FreeLookReleased();
    void LookX(const FInputActionValue& Value);
    void LookY(const FInputActionValue& Value);
    void PushLocalDriveInputs();
    void ApplyLocalCamera(float DeltaSeconds);
    void UpdateWheelVisuals(float DeltaSeconds);

    void SimulateVehicleServer(float DeltaSeconds);
    int32 ApplySuspensionServer(float DeltaSeconds);
    void ApplyDriveAndGripServer(float DeltaSeconds, int32 ContactCount);
    void ApplyVehicleDamageServer(float Amount, AController* EventInstigator, const FVector& DamageLocation);
    void UpdateDamageStageServer();
    void EnterWreckStateServer();
    void ApplyDamagePresentation();
    void ExitDriverServer(bool bForced);
    void DestroyWreck();
};
