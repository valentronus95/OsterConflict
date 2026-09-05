#pragma once

#include "CoreMinimal.h"

class UAnimSequence;
class USkeletalMesh;
class UStaticMesh;
class UTexture2D;

/**
 * Deterministic, fail-soft resolver for already-imported local Fab/content packages.
 * The public repository intentionally does not carry every third-party package, so runtime code
 * must discover a suitable local asset when it exists and preserve the tracked fallback when it does not.
 */
OSTERCONFLICT_API UStaticMesh* OCPass45FindLocalStaticMesh(
    const TArray<FName>& PackageRoots,
    const TArray<FString>& PreferredTokens = {});

/** Same resolver, but returns null unless at least one preferred token matches the asset path/name. */
OSTERCONFLICT_API UStaticMesh* OCPass45FindLocalStaticMeshStrict(
    const TArray<FName>& PackageRoots,
    const TArray<FString>& RequiredTokens);

OSTERCONFLICT_API USkeletalMesh* OCPass45FindLocalSkeletalMesh(
    const TArray<FName>& PackageRoots,
    const TArray<FString>& PreferredTokens = {});

/** Skeletal equivalent of the strict static-mesh resolver. */
OSTERCONFLICT_API USkeletalMesh* OCPass45FindLocalSkeletalMeshStrict(
    const TArray<FName>& PackageRoots,
    const TArray<FString>& RequiredTokens);

OSTERCONFLICT_API UAnimSequence* OCPass45FindLocalAnimation(
    const TArray<FName>& PackageRoots,
    const TArray<FString>& PreferredTokens = {});

OSTERCONFLICT_API UTexture2D* OCPass45FindLocalTexture(
    const TArray<FName>& PackageRoots,
    const TArray<FString>& PreferredTokens = {});
