#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "OCCharacterProductionProfiles.h"

#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOCR14CharacterProductionProfilesTest,
    "OsterConflict.R14.Characters.ProductionProfiles",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOCR14CharacterProductionProfilesTest::RunTest(const FString& Parameters)
{
    const EOCFactionArchetype ExpectedFactions[] =
    {
        EOCFactionArchetype::UASpecialUnit,
        EOCFactionArchetype::MaskedFighters,
        EOCFactionArchetype::USRangers,
        EOCFactionArchetype::Insurgents,
    };

    const FString ExpectedBodyPath = TEXT("/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter.SKM_QuantumCharacter");
    const FString ExpectedArmsPath = TEXT("/Game/QuantumCharacter/Mesh/Modules/SKM_Arms.SKM_Arms");

    for (const EOCFactionArchetype Faction : ExpectedFactions)
    {
        TestTrue(TEXT("Faction has a declared R14 production profile"),
            OCHasDeclaredCharacterProductionProfile(Faction));

        const FOCCharacterProductionProfile Profile = OCResolveCharacterProductionProfile(Faction);
        TestTrue(TEXT("Resolved character profile keeps requested faction"), Profile.Faction == Faction);
        TestTrue(TEXT("Current faction body path stays on verified QuantumCharacter base"),
            Profile.ThirdPersonBodyObjectPath == ExpectedBodyPath);
        TestTrue(TEXT("Current faction FP arms path stays on verified QuantumCharacter base"),
            Profile.FirstPersonArmsObjectPath == ExpectedArmsPath);
        TestFalse(TEXT("Shared QuantumCharacter body is not falsely marked faction-unique"),
            Profile.bFactionUniqueBody);
        TestFalse(TEXT("Shared QuantumCharacter arms are not falsely marked faction-unique"),
            Profile.bFactionUniqueArms);
    }

    struct FExpectedRole
    {
        EOCPlayerRole Role;
        EOCCharacterGearClass PrimaryGearClass;
        bool bAllowsLightGearVariant;
    };

    const FExpectedRole ExpectedRoles[] =
    {
        { EOCPlayerRole::Rifleman, EOCCharacterGearClass::Standard, true },
        { EOCPlayerRole::Medic, EOCCharacterGearClass::Standard, false },
        { EOCPlayerRole::Engineer, EOCCharacterGearClass::Heavy, false },
        { EOCPlayerRole::Support, EOCCharacterGearClass::Heavy, false },
    };

    for (const FExpectedRole& Expected : ExpectedRoles)
    {
        TestTrue(TEXT("Gameplay role has a declared R14 production profile"),
            OCHasDeclaredCharacterRoleProductionProfile(Expected.Role));

        const FOCCharacterRoleProductionProfile Profile = OCResolveCharacterRoleProductionProfile(Expected.Role);
        TestTrue(TEXT("Resolved role profile keeps requested authoritative role"), Profile.Role == Expected.Role);
        TestTrue(TEXT("Role profile preserves current runtime GearClass behavior"),
            Profile.PrimaryGearClass == Expected.PrimaryGearClass);
        TestTrue(TEXT("Role profile preserves current Rifleman-only light gear variation rule"),
            Profile.bAllowsLightGearVariant == Expected.bAllowsLightGearVariant);
        TestFalse(TEXT("Shared current gear is not falsely marked role-unique"), Profile.bRoleUniqueVisual);
    }

    TestTrue(TEXT("Engineer remains heavy gear in the R14 production contract"),
        OCResolveCharacterRoleProductionProfile(EOCPlayerRole::Engineer).PrimaryGearClass == EOCCharacterGearClass::Heavy);
    TestTrue(TEXT("Support remains heavy gear in the R14 production contract"),
        OCResolveCharacterRoleProductionProfile(EOCPlayerRole::Support).PrimaryGearClass == EOCCharacterGearClass::Heavy);
    TestFalse(TEXT("Engineer visual is not yet falsely declared role-unique"),
        OCResolveCharacterRoleProductionProfile(EOCPlayerRole::Engineer).bRoleUniqueVisual);
    TestFalse(TEXT("Support visual is not yet falsely declared role-unique"),
        OCResolveCharacterRoleProductionProfile(EOCPlayerRole::Support).bRoleUniqueVisual);

    USkeletalMesh* Body = LoadObject<USkeletalMesh>(nullptr, *ExpectedBodyPath);
    USkeletalMesh* Arms = LoadObject<USkeletalMesh>(nullptr, *ExpectedArmsPath);
    TestNotNull(TEXT("QuantumCharacter production body loads"), Body);
    TestNotNull(TEXT("QuantumCharacter first-person arms load"), Arms);

    if (Body)
    {
        TestNotNull(TEXT("QuantumCharacter production body has skeleton"), Body->GetSkeleton());
        TestTrue(TEXT("QuantumCharacter production body has material slots"), Body->GetMaterials().Num() > 0);
        TestTrue(TEXT("QuantumCharacter production body has render LOD"), Body->GetLODNum() > 0);
    }
    if (Arms)
    {
        TestNotNull(TEXT("QuantumCharacter first-person arms have skeleton"), Arms->GetSkeleton());
        TestTrue(TEXT("QuantumCharacter first-person arms have material slots"), Arms->GetMaterials().Num() > 0);
        TestTrue(TEXT("QuantumCharacter first-person arms have render LOD"), Arms->GetLODNum() > 0);
    }

    const TArray<FOCCharacterProductionModule> Modules = OCGetDeclaredCharacterProductionModules();
    TestTrue(TEXT("R14 character registry contains all audited QuantumCharacter modules"), Modules.Num() >= 10);

    bool bFoundNoHeadBody = false;
    for (const FOCCharacterProductionModule& Module : Modules)
    {
        TestFalse(TEXT("Character production module has a non-empty id"), Module.ModuleId.IsNone());
        TestFalse(TEXT("Character production module has a non-empty object path"), Module.ObjectPath.IsEmpty());
        if (Module.ModuleId == FName(TEXT("BodyNoHead")))
        {
            bFoundNoHeadBody = true;
            TestTrue(TEXT("BodyNoHead keeps its canonical modular body path"),
                Module.ObjectPath == TEXT("/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter_NoHead.SKM_QuantumCharacter_NoHead"));
            TestFalse(TEXT("BodyNoHead is only an audited candidate, not falsely marked runtime-active"),
                Module.bCurrentlyUsedByRuntimeSubsystem);
        }

        if (Module.Type == EOCCharacterProductionModuleType::Skeletal)
        {
            USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, *Module.ObjectPath);
            TestNotNull(FString::Printf(TEXT("%s skeletal module loads"), *Module.ModuleId.ToString()), Mesh);
            if (Mesh)
            {
                TestNotNull(FString::Printf(TEXT("%s skeletal module has skeleton"), *Module.ModuleId.ToString()), Mesh->GetSkeleton());
                TestTrue(FString::Printf(TEXT("%s skeletal module has material slots"), *Module.ModuleId.ToString()),
                    Mesh->GetMaterials().Num() > 0);
                TestTrue(FString::Printf(TEXT("%s skeletal module has render LOD"), *Module.ModuleId.ToString()),
                    Mesh->GetLODNum() > 0);
            }
        }
        else
        {
            UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *Module.ObjectPath);
            TestNotNull(FString::Printf(TEXT("%s static module loads"), *Module.ModuleId.ToString()), Mesh);
            if (Mesh)
            {
                TestTrue(FString::Printf(TEXT("%s static module has material slots"), *Module.ModuleId.ToString()),
                    Mesh->GetStaticMaterials().Num() > 0);
                TestTrue(FString::Printf(TEXT("%s static module has render LOD"), *Module.ModuleId.ToString()),
                    Mesh->GetNumLODs() > 0);
            }
        }
    }
    TestTrue(TEXT("R14 character registry explicitly contains BodyNoHead modular candidate"), bFoundNoHeadBody);

    return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS
