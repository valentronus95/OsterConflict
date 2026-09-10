#include "OCAntiArmorLauncher.h"
#include "OCAntiArmorProjectile.h"
#include "OCCharacter.h"
#include "OCWeaponAudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StaticMesh.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr const TCHAR* ProductionLauncherPath = TEXT("/Game/R13/Weapons/rocketlauncherModern.rocketlauncherModern");
    constexpr const TCHAR* ForcedCategoryPrefix = TEXT("OC_FORCE_WEAPON_CATEGORY_");

    bool HasForcedLocalInboxVisual(const AOCAntiArmorLauncher* Launcher)
    {
        if (!Launcher) return false;
        for (const FName& Tag : Launcher->Tags)
        {
            if (Tag.ToString().StartsWith(ForcedCategoryPrefix, ESearchCase::CaseSensitive)) return true;
        }
        return false;
    }
}

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

    // Pass45 fail-closed visual rule: the source BasicShape launcher is collision/debug history only.
    // Hide it immediately, then preload the exact production mesh asynchronously. BeginPlay never loads a package.
    TArray<UStaticMeshComponent*> SourceStaticComponents;
    GetComponents<UStaticMeshComponent>(SourceStaticComponents);
    for (UStaticMeshComponent* Component : SourceStaticComponents)
    {
        if (!Component) continue;
        Component->SetVisibility(false, true);
        Component->SetHiddenInGame(true, true);
        Component->SetCastShadow(false);
        Component->SetCanEverAffectNavigation(false);
    }

    // Sandbox rack variants are explicitly owned by OCLocalInboxWeaponOverrideSubsystem. Do not also attach
    // rocketlauncherModern to the same actor or one launcher becomes two overlapping launcher models.
    if (HasForcedLocalInboxVisual(this))
    {
        UE_LOG(LogTemp, Display,
            TEXT("GAME_RECOVERY_LAUNCHER_FORCED_VISUAL_OWNER_READY weapon=OC_RPG1 builtin_visual=0 local_inbox_visual=1 duplicate_visual=0"));
        return;
    }

    ProductionVisualPreloadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        FSoftObjectPath(ProductionLauncherPath),
        FStreamableDelegate::CreateUObject(this, &AOCAntiArmorLauncher::CompleteProductionVisualPreload));

    if (!ProductionVisualPreloadHandle.IsValid())
    {
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_LAUNCHER_PRELOAD_GAP weapon=OC_RPG1 reason=invalid_handle primitive_visible=0 sync_load=0 runtime_acceptance=0"));
        return;
    }

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_LAUNCHER_PRELOAD_BEGIN weapon=OC_RPG1 async=1 primitive_visible=0 sync_load=0 resident_until_end_play=1"));
}

void AOCAntiArmorLauncher::CompleteProductionVisualPreload()
{
    // Guard the async completion too. A forced category may have been assigned after spawn by compatibility code.
    if (HasForcedLocalInboxVisual(this))
    {
        UE_LOG(LogTemp, Display,
            TEXT("GAME_RECOVERY_LAUNCHER_BUILTIN_VISUAL_SKIPPED weapon=OC_RPG1 reason=forced_local_inbox_owner duplicate_visual=0"));
        return;
    }

    UStaticMesh* ProductionMesh = Cast<UStaticMesh>(FSoftObjectPath(ProductionLauncherPath).ResolveObject());
    if (!ProductionMesh || !WeaponRoot)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_LAUNCHER_PRODUCTION_VISUAL_FAIL weapon=OC_RPG1 reason=mesh_or_visual_root_missing primitive_visible=0 sync_load=0 runtime_acceptance=0"));
        return;
    }

    const FBoxSphereBounds Bounds = ProductionMesh->GetBounds();
    const FVector NativeSize = Bounds.BoxExtent * 2.0f;
    const float NativeLength = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
    if (NativeLength <= 1.0f)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_LAUNCHER_PRODUCTION_VISUAL_FAIL weapon=OC_RPG1 reason=invalid_bounds primitive_visible=0 sync_load=0 runtime_acceptance=0"));
        return;
    }

    UStaticMeshComponent* ProductionVisual = NewObject<UStaticMeshComponent>(
        this,
        MakeUniqueObjectName(this, UStaticMeshComponent::StaticClass(), FName(TEXT("ProductionAntiArmorLauncher"))));
    if (!ProductionVisual)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_LAUNCHER_PRODUCTION_VISUAL_FAIL weapon=OC_RPG1 reason=component_allocation_failed primitive_visible=0 sync_load=0 runtime_acceptance=0"));
        return;
    }

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

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_LAUNCHER_PRODUCTION_VISUAL_READY weapon=OC_RPG1 asset=rocketlauncherModern primitive_visible=0 production_visual=1 async_preloaded=1 resident_asset=1 sync_load=0"));
}

void AOCAntiArmorLauncher::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ProductionVisualPreloadHandle.IsValid())
    {
        ProductionVisualPreloadHandle->CancelHandle();
        ProductionVisualPreloadHandle.Reset();
    }
    Super::EndPlay(EndPlayReason);
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
