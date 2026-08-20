#include "OCR13SilpoPlaytestCorrectionSubsystem.h"

#include "OCGameMode.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

namespace
{
    // Silpo photo model starts at 5.60 s. Run the cleanup immediately after it instead of leaving overlapping
    // logo/text layers shimmering for another 1.5 seconds in front of the player.
    constexpr float CorrectionDelaySeconds = 5.85f;

    AActor* FindSilpoModel(UWorld& World)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (Actor && Actor->ActorHasTag(TEXT("R13_SilpoPhotoModel"))) return Actor;
        }
        return nullptr;
    }

    bool IsVegetationComponent(const UInstancedStaticMeshComponent* Component)
    {
        if (!Component) return false;
        const FString Name = Component->GetName();
        return Name.Contains(TEXT("Tree"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("Pine"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("Foliage"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("Vegetation"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("Bush"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("Shrub"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("Plant"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("Grass"), ESearchCase::IgnoreCase) ||
            Name.Contains(TEXT("Flower"), ESearchCase::IgnoreCase);
    }

    bool IsInsideSilpoHardscape(const AActor* Model, const FVector& WorldLocation)
    {
        if (!Model) return false;
        const FVector Local = Model->GetActorTransform().InverseTransformPosition(WorldLocation);

        const bool bBuilding = FMath::Abs(Local.X) <= 1900.0f && Local.Y >= -1250.0f && Local.Y <= 1150.0f;
        const bool bParking = Local.X >= -1950.0f && Local.X <= 2200.0f &&
            Local.Y >= -3100.0f && Local.Y <= -1250.0f;
        return bBuilding || bParking;
    }

    UMaterialInstanceDynamic* MakeColorMaterial(AActor* Owner, UMaterialInterface* Base,
        const FName Name, const FLinearColor& Color)
    {
        if (!Owner || !Base) return nullptr;
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Owner, Name);
        if (Material) Material->SetVectorParameterValue(TEXT("Color"), Color);
        return Material;
    }

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        UMaterialInterface* Material, const FName Name)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material) Component->SetMaterial(0, Material);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetCollisionProfileName(TEXT("NoCollision"));
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(true);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center, const FVector& Size)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(FRotator::ZeroRotator, Center, Size / 100.0f), false);
    }

    void RemoveLastInstanceByName(AActor* Model, const FName ComponentName)
    {
        if (!Model) return;
        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Model->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!Component || Component->GetFName() != ComponentName || Component->GetInstanceCount() <= 0) continue;
            Component->RemoveInstance(Component->GetInstanceCount() - 1);
            Component->MarkRenderStateDirty();
            return;
        }
    }
}

bool UOCR13SilpoPlaytestCorrectionSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13SilpoPlaytestCorrectionSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ApplyCorrection(*World);
        }), CorrectionDelaySeconds, false);
}

void UOCR13SilpoPlaytestCorrectionSubsystem::ApplyCorrection(UWorld& World)
{
    AActor* Model = FindSilpoModel(World);
    if (!Model || Model->ActorHasTag(TEXT("R13_SilpoPlaytestCorrectionApplied"))) return;

    // Remove imported/generic trees and plants from the actual building footprint and parking apron. Keep the
    // Silpo-owned flower strip itself: it lives on the store actor and is intentionally outside the parking surface.
    int32 RemovedVegetation = 0;
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor == Model) continue;

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!IsVegetationComponent(Component)) continue;
            for (int32 Index = Component->GetInstanceCount() - 1; Index >= 0; --Index)
            {
                FTransform Transform;
                if (!Component->GetInstanceTransform(Index, Transform, true)) continue;
                if (!IsInsideSilpoHardscape(Model, Transform.GetLocation())) continue;
                if (Component->RemoveInstance(Index)) ++RemovedVegetation;
            }
            Component->MarkRenderStateDirty();
        }
    }

    // The old cylinder-based logo produced the tall blue/orange arcs visible in the playtest. Hide every obsolete
    // plaque layer and keep only the geometric Cyrillic fallback face, which does not depend on the default font.
    TInlineComponentArray<UInstancedStaticMeshComponent*> SilpoMeshes;
    Model->GetComponents(SilpoMeshes);
    for (UInstancedStaticMeshComponent* Component : SilpoMeshes)
    {
        if (!Component) continue;
        const FName Name = Component->GetFName();
        if (Name == TEXT("R13Silpo_LogoBlueBorder") ||
            Name == TEXT("R13Silpo_LogoOrangeFace") ||
            Name == TEXT("R13SilpoDetail_LogoBlueCloud") ||
            Name == TEXT("R13SilpoDetail_LogoOrangeCloud") ||
            Name == TEXT("R13SilpoGlyph_WhiteOutline"))
        {
            Component->SetVisibility(false, true);
            Component->SetHiddenInGame(true, true);
        }
    }

    // Default TextRender does not reliably cover the Ukrainian promo glyph set here and the overlapping layers were
    // visibly shimmering. Hide the temporary text pass; the store name remains as geometric СІЛЬПО lettering.
    TInlineComponentArray<UTextRenderComponent*> TextComponents;
    Model->GetComponents(TextComponents);
    for (UTextRenderComponent* Component : TextComponents)
    {
        if (!Component) continue;
        const FString Name = Component->GetName();
        if (Name.StartsWith(TEXT("R13SilpoDetail_")) || Name == TEXT("R13Silpo_LogoText"))
        {
            Component->SetVisibility(false, true);
            Component->SetHiddenInGame(true, true);
        }
    }

    // Remove the primitive parking sign instances that were appended to shared facade components.
    RemoveLastInstanceByName(Model, TEXT("R13Silpo_FacadeMetal"));
    RemoveLastInstanceByName(Model, TEXT("R13Silpo_AdvertisingBlue"));
    RemoveLastInstanceByName(Model, TEXT("R13Silpo_ParkingLines"));

    USceneComponent* Root = Model->GetRootComponent();
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (Root && Cube && Basic)
    {
        UMaterialInstanceDynamic* Blue = MakeColorMaterial(Model, Basic, TEXT("R13SilpoFix_ParkingBlue"),
            FLinearColor(0.025f, 0.11f, 0.34f, 1.0f));
        UMaterialInstanceDynamic* White = MakeColorMaterial(Model, Basic, TEXT("R13SilpoFix_ParkingWhite"),
            FLinearColor(0.94f, 0.94f, 0.91f, 1.0f));
        UMaterialInstanceDynamic* Metal = MakeColorMaterial(Model, Basic, TEXT("R13SilpoFix_ParkingMetal"),
            FLinearColor(0.16f, 0.17f, 0.18f, 1.0f));

        UInstancedStaticMeshComponent* BlueParts = MakeISM(Model, Root, Cube, Blue, TEXT("R13SilpoFix_ParkingPanel"));
        UInstancedStaticMeshComponent* WhiteParts = MakeISM(Model, Root, Cube, White, TEXT("R13SilpoFix_ParkingGlyph"));
        UInstancedStaticMeshComponent* MetalParts = MakeISM(Model, Root, Cube, Metal, TEXT("R13SilpoFix_ParkingPost"));

        constexpr float SignX = 1720.0f;
        constexpr float SignY = -2720.0f;
        AddBox(MetalParts, FVector(SignX, SignY, 92.0f), FVector(10.0f, 10.0f, 184.0f));
        AddBox(BlueParts, FVector(SignX, SignY, 205.0f), FVector(92.0f, 10.0f, 92.0f));

        // Simple geometric P, avoiding another font-dependent TextRender component.
        AddBox(WhiteParts, FVector(SignX - 19.0f, SignY - 8.0f, 205.0f), FVector(10.0f, 5.0f, 58.0f));
        AddBox(WhiteParts, FVector(SignX + 2.0f, SignY - 8.0f, 229.0f), FVector(42.0f, 5.0f, 10.0f));
        AddBox(WhiteParts, FVector(SignX + 2.0f, SignY - 8.0f, 205.0f), FVector(42.0f, 5.0f, 10.0f));
        AddBox(WhiteParts, FVector(SignX + 19.0f, SignY - 8.0f, 217.0f), FVector(8.0f, 5.0f, 24.0f));
    }

    Model->Tags.Add(TEXT("R13_SilpoPlaytestCorrectionApplied"));
    UE_LOG(LogTemp, Display,
        TEXT("R13 Silpo playtest correction applied: vegetation removed from hardscape=%d; obsolete logo/text and parking sign cleaned."),
        RemovedVegetation);
}
