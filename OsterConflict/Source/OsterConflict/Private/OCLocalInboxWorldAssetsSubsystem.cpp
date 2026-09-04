#include "OCLocalInboxWorldAssetsSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    struct FLocalWorldAsset
    {
        FString ObjectPath;
        FString Source;
        FString Category;
    };

    FString BindingManifestPath()
    {
        return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("LocalModelInbox"), TEXT("runtime_bindings.json"));
    }

    FString WorldRuntimeReportPath()
    {
        return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("AutomationReports"), TEXT("ProductionModels"),
            TEXT("local_world_runtime_validation.txt"));
    }

    void WriteWorldReport(const bool bPass, const FString& Detail)
    {
        if (!FParse::Param(FCommandLine::Get(), TEXT("ValidateLocalInbox"))) return;
        const FString Path = WorldRuntimeReportPath();
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
        const FString Text = FString::Printf(TEXT("PASS45_LOCAL_WORLD_RUNTIME=%s\n%s\n"),
            bPass ? TEXT("PASS") : TEXT("FAIL"), *Detail);
        FFileHelper::SaveStringToFile(Text, *Path);
    }

    FString PackageToObjectPath(const FString& InPath)
    {
        if (InPath.IsEmpty() || InPath.Contains(TEXT("."))) return InPath;
        int32 Slash = INDEX_NONE;
        const FString AssetName = InPath.FindLastChar(TEXT('/'), Slash) ? InPath.Mid(Slash + 1) : InPath;
        return InPath + TEXT(".") + AssetName;
    }

    bool LoadWorldAssets(TArray<FLocalWorldAsset>& OutAssets)
    {
        FString Text;
        if (!FFileHelper::LoadFileToString(Text, *BindingManifestPath())) return false;

        TSharedPtr<FJsonObject> Root;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
        if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid()) return false;

        const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
        if (!Root->TryGetArrayField(TEXT("static_assets"), Values) || !Values) return true;

        for (const TSharedPtr<FJsonValue>& Value : *Values)
        {
            const TSharedPtr<FJsonObject>* Obj = nullptr;
            if (!Value.IsValid() || !Value->TryGetObject(Obj) || !Obj || !Obj->IsValid()) continue;

            FString Path;
            FString Category;
            FString Source;
            if (!(*Obj)->TryGetStringField(TEXT("path"), Path) || Path.IsEmpty()) continue;
            (*Obj)->TryGetStringField(TEXT("category"), Category);
            (*Obj)->TryGetStringField(TEXT("source"), Source);

            if (!Category.Equals(TEXT("BUILDING_WORLD"), ESearchCase::IgnoreCase) &&
                !Category.Equals(TEXT("FOLIAGE"), ESearchCase::IgnoreCase) &&
                !Category.Equals(TEXT("PROP"), ESearchCase::IgnoreCase) &&
                !Category.Equals(TEXT("ROAD_WORLD"), ESearchCase::IgnoreCase) &&
                !Category.Equals(TEXT("WATER_WORLD"), ESearchCase::IgnoreCase) &&
                !Category.Equals(TEXT("GROUND_WORLD"), ESearchCase::IgnoreCase) &&
                !Category.Equals(TEXT("UNCLASSIFIED"), ESearchCase::IgnoreCase))
            {
                continue;
            }

            FLocalWorldAsset& Asset = OutAssets.AddDefaulted_GetRef();
            Asset.ObjectPath = PackageToObjectPath(Path);
            Asset.Source = Source;
            Asset.Category = Category;
        }
        return true;
    }

    bool ContainsAny(const FString& Text, const TArray<FString>& Needles)
    {
        for (const FString& Needle : Needles)
        {
            if (Text.Contains(Needle, ESearchCase::IgnoreCase)) return true;
        }
        return false;
    }

    TArray<UInstancedStaticMeshComponent*> FindSourceComponents(AOCWorldSectorOster& Sector,
        const TArray<FString>& Names)
    {
        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Sector.GetComponents(Components);

        TArray<UInstancedStaticMeshComponent*> Result;
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (Component && Names.Contains(Component->GetName())) Result.Add(Component);
        }
        return Result;
    }

    int32 BindMeshPool(AOCWorldSectorOster& Sector, const TArray<FLocalWorldAsset>& Assets,
        const TArray<FString>& SourceComponentNames, const TCHAR* PoolTag,
        int32& OutLoadedAssets, int32& OutUsedAssets)
    {
        OutLoadedAssets = 0;
        OutUsedAssets = 0;
        if (Assets.IsEmpty()) return 0;

        const TArray<UInstancedStaticMeshComponent*> Sources = FindSourceComponents(Sector, SourceComponentNames);
        if (Sources.IsEmpty()) return 0;

        TArray<FTransform> AuthoredTransforms;
        for (UInstancedStaticMeshComponent* Source : Sources)
        {
            if (!Source) continue;
            for (int32 Index = 0; Index < Source->GetInstanceCount(); ++Index)
            {
                FTransform Transform;
                if (Source->GetInstanceTransform(Index, Transform, false)) AuthoredTransforms.Add(Transform);
            }
        }
        if (AuthoredTransforms.IsEmpty()) return 0;

        struct FPoolMesh
        {
            TObjectPtr<UStaticMesh> Mesh = nullptr;
            float Normalization = 1.0f;
            TObjectPtr<UInstancedStaticMeshComponent> Component = nullptr;
        };
        TArray<FPoolMesh> Loaded;

        for (const FLocalWorldAsset& Asset : Assets)
        {
            UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *Asset.ObjectPath);
            if (!Mesh) continue;

            const FVector NativeSize = Mesh->GetBounds().BoxExtent * 2.0f;
            const float Longest = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
            if (Longest <= 1.0f) continue;

            FPoolMesh& Item = Loaded.AddDefaulted_GetRef();
            Item.Mesh = Mesh;
            // The source-only Oster geometry is authored with 100 cm Engine BasicShapes.
            // Normalise arbitrary imported meshes to that same basis before reusing authored transforms.
            Item.Normalization = 100.0f / Longest;

            const FName ComponentName = MakeUniqueObjectName(
                &Sector, UInstancedStaticMeshComponent::StaticClass(),
                FName(*FString::Printf(TEXT("OC_Local_%s"), PoolTag)));
            UInstancedStaticMeshComponent* Visual = NewObject<UInstancedStaticMeshComponent>(&Sector, ComponentName);
            if (!Visual) { Loaded.Pop(); continue; }

            Visual->SetupAttachment(Sector.GetRootComponent());
            Visual->SetStaticMesh(Mesh);
            Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Visual->SetGenerateOverlapEvents(false);
            Visual->SetCanEverAffectNavigation(false);
            Visual->SetCastShadow(true);
            Visual->ComponentTags.Add(FName(TEXT("OC_LocalInboxWorldVisual")));
            Sector.AddInstanceComponent(Visual);
            Visual->RegisterComponent();
            Item.Component = Visual;
        }

        OutLoadedAssets = Loaded.Num();
        if (Loaded.IsEmpty()) return 0;

        TSet<int32> UsedMeshIndices;
        int32 ReplacedInstances = 0;
        for (int32 Index = 0; Index < AuthoredTransforms.Num(); ++Index)
        {
            const int32 MeshIndex = Index % Loaded.Num();
            FPoolMesh& Item = Loaded[MeshIndex];
            if (!Item.Mesh || !Item.Component) continue;

            FTransform Transform = AuthoredTransforms[Index];
            Transform.SetScale3D(Transform.GetScale3D() * Item.Normalization);
            Item.Component->AddInstance(Transform, false);
            UsedMeshIndices.Add(MeshIndex);
            ++ReplacedInstances;
        }
        OutUsedAssets = UsedMeshIndices.Num();

        if (ReplacedInstances > 0)
        {
            // Keep source collision/semantic ownership, retire only its visible BasicShape presentation.
            for (UInstancedStaticMeshComponent* Source : Sources)
            {
                if (!Source) continue;
                Source->SetVisibility(false, true);
                Source->SetHiddenInGame(true, true);
                Source->SetCastShadow(false);
            }
        }

        return ReplacedInstances;
    }
}

bool UOCLocalInboxWorldAssetsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCLocalInboxWorldAssetsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    InWorld.GetTimerManager().SetTimer(ApplyTimer, this,
        &UOCLocalInboxWorldAssetsSubsystem::ApplyWorldAssets, 0.70f, false);
}

void UOCLocalInboxWorldAssetsSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(ApplyTimer);
    Super::Deinitialize();
}

void UOCLocalInboxWorldAssetsSubsystem::ApplyWorldAssets()
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<FLocalWorldAsset> AllAssets;
    if (!LoadWorldAssets(AllAssets))
    {
        UE_LOG(LogTemp, Verbose, TEXT("PASS45_LOCAL_WORLD_ASSETS_SKIP manifest_missing=%s"), *BindingManifestPath());
        return;
    }

    TArray<FLocalWorldAsset> Buildings;
    TArray<FLocalWorldAsset> TreeFoliage;
    TArray<FLocalWorldAsset> GroundFoliage;
    TArray<FLocalWorldAsset> Fences;
    TArray<FLocalWorldAsset> Bridges;
    TArray<FLocalWorldAsset> Roads;
    TArray<FLocalWorldAsset> Water;
    TArray<FLocalWorldAsset> Props;

    const TArray<FString> GroundWords = { TEXT("grass"), TEXT("flower"), TEXT("fern"), TEXT("heather"), TEXT("yarrow"), TEXT("leaf"), TEXT("moss") };
    const TArray<FString> FenceWords = { TEXT("fence"), TEXT("wall"), TEXT("gate"), TEXT("barrier"), TEXT("паркан") };
    const TArray<FString> BridgeWords = { TEXT("bridge"), TEXT("міст") };

    for (const FLocalWorldAsset& Asset : AllAssets)
    {
        const FString Hint = Asset.Source + TEXT(" ") + Asset.ObjectPath;
        if (Asset.Category.Equals(TEXT("BUILDING_WORLD"), ESearchCase::IgnoreCase)) Buildings.Add(Asset);
        else if (Asset.Category.Equals(TEXT("FOLIAGE"), ESearchCase::IgnoreCase))
        {
            if (ContainsAny(Hint, GroundWords)) GroundFoliage.Add(Asset);
            else TreeFoliage.Add(Asset);
        }
        else if (Asset.Category.Equals(TEXT("PROP"), ESearchCase::IgnoreCase))
        {
            if (ContainsAny(Hint, FenceWords)) Fences.Add(Asset);
            else if (ContainsAny(Hint, BridgeWords)) Bridges.Add(Asset);
            else Props.Add(Asset);
        }
        else if (Asset.Category.Equals(TEXT("ROAD_WORLD"), ESearchCase::IgnoreCase)) Roads.Add(Asset);
        else if (Asset.Category.Equals(TEXT("WATER_WORLD"), ESearchCase::IgnoreCase)) Water.Add(Asset);
        else if (Asset.Category.Equals(TEXT("GROUND_WORLD"), ESearchCase::IgnoreCase)) GroundFoliage.Add(Asset);
        else Props.Add(Asset);
    }

    int32 TotalReplaced = 0;
    int32 TotalLoadedAssets = 0;
    int32 TotalUsedAssets = 0;
    int32 SectorCount = 0;

    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        AOCWorldSectorOster& Sector = **It;
        ++SectorCount;

        int32 Loaded = 0;
        int32 Used = 0;
        TotalReplaced += BindMeshPool(Sector, Buildings,
            { TEXT("Buildings") }, TEXT("Buildings"), Loaded, Used);
        TotalLoadedAssets += Loaded; TotalUsedAssets += Used;

        TotalReplaced += BindMeshPool(Sector, TreeFoliage,
            { TEXT("TreeCrowns"), TEXT("SovietPoplarCrowns"), TEXT("BirchCrowns"), TEXT("PineCrowns") },
            TEXT("Trees"), Loaded, Used);
        TotalLoadedAssets += Loaded; TotalUsedAssets += Used;

        TotalReplaced += BindMeshPool(Sector, GroundFoliage,
            { TEXT("GrassMown"), TEXT("GrassRough"), TEXT("GrassWetland") },
            TEXT("GroundFoliage"), Loaded, Used);
        TotalLoadedAssets += Loaded; TotalUsedAssets += Used;

        TotalReplaced += BindMeshPool(Sector, Fences,
            { TEXT("Fences"), TEXT("WoodFences"), TEXT("MetalFences"), TEXT("LightSheetFences") },
            TEXT("Fences"), Loaded, Used);
        TotalLoadedAssets += Loaded; TotalUsedAssets += Used;

        TotalReplaced += BindMeshPool(Sector, Bridges,
            { TEXT("Bridges") }, TEXT("Bridges"), Loaded, Used);
        TotalLoadedAssets += Loaded; TotalUsedAssets += Used;

        TotalReplaced += BindMeshPool(Sector, Roads,
            { TEXT("Roads"), TEXT("Sidewalks") }, TEXT("Roads"), Loaded, Used);
        TotalLoadedAssets += Loaded; TotalUsedAssets += Used;

        TotalReplaced += BindMeshPool(Sector, Water,
            { TEXT("Waterways") }, TEXT("Water"), Loaded, Used);
        TotalLoadedAssets += Loaded; TotalUsedAssets += Used;

        TotalReplaced += BindMeshPool(Sector, Props,
            { TEXT("ResidentialDetails"), TEXT("LandmarkDetails"), TEXT("StadiumDetails"), TEXT("ParkDetails") },
            TEXT("Props"), Loaded, Used);
        TotalLoadedAssets += Loaded; TotalUsedAssets += Used;
    }

    if (AllAssets.IsEmpty())
    {
        UE_LOG(LogTemp, Display, TEXT("PASS45_LOCAL_WORLD_ASSETS_READY supplied_world_assets=0"));
        WriteWorldReport(true, TEXT("supplied_world_assets=0"));
        return;
    }

    if (SectorCount > 0 && TotalLoadedAssets > 0 && TotalReplaced > 0)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_LOCAL_WORLD_ASSETS_READY supplied=%d loaded_pool_entries=%d visible_pool_entries=%d replaced_instances=%d sectors=%d"),
            AllAssets.Num(), TotalLoadedAssets, TotalUsedAssets, TotalReplaced, SectorCount);
        WriteWorldReport(true, FString::Printf(TEXT("supplied=%d loaded_pool_entries=%d visible_pool_entries=%d replaced_instances=%d sectors=%d"),
            AllAssets.Num(), TotalLoadedAssets, TotalUsedAssets, TotalReplaced, SectorCount));
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_LOCAL_WORLD_ASSETS_FAIL supplied=%d loaded_pool_entries=%d replaced_instances=%d sectors=%d"),
            AllAssets.Num(), TotalLoadedAssets, TotalReplaced, SectorCount);
        WriteWorldReport(false, FString::Printf(TEXT("supplied=%d loaded_pool_entries=%d replaced_instances=%d sectors=%d"),
            AllAssets.Num(), TotalLoadedAssets, TotalReplaced, SectorCount));
    }
}
