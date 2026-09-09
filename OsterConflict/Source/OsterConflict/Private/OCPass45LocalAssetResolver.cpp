#include "OCPass45LocalAssetResolver.h"

#include "Animation/AnimSequence.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Modules/ModuleManager.h"

namespace
{
    bool ContainsPreferredToken(const FAssetData& Asset, const TArray<FString>& PreferredTokens)
    {
        if (PreferredTokens.IsEmpty()) return true;
        const FString Candidate = (Asset.PackageName.ToString() + TEXT("/") + Asset.AssetName.ToString()).ToLower();
        for (const FString& RawToken : PreferredTokens)
        {
            const FString Token = RawToken.ToLower();
            if (!Token.IsEmpty() && Candidate.Contains(Token)) return true;
        }
        return false;
    }

    bool IsObviousWeaponPartOrHelper(const FAssetData& Asset)
    {
        const FString Name = Asset.AssetName.ToString().ToLower();
        const FString Path = Asset.PackageName.ToString().ToLower();
        const FString Candidate = Path + TEXT("/") + Name;

        static const TCHAR* RejectedTerms[] =
        {
            TEXT("collision"), TEXT("proxy"), TEXT("preview"), TEXT("socket"),
            TEXT("scope"), TEXT("sight"), TEXT("optic"), TEXT("reticle"),
            TEXT("magazine"), TEXT("_mag"), TEXT("mag_"), TEXT("ammo"),
            TEXT("bullet"), TEXT("cartridge"), TEXT("shell"), TEXT("casing"), TEXT("projectile"),
            TEXT("grenade"), TEXT("rocket"), TEXT("warhead"),
            TEXT("muzzle"), TEXT("silencer"), TEXT("suppressor"), TEXT("flash_hider"),
            TEXT("tube"), TEXT("barrel"), TEXT("trigger"), TEXT("handguard"), TEXT("bayonet"),
            TEXT("bolt_only"), TEXT("stock_only")
        };

        for (const TCHAR* Term : RejectedTerms)
        {
            if (Candidate.Contains(Term, ESearchCase::IgnoreCase)) return true;
        }
        return false;
    }

    FAssetData FindBestAsset(
        const UClass* AssetClass,
        const TArray<FName>& PackageRoots,
        const TArray<FString>& PreferredTokens,
        bool bRequireTokenMatch)
    {
        if (!AssetClass || PackageRoots.IsEmpty()) return FAssetData();

        FARFilter Filter;
        Filter.ClassPaths.Add(AssetClass->GetClassPathName());
        Filter.PackagePaths.Append(PackageRoots);
        Filter.bRecursivePaths = true;
        Filter.bRecursiveClasses = true;

        FAssetRegistryModule& AssetRegistryModule =
            FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
        TArray<FAssetData> Assets;
        AssetRegistryModule.Get().GetAssets(Filter, Assets);
        if (Assets.IsEmpty()) return FAssetData();

        if (bRequireTokenMatch)
        {
            Assets.RemoveAll([&PreferredTokens](const FAssetData& Asset)
            {
                return !ContainsPreferredToken(Asset, PreferredTokens) || IsObviousWeaponPartOrHelper(Asset);
            });
            if (Assets.IsEmpty()) return FAssetData();
        }

        auto Score = [&PreferredTokens](const FAssetData& Asset)
        {
            const FString Package = Asset.PackageName.ToString().ToLower();
            const FString Name = Asset.AssetName.ToString().ToLower();
            const FString Candidate = Package + TEXT("/") + Name;
            int32 Result = 0;
            for (const FString& RawToken : PreferredTokens)
            {
                const FString Token = RawToken.ToLower();
                if (Token.IsEmpty()) continue;
                if (Name.Contains(Token)) Result += 80;
                else if (Package.Contains(Token)) Result += 10;
            }

            // Prefer complete firearm assets. A token in a package folder alone is not proof that a child mesh
            // is the weapon itself; modular packs commonly contain sights, barrels, magazines and projectiles.
            if (Name.Contains(TEXT("weapon")) || Name.Contains(TEXT("rifle")) ||
                Name.Contains(TEXT("shotgun")) || Name.Contains(TEXT("pistol")) ||
                Name.Contains(TEXT("smg")) || Name.Contains(TEXT("launcher")) ||
                Name.Contains(TEXT("assembled")) || Name.Contains(TEXT("complete")))
            {
                Result += 24;
            }
            if (Name.StartsWith(TEXT("sk_")) || Name.StartsWith(TEXT("sm_"))) Result += 4;
            if (Candidate.Contains(TEXT("lod"))) Result -= 3;
            if (IsObviousWeaponPartOrHelper(Asset)) Result -= 1000;
            return Result;
        };

        Assets.Sort([&Score](const FAssetData& A, const FAssetData& B)
        {
            const int32 ScoreA = Score(A);
            const int32 ScoreB = Score(B);
            if (ScoreA != ScoreB) return ScoreA > ScoreB;
            return A.GetObjectPathString() < B.GetObjectPathString();
        });
        return Assets[0];
    }

    FSoftObjectPath ResolvePath(
        const UClass* AssetClass,
        const TArray<FName>& PackageRoots,
        const TArray<FString>& PreferredTokens,
        bool bRequireTokenMatch)
    {
        const FAssetData Best = FindBestAsset(AssetClass, PackageRoots, PreferredTokens, bRequireTokenMatch);
        return Best.IsValid() ? FSoftObjectPath(Best.GetObjectPathString()) : FSoftObjectPath();
    }

    template <typename TObjectType>
    TObjectType* Resolve(
        const UClass* AssetClass,
        const TArray<FName>& PackageRoots,
        const TArray<FString>& PreferredTokens,
        bool bRequireTokenMatch = false)
    {
        const FAssetData Best = FindBestAsset(AssetClass, PackageRoots, PreferredTokens, bRequireTokenMatch);
        return Best.IsValid() ? Cast<TObjectType>(Best.GetAsset()) : nullptr;
    }
}

UStaticMesh* OCPass45FindLocalStaticMesh(
    const TArray<FName>& PackageRoots,
    const TArray<FString>& PreferredTokens)
{
    return Resolve<UStaticMesh>(UStaticMesh::StaticClass(), PackageRoots, PreferredTokens);
}

UStaticMesh* OCPass45FindLocalStaticMeshStrict(
    const TArray<FName>& PackageRoots,
    const TArray<FString>& RequiredTokens)
{
    return Resolve<UStaticMesh>(UStaticMesh::StaticClass(), PackageRoots, RequiredTokens, true);
}

FSoftObjectPath OCPass45FindLocalStaticMeshPathStrict(
    const TArray<FName>& PackageRoots,
    const TArray<FString>& RequiredTokens)
{
    return ResolvePath(UStaticMesh::StaticClass(), PackageRoots, RequiredTokens, true);
}

USkeletalMesh* OCPass45FindLocalSkeletalMesh(
    const TArray<FName>& PackageRoots,
    const TArray<FString>& PreferredTokens)
{
    return Resolve<USkeletalMesh>(USkeletalMesh::StaticClass(), PackageRoots, PreferredTokens);
}

USkeletalMesh* OCPass45FindLocalSkeletalMeshStrict(
    const TArray<FName>& PackageRoots,
    const TArray<FString>& RequiredTokens)
{
    return Resolve<USkeletalMesh>(USkeletalMesh::StaticClass(), PackageRoots, RequiredTokens, true);
}

FSoftObjectPath OCPass45FindLocalSkeletalMeshPathStrict(
    const TArray<FName>& PackageRoots,
    const TArray<FString>& RequiredTokens)
{
    return ResolvePath(USkeletalMesh::StaticClass(), PackageRoots, RequiredTokens, true);
}

UAnimSequence* OCPass45FindLocalAnimation(
    const TArray<FName>& PackageRoots,
    const TArray<FString>& PreferredTokens)
{
    return Resolve<UAnimSequence>(UAnimSequence::StaticClass(), PackageRoots, PreferredTokens);
}

UTexture2D* OCPass45FindLocalTexture(
    const TArray<FName>& PackageRoots,
    const TArray<FString>& PreferredTokens)
{
    return Resolve<UTexture2D>(UTexture2D::StaticClass(), PackageRoots, PreferredTokens);
}
