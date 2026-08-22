#include "OCTransientVisualFX.h"

#include "OCCharacter.h"
#include "OCWeaponBase.h"
#include "Components/PointLightComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
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
        FVector LocalMin;
        FVector LocalMax;
        Component.GetLocalBounds(LocalMin, LocalMax);
        if (!LocalMin.IsFinite() || !LocalMax.IsFinite()) return false;

        const FVector LocalSize = LocalMax - LocalMin;
        if (LocalSize.GetAbsMax() <= 1.0f) return false;

        const FTransform Transform = Component.GetComponentTransform();
        const FVector LocalShot = Transform.InverseTransformVectorNoScale(ShotDirection).GetSafeNormal();
        if (LocalShot.IsNearlyZero()) return false;

        const FVector Center = (LocalMin + LocalMax) * 0.5f;
        const FVector Extent = (LocalMax - LocalMin) * 0.5f;
        FVector LocalMuzzle = Center;

        // Use the end of the weapon's dominant axis, not the centre of its world AABB. The previous
        // AABB projection placed the FX around the receiver/magazine height, visibly below the barrel.
        const FVector AbsShot(FMath::Abs(LocalShot.X), FMath::Abs(LocalShot.Y), FMath::Abs(LocalShot.Z));
        if (AbsShot.X >= AbsShot.Y && AbsShot.X >= AbsShot.Z)
        {
            LocalMuzzle.X = LocalShot.X >= 0.0f ? LocalMax.X : LocalMin.X;
            LocalMuzzle.Z = Center.Z + Extent.Z * 0.34f;
        }
        else if (AbsShot.Y >= AbsShot.Z)
        {
            LocalMuzzle.Y = LocalShot.Y >= 0.0f ? LocalMax.Y : LocalMin.Y;
            LocalMuzzle.Z = Center.Z + Extent.Z * 0.34f;
        }
        else
        {
            LocalMuzzle.Z = LocalShot.Z >= 0.0f ? LocalMax.Z : LocalMin.Z;
        }

        OutMuzzle = Transform.TransformPosition(LocalMuzzle);
        return true;
    }

    FVector ResolveLocalWeaponMuzzle(UWorld* World, const FVector& NetworkStart, const FVector& Direction)
    {
        if (!World) return NetworkStart;

        APlayerController* PC = World->GetFirstPlayerController();
        AOCCharacter* Character = PC ? Cast<AOCCharacter>(PC->GetPawn()) : nullptr;
        AOCWeaponBase* Weapon = Character ? Character->GetCurrentWeapon() : nullptr;
        if (!PC || !Character || !Weapon || Weapon->IsWorldPickup()) return NetworkStart;

        FVector ViewLocation;
        FRotator ViewRotation;
        PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

        // Network trace start remains camera-authored for hit validation. Rewrite presentation only for
        // the local shooter's current first-person weapon.
        if (FVector::DistSquared(ViewLocation, NetworkStart) > FMath::Square(90.0f)) return NetworkStart;

        const FVector SafeDirection = Direction.GetSafeNormal();
        if (SafeDirection.IsNearlyZero()) return NetworkStart;

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
        float BestProjection = -TNumericLimits<float>::Max();
        for (const UPrimitiveComponent* Component : Components)
        {
            if (!Component || !Component->IsRegistered() || !Component->IsVisible()) continue;
            if (bHasProductionVisual && !Component->ComponentHasTag(ProductionVisualTag)) continue;

            FVector Candidate;
            if (!TryResolveSocketMuzzle(*Component, Candidate) &&
                !TryResolveBoundsMuzzle(*Component, SafeDirection, Candidate))
            {
                continue;
            }

            const float DistanceFromView = FVector::Distance(ViewLocation, Candidate);
            if (DistanceFromView < 20.0f || DistanceFromView > 240.0f) continue;

            const float Projection = FVector::DotProduct(Candidate, SafeDirection);
            if (Projection > BestProjection)
            {
                BestProjection = Projection;
                BestPoint = Candidate;
            }
        }

        return BestProjection > -TNumericLimits<float>::Max() * 0.5f ? BestPoint : NetworkStart;
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
    const FVector VisualStart = ResolveLocalWeaponMuzzle(GetWorld(), Start, NetworkDirection);
    const FVector Delta = End - VisualStart;
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
    const float StreakLength = FVector::Distance(StreakStart, End);
    if (StreakLength <= 1.0f)
    {
        Destroy();
        return;
    }

    SetActorLocation((StreakStart + End) * 0.5f);
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
    const FVector VisualMuzzle = ResolveLocalWeaponMuzzle(GetWorld(), Location, SafeDirection);
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
