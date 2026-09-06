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
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"

namespace
{
    constexpr float Pass45GrenadeDesiredLengthCm = 14.0f;
    constexpr float Pass45FragExplosionVisualScale = 1.85f;
    constexpr float Pass45FragExplosionCleanupSeconds = 2.80f;
    const TCHAR* Pass45GrenadeVisualPath = TEXT("/Game/R13/Weapons/grenade.grenade");
    const TCHAR* Pass45FragIdentityMaterialPath = TEXT("/Game/R13/Weapons/green.green");
    const TCHAR* Pass45SmokeIdentityMaterialPath = TEXT("/Game/R13/Weapons/greyLight.greyLight");
    const TCHAR* Pass45FlashIdentityMaterialPath = TEXT("/Game/R13/Weapons/sand.sand");
    const TCHAR* Pass45FragExplosionVFXPath =
        TEXT("/Game/Fire_EXP_Vol01_Free/Niagara/EXP/NS_Sub_EXP_Small_002.NS_Sub_EXP_Small_002");

    const TCHAR* GetPass45GrenadeIdentityMaterialPath(EOCGrenadeType Type)
    {
        switch (Type)
        {
            case EOCGrenadeType::Fragmentation: return Pass45FragIdentityMaterialPath;
            case EOCGrenadeType::Smoke: return Pass45SmokeIdentityMaterialPath;
            case EOCGrenadeType::Flash: return Pass45FlashIdentityMaterialPath;
            default: return Pass45FragIdentityMaterialPath;
        }
    }
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

    // GAME_RECOVERY grenade assets are async-preloaded by the world subsystem before deployment releases the player.
    // Refresh is therefore lookup-only and never allowed to turn the first throw into a synchronous package load.
    RefreshGrenadePresentation();

    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(FuseTimerHandle, this, &AOCGrenadeProjectile::DetonateServer, FuseSeconds, false);
    }
}

void AOCGrenadeProjectile::RefreshGrenadePresentation()
{
    if (GetNetMode() == NM_DedicatedServer) return;

    if (!GrenadeMesh)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_GRENADE_PRODUCTION_VISUAL_FAIL asset=%s grenade_mesh_component=0 primitive_visible=0 gameplay_collision_preserved=1 runtime_acceptance=0"),
            Pass45GrenadeVisualPath);
        return;
    }

    GrenadeMesh->SetVisibility(false, true);
    GrenadeMesh->SetHiddenInGame(true, true);

    // No LoadObject here. Missing residency is fail-visible instead of blocking the game thread on first use.
    UStaticMesh* ProductionMesh = Cast<UStaticMesh>(FSoftObjectPath(Pass45GrenadeVisualPath).ResolveObject());
    if (!ProductionMesh)
    {
        GrenadeMesh->SetStaticMesh(nullptr);
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_GRENADE_PRELOAD_MISS asset=%s type=%d phase=throw_visual sync_package_loads=0 primitive_visible=0 runtime_acceptance=0"),
            Pass45GrenadeVisualPath, static_cast<int32>(GrenadeType));
        return;
    }

    const FBoxSphereBounds Bounds = ProductionMesh->GetBounds();
    const FVector NativeSize = Bounds.BoxExtent * 2.0f;
    const float NativeLength = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
    if (NativeLength <= 1.0f)
    {
        GrenadeMesh->SetStaticMesh(nullptr);
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_GRENADE_PRODUCTION_VISUAL_FAIL asset=%s type=%d invalid_bounds=1 primitive_visible=0 runtime_acceptance=0"),
            Pass45GrenadeVisualPath, static_cast<int32>(GrenadeType));
        return;
    }

    const float UniformScale = Pass45GrenadeDesiredLengthCm / NativeLength;
    GrenadeMesh->SetStaticMesh(ProductionMesh);
    GrenadeMesh->EmptyOverrideMaterials();
    GrenadeMesh->SetRelativeLocation(-Bounds.Origin * UniformScale);
    GrenadeMesh->SetRelativeRotation(FRotator::ZeroRotator);
    GrenadeMesh->SetRelativeScale3D(FVector(UniformScale));
    GrenadeMesh->ComponentTags.AddUnique(FName(TEXT("OC_ProductionGrenadeVisual")));

    const TCHAR* IdentityMaterialPath = GetPass45GrenadeIdentityMaterialPath(GrenadeType);
    UMaterialInterface* IdentityMaterial =
        Cast<UMaterialInterface>(FSoftObjectPath(IdentityMaterialPath).ResolveObject());
    const int32 MaterialSlotCount = ProductionMesh->GetStaticMaterials().Num();
    const bool bTypeIdentityMaterialReady = IdentityMaterial != nullptr && MaterialSlotCount > 0;
    if (bTypeIdentityMaterialReady)
    {
        for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotCount; ++MaterialIndex)
        {
            GrenadeMesh->SetMaterial(MaterialIndex, IdentityMaterial);
        }
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_GRENADE_TYPE_IDENTITY_MATERIAL_READY material=%s type=%d authored_material=1 shared_generic_body=1 exact_type_body=0 type_distinguishable=1 runtime_acceptance=0"),
            IdentityMaterialPath, static_cast<int32>(GrenadeType));
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_GRENADE_PRELOAD_MISS asset=%s type=%d phase=identity_material material_slots=%d sync_package_loads=0 runtime_acceptance=0"),
            IdentityMaterialPath, static_cast<int32>(GrenadeType), MaterialSlotCount);
    }

    GrenadeMesh->SetHiddenInGame(false, true);
    GrenadeMesh->SetVisibility(true, true);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_GRENADE_PRODUCTION_VISUAL_READY asset=%s type=%d primitive_visible=0 production_visual=1 shared_generic_body=1 type_identity_material=%d type_distinguishable=%d exact_type_body=0 type_specific_content_gap=1 runtime_acceptance=0"),
        Pass45GrenadeVisualPath, static_cast<int32>(GrenadeType), bTypeIdentityMaterialReady ? 1 : 0, bTypeIdentityMaterialReady ? 1 : 0);
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_GRENADE_PRESENTATION_TYPE_REFRESH type=%d replicated_type_refresh=1 shared_generic_body=1 type_identity_material=%d type_distinguishable=%d exact_type_body=0 type_specific_content_gap=1 sync_package_loads=0 runtime_acceptance=0"),
        static_cast<int32>(GrenadeType), bTypeIdentityMaterialReady ? 1 : 0, bTypeIdentityMaterialReady ? 1 : 0);
}

void AOCGrenadeProjectile::OnRep_GrenadeType()
{
    RefreshGrenadePresentation();
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
    RefreshGrenadePresentation();
    if (ProjectileMovement) ProjectileMovement->Velocity = InitialVelocity;
    ForceNetUpdate();
}

void AOCGrenadeProjectile::MulticastDetonationVFX_Implementation(EOCGrenadeType Type, FVector_NetQuantize Location)
{
    if (GetNetMode() == NM_DedicatedServer || Type != EOCGrenadeType::Fragmentation) return;

    // The Niagara package is retained by the pre-spawn preload handle. ResolveObject is lookup-only.
    UNiagaraSystem* ExplosionSystem = Cast<UNiagaraSystem>(FSoftObjectPath(Pass45FragExplosionVFXPath).ResolveObject());
    if (!ExplosionSystem)
    {
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_GRENADE_PRELOAD_MISS asset=%s phase=detonation_vfx sync_package_loads=0 runtime_acceptance=0"),
            Pass45FragExplosionVFXPath);
        return;
    }

    UNiagaraComponent* ExplosionComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        this,
        ExplosionSystem,
        Location,
        FRotator::ZeroRotator,
        FVector(Pass45FragExplosionVisualScale),
        true,
        true,
        ENCPoolMethod::None,
        true);

    if (ExplosionComponent && GetWorld())
    {
        TWeakObjectPtr<UNiagaraComponent> WeakExplosion(ExplosionComponent);
        FTimerHandle CleanupHandle;
        GetWorldTimerManager().SetTimer(
            CleanupHandle,
            FTimerDelegate::CreateLambda([WeakExplosion]()
            {
                if (UNiagaraComponent* Component = WeakExplosion.Get())
                {
                    Component->DeactivateImmediate();
                    Component->DestroyComponent();
                }
            }),
            Pass45FragExplosionCleanupSeconds,
            false);
    }

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_FRAG_EXPLOSION_VFX_READY asset=%s authored_niagara=1 replicated_presentation=1 visual_scale=%.2f forced_cleanup_s=%.2f looping_residue=0 sync_package_loads=0 runtime_acceptance=0"),
        Pass45FragExplosionVFXPath, Pass45FragExplosionVisualScale, Pass45FragExplosionCleanupSeconds);
}

void AOCGrenadeProjectile::DetonateServer()
{
    if (!HasAuthority() || bDetonated) return;
    bDetonated = true;
    GetWorldTimerManager().ClearTimer(FuseTimerHandle);

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_GRENADE_DETONATION_COMMIT type=%d one_explosion_event=1 duplicate_guard=1"),
        static_cast<int32>(GrenadeType));

    if (WorldAudioComponent && GrenadeType != EOCGrenadeType::Smoke)
    {
        const EOCWorldAudioEvent AudioEvent = GrenadeType == EOCGrenadeType::Fragmentation
            ? EOCWorldAudioEvent::ExplosionLarge
            : EOCWorldAudioEvent::ExplosionSmall;
        WorldAudioComponent->PlayEventServer(AudioEvent, GetActorLocation());
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