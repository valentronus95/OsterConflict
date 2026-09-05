#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OCLocalInboxRuntimeSubsystem.generated.h"

class AStaticMeshActor;
class UStaticMesh;
class USkeletalMesh;
class UTexture2D;
class UUserWidget;

/**
 * Runtime bridge for user-supplied models_game_OC content.
 * The local intake writes Saved/LocalModelInbox/runtime_bindings.json after UE has actually loaded/imported
 * every supported source. This subsystem consumes that factual manifest instead of hard-coding filenames.
 */
UCLASS()
class OSTERCONFLICT_API UOCLocalInboxRuntimeSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

    /** Returns a loadable UObject path (Package.Asset) for the first bound mesh in a category. */
    static bool ResolveFirstAssetObjectPathForCategory(const FString& Category, FString& OutObjectPath,
        bool bRequireSkeletal = false, bool bRequireCharacterCompatible = false);

    static UStaticMesh* LoadFirstStaticMeshForCategory(const FString& Category);
    static void GetAssetObjectPathsForCategory(const FString& Category, TArray<FString>& OutPaths);
    static USkeletalMesh* LoadCompatibleCharacterSkin(int32 Index);
    static int32 GetCompatibleCharacterSkinCount();
    static UTexture2D* LoadHUDTexture();
    static UClass* LoadHUDWidgetClass();

private:
    void ApplyRuntimeBindings();
    void SpawnValidationShowcase();
    void CycleValidationShowcase();
    void WriteRuntimeReport(bool bPass, const FString& Detail) const;

    bool bValidateLocalInbox = false;
    bool bBindingsApplied = false;
    int32 ShowcaseCursor = 0;
    TArray<FString> BoundStaticObjectPaths;
    TArray<TWeakObjectPtr<AStaticMeshActor>> ShowcaseActors;
    TWeakObjectPtr<UUserWidget> LocalHUDWidget;
    FTimerHandle ApplyTimer;
    FTimerHandle ShowcaseTimer;
};
