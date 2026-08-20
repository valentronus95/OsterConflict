#include "OCMuseumServiceDoubleDoor.h"

#include "OCCharacter.h"
#include "OCWorldAudioComponent.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float TotalWidthCm = 156.0f;
    constexpr float LeafWidthCm = TotalWidthCm * 0.5f;
    constexpr float DoorHeightCm = 276.0f;
    constexpr float LeafThicknessCm = 7.0f;
    constexpr float FrameCm = 11.0f;
}

AOCMuseumServiceDoubleDoor::AOCMuseumServiceDoubleDoor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    bReplicates = true;
    SetReplicateMovement(false);
    SetNetUpdateFrequency(4.0f);
    SetMinNetUpdateFrequency(1.0f);
    SetNetCullDistanceSquared(FMath::Square(20000.0f));
    MaxInteractionDistance = 360.0f;

    WorldAudioComponent = CreateDefaultSubobject<UOCWorldAudioComponent>(TEXT("WorldAudioComponent"));
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    LeftHinge = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHinge"));
    LeftHinge->SetupAttachment(SceneRoot);
    LeftHinge->SetRelativeLocation(FVector(-TotalWidthCm * 0.5f, 0.0f, 0.0f));

    RightHinge = CreateDefaultSubobject<USceneComponent>(TEXT("RightHinge"));
    RightHinge->SetupAttachment(SceneRoot);
    RightHinge->SetRelativeLocation(FVector(TotalWidthCm * 0.5f, 0.0f, 0.0f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Cube = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;
    if (!Cube) return;

    LeftLeaf = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftLeaf"));
    LeftLeaf->SetupAttachment(LeftHinge);
    LeftLeaf->SetStaticMesh(Cube);
    LeftLeaf->SetRelativeLocation(FVector(LeafWidthCm * 0.5f, 0.0f, DoorHeightCm * 0.5f));
    LeftLeaf->SetRelativeScale3D(FVector(LeafWidthCm, LeafThicknessCm, DoorHeightCm) / 100.0f);
    LeftLeaf->SetMobility(EComponentMobility::Movable);
    LeftLeaf->SetCollisionProfileName(TEXT("BlockAll"));

    RightLeaf = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightLeaf"));
    RightLeaf->SetupAttachment(RightHinge);
    RightLeaf->SetStaticMesh(Cube);
    RightLeaf->SetRelativeLocation(FVector(-LeafWidthCm * 0.5f, 0.0f, DoorHeightCm * 0.5f));
    RightLeaf->SetRelativeScale3D(FVector(LeafWidthCm, LeafThicknessCm, DoorHeightCm) / 100.0f);
    RightLeaf->SetMobility(EComponentMobility::Movable);
    RightLeaf->SetCollisionProfileName(TEXT("BlockAll"));

    FrameLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameLeft"));
    FrameLeft->SetupAttachment(SceneRoot);
    FrameLeft->SetStaticMesh(Cube);
    FrameLeft->SetRelativeLocation(FVector(-TotalWidthCm * 0.5f - FrameCm * 0.5f, 0.0f, DoorHeightCm * 0.5f));
    FrameLeft->SetRelativeScale3D(FVector(FrameCm, 15.0f, DoorHeightCm + FrameCm * 2.0f) / 100.0f);
    FrameLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    FrameRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameRight"));
    FrameRight->SetupAttachment(SceneRoot);
    FrameRight->SetStaticMesh(Cube);
    FrameRight->SetRelativeLocation(FVector(TotalWidthCm * 0.5f + FrameCm * 0.5f, 0.0f, DoorHeightCm * 0.5f));
    FrameRight->SetRelativeScale3D(FVector(FrameCm, 15.0f, DoorHeightCm + FrameCm * 2.0f) / 100.0f);
    FrameRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    FrameTop = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameTop"));
    FrameTop->SetupAttachment(SceneRoot);
    FrameTop->SetStaticMesh(Cube);
    FrameTop->SetRelativeLocation(FVector(0.0f, 0.0f, DoorHeightCm + FrameCm * 0.5f));
    FrameTop->SetRelativeScale3D(FVector(TotalWidthCm + FrameCm * 2.0f, 15.0f, FrameCm) / 100.0f);
    FrameTop->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    auto AddPanel = [this, Cube](USceneComponent* Parent, const TCHAR* Name,
        const FVector& LocalCenter, const FVector& SizeCm)
    {
        UStaticMeshComponent* Detail = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        Detail->SetupAttachment(Parent);
        Detail->SetStaticMesh(Cube);
        Detail->SetRelativeLocation(LocalCenter);
        Detail->SetRelativeScale3D(SizeCm / 100.0f);
        Detail->SetMobility(EComponentMobility::Movable);
        Detail->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PanelDetails.Add(Detail);
    };

    // REF-11/REF-20: restrained rectangular relief, unlike the ornate main entrance.
    AddPanel(LeftHinge, TEXT("LeftLowerPanel"),
        FVector(LeafWidthCm * 0.5f, -5.0f, 58.0f), FVector(58.0f, 3.0f, 42.0f));
    AddPanel(RightHinge, TEXT("RightLowerPanel"),
        FVector(-LeafWidthCm * 0.5f, -5.0f, 58.0f), FVector(58.0f, 3.0f, 42.0f));
    AddPanel(LeftHinge, TEXT("LeftCenterStile"),
        FVector(LeafWidthCm - 5.0f, -5.0f, 138.0f), FVector(7.0f, 3.0f, 250.0f));
    AddPanel(RightHinge, TEXT("RightCenterStile"),
        FVector(-LeafWidthCm + 5.0f, -5.0f, 138.0f), FVector(7.0f, 3.0f, 250.0f));
}

void AOCMuseumServiceDoubleDoor::BeginPlay()
{
    Super::BeginPlay();
    ApplyPhotoMaterials();
}

void AOCMuseumServiceDoubleDoor::ApplyPhotoMaterials()
{
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Basic) return;

    UMaterialInstanceDynamic* Grey = UMaterialInstanceDynamic::Create(Basic, this,
        MakeUniqueObjectName(this, UMaterialInstanceDynamic::StaticClass(), FName(TEXT("MuseumServiceDoorGrey"))));
    if (Grey)
    {
        Grey->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.34f, 0.37f, 0.38f, 1.0f));
    }

    UStaticMeshComponent* GreyParts[] =
    {
        LeftLeaf.Get(), RightLeaf.Get(), FrameLeft.Get(), FrameRight.Get(), FrameTop.Get()
    };
    for (UStaticMeshComponent* Component : GreyParts)
    {
        if (Component && Grey) Component->SetMaterial(0, Grey);
    }
    for (const TObjectPtr<UStaticMeshComponent>& Detail : PanelDetails)
    {
        if (Detail && Grey) Detail->SetMaterial(0, Grey);
    }
}

void AOCMuseumServiceDoubleDoor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!LeftHinge || !RightHinge) return;

    const float LeftTarget = bOpen ? -OpenYawDegrees : 0.0f;
    const float RightTarget = bOpen ? OpenYawDegrees : 0.0f;
    const float LeftYaw = FMath::FInterpTo(LeftHinge->GetRelativeRotation().Yaw, LeftTarget, DeltaSeconds, DoorInterpSpeed);
    const float RightYaw = FMath::FInterpTo(RightHinge->GetRelativeRotation().Yaw, RightTarget, DeltaSeconds, DoorInterpSpeed);
    LeftHinge->SetRelativeRotation(FRotator(0.0f, LeftYaw, 0.0f));
    RightHinge->SetRelativeRotation(FRotator(0.0f, RightYaw, 0.0f));

    if (FMath::Abs(FMath::FindDeltaAngleDegrees(LeftYaw, LeftTarget)) < 0.2f &&
        FMath::Abs(FMath::FindDeltaAngleDegrees(RightYaw, RightTarget)) < 0.2f)
    {
        LeftHinge->SetRelativeRotation(FRotator(0.0f, LeftTarget, 0.0f));
        RightHinge->SetRelativeRotation(FRotator(0.0f, RightTarget, 0.0f));
        SetActorTickEnabled(false);
    }
}

void AOCMuseumServiceDoubleDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOCMuseumServiceDoubleDoor, bOpen);
}

FString AOCMuseumServiceDoubleDoor::GetInteractionPrompt(const AOCCharacter* InteractingCharacter) const
{
    return bOpen ? TEXT("E  CLOSE SERVICE DOOR") : TEXT("E  OPEN SERVICE DOOR");
}

bool AOCMuseumServiceDoubleDoor::CanInteractServer(const AOCCharacter* InteractingCharacter) const
{
    return Super::CanInteractServer(InteractingCharacter);
}

void AOCMuseumServiceDoubleDoor::InteractServer(AOCCharacter* InteractingCharacter)
{
    if (!CanInteractServer(InteractingCharacter)) return;
    bOpen = !bOpen;
    SetActorTickEnabled(true);
    if (WorldAudioComponent)
    {
        WorldAudioComponent->PlayEventServer(
            bOpen ? EOCWorldAudioEvent::DoorOpen : EOCWorldAudioEvent::DoorClose,
            GetActorLocation());
    }
    ForceNetUpdate();
}

void AOCMuseumServiceDoubleDoor::OnRep_Open()
{
    SetActorTickEnabled(true);
}

void AOCMuseumServiceDoubleDoor::ResetServer()
{
    if (!HasAuthority()) return;
    bOpen = false;
    SetActorTickEnabled(true);
    ForceNetUpdate();
}
