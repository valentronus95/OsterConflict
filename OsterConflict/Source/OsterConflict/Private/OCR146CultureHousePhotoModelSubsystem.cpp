#include "OCR146CultureHousePhotoModelSubsystem.h"

#include "OCGameMode.h"
#include "OCGeoReference.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float CultureHouseStartupDelaySeconds = 0.28f;
    // Hranovskoho 3 is verified. Exact facade bearing is still provisional rather than invented.
    constexpr float CultureHouseYawDegrees = 0.0f;

    FVector CultureHouseAnchor()
    {
        const FOCGeoReferencePoint Ref = FOCGeoReference::CultureHouse();
        return FOCGeoReference::ToLocalCm(Ref.Latitude, Ref.Longitude, 0.0);
    }

    UInstancedStaticMeshComponent* MakeAuthoredISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        const TCHAR* RequestedName, const bool bCollision, const bool bShadow = true)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(
            Owner, MakeUniqueObjectName(Owner, UInstancedStaticMeshComponent::StaticClass(), FName(RequestedName)));
        if (!Component) return nullptr;

        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        // Pass45 Gate K: authored mesh materials stay authoritative. Do not repaint a landmark with
        // engine primitive materials just to approximate a reference colour in source code.
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

    bool AddFittedAuthoredMesh(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const FVector& DesiredSizeCm, const FRotator& Rotation = FRotator::ZeroRotator)
    {
        if (!Component || !Component->GetStaticMesh()) return false;
        UStaticMesh* Mesh = Component->GetStaticMesh();
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) return false;

        const FVector Scale(
            DesiredSizeCm.X / NativeSize.X,
            DesiredSizeCm.Y / NativeSize.Y,
            DesiredSizeCm.Z / NativeSize.Z);
        const FQuat Quat = Rotation.Quaternion();
        const FVector Location = Center - Quat.RotateVector(Bounds.Origin * Scale);
        return Component->AddInstance(FTransform(Quat, Location, Scale), false) != INDEX_NONE;
    }

    void AddAuthoredFrame(UInstancedStaticMeshComponent* Frame, const FVector& Center,
        const FVector& HorizontalDirection, const float WidthCm, const float HeightCm)
    {
        if (!Frame || !Frame->GetStaticMesh()) return;
        const FBoxSphereBounds Bounds = Frame->GetStaticMesh()->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        const float NativeLengths[3] = { NativeSize.X, NativeSize.Y, NativeSize.Z };
        int32 NativeLongestAxis = 0;
        for (int32 Axis = 1; Axis < 3; ++Axis)
        {
            if (NativeLengths[Axis] > NativeLengths[NativeLongestAxis]) NativeLongestAxis = Axis;
        }
        if (NativeLengths[NativeLongestAxis] <= 1.0f) return;

        const FVector UnitAxes[3] = { FVector::ForwardVector, FVector::RightVector, FVector::UpVector };
        auto AddSegment = [&](const FVector& SegmentCenter, const FVector& TargetDirection, const float LengthCm)
        {
            const FQuat Rotation = FQuat::FindBetweenNormals(UnitAxes[NativeLongestAxis], TargetDirection.GetSafeNormal());
            const float UniformScale = LengthCm / NativeLengths[NativeLongestAxis];
            const FVector Location = SegmentCenter - Rotation.RotateVector(Bounds.Origin * UniformScale);
            Frame->AddInstance(FTransform(Rotation, Location, FVector(UniformScale)), false);
        };

        const FVector Horizontal = HorizontalDirection.GetSafeNormal();
        const FVector Vertical = FVector::UpVector;
        AddSegment(Center + Vertical * HeightCm * 0.5f, Horizontal, WidthCm);
        AddSegment(Center - Vertical * HeightCm * 0.5f, Horizontal, WidthCm);
        AddSegment(Center - Horizontal * WidthCm * 0.5f, Vertical, HeightCm);
        AddSegment(Center + Horizontal * WidthCm * 0.5f, Vertical, HeightCm);
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
        if (Existing->ActorHasTag(TEXT("R13_CultureHousePhotoModel"))) Existing->Destroy();
    }

    const TCHAR* ModularRoot = TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/");
    UStaticMesh* Wall8 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_8m.Wall_8m"));
    UStaticMesh* WindowWall4 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Window_4m.Wall_Window_4m"));
    UStaticMesh* DoorWindowWall8 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Door_Windows_8m.Wall_Door_Windows_8m"));
    UStaticMesh* WallTop4 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Top_4m.Wall_Top_4m"));
    UStaticMesh* Pillar = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Wall_Pillar.Wall_Pillar"));
    UStaticMesh* Door = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Door_01.Door_01"));
    UStaticMesh* RoofTile = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Porch_Roof_8x4m.Porch_Roof_8x4m"));
    UStaticMesh* BottomExtender = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Bottom_Extender_4m.Bottom_Extender_4m"));
    UStaticMesh* Porch = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Porch_4x4m.Porch_4x4m"));
    UStaticMesh* WindowFramePart = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Window_Frame_Part.Window_Frame_Part"));

    if (!Wall8 || !WindowWall4 || !DoorWindowWall8 || !WallTop4 || !Pillar || !Door ||
        !RoofTile || !BottomExtender || !Porch || !WindowFramePart)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_CULTURE_HOUSE_AUTHORED_SHELL_FAIL reason=missing_committed_modular_asset modular_root=%s basicshape_fallback=0 runtime_acceptance=0"),
            ModularRoot);
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = MakeUniqueObjectName(&World, AActor::StaticClass(), TEXT("R146_OsterCultureHouse"));
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Model = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
    if (!Model) return;

    Model->SetReplicates(false);
    Model->SetActorEnableCollision(true);
    Model->Tags.Add(TEXT("R146_CultureHouseAuthoritative"));
    Model->Tags.Add(TEXT("R146_CultureHouseModel"));
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

    UInstancedStaticMeshComponent* FrontShell = MakeAuthoredISM(Model, Root, DoorWindowWall8,
        TEXT("R146Culture_AuthoredFrontShell"), true);
    UInstancedStaticMeshComponent* WindowShell = MakeAuthoredISM(Model, Root, WindowWall4,
        TEXT("R146Culture_AuthoredWindowShell"), true);
    UInstancedStaticMeshComponent* RearShell = MakeAuthoredISM(Model, Root, Wall8,
        TEXT("R146Culture_AuthoredRearShell"), true);
    UInstancedStaticMeshComponent* TopTrim = MakeAuthoredISM(Model, Root, WallTop4,
        TEXT("R146Culture_AuthoredTopTrim"), false);
    UInstancedStaticMeshComponent* Columns = MakeAuthoredISM(Model, Root, Pillar,
        TEXT("R146Culture_Columns"), false);
    UInstancedStaticMeshComponent* Doors = MakeAuthoredISM(Model, Root, Door,
        TEXT("R146Culture_AuthoredDoors"), false);
    UInstancedStaticMeshComponent* Roof = MakeAuthoredISM(Model, Root, RoofTile,
        TEXT("R146Culture_AuthoredRoof"), false);
    UInstancedStaticMeshComponent* Foundation = MakeAuthoredISM(Model, Root, BottomExtender,
        TEXT("R146Culture_AuthoredFoundation"), true);
    UInstancedStaticMeshComponent* Ground = MakeAuthoredISM(Model, Root, Porch,
        TEXT("R146Culture_AuthoredForecourt"), true, false);
    UInstancedStaticMeshComponent* Frames = MakeAuthoredISM(Model, Root, WindowFramePart,
        TEXT("R146Culture_AuthoredWindowFrames"), false);

    if (!FrontShell || !WindowShell || !RearShell || !TopTrim || !Columns || !Doors ||
        !Roof || !Foundation || !Ground || !Frames)
    {
        Model->Destroy();
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_CULTURE_HOUSE_AUTHORED_SHELL_FAIL reason=component_creation basicshape_fallback=0 runtime_acceptance=0"));
        return;
    }

    // Main 32 m x 18.5 m hall. The old one-piece Cube is replaced by authored modular wall families.
    const float FacadeXs[] = { -1200.0f, -400.0f, 400.0f, 1200.0f };
    for (const float X : FacadeXs)
    {
        AddFittedAuthoredMesh(FrontShell, FVector(X, -925.0f, 365.0f), FVector(800.0f, 52.0f, 730.0f));
        AddFittedAuthoredMesh(RearShell, FVector(X, 925.0f, 365.0f), FVector(800.0f, 52.0f, 730.0f),
            FRotator(0.0f, 180.0f, 0.0f));
        AddFittedAuthoredMesh(TopTrim, FVector(X, -960.0f, 720.0f), FVector(800.0f, 70.0f, 120.0f));
    }

    const float SideYs[] = { -600.0f, 0.0f, 600.0f };
    for (const float Y : SideYs)
    {
        AddFittedAuthoredMesh(WindowShell, FVector(-1600.0f, Y, 365.0f), FVector(600.0f, 52.0f, 730.0f),
            FRotator(0.0f, -90.0f, 0.0f));
        AddFittedAuthoredMesh(WindowShell, FVector(1600.0f, Y, 365.0f), FVector(600.0f, 52.0f, 730.0f),
            FRotator(0.0f, 90.0f, 0.0f));
    }

    // Six-column civic facade, now authored Wall_Pillar geometry instead of Engine cylinders.
    const float ColumnXs[] = { -1130.0f, -680.0f, -230.0f, 230.0f, 680.0f, 1130.0f };
    for (const float X : ColumnXs)
    {
        AddFittedAuthoredMesh(Columns, FVector(X, -1080.0f, 365.0f), FVector(96.0f, 96.0f, 610.0f));
    }

    // Three entrance bays use authored door meshes. The surrounding wall modules already own the facade openings.
    for (const float X : { -650.0f, 0.0f, 650.0f })
    {
        AddFittedAuthoredMesh(Doors, FVector(X, -972.0f, 215.0f), FVector(195.0f, 36.0f, 250.0f));
        AddAuthoredFrame(Frames, FVector(X, -982.0f, 385.0f), FVector::ForwardVector, 220.0f, 360.0f);
    }

    // Tiled authored roof. No giant stretched primitive slab remains over the building.
    const float RoofXs[] = { -1200.0f, -400.0f, 400.0f, 1200.0f };
    const float RoofYs[] = { -700.0f, -300.0f, 100.0f, 500.0f, 900.0f };
    for (const float X : RoofXs)
    {
        for (const float Y : RoofYs)
        {
            AddFittedAuthoredMesh(Roof, FVector(X, Y, 790.0f), FVector(800.0f, 400.0f, 150.0f));
        }
    }

    // Authored base/foundation strips replace the old Cube plinth and steps.
    for (const float X : FacadeXs)
    {
        AddFittedAuthoredMesh(Foundation, FVector(X, -925.0f, 42.0f), FVector(800.0f, 100.0f, 84.0f));
        AddFittedAuthoredMesh(Foundation, FVector(X, 925.0f, 42.0f), FVector(800.0f, 100.0f, 84.0f),
            FRotator(0.0f, 180.0f, 0.0f));
    }
    for (const float Y : SideYs)
    {
        AddFittedAuthoredMesh(Foundation, FVector(-1600.0f, Y, 42.0f), FVector(600.0f, 100.0f, 84.0f),
            FRotator(0.0f, -90.0f, 0.0f));
        AddFittedAuthoredMesh(Foundation, FVector(1600.0f, Y, 42.0f), FVector(600.0f, 100.0f, 84.0f),
            FRotator(0.0f, 90.0f, 0.0f));
    }

    // Modular 4x4 m forecourt tiles. The old single grey Cube path is gone.
    for (const float X : { -200.0f, 200.0f })
    {
        for (const float Y : { -1300.0f, -1700.0f, -2100.0f, -2500.0f })
        {
            AddFittedAuthoredMesh(Ground, FVector(X, Y, 12.0f), FVector(400.0f, 400.0f, 24.0f));
        }
    }

    // Side/rear window-profile detail remains authored and independent from the load-bearing shell.
    for (const float Y : SideYs)
    {
        AddAuthoredFrame(Frames, FVector(-1632.0f, Y, 390.0f), FVector::RightVector, 250.0f, 220.0f);
        AddAuthoredFrame(Frames, FVector(1632.0f, Y, 390.0f), FVector::RightVector, 250.0f, 220.0f);
    }
    for (const float X : { -1050.0f, -525.0f, 0.0f, 525.0f, 1050.0f })
    {
        AddAuthoredFrame(Frames, FVector(X, 952.0f, 390.0f), FVector::ForwardVector, 230.0f, 220.0f);
    }

    const FVector Site = CultureHouseAnchor();
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_CULTURE_HOUSE_AUTHORED_SHELL_READY site=Hranovskoho3 x=%.1f y=%.1f authored_wall=1 authored_pillar=1 authored_door=1 authored_roof=1 authored_forecourt=1 basicshape_visible=0 basicshape_material=0 six_column_facade=1 runtime_visual_acceptance=pending"),
        Site.X, Site.Y);
    UE_LOG(LogTemp, Display,
        TEXT("R14.6 Culture House authoritative owner built at Hranovskoho 3 [%.1f %.1f]; separate site root, authored modular shell, six-column facade and conservative side/rear detail active."),
        Site.X, Site.Y);
}
