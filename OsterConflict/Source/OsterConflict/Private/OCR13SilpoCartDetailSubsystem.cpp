#include "OCR13SilpoCartDetailSubsystem.h"

#include "OCGameMode.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

namespace
{
    constexpr float SilpoCartDetailDelaySeconds = 7.05f;
    constexpr FVector CartOrigin(250.0f, -1515.0f, 0.0f);
    constexpr float CartYawDegrees = -7.0f;

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
        Component->SetCullDistances(0, 55000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    FVector ToCartWorld(const FVector& Local)
    {
        return CartOrigin + FRotator(0.0f, CartYawDegrees, 0.0f).RotateVector(Local);
    }

    void AddBar(UInstancedStaticMeshComponent* Component, const FVector& LocalCenter,
        const FVector& SizeCm, const FRotator& LocalRotation = FRotator::ZeroRotator)
    {
        if (!Component) return;
        const FRotator WorldRotation = FRotator(0.0f, CartYawDegrees, 0.0f) + LocalRotation;
        Component->AddInstance(FTransform(WorldRotation, ToCartWorld(LocalCenter), SizeCm / 100.0f), false);
    }

    void AddWheel(UInstancedStaticMeshComponent* Component, const FVector& LocalCenter,
        const float DiameterCm, const float WidthCm)
    {
        if (!Component) return;
        const FRotator WheelRotation(90.0f, CartYawDegrees, 0.0f);
        Component->AddInstance(FTransform(WheelRotation, ToCartWorld(LocalCenter),
            FVector(DiameterCm / 100.0f, DiameterCm / 100.0f, WidthCm / 100.0f)), false);
    }
}

bool UOCR13SilpoCartDetailSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13SilpoCartDetailSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) ApplyCartDetail(*World);
        }), SilpoCartDetailDelaySeconds, false);
}

void UOCR13SilpoCartDetailSubsystem::ApplyCartDetail(UWorld& World)
{
    AActor* Model = FindSilpoModel(World);
    if (!Model || Model->ActorHasTag(TEXT("R13_SilpoCartDetailApplied"))) return;

    USceneComponent* Root = Model->GetRootComponent();
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Root || !Cube || !Cylinder || !Basic) return;

    UMaterialInstanceDynamic* MetalMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoCart_MetalMat"),
        FLinearColor(0.47f, 0.50f, 0.50f, 1.0f));
    UMaterialInstanceDynamic* DarkMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoCart_DarkMat"),
        FLinearColor(0.035f, 0.04f, 0.04f, 1.0f));
    UMaterialInstanceDynamic* HandleMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoCart_HandleMat"),
        FLinearColor(0.78f, 0.10f, 0.035f, 1.0f));

    UInstancedStaticMeshComponent* Frame = MakeISM(Model, Root, Cube, MetalMat,
        TEXT("R13SilpoCart_Frame"), true);
    UInstancedStaticMeshComponent* BasketWire = MakeISM(Model, Root, Cube, MetalMat,
        TEXT("R13SilpoCart_BasketWire"), true);
    UInstancedStaticMeshComponent* Handle = MakeISM(Model, Root, Cube, HandleMat,
        TEXT("R13SilpoCart_Handle"), true);
    UInstancedStaticMeshComponent* Wheels = MakeISM(Model, Root, Cylinder, DarkMat,
        TEXT("R13SilpoCart_Wheels"), true);

    // Lower chassis and uprights.
    AddBar(Frame, FVector(0.0f, 0.0f, 29.0f), FVector(92.0f, 48.0f, 4.0f));
    AddBar(Frame, FVector(-37.0f, -20.0f, 53.0f), FVector(4.0f, 4.0f, 50.0f));
    AddBar(Frame, FVector(-37.0f, 20.0f, 53.0f), FVector(4.0f, 4.0f, 50.0f));
    AddBar(Frame, FVector(38.0f, -20.0f, 46.0f), FVector(4.0f, 4.0f, 36.0f));
    AddBar(Frame, FVector(38.0f, 20.0f, 46.0f), FVector(4.0f, 4.0f, 36.0f));

    // Open supermarket basket, wider at the handle end and slightly shallower toward the front.
    AddBar(BasketWire, FVector(-3.0f, -25.0f, 75.0f), FVector(86.0f, 3.0f, 3.0f));
    AddBar(BasketWire, FVector(-3.0f, 25.0f, 75.0f), FVector(86.0f, 3.0f, 3.0f));
    AddBar(BasketWire, FVector(-3.0f, -21.0f, 47.0f), FVector(78.0f, 3.0f, 3.0f));
    AddBar(BasketWire, FVector(-3.0f, 21.0f, 47.0f), FVector(78.0f, 3.0f, 3.0f));
    AddBar(BasketWire, FVector(-45.0f, 0.0f, 75.0f), FVector(3.0f, 52.0f, 3.0f));
    AddBar(BasketWire, FVector(39.0f, 0.0f, 72.0f), FVector(3.0f, 44.0f, 3.0f));

    for (float X = -35.0f; X <= 30.0f; X += 13.0f)
    {
        AddBar(BasketWire, FVector(X, -23.0f, 61.0f), FVector(2.0f, 2.0f, 28.0f));
        AddBar(BasketWire, FVector(X, 23.0f, 61.0f), FVector(2.0f, 2.0f, 28.0f));
    }
    for (float Y = -18.0f; Y <= 18.0f; Y += 12.0f)
    {
        AddBar(BasketWire, FVector(-44.0f, Y, 61.0f), FVector(2.0f, 2.0f, 28.0f));
        AddBar(BasketWire, FVector(38.0f, Y, 59.0f), FVector(2.0f, 2.0f, 24.0f));
    }

    // Red/orange push handle visible as the strongest non-metal accent on the trolley in the supplied photo.
    AddBar(Handle, FVector(-49.0f, 0.0f, 91.0f), FVector(5.0f, 62.0f, 5.0f));
    AddBar(Frame, FVector(-44.0f, -27.0f, 79.0f), FVector(4.0f, 4.0f, 28.0f));
    AddBar(Frame, FVector(-44.0f, 27.0f, 79.0f), FVector(4.0f, 4.0f, 28.0f));

    // Four small caster wheels. Visual only, so they cannot trap the player or bots.
    AddWheel(Wheels, FVector(-34.0f, -21.0f, 12.0f), 18.0f, 7.0f);
    AddWheel(Wheels, FVector(-34.0f, 21.0f, 12.0f), 18.0f, 7.0f);
    AddWheel(Wheels, FVector(34.0f, -21.0f, 12.0f), 18.0f, 7.0f);
    AddWheel(Wheels, FVector(34.0f, 21.0f, 12.0f), 18.0f, 7.0f);

    Model->Tags.Add(TEXT("R13_SilpoCartDetailApplied"));
    UE_LOG(LogTemp, Display,
        TEXT("R13 Silpo cart detail: procedural supermarket trolley added at photographed frontage position; collision/nav disabled."));
}
