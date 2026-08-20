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
    virtual void BeginPlay() override;
};

UCLASS()
class OSTERCONFLICT_API AOCWeapon_Pistol : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_Pistol();
    virtual void BeginPlay() override;
};

UCLASS()
class OSTERCONFLICT_API AOCWeapon_Sniper : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_Sniper();
    virtual void BeginPlay() override;
};

UCLASS()
class OSTERCONFLICT_API AOCWeapon_Shotgun : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_Shotgun();
    virtual void BeginPlay() override;
};

UCLASS()
class OSTERCONFLICT_API AOCWeapon_LMG : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_LMG();
    virtual void BeginPlay() override;
};

/** Restored R13 M14 as a distinct battle-rifle variant. */
UCLASS()
class OSTERCONFLICT_API AOCWeapon_M14 : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_M14();
    virtual void BeginPlay() override;
};

/** Restored R13 MAC-10 as a compact automatic SMG variant. */
UCLASS()
class OSTERCONFLICT_API AOCWeapon_Mac10 : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_Mac10();
    virtual void BeginPlay() override;
};

/** Restored R13 TEC-9 as a compact semi-automatic secondary. */
UCLASS()
class OSTERCONFLICT_API AOCWeapon_Tec9 : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_Tec9();
    virtual void BeginPlay() override;
};

/** Restored R13 lever-action rifle. Kept distinct from the shotgun class. */
UCLASS()
class OSTERCONFLICT_API AOCWeapon_LeverAction : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_LeverAction();
    virtual void BeginPlay() override;
};
