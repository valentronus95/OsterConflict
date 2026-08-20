#include "OCR143SilpoFacadeIdentitySubsystem.h"

#include "OCGameMode.h"
#include "OCGeoReference.h"

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
    constexpr float DetailBuildDelaySeconds = 6.75f;
    constexpr float BuildingDepthCm = 1750.0f;
    constexpr float FrontY = -BuildingDepthCm * 0.5f;

    struct FTopRail
    {
        float X;
        float Z;
        float Width;
    };

    FVector SilpoAnchor()
    {
        const FOCGeoReferencePoint Ref = FOCGeoReference::Silpo();
        return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, 0.0);
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
        if (Material)
        {
            const int32 SlotCount = FMath::Max(1, Mesh->GetStaticMaterials().Num());
            for (int32 Slot = 0; Slot < SlotCount; ++Slot) Component->SetMaterial(Slot, Material);
        }
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(TEXT("NoCollision")));
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(bCastShadow);
        Component->SetCullDistances(0, 70000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddLocalBox(UInstancedStaticMeshComponent* Component, const FVector& Center, const FVector& SizeCm)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(FRotator::ZeroRotator, Center, SizeCm / 100.0f), false);
    }

    void AddFacadeOval(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const float WidthCm, const float HeightCm, const float DepthCm)
    {
        if (!Component) return;
        // Engine cylinder is Z-axis aligned. Roll 90 degrees turns the cylinder depth toward facade Y,
        // while X/Y local scale becomes the horizontal/vertical ellipse on the facade plane.
        const FVector Scale(WidthCm / 100.0f, HeightCm / 100.0f, DepthCm / 100.0f);
        Component->AddInstance(FTransform(FRotator(0.0f, 0.0f, 90.0f), Center, Scale), false);
    }

    bool HasActorTag(UWorld& World, const FName Tag)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            const AActor* Actor = *It;
            if (Actor && Actor->ActorHasTag(Tag)) return true;
        }
        return false;
    }
}

bool UOCR143SilpoFacadeIdentitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR143SilpoFacadeIdentitySubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) BuildFacadeIdentity(*World);
        }), DetailBuildDelaySeconds, false);
}

void UOCR143SilpoFacadeIdentitySubsystem::BuildFacadeIdentity(UWorld& World)
{
    if (HasActorTag(World, TEXT("R143_SilpoFacadeIdentity"))) return;

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube || !Cylinder || !Basic) return;

    const FVector Site = SilpoAnchor();
    AActor* Identity = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform(FRotator::ZeroRotator, Site));
    if (!Identity) return;
    Identity->SetReplicates(false);
    Identity->Tags.Add(TEXT("R143_SilpoFacadeIdentity"));
    Identity->Tags.Add(TEXT("SilpoOster_BohdanaKhmelnytskoho54"));

    USceneComponent* Root = NewObject<USceneComponent>(Identity, TEXT("R143Silpo_FacadeIdentityRoot"));
    if (!Root)
    {
        Identity->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    Identity->SetRootComponent(Root);
    Identity->AddInstanceComponent(Root);
    Root->RegisterComponent();
    Identity->SetActorLocation(Site);

    UMaterialInstanceDynamic* Stucco = MakeColorMaterial(Identity, Basic, TEXT("R143Silpo_StuccoPatchMat"),
        FLinearColor(0.72f, 0.57f, 0.42f, 1.0f));
    UMaterialInstanceDynamic* Blue = MakeColorMaterial(Identity, Basic, TEXT("R143Silpo_LogoBlueMat"),
        FLinearColor(0.05f, 0.22f, 0.42f, 1.0f));
    UMaterialInstanceDynamic* Orange = MakeColorMaterial(Identity, Basic, TEXT("R143Silpo_LogoOrangeMat"),
        FLinearColor(0.93f, 0.40f, 0.12f, 1.0f));
    UMaterialInstanceDynamic* Dark = MakeColorMaterial(Identity, Basic, TEXT("R143Silpo_ParapetRailMat"),
        FLinearColor(0.12f, 0.13f, 0.13f, 1.0f));
    UMaterialInstanceDynamic* ParkingBlue = MakeColorMaterial(Identity, Basic, TEXT("R143Silpo_ParkingBlueMat"),
        FLinearColor(0.04f, 0.18f, 0.38f, 1.0f));
    UMaterialInstanceDynamic* White = MakeColorMaterial(Identity, Basic, TEXT("R143Silpo_SignWhiteMat"),
        FLinearColor(0.93f, 0.93f, 0.90f, 1.0f));

    UInstancedStaticMeshComponent* Patch = MakeISM(Identity, Root, Cube, Stucco,
        TEXT("R143Silpo_OldSignPatch"), false);
    UInstancedStaticMeshComponent* LogoBlue = MakeISM(Identity, Root, Cylinder, Blue,
        TEXT("R143Silpo_LogoBlueOutline"), true);
    UInstancedStaticMeshComponent* LogoOrange = MakeISM(Identity, Root, Cylinder, Orange,
        TEXT("R143Silpo_LogoOrangeFace"), true);
    UInstancedStaticMeshComponent* Rails = MakeISM(Identity, Root, Cube, Dark,
        TEXT("R143Silpo_ParapetDarkRails"), true);
    UInstancedStaticMeshComponent* Parking = MakeISM(Identity, Root, Cube, ParkingBlue,
        TEXT("R143Silpo_ParkingSign"), true);
    UInstancedStaticMeshComponent* ParkingWhite = MakeISM(Identity, Root, Cube, White,
        TEXT("R143Silpo_ParkingSupplement"), true);

    // Cover the R14.0 rectangular logo placeholder without editing the gameplay shell.
    AddLocalBox(Patch, FVector(50.0f, FrontY - 56.0f, 466.0f), FVector(850.0f, 10.0f, 166.0f));

    // Photo-driven layered oval approximation of the prominent orange/blue facade logo.
    AddFacadeOval(LogoBlue, FVector(80.0f, FrontY - 68.0f, 470.0f), 790.0f, 255.0f, 12.0f);
    AddFacadeOval(LogoOrange, FVector(80.0f, FrontY - 76.0f, 470.0f), 755.0f, 225.0f, 10.0f);

    // Blue shadow and white face approximate the strongly outlined script from the reference sign.
    UTextRenderComponent* LogoShadow = NewObject<UTextRenderComponent>(Identity, TEXT("R143Silpo_LogoTextShadow"));
    if (LogoShadow)
    {
        LogoShadow->SetupAttachment(Root);
        LogoShadow->SetText(FText::FromString(TEXT("Сільпо")));
        LogoShadow->SetTextRenderColor(FColor(13, 48, 93));
        LogoShadow->SetHorizontalAlignment(EHTA_Center);
        LogoShadow->SetWorldSize(92.0f);
        LogoShadow->SetRelativeLocation(FVector(95.0f, FrontY - 88.0f, 456.0f));
        LogoShadow->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
        LogoShadow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Identity->AddInstanceComponent(LogoShadow);
        LogoShadow->RegisterComponent();
    }

    UTextRenderComponent* LogoText = NewObject<UTextRenderComponent>(Identity, TEXT("R143Silpo_LogoText"));
    if (LogoText)
    {
        LogoText->SetupAttachment(Root);
        LogoText->SetText(FText::FromString(TEXT("Сільпо")));
        LogoText->SetTextRenderColor(FColor(246, 242, 226));
        LogoText->SetHorizontalAlignment(EHTA_Center);
        LogoText->SetWorldSize(86.0f);
        LogoText->SetRelativeLocation(FVector(80.0f, FrontY - 92.0f, 464.0f));
        LogoText->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
        LogoText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Identity->AddInstanceComponent(LogoText);
        LogoText->RegisterComponent();
    }

    // Dark metal cap lines visible along each stepped parapet tier.
    const FTopRail TopRails[] =
    {
        { -1170.0f, 456.0f, 660.0f },
        { -620.0f, 507.0f, 440.0f },
        { 0.0f, 567.0f, 820.0f },
        { 620.0f, 507.0f, 440.0f },
        { 1170.0f, 456.0f, 660.0f },
    };
    for (const FTopRail& Rail : TopRails)
    {
        AddLocalBox(Rails, FVector(Rail.X, FrontY - 17.0f, Rail.Z), FVector(Rail.Width, 34.0f, 5.0f));
    }

    // Freestanding P sign visible in the facade reference. The lower plate stays deliberately generic.
    constexpr float ParkingX = 1030.0f;
    constexpr float ParkingY = FrontY - 245.0f;
    AddLocalBox(Rails, FVector(ParkingX, ParkingY, 92.0f), FVector(6.0f, 6.0f, 184.0f));
    AddLocalBox(Parking, FVector(ParkingX, ParkingY, 215.0f), FVector(72.0f, 8.0f, 92.0f));
    AddLocalBox(ParkingWhite, FVector(ParkingX, ParkingY, 148.0f), FVector(72.0f, 8.0f, 38.0f));

    UTextRenderComponent* ParkingText = NewObject<UTextRenderComponent>(Identity, TEXT("R143Silpo_ParkingText"));
    if (ParkingText)
    {
        ParkingText->SetupAttachment(Root);
        ParkingText->SetText(FText::FromString(TEXT("P")));
        ParkingText->SetTextRenderColor(FColor::White);
        ParkingText->SetHorizontalAlignment(EHTA_Center);
        ParkingText->SetWorldSize(58.0f);
        ParkingText->SetRelativeLocation(FVector(ParkingX, ParkingY - 7.0f, 195.0f));
        ParkingText->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
        ParkingText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Identity->AddInstanceComponent(ParkingText);
        ParkingText->RegisterComponent();
    }

    UE_LOG(LogTemp, Display,
        TEXT("R14.3 Silpo facade identity pass built at [%.0f %.0f]: layered logo, parapet rails and parking sign."),
        Site.X, Site.Y);
}
