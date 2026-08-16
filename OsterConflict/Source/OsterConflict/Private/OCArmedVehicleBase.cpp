#include "OCArmedVehicleBase.h"

#include "OCCharacter.h"
#include "OCCombatVisualComponent.h"
#include "OCHealthComponent.h"
#include "OCDamageTypes.h"
#include "OCGameMode.h"
#include "OCPlayerState.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AOCArmedVehicleBase::AOCArmedVehicleBase()
{
    TurretPivot = CreateDefaultSubobject<USceneComponent>(TEXT("TurretPivot"));
    TurretPivot->SetupAttachment(PhysicsBody.Get());
    TurretPivot->SetRelativeLocation(FVector(-25.0f, 0.0f, 92.0f));

    TurretBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretBaseMesh"));
    TurretBaseMesh->SetupAttachment(TurretPivot);
    TurretBaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    BarrelPivot = CreateDefaultSubobject<USceneComponent>(TEXT("BarrelPivot"));
    BarrelPivot->SetupAttachment(TurretPivot);
    BarrelPivot->SetRelativeLocation(FVector(15.0f, 0.0f, 16.0f));

    BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
    BarrelMesh->SetupAttachment(BarrelPivot);
    BarrelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
    MuzzlePoint->SetupAttachment(BarrelPivot);
    MuzzlePoint->SetRelativeLocation(FVector(190.0f, 0.0f, 0.0f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CylinderMesh.Succeeded())
    {
        TurretBaseMesh->SetStaticMesh(CylinderMesh.Object);
        TurretBaseMesh->SetRelativeScale3D(FVector(0.70f, 0.70f, 0.20f));
    }
    if (CubeMesh.Succeeded())
    {
        BarrelMesh->SetStaticMesh(CubeMesh.Object);
        BarrelMesh->SetRelativeLocation(FVector(90.0f, 0.0f, 0.0f));
        BarrelMesh->SetRelativeScale3D(FVector(1.80f, 0.08f, 0.08f));
    }

    TurretDamageTypeClass = UOCBallisticDamageType::StaticClass();

    // R13 solo-driver fallback. Dedicated gunner remains the preferred multiplayer seat, but one human can now
    // test the mounted weapon without needing a second client just to hold the driver's seat.
    DriverTurretMappingContext = CreateDefaultSubobject<UInputMappingContext>(TEXT("IMC_DriverTurretRuntime"));
    DriverTurretAimAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_DriverTurretAim"));
    DriverTurretFireAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_DriverTurretFire"));
    DriverTurretReloadAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_DriverTurretReload"));
    DriverTurretLookXAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_DriverTurretLookX"));
    DriverTurretLookYAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_DriverTurretLookY"));
    DriverTurretLookXAction->ValueType = EInputActionValueType::Axis1D;
    DriverTurretLookYAction->ValueType = EInputActionValueType::Axis1D;

    if (DriverTurretMappingContext)
    {
        DriverTurretMappingContext->MapKey(DriverTurretAimAction, EKeys::RightMouseButton);
        DriverTurretMappingContext->MapKey(DriverTurretFireAction, EKeys::LeftMouseButton);
        DriverTurretMappingContext->MapKey(DriverTurretReloadAction, EKeys::R);
        DriverTurretMappingContext->MapKey(DriverTurretLookXAction, EKeys::MouseX);
        DriverTurretMappingContext->MapKey(DriverTurretLookYAction, EKeys::MouseY);
    }
}

void AOCArmedVehicleBase::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
    {
        TurretAmmoInMagazine = TurretMagazineSize;
        TurretReserveAmmo = StartingTurretReserveAmmo;
        ForceNetUpdate();
    }
    ApplyArmedVehicleStyle();
    ApplyTurretPresentation();
}

void AOCArmedVehicleBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    ApplyTurretPresentation();

    if (HasAuthority() && !HasDriver() && !GunnerCharacter && OccupantTeam != EOCTeam::None)
    {
        OccupantTeam = EOCTeam::None;
        ForceNetUpdate();
    }

    AOCCharacter* Operator = GetActiveTurretOperator();
    if (HasAuthority() && bGunnerFireHeld && CanGunnerOperateServer(Operator) && !bTurretReloading)
    {
        if (!GetWorldTimerManager().IsTimerActive(TurretFireTimerHandle))
        {
            BeginTurretFireServer();
        }
    }
}

void AOCArmedVehicleBase::PawnClientRestart()
{
    Super::PawnClientRestart();
    ConfigureDriverTurretInput();
}

void AOCArmedVehicleBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    UEnhancedInputComponent* Enhanced = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!Enhanced) return;

    Enhanced->BindAction(DriverTurretAimAction, ETriggerEvent::Started, this, &AOCArmedVehicleBase::DriverTurretAimPressed);
    Enhanced->BindAction(DriverTurretAimAction, ETriggerEvent::Completed, this, &AOCArmedVehicleBase::DriverTurretAimReleased);
    Enhanced->BindAction(DriverTurretAimAction, ETriggerEvent::Canceled, this, &AOCArmedVehicleBase::DriverTurretAimReleased);
    Enhanced->BindAction(DriverTurretFireAction, ETriggerEvent::Started, this, &AOCArmedVehicleBase::DriverTurretFirePressed);
    Enhanced->BindAction(DriverTurretFireAction, ETriggerEvent::Completed, this, &AOCArmedVehicleBase::DriverTurretFireReleased);
    Enhanced->BindAction(DriverTurretFireAction, ETriggerEvent::Canceled, this, &AOCArmedVehicleBase::DriverTurretFireReleased);
    Enhanced->BindAction(DriverTurretReloadAction, ETriggerEvent::Started, this, &AOCArmedVehicleBase::DriverTurretReloadPressed);
    Enhanced->BindAction(DriverTurretLookXAction, ETriggerEvent::Triggered, this, &AOCArmedVehicleBase::DriverTurretLookX);
    Enhanced->BindAction(DriverTurretLookYAction, ETriggerEvent::Triggered, this, &AOCArmedVehicleBase::DriverTurretLookY);
}

void AOCArmedVehicleBase::ConfigureDriverTurretInput()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC || !PC->IsLocalController() || !DriverTurretMappingContext) return;

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
    {
        Subsystem->RemoveMappingContext(DriverTurretMappingContext);
        Subsystem->AddMappingContext(DriverTurretMappingContext, 30);
    }
}

void AOCArmedVehicleBase::DriverTurretAimPressed()
{
    if (!CanDriverUseTurret()) return;
    bDriverTurretAimHeld = true;
    LocalDriverTurretYaw = TurretYaw;
    LocalDriverTurretPitch = TurretPitch;
}

void AOCArmedVehicleBase::DriverTurretAimReleased()
{
    bDriverTurretAimHeld = false;
}

void AOCArmedVehicleBase::DriverTurretFirePressed()
{
    if (!CanDriverUseTurret()) return;
    if (HasAuthority()) ServerSetDriverTurretFireHeld_Implementation(true);
    else ServerSetDriverTurretFireHeld(true);
}

void AOCArmedVehicleBase::DriverTurretFireReleased()
{
    if (HasAuthority()) ServerSetDriverTurretFireHeld_Implementation(false);
    else ServerSetDriverTurretFireHeld(false);
}

void AOCArmedVehicleBase::DriverTurretReloadPressed()
{
    if (!CanDriverUseTurret()) return;
    if (HasAuthority()) ServerReloadDriverTurret_Implementation();
    else ServerReloadDriverTurret();
}

void AOCArmedVehicleBase::DriverTurretLookX(const FInputActionValue& Value)
{
    if (!bDriverTurretAimHeld || !CanDriverUseTurret()) return;
    LocalDriverTurretYaw = FMath::Clamp(LocalDriverTurretYaw + Value.Get<float>() * 1.35f, -MaxTurretYaw, MaxTurretYaw);
    if (HasAuthority()) ServerSetDriverTurretAim_Implementation(LocalDriverTurretYaw, LocalDriverTurretPitch);
    else ServerSetDriverTurretAim(LocalDriverTurretYaw, LocalDriverTurretPitch);
}

void AOCArmedVehicleBase::DriverTurretLookY(const FInputActionValue& Value)
{
    if (!bDriverTurretAimHeld || !CanDriverUseTurret()) return;
    LocalDriverTurretPitch = FMath::Clamp(LocalDriverTurretPitch - Value.Get<float>() * 1.15f, MinTurretPitch, MaxTurretPitch);
    if (HasAuthority()) ServerSetDriverTurretAim_Implementation(LocalDriverTurretYaw, LocalDriverTurretPitch);
    else ServerSetDriverTurretAim(LocalDriverTurretYaw, LocalDriverTurretPitch);
}

void AOCArmedVehicleBase::ServerSetDriverTurretAim_Implementation(float RelativeYaw, float RelativePitch)
{
    if (AOCCharacter* Driver = GetDriverCharacter())
    {
        SetGunnerAimServer(Driver, RelativeYaw, RelativePitch);
    }
}

void AOCArmedVehicleBase::ServerSetDriverTurretFireHeld_Implementation(bool bHeld)
{
    if (AOCCharacter* Driver = GetDriverCharacter())
    {
        SetGunnerFireHeldServer(Driver, bHeld);
    }
    else if (!bHeld)
    {
        StopTurretFireServer();
    }
}

void AOCArmedVehicleBase::ServerReloadDriverTurret_Implementation()
{
    if (AOCCharacter* Driver = GetDriverCharacter())
    {
        RequestGunnerReloadServer(Driver);
    }
}

bool AOCArmedVehicleBase::TryEnterVehicleServer(AOCCharacter* Character)
{
    if (!HasAuthority() || !Character || IsVehicleDestroyed())
    {
        return false;
    }

    const EOCTeam CharacterTeam = ResolveCharacterTeam(Character);

    if (!HasDriver())
    {
        if (GunnerCharacter && OccupantTeam != EOCTeam::None && CharacterTeam != OccupantTeam)
        {
            return false;
        }
        const bool bEnteredDriver = Super::TryEnterVehicleServer(Character);
        if (bEnteredDriver)
        {
            OccupantTeam = CharacterTeam;
            ForceNetUpdate();
        }
        return bEnteredDriver;
    }

    if (GunnerCharacter || Character == GetDriverCharacter() || Character->IsInVehicle())
    {
        return false;
    }

    if (!Character->GetHealthComponent() || !Character->GetHealthComponent()->IsAlive())
    {
        return false;
    }
    if (OccupantTeam != EOCTeam::None && CharacterTeam != OccupantTeam)
    {
        return false;
    }

    if (FVector::DistSquared(Character->GetActorLocation(), GetActorLocation()) > FMath::Square(GunnerEnterDistanceCm))
    {
        return false;
    }

    // Dedicated gunner takes priority over the solo-driver fallback immediately.
    StopTurretFireServer();
    GunnerCharacter = Character;
    if (OccupantTeam == EOCTeam::None) OccupantTeam = CharacterTeam;
    Character->EnterVehicleGunnerServer(this, GetGunnerCameraWorldLocation());
    TurretYaw = 0.0f;
    TurretPitch = 0.0f;
    ForceNetUpdate();
    return true;
}

FString AOCArmedVehicleBase::GetSeatPrompt(const AOCCharacter* Character) const
{
    if (IsVehicleDestroyed()) return TEXT("ТЕХНІКА ЗНИЩЕНА");
    const EOCTeam ViewerTeam = ResolveCharacterTeam(Character);
    if (OccupantTeam != EOCTeam::None && ViewerTeam != EOCTeam::None && ViewerTeam != OccupantTeam)
    {
        return TEXT("ВОРОЖА ТЕХНІКА ЗАЙНЯТА");
    }
    if (!HasDriver()) return TEXT("E  СІСТИ ЗА КЕРМО");
    if (!GunnerCharacter) return TEXT("E  СІСТИ ЗА КУЛЕМЕТ");

    FString DriverName = GetDriverDisplayName();
    FString GunnerName = TEXT("СТРІЛЕЦЬ");
    if (const AOCPlayerState* PS = GunnerCharacter->GetPlayerState<AOCPlayerState>())
    {
        GunnerName = PS->GetPlayerName();
    }
    return FString::Printf(TEXT("ЗАЙНЯТО  ВОДІЙ:%s  СТРІЛЕЦЬ:%s"), *DriverName, *GunnerName);
}

EOCTeam AOCArmedVehicleBase::ResolveCharacterTeam(const AOCCharacter* Character) const
{
    if (!Character) return EOCTeam::None;
    if (const AOCPlayerState* PS = Character->GetPlayerState<AOCPlayerState>()) return PS->GetTeamId();
    return EOCTeam::None;
}

AOCCharacter* AOCArmedVehicleBase::GetActiveTurretOperator() const
{
    if (GunnerCharacter) return GunnerCharacter.Get();
    return GetDriverCharacter();
}

bool AOCArmedVehicleBase::CanGunnerOperateServer(const AOCCharacter* Requester) const
{
    if (!HasAuthority() || !Requester || !HasDriver() || IsVehicleDestroyed()) return false;

    const bool bDedicatedGunner = Requester == GunnerCharacter;
    const bool bSoloDriver = !GunnerCharacter && Requester == GetDriverCharacter();
    if (!bDedicatedGunner && !bSoloDriver) return false;

    return Requester->GetHealthComponent() && Requester->GetHealthComponent()->IsAlive();
}

void AOCArmedVehicleBase::SetGunnerAimServer(AOCCharacter* Requester, float RelativeYaw, float RelativePitch)
{
    if (!CanGunnerOperateServer(Requester)) return;
    TurretYaw = FMath::Clamp(RelativeYaw, -MaxTurretYaw, MaxTurretYaw);
    TurretPitch = FMath::Clamp(RelativePitch, MinTurretPitch, MaxTurretPitch);
    ApplyTurretPresentation();
    ForceNetUpdate();
}

void AOCArmedVehicleBase::SetGunnerFireHeldServer(AOCCharacter* Requester, bool bHeld)
{
    if (!CanGunnerOperateServer(Requester))
    {
        if (!bHeld) StopTurretFireServer();
        return;
    }
    bGunnerFireHeld = bHeld;
    if (bHeld) BeginTurretFireServer();
    else StopTurretFireServer();
}

void AOCArmedVehicleBase::BeginTurretFireServer()
{
    AOCCharacter* Operator = GetActiveTurretOperator();
    if (!HasAuthority() || !bGunnerFireHeld || bTurretReloading || TurretAmmoInMagazine <= 0 ||
        !CanGunnerOperateServer(Operator))
    {
        return;
    }

    FireTurretShotServer();
}

void AOCArmedVehicleBase::StopTurretFireServer()
{
    bGunnerFireHeld = false;
    GetWorldTimerManager().ClearTimer(TurretFireTimerHandle);
}

void AOCArmedVehicleBase::FireTurretShotServer()
{
    AOCCharacter* Operator = GetActiveTurretOperator();
    if (!HasAuthority() || !bGunnerFireHeld || bTurretReloading || TurretAmmoInMagazine <= 0 ||
        !CanGunnerOperateServer(Operator))
    {
        StopTurretFireServer();
        return;
    }

    --TurretAmmoInMagazine;
    const FVector Origin = MuzzlePoint ? MuzzlePoint->GetComponentLocation() : GetActorLocation();
    const FRotator ShotRotation(TurretPitch, GetActorRotation().Yaw + TurretYaw, 0.0f);
    const float Cone = FMath::DegreesToRadians(FMath::Max(0.0f, TurretSpreadDegrees));
    const FVector Direction = Cone > KINDA_SMALL_NUMBER ? FMath::VRandCone(ShotRotation.Vector(), Cone) : ShotRotation.Vector();
    const FVector End = Origin + Direction * TurretRangeCm;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(OCVehicleTurretTrace), true, this);
    Params.AddIgnoredActor(this);
    if (GunnerCharacter) Params.AddIgnoredActor(GunnerCharacter);
    if (GetDriverCharacter()) Params.AddIgnoredActor(GetDriverCharacter());

    FHitResult Hit;
    if (GetWorld()->LineTraceSingleByChannel(Hit, Origin, End, ECC_Visibility, Params) && Hit.GetActor())
    {
        const AOCGameMode* GM = GetWorld()->GetAuthGameMode<AOCGameMode>();
        AController* InstigatorController = nullptr;
        if (Operator == GetDriverCharacter()) InstigatorController = Controller;
        else if (Operator) InstigatorController = Operator->GetController();

        if (!GM || GM->CanDealDamage(InstigatorController, Hit.GetActor()))
        {
            TSubclassOf<UDamageType> AppliedDamageType = TurretDamageTypeClass;
            if (!AppliedDamageType)
            {
                AppliedDamageType = UOCBallisticDamageType::StaticClass();
            }
            const float AppliedDamage = UGameplayStatics::ApplyPointDamage(Hit.GetActor(), TurretDamage, Direction, Hit,
                InstigatorController, this, AppliedDamageType);
            if (AppliedDamage > 0.0f)
            {
                if (AOCCharacter* TargetCharacter = Cast<AOCCharacter>(Hit.GetActor()))
                {
                    const UOCHealthComponent* TargetHealth = TargetCharacter->GetHealthComponent();
                    if (UOCCombatVisualComponent* Trauma = TargetCharacter->GetCombatVisualComponent())
                    {
                        Trauma->RecordPointTraumaServer(AppliedDamage, Hit.ImpactPoint, Direction, Hit.BoneName,
                            EOCWeaponClass::LMG, AppliedDamageType, TargetHealth && TargetHealth->IsDead());
                    }
                }
            }
        }
    }

    ForceNetUpdate();
    if (TurretAmmoInMagazine <= 0)
    {
        StopTurretFireServer();
        return;
    }

    const float Interval = 60.0f / FMath::Max(1.0f, TurretRoundsPerMinute);
    GetWorldTimerManager().SetTimer(TurretFireTimerHandle, this, &AOCArmedVehicleBase::FireTurretShotServer, Interval, false);
}

void AOCArmedVehicleBase::RequestGunnerReloadServer(AOCCharacter* Requester)
{
    if (!CanGunnerOperateServer(Requester) || bTurretReloading || TurretAmmoInMagazine >= TurretMagazineSize || TurretReserveAmmo <= 0)
    {
        return;
    }
    StopTurretFireServer();
    bTurretReloading = true;
    GetWorldTimerManager().SetTimer(TurretReloadTimerHandle, this, &AOCArmedVehicleBase::FinishTurretReloadServer,
        FMath::Max(0.1f, TurretReloadSeconds), false);
    ForceNetUpdate();
}

void AOCArmedVehicleBase::FinishTurretReloadServer()
{
    if (!HasAuthority() || IsVehicleDestroyed()) return;
    const int32 Needed = FMath::Max(0, TurretMagazineSize - TurretAmmoInMagazine);
    const int32 Loaded = FMath::Min(Needed, TurretReserveAmmo);
    TurretAmmoInMagazine += Loaded;
    TurretReserveAmmo -= Loaded;
    bTurretReloading = false;
    ForceNetUpdate();
}

bool AOCArmedVehicleBase::ExitGunnerServer(AOCCharacter* Requester, bool bForced)
{
    if (!HasAuthority() || !GunnerCharacter || Requester != GunnerCharacter)
    {
        return false;
    }
    if (!bForced && GetSpeedKmh() > 14.0f)
    {
        return false;
    }

    StopTurretFireServer();
    GetWorldTimerManager().ClearTimer(TurretReloadTimerHandle);
    bTurretReloading = false;

    AOCCharacter* Character = GunnerCharacter;
    GunnerCharacter = nullptr;
    if (!HasDriver()) OccupantTeam = EOCTeam::None;
    const FVector ExitLocation = FindSafeExitLocationForCharacter(Character, 1.0f, bForced);
    Character->ExitVehicleGunnerServer(ExitLocation, FRotator(0.0f, GetActorRotation().Yaw, 0.0f));
    ForceNetUpdate();
    return true;
}

void AOCArmedVehicleBase::ForceExitGunnerServer()
{
    if (HasAuthority() && GunnerCharacter)
    {
        ExitGunnerServer(GunnerCharacter, true);
    }
}

float AOCArmedVehicleBase::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (!CanHullAcceptDamage(DamageEvent))
    {
        return 0.0f;
    }
    return Super::TakeDamage(ModifyHullDamage(DamageAmount, DamageEvent), DamageEvent, EventInstigator, DamageCauser);
}

bool AOCArmedVehicleBase::CanHullAcceptDamage(const FDamageEvent&) const
{
    return true;
}

float AOCArmedVehicleBase::ModifyHullDamage(float DamageAmount, const FDamageEvent&) const
{
    return DamageAmount;
}

FVector AOCArmedVehicleBase::GetGunnerCameraWorldLocation() const
{
    const FVector Base = TurretPivot ? TurretPivot->GetComponentLocation() : GetActorLocation();
    return Base + GetActorUpVector() * 58.0f - GetActorForwardVector() * 25.0f;
}

void AOCArmedVehicleBase::OnRep_Gunner()
{
    if (GunnerCharacter)
    {
        bDriverTurretAimHeld = false;
    }
}

void AOCArmedVehicleBase::OnRep_TurretAim()
{
    ApplyTurretPresentation();
}

void AOCArmedVehicleBase::ApplyTurretPresentation()
{
    if (TurretPivot) TurretPivot->SetRelativeRotation(FRotator(0.0f, TurretYaw, 0.0f));
    if (BarrelPivot) BarrelPivot->SetRelativeRotation(FRotator(TurretPitch, 0.0f, 0.0f));
}

void AOCArmedVehicleBase::OnVehicleEnteredWreckServer()
{
    StopTurretFireServer();
    ForceExitGunnerServer();
}

void AOCArmedVehicleBase::ApplyArmedVehicleStyle()
{
}

void AOCArmedVehicleBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOCArmedVehicleBase, GunnerCharacter);
    DOREPLIFETIME(AOCArmedVehicleBase, OccupantTeam);
    DOREPLIFETIME(AOCArmedVehicleBase, TurretYaw);
    DOREPLIFETIME(AOCArmedVehicleBase, TurretPitch);
    DOREPLIFETIME(AOCArmedVehicleBase, TurretAmmoInMagazine);
    DOREPLIFETIME(AOCArmedVehicleBase, TurretReserveAmmo);
    DOREPLIFETIME(AOCArmedVehicleBase, bTurretReloading);
}
