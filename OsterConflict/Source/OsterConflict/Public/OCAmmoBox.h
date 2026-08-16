#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OCWeaponTypes.h"
#include "OCAmmoBox.generated.h"

class AOCCharacter;
class UStaticMeshComponent;

UCLASS()
class OSTERCONFLICT_API AOCAmmoBox : public AActor
{
    GENERATED_BODY()

public:
    AOCAmmoBox();

    bool TryGiveAmmoServer(AOCCharacter* Character);

    UFUNCTION(BlueprintPure, Category="Ammo")
    EOCAmmoType GetAmmoType() const { return AmmoType; }

    UFUNCTION(BlueprintPure, Category="Ammo")
    int32 GetAmmoAmount() const { return AmmoAmount; }

    UFUNCTION(BlueprintPure, Category="Ammo")
    FString GetPromptText() const;

protected:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(EditDefaultsOnly, Category="Ammo")
    EOCAmmoType AmmoType = EOCAmmoType::Any;

    UPROPERTY(EditDefaultsOnly, Category="Ammo", meta=(ClampMin="1"))
    int32 AmmoAmount = 90;
};
