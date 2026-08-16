#include "OCAmbientAudioZone.h"
#include "Sound/SoundBase.h"
#include "OCAudioUserSettings.h"
#include "OCWorldAudioProfile.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AOCAmbientAudioZone::AOCAmbientAudioZone()
{
    PrimaryActorTick.bCanEverTick=true;
    PrimaryActorTick.TickInterval=0.20f;
    bReplicates=true;
    SetNetUpdateFrequency(1.0f);
    SetMinNetUpdateFrequency(0.20f);
    SetNetCullDistanceSquared(FMath::Square(30000.0f));
    SetReplicateMovement(false);
    Zone=CreateDefaultSubobject<UBoxComponent>(TEXT("AmbientZone")); SetRootComponent(Zone);
    Zone->SetBoxExtent(FVector(1800,1800,600)); Zone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void AOCAmbientAudioZone::BeginPlay(){Super::BeginPlay();if(GetWorld()&&GetWorld()->GetNetMode()==NM_DedicatedServer){SetActorTickEnabled(false);return;}NextOneShotTime=GetWorld()?GetWorld()->GetTimeSeconds()+3.0f:3.0f;}
void AOCAmbientAudioZone::ConfigureRuntime(const FVector& E,float MinI,float MaxI,float R)
{
    const FVector SafeExtent(FMath::Max(E.X,100.0f),FMath::Max(E.Y,100.0f),FMath::Max(E.Z,100.0f));
    if(Zone) Zone->SetBoxExtent(SafeExtent);
    OneShotMinInterval=FMath::Max(1.0f,MinI);
    OneShotMaxInterval=FMath::Max(OneShotMinInterval,MaxI);
    OneShotRadius=FMath::Max(200.0f,R);
}
void AOCAmbientAudioZone::Tick(float D){Super::Tick(D);if(!GetWorld()||GetWorld()->GetNetMode()==NM_DedicatedServer)return;UpdateListenerState();if(bListenerInside&&GetWorld()->GetTimeSeconds()>=NextOneShotTime){PlayRandomOneShot();NextOneShotTime=GetWorld()->GetTimeSeconds()+FMath::FRandRange(OneShotMinInterval,FMath::Max(OneShotMinInterval,OneShotMaxInterval));}}
void AOCAmbientAudioZone::UpdateListenerState()
{
    APlayerController* PC=UGameplayStatics::GetPlayerController(this,0); if(!PC||!PC->PlayerCameraManager)return;
    const FVector L=PC->PlayerCameraManager->GetCameraLocation(); const bool bNow=Zone->Bounds.GetBox().IsInside(L);
    const float Vol=UOCAudioUserSettings::Get()->GetBusVolume(EOCAudioBus::Ambience);
    if(bNow&&!bListenerInside&&AudioProfile&&AudioProfile->AmbientBed&&Vol>0.0f) BedComponent=UGameplayStatics::SpawnSoundAtLocation(this,AudioProfile->AmbientBed,GetActorLocation(),FRotator::ZeroRotator,Vol);
    if((!bNow||Vol<=0.0f)&&BedComponent){BedComponent->FadeOut(0.35f,0.0f);BedComponent=nullptr;}
    if(BedComponent) BedComponent->SetVolumeMultiplier(Vol);
    bListenerInside=bNow;
}
void AOCAmbientAudioZone::PlayRandomOneShot()
{
    if(!AudioProfile)return; const float Vol=UOCAudioUserSettings::Get()->GetBusVolume(EOCAudioBus::Ambience);if(Vol<=0.0f)return;
    TArray<USoundBase*> Pool;
    auto Add=[&Pool](const TArray<TObjectPtr<USoundBase>>& A){for(const TObjectPtr<USoundBase>& S:A)if(S)Pool.Add(S.Get());};
    Add(AudioProfile->Birds);Add(AudioProfile->WindLeaves);Add(AudioProfile->YardAnimals);Add(AudioProfile->DistantDogs);Add(AudioProfile->DistantTraffic);Add(AudioProfile->Water);
    if(Pool.IsEmpty())return; const FVector Offset=FVector(FMath::FRandRange(-OneShotRadius,OneShotRadius),FMath::FRandRange(-OneShotRadius,OneShotRadius),FMath::FRandRange(80.0f,420.0f));
    UGameplayStatics::PlaySoundAtLocation(this,Pool[FMath::RandRange(0,Pool.Num()-1)],GetActorLocation()+Offset,Vol*FMath::FRandRange(0.55f,0.95f),FMath::FRandRange(0.97f,1.03f));
}
