#include "OCCharacterProductionProfiles.h"

namespace
{
    constexpr const TCHAR* SharedBodyPath = TEXT("/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter.SKM_QuantumCharacter");
    constexpr const TCHAR* SharedArmsPath = TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Arms.SKM_Arms");

    FOCCharacterProductionProfile MakeSharedQuantumProfile(
        const EOCFactionArchetype Faction,
        const TCHAR* DisplayName)
    {
        FOCCharacterProductionProfile Profile;
        Profile.Faction = Faction;
        Profile.DisplayName = DisplayName;
        Profile.ThirdPersonBodyObjectPath = SharedBodyPath;
        Profile.FirstPersonArmsObjectPath = SharedArmsPath;
        Profile.bFactionUniqueBody = false;
        Profile.bFactionUniqueArms = false;
        return Profile;
    }
}

TArray<FOCCharacterProductionProfile> OCGetDeclaredCharacterProductionProfiles()
{
    return
    {
        MakeSharedQuantumProfile(EOCFactionArchetype::UASpecialUnit, TEXT("UA Special Unit")),
        MakeSharedQuantumProfile(EOCFactionArchetype::MaskedFighters, TEXT("Masked Fighters")),
        MakeSharedQuantumProfile(EOCFactionArchetype::USRangers, TEXT("US Rangers Style")),
        MakeSharedQuantumProfile(EOCFactionArchetype::Insurgents, TEXT("Insurgents")),
    };
}

bool OCHasDeclaredCharacterProductionProfile(const EOCFactionArchetype Faction)
{
    for (const FOCCharacterProductionProfile& Profile : OCGetDeclaredCharacterProductionProfiles())
    {
        if (Profile.Faction == Faction)
        {
            return true;
        }
    }
    return false;
}

FOCCharacterProductionProfile OCResolveCharacterProductionProfile(const EOCFactionArchetype Faction)
{
    for (const FOCCharacterProductionProfile& Profile : OCGetDeclaredCharacterProductionProfiles())
    {
        if (Profile.Faction == Faction)
        {
            return Profile;
        }
    }

    return MakeSharedQuantumProfile(Faction, TEXT("Unknown faction"));
}

TArray<FOCCharacterProductionModule> OCGetDeclaredCharacterProductionModules()
{
    return
    {
        { FName(TEXT("Arms")), TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Arms.SKM_Arms"), EOCCharacterProductionModuleType::Skeletal, true },
        { FName(TEXT("Head")), TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Head.SKM_Head"), EOCCharacterProductionModuleType::Skeletal, false },
        { FName(TEXT("BulletproofBeige")), TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Bulletproof_Bege.SKM_Bulletproof_Bege"), EOCCharacterProductionModuleType::Skeletal, true },
        { FName(TEXT("DropsBeige")), TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Drops_1_Bege.SKM_Drops_1_Bege"), EOCCharacterProductionModuleType::Skeletal, true },
        { FName(TEXT("HolsterHardBeige")), TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Holster_Hard_Bege.SKM_Holster_Hard_Bege"), EOCCharacterProductionModuleType::Skeletal, true },
        { FName(TEXT("Jeans")), TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Jeans.SKM_Jeans"), EOCCharacterProductionModuleType::Skeletal, false },
        { FName(TEXT("BackPatch")), TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Patch_Back.SKM_Patch_Back"), EOCCharacterProductionModuleType::Skeletal, false },
        { FName(TEXT("RolledUpBlueShirt")), TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Shirt_RolledUp_Blue.SKM_Shirt_RolledUp_Blue"), EOCCharacterProductionModuleType::Skeletal, false },
        { FName(TEXT("CapBeige")), TEXT("/Game/QuantumCharacter/Mesh/Modules/SM_Cap_Bege.SM_Cap_Bege"), EOCCharacterProductionModuleType::Static, true },
    };
}
