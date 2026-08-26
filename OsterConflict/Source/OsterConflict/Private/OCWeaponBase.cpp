#include "OCWeaponBase.h"
#include "OCDamageTypes.h"

#include "OCCharacter.h"
#include "OCGameMode.h"
#include "OCHealthComponent.h"
#include "OCCombatVisualComponent.h"
#include "OCBreakableWindow.h"
#include "OCDestructibleProp.h"
#include "OCWeaponDefinition.h"
#include "OCWeaponAudioComponent.h"
#include "OCWeaponAudioProfile.h"
#include "OCTransientVisualFX.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    float ResolveCharacterDamageMultiplier(const AOCCharacter* Target, const FHitResult& Hit)
    {
        if (!Target) return 1.0f;
        const float LocalZ = Target->GetActorTransform().InverseTransformPosition(Hit.ImpactPoint).Z;
        // Source proxy/capsule-safe zones. Production skeletal profiles can replace this with authored hitboxes.
        if (LocalZ >= 62.0f) return 2.0f;   // head / neck band
        if (LocalZ <= -28.0f) return 0.85f; // lower limbs; keep pelvis/lower torso at normal damage
        return 1.0f;
    }

    EOCImpactSurface ResolveImpactSurface(const FHitResult& Hit)
    {
        if (Cast<AOCCharacter>(Hit.GetActor())) return EOCImpactSurface::Flesh;
        if (Cast<AOCBreakableWindow>(Hit.GetActor())) return EOCImpactSurface::Glass;
        if (const AOCDestructibleProp* Prop = Cast<AOCDestructibleProp>(Hit.GetActor())) return Prop->GetImpactSurface();
        if (const AActor* Actor = Hit.GetActor())
        {
            if (Actor->ActorHasTag(TEXT("Metal"))) return EOCImpactSurface::Metal;
            if (Actor->ActorHasTag(TEXT("Wood"))) return EOCImpactSurface::Wood;
            if (Actor->ActorHasTag(TEXT("Dirt"))) return EOCImpactSurface::Dirt;
            if (Actor->ActorHasTag(TEXT("Glass"))) return EOCImpactSurface::Glass;
        }
        return EOCImpactSurface::Masonry;
    }
}

AOCWeaponBase::AOCWeaponBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    bReplicates = true;
    SetReplicateMovement(false);

    // Pass45 runtime evidence proved that dropped weapons cannot use a non-physical SceneComponent root:
    // simulating only the hidden child mesh leaves the rendered production visual floating. Make the existing
    // source/collision mesh the actor physics root and keep WeaponRoot as an absolute-scale visual attach point.
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    SetRootComponent(WeaponMesh);

    WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));
    WeaponRoot->SetupAttachment(WeaponMesh);
    WeaponRoot->SetAbsolute(false, false, true);

    WeaponAudioComponent = CreateDefaultSubobject<UOCWeaponAudioComponent>(TEXT("WeaponAudio"));
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetEnableGravity(true);
    WeaponMesh->SetLinearDamping(0.55f);
    WeaponMesh->SetAngularDamping(1.10f);
    WeaponMesh->SetRelativeScale3D(FVector(0.35f, 0.08f, 0.08f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        WeaponMesh->SetStaticMesh(CubeMesh.Object);
    }
}

void AOCWeaponBase::BeginPlay()
{
    Super::BeginPlay();
    ApplyDefinitionIfAssigned();
    BuildSourceOnlyWeaponVisual();

    if (HasAuthority())
    {
        AmmoInMagazine = FMath::Clamp(AmmoInMagazine, 0, GetMagazineSize());
        ReserveAmmo = FMath::Clamp(ReserveAmmo, 0, Tuning.MaxReserveAmmo);
    }

    if (WeaponAudioComponent)
    {
        WeaponAudioComponent->SetAudioProfile(WeaponDefinition && WeaponDefinition->AudioProfile
            ? WeaponDefinition->AudioProfile : DefaultAudioProfile);
    }

    ApplyWorldPickupPresentation();
}

void AOCWeaponBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    RecoverConfirmedLocalShotRecoil(DeltaSeconds);
}

void AOCWeaponBase::ConfigureBuiltInTuning(const FOCWeaponTuning& NewTuning)
{
    Tuning = NewTuning;
    AmmoInMagazine = Tuning.MagazineSize;
    ReserveAmmo = Tuning.InitialReserveAmmo;
    CurrentFireMode = Tuning.bSupportsAutomatic ? EOCFireMode::Automatic : EOCFireMode::SemiAutomatic;
}

void AOCWeaponBase::ApplyDefinitionIfAssigned()
{
    if (!WeaponDefinition)
    {
        return;
    }

    Tuning = WeaponDefinition->Tuning;
    if (WeaponAudioComponent && WeaponDefinition->AudioProfile)
    {
        WeaponAudioComponent->SetAudioProfile(WeaponDefinition->AudioProfile);
    }
    if (HasAuthority())
    {
        AmmoInMagazine = Tuning.MagazineSize;
        ReserveAmmo = Tuning.InitialReserveAmmo;
        CurrentFireMode = Tuning.bSupportsAutomatic ? EOCFireMode::Automatic : EOCFireMode::SemiAutomatic;
    }
}

void AOCWeaponBase::BuildSourceOnlyWeaponVisual()
{
    if (!WeaponMesh || SourceVisualParts.Num() > 0) return;

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube || !Cylinder) return;

    auto ColorPart = [BaseMaterial](UStaticMeshComponent* Part, const FLinearColor& Color)
    {
        if (!BaseMaterial || !Part) return;
        if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, Part))
        {
            MID->SetVectorParameterValue(TEXT("Color"), Color);
            Part->SetMaterial(0, MID);
        }
    };

    const FLinearColor Metal(0.055f, 0.060f, 0.058f);
    const FLinearColor Polymer(0.075f, 0.085f, 0.075f);
    const FLinearColor Accent(0.18f, 0.16f, 0.10f);

    auto AddPart = [this, Cube, Cylinder, &ColorPart](const TCHAR* Name, bool bCylinder,
        const FVector& Location, const FVector& Scale, const FRotator& Rotation, const FLinearColor& Color)
    {
        UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this, FName(Name));
        if (!Part) return static_cast<UStaticMeshComponent*>(nullptr);
        Part->SetStaticMesh(bCylinder ? Cylinder : Cube);
        Part->SetupAttachment(WeaponRoot);
        Part->SetRelativeLocation(Location);
        Part->SetRelativeRotation(Rotation);
        Part->SetRelativeScale3D(Scale);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Part->SetGenerateOverlapEvents(false);
        Part->SetCastShadow(true);
        Part->RegisterComponent();
        ColorPart(Part, Color);
        SourceVisualParts.Add(Part);
        return Part;
    };

    // The root mesh becomes the receiver rather than the entire "gun cube".
    WeaponMesh->SetRelativeLocation(FVector::ZeroVector);
    WeaponMesh->SetRelativeRotation(FRotator::ZeroRotator);
    ColorPart(WeaponMesh, Metal);

    switch (Tuning.WeaponClass)
    {
    case EOCWeaponClass::Pistol:
        WeaponMesh->SetRelativeScale3D(FVector(0.16f, 0.055f, 0.075f));
        AddPart(TEXT("PistolSlide"), false, FVector(10,0,6), FVector(0.18f,0.048f,0.035f), FRotator::ZeroRotator, Metal);
        AddPart(TEXT("PistolGrip"), false, FVector(-3,0,-10), FVector(0.055f,0.050f,0.13f), FRotator(0,0,-12), Polymer);
        AddPart(TEXT("PistolBarrel"), true, FVector(22,0,5), FVector(0.018f,0.018f,0.11f), FRotator(0,90,0), Metal);
        AddPart(TEXT("PistolFrontSight"), false, FVector(21,0,10), FVector(0.012f,0.018f,0.025f), FRotator::ZeroRotator, Accent);
        break;

    case EOCWeaponClass::SMG:
        WeaponMesh->SetRelativeScale3D(FVector(0.25f, 0.07f, 0.09f));
        AddPart(TEXT("SMGHandguard"), false, FVector(28,0,0), FVector(0.20f,0.07f,0.07f), FRotator::ZeroRotator, Polymer);
        AddPart(TEXT("SMGBarrel"), true, FVector(51,0,1), FVector(0.020f,0.020f,0.17f), FRotator(0,90,0), Metal);
        AddPart(TEXT("SMGMagazine"), false, FVector(0,0,-18), FVector(0.07f,0.055f,0.19f), FRotator(0,0,-4), Polymer);
        AddPart(TEXT("SMGGrip"), false, FVector(-12,0,-15), FVector(0.055f,0.055f,0.15f), FRotator(0,0,-18), Polymer);
        AddPart(TEXT("SMGStock"), false, FVector(-34,0,0), FVector(0.18f,0.055f,0.045f), FRotator::ZeroRotator, Metal);
        break;

    case EOCWeaponClass::SniperRifle:
        WeaponMesh->SetRelativeScale3D(FVector(0.32f, 0.07f, 0.085f));
        AddPart(TEXT("SniperHandguard"), false, FVector(36,0,0), FVector(0.27f,0.065f,0.065f), FRotator::ZeroRotator, Accent);
        AddPart(TEXT("SniperBarrel"), true, FVector(79,0,1), FVector(0.017f,0.017f,0.31f), FRotator(0,90,0), Metal);
        AddPart(TEXT("SniperStock"), false, FVector(-38,0,-2), FVector(0.25f,0.08f,0.07f), FRotator::ZeroRotator, Accent);
        AddPart(TEXT("SniperGrip"), false, FVector(-9,0,-18), FVector(0.06f,0.055f,0.17f), FRotator(0,0,-15), Polymer);
        AddPart(TEXT("SniperMag"), false, FVector(7,0,-17), FVector(0.075f,0.055f,0.15f), FRotator(0,0,-5), Polymer);
        AddPart(TEXT("SniperScope"), true, FVector(2,0,15), FVector(0.045f,0.045f,0.19f), FRotator(0,90,0), Metal);
        break;

    case EOCWeaponClass::Shotgun:
        WeaponMesh->SetRelativeScale3D(FVector(0.30f, 0.065f, 0.075f));
        AddPart(TEXT("ShotgunForend"), false, FVector(40,0,-2), FVector(0.24f,0.075f,0.065f), FRotator::ZeroRotator, Accent);
        AddPart(TEXT("ShotgunBarrel"), true, FVector(77,0,4), FVector(0.022f,0.022f,0.32f), FRotator(0,90,0), Metal);
        AddPart(TEXT("ShotgunTube"), true, FVector(69,0,-6), FVector(0.018f,0.018f,0.25f), FRotator(0,90,0), Metal);
        AddPart(TEXT("ShotgunStock"), false, FVector(-39,0,-2), FVector(0.25f,0.075f,0.075f), FRotator::ZeroRotator, Accent);
        AddPart(TEXT("ShotgunGrip"), false, FVector(-9,0,-17), FVector(0.06f,0.055f,0.16f), FRotator(0,0,-15), Accent);
        break;

    case EOCWeaponClass::Launcher:
        WeaponMesh->SetRelativeScale3D(FVector(0.18f, 0.09f, 0.09f));
        AddPart(TEXT("LauncherTube"), true, FVector(28,0,0), FVector(0.10f,0.10f,0.62f), FRotator(0,90,0), Polymer);
        AddPart(TEXT("LauncherFrontRing"), true, FVector(84,0,0), FVector(0.13f,0.13f,0.07f), FRotator(0,90,0), Metal);
        AddPart(TEXT("LauncherRearRing"), true, FVector(-29,0,0), FVector(0.12f,0.12f,0.06f), FRotator(0,90,0), Metal);
        AddPart(TEXT("LauncherGrip"), false, FVector(4,0,-20), FVector(0.07f,0.06f,0.18f), FRotator(0,0,-14), Polymer);
        AddPart(TEXT("LauncherSight"), false, FVector(24,0,14), FVector(0.08f,0.035f,0.055f), FRotator::ZeroRotator, Metal);
        break;

    case EOCWeaponClass::LMG:
        WeaponMesh->SetRelativeScale3D(FVector(0.32f, 0.085f, 0.10f));
        AddPart(TEXT("LMGHandguard"), false, FVector(39,0,0), FVector(0.27f,0.085f,0.075f), FRotator::ZeroRotator, Polymer);
        AddPart(TEXT("LMGBarrel"), true, FVector(82,0,1), FVector(0.022f,0.022f,0.32f), FRotator(0,90,0), Metal);
        AddPart(TEXT("LMGStock"), false, FVector(-40,0,-2), FVector(0.24f,0.085f,0.075f), FRotator::ZeroRotator, Polymer);
        AddPart(TEXT("LMGGrip"), false, FVector(-8,0,-19), FVector(0.06f,0.06f,0.17f), FRotator(0,0,-16), Polymer);
        AddPart(TEXT("LMGBoxMag"), false, FVector(12,0,-19), FVector(0.13f,0.12f,0.16f), FRotator::ZeroRotator, Polymer);
        AddPart(TEXT("LMGBipodL"), true, FVector(45,-7,-20), FVector(0.012f,0.012f,0.18f), FRotator(8,0,18), Metal);
        AddPart(TEXT("LMGBipodR"), true, FVector(45,7,-20), FVector(0.012f,0.012f,0.18f), FRotator(-8,0,-18), Metal);
        break;

    default: // Assault rifle fallback.
        WeaponMesh->SetRelativeScale3D(FVector(0.29f, 0.075f, 0.09f));
        AddPart(TEXT("RifleHandguard"), false, FVector(35,0,0), FVector(0.24f,0.075f,0.07f), FRotator::ZeroRotator, Polymer);
        AddPart(TEXT("RifleBarrel"), true, FVector(71,0,1), FVector(0.019f,0.019f,0.27f), FRotator(0,90,0), Metal);
        AddPart(TEXT("RifleMuzzle"), true, FVector(88,0,1), FVector(0.028f,0.028f,0.07f), FRotator(0,90,0), Metal);
        AddPart(TEXT("RifleStock"), false, FVector(-38,0,-2), FVector(0.23f,0.075f,0.075f), FRotator::ZeroRotator, Polymer);
        AddPart(TEXT("RifleGrip"), false, FVector(-8,0,-18), FVector(0.06f,0.055f,0.17f), FRotator(0,0,-16), Polymer);
        AddPart(TEXT("RifleMagazine"), false, FVector(12,0,-19), FVector(0.075f,0.06f,0.18f), FRotator(0,0,-8), Polymer);
        AddPart(TEXT("RifleRearSight"), false, FVector(-4,0,13), FVector(0.025f,0.035f,0.035f), FRotator::ZeroRotator, Metal);
        AddPart(TEXT("RifleFrontSight"), false, FVector(55,0,12), FVector(0.018f,0.025f,0.05f), FRotator::ZeroRotator, Metal);
        break;
    }
}

void AOCWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOCWeaponBase, AmmoInMagazine);
    DOREPLIFETIME(AOCWeaponBase, ReserveAmmo);
    DOREPLIFETIME(AOCWeaponBase, bIsReloading);
    DOREPLIFETIME(AOCWeaponBase, CurrentFireMode);
    DOREPLIFETIME(AOCWeaponBase, bIsWorldPickup);
    DOREPLIFETIME(AOCWeaponBase, Attachments);
}

int32 AOCWeaponBase::GetMagazineSize() const
{
    return Tuning.MagazineSize + (HasAttachment(FName(TEXT("ExtendedMag"))) ? FMath::Max(5, Tuning.MagazineSize / 3) : 0);
}

float AOCWeaponBase::GetFireInterval() const
{
    return Tuning.RoundsPerMinute > 0.0f ? 60.0f / Tuning.RoundsPerMinute : 1.0f;
}

bool AOCWeaponBase::HasAttachment(FName AttachmentId) const
{
    for (const FOCWeaponAttachmentState& State : Attachments)
    {
        if (State.AttachmentId == AttachmentId)
        {
            return true;
        }
    }
    return false;
}

float AOCWeaponBase::GetRecoilMultiplier() const
{
    float Multiplier = 1.0f;
    if (HasAttachment(FName(TEXT("VerticalGrip"))))
    {
        Multiplier *= 0.82f;
    }
    if (HasAttachment(FName(TEXT("LightStock"))))
    {
        Multiplier *= 0.90f;
    }
    return Multiplier;
}

float AOCWeaponBase::GetADSSpreadMultiplier() const
{
    return HasAttachment(FName(TEXT("RedDot"))) ? 0.88f : 1.0f;
}

float AOCWeaponBase::GetDamageMultiplier() const
{
    return HasAttachment(FName(TEXT("Suppressor"))) ? 0.95f : 1.0f;
}

bool AOCWeaponBase::IsSuppressed() const
{
    return HasAttachment(FName(TEXT("Suppressor")));
}

bool AOCWeaponBase::ShouldNeutralizeLegacyLocalRecoil() const
{
    const AOCCharacter* OwnerCharacter = Cast<AOCCharacter>(GetOwner());
    return OwnerCharacter && OwnerCharacter->IsLocallyControlled();
}

float AOCWeaponBase::GetRecoilPitchMin() const
{
    return ShouldNeutralizeLegacyLocalRecoil() ? 0.0f : Tuning.RecoilPitchMin * GetRecoilMultiplier();
}

float AOCWeaponBase::GetRecoilPitchMax() const
{
    return ShouldNeutralizeLegacyLocalRecoil() ? 0.0f : Tuning.RecoilPitchMax * GetRecoilMultiplier();
}

float AOCWeaponBase::GetRecoilYawMax() const
{
    return ShouldNeutralizeLegacyLocalRecoil() ? 0.0f : Tuning.RecoilYawMax * GetRecoilMultiplier();
}

float AOCWeaponBase::CalculateSpreadDegrees(bool bAiming, bool bMoving) const
{
    float Spread = bAiming ? Tuning.ADSSpreadDegrees * GetADSSpreadMultiplier() : Tuning.HipSpreadDegrees;
    if (bMoving)
    {
        Spread *= Tuning.MovingSpreadMultiplier;
    }
    return FMath::Max(0.0f, Spread);
}

bool AOCWeaponBase::TryFireServer(AOCCharacter* Shooter, const FVector& TraceOrigin, const FVector& TraceDirection,
    bool bAiming, bool bMoving, FHitResult& OutHit, bool& bOutDamagedActor, bool& bOutFatalHit)
{
    bOutDamagedActor = false;
    bOutFatalHit = false;
    OutHit = FHitResult();

    if (!HasAuthority() || !Shooter || bIsWorldPickup || Tuning.RoundsPerMinute <= 0.0f || bIsReloading || !GetWorld())
    {
        return false;
    }

    const double CurrentTime = GetWorld()->GetTimeSeconds();
    if (AmmoInMagazine <= 0)
    {
        if ((CurrentTime - LastServerDryFireTime) >= 0.18)
        {
            LastServerDryFireTime = CurrentTime;
            MulticastWeaponStateAudio(EOCWeaponAudioEvent::DryFire, GetActorLocation(), ++ServerAudioEventCounter);
        }
        return false;
    }
    const double FireInterval = static_cast<double>(GetFireInterval());
    // Timer callbacks can arrive a few milliseconds before their nominal cadence. The old exact comparison could
    // reject that pulse, after which Character stopped authoritative autofire while its local held-input recoil timer
    // kept running. Accept a small bounded scheduler tolerance without permitting materially faster fire rates.
    const double CadenceTolerance = FMath::Min(0.008, FireInterval * 0.10);
    if ((CurrentTime - LastServerFireTime) + CadenceTolerance < FireInterval)
    {
        return false;
    }

    LastServerFireTime = CurrentTime;
    --AmmoInMagazine;

    const FVector SafeDirection = TraceDirection.GetSafeNormal();
    const FVector PresentationMuzzleOrigin = ResolvePresentationMuzzleOrigin(TraceOrigin, SafeDirection);
    const float SpreadRadians = FMath::DegreesToRadians(CalculateSpreadDegrees(bAiming, bMoving));
    const int32 PelletCount = FMath::Clamp(Tuning.PelletsPerShot, 1, 16);

    // Aim/hit authority remains the view ray so close cover and crosshair semantics do not regress. Presentation
    // is reconciled to the production muzzle below, preventing tracers/flash/audio from appearing under the barrel.
    FVector RepresentativeTraceEnd = TraceOrigin + SafeDirection * Tuning.RangeCm;
    bool bRepresentativeHit = false;

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(OCWeaponTrace), true, Shooter);
    QueryParams.AddIgnoredActor(this);

    for (int32 PelletIndex = 0; PelletIndex < PelletCount; ++PelletIndex)
    {
        const FVector ShotDirection = SpreadRadians > KINDA_SMALL_NUMBER
            ? FMath::VRandCone(SafeDirection, SpreadRadians)
            : SafeDirection;
        const FVector TraceEnd = TraceOrigin + ShotDirection * Tuning.RangeCm;

        FHitResult PelletHit;
        const bool bHit = GetWorld()->LineTraceSingleByChannel(PelletHit, TraceOrigin, TraceEnd, ECC_Visibility, QueryParams);
        if (PelletIndex == 0)
        {
            OutHit = PelletHit;
            bRepresentativeHit = bHit;
            RepresentativeTraceEnd = bHit ? PelletHit.ImpactPoint : TraceEnd;
        }

        if (bHit && PelletHit.GetActor())
        {
            const AOCGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AOCGameMode>() : nullptr;
            if (GameMode && !GameMode->CanDealDamage(Shooter->GetController(), PelletHit.GetActor()))
            {
                continue;
            }

            const AOCCharacter* HitCharacter = Cast<AOCCharacter>(PelletHit.GetActor());
            const float HitZoneMultiplier = ResolveCharacterDamageMultiplier(HitCharacter, PelletHit);
            const float AppliedDamage = UGameplayStatics::ApplyPointDamage(
                PelletHit.GetActor(),
                Tuning.Damage * GetDamageMultiplier() * HitZoneMultiplier,
                ShotDirection,
                PelletHit,
                Shooter->GetController(),
                this,
                UOCBallisticDamageType::StaticClass());

            if (AppliedDamage > 0.0f)
            {
                bOutDamagedActor = true;
                const UOCHealthComponent* TargetHealth = PelletHit.GetActor()->FindComponentByClass<UOCHealthComponent>();
                const bool bFatalThisPellet = TargetHealth && TargetHealth->IsDead();
                bOutFatalHit = bOutFatalHit || bFatalThisPellet;

                if (AOCCharacter* TargetCharacter = Cast<AOCCharacter>(PelletHit.GetActor()))
                {
                    if (UOCCombatVisualComponent* Trauma = TargetCharacter->GetCombatVisualComponent())
                    {
                        Trauma->RecordPointTraumaServer(AppliedDamage, PelletHit.ImpactPoint, ShotDirection,
                            PelletHit.BoneName, GetWeaponClass(), UOCBallisticDamageType::StaticClass(), bFatalThisPellet);
                    }
                }
            }
        }
    }

    MulticastFireTraceFX(PresentationMuzzleOrigin, RepresentativeTraceEnd, bRepresentativeHit);
    const EOCAcousticEnvironment AcousticEnvironment = WeaponAudioComponent
        ? WeaponAudioComponent->DetectEnvironmentAt(PresentationMuzzleOrigin) : EOCAcousticEnvironment::Outdoor;
    MulticastShotAudio(PresentationMuzzleOrigin, RepresentativeTraceEnd, IsSuppressed(), Tuning.bSupersonicAmmo,
        AcousticEnvironment, ++ServerAudioEventCounter);
    if (bRepresentativeHit)
    {
        MulticastImpactFX(OutHit.ImpactPoint, OutHit.ImpactNormal.GetSafeNormal(), ResolveImpactSurface(OutHit));
    }
    return true;
}

bool AOCWeaponBase::BeginReloadServer()
{
    if (!HasAuthority() || bIsWorldPickup || bIsReloading || AmmoInMagazine >= GetMagazineSize() || ReserveAmmo <= 0)
    {
        return false;
    }

    bIsReloading = true;
    MulticastWeaponStateAudio(EOCWeaponAudioEvent::ReloadStart, GetActorLocation(), ++ServerAudioEventCounter);
    GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &AOCWeaponBase::FinishReloadServer,
        FMath::Max(0.05f, Tuning.ReloadDuration), false);
    return true;
}

void AOCWeaponBase::CancelReloadServer()
{
    if (!HasAuthority() || !bIsReloading)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(ReloadTimerHandle);
    bIsReloading = false;
    MulticastWeaponStateAudio(EOCWeaponAudioEvent::ReloadCancel, GetActorLocation(), ++ServerAudioEventCounter);
}

void AOCWeaponBase::FinishReloadServer()
{
    if (!HasAuthority() || !bIsReloading)
    {
        return;
    }

    const int32 Needed = GetMagazineSize() - AmmoInMagazine;
    const int32 ToLoad = FMath::Min(Needed, ReserveAmmo);
    AmmoInMagazine += ToLoad;
    ReserveAmmo -= ToLoad;
    bIsReloading = false;
    MulticastWeaponStateAudio(EOCWeaponAudioEvent::ReloadEnd, GetActorLocation(), ++ServerAudioEventCounter);
}

bool AOCWeaponBase::CycleFireModeServer()
{
    if (!HasAuthority() || bIsWorldPickup)
    {
        return false;
    }

    if (Tuning.bSupportsSemiAutomatic && Tuning.bSupportsAutomatic)
    {
        CurrentFireMode = CurrentFireMode == EOCFireMode::Automatic
            ? EOCFireMode::SemiAutomatic
            : EOCFireMode::Automatic;
        MulticastWeaponStateAudio(EOCWeaponAudioEvent::FireModeSwitch, GetActorLocation(), ++ServerAudioEventCounter);
        return true;
    }

    CurrentFireMode = Tuning.bSupportsAutomatic ? EOCFireMode::Automatic : EOCFireMode::SemiAutomatic;
    return false;
}

void AOCWeaponBase::EquipToCharacterServer(AOCCharacter* NewOwnerCharacter)
{
    if (!HasAuthority() || !NewOwnerCharacter)
    {
        return;
    }

    CancelReloadServer();
    bIsWorldPickup = false;
    SetOwner(NewOwnerCharacter);
    SetInstigator(NewOwnerCharacter);
    SetReplicateMovement(false);
    WeaponMesh->SetSimulatePhysics(false);
    WeaponMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
    WeaponMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ForceNetUpdate();
    MulticastWeaponStateAudio(EOCWeaponAudioEvent::Equip, GetActorLocation(), ++ServerAudioEventCounter);
}

void AOCWeaponBase::StoreInInventoryServer(AOCCharacter* NewOwnerCharacter)
{
    EquipToCharacterServer(NewOwnerCharacter);
}

void AOCWeaponBase::DropToWorldServer(const FVector& DropLocation, const FRotator& DropRotation)
{
    if (!HasAuthority())
    {
        return;
    }

    const FVector InheritedVelocity = GetOwner() ? GetOwner()->GetVelocity() : FVector::ZeroVector;
    CancelReloadServer();
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    SetOwner(nullptr);
    SetInstigator(nullptr);
    SetActorLocationAndRotation(DropLocation, DropRotation, false, nullptr, ETeleportType::TeleportPhysics);
    bIsWorldPickup = true;
    SetReplicateMovement(true);
    ApplyWorldPickupPresentation();

    // Only a deliberate player drop becomes a simulated rigid body. Static/rack world pickups keep their authored
    // placement, while dropped weapons now actually fall, collide, settle and sleep instead of hovering in mid-air.
    if (WeaponMesh)
    {
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        WeaponMesh->SetSimulatePhysics(true);
        WeaponMesh->SetEnableGravity(true);
        WeaponMesh->SetPhysicsLinearVelocity(InheritedVelocity);
        WeaponMesh->SetPhysicsAngularVelocityInDegrees(FVector(0.0f, 0.0f, 28.0f));
        WeaponMesh->WakeAllRigidBodies();
    }

    ForceNetUpdate();
    MulticastWeaponStateAudio(EOCWeaponAudioEvent::Drop, GetActorLocation(), ++ServerAudioEventCounter);
}

int32 AOCWeaponBase::AddReserveAmmoServer(int32 Amount)
{
    if (!HasAuthority() || Amount <= 0)
    {
        return 0;
    }

    const int32 Previous = ReserveAmmo;
    ReserveAmmo = FMath::Clamp(ReserveAmmo + Amount, 0, Tuning.MaxReserveAmmo);
    return ReserveAmmo - Previous;
}

bool AOCWeaponBase::InstallAttachmentServer(EOCAttachmentSlot Slot, FName AttachmentId)
{
    if (!HasAuthority())
    {
        return false;
    }

    for (FOCWeaponAttachmentState& State : Attachments)
    {
        if (State.Slot == Slot)
        {
            State.AttachmentId = AttachmentId;
            ForceNetUpdate();
            return true;
        }
    }

    FOCWeaponAttachmentState NewState;
    NewState.Slot = Slot;
    NewState.AttachmentId = AttachmentId;
    Attachments.Add(NewState);
    ForceNetUpdate();
    return true;
}

FString AOCWeaponBase::GetAttachmentSummary() const
{
    TArray<FString> Names;
    for (const FOCWeaponAttachmentState& State : Attachments)
    {
        if (!State.AttachmentId.IsNone())
        {
            Names.Add(State.AttachmentId.ToString());
        }
    }
    return FString::Join(Names, TEXT(" | "));
}

void AOCWeaponBase::ApplyInventoryPresentation(bool bActive, USceneComponent* ActiveAttachParent)
{
    if (bIsWorldPickup)
    {
        ApplyWorldPickupPresentation();
        return;
    }

    WeaponMesh->SetSimulatePhysics(false);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetActorHiddenInGame(!bActive);

    if (bActive && ActiveAttachParent)
    {
        AttachToComponent(ActiveAttachParent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        SetActorRelativeLocation(FVector(38.0f, 12.0f, -14.0f));
        SetActorRelativeRotation(FRotator::ZeroRotator);
    }
}

void AOCWeaponBase::ApplyWorldPickupPresentation()
{
    if (!WeaponMesh)
    {
        return;
    }

    if (bIsWorldPickup)
    {
        SetActorHiddenInGame(false);
        // Rack/static pickups are intentionally not simulated here. DropToWorldServer enables authority physics
        // only for a weapon that has actually been dropped by a character.
        if (!HasAuthority())
        {
            WeaponMesh->SetSimulatePhysics(false);
        }
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
        WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    }
    else
    {
        WeaponMesh->SetSimulatePhysics(false);
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void AOCWeaponBase::OnRep_WorldPickup()
{
    ApplyWorldPickupPresentation();
}

void AOCWeaponBase::OnRep_Attachments()
{
    // Presentation hooks are intentionally light in S04. Functional modifiers already apply from replicated IDs.
}

void AOCWeaponBase::ApplyConfirmedLocalShotRecoil()
{
    AOCCharacter* LocalShooter = Cast<AOCCharacter>(GetOwner());
    if (!LocalShooter || !LocalShooter->IsLocallyControlled() || !GetWorld())
    {
        return;
    }

    const float RecoilMultiplier = GetRecoilMultiplier();
    const float PitchKick = FMath::FRandRange(Tuning.RecoilPitchMin, Tuning.RecoilPitchMax) * RecoilMultiplier;
    const float YawKick = FMath::FRandRange(-Tuning.RecoilYawMax, Tuning.RecoilYawMax) * RecoilMultiplier;

    LocalShooter->AddControllerPitchInput(-PitchKick);
    LocalShooter->AddControllerYawInput(YawKick);
    ConfirmedLocalRecoilPitchOffset += PitchKick;
    ConfirmedLocalRecoilYawOffset += YawKick;
    LastConfirmedLocalShotTime = GetWorld()->GetTimeSeconds();
}

void AOCWeaponBase::RecoverConfirmedLocalShotRecoil(float DeltaSeconds)
{
    AOCCharacter* LocalShooter = Cast<AOCCharacter>(GetOwner());
    if (!LocalShooter || !LocalShooter->IsLocallyControlled() || !GetWorld() ||
        GetWorld()->GetTimeSeconds() - LastConfirmedLocalShotTime < ConfirmedRecoilRecoveryDelay)
    {
        return;
    }

    if (ConfirmedLocalRecoilPitchOffset > KINDA_SMALL_NUMBER)
    {
        const float PitchStep = FMath::Min(ConfirmedLocalRecoilPitchOffset, ConfirmedRecoilRecoverySpeed * DeltaSeconds);
        LocalShooter->AddControllerPitchInput(PitchStep);
        ConfirmedLocalRecoilPitchOffset -= PitchStep;
    }

    if (!FMath::IsNearlyZero(ConfirmedLocalRecoilYawOffset, 0.001f))
    {
        const float NewYawOffset = FMath::FInterpTo(
            ConfirmedLocalRecoilYawOffset, 0.0f, DeltaSeconds, ConfirmedRecoilRecoverySpeed);
        LocalShooter->AddControllerYawInput(NewYawOffset - ConfirmedLocalRecoilYawOffset);
        ConfirmedLocalRecoilYawOffset = NewYawOffset;
    }
}

void AOCWeaponBase::MulticastFireTraceFX_Implementation(FVector_NetQuantize TraceStart, FVector_NetQuantize TraceEnd, bool bHit)
{
    if (!GetWorld()) return;

    const FVector Start(TraceStart);
    const FVector End(TraceEnd);
    const FVector Direction = (End - Start).GetSafeNormal();

    // This multicast is emitted only after TryFireServer accepts a factual shot, so local recoil is now tied to
    // accepted shot count rather than the duration of LMB input. The legacy Character timer receives zero recoil
    // values for the locally-owned weapon and therefore cannot create the old post-shot downward recovery drift.
    ApplyConfirmedLocalShotRecoil();

    if (AOCTransientVisualFX* Muzzle = GetWorld()->SpawnActor<AOCTransientVisualFX>(
        AOCTransientVisualFX::StaticClass(), Start, Direction.Rotation()))
    {
        Muzzle->ConfigureMuzzle(Start, Direction, FLinearColor(1.0f, 0.58f, 0.12f));
    }

    const float FullLength = FVector::Distance(Start, End);
    if (FullLength > 200.0f)
    {
        // A partial streak reads like a fast projectile rather than a permanent laser beam.
        const float VisibleLength = FMath::Clamp(FullLength * 0.18f, 180.0f, 900.0f);
        const FVector TracerEnd = End;
        const FVector TracerStart = End - Direction * VisibleLength;
        if (AOCTransientVisualFX* Tracer = GetWorld()->SpawnActor<AOCTransientVisualFX>(
            AOCTransientVisualFX::StaticClass(), (TracerStart + TracerEnd) * 0.5f, Direction.Rotation()))
        {
            Tracer->ConfigureTracer(TracerStart, TracerEnd,
                bHit ? FLinearColor(1.0f, 0.50f, 0.10f) : FLinearColor(1.0f, 0.70f, 0.18f));
        }
    }
}

void AOCWeaponBase::MulticastShotAudio_Implementation(FVector_NetQuantize ShotOrigin, FVector_NetQuantize TraceEnd,
    bool bSuppressed, bool bSupersonic, EOCAcousticEnvironment Environment, int32 EventSeed)
{
    if (WeaponAudioComponent)
    {
        WeaponAudioComponent->HandleShotLocal(ShotOrigin, TraceEnd, bSuppressed, bSupersonic, Environment, EventSeed);
    }
}

void AOCWeaponBase::MulticastWeaponStateAudio_Implementation(EOCWeaponAudioEvent Event,
    FVector_NetQuantize SourceLocation, int32 EventSeed)
{
    if (WeaponAudioComponent)
    {
        WeaponAudioComponent->HandleStateEventLocal(Event, SourceLocation, EventSeed);
    }
}

void AOCWeaponBase::MulticastImpactFX_Implementation(FVector_NetQuantize ImpactLocation,
    FVector_NetQuantizeNormal ImpactNormal, EOCImpactSurface SurfaceType)
{
    BP_PlayImpactFX(ImpactLocation, ImpactNormal, SurfaceType);
    if (WeaponAudioComponent)
    {
        WeaponAudioComponent->HandleImpactLocal(ImpactLocation, SurfaceType, ++ServerAudioEventCounter);
    }

    if (!GetWorld()) return;

    FLinearColor ImpactColor(0.48f, 0.46f, 0.42f);
    float Radius = 4.5f;
    switch (SurfaceType)
    {
        case EOCImpactSurface::Flesh: ImpactColor = FLinearColor(0.32f, 0.01f, 0.008f); Radius = 5.0f; break;
        case EOCImpactSurface::Glass: ImpactColor = FLinearColor(0.35f, 0.75f, 0.92f); Radius = 4.0f; break;
        case EOCImpactSurface::Wood: ImpactColor = FLinearColor(0.32f, 0.16f, 0.055f); Radius = 5.0f; break;
        case EOCImpactSurface::Metal: ImpactColor = FLinearColor(0.78f, 0.70f, 0.50f); Radius = 3.5f; break;
        case EOCImpactSurface::Masonry: ImpactColor = FLinearColor(0.46f, 0.44f, 0.39f); Radius = 5.5f; break;
        case EOCImpactSurface::Dirt: ImpactColor = FLinearColor(0.29f, 0.20f, 0.10f); Radius = 6.0f; break;
        default: break;
    }

    if (AOCTransientVisualFX* Impact = GetWorld()->SpawnActor<AOCTransientVisualFX>(
        AOCTransientVisualFX::StaticClass(), FVector(ImpactLocation), FRotator::ZeroRotator))
    {
        Impact->ConfigureImpact(FVector(ImpactLocation), FVector(ImpactNormal), ImpactColor, Radius);
    }
}
