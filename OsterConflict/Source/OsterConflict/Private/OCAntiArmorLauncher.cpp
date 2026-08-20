#include "OCAntiArmorLauncher.h"
#include "OCAntiArmorProjectile.h"
#include "OCCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"

AOCAntiArmorLauncher::AOCAntiArmorLauncher()
{
    FOCWeaponTuning T;
    T.WeaponId=FName(TEXT("OC_RPG1")); T.DisplayName=TEXT("OC Anti-Armor Launcher");
    T.WeaponClass=EOCWeaponClass::Launcher; T.PreferredSlot=EOCInventorySlot::Primary; T.AmmoType=EOCAmmoType::Rocket;
    T.Damage=620.0f; T.PelletsPerShot=1; T.RangeCm=18000.0f; T.RoundsPerMinute=18.0f;
    T.HipSpreadDegrees=1.7f; T.ADSSpreadDegrees=0.35f; T.MovingSpreadMultiplier=1.4f;
    T.RecoilPitchMin=4.0f; T.RecoilPitchMax=5.2f; T.RecoilYawMax=1.1f;
    T.MagazineSize=1; T.InitialReserveAmmo=4; T.MaxReserveAmmo=6; T.ReloadDuration=3.8f;
    T.bSupportsSemiAutomatic=true; T.bSupportsAutomatic=false;
    ConfigureBuiltInTuning(T);
}

void AOCAntiArmorLauncher::BeginPlay()
{
    Super::BeginPlay();

    UStaticMesh* ProductionMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/R13/Weapons/rocketlauncherModern.rocketlauncherModern"));
    if (!ProductionMesh || !WeaponRoot)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("OC_RPG1 production launcher mesh unavailable; keeping source-only fallback visual."));
        return;
    }

    const FBoxSphereBounds Bounds = ProductionMesh->GetBounds();
    const FVector NativeSize = Bounds.BoxExtent * 2.0f;
    const float NativeLength = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
    if (NativeLength <= 1.0f)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("OC_RPG1 production launcher mesh has invalid bounds; keeping source-only fallback visual."));
        return;
    }

    // The imported Kenney source is CC0 and already tracked in Raw/R13. Keep the gameplay actor,
    // projectile and authoritative damage logic unchanged; only replace the source-only visual.
    TArray<UStaticMeshComponent*> StaticComponents;
    GetComponents<UStaticMeshComponent>(StaticComponents);
    for (UStaticMeshComponent* Component : StaticComponents)
    {
        if (!Component) continue;
        Component->SetVisibility(false, true);
        Component->SetHiddenInGame(true, true);
    }

    UStaticMeshComponent* ProductionVisual = NewObject<UStaticMeshComponent>(
        this,
        MakeUniqueObjectName(this, UStaticMeshComponent::StaticClass(), FName(TEXT("ProductionAntiArmorLauncher"))));
    if (!ProductionVisual)
    {
        UE_LOG(LogTemp, Warning, TEXT("OC_RPG1 could not create production launcher component."));
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
        TEXT("OC_RPG1 now uses Kenney CC0 rocketlauncherModern production visual; first-person grip remains R14 visual-calibration pending."));
}

bool AOCAntiArmorLauncher::TryFireServer(AOCCharacter* Shooter, const FVector& TraceOrigin, const FVector& TraceDirection,
    bool, bool, FHitResult& OutHit, bool& bOutDamagedActor, bool& bOutFatalHit)
{
    OutHit=FHitResult(); bOutDamagedActor=false; bOutFatalHit=false;
    if(!HasAuthority()||!Shooter||IsWorldPickup()||AmmoInMagazine<=0||bIsReloading) return false;
    const double Now=GetWorld()->GetTimeSeconds();
    if((Now-LastLauncherFireTime)<GetFireInterval()) return false;
    LastLauncherFireTime=Now; --AmmoInMagazine;
    FActorSpawnParameters Params; Params.Owner=Shooter; Params.Instigator=Shooter; Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    const FVector Dir=TraceDirection.GetSafeNormal();
    GetWorld()->SpawnActor<AOCAntiArmorProjectile>(AOCAntiArmorProjectile::StaticClass(), TraceOrigin+Dir*90.0f, Dir.Rotation(), Params);
    MulticastFireTraceFX(TraceOrigin, TraceOrigin+Dir*220.0f, false);
    return true;
}
