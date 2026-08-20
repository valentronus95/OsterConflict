#include "OCR13SilpoFacadeDetailSubsystem.h"

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
    // The base Silpo shell runs at 5.60 s. Keep this as a strictly later facade/detail pass.
    constexpr float SilpoFacadeDetailDelaySeconds = 5.85f;
    constexpr float FrontWallY = -914.0f;
    constexpr float TextPlaneY = -946.0f;

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
        if (Material)
        {
            const int32 SlotCount = FMath::Max(1, Mesh->GetStaticMaterials().Num());
            for (int32 Slot = 0; Slot < SlotCount; ++Slot) Component->SetMaterial(Slot, Material);
        }
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetCollisionProfileName(TEXT("NoCollision"));
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

    void AddPlaque(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const float WidthCm, const float HeightCm, const float DepthCm)
    {
        if (!Component) return;
        const FVector Scale(WidthCm / 100.0f, HeightCm / 100.0f, DepthCm / 100.0f);
        Component->AddInstance(FTransform(FRotator(90.0f, 0.0f, 0.0f), Center, Scale), false);
    }

    UTextRenderComponent* AddFacadeText(AActor* Owner, USceneComponent* Root, const FName Name,
        const FString& Text, const FVector& Location, const float WorldSize, const FColor Color)
    {
        if (!Owner || !Root) return nullptr;

        UTextRenderComponent* Label = NewObject<UTextRenderComponent>(Owner, Name);
        if (!Label) return nullptr;
        Label->SetupAttachment(Root);
        Label->SetMobility(EComponentMobility::Static);
        Label->SetText(FText::FromString(Text));
        Label->SetWorldSize(WorldSize);
        Label->SetTextRenderColor(Color);
        Label->SetRelativeLocation(Location);
        Label->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
        Label->SetCastShadow(false);
        Owner->AddInstanceComponent(Label);
        Label->RegisterComponent();
        return Label;
    }

    AActor* FindSilpoModel(UWorld& World)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (Actor && Actor->ActorHasTag(TEXT("R13_SilpoPhotoModel"))) return Actor;
        }
        return nullptr;
    }

    void HideTemporaryLogo(AActor* Model)
    {
        if (!Model) return;

        TInlineComponentArray<UInstancedStaticMeshComponent*> MeshComponents;
        Model->GetComponents(MeshComponents);
        for (UInstancedStaticMeshComponent* Component : MeshComponents)
        {
            if (!Component) continue;
            const FName Name = Component->GetFName();
            if (Name == TEXT("R13Silpo_LogoBlueBorder") || Name == TEXT("R13Silpo_LogoOrangeFace"))
            {
                Component->SetVisibility(false, true);
                Component->SetHiddenInGame(true, true);
            }
        }

        TInlineComponentArray<UTextRenderComponent*> TextComponents;
        Model->GetComponents(TextComponents);
        for (UTextRenderComponent* Component : TextComponents)
        {
            if (Component && Component->GetFName() == TEXT("R13Silpo_LogoText"))
            {
                Component->SetVisibility(false, true);
                Component->SetHiddenInGame(true, true);
            }
        }
    }
}

bool UOCR13SilpoFacadeDetailSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13SilpoFacadeDetailSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ApplyFacadeDetails(*World);
        }), SilpoFacadeDetailDelaySeconds, false);
}

void UOCR13SilpoFacadeDetailSubsystem::ApplyFacadeDetails(UWorld& World)
{
    AActor* Model = FindSilpoModel(World);
    if (!Model || Model->ActorHasTag(TEXT("R13_SilpoFacadeDetailApplied"))) return;

    USceneComponent* Root = Model->GetRootComponent();
    if (!Root) return;

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube || !Cylinder || !Basic) return;

    HideTemporaryLogo(Model);

    UMaterialInstanceDynamic* BlueMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoDetail_Blue"),
        FLinearColor(0.025f, 0.095f, 0.30f, 1.0f));
    UMaterialInstanceDynamic* OrangeMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoDetail_Orange"),
        FLinearColor(0.88f, 0.31f, 0.075f, 1.0f));
    UMaterialInstanceDynamic* YellowMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoDetail_Yellow"),
        FLinearColor(0.92f, 0.73f, 0.11f, 1.0f));
    UMaterialInstanceDynamic* PinkMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoDetail_Pink"),
        FLinearColor(0.82f, 0.32f, 0.48f, 1.0f));
    UMaterialInstanceDynamic* PosterOrangeMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoDetail_PosterOrange"),
        FLinearColor(0.55f, 0.18f, 0.075f, 1.0f));
    UMaterialInstanceDynamic* WhiteMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoDetail_White"),
        FLinearColor(0.90f, 0.90f, 0.86f, 1.0f));

    UInstancedStaticMeshComponent* LogoBlue = MakeISM(Model, Root, Cylinder, BlueMat,
        TEXT("R13SilpoDetail_LogoBlueCloud"), true);
    UInstancedStaticMeshComponent* LogoOrange = MakeISM(Model, Root, Cylinder, OrangeMat,
        TEXT("R13SilpoDetail_LogoOrangeCloud"), true);
    UInstancedStaticMeshComponent* YellowAccent = MakeISM(Model, Root, Cube, YellowMat,
        TEXT("R13SilpoDetail_PosterYellowAccents"), false);
    UInstancedStaticMeshComponent* PinkAccent = MakeISM(Model, Root, Cube, PinkMat,
        TEXT("R13SilpoDetail_PosterPinkAccents"), false);
    UInstancedStaticMeshComponent* PosterOrange = MakeISM(Model, Root, Cube, PosterOrangeMat,
        TEXT("R13SilpoDetail_PizzaPosterFace"), false);
    UInstancedStaticMeshComponent* WhitePlate = MakeISM(Model, Root, Cube, WhiteMat,
        TEXT("R13SilpoDetail_SignPlates"), false);

    // Multi-lobed raised facade plaque approximates the photographed orange Silpo cloud with dark-blue edge.
    AddPlaque(LogoBlue, FVector(150.0f, -928.0f, 602.0f), 900.0f, 272.0f, 26.0f);
    AddPlaque(LogoBlue, FVector(-250.0f, -928.0f, 600.0f), 300.0f, 226.0f, 26.0f);
    AddPlaque(LogoBlue, FVector(535.0f, -928.0f, 598.0f), 330.0f, 216.0f, 26.0f);
    AddPlaque(LogoOrange, FVector(150.0f, -946.0f, 602.0f), 850.0f, 238.0f, 18.0f);
    AddPlaque(LogoOrange, FVector(-245.0f, -946.0f, 600.0f), 265.0f, 192.0f, 18.0f);
    AddPlaque(LogoOrange, FVector(520.0f, -946.0f, 598.0f), 295.0f, 184.0f, 18.0f);

    // White underlay plus slightly forward blue title gives a visible white rim without importing a bitmap logo.
    AddFacadeText(Model, Root, TEXT("R13SilpoDetail_LogoWhiteOutline"), TEXT("Сільпо"),
        FVector(-190.0f, -968.0f, 557.0f), 137.0f, FColor(245, 245, 240));
    AddFacadeText(Model, Root, TEXT("R13SilpoDetail_LogoText"), TEXT("Сільпо"),
        FVector(-178.0f, -974.0f, 565.0f), 118.0f, FColor(25, 58, 118));

    // Poster 1: narrow purple panel seen beside the Silpo promotions.
    AddBox(YellowAccent, FVector(-820.0f, FrontWallY - 17.0f, 330.0f), FVector(260.0f, 7.0f, 54.0f));
    AddFacadeText(Model, Root, TEXT("R13SilpoDetail_PosterZhuk"), TEXT("ЖУК"),
        FVector(-905.0f, TextPlaneY, 310.0f), 47.0f, FColor(245, 245, 240));

    // Poster 2: blue fish promotion from the supplied facade views.
    AddBox(YellowAccent, FVector(-390.0f, FrontWallY - 17.0f, 190.0f), FVector(320.0f, 7.0f, 34.0f));
    AddFacadeText(Model, Root, TEXT("R13SilpoDetail_FishTitle"), TEXT("РИБНИЙ ЧЕТВЕР"),
        FVector(-565.0f, TextPlaneY, 338.0f), 35.0f, FColor(246, 246, 240));
    AddFacadeText(Model, Root, TEXT("R13SilpoDetail_FishDiscount"), TEXT("-20%"),
        FVector(-505.0f, TextPlaneY, 235.0f), 65.0f, FColor(248, 248, 243));
    AddFacadeText(Model, Root, TEXT("R13SilpoDetail_FishUrl"), TEXT("silpo.ua"),
        FVector(-525.0f, TextPlaneY, 164.0f), 21.0f, FColor(238, 238, 235));

    // Poster 3: green price-of-the-week panel with colored speech-card blocks.
    AddBox(PinkAccent, FVector(70.0f, FrontWallY - 17.0f, 324.0f), FVector(300.0f, 7.0f, 70.0f));
    AddBox(YellowAccent, FVector(70.0f, FrontWallY - 18.0f, 225.0f), FVector(230.0f, 7.0f, 58.0f));
    AddFacadeText(Model, Root, TEXT("R13SilpoDetail_PriceWeek"), TEXT("ЦІНА ТИЖНЯ"),
        FVector(-75.0f, TextPlaneY, 334.0f), 36.0f, FColor(250, 247, 238));
    AddFacadeText(Model, Root, TEXT("R13SilpoDetail_SuperPrice"), TEXT("СУПЕР ЦІНА"),
        FVector(-45.0f, TextPlaneY, 224.0f), 26.0f, FColor(35, 50, 80));

    // Poster 4: blue Wednesday promotion.
    AddBox(YellowAccent, FVector(530.0f, FrontWallY - 17.0f, 190.0f), FVector(315.0f, 7.0f, 34.0f));
    AddFacadeText(Model, Root, TEXT("R13SilpoDetail_WednesdayTitle"), TEXT("СМАЧНА СЕРЕДА"),
        FVector(355.0f, TextPlaneY, 338.0f), 34.0f, FColor(246, 246, 240));
    AddFacadeText(Model, Root, TEXT("R13SilpoDetail_WednesdayDiscount"), TEXT("-15%"),
        FVector(410.0f, TextPlaneY, 235.0f), 65.0f, FColor(248, 248, 243));
    AddFacadeText(Model, Root, TEXT("R13SilpoDetail_WednesdayUrl"), TEXT("silpo.ua"),
        FVector(395.0f, TextPlaneY, 164.0f), 21.0f, FColor(238, 238, 235));

    // Poster 5: later supplied frontal reference shows a dark warm pizza promotion in the right bay.
    AddBox(PosterOrange, FVector(990.0f, FrontWallY - 20.0f, 285.0f), FVector(390.0f, 8.0f, 250.0f));
    AddBox(YellowAccent, FVector(990.0f, FrontWallY - 28.0f, 205.0f), FVector(310.0f, 6.0f, 42.0f));
    AddFacadeText(Model, Root, TEXT("R13SilpoDetail_PizzaTitle"), TEXT("ПІЦА"),
        FVector(905.0f, TextPlaneY - 12.0f, 332.0f), 58.0f, FColor(248, 242, 225));
    AddFacadeText(Model, Root, TEXT("R13SilpoDetail_PizzaSub"), TEXT("СМАЧНО ЩОДНЯ"),
        FVector(865.0f, TextPlaneY - 12.0f, 224.0f), 26.0f, FColor(246, 240, 228));

    // Door-hours plate and parking distance plate are both legible in the close reference angles.
    AddBox(WhitePlate, FVector(-1350.0f, -1184.0f, 250.0f), FVector(118.0f, 6.0f, 60.0f));
    AddFacadeText(Model, Root, TEXT("R13SilpoDetail_OpeningHours"), TEXT("8:00-22:00"),
        FVector(-1400.0f, -1191.0f, 235.0f), 22.0f, FColor(30, 54, 105));

    AddBox(WhitePlate, FVector(1180.0f, -1271.0f, 168.0f), FVector(160.0f, 6.0f, 58.0f));
    AddFacadeText(Model, Root, TEXT("R13SilpoDetail_ParkingP"), TEXT("P"),
        FVector(1142.0f, -1278.0f, 230.0f), 68.0f, FColor(245, 245, 240));
    AddFacadeText(Model, Root, TEXT("R13SilpoDetail_ParkingDistance"), TEXT("100 м"),
        FVector(1120.0f, -1279.0f, 157.0f), 27.0f, FColor(30, 35, 42));

    Model->Tags.Add(TEXT("R13_SilpoFacadeDetailApplied"));
    UE_LOG(LogTemp, Display,
        TEXT("R13 Silpo facade detail: cloud logo + Сільпо title + five poster texts + door/parking signage applied."));
}
