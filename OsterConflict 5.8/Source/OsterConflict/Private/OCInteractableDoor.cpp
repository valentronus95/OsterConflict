#include "OCInteractableDoor.h"

#include "OCCharacter.h"
#include "OCWorldAudioComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AOCInteractableDoor::AOCInteractableDoor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    bReplicates = true;
    SetNetUpdateFrequency(4.0f);
    SetMinNetUpdateFrequency(1.0f);
    SetNetCullDistanceSquared(FMath::Square(18000.0f));
    SetReplicateMovement(false);
    MaxInteractionDistance = 360.0f;
    WorldAudioComponent = CreateDefaultSubobject<UOCWorldAudioComponent>(TEXT("WorldAudioComponent"));

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    FrameLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameLeft"));
    FrameLeft->SetupAttachment(SceneRoot);
    FrameRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameRight"));
    FrameRight->SetupAttachment(SceneRoot);
    FrameTop = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameTop"));
    FrameTop->SetupAttachment(SceneRoot);

    DoorLeaf = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorLeaf"));
    DoorLeaf->SetupAttachment(SceneRoot);
    DoorLeaf->SetMobility(EComponentMobility::Movable);

    Handle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Handle"));
    Handle->SetupAttachment(DoorLeaf);
    Handle->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        FrameLeft->SetStaticMesh(CubeMesh.Object);
        FrameRight->SetStaticMesh(CubeMesh.Object);
        FrameTop->SetStaticMesh(CubeMesh.Object);
        DoorLeaf->SetStaticMesh(CubeMesh.Object);
        Handle->SetStaticMesh(CubeMesh.Object);
    }

    const float FrameThickness = 12.0f;
    const float LeafThickness = 6.0f;
    FrameLeft->SetRelativeLocation(FVector(-FrameThickness * 0.5f, 0.0f, DoorHeightCm * 0.5f));
    FrameLeft->SetRelativeScale3D(FVector(FrameThickness / 100.0f, 18.0f / 100.0f, DoorHeightCm / 100.0f));
    FrameRight->SetRelativeLocation(FVector(DoorWidthCm + FrameThickness * 0.5f, 0.0f, DoorHeightCm * 0.5f));
    FrameRight->SetRelativeScale3D(FVector(FrameThickness / 100.0f, 18.0f / 100.0f, DoorHeightCm / 100.0f));
    FrameTop->SetRelativeLocation(FVector(DoorWidthCm * 0.5f, 0.0f, DoorHeightCm + FrameThickness * 0.5f));
    FrameTop->SetRelativeScale3D(FVector((DoorWidthCm + FrameThickness * 2.0f) / 100.0f, 18.0f / 100.0f, FrameThickness / 100.0f));

    // Root is the hinge. The leaf extends in +X and rotates around Z.
    DoorLeaf->SetRelativeLocation(FVector(DoorWidthCm * 0.5f, 0.0f, DoorHeightCm * 0.5f));
    DoorLeaf->SetRelativeScale3D(FVector(DoorWidthCm / 100.0f, LeafThickness / 100.0f, DoorHeightCm / 100.0f));
    DoorLeaf->SetCollisionProfileName(TEXT("BlockAll"));

    Handle->SetRelativeLocation(FVector(DoorWidthCm * 0.36f, -7.0f, 5.0f));
    Handle->SetRelativeScale3D(FVector(0.08f, 0.10f, 0.08f));
}

void AOCInteractableDoor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!DoorLeaf)
    {
        return;
    }

    const float TargetYaw = bOpen ? OpenYawDegrees : 0.0f;
    const float CurrentYaw = DoorLeaf->GetRelativeRotation().Yaw;
    const float NewYaw = FMath::FInterpTo(CurrentYaw, TargetYaw, DeltaSeconds, DoorInterpSpeed);
    DoorLeaf->SetRelativeRotation(FRotator(0.0f, NewYaw, 0.0f));
    if (FMath::Abs(FMath::FindDeltaAngleDegrees(NewYaw, TargetYaw)) < 0.20f)
    {
        DoorLeaf->SetRelativeRotation(FRotator(0.0f, TargetYaw, 0.0f));
        SetActorTickEnabled(false);
    }
}

void AOCInteractableDoor::OnRep_Open()
{
    SetActorTickEnabled(true);
}

void AOCInteractableDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOCInteractableDoor, bOpen);
}

FString AOCInteractableDoor::GetInteractionPrompt(const AOCCharacter* InteractingCharacter) const
{
    return bOpen ? TEXT("E  CLOSE DOOR") : TEXT("E  OPEN DOOR");
}

bool AOCInteractableDoor::CanInteractServer(const AOCCharacter* InteractingCharacter) const
{
    return Super::CanInteractServer(InteractingCharacter);
}

void AOCInteractableDoor::ResetServer()
{
    if (!HasAuthority()) return;
    bOpen = false;
    SetActorTickEnabled(true);
    ForceNetUpdate();
}

void AOCInteractableDoor::InteractServer(AOCCharacter* InteractingCharacter)
{
    if (!CanInteractServer(InteractingCharacter))
    {
        return;
    }

    bOpen = !bOpen;
    SetActorTickEnabled(true);
    if (WorldAudioComponent) WorldAudioComponent->PlayEventServer(bOpen ? EOCWorldAudioEvent::DoorOpen : EOCWorldAudioEvent::DoorClose, GetActorLocation());
    ForceNetUpdate();
}
