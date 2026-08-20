#include "OCR13SilpoEnvelopeDetailSubsystem.h"

#include "OCGameMode.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

namespace
{
    // Keep this after cart/glyph/site passes so it only refines the already-established Silpo envelope.
    constexpr float SilpoEnvelopeDetailDelaySeconds = 7.20f;

    AActor* FindSilpoModel(UWorld& World)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (Actor && Actor->ActorHasTag(TEXT("R13_SilpoPhotoModel"))) return Actor;
        }
        return nullptr;
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
        UMaterialInterface* Material, const FName Name, const bool bCastShadow)
    {
        if (!Owner || !Root || !Mesh) return nullptr;

        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material) Component->SetMaterial(0, Material);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(TEXT("NoCollision"));
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(bCastShadow);
        Component->SetCullDistances(0, 90000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center, const FVector& SizeCm)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(FRotator::ZeroRotator, Center, SizeCm / 100.0f), false);
    }
}

bool UOCR13SilpoEnvelopeDetailSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13SilpoEnvelopeDetailSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ApplyEnvelopeDetails(*World);
        }), SilpoEnvelopeDetailDelaySeconds, false);
}

void UOCR13SilpoEnvelopeDetailSubsystem::ApplyEnvelopeDetails(UWorld& World)
{
    AActor* Model = FindSilpoModel(World);
    if (!Model || Model->ActorHasTag(TEXT("R13_SilpoEnvelopeDetailApplied"))) return;

    USceneComponent* Root = Model->GetRootComponent();
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Root || !Cube || !Basic) return;

    UMaterialInstanceDynamic* CopingMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoEnvelope_CopingMat"),
        FLinearColor(0.29f, 0.30f, 0.28f, 1.0f));
    UMaterialInstanceDynamic* FasciaMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoEnvelope_FasciaMat"),
        FLinearColor(0.73f, 0.70f, 0.64f, 1.0f));
    UMaterialInstanceDynamic* BaseRevealMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoEnvelope_BaseRevealMat"),
        FLinearColor(0.18f, 0.19f, 0.18f, 1.0f));

    UInstancedStaticMeshComponent* FrontCoping = MakeISM(Model, Root, Cube, CopingMat,
        TEXT("R13SilpoEnvelope_FrontParapetCoping"), true);
    UInstancedStaticMeshComponent* SideCoping = MakeISM(Model, Root, Cube, CopingMat,
        TEXT("R13SilpoEnvelope_SidePierCoping"), true);
    UInstancedStaticMeshComponent* RoofEdge = MakeISM(Model, Root, Cube, CopingMat,
        TEXT("R13SilpoEnvelope_RoofEdgeFlashing"), true);
    UInstancedStaticMeshComponent* EntranceFascia = MakeISM(Model, Root, Cube, FasciaMat,
        TEXT("R13SilpoEnvelope_EntranceCanopyFascia"), true);
    UInstancedStaticMeshComponent* BaseReveal = MakeISM(Model, Root, Cube, BaseRevealMat,
        TEXT("R13SilpoEnvelope_PlinthReveal"), false);

    // Thin coping follows the exact stepped silhouette already authored from the exterior references.
    AddBox(FrontCoping, FVector(-1420.0f, -870.0f, 609.0f), FVector(374.0f, 88.0f, 10.0f));
    AddBox(FrontCoping, FVector(-1080.0f, -870.0f, 661.0f), FVector(354.0f, 88.0f, 10.0f));
    AddBox(FrontCoping, FVector(-680.0f, -870.0f, 721.0f), FVector(474.0f, 88.0f, 10.0f));
    AddBox(FrontCoping, FVector(0.0f, -870.0f, 781.0f), FVector(914.0f, 88.0f, 10.0f));
    AddBox(FrontCoping, FVector(680.0f, -870.0f, 721.0f), FVector(474.0f, 88.0f, 10.0f));
    AddBox(FrontCoping, FVector(1080.0f, -870.0f, 661.0f), FVector(354.0f, 88.0f, 10.0f));
    AddBox(FrontCoping, FVector(1420.0f, -870.0f, 609.0f), FVector(374.0f, 88.0f, 10.0f));

    // Cap the repeated side parapet piers already present in the base reconstruction.
    for (float Y = -520.0f; Y <= 640.0f; Y += 290.0f)
    {
        AddBox(SideCoping, FVector(-1565.0f, Y, 671.0f), FVector(92.0f, 108.0f, 10.0f));
        AddBox(SideCoping, FVector(1565.0f, Y, 671.0f), FVector(92.0f, 108.0f, 10.0f));
    }

    // Give oblique views a readable roof edge without inventing rooftop equipment absent from the reference set.
    AddBox(RoofEdge, FVector(0.0f, 886.0f, 550.0f), FVector(3200.0f, 18.0f, 18.0f));
    AddBox(RoofEdge, FVector(-1591.0f, 10.0f, 550.0f), FVector(18.0f, 1770.0f, 18.0f));
    AddBox(RoofEdge, FVector(1591.0f, 10.0f, 550.0f), FVector(18.0f, 1770.0f, 18.0f));

    // Entrance canopy fascia/side returns add thickness to the vestibule silhouette seen in oblique views.
    AddBox(EntranceFascia, FVector(-1350.0f, -1302.0f, 425.0f), FVector(620.0f, 20.0f, 54.0f));
    AddBox(EntranceFascia, FVector(-1662.0f, -1120.0f, 425.0f), FVector(20.0f, 380.0f, 54.0f));
    AddBox(EntranceFascia, FVector(-1038.0f, -1120.0f, 425.0f), FVector(20.0f, 380.0f, 54.0f));

    // Continuous plinth reveal keeps the shell from visually melting into the pavement from side angles.
    AddBox(BaseReveal, FVector(0.0f, -908.0f, 79.0f), FVector(3200.0f, 16.0f, 22.0f));
    AddBox(BaseReveal, FVector(-1608.0f, 0.0f, 79.0f), FVector(16.0f, 1800.0f, 22.0f));
    AddBox(BaseReveal, FVector(1608.0f, 0.0f, 79.0f), FVector(16.0f, 1800.0f, 22.0f));

    Model->Tags.Add(TEXT("R13_SilpoEnvelopeDetailApplied"));
    UE_LOG(LogTemp, Display,
        TEXT("R13 Silpo envelope detail: parapet coping, roof edge, entrance fascia and plinth reveal applied; visual-only and reference-constrained."));
}
