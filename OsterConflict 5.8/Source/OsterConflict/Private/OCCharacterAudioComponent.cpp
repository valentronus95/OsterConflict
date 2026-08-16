#include "OCCharacterAudioComponent.h"
#include "Sound/SoundBase.h"
#include "OCAudioUserSettings.h"
#include "OCCharacter.h"
#include "OCCharacterAudioProfile.h"
#include "OCDestructibleProp.h"
#include "OCHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
UOCCharacterAudioComponent::UOCCharacterAudioComponent(){PrimaryComponentTick.bCanEverTick=true;SetIsReplicatedByDefault(false);}
void UOCCharacterAudioComponent::BeginPlay(){Super::BeginPlay();if(AOCCharacter* C=Cast<AOCCharacter>(GetOwner()))if(UOCHealthComponent* H=C->GetHealthComponent()){H->OnHealthChanged.AddDynamic(this,&UOCCharacterAudioComponent::OnHealthChanged);H->OnDowned.AddDynamic(this,&UOCCharacterAudioComponent::OnDowned);H->OnRevived.AddDynamic(this,&UOCCharacterAudioComponent::OnRevived);H->OnDeath.AddDynamic(this,&UOCCharacterAudioComponent::OnDeath);}}
void UOCCharacterAudioComponent::TickComponent(float D,ELevelTick T,FActorComponentTickFunction* F){Super::TickComponent(D,T,F);AOCCharacter* C=Cast<AOCCharacter>(GetOwner());if(!C||!AudioProfile||!C->GetCharacterMovement()||C->GetWorld()->GetNetMode()==NM_DedicatedServer)return;const float S=C->GetVelocity().Size2D();if(S<80.0f||C->GetCharacterMovement()->IsFalling()||C->IsDowned()){StepTimer=0.0f;return;}StepTimer+=D;const float I=S>600.0f?AudioProfile->SprintStepInterval:AudioProfile->WalkStepInterval;if(StepTimer>=I){StepTimer=0.0f;PlayFootstep();if(S>600.0f&&FMath::RandRange(0,2)==0)PlayVariant(AudioProfile->GearRustle,0.45f);}}
void UOCCharacterAudioComponent::PlayFootstep(){AOCCharacter* C=Cast<AOCCharacter>(GetOwner());if(!C||!AudioProfile)return;FHitResult H;FCollisionQueryParams P(SCENE_QUERY_STAT(OCFootstep),false,C);const FVector A=C->GetActorLocation();const FVector B=A-FVector(0,0,150);EOCImpactSurface Surface=EOCImpactSurface::Masonry;if(GetWorld()->LineTraceSingleByChannel(H,A,B,ECC_Visibility,P)){if(const AOCDestructibleProp* D=Cast<AOCDestructibleProp>(H.GetActor()))Surface=D->GetImpactSurface();else if(H.GetActor()&&H.GetActor()->ActorHasTag(TEXT("Dirt")))Surface=EOCImpactSurface::Dirt;else if(H.GetActor()&&H.GetActor()->ActorHasTag(TEXT("Wood")))Surface=EOCImpactSurface::Wood;else if(H.GetActor()&&H.GetActor()->ActorHasTag(TEXT("Metal")))Surface=EOCImpactSurface::Metal;}PlayVariant(AudioProfile->GetFootstepSet(Surface),1.0f);}
void UOCCharacterAudioComponent::PlayVariant(const TArray<TObjectPtr<USoundBase>>& Set,float Volume){if(Set.IsEmpty()||!GetOwner())return;USoundBase* S=Set[FMath::Abs(++EventSeed)%Set.Num()];if(!S)return;const float V=UOCAudioUserSettings::Get()->GetBusVolume(EOCAudioBus::Characters);if(V>0.0f)UGameplayStatics::PlaySoundAtLocation(this,S,GetOwner()->GetActorLocation(),V*Volume,FMath::FRandRange(0.97f,1.03f));}
void UOCCharacterAudioComponent::OnHealthChanged(float,float Delta){if(!AudioProfile||Delta>=0.0f)return;PlayVariant(FMath::Abs(Delta)>=35.0f?AudioProfile->PainHeavy:AudioProfile->PainLight,1.0f);}
void UOCCharacterAudioComponent::OnDowned(){if(AudioProfile)PlayVariant(AudioProfile->Downed,1.0f);}
void UOCCharacterAudioComponent::OnRevived(){if(AudioProfile)PlayVariant(AudioProfile->Revived,0.8f);}
void UOCCharacterAudioComponent::OnDeath(){if(AudioProfile)PlayVariant(AudioProfile->Death,1.0f);}
