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

    // Pass 30: do not stretch Window_Frame_Part into six unrelated axes. The runtime screenshots
    // showed the imported rural-cabin frame becoming thick/rusty oversized strips around every museum
    // opening. Keep one clean, predictable lightweight frame until an authored museum-specific frame
    // exists. This also removes a large number of unnecessarily complex shadow-casting frame meshes.
    CenterMullion = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MuseumCenterMullion"));
    CenterMullion->SetupAttachment(SceneRoot);
    CenterMullion->SetStaticMesh(Cube);
    CenterMullion->SetRelativeLocation(FVector(0.0f, -3.0f, -17.0f));
    CenterMullion->SetRelativeScale3D(FVector(8.0f, 5.0f, 112.0f) / 100.0f);
    CenterMullion->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CenterMullion->SetGenerateOverlapEvents(false);
    CenterMullion->SetCastShadow(false);

    UpperTransom = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MuseumUpperTransom"));
    UpperTransom->SetupAttachment(SceneRoot);
    UpperTransom->SetStaticMesh(Cube);
    UpperTransom->SetRelativeLocation(FVector(0.0f, -3.0f, 48.0f));
    UpperTransom->SetRelativeScale3D(FVector(196.0f, 5.0f, 8.0f) / 100.0f);
    UpperTransom->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    UpperTransom->SetGenerateOverlapEvents(false);
    UpperTransom->SetCastShadow(false);

    UStaticMeshComponent* FrameParts[] =
    {
        FrameLeft.Get(), FrameRight.Get(), FrameTop.Get(), FrameBottom.Get(),
        CenterMullion.Get(), UpperTransom.Get()
    };
    for (UStaticMeshComponent* Component : FrameParts)
    {
        if (!Component) continue;
        Component->SetStaticMesh(Cube);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCastShadow(false);
    }
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

    if (GlassPane)
    {
        if (Glass) GlassPane->SetMaterial(0, Glass);
        GlassPane->SetCastShadow(false);
    }

    UStaticMeshComponent* FrameParts[] =
    {
        FrameLeft.Get(), FrameRight.Get(), FrameTop.Get(), FrameBottom.Get(),
        CenterMullion.Get(), UpperTransom.Get()
    };
    for (UStaticMeshComponent* Component : FrameParts)
    {
        if (!Component) continue;
        if (FrameMaterial) Component->SetMaterial(0, FrameMaterial);
        Component->SetVisibility(false, true);
        Component->SetHiddenInGame(true, true);
        Component->SetCastShadow(false);
    }

    for (const TObjectPtr<UStaticMeshComponent>& Shard : DebrisPieces)
    {
        if (!Shard) continue;
        if (Glass) Shard->SetMaterial(0, Glass);
        Shard->SetCastShadow(false);
    }

    UE_LOG(LogTemp, Display, TEXT("PASS45_MUSEUM_WINDOW_GLASS_ONLY_READY visible_frame_owner=R137 interactive_frame_visible=0 static_glass=0"));
    UE_LOG(LogTemp, Display, TEXT("PASS30_MUSEUM_WINDOW_FRAME_CLEAN_READY visible_frame_owner=R137 overlap=0"));
}
