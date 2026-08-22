#include "OCMuseumBreakableWindow.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    const FName AuthoredMuseumFrameTag(TEXT("OC_AuthoredMuseumWindowFrame"));

    void FitAuthoredFramePart(UStaticMeshComponent* Component, UStaticMesh* Mesh,
        const FVector& DesiredCenter, const FVector& DesiredSizeCm)
    {
        if (!Component || !Mesh) return;

        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        const float NativeLengths[3] = { NativeSize.X, NativeSize.Y, NativeSize.Z };
        const float DesiredLengths[3] = { DesiredSizeCm.X, DesiredSizeCm.Y, DesiredSizeCm.Z };

        int32 NativeLongestAxis = 0;
        int32 DesiredLongestAxis = 0;
        for (int32 Axis = 1; Axis < 3; ++Axis)
        {
            if (NativeLengths[Axis] > NativeLengths[NativeLongestAxis]) NativeLongestAxis = Axis;
            if (DesiredLengths[Axis] > DesiredLengths[DesiredLongestAxis]) DesiredLongestAxis = Axis;
        }

        if (NativeLengths[NativeLongestAxis] <= 1.0f || DesiredLengths[DesiredLongestAxis] <= 1.0f) return;

        const FVector UnitAxes[3] = { FVector::ForwardVector, FVector::RightVector, FVector::UpVector };
        const FQuat AxisRotation = FQuat::FindBetweenNormals(UnitAxes[NativeLongestAxis], UnitAxes[DesiredLongestAxis]);
        const float UniformScale = DesiredLengths[DesiredLongestAxis] / NativeLengths[NativeLongestAxis];
        const FVector FittedCenterOffset = AxisRotation.RotateVector(Bounds.Origin * UniformScale);

        Component->SetStaticMesh(Mesh);
        Component->SetRelativeRotation(AxisRotation.Rotator());
        Component->SetRelativeScale3D(FVector(UniformScale));
        Component->SetRelativeLocation(DesiredCenter - FittedCenterOffset);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->ComponentTags.AddUnique(AuthoredMuseumFrameTag);
    }
}

AOCMuseumBreakableWindow::AOCMuseumBreakableWindow()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> AuthoredFrameFinder(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Window_Frame_Part.Window_Frame_Part"));

    UStaticMesh* Cube = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;
    UStaticMesh* AuthoredFrame = AuthoredFrameFinder.Succeeded() ? AuthoredFrameFinder.Object : nullptr;
    if (!Cube || !SceneRoot) return;

    // Base AOCBreakableWindow is 200 x 155 cm. Keep GlassPane as the replicated collision/break state,
    // while the visible museum frame prefers the authored rural-cabin frame profile when hydrated.
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

    if (AuthoredFrame)
    {
        // Fit by authored bounds and longest-axis orientation rather than assuming the import is a 100 cm cube.
        // Uniform scale preserves the actual frame profile; only its longest authored axis is remapped to the
        // required horizontal/vertical direction.
        FitAuthoredFramePart(FrameLeft, AuthoredFrame,
            FVector(-105.0f, 0.0f, 0.0f), FVector(10.0f, 10.0f, 175.0f));
        FitAuthoredFramePart(FrameRight, AuthoredFrame,
            FVector(105.0f, 0.0f, 0.0f), FVector(10.0f, 10.0f, 175.0f));
        FitAuthoredFramePart(FrameTop, AuthoredFrame,
            FVector(0.0f, 0.0f, 82.5f), FVector(200.0f, 10.0f, 10.0f));
        FitAuthoredFramePart(FrameBottom, AuthoredFrame,
            FVector(0.0f, 0.0f, -82.5f), FVector(200.0f, 10.0f, 10.0f));
        FitAuthoredFramePart(CenterMullion, AuthoredFrame,
            FVector(0.0f, -3.0f, -17.0f), FVector(8.0f, 5.0f, 112.0f));
        FitAuthoredFramePart(UpperTransom, AuthoredFrame,
            FVector(0.0f, -3.0f, 48.0f), FVector(196.0f, 5.0f, 8.0f));
    }
    else
    {
        // Legacy Cube fallback remains only for an unhydrated/missing authored asset.
        UStaticMeshComponent* FrameParts[] =
        {
            FrameLeft.Get(), FrameRight.Get(), FrameTop.Get(), FrameBottom.Get(),
            CenterMullion.Get(), UpperTransom.Get()
        };
        for (UStaticMeshComponent* Component : FrameParts)
        {
            if (Component)
            {
                Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                Component->SetGenerateOverlapEvents(false);
            }
        }
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
        // Preserve the authored frame mesh/materials when present. The flat BasicShape material is only
        // a fallback for legacy Cube frame geometry, otherwise a real model would be made to look fake again.
        if (Component && FrameMaterial && !Component->ComponentHasTag(AuthoredMuseumFrameTag))
        {
            Component->SetMaterial(0, FrameMaterial);
        }
    }

    for (const TObjectPtr<UStaticMeshComponent>& Shard : DebrisPieces)
    {
        if (Shard && Glass) Shard->SetMaterial(0, Glass);
    }
}
