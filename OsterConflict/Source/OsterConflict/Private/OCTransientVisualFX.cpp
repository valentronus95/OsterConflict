#include "OCTransientVisualFX.h"

#include "OCCharacter.h"
#include "OCWeaponBase.h"
#include "Components/PointLightComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    const FName ProductionVisualTag(TEXT("OC_ProductionWeaponVisual"));

    bool TryResolveSocketMuzzle(const UPrimitiveComponent& Component, FVector& OutMuzzle)
    {
        static const FName SocketNames[] =
        {
            TEXT("Muzzle"), TEXT("muzzle"), TEXT("MuzzleFlash"), TEXT("Muzzle_Flash"),
            TEXT("Muzzle_01"), TEXT("MuzzleSocket"), TEXT("BarrelEnd"), TEXT("barrel_end")
        };
        for (const FName Socket : SocketNames)
        {
            if (Component.DoesSocketExist(Socket))
            {
                OutMuzzle = Component.GetSocketLocation(Socket);
                return true;
            }
        }
        return false;
    }

    bool TryResolveBoundsMuzzle(const UPrimitiveComponent& Component, const FVector& ShotDirection, FVector& OutMuzzle)
    {
        const FBoxSphereBounds LocalBounds = Component.GetLocalBounds();
        const FVector LocalMin = LocalBounds.Origin - LocalBounds.BoxExtent;
        const FVector LocalMax = LocalBounds.Origin + LocalBounds.BoxExtent;
        if (LocalMin.ContainsNaN() || LocalMax.ContainsNaN()) return false;

        const FVector LocalSize = LocalMax - LocalMin;
        if (LocalSize.GetAbsMax() <= 1.0f) return false;

        const FTransform Transform = Component.GetComponentTransform();
        const FVector LocalShot = Transform.InverseTransformVectorNoScale(ShotDirection).GetSafeNormal();
        if (LocalShot.IsNearlyZero()) return false;

        const FVector Center = (LocalMin + LocalMax) * 0.5f;
        const FVector Extent = (LocalMax - LocalMin) * 0.5f;
        FVector LocalMuzzle = Center;

        const FVector AbsShot(FMath::Abs(LocalShot.X), FMath::Abs(LocalShot.Y), FMath::Abs(LocalShot.Z));
        if (AbsShot.X >= AbsShot.Y && AbsShot.X >= AbsShot.Z)
        {
            LocalMuzzle.X = LocalShot.X >= 0.0f ? LocalMax.X : LocalMin.X;
        }
        else if (AbsShot.Y >= AbsShot.Z)
        {
            LocalMuzzle.Y = LocalShot.Y >= 0.0f ? LocalMax.Y : LocalMin.Y;
        }
        else
        {
            LocalMuzzle.Z = LocalShot.Z >= 0.0f ? LocalMax.Z : LocalMin.Z;
        }

        // Imported weapon axes differ between packs. Bias the inferred barrel end toward world-up in
        // component-local space instead of assuming local Z is always the top of the gun.
        const FVector LocalUp = Transform.InverseTransformVectorNoScale(FVector::UpVector).GetSafeNormal();
        const float UpExtent =
            FMath::Abs(LocalUp.X) * Extent.X +
            FMath::Abs(LocalUp.Y) * Extent.Y +
            FMath::Abs(LocalUp.Z) * Extent.Z;
        LocalMuzzle += LocalUp * (UpExtent * 0.38f);
        LocalMuzzle.X = FMath::Clamp(LocalMuzzle.X, LocalMin.X, LocalMax.X);
        LocalMuzzle.Y = FMath::Clamp(LocalMuzzle.Y, LocalMin.Y, LocalMax.Y);
        LocalMuzzle.Z = FMath::Clamp(LocalMuzzle.Z, LocalMin.Z, LocalMax.Z);

        OutMuzzle = Transform.TransformPosition(LocalMuzzle);
        return true;
    }

    bool IsOnAimRay(const FVector& ViewLocation, const FVector& Point, const FVector& Direction,
        float& OutPerpendicularDistanceSquared)
    {
        const FVector ViewToPoint = Point - ViewLocation;
        const float Along = FVector::DotProduct(ViewToPoint, Direction);
        if (Along <= 0.0f) return false;

        const FVector Perpendicular = ViewToPoint - Direction * Along;
        OutPerpendicularDistanceSquared = Perpendicular.SizeSquared();
        return OutPerpendicularDistanceSquared <= FMath::Square(115.0f);
    }

    AOCWeaponBase* ResolveFiringWeapon(UWorld* World, const FVector& NetworkStart, const FVector& Direction)
    {
        if (!World) return nullptr;
        const FVector SafeDirection = Direction.GetSafeNormal();
        if (SafeDirection.IsNearlyZero()) return nullptr;

        AOCWeaponBase* BestWeapon = nullptr;
        float BestScore = TNumericLimits<float>::Max();

        // Multicast muzzle FX uses the camera trace origin; multicast tracer FX uses a short streak
        // near the target. Match either form back to the character whose replicated current weapon
        // owns that aim ray. This works for the local player and remote players instead of rebasing
        // every multicast onto the first local pawn's gun.
        for (TActorIterator<AOCCharacter> It(World); It; ++It)
        {
            AOCCharacter* Character = *It;
            AOCWeaponBase* Weapon = Character ? Character->GetCurrentWeapon() : nullptr;
            if (!Character || !Weapon || Weapon->IsWorldPickup()) continue;

            const FVector ViewLocation = Character->GetPawnViewLocation();
            const float NearDistanceSquared = FVector::DistSquared(ViewLocation, NetworkStart);
            const bool bNearView = NearDistanceSquared <= FMath::Square(110.0f);

            float PerpendicularDistanceSquared = TNumericLimits<float>::Max();
            const bool bOnAimRay = IsOnAimRay(ViewLocation, NetworkStart, SafeDirection, PerpendicularDistanceSquared);
            if (!bNearView && !bOnAimRay) continue;

            const float Score = bNearView ? NearDistanceSquared : (FMath::Square(110.0f) + PerpendicularDistanceSquared);
            if (Score < BestScore)
            {
                BestScore = Score;
                BestWeapon = Weapon;
            }
        }

        return BestWeapon;
    }

    FVector ResolveWeaponMuzzle(UWorld* World, const FVector& NetworkStart, const FVector& Direction)
    {
        if (!World) return NetworkStart;

        const FVector SafeDirection = Direction.GetSafeNormal();
        if (SafeDirection.IsNearlyZero()) return NetworkStart;

        AOCWeaponBase* Weapon = ResolveFiringWeapon(World, NetworkStart, SafeDirection);
        if (!Weapon) return NetworkStart;

        TArray<UPrimitiveComponent*> Components;
        Weapon->GetComponents<UPrimitiveComponent>(Components);

        bool bHasProductionVisual = false;
        for (const UPrimitiveComponent* Component : Components)
        {
            if (Component && Component->ComponentHasTag(ProductionVisualTag) && Component->IsVisible())
            {
                bHasProductionVisual = true;
                break;
            }
        }

        FVector BestPoint = NetworkStart;
        float BestScore = -TNumericLimits<float>::Max();
        const FVector WeaponOrigin = Weapon->GetActorLocation();
        for (const UPrimitiveComponent* Component : Components)
        {
            if (!Component || !Component->IsRegistered() || !Component->IsVisible()) continue;
            if (bHasProductionVisual && !Component->ComponentHasTag(ProductionVisualTag)) continue;

            FVector Candidate;
            const bool bSocket = TryResolveSocketMuzzle(*Component, Candidate);
            if (!bSocket && !TryResolveBoundsMuzzle(*Component, SafeDirection, Candidate)) continue;

            const float DistanceFromTraceOrigin = FVector::Distance(NetworkStart, Candidate);
            // For the camera-origin muzzle call this is a normal first-person offset. For tracer
            // rebasing the candidate can be far from NetworkStart because NetworkStart is target-side.
            if (DistanceFromTraceOrigin < 4.0f && FVector::DistSquared(NetworkStart, WeaponOrigin) < FMath::Square(300.0f))
            {
                continue;
            }

            const FString ComponentName = Component->GetName();
            const bool bNamedBarrel = ComponentName.Contains(TEXT("barrel"), ESearchCase::IgnoreCase) ||
                ComponentName.Contains(TEXT("muzzle"), ESearchCase::IgnoreCase);
            const float ForwardProjection = FVector::DotProduct(Candidate - WeaponOrigin, SafeDirection);
            const float Score = ForwardProjection + (bSocket ? 10000.0f : 0.0f) + (bNamedBarrel ? 1200.0f : 0.0f);
            if (Score > BestScore)
            {
                BestScore = Score;
                BestPoint = Candidate;
            }
        }

        return BestScore > -TNumericLimits<float>::Max() * 0.5f ? BestPoint : NetworkStart;
    }
}

AOCTransientVisualFX::AOCTransientVisualFX()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FXMesh"));
    Mesh->SetupAttachment(SceneRoot);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Mesh->SetGenerateOverlapEvents(false);
    Mesh->SetCastShadow(false);

    PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FXLight"));
    PointLight->SetupAttachment(SceneRoot);
    PointLight->SetMobility(EComponentMobility::Movable);
    PointLight->SetCastShadows(false);
    PointLight->SetVisibility(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        Mesh->SetStaticMesh(SphereMesh.Object);
    }
}

void AOCTransientVisualFX::ApplyColor(const FLinearColor& Color)
{
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!BaseMaterial || !Mesh) return;

    DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
    if (DynamicMaterial)
    {
        DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
        Mesh->SetMaterial(0, DynamicMaterial);
    }
}

void AOCTransientVisualFX::ConfigureTracer(const FVector& Start, const FVector& End, const FLinearColor& Color,
    float RadiusCm, float LifetimeSeconds)
{
    const FVector NetworkDelta = End - Start;
    const FVector NetworkDirection = NetworkDelta.GetSafeNormal();
    const FVector VisualStart = ResolveWeaponMuzzle(GetWorld(), Start, NetworkDirection);
    const bool bRebasedToMuzzle = !VisualStart.Equals(Start, 1.0f);

    FVector VisualEnd = End;
    if (bRebasedToMuzzle)
    {
        // The server multicasts a short target-side streak. Once rebound to the actual firing weapon,
        // keep the rendered streak short rather than drawing a full laser from muzzle to impact.
        const float DistanceToEnd = FVector::Distance(VisualStart, End);
        VisualEnd = VisualStart + NetworkDirection * FMath::Min(DistanceToEnd, 900.0f);
    }

    const FVector Delta = VisualEnd - VisualStart;
    const float Length = Delta.Size();
    if (Length <= 1.0f || !Mesh)
    {
        Destroy();
        return;
    }

    if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
    {
        Mesh->SetStaticMesh(Cylinder);
    }

    const FVector VisualDirection = Delta / Length;
    const FVector StreakStart = VisualStart + VisualDirection * 18.0f;
    const float StreakLength = FVector::Distance(StreakStart, VisualEnd);
    if (StreakLength <= 1.0f)
    {
        Destroy();
        return;
    }

    SetActorLocation((StreakStart + VisualEnd) * 0.5f);
    SetActorRotation(FQuat::FindBetweenNormals(FVector::UpVector, VisualDirection));
    const float ThinRadius = FMath::Clamp(RadiusCm, 0.12f, 0.30f);
    Mesh->SetRelativeScale3D(FVector(ThinRadius / 50.0f, ThinRadius / 50.0f, StreakLength / 100.0f));
    PointLight->SetVisibility(false);
    ApplyColor(Color);
    SetLifeSpan(FMath::Max(0.01f, LifetimeSeconds));
}

void AOCTransientVisualFX::ConfigureImpact(const FVector& Location, const FVector& Normal, const FLinearColor& Color,
    float RadiusCm, float LifetimeSeconds)
{
    if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
    {
        Mesh->SetStaticMesh(Sphere);
    }

    SetActorLocation(Location + Normal.GetSafeNormal() * 2.0f);
    SetActorRotation(FRotator::ZeroRotator);
    Mesh->SetRelativeScale3D(FVector(FMath::Max(1.5f, RadiusCm) / 50.0f));
    ApplyColor(Color);

    PointLight->SetVisibility(true);
    PointLight->SetLightColor(Color);
    PointLight->SetIntensity(900.0f);
    PointLight->SetAttenuationRadius(125.0f);
    SetLifeSpan(FMath::Max(0.03f, LifetimeSeconds));
}

void AOCTransientVisualFX::ConfigureMuzzle(const FVector& Location, const FVector& Direction, const FLinearColor& Color,
    float LifetimeSeconds)
{
    if (UStaticMesh* Cone = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone")))
    {
        Mesh->SetStaticMesh(Cone);
    }
    else if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
    {
        Mesh->SetStaticMesh(Cylinder);
    }

    const FVector SafeDirection = Direction.GetSafeNormal();
    const FVector VisualMuzzle = ResolveWeaponMuzzle(GetWorld(), Location, SafeDirection);
    SetActorLocation(VisualMuzzle + SafeDirection * 7.0f);
    SetActorRotation(FQuat::FindBetweenNormals(FVector::UpVector, SafeDirection));
    Mesh->SetRelativeScale3D(FVector(0.020f, 0.020f, 0.10f));
    ApplyColor(Color);

    PointLight->SetVisibility(true);
    PointLight->SetLightColor(Color);
    PointLight->SetIntensity(2100.0f);
    PointLight->SetAttenuationRadius(180.0f);
    SetLifeSpan(FMath::Max(0.015f, LifetimeSeconds));
}
