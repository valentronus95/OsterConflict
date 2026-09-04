#include "OCLocalInboxRuntimeSubsystem.h"

#include "OCGameMode.h"
#include "OCLocalInboxHUDOverlayWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
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
    FString BindingManifestPath()
    {
        return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("LocalModelInbox"), TEXT("runtime_bindings.json"));
    }

    FString RuntimeReportPath()
    {
        return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("AutomationReports"), TEXT("ProductionModels"),
            TEXT("local_inbox_runtime_validation.txt"));
    }

    FString PackageToObjectPath(const FString& PackagePath)
    {
        if (PackagePath.IsEmpty()) return FString();
        if (PackagePath.Contains(TEXT("."))) return PackagePath;
        FString AssetName;
        int32 Slash = INDEX_NONE;
        if (PackagePath.FindLastChar(TEXT('/'), Slash)) AssetName = PackagePath.Mid(Slash + 1);
        else AssetName = PackagePath;
        return PackagePath + TEXT(".") + AssetName;
    }

    bool LoadBindingRoot(TSharedPtr<FJsonObject>& OutRoot)
    {
        FString JsonText;
        if (!FFileHelper::LoadFileToString(JsonText, *BindingManifestPath())) return false;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
        return FJsonSerializer::Deserialize(Reader, OutRoot) && OutRoot.IsValid();
    }

    const TArray<TSharedPtr<FJsonValue>>* GetArray(const TSharedPtr<FJsonObject>& Root, const TCHAR* Field)
    {
        if (!Root.IsValid()) return nullptr;
        const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
        return Root->TryGetArrayField(Field, Values) ? Values : nullptr;
    }

    bool ReadEntry(const TSharedPtr<FJsonValue>& Value, FString& OutPath, FString& OutCategory,
        bool& OutCompatible)
    {
        const TSharedPtr<FJsonObject>* Obj = nullptr;
        if (!Value.IsValid() || !Value->TryGetObject(Obj) || !Obj || !Obj->IsValid()) return false;
        if (!(*Obj)->TryGetStringField(TEXT("path"), OutPath) || OutPath.IsEmpty()) return false;
        (*Obj)->TryGetStringField(TEXT("category"), OutCategory);
        OutCompatible = false;
        (*Obj)->TryGetBoolField(TEXT("character_compatible"), OutCompatible);
        OutPath = PackageToObjectPath(OutPath);
        return true;
    }

    bool ManifestSaysAllModelsBound(const TSharedPtr<FJsonObject>& Root)
    {
        bool bBound = false;
        return Root.IsValid() && Root->TryGetBoolField(TEXT("all_models_bound"), bBound) && bBound;
    }
}

bool UOCLocalInboxRuntimeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCLocalInboxRuntimeSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    bValidateLocalInbox = FParse::Param(FCommandLine::Get(), TEXT("ValidateLocalInbox"));
    InWorld.GetTimerManager().SetTimer(ApplyTimer, this,
        &UOCLocalInboxRuntimeSubsystem::ApplyRuntimeBindings, 0.45f, false);
}

void UOCLocalInboxRuntimeSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ApplyTimer);
        World->GetTimerManager().ClearTimer(ShowcaseTimer);
    }
    if (LocalHUDWidget.IsValid()) LocalHUDWidget->RemoveFromParent();
    ShowcaseActors.Reset();
    BoundStaticObjectPaths.Reset();
    Super::Deinitialize();
}

bool UOCLocalInboxRuntimeSubsystem::ResolveFirstAssetObjectPathForCategory(const FString& Category,
    FString& OutObjectPath, bool bRequireSkeletal, bool bRequireCharacterCompatible)
{
    TSharedPtr<FJsonObject> Root;
    if (!LoadBindingRoot(Root)) return false;

    const TCHAR* Fields[] = { TEXT("skeletal_assets"), TEXT("static_assets") };
    for (const TCHAR* Field : Fields)
    {
        if (bRequireSkeletal && FCString::Stricmp(Field, TEXT("skeletal_assets")) != 0) continue;
        const TArray<TSharedPtr<FJsonValue>>* Values = GetArray(Root, Field);
        if (!Values) continue;
        for (const TSharedPtr<FJsonValue>& Value : *Values)
        {
            FString Path;
            FString EntryCategory;
            bool bCompatible = false;
            if (!ReadEntry(Value, Path, EntryCategory, bCompatible)) continue;
            if (!EntryCategory.Equals(Category, ESearchCase::IgnoreCase)) continue;
            if (bRequireCharacterCompatible && !bCompatible) continue;
            OutObjectPath = Path;
            return true;
        }
    }
    return false;
}

UStaticMesh* UOCLocalInboxRuntimeSubsystem::LoadFirstStaticMeshForCategory(const FString& Category)
{
    TSharedPtr<FJsonObject> Root;
    if (!LoadBindingRoot(Root)) return nullptr;
    const TArray<TSharedPtr<FJsonValue>>* Values = GetArray(Root, TEXT("static_assets"));
    if (!Values) return nullptr;
    for (const TSharedPtr<FJsonValue>& Value : *Values)
    {
        FString Candidate;
        FString EntryCategory;
        bool bCompatible = false;
        if (ReadEntry(Value, Candidate, EntryCategory, bCompatible) &&
            EntryCategory.Equals(Category, ESearchCase::IgnoreCase))
        {
            if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *Candidate)) return Mesh;
        }
    }
    return nullptr;
}

int32 UOCLocalInboxRuntimeSubsystem::GetCompatibleCharacterSkinCount()
{
    TSharedPtr<FJsonObject> Root;
    if (!LoadBindingRoot(Root)) return 0;
    const TArray<TSharedPtr<FJsonValue>>* Values = GetArray(Root, TEXT("skeletal_assets"));
    if (!Values) return 0;
    int32 Count = 0;
    for (const TSharedPtr<FJsonValue>& Value : *Values)
    {
        FString Path;
        FString Category;
        bool bCompatible = false;
        if (ReadEntry(Value, Path, Category, bCompatible) && bCompatible &&
            Category.Equals(TEXT("CHARACTER_SKIN"), ESearchCase::IgnoreCase))
        {
            ++Count;
        }
    }
    return Count;
}

USkeletalMesh* UOCLocalInboxRuntimeSubsystem::LoadCompatibleCharacterSkin(const int32 Index)
{
    TSharedPtr<FJsonObject> Root;
    if (!LoadBindingRoot(Root)) return nullptr;
    const TArray<TSharedPtr<FJsonValue>>* Values = GetArray(Root, TEXT("skeletal_assets"));
    if (!Values) return nullptr;
    int32 CompatibleIndex = 0;
    for (const TSharedPtr<FJsonValue>& Value : *Values)
    {
        FString Path;
        FString Category;
        bool bCompatible = false;
        if (!ReadEntry(Value, Path, Category, bCompatible) || !bCompatible ||
            !Category.Equals(TEXT("CHARACTER_SKIN"), ESearchCase::IgnoreCase)) continue;
        if (CompatibleIndex++ == Index) return LoadObject<USkeletalMesh>(nullptr, *Path);
    }
    return nullptr;
}

UTexture2D* UOCLocalInboxRuntimeSubsystem::LoadHUDTexture()
{
    TSharedPtr<FJsonObject> Root;
    if (!LoadBindingRoot(Root)) return nullptr;
    const TArray<TSharedPtr<FJsonValue>>* Values = GetArray(Root, TEXT("hud_textures"));
    if (!Values || Values->IsEmpty()) return nullptr;
    const TSharedPtr<FJsonObject>* Obj = nullptr;
    if (!(*Values)[0]->TryGetObject(Obj) || !Obj || !Obj->IsValid()) return nullptr;
    FString Path;
    if (!(*Obj)->TryGetStringField(TEXT("path"), Path)) return nullptr;
    Path = PackageToObjectPath(Path);
    return LoadObject<UTexture2D>(nullptr, *Path);
}

UClass* UOCLocalInboxRuntimeSubsystem::LoadHUDWidgetClass()
{
    TSharedPtr<FJsonObject> Root;
    if (!LoadBindingRoot(Root)) return nullptr;
    const TArray<TSharedPtr<FJsonValue>>* Values = GetArray(Root, TEXT("hud_widget_classes"));
    if (!Values || Values->IsEmpty()) return nullptr;
    const TSharedPtr<FJsonObject>* Obj = nullptr;
    if (!(*Values)[0]->TryGetObject(Obj) || !Obj || !Obj->IsValid()) return nullptr;
    FString Path;
    if (!(*Obj)->TryGetStringField(TEXT("path"), Path) || Path.IsEmpty()) return nullptr;
    return LoadClass<UUserWidget>(nullptr, *Path);
}

void UOCLocalInboxRuntimeSubsystem::ApplyRuntimeBindings()
{
    if (bBindingsApplied) return;
    bBindingsApplied = true;

    TSharedPtr<FJsonObject> Root;
    if (!LoadBindingRoot(Root))
    {
        UE_LOG(LogTemp, Warning, TEXT("PASS45_LOCAL_INBOX_RUNTIME_GAP manifest_missing=%s"), *BindingManifestPath());
        if (bValidateLocalInbox) WriteRuntimeReport(false, TEXT("runtime_bindings.json missing"));
        return;
    }

    const bool bManifestBound = ManifestSaysAllModelsBound(Root);
    int32 StaticLoaded = 0;
    int32 SkeletalLoaded = 0;
    int32 LoadFailures = 0;

    if (const TArray<TSharedPtr<FJsonValue>>* Values = GetArray(Root, TEXT("static_assets")))
    {
        for (const TSharedPtr<FJsonValue>& Value : *Values)
        {
            FString Path;
            FString Category;
            bool bCompatible = false;
            if (!ReadEntry(Value, Path, Category, bCompatible)) continue;
            if (LoadObject<UStaticMesh>(nullptr, *Path))
            {
                ++StaticLoaded;
                BoundStaticObjectPaths.AddUnique(Path);
            }
            else ++LoadFailures;
        }
    }

    if (const TArray<TSharedPtr<FJsonValue>>* Values = GetArray(Root, TEXT("skeletal_assets")))
    {
        for (const TSharedPtr<FJsonValue>& Value : *Values)
        {
            FString Path;
            FString Category;
            bool bCompatible = false;
            if (!ReadEntry(Value, Path, Category, bCompatible)) continue;
            if (LoadObject<USkeletalMesh>(nullptr, *Path)) ++SkeletalLoaded;
            else ++LoadFailures;
        }
    }

    bool bHUDBound = false;
    if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
    {
        if (UClass* HUDClass = LoadHUDWidgetClass())
        {
            if (UUserWidget* Widget = CreateWidget<UUserWidget>(PC, TSubclassOf<UUserWidget>(HUDClass)))
            {
                Widget->AddToViewport(5);
                LocalHUDWidget = Widget;
                bHUDBound = true;
                UE_LOG(LogTemp, Display, TEXT("PASS45_LOCAL_HUD_WIDGET_BOUND class=%s"), *HUDClass->GetPathName());
            }
        }
        else if (UTexture2D* HUDTexture = LoadHUDTexture())
        {
            if (UOCLocalInboxHUDOverlayWidget* Overlay = CreateWidget<UOCLocalInboxHUDOverlayWidget>(
                PC, UOCLocalInboxHUDOverlayWidget::StaticClass()))
            {
                Overlay->SetHUDTexture(HUDTexture);
                Overlay->AddToViewport(1);
                LocalHUDWidget = Overlay;
                bHUDBound = true;
                UE_LOG(LogTemp, Display, TEXT("PASS45_LOCAL_HUD_TEXTURE_BOUND texture=%s"), *HUDTexture->GetPathName());
            }
        }
    }

    const bool bPass = bManifestBound && LoadFailures == 0;
    if (bPass)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_LOCAL_INBOX_RUNTIME_READY static=%d skeletal=%d hud_bound=%d all_models_bound=1"),
            StaticLoaded, SkeletalLoaded, bHUDBound ? 1 : 0);
        if (bValidateLocalInbox)
        {
            WriteRuntimeReport(true, FString::Printf(TEXT("static=%d skeletal=%d hud_bound=%d load_failures=0"),
                StaticLoaded, SkeletalLoaded, bHUDBound ? 1 : 0));
            SpawnValidationShowcase();
        }
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_LOCAL_INBOX_RUNTIME_FAIL manifest_bound=%d load_failures=%d static_loaded=%d skeletal_loaded=%d"),
            bManifestBound ? 1 : 0, LoadFailures, StaticLoaded, SkeletalLoaded);
        if (bValidateLocalInbox)
        {
            WriteRuntimeReport(false, FString::Printf(TEXT("manifest_bound=%d load_failures=%d"),
                bManifestBound ? 1 : 0, LoadFailures));
        }
    }
}

void UOCLocalInboxRuntimeSubsystem::WriteRuntimeReport(const bool bPass, const FString& Detail) const
{
    const FString ReportPath = RuntimeReportPath();
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(ReportPath), true);
    const FString Text = FString::Printf(TEXT("PASS45_LOCAL_INBOX_RUNTIME=%s\n%s\nmanifest=%s\n"),
        bPass ? TEXT("PASS") : TEXT("FAIL"), *Detail, *BindingManifestPath());
    FFileHelper::SaveStringToFile(Text, *ReportPath);
}

void UOCLocalInboxRuntimeSubsystem::SpawnValidationShowcase()
{
    UWorld* World = GetWorld();
    APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
    APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    if (!World || !Pawn || BoundStaticObjectPaths.IsEmpty()) return;

    constexpr int32 ShowcaseSlots = 12;
    const FVector Forward = Pawn->GetActorForwardVector().GetSafeNormal2D();
    const FVector Right = Pawn->GetActorRightVector().GetSafeNormal2D();
    const FVector Origin = Pawn->GetActorLocation() + Forward * 900.0f;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    for (int32 Slot = 0; Slot < ShowcaseSlots && Slot < BoundStaticObjectPaths.Num(); ++Slot)
    {
        const int32 Row = Slot / 4;
        const int32 Col = Slot % 4;
        const FVector Location = Origin + Forward * (Row * 360.0f) + Right * ((Col - 1.5f) * 330.0f) + FVector(0,0,120);
        AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(Location, Pawn->GetActorRotation(), Params);
        if (!Actor) continue;
        if (UStaticMeshComponent* Component = Actor->GetStaticMeshComponent())
        {
            Component->SetMobility(EComponentMobility::Movable);
            Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Component->SetGenerateOverlapEvents(false);
            Component->SetCanEverAffectNavigation(false);
        }
        ShowcaseActors.Add(Actor);
    }

    ShowcaseCursor = 0;
    CycleValidationShowcase();
    World->GetTimerManager().SetTimer(ShowcaseTimer, this,
        &UOCLocalInboxRuntimeSubsystem::CycleValidationShowcase, 1.25f, true);
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_LOCAL_INBOX_SHOWCASE_READY slots=%d asset_pool=%d cycle_seconds=1.25"),
        ShowcaseActors.Num(), BoundStaticObjectPaths.Num());
}

void UOCLocalInboxRuntimeSubsystem::CycleValidationShowcase()
{
    if (BoundStaticObjectPaths.IsEmpty() || ShowcaseActors.IsEmpty()) return;
    for (TWeakObjectPtr<AStaticMeshActor>& WeakActor : ShowcaseActors)
    {
        AStaticMeshActor* Actor = WeakActor.Get();
        if (!Actor) continue;
        const FString& Path = BoundStaticObjectPaths[ShowcaseCursor % BoundStaticObjectPaths.Num()];
        ++ShowcaseCursor;
        UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *Path);
        UStaticMeshComponent* Component = Actor->GetStaticMeshComponent();
        if (!Mesh || !Component) continue;
        Component->SetStaticMesh(Mesh);
        Component->EmptyOverrideMaterials();
        const FVector Size = Mesh->GetBounds().BoxExtent * 2.0f;
        const float Longest = FMath::Max3(Size.X, Size.Y, Size.Z);
        const float UniformScale = Longest > 1.0f ? FMath::Clamp(260.0f / Longest, 0.02f, 8.0f) : 1.0f;
        Component->SetRelativeScale3D(FVector(UniformScale));
    }
}
