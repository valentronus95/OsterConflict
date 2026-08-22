#include "OCR146CultureHousePhotoModelSubsystem.h"

#include "OCGameMode.h"
#include "OCGeoReference.h"

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
    // The sector actor is spawned synchronously from GameMode BeginPlay. This short startup handoff keeps the
    // Culture House out of the old multi-second late-replacement chain while still letting source geometry exist first.
    constexpr float CultureHouseStartupDelaySeconds = 0.28f;
    // Hranovskoho 3 is verified. Exact facade bearing is not survey data, so yaw stays provisional rather than invented.
    constexpr float CultureHouseYawDegrees = 0.0f;

    FVector CultureHouseAnchor()
    {
        const FOCGeoReferencePoint Ref = FOCGeoReference::CultureHouse();
        return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, 0.0);
    }

    UMaterialInstanceDynamic* MakeColor(AActor* Owner, UMaterialInterface* Base,
        const FName Name, const FLinearColor& Color)
    {
        if (!Owner || !Base) return nullptr;
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Owner, Name);
        if (Material) Material->SetVectorParameterValue(TEXT("Color"), Color);
        return Material;
    }

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        UMaterialInterface* Material, const TCHAR* RequestedName, const bool bCollision, const bool bShadow = true)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(
            Owner, MakeUniqueObjectName(Owner, UInstancedStaticMeshComponent::StaticClass(), FName(RequestedName)));
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material)
        {
            const int32 Slots = FMath::Max(1, Mesh->GetStaticMaterials().Num());
            for (int32 Slot = 0; Slot < Slots; ++Slot) Component->SetMaterial(Slot, Material);
        }
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(bCollision);
        Component->SetCastShadow(bShadow);
        Component->SetCullDistances(0, 100000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center, const FVector& SizeCm)
    {
        if (Component) Component->AddInstance(FTransform(FRotator::ZeroRotator, Center, SizeCm / 100.0f), false);
    }

    void AddBoxRotated(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const FVector& SizeCm, const FRotator& Rotation)
    {
        if (Component) Component->AddInstance(FTransform(Rotation, Center, SizeCm / 100.0f), false);
    }

    void AddCylinder(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const float DiameterCm, const float HeightCm, const FRotator& Rotation = FRotator::ZeroRotator)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(Rotation, Center,
            FVector(DiameterCm / 100.0f, DiameterCm / 100.0f, HeightCm / 100.0f)), false);
    }

    void AddFittedFrameSegment(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const FVector& TargetDirection, const float DesiredLengthCm)
    {
        if (!Component || !Component->GetStaticMesh() || DesiredLengthCm <= 1.0f) return;

        const FBoxSphereBounds Bounds = Component->GetStaticMesh()->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        const float NativeLengths[3] = { NativeSize.X, NativeSize.Y, NativeSize.Z };
        int32 NativeLongestAxis = 0;
        for (int32 Axis = 1; Axis < 3; ++Axis)
        {
            if (NativeLengths[Axis] > NativeLengths[NativeLongestAxis]) NativeLongestAxis = Axis;
        }
        if (NativeLengths[NativeLongestAxis] <= 1.0f || TargetDirection.IsNearlyZero()) return;

        const FVector UnitAxes[3] = { FVector::ForwardVector, FVector::RightVector, FVector::UpVector };
        const FQuat Rotation = FQuat::FindBetweenNormals(
            UnitAxes[NativeLongestAxis], TargetDirection.GetSafeNormal());
        const float UniformScale = DesiredLengthCm / NativeLengths[NativeLongestAxis];
        const FVector CenterOffset = Rotation.RotateVector(Bounds.Origin * UniformScale);
        Component->AddInstance(FTransform(Rotation, Center - CenterOffset, FVector(UniformScale)), false);
    }

    void AddAuthoredRectFrame(UInstancedStaticMeshComponent* AuthoredFrames, const FVector& Center,
        const FVector& HorizontalDirection, const float WidthCm, const float HeightCm)
    {
        if (!AuthoredFrames || !AuthoredFrames->GetStaticMesh()) return;

        const FVector Horizontal = HorizontalDirection.GetSafeNormal();
        const FVector Vertical = FVector::UpVector;
        const float HalfWidth = WidthCm * 0.5f;
        const float HalfHeight = HeightCm * 0.5f;

        AddFittedFrameSegment(AuthoredFrames, Center + Vertical * HalfHeight, Horizontal, WidthCm);
        AddFittedFrameSegment(AuthoredFrames, Center - Vertical * HalfHeight, Horizontal, WidthCm);
        AddFittedFrameSegment(AuthoredFrames, Center - Horizontal * HalfWidth, Vertical, HeightCm);
        AddFittedFrameSegment(AuthoredFrames, Center + Horizontal * HalfWidth, Vertical, HeightCm);
    }

    void AddSideWindow(UInstancedStaticMeshComponent* FallbackFrames,
        UInstancedStaticMeshComponent* AuthoredFrames, UInstancedStaticMeshComponent* Glass,
        const float X, const float Y, const float Z)
    {
        const float OutwardX = X + (X < 0.0f ? -14.0f : 14.0f);
        if (AuthoredFrames && AuthoredFrames->GetStaticMesh())
        {
            AddAuthoredRectFrame(AuthoredFrames, FVector(OutwardX, Y, Z), FVector::RightVector, 250.0f, 220.0f);
        }
        else
        {
            AddBox(FallbackFrames, FVector(X, Y, Z), FVector(24.0f, 250.0f, 220.0f));
        }
        AddBox(Glass, FVector(OutwardX, Y, Z), FVector(8.0f, 205.0f, 176.0f));
    }
}

bool UOCR146CultureHousePhotoModelSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR146CultureHousePhotoModelSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) BuildCultureHouse(*World);
        }), CultureHouseStartupDelaySeconds, false);
}

void UOCR146CultureHousePhotoModelSubsystem::BuildCultureHouse(UWorld& World) const
{
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Existing = *It;
        if (!Existing) continue;
        if (Existing->ActorHasTag(TEXT("R146_CultureHouseAuthoritative"))) return;
        if (Existing->ActorHasTag(TEXT("R13_CultureHousePhotoModel")))
        {
            Existing->Destroy();
        }
    }

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    UStaticMesh* AuthoredWindowFrame = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Window_Frame_Part.Window_Frame_Part"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    UMaterialInterface* GlassAsset = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Glass_Window.Glass_Window"));
    if (!Cube || !Cylinder || !Basic) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = MakeUniqueObjectName(&World, AActor::StaticClass(), TEXT("R146_OsterCultureHouse"));
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Model = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
    if (!Model) return;
    Model->SetReplicates(false);
    Model->SetActorEnableCollision(true);
    Model->Tags.Add(TEXT("R146_CultureHouseAuthoritative"));
    Model->Tags.Add(TEXT("CultureHouseOster_Hranovskoho3"));

    USceneComponent* Root = NewObject<USceneComponent>(Model,
        MakeUniqueObjectName(Model, USceneComponent::StaticClass(), TEXT("R146_CultureHouseRoot")));
    if (!Root)
    {
        Model->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    Model->SetRootComponent(Root);
    Model->AddInstanceComponent(Root);
    Root->RegisterComponent();
    Model->SetActorLocationAndRotation(CultureHouseAnchor(), FRotator(0.0f, CultureHouseYawDegrees, 0.0f));

    UMaterialInstanceDynamic* Facade = MakeColor(Model, Basic, TEXT("R146Culture_FacadeMat"),
        FLinearColor(0.63f, 0.45f, 0.26f, 1.0f));
    UMaterialInstanceDynamic* Classical = MakeColor(Model, Basic, TEXT("R146Culture_TrimMat"),
        FLinearColor(0.80f, 0.78f, 0.68f, 1.0f));
    UMaterialInstanceDynamic* DoorWood = MakeColor(Model, Basic, TEXT("R146Culture_DoorMat"),
        FLinearColor(0.30f, 0.13f, 0.07f, 1.0f));
    UMaterialInstanceDynamic* RoofMat = MakeColor(Model, Basic, TEXT("R146Culture_RoofMat"),
        FLinearColor(0.18f, 0.15f, 0.12f, 1.0f));
    UMaterialInstanceDynamic* Stone = MakeColor(Model, Basic, TEXT("R146Culture_StoneMat"),
        FLinearColor(0.41f, 0.39f, 0.35f, 1.0f));
    UMaterialInstanceDynamic* Path = MakeColor(Model, Basic, TEXT("R146Culture_PathMat"),
        FLinearColor(0.27f, 0.27f, 0.25f, 1.0f));
    UMaterialInstanceDynamic* GlassFallback = MakeColor(Model, Basic, TEXT("R146Culture_GlassFallback"),
        FLinearColor(0.10f, 0.20f, 0.25f, 1.0f));
    UMaterialInterface* GlassMat = GlassAsset ? GlassAsset : GlassFallback;

    UInstancedStaticMeshComponent* Shell = MakeISM(Model, Root, Cube, Facade, TEXT("R146Culture_Shell"), true);
    UInstancedStaticMeshComponent* Trim = MakeISM(Model, Root, Cube, Classical, TEXT("R146Culture_Trim"), false);
    UInstancedStaticMeshComponent* Columns = MakeISM(Model, Root, Cylinder, Classical, TEXT("R146Culture_Columns"), false);
    UInstancedStaticMeshComponent* DoorFrames = MakeISM(Model, Root, Cube, Classical, TEXT("R146Culture_DoorFrames"), false);
    UInstancedStaticMeshComponent* AuthoredWindowFrames = AuthoredWindowFrame
        ? MakeISM(Model, Root, AuthoredWindowFrame, nullptr, TEXT("R146Culture_AuthoredWindowFrames"), false)
        : nullptr;
    UInstancedStaticMeshComponent* Doors = MakeISM(Model, Root, Cube, DoorWood, TEXT("R146Culture_Doors"), false);
    UInstancedStaticMeshComponent* Glass = MakeISM(Model, Root, Cube, GlassMat, TEXT("R146Culture_Glass"), false);
    UInstancedStaticMeshComponent* RoundGlass = MakeISM(Model, Root, Cylinder, GlassMat, TEXT("R146Culture_ArchedGlass"), false);
    UInstancedStaticMeshComponent* Roof = MakeISM(Model, Root, Cube, RoofMat, TEXT("R146Culture_Roof"), false);
    UInstancedStaticMeshComponent* Stonework = MakeISM(Model, Root, Cube, Stone, TEXT("R146Culture_Stonework"), true);
    UInstancedStaticMeshComponent* Ground = MakeISM(Model, Root, Cube, Path, TEXT("R146Culture_Ground"), true, false);

    // Main hall. All dimensions are local to this one site root, so a map relocation moves the complete building
    // atomically and cannot leave a second shell inside another landmark.
    AddBox(Shell, FVector(0.0f, 0.0f, 365.0f), FVector(3200.0f, 1850.0f, 730.0f));
    AddBox(Stonework, FVector(0.0f, 0.0f, 42.0f), FVector(3280.0f, 1910.0f, 84.0f));

    AddBox(Trim, FVector(0.0f, -940.0f, 690.0f), FVector(3260.0f, 42.0f, 72.0f));
    AddBox(Trim, FVector(-1560.0f, -940.0f, 365.0f), FVector(56.0f, 42.0f, 620.0f));
    AddBox(Trim, FVector(1560.0f, -940.0f, 365.0f), FVector(56.0f, 42.0f, 620.0f));

    const float ColumnXs[] = { -1130.0f, -680.0f, -230.0f, 230.0f, 680.0f, 1130.0f };
    for (const float X : ColumnXs)
    {
        AddCylinder(Columns, FVector(X, -1080.0f, 365.0f), 92.0f, 610.0f);
        AddCylinder(Columns, FVector(X, -1080.0f, 75.0f), 126.0f, 34.0f);
        AddCylinder(Columns, FVector(X, -1080.0f, 660.0f), 132.0f, 40.0f);
    }
    AddBox(Trim, FVector(0.0f, -1070.0f, 708.0f), FVector(2650.0f, 225.0f, 86.0f));

    for (const float X : { -650.0f, 0.0f, 650.0f })
    {
        AddBox(DoorFrames, FVector(X, -954.0f, 300.0f), FVector(250.0f, 30.0f, 420.0f));
        AddBox(Doors, FVector(X, -972.0f, 215.0f), FVector(195.0f, 24.0f, 250.0f));
        AddBox(Glass, FVector(X, -977.0f, 415.0f), FVector(182.0f, 10.0f, 150.0f));
        AddCylinder(RoundGlass, FVector(X, -982.0f, 520.0f), 176.0f, 9.0f, FRotator(0.0f, 0.0f, 90.0f));
    }

    AddBox(Trim, FVector(0.0f, -1065.0f, 760.0f), FVector(2740.0f, 235.0f, 34.0f));
    AddBoxRotated(Trim, FVector(-650.0f, -1065.0f, 925.0f), FVector(1420.0f, 220.0f, 42.0f),
        FRotator(-14.0f, 0.0f, 0.0f));
    AddBoxRotated(Trim, FVector(650.0f, -1065.0f, 925.0f), FVector(1420.0f, 220.0f, 42.0f),
        FRotator(14.0f, 0.0f, 0.0f));
    AddBox(Roof, FVector(0.0f, 60.0f, 752.0f), FVector(3300.0f, 1960.0f, 66.0f));
    AddBoxRotated(Roof, FVector(0.0f, -470.0f, 815.0f), FVector(3260.0f, 1040.0f, 56.0f),
        FRotator(0.0f, 0.0f, -8.0f));
    AddBoxRotated(Roof, FVector(0.0f, 470.0f, 815.0f), FVector(3260.0f, 1040.0f, 56.0f),
        FRotator(0.0f, 0.0f, 8.0f));

    AddBox(Stonework, FVector(0.0f, -1170.0f, 64.0f), FVector(2500.0f, 460.0f, 44.0f));
    AddBox(Stonework, FVector(0.0f, -1375.0f, 43.0f), FVector(2660.0f, 190.0f, 34.0f));
    AddBox(Stonework, FVector(0.0f, -1500.0f, 25.0f), FVector(2780.0f, 150.0f, 26.0f));
    AddBox(Ground, FVector(0.0f, -2450.0f, 10.0f), FVector(760.0f, 1900.0f, 20.0f));

    // Conservative side/rear detail. Where the authored modular frame is hydrated, use four genuine
    // frame-profile segments around each glass pane instead of a solid Cube plate. Glass geometry and
    // building collision remain owned exactly as before.
    for (const float Y : { -560.0f, 0.0f, 560.0f })
    {
        AddSideWindow(DoorFrames, AuthoredWindowFrames, Glass, -1608.0f, Y, 390.0f);
        AddSideWindow(DoorFrames, AuthoredWindowFrames, Glass, 1608.0f, Y, 390.0f);
    }
    for (const float X : { -1050.0f, -525.0f, 0.0f, 525.0f, 1050.0f })
    {
        if (AuthoredWindowFrames && AuthoredWindowFrames->GetStaticMesh())
        {
            AddAuthoredRectFrame(AuthoredWindowFrames, FVector(X, 952.0f, 390.0f),
                FVector::ForwardVector, 230.0f, 220.0f);
        }
        else
        {
            AddBox(DoorFrames, FVector(X, 938.0f, 390.0f), FVector(230.0f, 24.0f, 220.0f));
        }
        AddBox(Glass, FVector(X, 952.0f, 390.0f), FVector(190.0f, 8.0f, 178.0f));
    }
    // Eaves and two downspouts make the long elevations read as one finished building rather than nested primitives.
    AddBox(Trim, FVector(0.0f, 948.0f, 700.0f), FVector(3260.0f, 32.0f, 28.0f));
    AddBox(Trim, FVector(-1580.0f, 930.0f, 345.0f), FVector(20.0f, 24.0f, 650.0f));
    AddBox(Trim, FVector(1580.0f, 930.0f, 345.0f), FVector(20.0f, 24.0f, 650.0f));

    const FVector Site = CultureHouseAnchor();
    UE_LOG(LogTemp, Display,
        TEXT("R14.6 Culture House authoritative owner built at Hranovskoho 3 [%.1f %.1f]; separate site root, six-column facade, three arched entrance bays and conservative side/rear detail active."),
        Site.X, Site.Y);
}
