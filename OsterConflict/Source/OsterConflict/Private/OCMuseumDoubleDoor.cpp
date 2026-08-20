#include "OCMuseumDoubleDoor.h"

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
    constexpr float TotalWidthCm = 216.0f;
    constexpr float LeafWidthCm = TotalWidthCm * 0.5f;
    constexpr float DoorHeightCm = 270.0f;
    constexpr float LeafThicknessCm = 7.0f;
    constexpr float FrameCm = 12.0f;
}

AOCMuseumDoubleDoor::AOCMuseumDoubleDoor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    bReplicates = true;
    SetReplicateMovement(false);
    SetNetUpdateFrequency(4.0f);
    SetMinNetUpdateFrequency(1.0f);
    SetNetCullDistanceSquared(FMath::Square(20000.0f));
    MaxInteractionDistance = 380.0f;

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
    FrameLeft->SetRelativeScale3D(FVector(FrameCm, 16.0f, DoorHeightCm + FrameCm * 2.0f) / 100.0f);
    FrameLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    FrameRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameRight"));
    FrameRight->SetupAttachment(SceneRoot);
    FrameRight->SetStaticMesh(Cube);
    FrameRight->SetRelativeLocation(FVector(TotalWidthCm * 0.5f + FrameCm * 0.5f, 0.0f, DoorHeightCm * 0.5f));
    FrameRight->SetRelativeScale3D(FVector(FrameCm, 16.0f, DoorHeightCm + FrameCm * 2.0f) / 100.0f);
    FrameRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    FrameTop = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameTop"));
    FrameTop->SetupAttachment(SceneRoot);
    FrameTop->SetStaticMesh(Cube);
    FrameTop->SetRelativeLocation(FVector(0.0f, 0.0f, DoorHeightCm + FrameCm * 0.5f));
    FrameTop->SetRelativeScale3D(FVector(TotalWidthCm + FrameCm * 2.0f, 16.0f, FrameCm) / 100.0f);
    FrameTop->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Photo-reference panel relief. These are shallow raised pieces attached to each moving leaf.
    auto AddPanel = [this, Cube](USceneComponent* Parent, TArray<TObjectPtr<UStaticMeshComponent>>& OutDetails,
        const TCHAR* Name, const FVector& LocalCenter, const FVector& SizeCm)
    {
        UStaticMeshComponent* Detail = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        Detail->SetupAttachment(Parent);
        Detail->SetStaticMesh(Cube);
        Detail->SetRelativeLocation(LocalCenter);
        Detail->SetRelativeScale3D(SizeCm / 100.0f);
        Detail->SetMobility(EComponentMobility::Movable);
        Detail->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        OutDetails.Add(Detail);
    };

    const FVector PanelSize(66.0f, 3.0f, 54.0f);
    AddPanel(LeftHinge, LeftPanelDetails, TEXT("LeftUpperPanel"),
        FVector(LeafWidthCm * 0.5f, -5.0f, 222.0f), PanelSize);
    AddPanel(LeftHinge, LeftPanelDetails, TEXT("LeftMiddlePanel"),
        FVector(LeafWidthCm * 0.5f, -5.0f, 145.0f), FVector(66.0f, 3.0f, 74.0f));
    AddPanel(LeftHinge, LeftPanelDetails, TEXT("LeftLowerPanel"),
        FVector(LeafWidthCm * 0.5f, -5.0f, 58.0f), FVector(66.0f, 3.0f, 58.0f));

    AddPanel(RightHinge, RightPanelDetails, TEXT("RightUpperPanel"),
        FVector(-LeafWidthCm * 0.5f, -5.0f, 222.0f), PanelSize);
    AddPanel(RightHinge, RightPanelDetails, TEXT("RightMiddlePanel"),
        FVector(-LeafWidthCm * 0.5f, -5.0f, 145.0f), FVector(66.0f, 3.0f, 74.0f));
    AddPanel(RightHinge, RightPanelDetails, TEXT("RightLowerPanel"),
        FVector(-LeafWidthCm * 0.5f, -5.0f, 58.0f), FVector(66.0f, 3.0f, 58.0f));

    // Narrow central raised strips reproduce the characteristic vertical spine visible in REF-06.
    AddPanel(LeftHinge, LeftPanelDetails, TEXT("LeftCenterSpine"),
        FVector(LeafWidthCm - 7.0f, -6.0f, 136.0f), FVector(8.0f, 4.0f, 252.0f));
    AddPanel(RightHinge, RightPanelDetails, TEXT("RightCenterSpine"),
        FVector(-LeafWidthCm + 7.0f, -6.0f, 136.0f), FVector(8.0f, 4.0f, 252.0f));

    LeftHandle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftHandle"));
    LeftHandle->SetupAttachment(LeftHinge);
    LeftHandle->SetStaticMesh(Cube);
    LeftHandle->SetRelativeLocation(FVector(LeafWidthCm - 15.0f, -10.0f, 125.0f));
    LeftHandle->SetRelativeScale3D(FVector(4.0f, 8.0f, 22.0f) / 100.0f);
    LeftHandle->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    RightHandle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightHandle"));
    RightHandle->SetupAttachment(RightHinge);
    RightHandle->SetStaticMesh(Cube);
    RightHandle->SetRelativeLocation(FVector(-LeafWidthCm + 15.0f, -10.0f, 125.0f));
    RightHandle->SetRelativeScale3D(FVector(4.0f, 8.0f, 22.0f) / 100.0f);
    RightHandle->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AOCMuseumDoubleDoor::BeginPlay()
{
    Super::BeginPlay();
    ApplyPhotoMaterials();
}

void AOCMuseumDoubleDoor::ApplyPhotoMaterials()
{
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Basic) return;

    UMaterialInstanceDynamic* Grey = UMaterialInstanceDynamic::Create(Basic, this,
        MakeUniqueObjectName(this, UMaterialInstanceDynamic::StaticClass(), FName(TEXT("MuseumDoorGrey"))));
    UMaterialInstanceDynamic* Bronze = UMaterialInstanceDynamic::Create(Basic, this,
        MakeUniqueObjectName(this, UMaterialInstanceDynamic::StaticClass(), FName(TEXT("MuseumDoorBronze"))));
    if (Grey) Grey->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.30f, 0.33f, 0.34f, 1.0f));
    if (Bronze) Bronze->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.56f, 0.39f, 0.15f, 1.0f));

    UStaticMeshComponent* GreyParts[] = { LeftLeaf.Get(), RightLeaf.Get(), FrameLeft.Get(), FrameRight.Get(), FrameTop.Get() };
    for (UStaticMeshComponent* Component : GreyParts)
    {
        if (Component && Grey) Component->SetMaterial(0, Grey);
    }
    for (const TObjectPtr<UStaticMeshComponent>& Detail : LeftPanelDetails)
        if (Detail && Grey) Detail->SetMaterial(0, Grey);
    for (const TObjectPtr<UStaticMeshComponent>& Detail : RightPanelDetails)
        if (Detail && Grey) Detail->SetMaterial(0, Grey);
    if (LeftHandle && Bronze) LeftHandle->SetMaterial(0, Bronze);
    if (RightHandle && Bronze) RightHandle->SetMaterial(0, Bronze);
}

void AOCMuseumDoubleDoor::Tick(float DeltaSeconds)
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

void AOCMuseumDoubleDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOCMuseumDoubleDoor, bOpen);
}

FString AOCMuseumDoubleDoor::GetInteractionPrompt(const AOCCharacter* InteractingCharacter) const
{
    return bOpen ? TEXT("E  CLOSE MUSEUM DOOR") : TEXT("E  OPEN MUSEUM DOOR");
}

bool AOCMuseumDoubleDoor::CanInteractServer(const AOCCharacter* InteractingCharacter) const
{
    return Super::CanInteractServer(InteractingCharacter);
}

void AOCMuseumDoubleDoor::InteractServer(AOCCharacter* InteractingCharacter)
{
    if (!CanInteractServer(InteractingCharacter)) return;
    bOpen = !bOpen;
    SetActorTickEnabled(true);
    if (WorldAudioComponent)
    {
        WorldAudioComponent->PlayEventServer(bOpen ? EOCWorldAudioEvent::DoorOpen : EOCWorldAudioEvent::DoorClose,
            GetActorLocation());
    }
    ForceNetUpdate();
}

void AOCMuseumDoubleDoor::OnRep_Open()
{
    SetActorTickEnabled(true);
}

void AOCMuseumDoubleDoor::ResetServer()
{
    if (!HasAuthority()) return;
    bOpen = false;
    SetActorTickEnabled(true);
    ForceNetUpdate();
}
