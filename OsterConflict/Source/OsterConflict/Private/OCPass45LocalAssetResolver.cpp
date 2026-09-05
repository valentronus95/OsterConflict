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
                return !ContainsPreferredToken(Asset, PreferredTokens);
            });
            if (Assets.IsEmpty()) return FAssetData();
        }

        auto Score = [&PreferredTokens](const FAssetData& Asset)
        {
            const FString Candidate = (Asset.PackageName.ToString() + TEXT("/") + Asset.AssetName.ToString()).ToLower();
            int32 Result = 0;
            for (const FString& RawToken : PreferredTokens)
            {
                const FString Token = RawToken.ToLower();
                if (!Token.IsEmpty() && Candidate.Contains(Token)) Result += 10;
            }
            // Prefer actual mesh/animation assets over obvious collision, proxy, LOD or preview helpers.
            if (Candidate.Contains(TEXT("collision")) || Candidate.Contains(TEXT("proxy")) ||
                Candidate.Contains(TEXT("preview"))) Result -= 12;
            if (Candidate.Contains(TEXT("lod"))) Result -= 3;
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
