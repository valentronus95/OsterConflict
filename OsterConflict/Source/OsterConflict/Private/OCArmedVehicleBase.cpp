#include "OCArmedVehicleBase.h"

#include "OCCharacter.h"
#include "OCCombatVisualComponent.h"
#include "OCHealthComponent.h"
#include "OCDamageTypes.h"
#include "OCGameMode.h"
#include "OCHealthComponent.h"
#include "OCPlayerState.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
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

    // PASS45 item 27: the gunner viewpoint belongs to the yaw hierarchy. This keeps the view
    // coupled to the real ring instead of leaving the hidden gunner pawn at a vehicle-local point.
    GunnerCameraPivot = CreateDefaultSubobject<USceneComponent>(TEXT("GunnerCameraPivot"));
    GunnerCameraPivot->SetupAttachment(TurretPivot);
    GunnerCameraPivot->SetRelativeLocation(FVector(-25.0f, 0.0f, 58.0f));

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

    // Gunner pawn is hidden while mounted, but its camera still originates from the pawn. Keep that
    // pawn pinned to the turret-owned camera pivot so yaw and vehicle motion cannot desynchronise view/weapon.
    if (HasAuthority() && GunnerCharacter && GunnerCameraPivot)
    {
        GunnerCharacter->SetActorLocation(
            GetGunnerCameraWorldLocation() - FVector(0.0f, 0.0f, 64.0f),
            false,
            nullptr,
            ETeleportType::TeleportPhysics);
    }

    if (HasAuthority() && !HasDriver() && !GunnerCharacter && OccupantTeam != EOCTeam::None)
    {
        OccupantTeam = EOCTeam::None;
        ForceNetUpdate();
    }

    if (HasAuthority() && bGunnerFireHeld && CanGunnerOperateServer(GunnerCharacter) && !bTurretReloading)
    {
        if (!GetWorldTimerManager().IsTimerActive(TurretFireTimerHandle))
        {
            BeginTurretFireServer();
        }
    }
}

bool AOCArmedVehicleBase::TryEnterVehicleServer(AOCCharacter* Character)
{
    if (!HasAuthority() || !Character || IsVehicleDestroyed())
    {
        return false;
    }

    const EOCTeam CharacterTeam = ResolveCharacterTeam(Character);
    if (!Character->GetHealthComponent() || !Character->GetHealthComponent()->IsAlive() || Character->IsInVehicle())
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

    // RUNTIME 2026-08-22: approaching an armed vehicle from the rear/turret side is an explicit gunner entry.
    // This allows a solo tester to operate the mounted gun without first filling the driver seat.
    const FVector LocalApproach = GetActorTransform().InverseTransformPosition(Character->GetActorLocation());
    const bool bApproachingTurretSide = LocalApproach.X < -25.0f;
    if (!GunnerCharacter && bApproachingTurretSide)
    {
        GunnerCharacter = Character;
        if (OccupantTeam == EOCTeam::None) OccupantTeam = CharacterTeam;
        Character->EnterVehicleGunnerServer(this, GetGunnerCameraWorldLocation());
        TurretYaw = 0.0f;
        TurretPitch = 0.0f;
        ForceNetUpdate();
        return true;
    }

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

    if (GunnerCharacter || Character == GetDriverCharacter())
    {
        return false;
    }

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
    if (IsVehicleDestroyed()) return TEXT("VEHICLE DESTROYED");
    const EOCTeam ViewerTeam = ResolveCharacterTeam(Character);
    if (OccupantTeam != EOCTeam::None && ViewerTeam != EOCTeam::None && ViewerTeam != OccupantTeam)
    {
        return TEXT("ENEMY VEHICLE OCCUPIED");
    }

    if (!HasDriver() && !GunnerCharacter && Character)
    {
        const FVector LocalApproach = GetActorTransform().InverseTransformPosition(Character->GetActorLocation());
        return LocalApproach.X < -25.0f ? TEXT("E  ENTER GUNNER") : TEXT("E  ENTER DRIVER");
    }
    if (!HasDriver()) return TEXT("E  ENTER DRIVER");
    if (!GunnerCharacter) return TEXT("E  ENTER GUNNER");

    FString DriverName = GetDriverDisplayName();
    FString GunnerName = TEXT("GUNNER");
    if (const AOCPlayerState* PS = GunnerCharacter->GetPlayerState<AOCPlayerState>())
    {
        GunnerName = PS->GetPlayerName();
    }
    return FString::Printf(TEXT("FULL  D:%s  G:%s"), *DriverName, *GunnerName);
}

EOCTeam AOCArmedVehicleBase::ResolveCharacterTeam(const AOCCharacter* Character) const
{
    if (!Character) return EOCTeam::None;
    if (const AOCPlayerState* PS = Character->GetPlayerState<AOCPlayerState>()) return PS->GetTeamId();
    return EOCTeam::None;
}

bool AOCArmedVehicleBase::CanGunnerOperateServer(const AOCCharacter* Requester) const
{
    return HasAuthority() && Requester && Requester == GunnerCharacter &&
        Requester->GetHealthComponent() && Requester->GetHealthComponent()->IsAlive() && !IsVehicleDestroyed();
}

void AOCArmedVehicleBase::SetGunnerAimServer(AOCCharacter* Requester, float RelativeYaw, float RelativePitch)
{
    if (!CanGunnerOperateServer(Requester)) return;
    TurretYaw = bContinuousTurretYaw
        ? FMath::UnwindDegrees(RelativeYaw)
        : FMath::Clamp(RelativeYaw, -MaxTurretYaw, MaxTurretYaw);
    TurretPitch = FMath::Clamp(RelativePitch, MinTurretPitch, MaxTurretPitch);
    ApplyTurretPresentation();
    ForceNetUpdate();
}

void AOCArmedVehicleBase::SetGunnerFireHeldServer(AOCCharacter* Requester, bool bHeld)
{
    if (!CanGunnerOperateServer(Requester)) return;
    bGunnerFireHeld = bHeld;
    if (bHeld) BeginTurretFireServer();
    else StopTurretFireServer();
}

void AOCArmedVehicleBase::BeginTurretFireServer()
{
    if (!HasAuthority() || !bGunnerFireHeld || bTurretReloading || TurretAmmoInMagazine <= 0 || !CanGunnerOperateServer(GunnerCharacter))
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
    if (!HasAuthority() || !bGunnerFireHeld || bTurretReloading || TurretAmmoInMagazine <= 0 || !CanGunnerOperateServer(GunnerCharacter))
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
        AController* InstigatorController = GunnerCharacter ? GunnerCharacter->GetController() : nullptr;
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
    const FVector VehicleLocationAtExit = GetActorLocation();
    const FVector ExitLocation = FindSafeExitLocationForCharacter(Character, 1.0f, bForced);
    Character->ExitVehicleGunnerServer(ExitLocation, FRotator(0.0f, GetActorRotation().Yaw, 0.0f));
    ForceNetUpdate();

    const FVector ResultingPawnLocation = Character->GetActorLocation();
    const float ExitErrorCm = FVector::Dist(ResultingPawnLocation, ExitLocation);
    const float PawnToVehicleCm = FVector::Dist(ResultingPawnLocation, VehicleLocationAtExit);
    if (ExitErrorCm <= 100.0f)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_GUNNER_EXIT_TRANSFORM_READY vehicle=%s vehicle_location=%s requested_exit=%s resulting_pawn=%s result_error_cm=%.1f pawn_to_vehicle_m=%.1f museum_respawn_path=0"),
            *GetName(),
            *VehicleLocationAtExit.ToCompactString(),
            *ExitLocation.ToCompactString(),
            *ResultingPawnLocation.ToCompactString(),
            ExitErrorCm,
            PawnToVehicleCm / 100.0f);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_GUNNER_EXIT_TRANSFORM_FAIL vehicle=%s vehicle_location=%s requested_exit=%s resulting_pawn=%s result_error_cm=%.1f museum_respawn_path=0"),
            *GetName(),
            *VehicleLocationAtExit.ToCompactString(),
            *ExitLocation.ToCompactString(),
            *ResultingPawnLocation.ToCompactString(),
            ExitErrorCm);
    }
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
    if (GunnerCameraPivot)
    {
        return GunnerCameraPivot->GetComponentLocation();
    }
    const FVector Base = TurretPivot ? TurretPivot->GetComponentLocation() : GetActorLocation();
    return Base + GetActorUpVector() * 58.0f - GetActorForwardVector() * 25.0f;
}

void AOCArmedVehicleBase::OnRep_Gunner()
{
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
