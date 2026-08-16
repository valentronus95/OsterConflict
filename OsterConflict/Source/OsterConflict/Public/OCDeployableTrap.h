#pragma once

#include "CoreMinimal.h"
#include "OCInteractableActor.h"
#include "OCOrdnanceTypes.h"
#include "OCTeamTypes.h"
#include "OCDeployableTrap.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/** Abstract in-game trap actor; deliberately contains no real construction data. */
UCLASS()
class OSTERCONFLICT_API AOCDeployableTrap : public AOCInteractableActor
{
    GENERATED_BODY()
public:
    AOCDeployableTrap();
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual FString GetInteractionPrompt(const AOCCharacter* InteractingCharacter) const override;
    virtual bool CanInteractServer(const AOCCharacter* InteractingCharacter) const override;
    virtual void InteractServer(AOCCharacter* InteractingCharacter) override;

    void ConfigureTrapServer(EOCTrapPreset NewPreset, EOCTeam NewTeam);

    UFUNCTION(BlueprintPure, Category="Trap") EOCTrapPreset GetTrapPreset() const { return TrapPreset; }
    UFUNCTION(BlueprintPure, Category="Trap") bool IsArmed() const { return bArmed; }

protected:
    UPROPERTY(VisibleAnywhere) TObjectPtr<USphereComponent> Trigger;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> TrapMesh;
    UPROPERTY(Replicated, VisibleInstanceOnly, Category="Trap") EOCTrapPreset TrapPreset = EOCTrapPreset::ContactInfantry;
    UPROPERTY(Replicated, VisibleInstanceOnly, Category="Trap") EOCTeam OwningTeam = EOCTeam::None;
    UPROPERTY(Replicated, VisibleInstanceOnly, Category="Trap") bool bArmed = false;

private:
    FTimerHandle ArmTimerHandle;
    void ArmServer();
    UFUNCTION() void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    void TriggerTrapServer(AActor* TriggeringActor);
};
