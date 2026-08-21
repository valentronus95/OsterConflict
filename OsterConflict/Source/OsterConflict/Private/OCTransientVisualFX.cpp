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

        // Multicast trace data is camera-authored for hit validation. Only rewrite the local shooter's
        // presentation. Remote shots keep their replicated start because their weapon geometry is not
        // authoritative on this client's first-person camera.
        if (FVector::DistSquared(ViewLocation, NetworkStart) > FMath::Square(90.0f)) return NetworkStart;

        const FVector SafeDirection = Direction.GetSafeNormal();
        if (SafeDirection.IsNearlyZero()) return NetworkStart;

        TArray<UPrimitiveComponent*> Components;
        Weapon->GetComponents<UPrimitiveComponent>(Components);

        bool bHasProductionVisual = false;
        for (const UPrimitiveComponent* Component : Components)
        {
            if (Component && Component->ComponentHasTag(FName(TEXT("OC_ProductionWeaponVisual"))))
            {
                bHasProductionVisual = true;
                break;
            }
        }

        FVector BestPoint = NetworkStart;
        float BestProjection = -TNumericLimits<float>::Max();
        for (const UPrimitiveComponent* Component : Components)
        {
            if (!Component || !Component->IsRegistered()) continue;
            if (bHasProductionVisual && !Component->ComponentHasTag(FName(TEXT("OC_ProductionWeaponVisual")))) continue;

            const FBoxSphereBounds& Bounds = Component->Bounds;
            if (Bounds.SphereRadius <= 1.0f) continue;

            // Project the component's world AABB onto the shot direction and choose the furthest
            // rendered weapon point. This works for AK, pistol and imported static/skeletal weapons
            // without inventing one hard-coded muzzle offset for every mesh.
            const float Support =
                FMath::Abs(SafeDirection.X) * Bounds.BoxExtent.X +
                FMath::Abs(SafeDirection.Y) * Bounds.BoxExtent.Y +
                FMath::Abs(SafeDirection.Z) * Bounds.BoxExtent.Z;
            const float Projection = FVector::DotProduct(Bounds.Origin, SafeDirection) + Support;
            if (Projection > BestProjection)
            {
                BestProjection = Projection;
                BestPoint = Bounds.Origin + SafeDirection * Support;
            }
        }

        const float DistanceFromView = FVector::Distance(ViewLocation, BestPoint);
        if (BestProjection <= -TNumericLimits<float>::Max() * 0.5f || DistanceFromView < 20.0f || DistanceFromView > 220.0f)
        {
            return NetworkStart;
        }
        return BestPoint;
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

    const FVector Direction = Delta / Length;
    SetActorLocation((VisualStart + End) * 0.5f);
    SetActorRotation(FQuat::FindBetweenNormals(FVector::UpVector, Direction));
    // Thin high-speed streak instead of an oversized projectile bead.
    const float ThinRadius = FMath::Clamp(RadiusCm, 0.18f, 0.42f);
    Mesh->SetRelativeScale3D(FVector(ThinRadius / 50.0f, ThinRadius / 50.0f, Length / 100.0f));
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
    // Muzzle presentation is a short directional plume rather than a glowing sphere.
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
    SetActorLocation(VisualMuzzle + SafeDirection * 11.0f);
    SetActorRotation(FQuat::FindBetweenNormals(FVector::UpVector, SafeDirection));
    Mesh->SetRelativeScale3D(FVector(0.025f, 0.025f, 0.14f));
    ApplyColor(Color);

    PointLight->SetVisibility(true);
    PointLight->SetLightColor(Color);
    PointLight->SetIntensity(2500.0f);
    PointLight->SetAttenuationRadius(220.0f);
    SetLifeSpan(FMath::Max(0.015f, LifetimeSeconds));
}
