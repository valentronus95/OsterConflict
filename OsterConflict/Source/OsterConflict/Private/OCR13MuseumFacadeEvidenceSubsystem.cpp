#include "OCR13MuseumFacadeEvidenceSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

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
    constexpr float FacadeEvidenceDelaySeconds = 5.34f;
    const FName FacadeEvidenceTag(TEXT("R13_MuseumFacadeEvidence"));

    UMaterialInstanceDynamic* MakeColor(AActor* Owner, UMaterialInterface* Base,
        const FName Name, const FLinearColor& Color)
    {
        if (!Owner || !Base) return nullptr;
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Owner, Name);
        if (Material) Material->SetVectorParameterValue(TEXT("Color"), Color);
        return Material;
    }

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        UMaterialInterface* Material, const FName Name, const int32 CullEndCm = 50000)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material)
        {
            const int32 Slots = FMath::Max(1, Mesh->GetStaticMaterials().Num());
            for (int32 Slot = 0; Slot < Slots; ++Slot) Component->SetMaterial(Slot, Material);
        }
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(TEXT("NoCollision"));
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(true);
        Component->SetCullDistances(0, CullEndCm);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const FVector& SizeCm, const FRotator& Rotation = FRotator::ZeroRotator)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(Rotation, Center, SizeCm / 100.0f), true);
    }

    void HideCoarseWindowBars(UWorld& World)
    {
        int32 Hidden = 0;
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (!Actor) continue;
            TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
            Actor->GetComponents(Components);
            for (UInstancedStaticMeshComponent* Component : Components)
            {
                if (!Component || Component->GetFName() != TEXT("R137Museum_WindowGrilles")) continue;
                Component->SetVisibility(false, true);
                Component->SetHiddenInGame(true, true);
                Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                ++Hidden;
            }
        }
        UE_LOG(LogTemp, Display, TEXT("R13 museum facade evidence: coarse grille components hidden=%d."), Hidden);
    }

    void AddFrontMullions(UInstancedStaticMeshComponent* Mullions, const FVector& Origin,
        const float X, const float Y, const float Z, const float Width, const float Height)
    {
        AddBox(Mullions, Origin + FVector(X, Y, Z), FVector(5.0f, 7.0f, Height - 18.0f));
        AddBox(Mullions, Origin + FVector(X, Y, Z + 14.0f), FVector(Width - 18.0f, 7.0f, 5.0f));
    }

    void AddSideMullions(UInstancedStaticMeshComponent* Mullions, const FVector& Origin,
        const float X, const float Y, const float Z, const float Width, const float Height)
    {
        AddBox(Mullions, Origin + FVector(X, Y, Z), FVector(7.0f, 5.0f, Height - 18.0f));
        AddBox(Mullions, Origin + FVector(X, Y, Z + 14.0f), FVector(7.0f, Width - 18.0f, 5.0f));
    }

    void AddFrontDiamondGrille(UInstancedStaticMeshComponent* Grille, const FVector& Origin,
        const float X, const float Y, const float Z, const float Width, const float Height)
    {
        const float BarLength = FMath::Min(Height * 0.82f, Width * 1.28f);
        const float Offset = Width * 0.23f;
        for (const float XOffset : { -Offset, 0.0f, Offset })
        {
            AddBox(Grille, Origin + FVector(X + XOffset, Y, Z),
                FVector(BarLength, 5.0f, 4.0f), FRotator(52.0f, 0.0f, 0.0f));
            AddBox(Grille, Origin + FVector(X + XOffset, Y, Z),
                FVector(BarLength, 5.0f, 4.0f), FRotator(-52.0f, 0.0f, 0.0f));
        }
    }

    void AddSideDiamondGrille(UInstancedStaticMeshComponent* Grille, const FVector& Origin,
        const float X, const float Y, const float Z, const float Width, const float Height)
    {
        const float BarLength = FMath::Min(Height * 0.82f, Width * 1.28f);
        const float Offset = Width * 0.23f;
        for (const float YOffset : { -Offset, 0.0f, Offset })
        {
            AddBox(Grille, Origin + FVector(X, Y + YOffset, Z),
                FVector(5.0f, BarLength, 4.0f), FRotator(0.0f, 0.0f, 52.0f));
            AddBox(Grille, Origin + FVector(X, Y + YOffset, Z),
                FVector(5.0f, BarLength, 4.0f), FRotator(0.0f, 0.0f, -52.0f));
        }
    }

    void AddFrontReliefCross(UInstancedStaticMeshComponent* Trim, const FVector& Origin,
        const float X, const float Y, const float Z)
    {
        AddBox(Trim, Origin + FVector(X, Y, Z), FVector(28.0f, 5.0f, 5.0f), FRotator(45.0f, 0.0f, 0.0f));
        AddBox(Trim, Origin + FVector(X, Y, Z), FVector(28.0f, 5.0f, 5.0f), FRotator(-45.0f, 0.0f, 0.0f));
    }
}

bool UOCR13MuseumFacadeEvidenceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13MuseumFacadeEvidenceSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) BuildFacadeEvidence(*World);
        }), FacadeEvidenceDelaySeconds, false);
}

void UOCR13MuseumFacadeEvidenceSubsystem::BuildFacadeEvidence(UWorld& World)
{
    HideCoarseWindowBars(World);

    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Existing = *It;
        if (Existing && Existing->ActorHasTag(FacadeEvidenceTag)) return;
    }

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube || !Basic) return;

    FActorSpawnParameters Params;
    Params.Name = TEXT("R13_OsterMuseumFacadeEvidence");
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Detail = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Params);
    if (!Detail) return;
    Detail->SetReplicates(false);
    Detail->SetActorEnableCollision(false);
    Detail->Tags.Add(FacadeEvidenceTag);

    USceneComponent* Root = NewObject<USceneComponent>(Detail, TEXT("R13_MuseumFacadeEvidenceRoot"));
    if (!Root)
    {
        Detail->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    Detail->SetRootComponent(Root);
    Detail->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UMaterialInstanceDynamic* MullionMat = MakeColor(Detail, Basic, TEXT("R13_MuseumMullionPale"),
        FLinearColor(0.62f, 0.64f, 0.59f, 1.0f));
    UMaterialInstanceDynamic* GrilleMat = MakeColor(Detail, Basic, TEXT("R13_MuseumDiamondGrille"),
        FLinearColor(0.24f, 0.27f, 0.26f, 1.0f));
    UMaterialInstanceDynamic* ReliefMat = MakeColor(Detail, Basic, TEXT("R13_MuseumReliefCross"),
        FLinearColor(0.68f, 0.68f, 0.62f, 1.0f));

    UInstancedStaticMeshComponent* Mullions = MakeISM(Detail, Root, Cube, MullionMat,
        TEXT("R13Museum_PhotoWindowMullions"));
    UInstancedStaticMeshComponent* Grilles = MakeISM(Detail, Root, Cube, GrilleMat,
        TEXT("R13Museum_PhotoDiamondGrilles"));
    UInstancedStaticMeshComponent* Relief = MakeISM(Detail, Root, Cube, ReliefMat,
        TEXT("R13Museum_PhotoCorniceRelief"));

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();

    // Ground-floor front windows: public frontal and 3/4 photography consistently shows pale frames behind a
    // diagonal diamond security lattice. Keep the lattice shallow and external to the wall plane.
    for (const float X : { -650.0f, -355.0f, 355.0f, 650.0f })
    {
        AddFrontMullions(Mullions, Museum, X, -446.0f, 235.0f, 140.0f, 205.0f);
        AddFrontDiamondGrille(Grilles, Museum, X, -452.0f, 235.0f, 140.0f, 205.0f);
    }

    // Glazed entrance wings use the same photographed diagonal grille language, but with narrower bays.
    for (const float X : { -190.0f, 190.0f })
    {
        AddFrontMullions(Mullions, Museum, X, -681.0f, 225.0f, 125.0f, 205.0f);
        AddFrontDiamondGrille(Grilles, Museum, X, -687.0f, 225.0f, 125.0f, 205.0f);
    }

    // Raised timber center reads as ordinary glazed joinery in the available facade photographs, not the heavy
    // security lattice used below. Recreate only its internal pale mullions after suppressing the coarse old bars.
    AddFrontMullions(Mullions, Museum, -190.0f, -286.0f, 520.0f, 115.0f, 165.0f);
    AddFrontMullions(Mullions, Museum, 0.0f, -286.0f, 520.0f, 130.0f, 178.0f);
    AddFrontMullions(Mullions, Museum, 190.0f, -286.0f, 520.0f, 115.0f, 165.0f);

    // Side-ground-floor windows visible in oblique reference photography retain diamond guards.
    for (const float Y : { -270.0f, 20.0f, 300.0f })
    {
        AddSideMullions(Mullions, Museum, 868.0f, Y, 235.0f, 135.0f, 205.0f);
        AddSideDiamondGrille(Grilles, Museum, 874.0f, Y, 235.0f, 135.0f, 205.0f);
    }

    // Rear and left-side openings are not currently supported by a close enough exterior photo for security-lattice
    // reconstruction. Preserve only neutral pale mullions instead of inventing a grille pattern there.
    for (const float X : { -650.0f, -330.0f, 0.0f, 330.0f, 650.0f })
        AddFrontMullions(Mullions, Museum, X, 446.0f, 235.0f, 140.0f, 205.0f);
    for (const float Y : { -55.0f, 85.0f, 225.0f, 365.0f })
        AddSideMullions(Mullions, Museum, -1117.0f, Y, 220.0f, 115.0f, 195.0f);

    // The red-brick facade has a repeated pale decorative band directly below the eaves. The existing block rhythm is
    // retained; shallow diagonal crosses add the visible relief without inventing lettering or ornamental plaques.
    for (int32 Index = -7; Index <= 7; ++Index)
    {
        AddFrontReliefCross(Relief, Museum, static_cast<float>(Index) * 105.0f, -450.0f, 370.0f);
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13 museum facade evidence pass: coarse vertical grille layer replaced with pale window mullions and photo-matched diamond security lattice on confirmed front/oblique openings; upper/rear unsupported lattice omitted; eave relief crosses added."));
}
