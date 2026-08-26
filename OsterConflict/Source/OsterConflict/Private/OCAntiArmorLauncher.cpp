#include "OCAntiArmorLauncher.h"
#include "OCAntiArmorProjectile.h"
#include "OCCharacter.h"
#include "OCWeaponAudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"

AOCAntiArmorLauncher::AOCAntiArmorLauncher()
{
    FOCWeaponTuning T;
    T.WeaponId=FName(TEXT("OC_RPG1")); T.DisplayName=TEXT("OC Anti-Armor Launcher");
    T.WeaponClass=EOCWeaponClass::Launcher; T.ActionType=EOCWeaponActionType::LauncherSingleShot;
    T.PreferredSlot=EOCInventorySlot::Primary; T.AmmoType=EOCAmmoType::Rocket;
    T.Damage=620.0f; T.PelletsPerShot=1; T.RangeCm=18000.0f; T.RoundsPerMinute=18.0f;
    T.HipSpreadDegrees=1.7f; T.ADSSpreadDegrees=0.35f; T.MovingSpreadMultiplier=1.4f;
    T.RecoilPitchMin=4.0f; T.RecoilPitchMax=5.2f; T.RecoilYawMax=1.1f;
    T.MagazineSize=1; T.InitialReserveAmmo=4; T.MaxReserveAmmo=6; T.ReloadDuration=3.8f;
    T.bSupportsSemiAutomatic=true; T.bSupportsAutomatic=false;
    T.bSupersonicAmmo=false;
    T.AudioLoudnessScale=1.20f;
    ConfigureBuiltInTuning(T);
}

void AOCAntiArmorLauncher::BeginPlay()
{
    Super::BeginPlay();

    UStaticMesh* ProductionMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/R13/Weapons/rocketlauncherModern.rocketlauncherModern"));
    if (!ProductionMesh || !WeaponRoot)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_LAUNCHER_PRODUCTION_VISUAL_FAIL OC_RPG1 production launcher mesh unavailable; BasicShape fallback is runtime-rejected."));
        return;
    }

    const FBoxSphereBounds Bounds = ProductionMesh->GetBounds();
    const FVector NativeSize = Bounds.BoxExtent * 2.0f;
    const float NativeLength = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
    if (NativeLength <= 1.0f)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_LAUNCHER_PRODUCTION_VISUAL_FAIL OC_RPG1 production launcher mesh has invalid bounds; BasicShape fallback is runtime-rejected."));
        return;
    }

    UStaticMeshComponent* ProductionVisual = NewObject<UStaticMeshComponent>(
        this,
        MakeUniqueObjectName(this, UStaticMeshComponent::StaticClass(), FName(TEXT("ProductionAntiArmorLauncher"))));
    if (!ProductionVisual)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_LAUNCHER_PRODUCTION_VISUAL_FAIL OC_RPG1 could not create production launcher component; BasicShape fallback is runtime-rejected."));
        return;
    }

    // The imported Kenney source is CC0 and already tracked in Raw/R13. Keep authoritative projectile/damage
    // gameplay unchanged while using the real visual as the only rendered launcher representation.
    constexpr float DesiredLauncherLengthCm = 105.0f;
    const float UniformScale = DesiredLauncherLengthCm / NativeLength;
    ProductionVisual->SetupAttachment(WeaponRoot);
    ProductionVisual->SetStaticMesh(ProductionMesh);
    ProductionVisual->SetRelativeLocation(-Bounds.Origin * UniformScale);
    ProductionVisual->SetRelativeRotation(FRotator::ZeroRotator);
    ProductionVisual->SetRelativeScale3D(FVector(UniformScale));
    ProductionVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ProductionVisual->SetGenerateOverlapEvents(false);
    ProductionVisual->SetCanEverAffectNavigation(false);
    ProductionVisual->SetCastShadow(true);
    ProductionVisual->SetHiddenInGame(false, true);
    ProductionVisual->SetVisibility(true, true);
    ProductionVisual->ComponentTags.Add(FName(TEXT("OC_ProductionWeaponVisual")));
    AddInstanceComponent(ProductionVisual);
    ProductionVisual->RegisterComponent();

    // Hide old BasicShape/source components only after the replacement component exists and is registered.
    TArray<UStaticMeshComponent*> StaticComponents;
    GetComponents<UStaticMeshComponent>(StaticComponents);
    for (UStaticMeshComponent* Component : StaticComponents)
    {
        if (!Component || Component == ProductionVisual) continue;
        Component->SetVisibility(false, true);
        Component->SetHiddenInGame(true, true);
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_LAUNCHER_PRODUCTION_VISUAL_READY OC_RPG1 uses rocketlauncherModern; primitive source visual hidden."));
}

bool AOCAntiArmorLauncher::TryFireServer(AOCCharacter* Shooter, const FVector& TraceOrigin, const FVector& TraceDirection,
    bool, bool, FHitResult& OutHit, bool& bOutDamagedActor, bool& bOutFatalHit)
{
    OutHit=FHitResult(); bOutDamagedActor=false; bOutFatalHit=false;
    if(!HasAuthority()||!Shooter||IsWorldPickup()||AmmoInMagazine<=0||bIsReloading||!GetWorld()) return false;
    const double Now=GetWorld()->GetTimeSeconds();
    if((Now-LastLauncherFireTime)<GetFireInterval()) return false;

    const FVector Dir=TraceDirection.GetSafeNormal();
    if (Dir.IsNearlyZero()) return false;

    // Hit/aim intent still comes from the player's view ray, but the actual projectile, muzzle FX and shot audio
    // must originate at the rendered production weapon. This removes the camera/under-barrel launch artifact.
    const FVector MuzzleOrigin = ResolvePresentationMuzzleOrigin(TraceOrigin, Dir);
    const FVector PresentationEnd = MuzzleOrigin + Dir * FMath::Min(Tuning.RangeCm, 2200.0f);

    FActorSpawnParameters Params;
    Params.Owner=Shooter;
    Params.Instigator=Shooter;
    Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AOCAntiArmorProjectile* Projectile = GetWorld()->SpawnActor<AOCAntiArmorProjectile>(
        AOCAntiArmorProjectile::StaticClass(), MuzzleOrigin + Dir * 8.0f, Dir.Rotation(), Params);
    if (!Projectile)
    {
        UE_LOG(LogTemp, Error, TEXT("PASS45_LAUNCHER_SHOT_FAIL projectile spawn failed; ammo/recoil/audio not committed."));
        return false;
    }

    LastLauncherFireTime=Now;
    --AmmoInMagazine;
    MulticastFireTraceFX(MuzzleOrigin, PresentationEnd, false);

    const EOCAcousticEnvironment AcousticEnvironment = WeaponAudioComponent
        ? WeaponAudioComponent->DetectEnvironmentAt(MuzzleOrigin)
        : EOCAcousticEnvironment::Outdoor;
    MulticastShotAudio(MuzzleOrigin, PresentationEnd, IsSuppressed(), Tuning.bSupersonicAmmo,
        AcousticEnvironment, ++LauncherAudioEventCounter);

    UE_LOG(LogTemp, Verbose,
        TEXT("PASS45_LAUNCHER_CONFIRMED_SHOT muzzle=(%.1f,%.1f,%.1f) ammo=%d audio_event=%d"),
        MuzzleOrigin.X, MuzzleOrigin.Y, MuzzleOrigin.Z, AmmoInMagazine, LauncherAudioEventCounter);
    return true;
}
