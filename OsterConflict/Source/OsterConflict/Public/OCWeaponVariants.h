#pragma once

#include "CoreMinimal.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.generated.h"

UCLASS()
class OSTERCONFLICT_API AOCWeapon_AssaultRifle : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_AssaultRifle();
    virtual void BeginPlay() override;
};

UCLASS()
class OSTERCONFLICT_API AOCWeapon_SMG : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_SMG();
};

UCLASS()
class OSTERCONFLICT_API AOCWeapon_Pistol : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_Pistol();
};

UCLASS()
class OSTERCONFLICT_API AOCWeapon_Sniper : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_Sniper();
};

UCLASS()
class OSTERCONFLICT_API AOCWeapon_Shotgun : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_Shotgun();
};

UCLASS()
class OSTERCONFLICT_API AOCWeapon_LMG : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_LMG();
};
