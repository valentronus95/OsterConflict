#include "OCMuseumBreakableWindow.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

AOCMuseumBreakableWindow::AOCMuseumBreakableWindow()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Cube = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;
    if (!Cube || !SceneRoot) return;

    // Base AOCBreakableWindow is 200 x 155 cm. These details scale together with the actor
    // when the subsystem fits a photographed opening.
    CenterMullion = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MuseumCenterMullion"));
    CenterMullion->SetupAttachment(SceneRoot);
    CenterMullion->SetStaticMesh(Cube);
    CenterMullion->SetRelativeLocation(FVector(0.0f, -3.0f, -17.0f));
    CenterMullion->SetRelativeScale3D(FVector(8.0f, 5.0f, 112.0f) / 100.0f);
    CenterMullion->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    UpperTransom = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MuseumUpperTransom"));
    UpperTransom->SetupAttachment(SceneRoot);
    UpperTransom->SetStaticMesh(Cube);
    UpperTransom->SetRelativeLocation(FVector(0.0f, -3.0f, 48.0f));
    UpperTransom->SetRelativeScale3D(FVector(196.0f, 5.0f, 8.0f) / 100.0f);
    UpperTransom->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AOCMuseumBreakableWindow::BeginPlay()
{
    Super::BeginPlay();
    ApplyMuseumMaterials();
}

void AOCMuseumBreakableWindow::ApplyMuseumMaterials()
{
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    UMaterialInterface* Glass = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Glass_Window.Glass_Window"));
    if (!Basic) return;

    UMaterialInstanceDynamic* FrameMaterial = UMaterialInstanceDynamic::Create(Basic, this,
        MakeUniqueObjectName(this, UMaterialInstanceDynamic::StaticClass(), FName(TEXT("MuseumWindowFrame"))));
    if (FrameMaterial)
    {
        FrameMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.82f, 0.82f, 0.75f, 1.0f));
    }

    if (GlassPane && Glass)
    {
        GlassPane->SetMaterial(0, Glass);
    }

    UStaticMeshComponent* FrameParts[] =
    {
        FrameLeft.Get(), FrameRight.Get(), FrameTop.Get(), FrameBottom.Get(),
        CenterMullion.Get(), UpperTransom.Get()
    };
    for (UStaticMeshComponent* Component : FrameParts)
    {
        if (Component && FrameMaterial) Component->SetMaterial(0, FrameMaterial);
    }

    for (const TObjectPtr<UStaticMeshComponent>& Shard : DebrisPieces)
    {
        if (Shard && Glass) Shard->SetMaterial(0, Glass);
    }
}
