#include "OCGrenadeProjectile.h"

#include "OCCharacter.h"
#include "OCCombatVisualComponent.h"
#include "OCHealthComponent.h"
#include "OCDamageTypes.h"
#include "OCAudioUserSettings.h"
#include "OCSmokeCloud.h"
#include "OCWorldAudioComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AOCGrenadeProjectile::AOCGrenadeProjectile()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);
    WorldAudioComponent = CreateDefaultSubobject<UOCWorldAudioComponent>(TEXT("WorldAudioComponent"));

    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    Collision->InitSphereRadius(6.5f);
    Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    SetRootComponent(Collision);

    GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
    GrenadeMesh->SetupAttachment(Collision);
    GrenadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded()) GrenadeMesh->SetStaticMesh(SphereMesh.Object);
    GrenadeMesh->SetRelativeScale3D(FVector(0.13f, 0.13f, 0.19f));

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = Collision;
    ProjectileMovement->InitialSpeed = 1350.0f;
    ProjectileMovement->MaxSpeed = 1600.0f;
    ProjectileMovement->ProjectileGravityScale = 1.0f;
    ProjectileMovement->bShouldBounce = true;
    ProjectileMovement->Bounciness = 0.32f;
    ProjectileMovement->Friction = 0.62f;
}

void AOCGrenadeProjectile::BeginPlay()
{
    Super::BeginPlay();

    // R13 imported art bridge. Gameplay collision remains the small replicated sphere; only the visible mesh changes.
    if (GrenadeMesh)
    {
        if (UStaticMesh* ImportedGrenade = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/R13/Weapons/grenade.grenade")))
        {
            GrenadeMesh->SetStaticMesh(ImportedGrenade);
            GrenadeMesh->SetRelativeLocation(FVector::ZeroVector);
            GrenadeMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 90.0f));
            GrenadeMesh->SetRelativeScale3D(FVector(100.0f));
        }
    }

    // R13 practical feedback. The throw existed before, but without a sound it was easy to miss among placeholder art.
    if (GetWorld() && GetWorld()->GetNetMode() != NM_DedicatedServer)
    {
        if (USoundBase* ThrowSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/R13/Audio/snd_throw1.snd_throw1")))
        {
            const float Bus = UOCAudioUserSettings::Get()->GetBusVolume(EOCAudioBus::WorldSFX);
            if (Bus > 0.0f)
            {
                UGameplayStatics::PlaySoundAtLocation(this, ThrowSound, GetActorLocation(),
                    FMath::Clamp(Bus * 0.55f, 0.0f, 1.0f));
            }
        }
    }

    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(FuseTimerHandle, this, &AOCGrenadeProjectile::DetonateServer, FuseSeconds, false);
    }
}

void AOCGrenadeProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOCGrenadeProjectile, GrenadeType);
}

void AOCGrenadeProjectile::InitializeGrenadeServer(EOCGrenadeType NewType, const FVector& InitialVelocity)
{
    if (!HasAuthority()) return;
    GrenadeType = NewType;
    if (ProjectileMovement) ProjectileMovement->Velocity = InitialVelocity;
    ForceNetUpdate();
}

void AOCGrenadeProjectile::DetonateServer()
{
    if (!HasAuthority()) return;
    if (WorldAudioComponent && GrenadeType != EOCGrenadeType::Smoke)
    {
        WorldAudioComponent->PlayEventServer(EOCWorldAudioEvent::ExplosionSmall, GetActorLocation());
    }

    if (GrenadeType == EOCGrenadeType::Fragmentation)
    {
        TArray<TWeakObjectPtr<AOCCharacter>> VisualTargets;
        for (TActorIterator<AOCCharacter> It(GetWorld()); It; ++It)
        {
            AOCCharacter* Target = *It;
            if (!Target || !Target->GetHealthComponent() || Target->GetHealthComponent()->IsDead()) continue;
            if (FVector::Dist(Target->GetActorLocation(), GetActorLocation()) <= FragOuterRadius) VisualTargets.Add(Target);
        }

        UGameplayStatics::ApplyRadialDamageWithFalloff(
            this, FragBaseDamage, FragMinDamage, GetActorLocation(), FragInnerRadius, FragOuterRadius,
            1.0f, UOCExplosiveDamageType::StaticClass(), TArray<AActor*>(), this, GetInstigatorController(), ECC_Visibility);
        ApplyBoundedPhysicsImpulseServer(FragOuterRadius, FragPhysicsImpulse);

        for (const TWeakObjectPtr<AOCCharacter>& WeakTarget : VisualTargets)
        {
            AOCCharacter* Target = WeakTarget.Get();
            if (!Target || !Target->GetHealthComponent()) continue;

            FHitResult LOSHit;
            FCollisionQueryParams LOSParams(SCENE_QUERY_STAT(OCFragVisualLOS), false, this);
            LOSParams.AddIgnoredActor(this);
            const FVector TargetPoint = Target->GetActorLocation() + FVector(0,0,45);
            const bool bBlocked = GetWorld()->LineTraceSingleByChannel(LOSHit, GetActorLocation(), TargetPoint, ECC_Visibility);
            if (bBlocked && LOSHit.GetActor() != Target) continue;

            const float Distance = FVector::Dist(Target->GetActorLocation(), GetActorLocation());
            const float Alpha = FragOuterRadius > FragInnerRadius
                ? FMath::Clamp((Distance - FragInnerRadius) / (FragOuterRadius - FragInnerRadius), 0.0f, 1.0f)
                : 1.0f;
            const float EstimatedAppliedDamage = FMath::Lerp(FragBaseDamage, FragMinDamage, Alpha);
            if (UOCCombatVisualComponent* Trauma = Target->GetCombatVisualComponent())
            {
                Trauma->RecordRadialTraumaServer(EstimatedAppliedDamage, GetActorLocation(),
                    UOCExplosiveDamageType::StaticClass(), Target->GetHealthComponent()->IsDead());
            }
        }
    }
    else if (GrenadeType == EOCGrenadeType::Smoke)
    {
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        GetWorld()->SpawnActor<AOCSmokeCloud>(AOCSmokeCloud::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, Params);
    }
    else
    {
        ApplyFlashServer();
    }

    Destroy();
}

void AOCGrenadeProjectile::ApplyBoundedPhysicsImpulseServer(float Radius, float Strength)
{
    if (!HasAuthority() || !GetWorld() || Radius <= 0.0f || Strength <= 0.0f) return;
    const FVector Origin = GetActorLocation();
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor == this) continue;
        TInlineComponentArray<UPrimitiveComponent*> Components(Actor);
        for (UPrimitiveComponent* Component : Components)
        {
            if (!Component || !Component->IsSimulatingPhysics()) continue;
            const FVector Location = Component->GetComponentLocation();
            const float Distance = FVector::Dist(Location, Origin);
            if (Distance > Radius || Component->GetMass() > MaxImpulseBodyMassKg) continue;

            FHitResult OcclusionHit;
            FCollisionQueryParams OcclusionParams(SCENE_QUERY_STAT(OCFragImpulseLOS), false, this);
            OcclusionParams.AddIgnoredActor(this);
            const bool bOccluded = GetWorld()->LineTraceSingleByChannel(OcclusionHit, Origin, Location, ECC_Visibility, OcclusionParams);
            if (bOccluded && OcclusionHit.GetActor() != Actor) continue;

            const float Alpha = 1.0f - FMath::Clamp(Distance / Radius, 0.0f, 1.0f);
            const FVector Direction = (Location - Origin).GetSafeNormal();
            Component->AddImpulseAtLocation(Direction * Strength * Alpha, Location, NAME_None);
        }
    }
}

void AOCGrenadeProjectile::ApplyFlashServer()
{
    for (TActorIterator<AOCCharacter> It(GetWorld()); It; ++It)
    {
        AOCCharacter* Target = *It;
        if (!Target || !Target->GetHealthComponent() || Target->GetHealthComponent()->IsDead()) continue;
        const float Distance = FVector::Dist(Target->GetActorLocation(), GetActorLocation());
        if (Distance > FlashRadius) continue;

        const FVector Eye = Target->GetFirstPersonCamera() ? Target->GetFirstPersonCamera()->GetComponentLocation() : Target->GetActorLocation();
        FHitResult Hit;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(OCFlashLOS), false, this);
        Params.AddIgnoredActor(Target);
        const bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(), Eye, ECC_Visibility, Params);
        if (bBlocked && Hit.GetActor() != Target) continue;

        const FVector ToFlash = (GetActorLocation() - Eye).GetSafeNormal();
        const FVector ViewForward = Target->GetFirstPersonCamera() ? Target->GetFirstPersonCamera()->GetForwardVector() : Target->GetActorForwardVector();
        const float Facing = FMath::Clamp((FVector::DotProduct(ViewForward, ToFlash) + 0.35f) / 1.35f, 0.0f, 1.0f);
        const float DistanceFactor = 1.0f - FMath::Clamp(Distance / FlashRadius, 0.0f, 1.0f);
        const float Intensity = FMath::Clamp(DistanceFactor * (0.35f + 0.65f * Facing), 0.0f, 1.0f);
        if (Intensity > 0.05f) Target->ApplyFlashEffectServer(Intensity, 1.2f + Intensity * 3.2f);
    }
}
