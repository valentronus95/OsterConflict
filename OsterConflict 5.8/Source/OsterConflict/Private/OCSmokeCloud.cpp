#include "OCSmokeCloud.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AOCSmokeCloud::AOCSmokeCloud()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(false);

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    const FVector Offsets[] = {
        FVector(0,0,80), FVector(150,20,105), FVector(-145,-35,115), FVector(45,170,90),
        FVector(-30,-165,100), FVector(225,-120,125), FVector(-215,110,120), FVector(95,-245,95),
        FVector(-105,245,110), FVector(285,70,100), FVector(-275,-85,120), FVector(20,20,185)
    };
    for (int32 Index=0; Index<UE_ARRAY_COUNT(Offsets); ++Index)
    {
        UStaticMeshComponent* Puff = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("SmokePuff_%02d"), Index));
        Puff->SetupAttachment(SceneRoot);
        Puff->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Puff->SetCastShadow(false);
        if (SphereMesh.Succeeded()) Puff->SetStaticMesh(SphereMesh.Object);
        Puff->SetRelativeLocation(Offsets[Index]);
        const float Scale = 2.7f + static_cast<float>(Index % 4) * 0.42f;
        Puff->SetRelativeScale3D(FVector(Scale, Scale, Scale * 0.78f));
        Puffs.Add(Puff);
    }
}

void AOCSmokeCloud::BeginPlay()
{
    Super::BeginPlay();
    SetLifeSpan(LifetimeSeconds);
}

bool AOCSmokeCloud::ContainsPoint(const FVector& WorldPoint) const
{
    return FVector::DistSquared2D(GetActorLocation(), WorldPoint) <= FMath::Square(SmokeRadiusCm);
}
