#include "OCCharacter.h"

#include "OCGameMode.h"
#include "OCGameState.h"
#include "OCHealthComponent.h"
#include "OCPlayerUserSettings.h"
#include "OCCombatVisualComponent.h"
#include "OCCharacterAudioComponent.h"
#include "OCCharacterVisualComponent.h"
#include "OCCharacterVisualTypes.h"
#include "OCInteractableActor.h"
#include "OCPlayerState.h"
#include "OCAmmoBox.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"
#include "OCVehicleBase.h"
#include "OCArmedVehicleBase.h"
#include "OCGrenadeProjectile.h"
#include "OCDeployableTrap.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AOCCharacter::AOCCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    GetCapsuleComponent()->InitCapsuleSize(42.0f, 92.0f);

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
    FirstPersonCamera->bUsePawnControlRotation = true;

    HealthComponent = CreateDefaultSubobject<UOCHealthComponent>(TEXT("HealthComponent"));
    CombatVisualComponent = CreateDefaultSubobject<UOCCombatVisualComponent>(TEXT("CombatVisualComponent"));
    CharacterAudioComponent = CreateDefaultSubobject<UOCCharacterAudioComponent>(TEXT("CharacterAudioComponent"));
    CharacterVisualComponent = CreateDefaultSubobject<UOCCharacterVisualComponent>(TEXT("CharacterVisualComponent"));

    FirstPersonArms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonArms"));
    FirstPersonArms->SetupAttachment(FirstPersonCamera);
    FirstPersonArms->SetOnlyOwnerSee(true);
    FirstPersonArms->SetCastShadow(false);
    FirstPersonArms->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    StarterPrimaryWeaponClass = AOCWeapon_AssaultRifle::StaticClass();
    StarterSecondaryWeaponClass = AOCWeapon_Pistol::StaticClass();

    DefaultMappingContext = CreateDefaultSubobject<UInputMappingContext>(TEXT("IMC_RuntimeDefault"));

    MoveForwardAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_MoveForward"));
    MoveForwardAction->ValueType = EInputActionValueType::Axis1D;
    MoveBackwardAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_MoveBackward"));
    MoveBackwardAction->ValueType = EInputActionValueType::Axis1D;
    MoveRightAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_MoveRight"));
    MoveRightAction->ValueType = EInputActionValueType::Axis1D;
    MoveLeftAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_MoveLeft"));
    MoveLeftAction->ValueType = EInputActionValueType::Axis1D;
    TurnAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Turn"));
    TurnAction->ValueType = EInputActionValueType::Axis1D;
    LookUpAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_LookUp"));
    LookUpAction->ValueType = EInputActionValueType::Axis1D;

    JumpAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Jump"));
    SprintAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Sprint"));
    CrouchAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Crouch"));
    FireAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Fire"));
    AimAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Aim"));
    ReloadAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Reload"));
    FireModeAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_FireMode"));
    InteractAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Interact"));
    DropWeaponAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_DropWeapon"));
    PrimaryWeaponAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_PrimaryWeapon"));
    SecondaryWeaponAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_SecondaryWeapon"));
    ThrowGrenadeAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_ThrowGrenade"));
    CycleGrenadeAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_CycleGrenade"));
    DeployTrapAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_DeployTrap"));
    CycleTrapAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_CycleTrap"));

    if (DefaultMappingContext)
    {
        DefaultMappingContext->MapKey(MoveForwardAction, EKeys::W);
        DefaultMappingContext->MapKey(MoveBackwardAction, EKeys::S);
        DefaultMappingContext->MapKey(MoveRightAction, EKeys::D);
        DefaultMappingContext->MapKey(MoveLeftAction, EKeys::A);
        DefaultMappingContext->MapKey(TurnAction, EKeys::MouseX);
        DefaultMappingContext->MapKey(LookUpAction, EKeys::MouseY);
        DefaultMappingContext->MapKey(JumpAction, EKeys::SpaceBar);
        DefaultMappingContext->MapKey(SprintAction, EKeys::LeftShift);
        DefaultMappingContext->MapKey(CrouchAction, EKeys::LeftControl);
        DefaultMappingContext->MapKey(FireAction, EKeys::LeftMouseButton);
        DefaultMappingContext->MapKey(AimAction, EKeys::RightMouseButton);
        DefaultMappingContext->MapKey(ReloadAction, EKeys::R);
        DefaultMappingContext->MapKey(FireModeAction, EKeys::B);
        DefaultMappingContext->MapKey(InteractAction, EKeys::E);
        DefaultMappingContext->MapKey(DropWeaponAction, EKeys::G);
        DefaultMappingContext->MapKey(PrimaryWeaponAction, EKeys::One);
        DefaultMappingContext->MapKey(SecondaryWeaponAction, EKeys::Two);
        DefaultMappingContext->MapKey(ThrowGrenadeAction, EKeys::F);
        DefaultMappingContext->MapKey(CycleGrenadeAction, EKeys::Four);
        DefaultMappingContext->MapKey(DeployTrapAction, EKeys::V);
        DefaultMappingContext->MapKey(CycleTrapAction, EKeys::N);
    }
}

void AOCCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (FirstPersonCamera)
    {
        DefaultFieldOfView = FMath::Clamp(UOCPlayerUserSettings::Get()->FieldOfView, 75.0f, 120.0f);
        FirstPersonCamera->SetFieldOfView(DefaultFieldOfView);
        StandingCameraRelativeLocation = FirstPersonCamera->GetRelativeLocation();
    }
    UOCPlayerUserSettings::Get()->ApplyPresentationCVars();

    if (HealthComponent)
    {
        HealthComponent->OnDowned.AddDynamic(this, &AOCCharacter::HandleDowned);
        HealthComponent->OnRevived.AddDynamic(this, &AOCCharacter::HandleRevived);
        HealthComponent->OnDeath.AddDynamic(this, &AOCCharacter::HandleDeath);
    }

    if (CharacterVisualComponent)
    {
        CharacterVisualComponent->InitializeFirstPersonArms(FirstPersonArms);
        CharacterVisualComponent->RefreshPresentation(true);
    }

    if (HasAuthority())
    {
        SpawnStarterLoadoutServer();
    }

    ApplyLifeStatePresentation();
}

void AOCCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (HasAuthority() && ReviveTarget && !CanReviveTargetServer(ReviveTarget))
    {
        CancelReviveServer();
    }

    if (bVehicleGunner && IsLocallyControlled())
    {
        UpdateVehicleGunnerView();
    }

    if (FirstPersonCamera && IsLocallyControlled())
    {
        const float TargetFOV = bIsAiming ? ADSFieldOfView : DefaultFieldOfView;
        FirstPersonCamera->SetFieldOfView(FMath::FInterpTo(
            FirstPersonCamera->FieldOfView, TargetFOV, DeltaSeconds, ADSInterpSpeed));
    }
}

void AOCCharacter::PawnClientRestart()
{
    Super::PawnClientRestart();
    ConfigureEnhancedInput();
    if (CharacterVisualComponent) CharacterVisualComponent->RefreshPresentation(true);
}

void AOCCharacter::ConfigureEnhancedInput()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC || !PC->IsLocalController() || !DefaultMappingContext)
    {
        return;
    }

    RefreshInputMappingsFromUserSettings();
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
    {
        Subsystem->RemoveMappingContext(DefaultMappingContext);
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
    }
}

void AOCCharacter::RefreshInputMappingsFromUserSettings()
{
    if (!DefaultMappingContext) return;
    const UOCPlayerUserSettings* Settings = UOCPlayerUserSettings::Get();
    auto Remap = [this, Settings](UInputAction* Action, FName Id)
    {
        if (!Action) return;
        const FKey Key = Settings->GetKey(Id);
        if (!Key.IsValid()) return;
        DefaultMappingContext->UnmapAllKeysFromAction(Action);
        DefaultMappingContext->MapKey(Action, Key);
    };
    Remap(MoveForwardAction, TEXT("MoveForward"));
    Remap(MoveBackwardAction, TEXT("MoveBackward"));
    Remap(MoveLeftAction, TEXT("MoveLeft"));
    Remap(MoveRightAction, TEXT("MoveRight"));
    Remap(JumpAction, TEXT("Jump"));
    Remap(SprintAction, TEXT("Sprint"));
    Remap(CrouchAction, TEXT("Crouch"));
    Remap(FireAction, TEXT("Fire"));
    Remap(AimAction, TEXT("Aim"));
    Remap(ReloadAction, TEXT("Reload"));
    Remap(InteractAction, TEXT("Interact"));
    Remap(ThrowGrenadeAction, TEXT("ThrowGrenade"));
}

void AOCCharacter::ApplyLocalUserPreferences()
{
    if (!IsLocallyControlled()) return;
    const UOCPlayerUserSettings* Settings = UOCPlayerUserSettings::Get();
    DefaultFieldOfView = FMath::Clamp(Settings->FieldOfView, 75.0f, 120.0f);
    if (FirstPersonCamera) FirstPersonCamera->SetFieldOfView(bIsAiming ? ADSFieldOfView : DefaultFieldOfView);
    RefreshInputMappingsFromUserSettings();
    ConfigureEnhancedInput();
}

void AOCCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EnhancedInput)
    {
        UE_LOG(LogTemp, Error, TEXT("OsterConflict requires EnhancedInputComponent. Check DefaultInput.ini."));
        return;
    }

    EnhancedInput->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AOCCharacter::MoveForward);
    EnhancedInput->BindAction(MoveBackwardAction, ETriggerEvent::Triggered, this, &AOCCharacter::MoveBackward);
    EnhancedInput->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AOCCharacter::MoveRight);
    EnhancedInput->BindAction(MoveLeftAction, ETriggerEvent::Triggered, this, &AOCCharacter::MoveLeft);
    EnhancedInput->BindAction(TurnAction, ETriggerEvent::Triggered, this, &AOCCharacter::Turn);
    EnhancedInput->BindAction(LookUpAction, ETriggerEvent::Triggered, this, &AOCCharacter::LookUp);
    EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &AOCCharacter::JumpPressed);
    EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &AOCCharacter::JumpReleased);
    EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &AOCCharacter::StartSprint);
    EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &AOCCharacter::StopSprint);
    EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &AOCCharacter::ToggleCrouch);
    EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &AOCCharacter::FirePressed);
    EnhancedInput->BindAction(FireAction, ETriggerEvent::Completed, this, &AOCCharacter::FireReleased);
    EnhancedInput->BindAction(AimAction, ETriggerEvent::Started, this, &AOCCharacter::StartAim);
    EnhancedInput->BindAction(AimAction, ETriggerEvent::Completed, this, &AOCCharacter::StopAim);
    EnhancedInput->BindAction(ReloadAction, ETriggerEvent::Started, this, &AOCCharacter::ReloadPressed);
    EnhancedInput->BindAction(FireModeAction, ETriggerEvent::Started, this, &AOCCharacter::CycleFireModePressed);
    EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &AOCCharacter::InteractPressed);
    EnhancedInput->BindAction(InteractAction, ETriggerEvent::Completed, this, &AOCCharacter::InteractReleased);
    EnhancedInput->BindAction(InteractAction, ETriggerEvent::Canceled, this, &AOCCharacter::InteractReleased);
    EnhancedInput->BindAction(DropWeaponAction, ETriggerEvent::Started, this, &AOCCharacter::DropWeaponPressed);
    EnhancedInput->BindAction(PrimaryWeaponAction, ETriggerEvent::Started, this, &AOCCharacter::EquipPrimaryPressed);
    EnhancedInput->BindAction(SecondaryWeaponAction, ETriggerEvent::Started, this, &AOCCharacter::EquipSecondaryPressed);
    EnhancedInput->BindAction(ThrowGrenadeAction, ETriggerEvent::Started, this, &AOCCharacter::ThrowGrenadePressed);
    EnhancedInput->BindAction(CycleGrenadeAction, ETriggerEvent::Started, this, &AOCCharacter::CycleGrenadePressed);
    EnhancedInput->BindAction(DeployTrapAction, ETriggerEvent::Started, this, &AOCCharacter::DeployTrapPressed);
    EnhancedInput->BindAction(CycleTrapAction, ETriggerEvent::Started, this, &AOCCharacter::CycleTrapPressed);
}

void AOCCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOCCharacter, PrimaryWeapon);
    DOREPLIFETIME(AOCCharacter, SecondaryWeapon);
    DOREPLIFETIME(AOCCharacter, CurrentWeapon);
    DOREPLIFETIME(AOCCharacter, ActiveWeaponSlot);
    DOREPLIFETIME(AOCCharacter, bIsSprinting);
    DOREPLIFETIME(AOCCharacter, bIsAiming);
    DOREPLIFETIME(AOCCharacter, bInVehicle);
    DOREPLIFETIME(AOCCharacter, CurrentVehicle);
    DOREPLIFETIME(AOCCharacter, bVehicleGunner);
    DOREPLIFETIME(AOCCharacter, ReviveTarget);
    DOREPLIFETIME(AOCCharacter, ReviveEndServerTime);
    DOREPLIFETIME(AOCCharacter, GiveUpEndServerTime);
    DOREPLIFETIME(AOCCharacter, SelectedGrenadeType);
    DOREPLIFETIME(AOCCharacter, FragGrenades);
    DOREPLIFETIME(AOCCharacter, SmokeGrenades);
    DOREPLIFETIME(AOCCharacter, FlashGrenades);
    DOREPLIFETIME(AOCCharacter, SelectedTrapPreset);
    DOREPLIFETIME(AOCCharacter, TrapCount);
}

void AOCCharacter::MoveForward(const FInputActionValue& Value)
{
    if (Controller)
    {
        AddMovementInput(GetActorForwardVector(), Value.Get<float>());
    }
}

void AOCCharacter::MoveBackward(const FInputActionValue& Value)
{
    if (Controller)
    {
        AddMovementInput(GetActorForwardVector(), -Value.Get<float>());
    }
}

void AOCCharacter::MoveRight(const FInputActionValue& Value)
{
    if (Controller)
    {
        AddMovementInput(GetActorRightVector(), Value.Get<float>());
    }
}

void AOCCharacter::MoveLeft(const FInputActionValue& Value)
{
    if (Controller)
    {
        AddMovementInput(GetActorRightVector(), -Value.Get<float>());
    }
}

void AOCCharacter::Turn(const FInputActionValue& Value)
{
    if (bVehicleGunner && CurrentVehicle)
    {
        float YawLimit = 180.0f;
        if (const AOCArmedVehicleBase* Armed = Cast<AOCArmedVehicleBase>(CurrentVehicle)) YawLimit = Armed->GetMaxTurretYawLimit();
        const float Sens = UOCPlayerUserSettings::Get()->MouseSensitivity;
        LocalVehicleGunnerYaw = FMath::Clamp(LocalVehicleGunnerYaw + Value.Get<float>() * 1.35f * Sens, -YawLimit, YawLimit);
        UpdateVehicleGunnerView();
        if (HasAuthority()) ServerSetVehicleGunnerAim_Implementation(LocalVehicleGunnerYaw, LocalVehicleGunnerPitch);
        else ServerSetVehicleGunnerAim(LocalVehicleGunnerYaw, LocalVehicleGunnerPitch);
        return;
    }
    const UOCPlayerUserSettings* Settings = UOCPlayerUserSettings::Get();
    const float AimScale = bIsAiming ? Settings->AimSensitivityMultiplier : 1.0f;
    AddControllerYawInput(Value.Get<float>() * Settings->MouseSensitivity * AimScale);
}

void AOCCharacter::LookUp(const FInputActionValue& Value)
{
    if (bVehicleGunner && CurrentVehicle)
    {
        float MinPitch = -18.0f;
        float MaxPitch = 46.0f;
        if (const AOCArmedVehicleBase* Armed = Cast<AOCArmedVehicleBase>(CurrentVehicle))
        {
            MinPitch = Armed->GetMinTurretPitchLimit();
            MaxPitch = Armed->GetMaxTurretPitchLimit();
        }
        const UOCPlayerUserSettings* Settings = UOCPlayerUserSettings::Get();
        const float GunnerPitchSign = Settings->bInvertMouseY ? -1.0f : 1.0f;
        LocalVehicleGunnerPitch = FMath::Clamp(
            LocalVehicleGunnerPitch + Value.Get<float>() * 1.15f * Settings->MouseSensitivity * GunnerPitchSign,
            MinPitch,
            MaxPitch);
        UpdateVehicleGunnerView();
        if (HasAuthority()) ServerSetVehicleGunnerAim_Implementation(LocalVehicleGunnerYaw, LocalVehicleGunnerPitch);
        else ServerSetVehicleGunnerAim(LocalVehicleGunnerYaw, LocalVehicleGunnerPitch);
        return;
    }
    const UOCPlayerUserSettings* Settings = UOCPlayerUserSettings::Get();
    const float YSign = Settings->bInvertMouseY ? 1.0f : -1.0f;
    const float AimScale = bIsAiming ? Settings->AimSensitivityMultiplier : 1.0f;
    AddControllerPitchInput(Value.Get<float>() * Settings->MouseSensitivity * AimScale * YSign);
}

void AOCCharacter::JumpPressed()
{
    if (IsDowned())
    {
        if (HasAuthority())
        {
            ServerSetGiveUpHeld_Implementation(true);
        }
        else
        {
            ServerSetGiveUpHeld(true);
        }
        return;
    }

    if (!HealthComponent || HealthComponent->IsAlive())
    {
        Jump();
    }
}

void AOCCharacter::JumpReleased()
{
    StopJumping();

    if (GiveUpEndServerTime > 0.0 || IsDowned())
    {
        if (HasAuthority())
        {
            ServerSetGiveUpHeld_Implementation(false);
        }
        else
        {
            ServerSetGiveUpHeld(false);
        }
    }
}

void AOCCharacter::StartSprint()
{
    if (HealthComponent && !HealthComponent->IsAlive())
    {
        return;
    }

    bIsSprinting = true;
    bIsAiming = false;
    ApplyMovementSpeed();

    if (!HasAuthority())
    {
        ServerSetAiming(false);
        ServerSetSprinting(true);
        ServerSetFireHeld(false);
    }
}

void AOCCharacter::StopSprint()
{
    bIsSprinting = false;
    ApplyMovementSpeed();

    if (!HasAuthority())
    {
        ServerSetSprinting(false);
    }
}

void AOCCharacter::ServerSetSprinting_Implementation(bool bNewSprinting)
{
    bIsSprinting = bNewSprinting && (!HealthComponent || HealthComponent->IsAlive());
    if (bIsSprinting)
    {
        bIsAiming = false;
        StopServerFireTimer();
        if (CurrentWeapon)
        {
            CurrentWeapon->CancelReloadServer();
        }
    }
    ApplyMovementSpeed();
}

void AOCCharacter::OnRep_IsSprinting()
{
    ApplyMovementSpeed();
}

void AOCCharacter::StartAim()
{
    if (HealthComponent && !HealthComponent->IsAlive())
    {
        return;
    }

    bIsAiming = true;
    bIsSprinting = false;
    ApplyMovementSpeed();

    if (!HasAuthority())
    {
        ServerSetSprinting(false);
        ServerSetAiming(true);
    }
}

void AOCCharacter::StopAim()
{
    bIsAiming = false;
    ApplyMovementSpeed();

    if (!HasAuthority())
    {
        ServerSetAiming(false);
    }
}

void AOCCharacter::ServerSetAiming_Implementation(bool bNewAiming)
{
    bIsAiming = bNewAiming && (!HealthComponent || HealthComponent->IsAlive());
    if (bIsAiming)
    {
        bIsSprinting = false;
    }
    ApplyMovementSpeed();
}

void AOCCharacter::OnRep_IsAiming()
{
    ApplyMovementSpeed();
}

void AOCCharacter::OnRep_InVehicle()
{
    if (bVehicleGunner)
    {
        LocalVehicleGunnerYaw = 0.0f;
        LocalVehicleGunnerPitch = 0.0f;
    }
    ApplyVehicleOccupancyPresentation();
    UpdateVehicleGunnerView();
}

void AOCCharacter::EnterVehicleServer(AOCVehicleBase* Vehicle)
{
    if (!HasAuthority() || !Vehicle || bInVehicle || !HealthComponent || !HealthComponent->IsAlive())
    {
        return;
    }

    StopServerFireTimer();
    CancelReviveServer();
    if (CurrentWeapon) CurrentWeapon->CancelReloadServer();

    CurrentVehicle = Vehicle;
    bInVehicle = true;
    bVehicleGunner = false;
    AttachToActor(Vehicle, FAttachmentTransformRules::KeepWorldTransform);
    SetActorLocation(Vehicle->GetActorLocation() + FVector(0.0f, 0.0f, 40.0f));
    ApplyVehicleOccupancyPresentation();
    ForceNetUpdate();
}

void AOCCharacter::ExitVehicleServer(const FVector& ExitLocation, const FRotator& ExitRotation)
{
    if (!HasAuthority())
    {
        return;
    }

    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    CurrentVehicle = nullptr;
    bInVehicle = false;
    bVehicleGunner = false;
    SetActorLocationAndRotation(ExitLocation, ExitRotation, false, nullptr, ETeleportType::TeleportPhysics);
    ApplyVehicleOccupancyPresentation();
    ForceNetUpdate();
}

void AOCCharacter::EnterVehicleGunnerServer(AOCArmedVehicleBase* Vehicle, const FVector& GunnerCameraWorldLocation)
{
    if (!HasAuthority() || !Vehicle || bInVehicle || !HealthComponent || !HealthComponent->IsAlive())
    {
        return;
    }

    StopServerFireTimer();
    CancelReviveServer();
    if (CurrentWeapon) CurrentWeapon->CancelReloadServer();

    CurrentVehicle = Vehicle;
    bInVehicle = true;
    bVehicleGunner = true;
    LocalVehicleGunnerYaw = 0.0f;
    LocalVehicleGunnerPitch = 0.0f;
    AttachToActor(Vehicle, FAttachmentTransformRules::KeepWorldTransform);
    SetActorLocation(GunnerCameraWorldLocation - FVector(0.0f, 0.0f, 64.0f));
    ApplyVehicleOccupancyPresentation();
    ForceNetUpdate();
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_M2_GUNNER_PITCH_CONTRACT_READY default_invert=0 mouse_up_raises=1 direct_local_pitch=1"));
}

void AOCCharacter::ExitVehicleGunnerServer(const FVector& ExitLocation, const FRotator& ExitRotation)
{
    if (!HasAuthority())
    {
        return;
    }

    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    CurrentVehicle = nullptr;
    bInVehicle = false;
    bVehicleGunner = false;
    LocalVehicleGunnerYaw = 0.0f;
    LocalVehicleGunnerPitch = 0.0f;
    SetActorLocationAndRotation(ExitLocation, ExitRotation, false, nullptr, ETeleportType::TeleportPhysics);
    ApplyVehicleOccupancyPresentation();
    ForceNetUpdate();
}

void AOCCharacter::UpdateVehicleGunnerView()
{
    if (!IsLocallyControlled() || !bVehicleGunner || !CurrentVehicle || !Controller)
    {
        return;
    }
    const float WorldYaw = CurrentVehicle->GetActorRotation().Yaw + LocalVehicleGunnerYaw;
    Controller->SetControlRotation(FRotator(LocalVehicleGunnerPitch, WorldYaw, 0.0f));
}

void AOCCharacter::ApplyVehicleOccupancyPresentation()
{
    SetActorHiddenInGame(bInVehicle);
    SetActorEnableCollision(!bInVehicle);
    if (GetCharacterMovement())
    {
        if (bInVehicle)
        {
            GetCharacterMovement()->DisableMovement();
        }
        else if (HealthComponent && HealthComponent->IsAlive())
        {
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
            ApplyMovementSpeed();
        }
    }
    RefreshWeaponPresentation();
}


void AOCCharacter::ApplyMovementSpeed()
{
    if (!GetCharacterMovement())
    {
        return;
    }

    float TargetSpeed = WalkSpeed;
    if (IsDowned())
    {
        TargetSpeed = DownedCrawlSpeed;
    }
    else if (bIsSprinting)
    {
        TargetSpeed = SprintSpeed;
    }
    else if (bIsAiming)
    {
        TargetSpeed = AimWalkSpeed;
    }

    GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
    GetCharacterMovement()->MaxWalkSpeedCrouched = IsDowned() ? DownedCrawlSpeed : CrouchSpeed;
}

void AOCCharacter::ToggleCrouch()
{
    if (HealthComponent && !HealthComponent->IsAlive())
    {
        return;
    }

    if (bIsCrouched)
    {
        UnCrouch();
    }
    else
    {
        Crouch();
    }
}

void AOCCharacter::FirePressed()
{
    if (bVehicleGunner && CurrentVehicle)
    {
        if (HasAuthority()) ServerSetVehicleGunnerFireHeld_Implementation(true);
        else ServerSetVehicleGunnerFireHeld(true);
        return;
    }

    if (!CurrentWeapon || !Controller || bIsSprinting || (HealthComponent && !HealthComponent->IsAlive()))
    {
        return;
    }

    if (HasAuthority())
    {
        ServerSetFireHeld_Implementation(true);
    }
    else
    {
        ServerSetFireHeld(true);
    }
}

void AOCCharacter::FireReleased()
{
    if (bVehicleGunner && CurrentVehicle)
    {
        if (HasAuthority()) ServerSetVehicleGunnerFireHeld_Implementation(false);
        else ServerSetVehicleGunnerFireHeld(false);
        return;
    }

    if (HasAuthority())
    {
        ServerSetFireHeld_Implementation(false);
    }
    else
    {
        ServerSetFireHeld(false);
    }
}

void AOCCharacter::ServerSetVehicleGunnerAim_Implementation(float RelativeYaw, float RelativePitch)
{
    if (!bVehicleGunner || !CurrentVehicle) return;
    if (AOCArmedVehicleBase* Armed = Cast<AOCArmedVehicleBase>(CurrentVehicle))
    {
        Armed->SetGunnerAimServer(this, RelativeYaw, RelativePitch);
    }
}

void AOCCharacter::ServerSetVehicleGunnerFireHeld_Implementation(bool bHeld)
{
    if (!bVehicleGunner || !CurrentVehicle) return;
    if (AOCArmedVehicleBase* Armed = Cast<AOCArmedVehicleBase>(CurrentVehicle))
    {
        Armed->SetGunnerFireHeldServer(this, bHeld);
    }
}

void AOCCharacter::ServerReloadVehicleTurret_Implementation()
{
    if (!bVehicleGunner || !CurrentVehicle) return;
    if (AOCArmedVehicleBase* Armed = Cast<AOCArmedVehicleBase>(CurrentVehicle))
    {
        Armed->RequestGunnerReloadServer(this);
    }
}

void AOCCharacter::ServerSetFireHeld_Implementation(bool bHeld)
{
    const AOCGameState* MatchState = GetWorld() ? GetWorld()->GetGameState<AOCGameState>() : nullptr;

    // Trigger release stops automatic fire immediately, but an already accepted finite three-round burst owns the
    // remaining pulses. Otherwise rapid release could turn Burst3 into an accidental semi-auto selector.
    if (!bHeld)
    {
        bServerFireHeld = false;
        if (ServerBurstShotsRemaining <= 0)
        {
            GetWorldTimerManager().ClearTimer(ServerFireTimerHandle);
        }
        return;
    }

    // Do not restart or stack a burst while its authoritative sequence is still in flight.
    if (ServerBurstShotsRemaining > 0)
    {
        return;
    }

    StopServerFireTimer();
    bServerFireHeld = true;

    if ((MatchState && MatchState->GetOCMatchPhase() == EOCMatchPhase::Ended) ||
        !CurrentWeapon || bIsSprinting || (HealthComponent && !HealthComponent->IsAlive()))
    {
        StopServerFireTimer();
        return;
    }

    if (CurrentWeapon->IsReloading())
    {
        if (CurrentWeapon->GetAmmoInMagazine() <= 0)
        {
            StopServerFireTimer();
            return;
        }
        CurrentWeapon->CancelReloadServer();
    }

    const EOCFireMode FireMode = CurrentWeapon->GetCurrentFireMode();
    if (FireMode == EOCFireMode::Burst3)
    {
        ServerBurstShotsRemaining = FMath::Min(3, CurrentWeapon->GetAmmoInMagazine());
    }

    ServerHandleFirePulse();

    if (!CurrentWeapon || CurrentWeapon->GetAmmoInMagazine() <= 0)
    {
        return;
    }

    const float Interval = FMath::Max(0.02f, CurrentWeapon->GetFireInterval());
    if (FireMode == EOCFireMode::Automatic && bServerFireHeld)
    {
        GetWorldTimerManager().SetTimer(ServerFireTimerHandle, this, &AOCCharacter::ServerHandleFirePulse,
            Interval, true, Interval);
    }
    else if (FireMode == EOCFireMode::Burst3 && ServerBurstShotsRemaining > 0)
    {
        GetWorldTimerManager().SetTimer(ServerFireTimerHandle, this, &AOCCharacter::ServerHandleFirePulse,
            Interval, true, Interval);
        UE_LOG(LogTemp, Verbose,
            TEXT("PASS45_BURST3_SEQUENCE_READY authoritative=1 finite_shots=3 release_cancel=0 remaining=%d"),
            ServerBurstShotsRemaining);
    }
    else
    {
        bServerFireHeld = false;
    }
}

void AOCCharacter::ServerHandleFirePulse()
{
    const AOCGameState* MatchState = GetWorld() ? GetWorld()->GetGameState<AOCGameState>() : nullptr;
    const bool bBurstActive = ServerBurstShotsRemaining > 0;
    if (!HasAuthority() || (MatchState && MatchState->GetOCMatchPhase() == EOCMatchPhase::Ended) ||
        (!bServerFireHeld && !bBurstActive) || !CurrentWeapon || !Controller || bIsSprinting ||
        (HealthComponent && !HealthComponent->IsAlive()))
    {
        StopServerFireTimer();
        return;
    }

    FVector ViewLocation;
    FRotator ViewRotation;
    Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

    const bool bMoving = GetVelocity().SizeSquared2D() > FMath::Square(25.0f);
    FHitResult Hit;
    bool bDamagedActor = false;
    bool bFatalHit = false;
    const EOCFireMode FireMode = CurrentWeapon->GetCurrentFireMode();
    const bool bShotFired = CurrentWeapon->TryFireServer(
        this, ViewLocation, ViewRotation.Vector(), bIsAiming, bMoving, Hit, bDamagedActor, bFatalHit);

    if (bShotFired)
    {
        if (FireMode == EOCFireMode::Burst3 && ServerBurstShotsRemaining > 0)
        {
            --ServerBurstShotsRemaining;
        }
        if (CharacterVisualComponent) CharacterVisualComponent->BroadcastActionServer(EOCCharacterActionEvent::Fire);
        if (bDamagedActor) ClientConfirmHit(bFatalHit);
    }

    if (!bShotFired || CurrentWeapon->GetAmmoInMagazine() <= 0)
    {
        StopServerFireTimer();
        return;
    }

    if (FireMode == EOCFireMode::Burst3)
    {
        if (ServerBurstShotsRemaining <= 0)
        {
            StopServerFireTimer();
        }
        return;
    }

    if (FireMode != EOCFireMode::Automatic)
    {
        GetWorldTimerManager().ClearTimer(ServerFireTimerHandle);
        bServerFireHeld = false;
    }
}

void AOCCharacter::StopServerFireTimer()
{
    GetWorldTimerManager().ClearTimer(ServerFireTimerHandle);
    bServerFireHeld = false;
    ServerBurstShotsRemaining = 0;
}

void AOCCharacter::ReloadPressed()
{
    if (bVehicleGunner && CurrentVehicle)
    {
        if (HasAuthority()) ServerReloadVehicleTurret_Implementation();
        else ServerReloadVehicleTurret();
        return;
    }

    if (!CurrentWeapon || (HealthComponent && !HealthComponent->IsAlive()))
    {
        return;
    }

    if (HasAuthority())
    {
        ServerSetFireHeld_Implementation(false);
        ServerReload_Implementation();
    }
    else
    {
        ServerSetFireHeld(false);
        ServerReload();
    }
}

void AOCCharacter::ServerReload_Implementation()
{
    StopServerFireTimer();
    if (CurrentWeapon && (!HealthComponent || HealthComponent->IsAlive()) && !bIsSprinting)
    {
        if (CurrentWeapon->BeginReloadServer() && CharacterVisualComponent)
        {
            CharacterVisualComponent->BroadcastActionServer(EOCCharacterActionEvent::ReloadStart);
        }
    }
}

void AOCCharacter::CycleFireModePressed()
{
    if (HasAuthority())
    {
        ServerCycleFireMode_Implementation();
    }
    else
    {
        ServerSetFireHeld(false);
        ServerCycleFireMode();
    }
}

void AOCCharacter::ServerCycleFireMode_Implementation()
{
    StopServerFireTimer();
    if (CurrentWeapon && (!HealthComponent || HealthComponent->IsAlive()))
    {
        CurrentWeapon->CycleFireModeServer();
    }
}

void AOCCharacter::ClientConfirmHit_Implementation(bool bFatalHit)
{
    LastHitConfirmTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
    bLastHitFatal = bFatalHit;
}

void AOCCharacter::NotifyConfirmedWeaponShotPresentation()
{
    if (!IsLocallyControlled())
    {
        return;
    }

    if (FireCameraShakeClass)
    {
        if (APlayerController* PC = Cast<APlayerController>(Controller))
        {
            PC->ClientStartCameraShake(
                FireCameraShakeClass,
                FMath::Clamp(UOCPlayerUserSettings::Get()->CameraShakeScale, 0.0f, 1.0f));
        }
    }
}

void AOCCharacter::NotifyDamageReceived(const FVector& DamageOrigin)
{
    if (!HasAuthority())
    {
        return;
    }
    ClientDamageFeedback(DamageOrigin);
}

void AOCCharacter::ClientDamageFeedback_Implementation(FVector_NetQuantize DamageOrigin)
{
    const FVector ToDamage = (FVector(DamageOrigin) - GetActorLocation()).GetSafeNormal2D();
    if (!ToDamage.IsNearlyZero())
    {
        const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
        const FVector Right = GetActorRightVector().GetSafeNormal2D();
        LastDamageDirectionDegrees = FMath::RadiansToDegrees(FMath::Atan2(
            FVector::DotProduct(ToDamage, Right), FVector::DotProduct(ToDamage, Forward)));
    }
    LastDamageFeedbackTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
}

float AOCCharacter::GetCrosshairGap() const
{
    float Gap = bIsAiming ? 2.0f : 5.0f;

    if (GetCharacterMovement())
    {
        const float SpeedAlpha = FMath::Clamp(GetVelocity().Size2D() / FMath::Max(1.0f, SprintSpeed), 0.0f, 1.0f);
        Gap += SpeedAlpha * (bIsAiming ? 2.0f : 7.0f);
        if (GetCharacterMovement()->IsFalling())
        {
            Gap += 6.0f;
        }
    }

    if (CurrentWeapon)
    {
        Gap += FMath::Clamp(CurrentWeapon->GetConfirmedLocalRecoilPitchOffset() * 1.5f, 0.0f, 8.0f);
    }
    return Gap;
}

float AOCCharacter::GetHitMarkerAlpha() const
{
    if (!GetWorld())
    {
        return 0.0f;
    }

    const float Age = static_cast<float>(GetWorld()->GetTimeSeconds() - LastHitConfirmTime);
    return 1.0f - FMath::Clamp(Age / 0.16f, 0.0f, 1.0f);
}

float AOCCharacter::GetDamageIndicatorAlpha() const
{
    if (!GetWorld())
    {
        return 0.0f;
    }

    const float Age = static_cast<float>(GetWorld()->GetTimeSeconds() - LastDamageFeedbackTime);
    return 1.0f - FMath::Clamp(Age / 0.85f, 0.0f, 1.0f);
}

void AOCCharacter::OnRep_CurrentWeapon()
{
    RefreshWeaponPresentation();
}

void AOCCharacter::OnRep_Inventory()
{
    RefreshWeaponPresentation();
}

void AOCCharacter::RefreshWeaponPresentation()
{
    const bool bCanPresentEquippedWeapon = (!HealthComponent || HealthComponent->IsAlive()) && !bInVehicle;
    if (PrimaryWeapon)
    {
        PrimaryWeapon->ApplyInventoryPresentation(bCanPresentEquippedWeapon && PrimaryWeapon == CurrentWeapon, FirstPersonCamera);
    }
    if (SecondaryWeapon)
    {
        SecondaryWeapon->ApplyInventoryPresentation(bCanPresentEquippedWeapon && SecondaryWeapon == CurrentWeapon, FirstPersonCamera);
    }
}

void AOCCharacter::InteractPressed()
{
    if (HealthComponent && !HealthComponent->IsAlive())
    {
        return;
    }

    if (HasAuthority())
    {
        ServerInteract_Implementation();
    }
    else
    {
        ServerInteract();
    }
}

void AOCCharacter::InteractReleased()
{
    if (HasAuthority())
    {
        ServerCancelInteract_Implementation();
    }
    else
    {
        ServerCancelInteract();
    }
}

void AOCCharacter::DropWeaponPressed()
{
    if (!CurrentWeapon || (HealthComponent && !HealthComponent->IsAlive()))
    {
        return;
    }

    if (HasAuthority())
    {
        ServerDropCurrentWeapon_Implementation();
    }
    else
    {
        ServerDropCurrentWeapon();
    }
}

void AOCCharacter::EquipPrimaryPressed()
{
    if (HasAuthority())
    {
        ServerEquipWeaponSlot_Implementation(EOCInventorySlot::Primary);
    }
    else
    {
        ServerEquipWeaponSlot(EOCInventorySlot::Primary);
    }
}

void AOCCharacter::EquipSecondaryPressed()
{
    if (HasAuthority())
    {
        ServerEquipWeaponSlot_Implementation(EOCInventorySlot::Secondary);
    }
    else
    {
        ServerEquipWeaponSlot(EOCInventorySlot::Secondary);
    }
}

void AOCCharacter::ServerEquipWeaponSlot_Implementation(EOCInventorySlot RequestedSlot)
{
    EquipSlotServer(RequestedSlot);
}

void AOCCharacter::EquipSlotServer(EOCInventorySlot Slot)
{
    if (!HasAuthority() || (HealthComponent && !HealthComponent->IsAlive()))
    {
        return;
    }

    AOCWeaponBase* RequestedWeapon = nullptr;
    if (Slot == EOCInventorySlot::Primary)
    {
        RequestedWeapon = PrimaryWeapon;
    }
    else if (Slot == EOCInventorySlot::Secondary)
    {
        RequestedWeapon = SecondaryWeapon;
    }

    if (!RequestedWeapon || RequestedWeapon == CurrentWeapon)
    {
        return;
    }

    StopServerFireTimer();
    if (CurrentWeapon)
    {
        CurrentWeapon->CancelReloadServer();
        CurrentWeapon->StoreInInventoryServer(this);
    }

    CurrentWeapon = RequestedWeapon;
    ActiveWeaponSlot = Slot;
    CurrentWeapon->EquipToCharacterServer(this);
    RefreshWeaponPresentation();
    ForceNetUpdate();
}

AOCWeaponBase* AOCCharacter::SpawnInventoryWeaponServer(TSubclassOf<AOCWeaponBase> WeaponClass)
{
    if (!HasAuthority() || !WeaponClass)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AOCWeaponBase* Weapon = GetWorld()->SpawnActor<AOCWeaponBase>(WeaponClass, GetActorTransform(), SpawnParams);
    if (Weapon)
    {
        Weapon->StoreInInventoryServer(this);
    }
    return Weapon;
}

void AOCCharacter::SpawnStarterLoadoutServer()
{
    if (!HasAuthority() || PrimaryWeapon || SecondaryWeapon)
    {
        return;
    }

    PrimaryWeapon = SpawnInventoryWeaponServer(StarterPrimaryWeaponClass);
    SecondaryWeapon = SpawnInventoryWeaponServer(StarterSecondaryWeaponClass);
    CurrentWeapon = PrimaryWeapon ? PrimaryWeapon : SecondaryWeapon;
    ActiveWeaponSlot = PrimaryWeapon ? EOCInventorySlot::Primary
        : (SecondaryWeapon ? EOCInventorySlot::Secondary : EOCInventorySlot::None);

    if (CurrentWeapon)
    {
        CurrentWeapon->EquipToCharacterServer(this);
    }
    RefreshWeaponPresentation();
    ForceNetUpdate();
}

void AOCCharacter::ServerDropCurrentWeapon_Implementation()
{
    if (!CurrentWeapon || (HealthComponent && !HealthComponent->IsAlive()))
    {
        return;
    }

    DropWeaponServer(CurrentWeapon);
}

void AOCCharacter::DropWeaponServer(AOCWeaponBase* Weapon)
{
    if (!HasAuthority() || !Weapon)
    {
        return;
    }

    StopServerFireTimer();
    Weapon->CancelReloadServer();

    const bool bWasCurrent = Weapon == CurrentWeapon;
    const FVector DropLocation = GetActorLocation() + GetActorForwardVector() * 110.0f + FVector(0.0f, 0.0f, 35.0f);
    const FRotator DropRotation(0.0f, GetActorRotation().Yaw, 0.0f);

    if (Weapon == PrimaryWeapon)
    {
        PrimaryWeapon = nullptr;
    }
    if (Weapon == SecondaryWeapon)
    {
        SecondaryWeapon = nullptr;
    }

    Weapon->DropToWorldServer(DropLocation, DropRotation);

    if (bWasCurrent)
    {
        CurrentWeapon = PrimaryWeapon ? PrimaryWeapon : SecondaryWeapon;
        ActiveWeaponSlot = PrimaryWeapon ? EOCInventorySlot::Primary
            : (SecondaryWeapon ? EOCInventorySlot::Secondary : EOCInventorySlot::None);
        if (CurrentWeapon)
        {
            CurrentWeapon->EquipToCharacterServer(this);
        }
    }

    RefreshWeaponPresentation();
    ForceNetUpdate();
}

AOCWeaponBase* AOCCharacter::FindClosestWorldWeapon(float& OutDistance) const
{
    OutDistance = InteractionDistance + 1.0f;
    AOCWeaponBase* Best = nullptr;
    if (!GetWorld())
    {
        return Best;
    }

    for (TActorIterator<AOCWeaponBase> It(GetWorld()); It; ++It)
    {
        AOCWeaponBase* Candidate = *It;
        if (!Candidate || !Candidate->IsWorldPickup())
        {
            continue;
        }

        const float Distance = FVector::Dist(GetActorLocation(), Candidate->GetActorLocation());
        if (Distance <= InteractionDistance && Distance < OutDistance)
        {
            OutDistance = Distance;
            Best = Candidate;
        }
    }
    return Best;
}

AOCAmmoBox* AOCCharacter::FindClosestAmmoBox(float& OutDistance) const
{
    OutDistance = InteractionDistance + 1.0f;
    AOCAmmoBox* Best = nullptr;
    if (!GetWorld())
    {
        return Best;
    }

    for (TActorIterator<AOCAmmoBox> It(GetWorld()); It; ++It)
    {
        AOCAmmoBox* Candidate = *It;
        if (!Candidate)
        {
            continue;
        }

        const float Distance = FVector::Dist(GetActorLocation(), Candidate->GetActorLocation());
        if (Distance <= InteractionDistance && Distance < OutDistance)
        {
            OutDistance = Distance;
            Best = Candidate;
        }
    }
    return Best;
}

AOCInteractableActor* AOCCharacter::FindFocusedWorldInteractable(float& OutDistance) const
{
    OutDistance = WorldInteractionTraceDistance + 1.0f;
    if (!GetWorld())
    {
        return nullptr;
    }

    FVector EyeLocation;
    FRotator EyeRotation;
    GetActorEyesViewPoint(EyeLocation, EyeRotation);
    const FVector TraceEnd = EyeLocation + EyeRotation.Vector() * WorldInteractionTraceDistance;

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(OCWorldInteractionTrace), false, this);
    QueryParams.AddIgnoredActor(this);

    FHitResult Hit;
    if (!GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryParams))
    {
        return nullptr;
    }

    AOCInteractableActor* Interactable = Cast<AOCInteractableActor>(Hit.GetActor());
    if (!Interactable)
    {
        return nullptr;
    }

    OutDistance = FVector::Dist(GetActorLocation(), Interactable->GetActorLocation());
    return OutDistance <= Interactable->GetMaxInteractionDistance() ? Interactable : nullptr;
}

AOCVehicleBase* AOCCharacter::FindFocusedVehicle(float& OutDistance) const
{
    OutDistance = WorldInteractionTraceDistance + 1.0f;
    if (!GetWorld())
    {
        return nullptr;
    }

    FVector EyeLocation;
    FRotator EyeRotation;
    GetActorEyesViewPoint(EyeLocation, EyeRotation);
    const FVector TraceEnd = EyeLocation + EyeRotation.Vector() * WorldInteractionTraceDistance;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(OCVehicleInteractionTrace), false, this);
    Params.AddIgnoredActor(this);
    FHitResult Hit;
    if (!GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, Params))
    {
        return nullptr;
    }

    AOCVehicleBase* Vehicle = Cast<AOCVehicleBase>(Hit.GetActor());
    if (!Vehicle)
    {
        return nullptr;
    }

    OutDistance = FVector::Dist(GetActorLocation(), Vehicle->GetActorLocation());
    return OutDistance <= WorldInteractionTraceDistance ? Vehicle : nullptr;
}

AOCCharacter* AOCCharacter::FindClosestDownedCharacter(float& OutDistance) const
{
    OutDistance = ReviveDistance + 1.0f;
    AOCCharacter* Best = nullptr;
    const AOCPlayerState* SelfState = GetPlayerState<AOCPlayerState>();
    if (!GetWorld() || !bHasMedicCapability || !SelfState || !SelfState->IsMedic() || SelfState->GetTeamId() == EOCTeam::None)
    {
        return Best;
    }

    for (TActorIterator<AOCCharacter> It(GetWorld()); It; ++It)
    {
        AOCCharacter* Candidate = *It;
        const AOCPlayerState* CandidateState = Candidate ? Candidate->GetPlayerState<AOCPlayerState>() : nullptr;
        if (!Candidate || Candidate == this || !CandidateState || CandidateState->GetTeamId() != SelfState->GetTeamId() ||
            !Candidate->GetHealthComponent() || !Candidate->GetHealthComponent()->IsDowned())
        {
            continue;
        }

        const float Distance = FVector::Dist(GetActorLocation(), Candidate->GetActorLocation());
        if (Distance <= ReviveDistance && Distance < OutDistance)
        {
            OutDistance = Distance;
            Best = Candidate;
        }
    }
    return Best;
}

bool AOCCharacter::CanReviveTargetServer(const AOCCharacter* Target) const
{
    const AOCPlayerState* SelfState = GetPlayerState<AOCPlayerState>();
    const AOCPlayerState* TargetState = Target ? Target->GetPlayerState<AOCPlayerState>() : nullptr;
    if (!HasAuthority() || !bHasMedicCapability || !SelfState || !SelfState->IsMedic() || !TargetState ||
        SelfState->GetTeamId() == EOCTeam::None || SelfState->GetTeamId() != TargetState->GetTeamId() ||
        !Target || Target == this || !HealthComponent || !HealthComponent->IsAlive() || !Target->GetHealthComponent() ||
        !Target->GetHealthComponent()->IsDowned())
    {
        return false;
    }

    if (FVector::DistSquared(GetActorLocation(), Target->GetActorLocation()) > FMath::Square(ReviveDistance))
    {
        return false;
    }

    FVector EyeLocation;
    FRotator EyeRotation;
    GetActorEyesViewPoint(EyeLocation, EyeRotation);

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(OCReviveLineOfSight), false, this);
    FHitResult Hit;
    const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
        Hit, EyeLocation, Target->GetActorLocation() + FVector(0.0f, 0.0f, 35.0f), ECC_Visibility, QueryParams);

    return !bBlocked || Hit.GetActor() == Target;
}

void AOCCharacter::StartReviveServer(AOCCharacter* Target)
{
    if (!CanReviveTargetServer(Target))
    {
        return;
    }

    CancelReviveServer();
    ReviveTarget = Target;
    ReviveEndServerTime = GetSynchronizedServerTime() + static_cast<double>(ReviveHoldSeconds);
    if (CharacterVisualComponent) CharacterVisualComponent->BroadcastActionServer(EOCCharacterActionEvent::ReviveStart);
    GetWorldTimerManager().SetTimer(ReviveTimerHandle, this, &AOCCharacter::CompleteReviveServer, ReviveHoldSeconds, false);
    ForceNetUpdate();
}

void AOCCharacter::CancelReviveServer()
{
    if (!HasAuthority())
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(ReviveTimerHandle);
    ReviveTarget = nullptr;
    ReviveEndServerTime = 0.0;
    ForceNetUpdate();
}

void AOCCharacter::CompleteReviveServer()
{
    if (!HasAuthority())
    {
        return;
    }

    AOCCharacter* Target = ReviveTarget;
    if (!CanReviveTargetServer(Target))
    {
        CancelReviveServer();
        return;
    }

    const bool bRevived = Target->GetHealthComponent()->ReviveServer(GetController());
    if (bRevived)
    {
        if (AOCPlayerState* ReviverState = GetPlayerState<AOCPlayerState>())
        {
            ReviverState->RegisterRevive(50);
        }
        if (CharacterVisualComponent) CharacterVisualComponent->BroadcastActionServer(EOCCharacterActionEvent::ReviveComplete);
    }
    CancelReviveServer();
}


void AOCCharacter::ThrowGrenadePressed()
{
    if (HealthComponent && !HealthComponent->IsAlive()) return;
    if (HasAuthority()) ServerThrowSelectedGrenade_Implementation(); else ServerThrowSelectedGrenade();
}

void AOCCharacter::CycleGrenadePressed()
{
    if (HasAuthority()) ServerCycleGrenadeType_Implementation(); else ServerCycleGrenadeType();
}

void AOCCharacter::DeployTrapPressed()
{
    if (HealthComponent && !HealthComponent->IsAlive()) return;
    if (HasAuthority()) ServerDeploySelectedTrap_Implementation(); else ServerDeploySelectedTrap();
}

void AOCCharacter::CycleTrapPressed()
{
    if (HasAuthority()) ServerCycleTrapPreset_Implementation(); else ServerCycleTrapPreset();
}

void AOCCharacter::ServerThrowSelectedGrenade_Implementation()
{
    if (!HealthComponent || !HealthComponent->IsAlive() || bInVehicle) return;
    int32* Count = SelectedGrenadeType == EOCGrenadeType::Fragmentation ? &FragGrenades :
        (SelectedGrenadeType == EOCGrenadeType::Smoke ? &SmokeGrenades : &FlashGrenades);
    if (!Count || *Count <= 0) return;
    --(*Count);
    const FVector Origin = FirstPersonCamera ? FirstPersonCamera->GetComponentLocation() : GetActorLocation()+FVector(0,0,60);
    const FVector Forward = FirstPersonCamera ? FirstPersonCamera->GetForwardVector() : GetActorForwardVector();
    FActorSpawnParameters Params; Params.Owner=this; Params.Instigator=this; Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    if (AOCGrenadeProjectile* G = GetWorld()->SpawnActor<AOCGrenadeProjectile>(AOCGrenadeProjectile::StaticClass(), Origin+Forward*70.0f, Forward.Rotation(), Params))
    {
        G->InitializeGrenadeServer(SelectedGrenadeType, Forward*1350.0f + FVector::UpVector*190.0f);
    }
    ForceNetUpdate();
}

void AOCCharacter::ServerCycleGrenadeType_Implementation()
{
    const int32 Next=(static_cast<int32>(SelectedGrenadeType)+1)%3;
    SelectedGrenadeType=static_cast<EOCGrenadeType>(Next);
    ForceNetUpdate();
}

void AOCCharacter::ServerDeploySelectedTrap_Implementation()
{
    const AOCPlayerState* State=GetPlayerState<AOCPlayerState>();
    if (!State || !State->IsEngineer() || TrapCount<=0 || !HealthComponent || !HealthComponent->IsAlive() || bInVehicle) return;
    const FVector Start=GetActorLocation()+GetActorForwardVector()*145.0f+FVector(0,0,80);
    const FVector End=Start-FVector(0,0,260);
    FHitResult Hit; FCollisionQueryParams Params(SCENE_QUERY_STAT(OCTrapPlacement),false,this);
    FVector Place=End;
    if(GetWorld()->LineTraceSingleByChannel(Hit,Start,End,ECC_Visibility,Params)) Place=Hit.ImpactPoint+FVector(0,0,4);
    FActorSpawnParameters SpawnParams; SpawnParams.Owner=this; SpawnParams.Instigator=this; SpawnParams.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    if(AOCDeployableTrap* Trap=GetWorld()->SpawnActor<AOCDeployableTrap>(AOCDeployableTrap::StaticClass(),Place,FRotator(0,GetActorRotation().Yaw,0),SpawnParams))
    {
        Trap->ConfigureTrapServer(SelectedTrapPreset,State->GetTeamId()); --TrapCount; ForceNetUpdate();
    }
}

void AOCCharacter::ServerCycleTrapPreset_Implementation()
{
    const int32 Next=(static_cast<int32>(SelectedTrapPreset)+1)%15;
    SelectedTrapPreset=static_cast<EOCTrapPreset>(Next); ForceNetUpdate();
}

int32 AOCCharacter::GetSelectedGrenadeCount() const
{
    if(SelectedGrenadeType==EOCGrenadeType::Fragmentation) return FragGrenades;
    if(SelectedGrenadeType==EOCGrenadeType::Smoke) return SmokeGrenades;
    return FlashGrenades;
}

void AOCCharacter::ApplyFlashEffectServer(float Intensity, float DurationSeconds)
{
    if(!HasAuthority()) return;
    ClientApplyFlash(FMath::Clamp(Intensity,0.0f,1.0f),FMath::Clamp(DurationSeconds,0.1f,6.0f));
}

void AOCCharacter::ClientApplyFlash_Implementation(float Intensity, float DurationSeconds)
{
    FlashEffectPeakAlpha=FMath::Clamp(Intensity,0.0f,1.0f);
    FlashEffectDuration=FMath::Max(0.1f,DurationSeconds);
    FlashEffectEndLocalTime=(GetWorld()?GetWorld()->GetTimeSeconds():0.0)+FlashEffectDuration;
}

float AOCCharacter::GetFlashEffectAlpha() const
{
    if(!GetWorld()||FlashEffectEndLocalTime<=0.0||FlashEffectDuration<=0.0f) return 0.0f;
    const float Remaining=FMath::Max(0.0f,static_cast<float>(FlashEffectEndLocalTime-GetWorld()->GetTimeSeconds()));
    const float AccessibilityScale = UOCPlayerUserSettings::Get()->bReduceFlashes ? 0.35f : 1.0f;
    return FlashEffectPeakAlpha * AccessibilityScale * FMath::Clamp(Remaining/FlashEffectDuration,0.0f,1.0f);
}

void AOCCharacter::ServerInteract_Implementation()
{
    if (HealthComponent && !HealthComponent->IsAlive())
    {
        return;
    }

    if (bVehicleGunner && CurrentVehicle)
    {
        if (AOCArmedVehicleBase* Armed = Cast<AOCArmedVehicleBase>(CurrentVehicle))
        {
            Armed->ExitGunnerServer(this, false);
        }
        return;
    }

    CancelReviveServer();

    float DownedDistance = 0.0f;
    if (AOCCharacter* DownedCharacter = FindClosestDownedCharacter(DownedDistance))
    {
        if (CanReviveTargetServer(DownedCharacter))
        {
            StartReviveServer(DownedCharacter);
            return;
        }
    }

    float WorldInteractDistance = 0.0f;
    if (AOCInteractableActor* Interactable = FindFocusedWorldInteractable(WorldInteractDistance))
    {
        if (Interactable->CanInteractServer(this))
        {
            Interactable->InteractServer(this);
            return;
        }
    }

    float VehicleDistance = 0.0f;
    if (AOCVehicleBase* Vehicle = FindFocusedVehicle(VehicleDistance))
    {
        if (const AOCPlayerState* SelfState = GetPlayerState<AOCPlayerState>(); SelfState && SelfState->IsEngineer() &&
            !Vehicle->IsVehicleDestroyed() && Vehicle->GetVehicleHealthNormalized() < 0.999f)
        {
            if (Vehicle->RepairVehicleServer(65.0f, this) > 0.0f) return;
        }
        if (Vehicle->TryEnterVehicleServer(this))
        {
            return;
        }
    }

    float WeaponDistance = 0.0f;
    float AmmoDistance = 0.0f;
    AOCWeaponBase* Weapon = FindClosestWorldWeapon(WeaponDistance);
    AOCAmmoBox* AmmoBox = FindClosestAmmoBox(AmmoDistance);

    if (Weapon && (!AmmoBox || WeaponDistance <= AmmoDistance))
    {
        PickupWeaponServer(Weapon);
        return;
    }

    if (AmmoBox)
    {
        AmmoBox->TryGiveAmmoServer(this);
    }
}

void AOCCharacter::ServerCancelInteract_Implementation()
{
    CancelReviveServer();
}

void AOCCharacter::PickupWeaponServer(AOCWeaponBase* Weapon)
{
    if (!HasAuthority() || !Weapon || !Weapon->IsWorldPickup())
    {
        return;
    }

    StopServerFireTimer();
    if (CurrentWeapon)
    {
        CurrentWeapon->CancelReloadServer();
    }

    EOCInventorySlot Slot = Weapon->GetPreferredSlot();
    if (Slot == EOCInventorySlot::None)
    {
        Slot = EOCInventorySlot::Primary;
    }

    AOCWeaponBase* Existing = Slot == EOCInventorySlot::Secondary ? SecondaryWeapon : PrimaryWeapon;
    if (Existing && Existing != Weapon)
    {
        DropWeaponServer(Existing);
    }

    Weapon->EquipToCharacterServer(this);
    if (Slot == EOCInventorySlot::Secondary)
    {
        SecondaryWeapon = Weapon;
    }
    else
    {
        PrimaryWeapon = Weapon;
    }

    CurrentWeapon = Weapon;
    ActiveWeaponSlot = Slot;
    RefreshWeaponPresentation();
    ForceNetUpdate();
}

bool AOCCharacter::StartAIReviveServer(AOCCharacter* Target)
{
    if (!HasAuthority() || !Target || !CanReviveTargetServer(Target)) return false;
    if (ReviveTarget != Target) StartReviveServer(Target);
    return ReviveTarget == Target;
}

void AOCCharacter::CancelAIReviveServer()
{
    if (HasAuthority()) CancelReviveServer();
}

int32 AOCCharacter::AddAmmoFromBoxServer(EOCAmmoType AmmoType, int32 Amount)
{
    if (!HasAuthority() || Amount <= 0)
    {
        return 0;
    }

    int32 Remaining = Amount;
    int32 GrantedTotal = 0;

    auto GrantToWeapon = [&](AOCWeaponBase* Weapon)
    {
        if (!Weapon || Remaining <= 0)
        {
            return;
        }
        if (AmmoType != EOCAmmoType::Any && Weapon->GetAmmoType() != AmmoType)
        {
            return;
        }

        const int32 Granted = Weapon->AddReserveAmmoServer(Remaining);
        GrantedTotal += Granted;
        Remaining -= Granted;
    };

    GrantToWeapon(CurrentWeapon);
    if (PrimaryWeapon != CurrentWeapon)
    {
        GrantToWeapon(PrimaryWeapon);
    }
    if (SecondaryWeapon != CurrentWeapon && SecondaryWeapon != PrimaryWeapon)
    {
        GrantToWeapon(SecondaryWeapon);
    }

    return GrantedTotal;
}

FString AOCCharacter::GetInteractionPrompt() const
{
    if (!IsLocallyControlled() || (HealthComponent && !HealthComponent->IsAlive()))
    {
        return FString();
    }

    if (bVehicleGunner && CurrentVehicle)
    {
        return CurrentVehicle->GetSpeedKmh() <= 14.0f ? TEXT("E  EXIT GUNNER") : TEXT("SLOW DOWN TO EXIT");
    }

    float DownedDistance = 0.0f;
    if (AOCCharacter* DownedCharacter = FindClosestDownedCharacter(DownedDistance))
    {
        FString PlayerName = TEXT("PLAYER");
        if (const AOCPlayerState* State = DownedCharacter->GetPlayerState<AOCPlayerState>())
        {
            PlayerName = State->GetPlayerName();
        }
        return FString::Printf(TEXT("HOLD E  REVIVE  %s"), *PlayerName);
    }

    float WorldInteractDistance = 0.0f;
    if (AOCInteractableActor* Interactable = FindFocusedWorldInteractable(WorldInteractDistance))
    {
        return Interactable->GetInteractionPrompt(this);
    }

    float VehicleDistance = 0.0f;
    if (AOCVehicleBase* Vehicle = FindFocusedVehicle(VehicleDistance))
    {
        if (const AOCPlayerState* SelfState = GetPlayerState<AOCPlayerState>(); SelfState && SelfState->IsEngineer() &&
            !Vehicle->IsVehicleDestroyed() && Vehicle->GetVehicleHealthNormalized() < 0.999f)
        {
            return TEXT("E  REPAIR VEHICLE");
        }
        return Vehicle->GetSeatPrompt(this);
    }

    float WeaponDistance = 0.0f;
    float AmmoDistance = 0.0f;
    AOCWeaponBase* Weapon = FindClosestWorldWeapon(WeaponDistance);
    AOCAmmoBox* AmmoBox = FindClosestAmmoBox(AmmoDistance);

    if (Weapon && (!AmmoBox || WeaponDistance <= AmmoDistance))
    {
        return FString::Printf(TEXT("E  PICK UP  %s"), *Weapon->GetWeaponDisplayName());
    }
    if (AmmoBox)
    {
        return AmmoBox->GetPromptText();
    }
    return FString();
}

bool AOCCharacter::IsDowned() const
{
    return HealthComponent && HealthComponent->IsDowned();
}

float AOCCharacter::GetDownedTimeRemaining() const
{
    return HealthComponent ? HealthComponent->GetDownedTimeRemaining() : 0.0f;
}

double AOCCharacter::GetSynchronizedServerTime() const
{
    if (const UWorld* World = GetWorld())
    {
        if (const AGameStateBase* GameState = World->GetGameState<AGameStateBase>())
        {
            return GameState->GetServerWorldTimeSeconds();
        }
        return World->GetTimeSeconds();
    }
    return 0.0;
}

float AOCCharacter::GetGiveUpProgress() const
{
    if (!IsDowned() || GiveUpEndServerTime <= 0.0 || GiveUpHoldSeconds <= 0.0f)
    {
        return 0.0f;
    }

    const float Remaining = FMath::Max(0.0f, static_cast<float>(GiveUpEndServerTime - GetSynchronizedServerTime()));
    return 1.0f - FMath::Clamp(Remaining / GiveUpHoldSeconds, 0.0f, 1.0f);
}

float AOCCharacter::GetReviveProgress() const
{
    if (!ReviveTarget || ReviveEndServerTime <= 0.0 || ReviveHoldSeconds <= 0.0f)
    {
        return 0.0f;
    }

    const float Remaining = FMath::Max(0.0f, static_cast<float>(ReviveEndServerTime - GetSynchronizedServerTime()));
    return 1.0f - FMath::Clamp(Remaining / ReviveHoldSeconds, 0.0f, 1.0f);
}

FString AOCCharacter::GetReviveTargetName() const
{
    if (!ReviveTarget)
    {
        return FString();
    }

    if (const AOCPlayerState* State = ReviveTarget->GetPlayerState<AOCPlayerState>())
    {
        return State->GetPlayerName();
    }
    return TEXT("PLAYER");
}

void AOCCharacter::ServerSetGiveUpHeld_Implementation(bool bHeld)
{
    if (bHeld)
    {
        StartGiveUpServer();
    }
    else
    {
        CancelGiveUpServer();
    }
}

void AOCCharacter::StartGiveUpServer()
{
    if (!HasAuthority() || !IsDowned() || GiveUpEndServerTime > 0.0)
    {
        return;
    }

    GiveUpEndServerTime = GetSynchronizedServerTime() + static_cast<double>(GiveUpHoldSeconds);
    GetWorldTimerManager().SetTimer(GiveUpTimerHandle, this, &AOCCharacter::CompleteGiveUpServer, GiveUpHoldSeconds, false);
    ForceNetUpdate();
}

void AOCCharacter::CancelGiveUpServer()
{
    if (!HasAuthority())
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(GiveUpTimerHandle);
    GiveUpEndServerTime = 0.0;
    ForceNetUpdate();
}

void AOCCharacter::CompleteGiveUpServer()
{
    if (!HasAuthority())
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(GiveUpTimerHandle);
    GiveUpEndServerTime = 0.0;
    if (HealthComponent && HealthComponent->IsDowned())
    {
        HealthComponent->GiveUpServer();
    }
    ForceNetUpdate();
}

void AOCCharacter::ApplyLifeStatePresentation()
{
    const bool bDowned = HealthComponent && HealthComponent->IsDowned();

    bIsSprinting = false;
    if (bDowned)
    {
        bIsAiming = false;
        Crouch();
    }
    else if (!HealthComponent || HealthComponent->IsAlive())
    {
        UnCrouch();
    }

    if (FirstPersonCamera)
    {
        FVector CameraLocation = StandingCameraRelativeLocation;
        if (bDowned)
        {
            CameraLocation.Z = DownedCameraZ;
        }
        FirstPersonCamera->SetRelativeLocation(CameraLocation);
    }

    ApplyMovementSpeed();
    RefreshWeaponPresentation();
}

void AOCCharacter::HandleDowned()
{
    bIsAiming = false;
    bIsSprinting = false;

    if (HasAuthority())
    {
        StopServerFireTimer();
        CancelReviveServer();
        if (CurrentWeapon)
        {
            CurrentWeapon->CancelReloadServer();
            CurrentWeapon->StoreInInventoryServer(this);
        }
    }

    if (HasAuthority() && CharacterVisualComponent) CharacterVisualComponent->BroadcastActionServer(EOCCharacterActionEvent::Downed);
    ApplyLifeStatePresentation();
}

void AOCCharacter::HandleRevived()
{
    if (HasAuthority())
    {
        CancelGiveUpServer();
        if (!CurrentWeapon)
        {
            CurrentWeapon = PrimaryWeapon ? PrimaryWeapon : SecondaryWeapon;
            ActiveWeaponSlot = PrimaryWeapon ? EOCInventorySlot::Primary
                : (SecondaryWeapon ? EOCInventorySlot::Secondary : EOCInventorySlot::None);
        }
        if (CurrentWeapon)
        {
            CurrentWeapon->EquipToCharacterServer(this);
        }
        ForceNetUpdate();
    }

    if (HasAuthority() && CharacterVisualComponent) CharacterVisualComponent->BroadcastActionServer(EOCCharacterActionEvent::Revived);
    ApplyLifeStatePresentation();
}

void AOCCharacter::HandleDeath()
{
    bIsAiming = false;
    bIsSprinting = false;

    // Presentation-side corpse state must be applied on every machine; capsule collision flags are not a gameplay RPC.
    if (GetCharacterMovement()) GetCharacterMovement()->DisableMovement();
    if (GetCapsuleComponent()) GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ApplyMovementSpeed();
    RefreshWeaponPresentation();

    if (HasAuthority() && CharacterVisualComponent) CharacterVisualComponent->BroadcastActionServer(EOCCharacterActionEvent::Death);

    if (!HasAuthority())
    {
        return;
    }

    StopServerFireTimer();
    CancelReviveServer();
    CancelGiveUpServer();
    if (CurrentWeapon)
    {
        CurrentWeapon->CancelReloadServer();
        DropWeaponServer(CurrentWeapon);
    }

    if (PrimaryWeapon)
    {
        PrimaryWeapon->Destroy();
        PrimaryWeapon = nullptr;
    }
    if (SecondaryWeapon)
    {
        SecondaryWeapon->Destroy();
        SecondaryWeapon = nullptr;
    }
    CurrentWeapon = nullptr;
    ActiveWeaponSlot = EOCInventorySlot::None;

    if (CombatVisualComponent)
    {
        CombatVisualComponent->HandleDeathServer();
    }

    if (AOCGameMode* GameMode = GetWorld()->GetAuthGameMode<AOCGameMode>())
    {
        AController* KillerController = HealthComponent ? HealthComponent->GetLastDamageInstigator() : nullptr;
        GameMode->HandleCharacterDeath(this, KillerController);
    }
}
