#include "OCTransientVisualFX.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

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
    const FVector Delta = End - Start;
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
    SetActorLocation((Start + End) * 0.5f);
    SetActorRotation(FQuat::FindBetweenNormals(FVector::UpVector, Direction));
    // Thin high-speed streak. Never render the old oversized yellow projectile bead.
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
    // A muzzle flash is a short directional plume, not a glowing ball. Human beings apparently needed
    // several centuries of firearms and one game prototype to settle this visual distinction.
    if (UStaticMesh* Cone = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone")))
    {
        Mesh->SetStaticMesh(Cone);
    }
    else if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
    {
        Mesh->SetStaticMesh(Cylinder);
    }

    const FVector SafeDirection = Direction.GetSafeNormal();
    SetActorLocation(Location + SafeDirection * 11.0f);
    SetActorRotation(FQuat::FindBetweenNormals(FVector::UpVector, SafeDirection));
    Mesh->SetRelativeScale3D(FVector(0.025f, 0.025f, 0.14f));
    ApplyColor(Color);

    PointLight->SetVisibility(true);
    PointLight->SetLightColor(Color);
    PointLight->SetIntensity(2500.0f);
    PointLight->SetAttenuationRadius(220.0f);
    SetLifeSpan(FMath::Max(0.015f, LifetimeSeconds));
}
