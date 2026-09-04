#pragma once

#include "CoreMinimal.h"
#include "OCAntiArmorLauncher.h"
#include "OCWeaponBase.h"
#include "OCImportedWeaponVariants.generated.h"

/**
 * Distinct gameplay identities for weapon models already imported by the user.
 * Visuals are resolved from their exact local Fab/content package by the PASS45 imported-weapon bridge.
 * No unrelated model may substitute for a missing identity.
 */

UCLASS()
class OSTERCONFLICT_API AOCWeapon_AK74M : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_AK74M();
    virtual void BeginPlay() override;
};

UCLASS()
class OSTERCONFLICT_API AOCWeapon_AR15 : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_AR15();
    virtual void BeginPlay() override;
};

UCLASS()
class OSTERCONFLICT_API AOCWeapon_M4A1 : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_M4A1();
    virtual void BeginPlay() override;
};

UCLASS()
class OSTERCONFLICT_API AOCWeapon_FnBallista : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_FnBallista();
    virtual void BeginPlay() override;
};

UCLASS()
class OSTERCONFLICT_API AOCWeapon_Kar98k : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_Kar98k();
    virtual void BeginPlay() override;
};

UCLASS()
class OSTERCONFLICT_API AOCWeapon_Makarov : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_Makarov();
    virtual void BeginPlay() override;
};

UCLASS()
class OSTERCONFLICT_API AOCWeapon_TommyGun : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_TommyGun();
    virtual void BeginPlay() override;
};

UCLASS()
class OSTERCONFLICT_API AOCWeapon_M72LAW : public AOCAntiArmorLauncher
{
    GENERATED_BODY()
public:
    AOCWeapon_M72LAW();
    virtual void BeginPlay() override;
};

UCLASS()
class OSTERCONFLICT_API AOCWeapon_RPG26 : public AOCAntiArmorLauncher
{
    GENERATED_BODY()
public:
    AOCWeapon_RPG26();
    virtual void BeginPlay() override;
};

/** User-added compact AKS-74U Fab weapon. Kept distinct from AK-74M and AK-47. */
UCLASS()
class OSTERCONFLICT_API AOCWeapon_AKS74U : public AOCWeaponBase
{
    GENERATED_BODY()
public:
    AOCWeapon_AKS74U();
    virtual void BeginPlay() override;
};

/** User-added Fab RPG model. Kept distinct from OC_RPG1, M72 LAW and RPG-26. */
UCLASS()
class OSTERCONFLICT_API AOCWeapon_FabRPG : public AOCAntiArmorLauncher
{
    GENERATED_BODY()
public:
    AOCWeapon_FabRPG();
    virtual void BeginPlay() override;
};
