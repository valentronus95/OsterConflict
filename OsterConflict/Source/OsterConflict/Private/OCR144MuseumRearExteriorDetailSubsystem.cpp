#include "OCR144MuseumRearExteriorDetailSubsystem.h"

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
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float R144RearDetailDelaySeconds = 6.26f;

    UMaterialInstanceDynamic* MakeMID(AActor* Owner, UMaterialInterface* Base,
        const TCHAR* Name, const FLinearColor& Color)
    {
        if (!Owner || !Base) return nullptr;
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(
            Base, Owner, MakeUniqueObjectName(Owner, UMaterialInstanceDynamic::StaticClass(), FName(Name)));
        if (Material) Material->SetVectorParameterValue(TEXT("Color"), Color);
        return Material;
    }

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        UMaterialInterface* Material, const TCHAR* Name, const bool bCollision = false)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(
            Owner, MakeUniqueObjectName(Owner, UInstancedStaticMeshComponent::StaticClass(), FName(Name)));
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material) Component->SetMaterial(0, Material);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(bCollision);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const FVector& SizeCm, const FRotator& Rotation = FRotator::ZeroRotator)
    {
        if (!Component || SizeCm.GetMin() <= 0.0f) return;
        Component->AddInstance(FTransform(Rotation, Center, SizeCm / 100.0f), true);
    }

    void AddFittedPipe(UInstancedStaticMeshComponent* Component, UStaticMesh* Cylinder,
        const FVector& Center, const float DiameterCm, const float LengthCm, const FRotator& Rotation)
    {
        if (!Component || !Cylinder || DiameterCm <= 0.0f || LengthCm <= 0.0f) return;
        const FBoxSphereBounds Bounds = Cylinder->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) return;

        const FVector Scale(DiameterCm / NativeSize.X, DiameterCm / NativeSize.Y, LengthCm / NativeSize.Z);
        const FQuat Quat = Rotation.Quaternion();
        const FVector Location = Center - Quat.RotateVector(Bounds.Origin * Scale);
        Component->AddInstance(FTransform(Quat, Location, Scale), true);
    }

    void AddServiceSteps(UInstancedStaticMeshComponent* Concrete, const FVector& Museum)
    {
        // REF-11/13: modest exterior steps on the service-door end wall. Door faces +X.
        for (int32 Step = 0; Step < 4; ++Step)
        {
            const float Depth = 68.0f + static_cast<float>(Step) * 28.0f;
            const float Height = 16.0f + static_cast<float>(Step) * 14.0f;
            const float X = 900.0f + static_cast<float>(Step) * 37.0f;
            AddBox(Concrete, Museum + FVector(X, 115.0f, Height * 0.5f),
                FVector(Depth, 220.0f, Height));
        }
    }

    void AddAnnexEdgeDetail(UInstancedStaticMeshComponent* DarkBase,
        UInstancedStaticMeshComponent* Fascia, const FVector& Museum)
    {
        // The reference pack confirms the annex mass and roof edge but does not reliably confirm door/window count.
        // Detail only the known silhouette and base, deliberately leaving unseen openings unspecified.
        AddBox(DarkBase, Museum + FVector(1020.0f, 235.0f, 47.0f), FVector(440.0f, 480.0f, 48.0f));
        AddBox(Fascia, Museum + FVector(1020.0f, -8.0f, 292.0f), FVector(455.0f, 16.0f, 26.0f));
        AddBox(Fascia, Museum + FVector(1020.0f, 478.0f, 292.0f), FVector(455.0f, 16.0f, 26.0f));
        AddBox(Fascia, Museum + FVector(793.0f, 235.0f, 292.0f), FVector(16.0f, 470.0f, 26.0f));
        AddBox(Fascia, Museum + FVector(1247.0f, 235.0f, 292.0f), FVector(16.0f, 470.0f, 26.0f));
    }

    void AddDrainage(UInstancedStaticMeshComponent* Pipe, UStaticMesh* Cylinder, const FVector& Museum)
    {
        // Long eave gutters. Cylinders are used here so the silhouette reads as actual drainage rather than square trim.
        AddFittedPipe(Pipe, Cylinder, Museum + FVector(0.0f, -458.0f, 414.0f), 11.0f, 1710.0f,
            FRotator(0.0f, 90.0f, 0.0f));
        AddFittedPipe(Pipe, Cylinder, Museum + FVector(0.0f, 458.0f, 414.0f), 11.0f, 1710.0f,
            FRotator(0.0f, 90.0f, 0.0f));

        // Downspouts at the photographed long-side corners, plus one annex drain.
        for (const FVector& Offset :
            { FVector(-832.0f, -454.0f, 215.0f), FVector(832.0f, 454.0f, 215.0f) })
        {
            AddFittedPipe(Pipe, Cylinder, Museum + Offset, 9.0f, 395.0f, FRotator::ZeroRotator);
        }
        AddFittedPipe(Pipe, Cylinder, Museum + FVector(1230.0f, 458.0f, 175.0f),
            8.0f, 285.0f, FRotator::ZeroRotator);

        // Short lower elbows keep the vertical pipes from ending abruptly in midair.
        AddFittedPipe(Pipe, Cylinder, Museum + FVector(-806.0f, -454.0f, 28.0f),
            9.0f, 55.0f, FRotator(0.0f, 90.0f, 0.0f));
        AddFittedPipe(Pipe, Cylinder, Museum + FVector(806.0f, 454.0f, 28.0f),
            9.0f, 55.0f, FRotator(0.0f, 90.0f, 0.0f));
    }
}

bool UOCR144MuseumRearExteriorDetailSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR144MuseumRearExteriorDetailSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) BuildRearExteriorDetail(*World);
        }), R144RearDetailDelaySeconds, false);
}

void UOCR144MuseumRearExteriorDetailSubsystem::BuildRearExteriorDetail(UWorld& World) const
{
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        if (AActor* Actor = *It; Actor && Actor->ActorHasTag(TEXT("R144_MuseumRearExteriorDetail"))) return;
    }

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube || !Cylinder || !Basic) return;

    AActor* DetailActor = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!DetailActor) return;
    DetailActor->SetReplicates(false);
    DetailActor->Tags.Add(TEXT("R144_MuseumRearExteriorDetail"));

    USceneComponent* Root = NewObject<USceneComponent>(DetailActor,
        MakeUniqueObjectName(DetailActor, USceneComponent::StaticClass(), FName(TEXT("R144MuseumRearDetailRoot"))));
    if (!Root)
    {
        DetailActor->Destroy();
        return;
    }
    DetailActor->SetRootComponent(Root);
    DetailActor->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UMaterialInstanceDynamic* Zinc = MakeMID(DetailActor, Basic, TEXT("R144MuseumMID_AgedZinc"),
        FLinearColor(0.37f, 0.40f, 0.39f, 1.0f));
    UMaterialInstanceDynamic* ConcreteMat = MakeMID(DetailActor, Basic, TEXT("R144MuseumMID_ServiceConcrete"),
        FLinearColor(0.43f, 0.43f, 0.40f, 1.0f));
    UMaterialInstanceDynamic* DarkBaseMat = MakeMID(DetailActor, Basic, TEXT("R144MuseumMID_AnnexBase"),
        FLinearColor(0.10f, 0.095f, 0.085f, 1.0f));
    UMaterialInstanceDynamic* FasciaMat = MakeMID(DetailActor, Basic, TEXT("R144MuseumMID_AnnexFascia"),
        FLinearColor(0.38f, 0.25f, 0.20f, 1.0f));

    UInstancedStaticMeshComponent* Pipe = MakeISM(DetailActor, Root, Cylinder, Zinc,
        TEXT("R144Museum_GuttersAndDownspouts"));
    UInstancedStaticMeshComponent* Concrete = MakeISM(DetailActor, Root, Cube, ConcreteMat,
        TEXT("R144Museum_ServiceSteps"), true);
    UInstancedStaticMeshComponent* DarkBase = MakeISM(DetailActor, Root, Cube, DarkBaseMat,
        TEXT("R144Museum_AnnexDarkBase"));
    UInstancedStaticMeshComponent* Fascia = MakeISM(DetailActor, Root, Cube, FasciaMat,
        TEXT("R144Museum_AnnexEdgeDetail"));

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    AddDrainage(Pipe, Cylinder, Museum);
    AddServiceSteps(Concrete, Museum);
    AddAnnexEdgeDetail(DarkBase, Fascia, Museum);

    UE_LOG(LogTemp, Display,
        TEXT("R14.4 museum rear exterior: cylindrical gutters/downspouts, service-entry steps and conservative annex base/fascia detail built from REF-03/09/11/13/19/20."));
}
