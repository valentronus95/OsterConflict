#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "OCWeaponAnimationProfiles.h"
#include "OCWeaponPresentationProfiles.h"

#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOCR14WeaponProfileContractsTest,
    "OsterConflict.R14.Weapons.ProfileContracts",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOCR14WeaponProfileContractsTest::RunTest(const FString& Parameters)
{
    const FName ExpectedWeaponIds[] =
    {
        FName(TEXT("OC_AR1")),
        FName(TEXT("OC_SMG1")),
        FName(TEXT("OC_PST1")),
        FName(TEXT("OC_SNP1")),
        FName(TEXT("OC_SG1")),
        FName(TEXT("OC_LMG1")),
        FName(TEXT("R13_M14")),
        FName(TEXT("R13_MAC10")),
        FName(TEXT("R13_TEC9")),
        FName(TEXT("R13_LEVER4570")),
        FName(TEXT("OC_RPG1")),
    };

    for (const FName WeaponId : ExpectedWeaponIds)
    {
        TestTrue(
            FString::Printf(TEXT("%s has a declared first-person profile"), *WeaponId.ToString()),
            OCHasDeclaredFirstPersonWeaponProfile(WeaponId));

        const FOCFirstPersonWeaponProfile GripProfile = OCResolveFirstPersonWeaponProfile(WeaponId);
        TestTrue(
            FString::Printf(TEXT("%s grip profile resolves to the requested id"), *WeaponId.ToString()),
            GripProfile.WeaponId == WeaponId);
        TestFalse(
            FString::Printf(TEXT("%s grip profile location contains no NaN"), *WeaponId.ToString()),
            GripProfile.CameraLocation.ContainsNaN());
        TestFalse(
            FString::Printf(TEXT("%s grip profile rotation contains no NaN"), *WeaponId.ToString()),
            GripProfile.CameraRotation.ContainsNaN());

        TestTrue(
            FString::Printf(TEXT("%s has a declared animation profile"), *WeaponId.ToString()),
            OCHasDeclaredWeaponAnimationProfile(WeaponId));

        const FOCWeaponAnimationProfile AnimationProfile = OCResolveWeaponAnimationProfile(WeaponId);
        TestTrue(
            FString::Printf(TEXT("%s animation profile resolves to the requested id"), *WeaponId.ToString()),
            AnimationProfile.WeaponId == WeaponId);

        if (AnimationProfile.HasFireAnimation())
        {
            UAnimSequence* Fire = LoadObject<UAnimSequence>(nullptr, *AnimationProfile.FireAnimationObjectPath);
            TestNotNull(
                FString::Printf(TEXT("%s declared fire animation loads"), *WeaponId.ToString()),
                Fire);
        }

        if (AnimationProfile.HasReloadAnimation())
        {
            UAnimSequence* Reload = LoadObject<UAnimSequence>(nullptr, *AnimationProfile.ReloadAnimationObjectPath);
            TestNotNull(
                FString::Printf(TEXT("%s declared reload animation loads"), *WeaponId.ToString()),
                Reload);
        }
    }

    const FOCWeaponAnimationProfile AKProfile = OCResolveWeaponAnimationProfile(FName(TEXT("OC_AR1")));
    TestTrue(TEXT("AK-47 has complete authored Fire/Reload coverage"), AKProfile.HasCompleteAuthoredCoverage());
    TestTrue(
        TEXT("AK-47 fire path stays canonical"),
        AKProfile.FireAnimationObjectPath == TEXT("/Game/AK-47/Animations/AK-47_Fire_W.AK-47_Fire_W"));
    TestTrue(
        TEXT("AK-47 reload path stays canonical"),
        AKProfile.ReloadAnimationObjectPath == TEXT("/Game/AK-47/Animations/AK-47_Reload_W.AK-47_Reload_W"));

    UAnimSequence* AKFire = LoadObject<UAnimSequence>(nullptr, *AKProfile.FireAnimationObjectPath);
    UAnimSequence* AKReload = LoadObject<UAnimSequence>(nullptr, *AKProfile.ReloadAnimationObjectPath);
    USkeletalMesh* AKMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/AK-47/Mesh/SKM_AK-47.SKM_AK-47"));

    TestNotNull(TEXT("AK-47 production skeletal mesh loads for animation contract"), AKMesh);
    if (AKMesh && AKMesh->GetSkeleton())
    {
        if (AKFire)
        {
            TestTrue(TEXT("AK-47 fire animation uses the production weapon skeleton"),
                AKFire->GetSkeleton() == AKMesh->GetSkeleton());
        }
        if (AKReload)
        {
            TestTrue(TEXT("AK-47 reload animation uses the production weapon skeleton"),
                AKReload->GetSkeleton() == AKMesh->GetSkeleton());
        }
    }

    TestTrue(
        TEXT("Remington 870 profile records articulated pump requirement"),
        OCResolveWeaponAnimationProfile(FName(TEXT("OC_SG1"))).bRequiresArticulatedWeapon);
    TestTrue(
        TEXT("M249 profile records articulated belt/magazine requirement"),
        OCResolveWeaponAnimationProfile(FName(TEXT("OC_LMG1"))).bRequiresArticulatedWeapon);

    return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS
