#include "OCInteractableGate.h"
#include "OCCharacter.h"
#include "OCWorldAudioComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AOCInteractableGate::AOCInteractableGate()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    bReplicates = true;
    SetNetUpdateFrequency(2.0f);
    SetMinNetUpdateFrequency(0.5f);
    MaxInteractionDistance = 430.0f;
    WorldAudioComponent = CreateDefaultSubobject<UOCWorldAudioComponent>(TEXT("WorldAudioComponent"));
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);
    GateLeaf = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateLeaf"));
    GateLeaf->SetupAttachment(SceneRoot);
    GateLeaf->SetMobility(EComponentMobility::Movable);
    GateLeaf->SetCollisionProfileName(TEXT("BlockAll"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded()) GateLeaf->SetStaticMesh(Cube.Object);
    GateLeaf->SetRelativeLocation(FVector(150,0,105));
    GateLeaf->SetRelativeScale3D(FVector(3.0f,0.10f,2.1f));
}

void AOCInteractableGate::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!GateLeaf) return;
    const float TargetYaw = bOpen ? OpenYaw : 0.0f;
    const float CurrentYaw = GateLeaf->GetRelativeRotation().Yaw;
    const float NewYaw = FMath::FInterpTo(CurrentYaw, TargetYaw, DeltaSeconds, 5.5f);
    GateLeaf->SetRelativeRotation(FRotator(0.0f, NewYaw, 0.0f));
    if (FMath::Abs(FMath::FindDeltaAngleDegrees(NewYaw, TargetYaw)) < 0.20f)
    {
        GateLeaf->SetRelativeRotation(FRotator(0.0f, TargetYaw, 0.0f));
        SetActorTickEnabled(false);
    }
}

void AOCInteractableGate::OnRep_Open() { SetActorTickEnabled(true); }
void AOCInteractableGate::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AOCInteractableGate,bOpen);
}
FString AOCInteractableGate::GetInteractionPrompt(const AOCCharacter*) const { return bOpen ? TEXT("E  CLOSE GATE") : TEXT("E  OPEN GATE"); }
void AOCInteractableGate::InteractServer(AOCCharacter* Character)
{
    if (!CanInteractServer(Character)) return;
    bOpen = !bOpen;
    SetActorTickEnabled(true);
    if (WorldAudioComponent) WorldAudioComponent->PlayEventServer(bOpen ? EOCWorldAudioEvent::GateOpen : EOCWorldAudioEvent::GateClose, GetActorLocation());
    ForceNetUpdate();
}
void AOCInteractableGate::ResetServer()
{
    if (!HasAuthority()) return;
    bOpen = false;
    SetActorTickEnabled(true);
    ForceNetUpdate();
}
