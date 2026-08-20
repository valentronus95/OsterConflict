#include "OCR13SilpoSiteDetailSubsystem.h"

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
    // Base shell: 5.60 s. Facade/signage: 5.85 s. This pass owns only small site details.
    constexpr float SilpoSiteDetailDelaySeconds = 6.10f;
    constexpr float FrontWallY = -920.0f;
    constexpr float FrontTextY = -959.0f;

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
        Component->SetCollisionProfileName(TEXT("NoCollision"));
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(bCastShadow);
        Component->SetCullDistances(0, 85000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center, const FVector& SizeCm,
        const FRotator& Rotation = FRotator::ZeroRotator)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(Rotation, Center, SizeCm / 100.0f), false);
    }

    void AddCylinder(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const float DiameterCm, const float HeightCm)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(FRotator::ZeroRotator, Center,
            FVector(DiameterCm / 100.0f, DiameterCm / 100.0f, HeightCm / 100.0f)), false);
    }

    void AddSphere(UInstancedStaticMeshComponent* Component, const FVector& Center, const float DiameterCm)
    {
        if (!Component) return;
        const float Scale = DiameterCm / 100.0f;
        Component->AddInstance(FTransform(FRotator::ZeroRotator, Center, FVector(Scale)), false);
    }

    UTextRenderComponent* AddFrontText(AActor* Owner, USceneComponent* Root, const FName Name,
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

    void AddPosterRails(UInstancedStaticMeshComponent* Rails, const float X, const float WidthCm)
    {
        AddBox(Rails, FVector(X, FrontWallY - 28.0f, 414.0f), FVector(WidthCm + 28.0f, 10.0f, 12.0f));
        AddBox(Rails, FVector(X, FrontWallY - 28.0f, 156.0f), FVector(WidthCm + 28.0f, 10.0f, 12.0f));
        AddBox(Rails, FVector(X - WidthCm * 0.5f - 8.0f, FrontWallY - 28.0f, 285.0f), FVector(12.0f, 10.0f, 270.0f));
        AddBox(Rails, FVector(X + WidthCm * 0.5f + 8.0f, FrontWallY - 28.0f, 285.0f), FVector(12.0f, 10.0f, 270.0f));
    }
}

bool UOCR13SilpoSiteDetailSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13SilpoSiteDetailSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ApplySiteDetails(*World);
        }), SilpoSiteDetailDelaySeconds, false);
}

void UOCR13SilpoSiteDetailSubsystem::ApplySiteDetails(UWorld& World)
{
    AActor* Model = FindSilpoModel(World);
    if (!Model || Model->ActorHasTag(TEXT("R13_SilpoSiteDetailApplied"))) return;

    USceneComponent* Root = Model->GetRootComponent();
    if (!Root) return;

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube || !Cylinder || !Sphere || !Basic) return;

    UMaterialInstanceDynamic* DarkMetalMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoSite_DarkMetal"),
        FLinearColor(0.10f, 0.105f, 0.11f, 1.0f));
    UMaterialInstanceDynamic* WhiteMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoSite_White"),
        FLinearColor(0.88f, 0.88f, 0.84f, 1.0f));
    UMaterialInstanceDynamic* BlueMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoSite_Blue"),
        FLinearColor(0.025f, 0.10f, 0.30f, 1.0f));
    UMaterialInstanceDynamic* YellowMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoSite_Yellow"),
        FLinearColor(0.93f, 0.72f, 0.08f, 1.0f));
    UMaterialInstanceDynamic* SoilMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoSite_Soil"),
        FLinearColor(0.12f, 0.075f, 0.035f, 1.0f));
    UMaterialInstanceDynamic* GreenMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoSite_Green"),
        FLinearColor(0.08f, 0.26f, 0.07f, 1.0f));
    UMaterialInstanceDynamic* GreenLightMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoSite_GreenLight"),
        FLinearColor(0.16f, 0.38f, 0.10f, 1.0f));
    UMaterialInstanceDynamic* FlowerOrangeMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoSite_FlowerOrange"),
        FLinearColor(0.92f, 0.32f, 0.035f, 1.0f));
    UMaterialInstanceDynamic* FlowerYellowMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoSite_FlowerYellow"),
        FLinearColor(0.96f, 0.75f, 0.08f, 1.0f));

    UInstancedStaticMeshComponent* Rails = MakeISM(Model, Root, Cube, DarkMetalMat,
        TEXT("R13SilpoSite_PosterMountingRails"), true);
    UInstancedStaticMeshComponent* Seams = MakeISM(Model, Root, Cube, DarkMetalMat,
        TEXT("R13SilpoSite_FacadeSeams"), false);
    UInstancedStaticMeshComponent* EntranceMetal = MakeISM(Model, Root, Cube, DarkMetalMat,
        TEXT("R13SilpoSite_EntranceHardware"), true);
    UInstancedStaticMeshComponent* EntranceWhite = MakeISM(Model, Root, Cube, WhiteMat,
        TEXT("R13SilpoSite_EntranceTrim"), true);
    UInstancedStaticMeshComponent* BinBlue = MakeISM(Model, Root, Cube, BlueMat,
        TEXT("R13SilpoSite_BlueEntranceBin"), true);
    UInstancedStaticMeshComponent* YellowPlate = MakeISM(Model, Root, Cube, YellowMat,
        TEXT("R13SilpoSite_YellowBinPlate"), false);
    UInstancedStaticMeshComponent* Soil = MakeISM(Model, Root, Cube, SoilMat,
        TEXT("R13SilpoSite_FlowerBedSoil"), false);
    UInstancedStaticMeshComponent* Stems = MakeISM(Model, Root, Cylinder, GreenMat,
        TEXT("R13SilpoSite_FlowerStems"), false);
    UInstancedStaticMeshComponent* Shrubs = MakeISM(Model, Root, Sphere, GreenLightMat,
        TEXT("R13SilpoSite_Shrubs"), false);
    UInstancedStaticMeshComponent* FlowersOrange = MakeISM(Model, Root, Sphere, FlowerOrangeMat,
        TEXT("R13SilpoSite_OrangeFlowers"), false);
    UInstancedStaticMeshComponent* FlowersYellow = MakeISM(Model, Root, Sphere, FlowerYellowMat,
        TEXT("R13SilpoSite_YellowFlowers"), false);

    // Thin vertical facade joints/downpipe-like lines visible across the supplied long-wall angles.
    for (float X : { -1120.0f, -690.0f, -230.0f, 240.0f, 700.0f, 1160.0f })
    {
        AddBox(Seams, FVector(X, FrontWallY - 3.0f, 360.0f), FVector(9.0f, 8.0f, 430.0f));
    }

    // The five poster faces are visibly mounted in dark rectangular perimeter rails rather than painted on wall.
    AddPosterRails(Rails, -820.0f, 360.0f);
    AddPosterRails(Rails, -390.0f, 390.0f);
    AddPosterRails(Rails, 70.0f, 390.0f);
    AddPosterRails(Rails, 530.0f, 390.0f);
    AddPosterRails(Rails, 990.0f, 390.0f);

    // Entrance door hardware and the segmented white canopy lip in the close left-side photographs.
    AddBox(EntranceMetal, FVector(-1366.0f, -1193.0f, 190.0f), FVector(7.0f, 9.0f, 165.0f));
    AddBox(EntranceMetal, FVector(-1334.0f, -1193.0f, 190.0f), FVector(7.0f, 9.0f, 165.0f));
    AddBox(EntranceMetal, FVector(-1377.0f, -1196.0f, 188.0f), FVector(8.0f, 11.0f, 62.0f));
    AddBox(EntranceMetal, FVector(-1323.0f, -1196.0f, 188.0f), FVector(8.0f, 11.0f, 62.0f));
    for (float X = -1580.0f; X <= -1120.0f; X += 58.0f)
    {
        AddBox(EntranceWhite, FVector(X, -1302.0f, 421.0f), FVector(36.0f, 20.0f, 22.0f));
    }

    // Blue litter bin with yellow front label beside the entrance, present in the close reference photos.
    AddBox(BinBlue, FVector(-1090.0f, -1268.0f, 50.0f), FVector(58.0f, 58.0f, 100.0f));
    AddBox(DarkMetalMat ? BinBlue : nullptr, FVector(-1090.0f, -1268.0f, 105.0f), FVector(64.0f, 64.0f, 12.0f));
    AddBox(YellowPlate, FVector(-1090.0f, -1299.0f, 58.0f), FVector(24.0f, 5.0f, 28.0f));

    // Narrow, slightly untidy planted strip along the wall. Keep it clear of the photographed walking path.
    AddBox(Soil, FVector(220.0f, -1092.0f, 10.0f), FVector(2420.0f, 92.0f, 18.0f));

    const float StemXs[] = { -880.0f, -735.0f, -585.0f, -440.0f, -270.0f, -90.0f,
        85.0f, 260.0f, 440.0f, 610.0f, 790.0f, 955.0f, 1110.0f, 1260.0f };
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(StemXs); ++Index)
    {
        const float Height = 46.0f + static_cast<float>((Index * 17) % 52);
        const float Y = -1090.0f + static_cast<float>((Index % 3) - 1) * 18.0f;
        AddCylinder(Stems, FVector(StemXs[Index], Y, 18.0f + Height * 0.5f), 8.0f, Height);
        UInstancedStaticMeshComponent* Head = (Index % 2 == 0) ? FlowersOrange : FlowersYellow;
        AddSphere(Head, FVector(StemXs[Index], Y, 22.0f + Height), 18.0f + static_cast<float>(Index % 3) * 4.0f);
    }

    AddSphere(Shrubs, FVector(650.0f, -1065.0f, 53.0f), 92.0f);
    AddSphere(Shrubs, FVector(720.0f, -1082.0f, 41.0f), 70.0f);
    AddSphere(Shrubs, FVector(-610.0f, -1075.0f, 36.0f), 58.0f);

    // Smaller copy visible on the supplied promotional boards. This is authored text, not copied image art.
    AddFrontText(Model, Root, TEXT("R13SilpoSite_ZhukPhones"), TEXT("ТЕЛЕФОНИ"),
        FVector(-908.0f, FrontTextY, 266.0f), 15.0f, FColor(245, 245, 240));
    AddFrontText(Model, Root, TEXT("R13SilpoSite_ZhukSmart"), TEXT("СМАРТ-ПРИСТРОЇ"),
        FVector(-932.0f, FrontTextY, 237.0f), 13.0f, FColor(245, 245, 240));
    AddFrontText(Model, Root, TEXT("R13SilpoSite_ZhukAccessories"), TEXT("АКСЕСУАРИ"),
        FVector(-913.0f, FrontTextY, 210.0f), 13.0f, FColor(245, 245, 240));
    AddFrontText(Model, Root, TEXT("R13SilpoSite_ZhukService"), TEXT("СЕРВІС"),
        FVector(-887.0f, FrontTextY, 183.0f), 13.0f, FColor(245, 245, 240));

    AddFrontText(Model, Root, TEXT("R13SilpoSite_FishCopy1"), TEXT("ЩОЧЕТВЕРГА ДІЮТЬ ЗНИЖКИ"),
        FVector(-558.0f, FrontTextY, 302.0f), 13.0f, FColor(244, 244, 240));
    AddFrontText(Model, Root, TEXT("R13SilpoSite_FishCopy2"), TEXT("НА ПРОДУКЦІЮ РИБНОГО ВІДДІЛУ"),
        FVector(-570.0f, FrontTextY, 278.0f), 11.0f, FColor(244, 244, 240));

    AddFrontText(Model, Root, TEXT("R13SilpoSite_WeekCopy"), TEXT("У СУПЕРМАРКЕТАХ СІЛЬПО"),
        FVector(-35.0f, FrontTextY, 177.0f), 12.0f, FColor(245, 245, 238));
    AddFrontText(Model, Root, TEXT("R13SilpoSite_WednesdayCopy"), TEXT("ЩОСЕРЕДИ ДІЮТЬ ЗНИЖКИ"),
        FVector(360.0f, FrontTextY, 302.0f), 13.0f, FColor(244, 244, 240));

    // Parking supplementary plate: simple dark car silhouette and double-sided 100 m cue from the photos.
    AddBox(EntranceMetal, FVector(1180.0f, -1288.0f, 184.0f), FVector(76.0f, 7.0f, 22.0f));
    AddBox(EntranceMetal, FVector(1153.0f, -1291.0f, 169.0f), FVector(18.0f, 7.0f, 18.0f));
    AddBox(EntranceMetal, FVector(1207.0f, -1291.0f, 169.0f), FVector(18.0f, 7.0f, 18.0f));
    AddFrontText(Model, Root, TEXT("R13SilpoSite_ParkingArrows"), TEXT("< 100 м >"),
        FVector(1126.0f, -1297.0f, 140.0f), 18.0f, FColor(30, 35, 42));

    Model->Tags.Add(TEXT("R13_SilpoSiteDetailApplied"));
    UE_LOG(LogTemp, Display,
        TEXT("R13 Silpo site detail: poster rails/copy, facade seams, entrance hardware/bin, planted strip and parking pictogram applied."));
}
