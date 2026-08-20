#include "OCR141SilpoDetailSubsystem.h"

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
    constexpr float DetailBuildDelaySeconds = 6.15f;
    constexpr float SilpoYawDegrees = 0.0f;

    constexpr float BuildingLengthCm = 3000.0f;
    constexpr float BuildingDepthCm = 1750.0f;
    constexpr float HalfLength = BuildingLengthCm * 0.5f;
    constexpr float HalfDepth = BuildingDepthCm * 0.5f;
    constexpr float FrontY = -HalfDepth;

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
        UMaterialInterface* Material, const FName Name, const bool bCollision, const bool bCastShadow,
        const int32 CullEndCm = 70000)
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
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(bCollision);
        Component->SetCastShadow(bCastShadow);
        if (CullEndCm > 0) Component->SetCullDistances(0, CullEndCm);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddLocalBox(UInstancedStaticMeshComponent* Component, const FVector& LocalCenter,
        const FVector& SizeCm, const FRotator& LocalRotation = FRotator::ZeroRotator)
    {
        if (!Component) return;
        const FRotator Rotation(LocalRotation.Pitch,
            LocalRotation.Yaw + SilpoYawDegrees, LocalRotation.Roll);
        Component->AddInstance(FTransform(Rotation, LocalCenter, SizeCm / 100.0f), false);
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

bool UOCR141SilpoDetailSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR141SilpoDetailSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) BuildDetails(*World);
        }), DetailBuildDelaySeconds, false);
}

void UOCR141SilpoDetailSubsystem::BuildDetails(UWorld& World)
{
    if (HasActorTag(World, TEXT("R141_SilpoPhotoDetails"))) return;

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube || !Basic) return;

    const FVector Site = SilpoAnchor();
    AActor* Details = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform(FRotator::ZeroRotator, Site));
    if (!Details) return;
    Details->SetReplicates(false);
    Details->Tags.Add(TEXT("R141_SilpoPhotoDetails"));
    Details->Tags.Add(TEXT("SilpoOster_BohdanaKhmelnytskoho54"));

    USceneComponent* Root = NewObject<USceneComponent>(Details, TEXT("R141Silpo_DetailRoot"));
    if (!Root)
    {
        Details->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    Details->SetRootComponent(Root);
    Details->AddInstanceComponent(Root);
    Root->RegisterComponent();
    Details->SetActorLocation(Site);

    UMaterialInstanceDynamic* CeilingMat = MakeColorMaterial(Details, Basic, TEXT("R141Silpo_CeilingMat"),
        FLinearColor(0.86f, 0.85f, 0.80f, 1.0f));
    UMaterialInstanceDynamic* GridMat = MakeColorMaterial(Details, Basic, TEXT("R141Silpo_GridMat"),
        FLinearColor(0.55f, 0.55f, 0.52f, 1.0f));
    UMaterialInstanceDynamic* MetalMat = MakeColorMaterial(Details, Basic, TEXT("R141Silpo_MetalMat"),
        FLinearColor(0.36f, 0.38f, 0.38f, 1.0f));
    UMaterialInstanceDynamic* FacadeTrimMat = MakeColorMaterial(Details, Basic, TEXT("R141Silpo_FacadeTrimMat"),
        FLinearColor(0.88f, 0.72f, 0.58f, 1.0f));
    UMaterialInstanceDynamic* DarkMat = MakeColorMaterial(Details, Basic, TEXT("R141Silpo_DarkMat"),
        FLinearColor(0.13f, 0.14f, 0.14f, 1.0f));
    UMaterialInstanceDynamic* AsphaltMat = MakeColorMaterial(Details, Basic, TEXT("R141Silpo_AsphaltCorrectionMat"),
        FLinearColor(0.17f, 0.17f, 0.17f, 1.0f));
    UMaterialInstanceDynamic* AwningMat = MakeColorMaterial(Details, Basic, TEXT("R141Silpo_AwningMat"),
        FLinearColor(0.58f, 0.50f, 0.39f, 1.0f));
    UMaterialInstanceDynamic* SignMat = MakeColorMaterial(Details, Basic, TEXT("R141Silpo_LaneSignMat"),
        FLinearColor(0.92f, 0.48f, 0.10f, 1.0f));

    UInstancedStaticMeshComponent* Ceiling = MakeISM(Details, Root, Cube, CeilingMat,
        TEXT("R141Silpo_SuspendedCeiling"), false, false);
    UInstancedStaticMeshComponent* CeilingGrid = MakeISM(Details, Root, Cube, GridMat,
        TEXT("R141Silpo_CeilingGrid"), false, false);
    UInstancedStaticMeshComponent* FacadeTrim = MakeISM(Details, Root, Cube, FacadeTrimMat,
        TEXT("R141Silpo_FacadeTrim"), false, true);
    UInstancedStaticMeshComponent* Metal = MakeISM(Details, Root, Cube, MetalMat,
        TEXT("R141Silpo_MetalDetails"), true, true);
    UInstancedStaticMeshComponent* DarkDetails = MakeISM(Details, Root, Cube, DarkMat,
        TEXT("R141Silpo_DarkDetails"), false, true);
    UInstancedStaticMeshComponent* AsphaltCorrection = MakeISM(Details, Root, Cube, AsphaltMat,
        TEXT("R141Silpo_AsphaltCorrection"), true, false);
    UInstancedStaticMeshComponent* MarketEdge = MakeISM(Details, Root, Cube, AwningMat,
        TEXT("R141Silpo_SideMarketEdge"), false, true);
    UInstancedStaticMeshComponent* LaneSigns = MakeISM(Details, Root, Cube, SignMat,
        TEXT("R141Silpo_CheckoutLaneSigns"), false, false);

    // The interior photos show a low suspended tile ceiling rather than an exposed roof volume.
    AddLocalBox(Ceiling, FVector(0.0f, 0.0f, 362.0f), FVector(2860.0f, 1620.0f, 6.0f));
    for (float X = -1320.0f; X <= 1320.0f; X += 220.0f)
    {
        AddLocalBox(CeilingGrid, FVector(X, 0.0f, 357.5f), FVector(3.0f, 1580.0f, 3.0f));
    }
    for (float Y = -730.0f; Y <= 730.0f; Y += 220.0f)
    {
        AddLocalBox(CeilingGrid, FVector(0.0f, Y, 357.5f), FVector(2820.0f, 3.0f, 3.0f));
    }

    // Correct the first-pass painted parking hints: the photographed apron reads as plain worn asphalt.
    AddLocalBox(AsphaltCorrection, FVector(100.0f, FrontY - 610.0f, 6.5f),
        FVector(3450.0f, 850.0f, 5.0f));

    // Entrance-side trim and the small projecting canopy/vestibule character visible in close views.
    AddLocalBox(FacadeTrim, FVector(-1412.0f, FrontY - 12.0f, 155.0f), FVector(18.0f, 22.0f, 285.0f));
    AddLocalBox(FacadeTrim, FVector(-1218.0f, FrontY - 12.0f, 155.0f), FVector(18.0f, 22.0f, 285.0f));
    AddLocalBox(FacadeTrim, FVector(-1315.0f, FrontY - 12.0f, 292.0f), FVector(215.0f, 22.0f, 18.0f));
    AddLocalBox(DarkDetails, FVector(-1315.0f, FrontY - 165.0f, 308.0f), FVector(315.0f, 315.0f, 12.0f));

    // Thin parapet cap pieces keep the stepped silhouette crisp at street-view distance.
    const struct FCap { float X; float Z; float W; } Caps[] =
    {
        { -1170.0f, 451.0f, 660.0f },
        { -620.0f, 503.0f, 440.0f },
        { 0.0f, 563.0f, 820.0f },
        { 620.0f, 503.0f, 440.0f },
        { 1170.0f, 451.0f, 660.0f },
    };
    for (const FCap& Cap : Caps)
    {
        AddLocalBox(FacadeTrim, FVector(Cap.X, FrontY - 13.0f, Cap.Z), FVector(Cap.W, 30.0f, 8.0f));
    }

    // Exterior wall-light brackets above the advertising panels.
    for (const float X : { -980.0f, -600.0f, -220.0f, 260.0f, 760.0f })
    {
        AddLocalBox(DarkDetails, FVector(X, FrontY - 45.0f, 315.0f), FVector(8.0f, 42.0f, 8.0f));
        AddLocalBox(DarkDetails, FVector(X, FrontY - 62.0f, 307.0f), FVector(34.0f, 20.0f, 10.0f));
    }

    // Compact checkout queue rails/cart bay. Geometry stays low so it cannot trap the player.
    const float QueueY = -520.0f;
    for (const float X : { 260.0f, 530.0f, 800.0f, 1070.0f })
    {
        AddLocalBox(Metal, FVector(X, QueueY, 52.0f), FVector(5.0f, 250.0f, 104.0f));
        AddLocalBox(Metal, FVector(X, QueueY, 102.0f), FVector(5.0f, 250.0f, 5.0f));
    }
    AddLocalBox(Metal, FVector(1160.0f, -470.0f, 48.0f), FVector(7.0f, 290.0f, 96.0f));
    AddLocalBox(Metal, FVector(1300.0f, -470.0f, 48.0f), FVector(7.0f, 290.0f, 96.0f));
    AddLocalBox(Metal, FVector(1230.0f, -605.0f, 92.0f), FVector(140.0f, 7.0f, 7.0f));

    // Overhead checkout lane plates. The reference photos show compact numbered orange markers.
    for (int32 Lane = 0; Lane < 4; ++Lane)
    {
        const float X = 300.0f + static_cast<float>(Lane) * 270.0f;
        AddLocalBox(LaneSigns, FVector(X, -595.0f, 280.0f), FVector(46.0f, 8.0f, 54.0f));

        UTextRenderComponent* Number = NewObject<UTextRenderComponent>(Details,
            FName(*FString::Printf(TEXT("R141Silpo_CheckoutNumber_%d"), Lane + 1)));
        if (!Number) continue;
        Number->SetupAttachment(Root);
        Number->SetText(FText::AsNumber(Lane + 1));
        Number->SetTextRenderColor(FColor::White);
        Number->SetHorizontalAlignment(EHTA_Center);
        Number->SetWorldSize(38.0f);
        Number->SetRelativeLocation(FVector(X, -603.0f, 273.0f));
        Number->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
        Number->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Details->AddInstanceComponent(Number);
        Number->RegisterComponent();
    }

    // One photo-supported utility pole just beyond the right end of the facade.
    AddLocalBox(Metal, FVector(HalfLength + 165.0f, FrontY + 15.0f, 315.0f), FVector(16.0f, 16.0f, 630.0f));
    AddLocalBox(Metal, FVector(HalfLength + 165.0f, FrontY + 15.0f, 565.0f), FVector(155.0f, 12.0f, 12.0f));

    // Immediate right-side market edge visible in the street references. This is intentionally low-detail context,
    // not a claim about stall ownership or a permanent measured footprint.
    for (int32 Stall = 0; Stall < 3; ++Stall)
    {
        const float Y = -420.0f + static_cast<float>(Stall) * 300.0f;
        AddLocalBox(MarketEdge, FVector(HalfLength + 270.0f, Y, 225.0f), FVector(280.0f, 220.0f, 12.0f),
            FRotator(-8.0f, 0.0f, 0.0f));
        AddLocalBox(Metal, FVector(HalfLength + 170.0f, Y - 80.0f, 108.0f), FVector(7.0f, 7.0f, 216.0f));
        AddLocalBox(Metal, FVector(HalfLength + 370.0f, Y - 80.0f, 108.0f), FVector(7.0f, 7.0f, 216.0f));
    }

    UE_LOG(LogTemp, Display,
        TEXT("R14.1 Silpo detail pass built at [%.0f %.0f]: suspended ceiling, facade trim, checkout markers and immediate street context."),
        Site.X, Site.Y);
}
