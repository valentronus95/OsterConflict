#include "OCVehicleBase.h"

#include "OCCharacter.h"
#include "OCHealthComponent.h"
#include "OCGameMode.h"
#include "OCPlayerState.h"
#include "OCVehicleAudioComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"


AOCVehicleBase::AOCVehicleBase()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);
    SetNetUpdateFrequency(60.0f);
    SetMinNetUpdateFrequency(30.0f);

    PhysicsBody = CreateDefaultSubobject<UBoxComponent>(TEXT("PhysicsBody"));
    SetRootComponent(PhysicsBody);
    PhysicsBody->SetBoxExtent(FVector(225.0f, 92.0f, 48.0f));
    PhysicsBody->SetCollisionProfileName(TEXT("PhysicsActor"));
    PhysicsBody->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    PhysicsBody->SetSimulatePhysics(true);
    PhysicsBody->SetEnableGravity(true);
    PhysicsBody->SetLinearDamping(0.22f);
    PhysicsBody->SetAngularDamping(2.4f);
    PhysicsBody->SetUseCCD(true, NAME_None);
    PhysicsBody->SetNotifyRigidBodyCollision(true);
    PhysicsBody->OnComponentHit.AddDynamic(this, &AOCVehicleBase::HandleChassisHit);

    Chassis = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Chassis"));
    Chassis->SetupAttachment(PhysicsBody);
    Chassis->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    InteriorRoot = CreateDefaultSubobject<USceneComponent>(TEXT("InteriorRoot"));
    InteriorRoot->SetupAttachment(PhysicsBody);

    Dashboard = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Dashboard"));
    Dashboard->SetupAttachment(InteriorRoot);
    Dashboard->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    SteeringWheel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SteeringWheel"));
    SteeringWheel->SetupAttachment(InteriorRoot);
    SteeringWheel->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    Windshield = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Windshield"));
    Windshield->SetupAttachment(InteriorRoot);
    Windshield->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    DriverDoor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DriverDoor"));
    DriverDoor->SetupAttachment(InteriorRoot);
    DriverDoor->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    PassengerDoor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PassengerDoor"));
    PassengerDoor->SetupAttachment(InteriorRoot);
    PassengerDoor->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    FrontBumper = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontBumper"));
    FrontBumper->SetupAttachment(InteriorRoot);
    FrontBumper->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    RearBumper = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RearBumper"));
    RearBumper->SetupAttachment(InteriorRoot);
    RearBumper->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    UStaticMeshComponent* WheelFL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelFL"));
    UStaticMeshComponent* WheelFR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelFR"));
    UStaticMeshComponent* WheelRL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelRL"));
    UStaticMeshComponent* WheelRR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelRR"));
    WheelVisuals.Add(WheelFL);
    WheelVisuals.Add(WheelFR);
    WheelVisuals.Add(WheelRL);
    WheelVisuals.Add(WheelRR);
    for (UStaticMeshComponent* Wheel : WheelVisuals)
    {
        Wheel->SetupAttachment(PhysicsBody);
        Wheel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    InteriorCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("InteriorCamera"));
    InteriorCamera->SetupAttachment(InteriorRoot);
    InteriorCamera->SetRelativeLocation(FVector(55.0f, -43.0f, 77.0f));
    InteriorCamera->SetFieldOfView(88.0f);

    ThirdPersonSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonSpringArm"));
    ThirdPersonSpringArm->SetupAttachment(PhysicsBody);
    ThirdPersonSpringArm->TargetArmLength = 560.0f;
    ThirdPersonSpringArm->SetRelativeLocation(FVector(-45.0f, 0.0f, 145.0f));
    ThirdPersonSpringArm->SetRelativeRotation(FRotator(-12.0f, 0.0f, 0.0f));
    ThirdPersonSpringArm->bEnableCameraLag = true;
    ThirdPersonSpringArm->CameraLagSpeed = 8.0f;
    ThirdPersonSpringArm->bDoCollisionTest = true;

    ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
    ThirdPersonCamera->SetupAttachment(ThirdPersonSpringArm, USpringArmComponent::SocketName);

    VehicleAudioComponent = CreateDefaultSubobject<UOCVehicleAudioComponent>(TEXT("VehicleAudioComponent"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CubeMesh.Succeeded())
    {
        Chassis->SetStaticMesh(CubeMesh.Object);
        Dashboard->SetStaticMesh(CubeMesh.Object);
        Windshield->SetStaticMesh(CubeMesh.Object);
        DriverDoor->SetStaticMesh(CubeMesh.Object);
        PassengerDoor->SetStaticMesh(CubeMesh.Object);
        FrontBumper->SetStaticMesh(CubeMesh.Object);
        RearBumper->SetStaticMesh(CubeMesh.Object);
    }
    if (CylinderMesh.Succeeded())
    {
        SteeringWheel->SetStaticMesh(CylinderMesh.Object);
        for (UStaticMeshComponent* Wheel : WheelVisuals)
        {
            Wheel->SetStaticMesh(CylinderMesh.Object);
        }
    }

    Chassis->SetRelativeScale3D(FVector(4.45f, 1.82f, 0.58f));
    Dashboard->SetRelativeLocation(FVector(72.0f, 0.0f, 54.0f));
    Dashboard->SetRelativeScale3D(FVector(0.75f, 1.45f, 0.17f));
    SteeringWheel->SetRelativeLocation(FVector(62.0f, -48.0f, 68.0f));
    SteeringWheel->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
    SteeringWheel->SetRelativeScale3D(FVector(0.30f, 0.30f, 0.07f));
    Windshield->SetRelativeLocation(FVector(82.0f, 0.0f, 88.0f));
    Windshield->SetRelativeRotation(FRotator(0.0f, 0.0f, -18.0f));
    Windshield->SetRelativeScale3D(FVector(0.05f, 1.50f, 0.58f));
    DriverDoor->SetRelativeLocation(FVector(15.0f, -94.0f, 30.0f));
    DriverDoor->SetRelativeScale3D(FVector(1.45f, 0.06f, 0.52f));
    PassengerDoor->SetRelativeLocation(FVector(15.0f, 94.0f, 30.0f));
    PassengerDoor->SetRelativeScale3D(FVector(1.45f, 0.06f, 0.52f));
    FrontBumper->SetRelativeLocation(FVector(228.0f, 0.0f, -24.0f));
    FrontBumper->SetRelativeScale3D(FVector(0.18f, 1.82f, 0.13f));
    RearBumper->SetRelativeLocation(FVector(-228.0f, 0.0f, -24.0f));
    RearBumper->SetRelativeScale3D(FVector(0.18f, 1.82f, 0.13f));

    const FVector WheelPositions[] =
    {
        FVector(142.0f, -92.0f, -55.0f), FVector(142.0f, 92.0f, -55.0f),
        FVector(-142.0f, -92.0f, -55.0f), FVector(-142.0f, 92.0f, -55.0f)
    };
    for (int32 Index = 0; Index < WheelVisuals.Num(); ++Index)
    {
        WheelVisuals[Index]->SetRelativeLocation(WheelPositions[Index]);
        WheelVisuals[Index]->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
        WheelVisuals[Index]->SetRelativeScale3D(FVector(0.62f, 0.62f, 0.24f));
        SuspensionPointsLocal.Add(WheelPositions[Index] + FVector(0.0f, 0.0f, 24.0f));
    }

    VehicleMappingContext = CreateDefaultSubobject<UInputMappingContext>(TEXT("IMC_VehicleRuntime"));
    DriveForwardAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_VehicleForward"));
    DriveReverseAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_VehicleReverse"));
    SteerLeftAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_VehicleLeft"));
    SteerRightAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_VehicleRight"));
    HandbrakeAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_VehicleHandbrake"));
    ExitAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_VehicleExit"));
    ToggleCameraAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_VehicleToggleCamera"));
    FreeLookAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_VehicleFreeLook"));
    LookXAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_VehicleLookX"));
    LookYAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_VehicleLookY"));

    DriveForwardAction->ValueType = EInputActionValueType::Axis1D;
    DriveReverseAction->ValueType = EInputActionValueType::Axis1D;
    SteerLeftAction->ValueType = EInputActionValueType::Axis1D;
    SteerRightAction->ValueType = EInputActionValueType::Axis1D;
    LookXAction->ValueType = EInputActionValueType::Axis1D;
    LookYAction->ValueType = EInputActionValueType::Axis1D;

    if (VehicleMappingContext)
    {
        VehicleMappingContext->MapKey(DriveForwardAction, EKeys::W);
        VehicleMappingContext->MapKey(DriveReverseAction, EKeys::S);
        VehicleMappingContext->MapKey(SteerLeftAction, EKeys::A);
        VehicleMappingContext->MapKey(SteerRightAction, EKeys::D);
        VehicleMappingContext->MapKey(HandbrakeAction, EKeys::SpaceBar);
        VehicleMappingContext->MapKey(ExitAction, EKeys::E);
        VehicleMappingContext->MapKey(ToggleCameraAction, EKeys::C);
        VehicleMappingContext->MapKey(FreeLookAction, EKeys::RightMouseButton);
        VehicleMappingContext->MapKey(LookXAction, EKeys::MouseX);
        VehicleMappingContext->MapKey(LookYAction, EKeys::MouseY);
    }
}

void AOCVehicleBase::BeginPlay()
{
    Super::BeginPlay();
    VehicleHealth = MaxVehicleHealth;
    if (PhysicsBody)
    {
        PhysicsBody->SetMassOverrideInKg(NAME_None, VehicleMassKg, true);
    }
    ApplyVehicleStyle();

    if (UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
    {
        const FString ClassName = GetClass()->GetName();
        const bool bMilitary = ClassName.Contains(TEXT("BTR")) || ClassName.Contains(TEXT("GunTruck")) ||
            ClassName.Contains(TEXT("ArmedVehicle"));

        const FLinearColor MilitaryBody(0.13f, 0.18f, 0.08f);
        const FLinearColor CivilianPalette[] =
        {
            FLinearColor(0.18f,0.23f,0.27f), FLinearColor(0.34f,0.06f,0.045f),
            FLinearColor(0.11f,0.19f,0.27f), FLinearColor(0.32f,0.30f,0.25f)
        };
        const int32 ColorIndex = FMath::Abs(FMath::FloorToInt(GetActorLocation().X / 1000.0f)) % UE_ARRAY_COUNT(CivilianPalette);
        const FLinearColor BodyColor = bMilitary ? MilitaryBody : CivilianPalette[ColorIndex];

        TArray<UStaticMeshComponent*> MeshComponents;
        GetComponents<UStaticMeshComponent>(MeshComponents);
        for (UStaticMeshComponent* Component : MeshComponents)
        {
            if (!Component) continue;

            UStaticMesh* Mesh = Component->GetStaticMesh();
            const FString AssetPath = Mesh ? Mesh->GetPathName() : FString();
            if (AssetPath.StartsWith(TEXT("/Game/Production/")))
            {
                // Pass45: authored production materials are authoritative. Legacy blockout tinting
                // may never repaint HMMWV/M2/BTR or any future production mesh after ApplyVehicleStyle().
                continue;
            }

            const FString Name = Component->GetName();
            FLinearColor Color = BodyColor;
            if (Name.Contains(TEXT("Wheel"))) Color = FLinearColor(0.025f,0.027f,0.026f);
            else if (Name.Contains(TEXT("Windshield"))) Color = FLinearColor(0.08f,0.18f,0.22f);
            else if (Name.Contains(TEXT("Dashboard")) || Name.Contains(TEXT("Steering"))) Color = FLinearColor(0.055f,0.06f,0.055f);
            else if (Name.Contains(TEXT("Bumper")) || Name.Contains(TEXT("Barrel")) || Name.Contains(TEXT("Turret"))) Color = FLinearColor(0.10f,0.11f,0.095f);
            else if (Name.Contains(TEXT("BedFloor"))) Color = FLinearColor(0.09f,0.11f,0.075f);

            if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, Component))
            {
                MID->SetVectorParameterValue(TEXT("Color"), Color);
                Component->SetMaterial(0, MID);
            }
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_VEHICLEBASE_PRODUCTION_MATERIAL_BYPASS_READY production_override=0 legacy_tint_blockout_only=1"));
    ApplyDamagePresentation();
}

void AOCVehicleBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (HasAuthority() && !bVehicleDestroyed)
    {
        SimulateVehicleServer(DeltaSeconds);
    }
    ApplyLocalCamera(DeltaSeconds);
    UpdateWheelVisuals(DeltaSeconds);

    if (SteeringWheel)
    {
        const FRotator Base(90.0f, 0.0f, 0.0f);
        SteeringWheel->SetRelativeRotation(Base + FRotator(0.0f, 0.0f, -SteeringInput * 115.0f));
    }
}

void AOCVehicleBase::PawnClientRestart()
{
    Super::PawnClientRestart();
    ConfigureVehicleInput();
    ApplyLocalCamera(0.0f);
}

void AOCVehicleBase::ConfigureVehicleInput()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC || !PC->IsLocalController() || !VehicleMappingContext)
    {
        return;
    }
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
    {
        Subsystem->RemoveMappingContext(VehicleMappingContext);
        Subsystem->AddMappingContext(VehicleMappingContext, 20);
    }
}

void AOCVehicleBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    UEnhancedInputComponent* Enhanced = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!Enhanced)
    {
        return;
    }

    Enhanced->BindAction(DriveForwardAction, ETriggerEvent::Triggered, this, &AOCVehicleBase::DriveForward);
    Enhanced->BindAction(DriveForwardAction, ETriggerEvent::Completed, this, &AOCVehicleBase::DriveForwardReleased);
    Enhanced->BindAction(DriveReverseAction, ETriggerEvent::Triggered, this, &AOCVehicleBase::DriveReverse);
    Enhanced->BindAction(DriveReverseAction, ETriggerEvent::Completed, this, &AOCVehicleBase::DriveReverseReleased);
    Enhanced->BindAction(SteerLeftAction, ETriggerEvent::Triggered, this, &AOCVehicleBase::SteerLeft);
    Enhanced->BindAction(SteerLeftAction, ETriggerEvent::Completed, this, &AOCVehicleBase::SteerLeftReleased);
    Enhanced->BindAction(SteerRightAction, ETriggerEvent::Triggered, this, &AOCVehicleBase::SteerRight);
    Enhanced->BindAction(SteerRightAction, ETriggerEvent::Completed, this, &AOCVehicleBase::SteerRightReleased);
    Enhanced->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &AOCVehicleBase::HandbrakePressed);
    Enhanced->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &AOCVehicleBase::HandbrakeReleased);
    Enhanced->BindAction(HandbrakeAction, ETriggerEvent::Canceled, this, &AOCVehicleBase::HandbrakeReleased);
    Enhanced->BindAction(ExitAction, ETriggerEvent::Started, this, &AOCVehicleBase::ExitPressed);
    Enhanced->BindAction(ToggleCameraAction, ETriggerEvent::Started, this, &AOCVehicleBase::ToggleCameraPressed);
    Enhanced->BindAction(FreeLookAction, ETriggerEvent::Started, this, &AOCVehicleBase::FreeLookPressed);
    Enhanced->BindAction(FreeLookAction, ETriggerEvent::Completed, this, &AOCVehicleBase::FreeLookReleased);
    Enhanced->BindAction(FreeLookAction, ETriggerEvent::Canceled, this, &AOCVehicleBase::FreeLookReleased);
    Enhanced->BindAction(LookXAction, ETriggerEvent::Triggered, this, &AOCVehicleBase::LookX);
    Enhanced->BindAction(LookYAction, ETriggerEvent::Triggered, this, &AOCVehicleBase::LookY);
}

void AOCVehicleBase::DriveForward(const FInputActionValue& Value) { LocalForward = FMath::Clamp(Value.Get<float>(), 0.0f, 1.0f); PushLocalDriveInputs(); }
void AOCVehicleBase::DriveForwardReleased(const FInputActionValue&) { LocalForward = 0.0f; PushLocalDriveInputs(); }
void AOCVehicleBase::DriveReverse(const FInputActionValue& Value) { LocalReverse = FMath::Clamp(Value.Get<float>(), 0.0f, 1.0f); PushLocalDriveInputs(); }
void AOCVehicleBase::DriveReverseReleased(const FInputActionValue&) { LocalReverse = 0.0f; PushLocalDriveInputs(); }
void AOCVehicleBase::SteerLeft(const FInputActionValue& Value) { LocalLeft = FMath::Clamp(Value.Get<float>(), 0.0f, 1.0f); PushLocalDriveInputs(); }
void AOCVehicleBase::SteerLeftReleased(const FInputActionValue&) { LocalLeft = 0.0f; PushLocalDriveInputs(); }
void AOCVehicleBase::SteerRight(const FInputActionValue& Value) { LocalRight = FMath::Clamp(Value.Get<float>(), 0.0f, 1.0f); PushLocalDriveInputs(); }
void AOCVehicleBase::SteerRightReleased(const FInputActionValue&) { LocalRight = 0.0f; PushLocalDriveInputs(); }
void AOCVehicleBase::HandbrakePressed() { bLocalHandbrake = true; PushLocalDriveInputs(); }
void AOCVehicleBase::HandbrakeReleased() { bLocalHandbrake = false; PushLocalDriveInputs(); }

void AOCVehicleBase::PushLocalDriveInputs()
{
    const float NewThrottle = FMath::Clamp(LocalForward - LocalReverse, -1.0f, 1.0f);
    const float NewSteering = FMath::Clamp(LocalRight - LocalLeft, -1.0f, 1.0f);
    if (HasAuthority())
    {
        ServerSetDriveInputs_Implementation(NewThrottle, NewSteering, bLocalHandbrake);
    }
    else
    {
        ServerSetDriveInputs(NewThrottle, NewSteering, bLocalHandbrake);
    }
}

void AOCVehicleBase::ServerSetDriveInputs_Implementation(float NewThrottle, float NewSteering, bool bNewHandbrake)
{
    if (!DriverCharacter || bVehicleDestroyed)
    {
        ThrottleInput = 0.0f;
        SteeringInput = 0.0f;
        bHandbrake = true;
        return;
    }
    ThrottleInput = FMath::Clamp(NewThrottle, -1.0f, 1.0f);
    SteeringInput = FMath::Clamp(NewSteering, -1.0f, 1.0f);
    bHandbrake = bNewHandbrake;
}

void AOCVehicleBase::ExitPressed()
{
    if (HasAuthority()) ServerRequestExit_Implementation();
    else ServerRequestExit();
}

void AOCVehicleBase::ServerRequestExit_Implementation()
{
    if (!DriverCharacter || bVehicleDestroyed || GetSpeedKmh() > MaxExitSpeedKmh)
    {
        return;
    }
    ExitDriverServer(false);
}

void AOCVehicleBase::ToggleCameraPressed()
{
    bFirstPersonCamera = !bFirstPersonCamera;
    ApplyLocalCamera(0.0f);
}

void AOCVehicleBase::FreeLookPressed() { bFreeLookHeld = true; }
void AOCVehicleBase::FreeLookReleased() { bFreeLookHeld = false; }

void AOCVehicleBase::LookX(const FInputActionValue& Value)
{
    if (bFreeLookHeld)
    {
        FreeLookYaw = FMath::Clamp(FreeLookYaw + Value.Get<float>() * 1.35f, -125.0f, 125.0f);
    }
}

void AOCVehicleBase::LookY(const FInputActionValue& Value)
{
    if (bFreeLookHeld)
    {
        FreeLookPitch = FMath::Clamp(FreeLookPitch - Value.Get<float>() * 1.15f, -48.0f, 38.0f);
    }
}

void AOCVehicleBase::ApplyLocalCamera(float DeltaSeconds)
{
    if (!IsLocallyControlled())
    {
        return;
    }

    if (!bFreeLookHeld && DeltaSeconds > 0.0f)
    {
        FreeLookYaw = FMath::FInterpTo(FreeLookYaw, 0.0f, DeltaSeconds, 5.0f);
        FreeLookPitch = FMath::FInterpTo(FreeLookPitch, 0.0f, DeltaSeconds, 5.0f);
    }

    InteriorCamera->SetActive(bFirstPersonCamera);
    ThirdPersonCamera->SetActive(!bFirstPersonCamera);
    InteriorCamera->SetRelativeRotation(FRotator(FreeLookPitch, FreeLookYaw, 0.0f));
    ThirdPersonSpringArm->SetRelativeRotation(FRotator(-12.0f + FreeLookPitch * 0.45f, FreeLookYaw, 0.0f));
}

void AOCVehicleBase::UpdateWheelVisuals(float DeltaSeconds)
{
    const float SpeedCms = PhysicsBody ? FVector::DotProduct(PhysicsBody->GetPhysicsLinearVelocity(NAME_None), GetActorForwardVector()) : 0.0f;
    WheelSpinDegrees = FMath::Fmod(WheelSpinDegrees + (SpeedCms / FMath::Max(WheelRadiusCm, 1.0f)) * DeltaSeconds * 57.29578f, 360.0f);
    for (int32 Index = 0; Index < WheelVisuals.Num(); ++Index)
    {
        if (!WheelVisuals[Index]) continue;
        const bool bFront = Index < 2;
        const float Yaw = bFront ? SteeringInput * 24.0f : 0.0f;
        WheelVisuals[Index]->SetRelativeRotation(FRotator(90.0f, Yaw, WheelSpinDegrees));
    }
}

void AOCVehicleBase::AddSuspensionPointLocal(const FVector& LocalPoint)
{
    SuspensionPointsLocal.Add(LocalPoint);
}

void AOCVehicleBase::ClearSuspensionPointsLocal()
{
    SuspensionPointsLocal.Reset();
}

void AOCVehicleBase::SimulateVehicleServer(float DeltaSeconds)
{
    if (!PhysicsBody || !PhysicsBody->IsSimulatingPhysics())
    {
        return;
    }
    const int32 ContactCount = ApplySuspensionServer(DeltaSeconds);
    ApplyDriveAndGripServer(DeltaSeconds, ContactCount);
}

int32 AOCVehicleBase::ApplySuspensionServer(float DeltaSeconds)
{
    if (!GetWorld() || !PhysicsBody)
    {
        return 0;
    }

    int32 ContactCount = 0;
    const FVector Up = GetActorUpVector();
    FCollisionQueryParams Params(SCENE_QUERY_STAT(OCVehicleSuspension), false, this);
    Params.AddIgnoredActor(this);

    for (const FVector& LocalPoint : SuspensionPointsLocal)
    {
        const FVector Start = PhysicsBody->GetComponentTransform().TransformPosition(LocalPoint);
        const FVector End = Start - Up * (SuspensionTraceLengthCm + WheelRadiusCm);
        FHitResult Hit;
        if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
        {
            continue;
        }

        ++ContactCount;
        const float DistanceToGround = Hit.Distance - WheelRadiusCm;
        const float Compression = FMath::Clamp(SuspensionTraceLengthCm - DistanceToGround, 0.0f, SuspensionTraceLengthCm);
        const FVector PointVelocity = PhysicsBody->GetPhysicsLinearVelocityAtPoint(Start, NAME_None);
        const float VerticalVelocity = FVector::DotProduct(PointVelocity, Up);
        const float SpringForce = FMath::Max(0.0f, Compression * SpringStiffness - VerticalVelocity * SuspensionDamping);
        PhysicsBody->AddForceAtLocation(Up * SpringForce, Start, NAME_None);
    }
    return ContactCount;
}

void AOCVehicleBase::ApplyDriveAndGripServer(float DeltaSeconds, int32 ContactCount)
{
    if (!PhysicsBody || ContactCount <= 0)
    {
        return;
    }

    const FVector Forward = GetActorForwardVector();
    const FVector Right = GetActorRightVector();
    const FVector Velocity = PhysicsBody->GetPhysicsLinearVelocity(NAME_None);
    const float ForwardSpeed = FVector::DotProduct(Velocity, Forward);
    const float LateralSpeed = FVector::DotProduct(Velocity, Right);
    const float SpeedKmh = Velocity.Size() * 0.036f;

    float DamagePowerScale = 1.0f;
    float DamageGripScale = 1.0f;
    switch (DamageStage)
    {
    case EOCVehicleDamageStage::Damaged:  DamagePowerScale = 0.92f; DamageGripScale = 0.94f; break;
    case EOCVehicleDamageStage::Heavy:    DamagePowerScale = 0.74f; DamageGripScale = 0.82f; break;
    case EOCVehicleDamageStage::Critical: DamagePowerScale = 0.48f; DamageGripScale = 0.66f; break;
    case EOCVehicleDamageStage::Wrecked:  DamagePowerScale = 0.0f;  DamageGripScale = 0.45f; break;
    default: break;
    }

    if (FMath::Abs(ThrottleInput) > KINDA_SMALL_NUMBER)
    {
        const bool bReverse = ThrottleInput < 0.0f;
        const float EffectiveMaxSpeedKmh = FMath::Max(1.0f, MaxForwardSpeedKmh * DamagePowerScale);
        const float SpeedLimitAlpha = FMath::Clamp(1.0f - SpeedKmh / EffectiveMaxSpeedKmh, 0.0f, 1.0f);
        const float ForceScale = bReverse ? ReverseForceScale : 1.0f;
        PhysicsBody->AddForce(Forward * ThrottleInput * DriveForce * DamagePowerScale * ForceScale * FMath::Max(0.12f, SpeedLimitAlpha), NAME_None, false);
    }
    else if (FMath::Abs(ForwardSpeed) > 15.0f)
    {
        PhysicsBody->AddForce(-Forward * FMath::Sign(ForwardSpeed) * RollingBrakeForce * 0.18f, NAME_None, false);
    }

    const float GripMultiplier = bHandbrake ? 0.22f : 1.0f;
    PhysicsBody->AddForce(-Right * LateralSpeed * LateralGrip * DamageGripScale * GripMultiplier, NAME_None, false);

    if (bHandbrake)
    {
        const FVector FlatVelocity(Velocity.X, Velocity.Y, 0.0f);
        if (!FlatVelocity.IsNearlyZero())
        {
            PhysicsBody->AddForce(-FlatVelocity.GetSafeNormal() * HandbrakeForce, NAME_None, false);
        }
    }

    // RUNTIME 2026-08-22: low-speed steering previously collapsed almost to zero because authority was
    // proportional only to current forward velocity. Give commanded steering a minimum authority, with
    // a stronger floor in reverse, then smoothly hand back to speed-based steering as velocity rises.
    float SteeringSpeedAlpha = FMath::Clamp(FMath::Abs(ForwardSpeed) / 650.0f, 0.0f, 1.0f);
    if (FMath::Abs(SteeringInput) > KINDA_SMALL_NUMBER && FMath::Abs(ThrottleInput) > 0.05f)
    {
        const float MinimumSteerAuthority = ThrottleInput < 0.0f ? 0.46f : 0.30f;
        SteeringSpeedAlpha = FMath::Max(SteeringSpeedAlpha, MinimumSteerAuthority);
    }
    if (SteeringSpeedAlpha > KINDA_SMALL_NUMBER)
    {
        const bool bMovingReverse = ForwardSpeed < -20.0f || (FMath::Abs(ForwardSpeed) < 20.0f && ThrottleInput < -0.05f);
        const float DirectionSign = bMovingReverse ? -1.0f : 1.0f;
        const float ReverseAuthorityBoost = bMovingReverse ? 1.15f : 1.0f;
        PhysicsBody->AddTorqueInRadians(GetActorUpVector() * SteeringInput * SteeringTorque * DamageGripScale *
            SteeringSpeedAlpha * DirectionSign * ReverseAuthorityBoost, NAME_None, false);
    }

    const float SpeedSq = Velocity.SizeSquared();
    if (SpeedSq > 100.0f)
    {
        PhysicsBody->AddForce(-Velocity.GetSafeNormal() * SpeedSq * AeroDrag, NAME_None, false);
    }
}

bool AOCVehicleBase::TryEnterVehicleServer(AOCCharacter* Character)
{
    if (!HasAuthority() || !Character || DriverCharacter || bVehicleDestroyed)
    {
        return false;
    }
    if (!Character->GetHealthComponent() || !Character->GetHealthComponent()->IsAlive() || Character->IsInVehicle())
    {
        return false;
    }
    if (FVector::DistSquared(Character->GetActorLocation(), GetActorLocation()) > FMath::Square(EnterDistanceCm))
    {
        return false;
    }

    AController* OccupantController = Character->GetController();
    if (!OccupantController)
    {
        return false;
    }

    DriverCharacter = Character;
    Character->EnterVehicleServer(this);
    SetOwner(OccupantController);
    OccupantController->Possess(this);
    ForceNetUpdate();
    return true;
}

void AOCVehicleBase::ForceExitDriverServer()
{
    if (HasAuthority() && DriverCharacter)
    {
        ExitDriverServer(true);
    }
}

FVector AOCVehicleBase::FindSafeExitLocationForCharacter(AOCCharacter* Character, float PreferredSideSign, bool bForced) const
{
    const FVector UpOffset(0.0f, 0.0f, 90.0f);
    const float Side = PreferredSideSign >= 0.0f ? 1.0f : -1.0f;
    const float RearExtra = bForced ? 110.0f : 70.0f;
    const FVector Candidates[] = {
        GetActorLocation() + GetActorRightVector() * (Side * 230.0f) + UpOffset,
        GetActorLocation() - GetActorRightVector() * (Side * 230.0f) + UpOffset,
        GetActorLocation() - GetActorForwardVector() * (250.0f + RearExtra) + UpOffset,
        GetActorLocation() + GetActorForwardVector() * 280.0f + UpOffset
    };

    const FRotator ExitRotation(0.0f, GetActorRotation().Yaw, 0.0f);
    if (Character && GetWorld())
    {
        const bool bHadCollision = Character->GetActorEnableCollision();
        Character->SetActorEnableCollision(true);
        for (const FVector& Candidate : Candidates)
        {
            FVector Adjusted = Candidate;
            if (GetWorld()->FindTeleportSpot(Character, Adjusted, ExitRotation))
            {
                Character->SetActorEnableCollision(bHadCollision);
                return Adjusted;
            }
        }
        Character->SetActorEnableCollision(bHadCollision);
    }
    return Candidates[0];
}

void AOCVehicleBase::ExitDriverServer(bool bForced)
{
    if (!HasAuthority() || !DriverCharacter)
    {
        return;
    }

    AController* OccupantController = Controller;
    AOCCharacter* Character = DriverCharacter;
    if (!OccupantController || !Character)
    {
        DriverCharacter = nullptr;
        return;
    }

    const FVector ExitLocation = FindSafeExitLocationForCharacter(Character, -1.0f, bForced);
    const FRotator ExitRotation(0.0f, GetActorRotation().Yaw, 0.0f);

    ThrottleInput = 0.0f;
    SteeringInput = 0.0f;
    bHandbrake = true;
    OccupantController->UnPossess();
    Character->ExitVehicleServer(ExitLocation, ExitRotation);
    DriverCharacter = nullptr;
    SetOwner(nullptr);
    OccupantController->Possess(Character);
    ForceNetUpdate();
}

void AOCVehicleBase::SetAIDriveInputsServer(float NewThrottle, float NewSteering, bool bNewHandbrake)
{
    if (!HasAuthority() || !DriverCharacter || bVehicleDestroyed) return;
    ThrottleInput = FMath::Clamp(NewThrottle, -1.0f, 1.0f);
    SteeringInput = FMath::Clamp(NewSteering, -1.0f, 1.0f);
    bHandbrake = bNewHandbrake;
}

void AOCVehicleBase::AIRequestExitServer()
{
    if (!HasAuthority() || !DriverCharacter || bVehicleDestroyed) return;
    if (GetSpeedKmh() <= MaxExitSpeedKmh + 4.0f) ExitDriverServer(false);
}

float AOCVehicleBase::RepairVehicleServer(float Amount, AOCCharacter* Engineer)
{
    if (!HasAuthority() || !Engineer || Amount <= 0.0f || bVehicleDestroyed || VehicleHealth >= MaxVehicleHealth) return 0.0f;
    if (FVector::DistSquared(Engineer->GetActorLocation(), GetActorLocation()) > FMath::Square(430.0f)) return 0.0f;
    const AOCPlayerState* State = Engineer->GetPlayerState<AOCPlayerState>();
    if (!State || !State->IsEngineer()) return 0.0f;
    const float Previous = VehicleHealth;
    VehicleHealth = FMath::Clamp(VehicleHealth + Amount, 0.0f, MaxVehicleHealth);
    UpdateDamageStageServer();
    ForceNetUpdate();
    return VehicleHealth - Previous;
}

float AOCVehicleBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (HasAuthority())
    {
        if (const AOCGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AOCGameMode>() : nullptr)
        {
            if (!GameMode->CanDealDamage(EventInstigator, this)) return 0.0f;
        }
    }
    const float BaseApplied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (!HasAuthority() || bVehicleDestroyed || DamageAmount <= 0.0f)
    {
        return 0.0f;
    }
    ApplyVehicleDamageServer(DamageAmount, EventInstigator, GetActorLocation());
    return BaseApplied > 0.0f ? BaseApplied : DamageAmount;
}

void AOCVehicleBase::HandleChassisHit(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!HasAuthority() || bVehicleDestroyed || !OtherActor || OtherActor == this)
    {
        return;
    }
    const float ImpactStrength = NormalImpulse.Size();
    if (VehicleAudioComponent && ImpactStrength > 85000.0f)
    {
        VehicleAudioComponent->PlayCollisionServer(ImpactStrength, Hit.ImpactPoint);
    }
    if (ImpactStrength > 260000.0f)
    {
        const float CollisionDamage = FMath::Clamp((ImpactStrength - 260000.0f) / 42000.0f, 2.0f, 48.0f);
        const float ScaledCollisionDamage = CollisionDamage * FMath::Max(0.0f, GetCollisionDamageScale());
        if (ScaledCollisionDamage > 0.0f)
        {
            ApplyVehicleDamageServer(ScaledCollisionDamage, Controller, Hit.ImpactPoint);
        }
    }
}

void AOCVehicleBase::ApplyVehicleDamageServer(float Amount, AController*, const FVector&)
{
    if (!HasAuthority() || bVehicleDestroyed)
    {
        return;
    }
    VehicleHealth = FMath::Clamp(VehicleHealth - Amount, 0.0f, MaxVehicleHealth);
    UpdateDamageStageServer();
    if (VehicleHealth <= 0.0f)
    {
        EnterWreckStateServer();
    }
    ForceNetUpdate();
}

void AOCVehicleBase::UpdateDamageStageServer()
{
    const float Health01 = GetVehicleHealthNormalized();
    EOCVehicleDamageStage NewStage = EOCVehicleDamageStage::Intact;
    if (Health01 <= 0.0f) NewStage = EOCVehicleDamageStage::Wrecked;
    else if (Health01 <= 0.22f) NewStage = EOCVehicleDamageStage::Critical;
    else if (Health01 <= 0.48f) NewStage = EOCVehicleDamageStage::Heavy;
    else if (Health01 <= 0.74f) NewStage = EOCVehicleDamageStage::Damaged;
    DamageStage = NewStage;
    ApplyDamagePresentation();
}

void AOCVehicleBase::EnterWreckStateServer()
{
    if (!HasAuthority() || bVehicleDestroyed)
    {
        return;
    }
    bVehicleDestroyed = true;
    DamageStage = EOCVehicleDamageStage::Wrecked;
    ThrottleInput = 0.0f;
    SteeringInput = 0.0f;
    bHandbrake = true;
    if (DriverCharacter)
    {
        ExitDriverServer(true);
    }
    OnVehicleEnteredWreckServer();
    ApplyDamagePresentation();
    OnVehicleWrecked.Broadcast(this);
    GetWorldTimerManager().SetTimer(WreckDestroyTimerHandle, this, &AOCVehicleBase::DestroyWreck,
        WreckLifetimeSeconds, false);
    ForceNetUpdate();
}

void AOCVehicleBase::DestroyWreck()
{
    if (HasAuthority())
    {
        Destroy();
    }
}

void AOCVehicleBase::OnRep_Driver()
{
}

void AOCVehicleBase::OnRep_DamageState()
{
    ApplyDamagePresentation();
}

void AOCVehicleBase::ApplyDamagePresentation()
{
    const int32 Stage = static_cast<int32>(DamageStage);
    if (Windshield) Windshield->SetVisibility(Stage < 2, true);
    if (FrontBumper) FrontBumper->SetVisibility(Stage < 3, true);
    if (RearBumper) RearBumper->SetVisibility(Stage < 4, true);
    if (DriverDoor) DriverDoor->SetVisibility(Stage < 4, true);
    if (PassengerDoor) PassengerDoor->SetVisibility(Stage < 4, true);
}

float AOCVehicleBase::GetSpeedKmh() const
{
    return PhysicsBody ? PhysicsBody->GetPhysicsLinearVelocity(NAME_None).Size() * 0.036f : 0.0f;
}

float AOCVehicleBase::GetVehicleHealthNormalized() const
{
    return MaxVehicleHealth > 0.0f ? FMath::Clamp(VehicleHealth / MaxVehicleHealth, 0.0f, 1.0f) : 0.0f;
}

FString AOCVehicleBase::GetDriverDisplayName() const
{
    if (!DriverCharacter)
    {
        return TEXT("EMPTY");
    }
    if (const AOCPlayerState* State = DriverCharacter->GetPlayerState<AOCPlayerState>())
    {
        return State->GetPlayerName();
    }
    return TEXT("DRIVER");
}

FString AOCVehicleBase::GetSeatPrompt(const AOCCharacter* Character) const
{
    if (bVehicleDestroyed) return TEXT("VEHICLE DESTROYED");
    if (DriverCharacter) return FString::Printf(TEXT("DRIVER: %s"), *GetDriverDisplayName());
    if (!Character) return FString();
    return TEXT("E  ENTER VEHICLE");
}

void AOCVehicleBase::ApplyVehicleStyle()
{
}

void AOCVehicleBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOCVehicleBase, DriverCharacter);
    DOREPLIFETIME(AOCVehicleBase, ThrottleInput);
    DOREPLIFETIME(AOCVehicleBase, SteeringInput);
    DOREPLIFETIME(AOCVehicleBase, bHandbrake);
    DOREPLIFETIME(AOCVehicleBase, VehicleHealth);
    DOREPLIFETIME(AOCVehicleBase, DamageStage);
    DOREPLIFETIME(AOCVehicleBase, bVehicleDestroyed);
}
