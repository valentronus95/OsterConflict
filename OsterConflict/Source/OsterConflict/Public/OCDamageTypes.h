#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "OCDamageTypes.generated.h"

/** Ordinary bullets and machine-gun fire. */
UCLASS()
class OSTERCONFLICT_API UOCBallisticDamageType : public UDamageType
{
    GENERATED_BODY()
};

/** Heavy direct-fire vehicle weapon. Still not an anti-armour explosive in S11. */
UCLASS()
class OSTERCONFLICT_API UOCVehicleCannonDamageType : public UDamageType
{
    GENERATED_BODY()
};

/** Gameplay tag class reserved for RPG / disposable anti-armour launchers in S12. */
UCLASS()
class OSTERCONFLICT_API UOCAntiArmorDamageType : public UDamageType
{
    GENERATED_BODY()
};

/** Grenades and non anti-armour blast damage. */
UCLASS()
class OSTERCONFLICT_API UOCExplosiveDamageType : public UDamageType
{
    GENERATED_BODY()
};
