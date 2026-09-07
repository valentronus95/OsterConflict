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
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    float ResolveCharacterDamageMultiplier(const AOCCharacter* Target, const FHitResult& Hit)
    {
        if (!Target) return 1.0f;
        const float LocalZ = Target->GetActorTransform().InverseTransformPosition(Hit.ImpactPoint).Z;
        if (LocalZ >= 62.0f) return 2.0f;
        if (LocalZ <= -28.0f) return 0.85f;
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
    CurrentFireMode = Tuning.bSupportsAutomatic ? EOCFireMode::Automatic
        : (Tuning.bSupportsBurst3 ? EOCFireMode::Burst3 : EOCFireMode::SemiAutomatic);
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
        CurrentFireMode = Tuning.bSupportsAutomatic ? EOCFireMode::Automatic
            : (Tuning.bSupportsBurst3 ? EOCFireMode::Burst3 : EOCFireMode::SemiAutomatic);
    }
}

void AOCWeaponBase::BuildSourceOnlyWeaponVisual()
{
    if (!WeaponMesh) return;

    // GAME_RECOVERY: the old runtime composite built Cube/Cylinder/material parts for every weapon during BeginPlay,
    // only for production/fallback owners to hide them immediately afterwards. Keep the root cube solely as invisible
    // collision/physics authority and retire all decorative BasicShape construction from normal gameplay.
    WeaponMesh->SetVisibility(false, false);
    WeaponMesh->SetHiddenInGame(true, false);
    WeaponMesh->SetCastShadow(false);
    WeaponMesh->SetCanEverAffectNavigation(false);

    UE_LOG(LogTemp, Verbose,
        TEXT("GAME_RECOVERY_SOURCE_WEAPON_COMPOSITE_RETIRED weapon=%s decorative_parts=0 blocking_asset_loads=0 primitive_visible=0 collision_authority_preserved=1"),
        *Tuning.WeaponId.ToString());
}

void AOCWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOCWeaponBase, AmmoInMagazine);
    DOREPLIFETIME(AOCWeaponBase, ReserveAmmo);
    DOREPLIFETIME(AOCWeaponBase, bIsReloading);
    DOREPLIFETIME(AOCWeaponBase, bActionCycling);
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
        if (State.AttachmentId == AttachmentId) return true;
    }
    return false;
}

float AOCWeaponBase::GetRecoilMultiplier() const
{
    float Multiplier = 1.0f;
    if (HasAttachment(FName(TEXT("VerticalGrip")))) Multiplier *= 0.82f;
    if (HasAttachment(FName(TEXT("LightStock")))) Multiplier *= 0.90f;
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

float AOCWeaponBase::GetRecoilPitchMin() const
{
    return Tuning.RecoilPitchMin * GetRecoilMultiplier();
}

float AOCWeaponBase::GetRecoilPitchMax() const
{
    return Tuning.RecoilPitchMax * GetRecoilMultiplier();
}

float AOCWeaponBase::GetRecoilYawMax() const
{
    return Tuning.RecoilYawMax * GetRecoilMultiplier();
}

bool AOCWeaponBase::RequiresManualActionCycle() const
{
    switch (Tuning.ActionType)
    {
        case EOCWeaponActionType::BoltAction:
        case EOCWeaponActionType::PumpAction:
        case EOCWeaponActionType::LeverAction:
            return Tuning.ManualActionCycleSeconds > KINDA_SMALL_NUMBER;
        default:
            return false;
    }
}

void AOCWeaponBase::BeginManualActionCycleServer()
{
    if (!HasAuthority() || !RequiresManualActionCycle() || !GetWorld()) return;

    const float Duration = FMath::Max(0.05f, Tuning.ManualActionCycleSeconds);
    bActionCycling = true;
    GetWorldTimerManager().SetTimer(ManualActionTimerHandle, this, &AOCWeaponBase::FinishManualActionCycleServer,
        Duration, false);
    ForceNetUpdate();
    UE_LOG(LogTemp, Verbose,
        TEXT("PASS45_MANUAL_ACTION_CYCLE_READY weapon=%s action=%s duration=%.3f authoritative=1"),
        *Tuning.WeaponId.ToString(), *UEnum::GetValueAsString(Tuning.ActionType), Duration);
}

void AOCWeaponBase::FinishManualActionCycleServer()
{
    if (!HasAuthority()) return;
    bActionCycling = false;
    ForceNetUpdate();
}

void AOCWeaponBase::CancelManualActionCycleServer()
{
    if (!HasAuthority()) return;
    GetWorldTimerManager().ClearTimer(ManualActionTimerHandle);
    bActionCycling = false;
    ForceNetUpdate();
}

float AOCWeaponBase::CalculateSpreadDegrees(bool bAiming, bool bMoving) const
{
    float Spread = bAiming ? Tuning.ADSSpreadDegrees * GetADSSpreadMultiplier() : Tuning.HipSpreadDegrees;
    if (bMoving) Spread *= Tuning.MovingSpreadMultiplier;
    return FMath::Max(0.0f, Spread);
}

bool AOCWeaponBase::TryFireServer(AOCCharacter* Shooter, const FVector& TraceOrigin, const FVector& TraceDirection,
    bool bAiming, bool bMoving, FHitResult& OutHit, bool& bOutDamagedActor, bool& bOutFatalHit)
{
    bOutDamagedActor = false;
    bOutFatalHit = false;
    OutHit = FHitResult();

    if (!HasAuthority() || !Shooter || bIsWorldPickup || Tuning.RoundsPerMinute <= 0.0f || bIsReloading ||
        bActionCycling || !GetWorld())
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
            if (GameMode && !GameMode->CanDealDamage(Shooter->GetController(), PelletHit.GetActor())) continue;

            const AOCCharacter* HitCharacter = Cast<AOCCharacter>(PelletHit.GetActor());
            const float HitZoneMultiplier = ResolveCharacterDamageMultiplier(HitCharacter, PelletHit);
            const float AppliedDamage = UGameplayStatics::ApplyPointDamage(
                PelletHit.GetActor(), Tuning.Damage * GetDamageMultiplier() * HitZoneMultiplier,
                ShotDirection, PelletHit, Shooter->GetController(), this, UOCBallisticDamageType::StaticClass());

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

    BeginManualActionCycleServer();
    return true;
}

bool AOCWeaponBase::BeginReloadServer()
{
    if (!HasAuthority() || bIsWorldPickup || bIsReloading || bActionCycling ||
        AmmoInMagazine >= GetMagazineSize() || ReserveAmmo <= 0)
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
    if (!HasAuthority() || !bIsReloading) return;
    GetWorldTimerManager().ClearTimer(ReloadTimerHandle);
    bIsReloading = false;
    MulticastWeaponStateAudio(EOCWeaponAudioEvent::ReloadCancel, GetActorLocation(), ++ServerAudioEventCounter);
}

void AOCWeaponBase::FinishReloadServer()
{
    if (!HasAuthority() || !bIsReloading) return;
    const int32 Needed = GetMagazineSize() - AmmoInMagazine;
    const int32 ToLoad = FMath::Min(Needed, ReserveAmmo);
    AmmoInMagazine += ToLoad;
    ReserveAmmo -= ToLoad;
    bIsReloading = false;
    MulticastWeaponStateAudio(EOCWeaponAudioEvent::ReloadEnd, GetActorLocation(), ++ServerAudioEventCounter);
}

bool AOCWeaponBase::CycleFireModeServer()
{
    if (!HasAuthority() || bIsWorldPickup || bActionCycling) return false;

    static constexpr EOCFireMode SelectorOrder[] =
    {
        EOCFireMode::SemiAutomatic,
        EOCFireMode::Burst3,
        EOCFireMode::Automatic,
    };

    int32 CurrentIndex = INDEX_NONE;
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(SelectorOrder); ++Index)
    {
        if (SelectorOrder[Index] == CurrentFireMode)
        {
            CurrentIndex = Index;
            break;
        }
    }

    for (int32 Step = 1; Step <= UE_ARRAY_COUNT(SelectorOrder); ++Step)
    {
        const int32 CandidateIndex = (FMath::Max(CurrentIndex, 0) + Step) % UE_ARRAY_COUNT(SelectorOrder);
        const EOCFireMode Candidate = SelectorOrder[CandidateIndex];
        if (!SupportsFireMode(Candidate) || Candidate == CurrentFireMode) continue;

        CurrentFireMode = Candidate;
        MulticastWeaponStateAudio(EOCWeaponAudioEvent::FireModeSwitch, GetActorLocation(), ++ServerAudioEventCounter);
        ForceNetUpdate();
        return true;
    }
    return false;
}

void AOCWeaponBase::EquipToCharacterServer(AOCCharacter* NewOwnerCharacter)
{
    if (!HasAuthority() || !NewOwnerCharacter) return;
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
    if (!HasAuthority()) return;

    const FVector InheritedVelocity = GetOwner() ? GetOwner()->GetVelocity() : FVector::ZeroVector;
    CancelReloadServer();
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    SetOwner(nullptr);
    SetInstigator(nullptr);
    SetActorLocationAndRotation(DropLocation, DropRotation, false, nullptr, ETeleportType::TeleportPhysics);
    bIsWorldPickup = true;
    SetReplicateMovement(true);
    ApplyWorldPickupPresentation();

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
    if (!HasAuthority() || Amount <= 0) return 0;
    const int32 Previous = ReserveAmmo;
    ReserveAmmo = FMath::Clamp(ReserveAmmo + Amount, 0, Tuning.MaxReserveAmmo);
    return ReserveAmmo - Previous;
}

bool AOCWeaponBase::InstallAttachmentServer(EOCAttachmentSlot Slot, FName AttachmentId)
{
    if (!HasAuthority()) return false;
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
        if (!State.AttachmentId.IsNone()) Names.Add(State.AttachmentId.ToString());
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
    if (!WeaponMesh) return;

    if (bIsWorldPickup)
    {
        SetActorHiddenInGame(false);
        if (!HasAuthority()) WeaponMesh->SetSimulatePhysics(false);
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
}

void AOCWeaponBase::ApplyConfirmedLocalShotRecoil()
{
    AOCCharacter* LocalShooter = Cast<AOCCharacter>(GetOwner());
    if (!LocalShooter || !LocalShooter->IsLocallyControlled() || !GetWorld()) return;

    const float RecoilMultiplier = GetRecoilMultiplier();
    const float PitchKick = FMath::FRandRange(Tuning.RecoilPitchMin, Tuning.RecoilPitchMax) * RecoilMultiplier;
    const float YawKick = FMath::FRandRange(-Tuning.RecoilYawMax, Tuning.RecoilYawMax) * RecoilMultiplier;

    LocalShooter->AddControllerPitchInput(-PitchKick);
    LocalShooter->AddControllerYawInput(YawKick);
    ConfirmedLocalRecoilPitchOffset += PitchKick;
    ConfirmedLocalRecoilYawOffset += YawKick;
    LastConfirmedLocalShotTime = GetWorld()->GetTimeSeconds();
    LocalShooter->NotifyConfirmedWeaponShotPresentation();
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
        const float NewYawOffset = FMath::FInterpTo(ConfirmedLocalRecoilYawOffset, 0.0f, DeltaSeconds, ConfirmedRecoilRecoverySpeed);
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

    // This multicast exists only after TryFireServer accepted a factual shot. Recoil, crosshair expansion and camera
    // shake therefore share the same accepted-shot source; the retired Character held-input timer cannot ghost-fire.
    ApplyConfirmedLocalShotRecoil();

    if (AOCTransientVisualFX* Muzzle = GetWorld()->SpawnActor<AOCTransientVisualFX>(
        AOCTransientVisualFX::StaticClass(), Start, Direction.Rotation()))
    {
        Muzzle->ConfigureMuzzle(Start, Direction, FLinearColor(1.0f, 0.58f, 0.12f));
    }

    const float FullLength = FVector::Distance(Start, End);
    if (FullLength > 200.0f)
    {
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
    if (WeaponAudioComponent) WeaponAudioComponent->HandleStateEventLocal(Event, SourceLocation, EventSeed);
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
