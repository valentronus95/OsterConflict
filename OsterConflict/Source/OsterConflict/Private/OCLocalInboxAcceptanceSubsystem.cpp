#include "OCLocalInboxAcceptanceSubsystem.h"

#include "OCGameMode.h"

#include "Components/StaticMeshComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    FString ManifestPath()
    {
        return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("LocalModelInbox"), TEXT("runtime_bindings.json"));
    }

    FString ReportPath()
    {
        return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("AutomationReports"), TEXT("ProductionModels"),
            TEXT("local_inbox_runtime_validation.txt"));
    }

    FString ToObjectPath(const FString& PackagePath)
    {
        if (PackagePath.IsEmpty() || PackagePath.Contains(TEXT("."))) return PackagePath;
        int32 Slash = INDEX_NONE;
        const FString Name = PackagePath.FindLastChar(TEXT('/'), Slash) ? PackagePath.Mid(Slash + 1) : PackagePath;
        return PackagePath + TEXT(".") + Name;
    }

    bool ReadRoot(TSharedPtr<FJsonObject>& OutRoot)
    {
        FString Text;
        if (!FFileHelper::LoadFileToString(Text, *ManifestPath())) return false;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
        return FJsonSerializer::Deserialize(Reader, OutRoot) && OutRoot.IsValid();
    }

    bool ReadPath(const TSharedPtr<FJsonValue>& Value, FString& OutPath)
    {
        const TSharedPtr<FJsonObject>* Obj = nullptr;
        if (!Value.IsValid() || !Value->TryGetObject(Obj) || !Obj || !Obj->IsValid()) return false;
        if (!(*Obj)->TryGetStringField(TEXT("path"), OutPath) || OutPath.IsEmpty()) return false;
        OutPath = ToObjectPath(OutPath);
        return true;
    }
}

bool UOCLocalInboxAcceptanceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCLocalInboxAcceptanceSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (!FPlatformMisc::GetEnvironmentVariable(TEXT("OC_FORCE_ACCEPTANCE")).Equals(TEXT("1"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    InWorld.GetTimerManager().SetTimer(ValidationTimer, this,
        &UOCLocalInboxAcceptanceSubsystem::ValidateAndExposeAssets, 0.75f, false);
}

void UOCLocalInboxAcceptanceSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimer);
        World->GetTimerManager().ClearTimer(ShowcaseTimer);
    }
    ShowcaseActors.Reset();
    StaticPaths.Reset();
    Super::Deinitialize();
}

void UOCLocalInboxAcceptanceSubsystem::ValidateAndExposeAssets()
{
    TSharedPtr<FJsonObject> Root;
    if (!ReadRoot(Root))
    {
        WriteReport(false, TEXT("runtime_bindings.json missing/unreadable"));
        UE_LOG(LogTemp, Error, TEXT("PASS45_LOCAL_INBOX_RUNTIME_FAIL manifest_missing=1"));
        return;
    }

    bool bAllBound = false;
    Root->TryGetBoolField(TEXT("all_models_bound"), bAllBound);
    int32 StaticLoaded = 0;
    int32 SkeletalLoaded = 0;
    int32 Failures = 0;

    const TArray<TSharedPtr<FJsonValue>>* StaticValues = nullptr;
    if (Root->TryGetArrayField(TEXT("static_assets"), StaticValues) && StaticValues)
    {
        for (const TSharedPtr<FJsonValue>& Value : *StaticValues)
        {
            FString Path;
            if (!ReadPath(Value, Path)) continue;
            if (LoadObject<UStaticMesh>(nullptr, *Path))
            {
                StaticPaths.AddUnique(Path);
                ++StaticLoaded;
            }
            else ++Failures;
        }
    }

    const TArray<TSharedPtr<FJsonValue>>* SkeletalValues = nullptr;
    if (Root->TryGetArrayField(TEXT("skeletal_assets"), SkeletalValues) && SkeletalValues)
    {
        for (const TSharedPtr<FJsonValue>& Value : *SkeletalValues)
        {
            FString Path;
            if (!ReadPath(Value, Path)) continue;
            if (LoadObject<USkeletalMesh>(nullptr, *Path)) ++SkeletalLoaded;
            else ++Failures;
        }
    }

    if (!bAllBound || Failures > 0)
    {
        const FString Detail = FString::Printf(TEXT("all_models_bound=%d load_failures=%d static=%d skeletal=%d"),
            bAllBound ? 1 : 0, Failures, StaticLoaded, SkeletalLoaded);
        WriteReport(false, Detail);
        UE_LOG(LogTemp, Error, TEXT("PASS45_LOCAL_INBOX_RUNTIME_FAIL %s"), *Detail);
        return;
    }

    WriteReport(true, FString::Printf(TEXT("all_models_bound=1 load_failures=0 static=%d skeletal=%d"),
        StaticLoaded, SkeletalLoaded));
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_LOCAL_INBOX_RUNTIME_READY all_models_bound=1 static=%d skeletal=%d load_failures=0"),
        StaticLoaded, SkeletalLoaded);

    UWorld* World = GetWorld();
    APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
    APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    if (!World || !Pawn || StaticPaths.IsEmpty()) return;

    constexpr int32 SlotCount = 12;
    const FVector Forward = Pawn->GetActorForwardVector().GetSafeNormal2D();
    const FVector Right = Pawn->GetActorRightVector().GetSafeNormal2D();
    const FVector Origin = Pawn->GetActorLocation() + Forward * 950.0f;
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    for (int32 Slot = 0; Slot < FMath::Min(SlotCount, StaticPaths.Num()); ++Slot)
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
    CycleShowcase();
    World->GetTimerManager().SetTimer(ShowcaseTimer, this,
        &UOCLocalInboxAcceptanceSubsystem::CycleShowcase, 1.25f, true);
    UE_LOG(LogTemp, Display, TEXT("PASS45_LOCAL_INBOX_SHOWCASE_READY slots=%d pool=%d"),
        ShowcaseActors.Num(), StaticPaths.Num());
}

void UOCLocalInboxAcceptanceSubsystem::CycleShowcase()
{
    if (StaticPaths.IsEmpty()) return;
    for (TWeakObjectPtr<AStaticMeshActor>& WeakActor : ShowcaseActors)
    {
        AStaticMeshActor* Actor = WeakActor.Get();
        if (!Actor) continue;
        UStaticMeshComponent* Component = Actor->GetStaticMeshComponent();
        if (!Component) continue;
        const FString& Path = StaticPaths[ShowcaseCursor++ % StaticPaths.Num()];
        UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *Path);
        if (!Mesh) continue;
        Component->SetStaticMesh(Mesh);
        Component->EmptyOverrideMaterials();
        const FVector Size = Mesh->GetBounds().BoxExtent * 2.0f;
        const float Longest = FMath::Max3(Size.X, Size.Y, Size.Z);
        const float Scale = Longest > 1.0f ? FMath::Clamp(260.0f / Longest, 0.02f, 8.0f) : 1.0f;
        Component->SetRelativeScale3D(FVector(Scale));
    }
}

void UOCLocalInboxAcceptanceSubsystem::WriteReport(const bool bPass, const FString& Detail) const
{
    const FString Path = ReportPath();
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
    const FString Text = FString::Printf(TEXT("PASS45_LOCAL_INBOX_RUNTIME=%s\n%s\nmanifest=%s\n"),
        bPass ? TEXT("PASS") : TEXT("FAIL"), *Detail, *ManifestPath());
    FFileHelper::SaveStringToFile(Text, *Path);
}
