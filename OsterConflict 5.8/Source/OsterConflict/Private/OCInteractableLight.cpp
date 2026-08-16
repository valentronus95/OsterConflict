#include "OCInteractableLight.h"
#include "OCCharacter.h"
#include "OCWorldAudioComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

AOCInteractableLight::AOCInteractableLight()
{
    bReplicates = true;
    WorldAudioComponent = CreateDefaultSubobject<UOCWorldAudioComponent>(TEXT("WorldAudioComponent"));
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot")); SetRootComponent(SceneRoot);
    Fixture = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Fixture")); Fixture->SetupAttachment(SceneRoot);
    Fixture->SetCollisionProfileName(TEXT("BlockAll"));
    Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light")); Light->SetupAttachment(SceneRoot);
    Light->SetRelativeLocation(FVector(0,0,-18)); Light->SetIntensity(4200.0f); Light->SetAttenuationRadius(720.0f);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded()) Fixture->SetStaticMesh(SphereMesh.Object);
    Fixture->SetRelativeScale3D(FVector(0.12f));
    ApplyState();
}
void AOCInteractableLight::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const { Super::GetLifetimeReplicatedProps(OutLifetimeProps); DOREPLIFETIME(AOCInteractableLight,bLightOn); }
FString AOCInteractableLight::GetInteractionPrompt(const AOCCharacter*) const { return bLightOn ? TEXT("E  LIGHT OFF") : TEXT("E  LIGHT ON"); }
void AOCInteractableLight::InteractServer(AOCCharacter* C) { if (!CanInteractServer(C)) return; bLightOn=!bLightOn; if(WorldAudioComponent) WorldAudioComponent->PlayEventServer(bLightOn?EOCWorldAudioEvent::LightOn:EOCWorldAudioEvent::LightOff,GetActorLocation()); ApplyState(); ForceNetUpdate(); }
void AOCInteractableLight::ResetServer() { if (!HasAuthority()) return; bLightOn=false; ApplyState(); ForceNetUpdate(); }
void AOCInteractableLight::OnRep_LightState() { ApplyState(); }
void AOCInteractableLight::ApplyState() { if (Light) Light->SetVisibility(bLightOn); }
