#include "OCGrenadeProjectile.h"

#include "OCCharacter.h"
#include "OCCombatVisualComponent.h"
#include "OCHealthComponent.h"
#include "OCDamageTypes.h"
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
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float Pass45GrenadeDesiredLengthCm = 14.0f;
    const TCHAR* Pass45GrenadeVisualPath = TEXT("/Game/R13/Weapons/grenade.grenade");
    const TCHAR* Pass45FragExplosionVFXPath =
        TEXT("/Game/Fire_EXP_Vol01_Free/Niagara/EXP/NS_Sub_EXP_Small_002.NS_Sub_EXP_Small_002");
}

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

    // Pass45: collision stays primitive and invisible, but no Engine BasicShape is ever accepted as the grenade body.
    GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
    GrenadeMesh->SetupAttachment(Collision);
    GrenadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GrenadeMesh->SetGenerateOverlapEvents(false);
    GrenadeMesh->SetCanEverAffectNavigation(false);
    GrenadeMesh->SetCastShadow(true);
    GrenadeMesh->SetVisibility(false, true);
    GrenadeMesh->SetHiddenInGame(true, true);

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

    // The repository already contains a tracked R13 grenade mesh. Use it fail-closed: if it cannot be loaded,
    // gameplay/collision may continue for diagnosis but the rejected Engine sphere never becomes visible.
    UStaticMesh* ProductionMesh = LoadObject<UStaticMesh>(nullptr, Pass45GrenadeVisualPath);
    if (!ProductionMesh || !GrenadeMesh)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_GRENADE_PRODUCTION_VISUAL_FAIL asset=%s primitive_visible=0 gameplay_collision_preserved=1 runtime_acceptance=0"),
            Pass45GrenadeVisualPath);
    }
    else
    {
        const FBoxSphereBounds Bounds = ProductionMesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        const float NativeLength = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
        if (NativeLength <= 1.0f)
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS45_GRENADE_PRODUCTION_VISUAL_FAIL asset=%s invalid_bounds=1 primitive_visible=0 runtime_acceptance=0"),
                Pass45GrenadeVisualPath);
        }
        else
        {
            const float UniformScale = Pass45GrenadeDesiredLengthCm / NativeLength;
            GrenadeMesh->SetStaticMesh(ProductionMesh);
            GrenadeMesh->SetRelativeLocation(-Bounds.Origin * UniformScale);
            GrenadeMesh->SetRelativeRotation(FRotator::ZeroRotator);
            GrenadeMesh->SetRelativeScale3D(FVector(UniformScale));
            GrenadeMesh->ComponentTags.AddUnique(FName(TEXT("OC_ProductionGrenadeVisual")));
            GrenadeMesh->SetHiddenInGame(false, true);
            GrenadeMesh->SetVisibility(true, true);
            UE_LOG(LogTemp, Display,
                TEXT("PASS45_GRENADE_PRODUCTION_VISUAL_READY asset=%s primitive_visible=0 production_visual=1 shared_generic_body=1 type_specific_content_gap=1"),
                Pass45GrenadeVisualPath);
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

void AOCGrenadeProjectile::MulticastDetonationVFX_Implementation(EOCGrenadeType Type, FVector_NetQuantize Location)
{
    if (GetNetMode() == NM_DedicatedServer || Type != EOCGrenadeType::Fragmentation) return;

    UNiagaraSystem* ExplosionSystem = LoadObject<UNiagaraSystem>(nullptr, Pass45FragExplosionVFXPath);
    if (!ExplosionSystem)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_FRAG_EXPLOSION_VFX_LOAD_FAIL asset=%s authored_niagara=0 runtime_acceptance=0"),
            Pass45FragExplosionVFXPath);
        return;
    }

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        this,
        ExplosionSystem,
        Location,
        FRotator::ZeroRotator,
        FVector::OneVector,
        true,
        true,
        ENCPoolMethod::AutoRelease,
        true);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_FRAG_EXPLOSION_VFX_DONOR_WIRED asset=%s authored_niagara=1 replicated_presentation=1 runtime_acceptance=0"),
        Pass45FragExplosionVFXPath);
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
        MulticastDetonationVFX(GrenadeType, GetActorLocation());

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
            const bool bBlocked = GetWorld()->LineTraceSingleByChannel(LOSHit, GetActorLocation(), TargetPoint, ECC_Visibility, LOSParams);
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
        if (!GetWorld()->SpawnActor<AOCSmokeCloud>(AOCSmokeCloud::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, Params))
        {
            UE_LOG(LogTemp, Error, TEXT("PASS45_SMOKE_GAMEPLAY_VOLUME_FAIL spawn_failed=1 runtime_acceptance=0"));
        }
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
