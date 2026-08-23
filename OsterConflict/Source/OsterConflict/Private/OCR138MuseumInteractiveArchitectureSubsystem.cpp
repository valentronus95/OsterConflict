#include "OCR138MuseumInteractiveArchitectureSubsystem.h"

#include "OCBreakableWindow.h"
#include "OCGameMode.h"
#include "OCInteractableDoor.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
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
    constexpr float R138MuseumDelaySeconds = 5.35f;
    constexpr float WallBottomZ = 70.0f;
    constexpr float WallTopZ = 390.0f;
    constexpr float WallThickness = 28.0f;
    constexpr float HalfLengthX = 850.0f;
    constexpr float HalfDepthY = 420.0f;

    struct FOpeningX
    {
        float CenterX;
        float Width;
        float BottomZ;
        float TopZ;
    };

    struct FOpeningY
    {
        float CenterY;
        float Width;
        float BottomZ;
        float TopZ;
    };

    UMaterialInstanceDynamic* MakeMuseumMID(AActor* Owner, UMaterialInterface* Base,
        const TCHAR* SemanticName, const FLinearColor& Color)
    {
        if (!Owner || !Base) return nullptr;
        const FName UniqueName = MakeUniqueObjectName(
            Owner, UMaterialInstanceDynamic::StaticClass(), FName(SemanticName));
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Owner, UniqueName);
        if (Material)
        {
            Material->SetVectorParameterValue(TEXT("Color"), Color);
        }
        return Material;
    }

    UStaticMeshComponent* AddSection(AActor* Owner, USceneComponent* Root, UStaticMesh* Cube,
        UMaterialInterface* Material, const FString& SectionId, const FVector& Center,
        const FVector& SizeCm, const bool bCollision = true, const FRotator& Rotation = FRotator::ZeroRotator)
    {
        if (!Owner || !Root || !Cube || SizeCm.GetMin() <= 0.0f) return nullptr;

        const FName BaseName(*FString::Printf(TEXT("R138Museum_%s"), *SectionId));
        const FName UniqueName = MakeUniqueObjectName(Owner, UStaticMeshComponent::StaticClass(), BaseName);
        UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(Owner, UniqueName);
        if (!Component) return nullptr;

        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Cube);
        if (Material) Component->SetMaterial(0, Material);
        Component->SetRelativeLocation(Center);
        Component->SetRelativeRotation(Rotation);
        Component->SetRelativeScale3D(SizeCm / 100.0f);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(bCollision);
        Component->SetCastShadow(false);
        Component->ComponentTags.Add(TEXT("MuseumStructural"));
        Component->ComponentTags.Add(FName(*FString::Printf(TEXT("Section:%s"), *SectionId)));
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    UInstancedStaticMeshComponent* MakeDetailISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Cube,
        UMaterialInterface* Material, const TCHAR* SemanticName, const bool bCollision = false)
    {
        if (!Owner || !Root || !Cube) return nullptr;
        const FName UniqueName = MakeUniqueObjectName(
            Owner, UInstancedStaticMeshComponent::StaticClass(), FName(SemanticName));
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, UniqueName);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Cube);
        if (Material) Component->SetMaterial(0, Material);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(bCollision);
        Component->SetCastShadow(false);
        Component->SetCullDistances(0, 30000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddDetailBox(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const FVector& SizeCm, const FRotator& Rotation = FRotator::ZeroRotator)
    {
        if (!Component || SizeCm.GetMin() <= 0.0f) return;
        Component->AddInstance(FTransform(Rotation, Center, SizeCm / 100.0f), true);
    }

    void BuildWallAlongX(AActor* Owner, USceneComponent* Root, UStaticMesh* Cube, UMaterialInterface* Material,
        const FString& Prefix, const FVector& Origin, const float Y, const float MinX, const float MaxX,
        const float BottomZ, const float TopZ, TArray<FOpeningX> Openings)
    {
        Openings.Sort([](const FOpeningX& A, const FOpeningX& B) { return A.CenterX < B.CenterX; });
        float Cursor = MinX;
        int32 ColumnIndex = 0;

        for (const FOpeningX& Opening : Openings)
        {
            const float Left = FMath::Clamp(Opening.CenterX - Opening.Width * 0.5f, MinX, MaxX);
            const float Right = FMath::Clamp(Opening.CenterX + Opening.Width * 0.5f, MinX, MaxX);
            if (Left > Cursor + 1.0f)
            {
                const float Width = Left - Cursor;
                AddSection(Owner, Root, Cube, Material,
                    FString::Printf(TEXT("%s_Column_%02d"), *Prefix, ColumnIndex++),
                    Origin + FVector(Cursor + Width * 0.5f, Y, (BottomZ + TopZ) * 0.5f),
                    FVector(Width, WallThickness, TopZ - BottomZ));
            }

            const float OpeningBottom = FMath::Clamp(Opening.BottomZ, BottomZ, TopZ);
            const float OpeningTop = FMath::Clamp(Opening.TopZ, BottomZ, TopZ);
            if (OpeningBottom > BottomZ + 1.0f)
            {
                AddSection(Owner, Root, Cube, Material,
                    FString::Printf(TEXT("%s_Sill_%02d"), *Prefix, ColumnIndex),
                    Origin + FVector(Opening.CenterX, Y, (BottomZ + OpeningBottom) * 0.5f),
                    FVector(Right - Left, WallThickness, OpeningBottom - BottomZ));
            }
            if (TopZ > OpeningTop + 1.0f)
            {
                AddSection(Owner, Root, Cube, Material,
                    FString::Printf(TEXT("%s_Header_%02d"), *Prefix, ColumnIndex),
                    Origin + FVector(Opening.CenterX, Y, (OpeningTop + TopZ) * 0.5f),
                    FVector(Right - Left, WallThickness, TopZ - OpeningTop));
            }
            Cursor = FMath::Max(Cursor, Right);
        }

        if (MaxX > Cursor + 1.0f)
        {
            const float Width = MaxX - Cursor;
            AddSection(Owner, Root, Cube, Material,
                FString::Printf(TEXT("%s_Column_%02d"), *Prefix, ColumnIndex),
                Origin + FVector(Cursor + Width * 0.5f, Y, (BottomZ + TopZ) * 0.5f),
                FVector(Width, WallThickness, TopZ - BottomZ));
        }
    }

    void BuildWallAlongY(AActor* Owner, USceneComponent* Root, UStaticMesh* Cube, UMaterialInterface* Material,
        const FString& Prefix, const FVector& Origin, const float X, const float MinY, const float MaxY,
        const float BottomZ, const float TopZ, TArray<FOpeningY> Openings)
    {
        Openings.Sort([](const FOpeningY& A, const FOpeningY& B) { return A.CenterY < B.CenterY; });
        float Cursor = MinY;
        int32 ColumnIndex = 0;

        for (const FOpeningY& Opening : Openings)
        {
            const float Low = FMath::Clamp(Opening.CenterY - Opening.Width * 0.5f, MinY, MaxY);
            const float High = FMath::Clamp(Opening.CenterY + Opening.Width * 0.5f, MinY, MaxY);
            if (Low > Cursor + 1.0f)
            {
                const float Width = Low - Cursor;
                AddSection(Owner, Root, Cube, Material,
                    FString::Printf(TEXT("%s_Column_%02d"), *Prefix, ColumnIndex++),
                    Origin + FVector(X, Cursor + Width * 0.5f, (BottomZ + TopZ) * 0.5f),
                    FVector(WallThickness, Width, TopZ - BottomZ));
            }

            const float OpeningBottom = FMath::Clamp(Opening.BottomZ, BottomZ, TopZ);
            const float OpeningTop = FMath::Clamp(Opening.TopZ, BottomZ, TopZ);
            if (OpeningBottom > BottomZ + 1.0f)
            {
                AddSection(Owner, Root, Cube, Material,
                    FString::Printf(TEXT("%s_Sill_%02d"), *Prefix, ColumnIndex),
                    Origin + FVector(X, Opening.CenterY, (BottomZ + OpeningBottom) * 0.5f),
                    FVector(WallThickness, High - Low, OpeningBottom - BottomZ));
            }
            if (TopZ > OpeningTop + 1.0f)
            {
                AddSection(Owner, Root, Cube, Material,
                    FString::Printf(TEXT("%s_Header_%02d"), *Prefix, ColumnIndex),
                    Origin + FVector(X, Opening.CenterY, (OpeningTop + TopZ) * 0.5f),
                    FVector(WallThickness, High - Low, TopZ - OpeningTop));
            }
            Cursor = FMath::Max(Cursor, High);
        }

        if (MaxY > Cursor + 1.0f)
        {
            const float Width = MaxY - Cursor;
            AddSection(Owner, Root, Cube, Material,
                FString::Printf(TEXT("%s_Column_%02d"), *Prefix, ColumnIndex),
                Origin + FVector(X, Cursor + Width * 0.5f, (BottomZ + TopZ) * 0.5f),
                FVector(WallThickness, Width, TopZ - BottomZ));
        }
    }

    void AddFrontWindowDecor(UInstancedStaticMeshComponent* Trim, UInstancedStaticMeshComponent* Grille,
        const FVector& Museum, const float X, const float Y, const float Z, const float Width, const float Height)
    {
        constexpr float Frame = 12.0f;
        AddDetailBox(Trim, Museum + FVector(X - Width * 0.5f - Frame * 0.5f, Y, Z),
            FVector(Frame, 12.0f, Height + Frame * 2.0f));
        AddDetailBox(Trim, Museum + FVector(X + Width * 0.5f + Frame * 0.5f, Y, Z),
            FVector(Frame, 12.0f, Height + Frame * 2.0f));
        AddDetailBox(Trim, Museum + FVector(X, Y, Z + Height * 0.5f + Frame * 0.5f),
            FVector(Width + Frame * 2.0f, 12.0f, Frame));
        AddDetailBox(Trim, Museum + FVector(X, Y, Z - Height * 0.5f - Frame * 0.5f),
            FVector(Width + Frame * 2.0f, 12.0f, Frame));
        AddDetailBox(Trim, Museum + FVector(X, Y, Z + 35.0f), FVector(Width, 10.0f, 9.0f));
        AddDetailBox(Trim, Museum + FVector(X, Y, Z - 35.0f), FVector(9.0f, 10.0f, Height - 70.0f));

        const float GrillY = Y - 10.0f;
        AddDetailBox(Grille, Museum + FVector(X, GrillY, Z - 42.0f), FVector(Width - 18.0f, 5.0f, 5.0f), FRotator(0.0f, 0.0f, 48.0f));
        AddDetailBox(Grille, Museum + FVector(X, GrillY, Z - 42.0f), FVector(Width - 18.0f, 5.0f, 5.0f), FRotator(0.0f, 0.0f, -48.0f));
        for (const float Offset : { -0.34f, 0.0f, 0.34f })
        {
            AddDetailBox(Grille, Museum + FVector(X + Offset * Width, GrillY, Z + Height * 0.27f),
                FVector(5.0f, 5.0f, Height * 0.42f), FRotator(0.0f, Offset * 18.0f, 0.0f));
        }
    }

    void AddSideWindowDecor(UInstancedStaticMeshComponent* Trim, UInstancedStaticMeshComponent* Grille,
        const FVector& Museum, const float X, const float Y, const float Z, const float Width, const float Height)
    {
        constexpr float Frame = 12.0f;
        AddDetailBox(Trim, Museum + FVector(X, Y - Width * 0.5f - Frame * 0.5f, Z),
            FVector(12.0f, Frame, Height + Frame * 2.0f));
        AddDetailBox(Trim, Museum + FVector(X, Y + Width * 0.5f + Frame * 0.5f, Z),
            FVector(12.0f, Frame, Height + Frame * 2.0f));
        AddDetailBox(Trim, Museum + FVector(X, Y, Z + Height * 0.5f + Frame * 0.5f),
            FVector(12.0f, Width + Frame * 2.0f, Frame));
        AddDetailBox(Trim, Museum + FVector(X, Y, Z - Height * 0.5f - Frame * 0.5f),
            FVector(12.0f, Width + Frame * 2.0f, Frame));
        AddDetailBox(Trim, Museum + FVector(X, Y, Z + 35.0f), FVector(10.0f, Width, 9.0f));
        AddDetailBox(Trim, Museum + FVector(X, Y, Z - 35.0f), FVector(10.0f, 9.0f, Height - 70.0f));

        const float GrillX = X + 10.0f;
        AddDetailBox(Grille, Museum + FVector(GrillX, Y, Z - 42.0f), FVector(5.0f, Width - 18.0f, 5.0f), FRotator(48.0f, 0.0f, 0.0f));
        AddDetailBox(Grille, Museum + FVector(GrillX, Y, Z - 42.0f), FVector(5.0f, Width - 18.0f, 5.0f), FRotator(-48.0f, 0.0f, 0.0f));
    }

    void SpawnMuseumWindow(UWorld& World, const FVector& Location, const float Yaw,
        const float WidthCm, const float HeightCm, AActor* Owner)
    {
        FActorSpawnParameters Params;
        Params.Owner = Owner;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AOCBreakableWindow* Window = World.SpawnActor<AOCBreakableWindow>(
            AOCBreakableWindow::StaticClass(), FTransform(FRotator(0.0f, Yaw, 0.0f), Location), Params);
        if (!Window) return;
        Window->SetActorScale3D(FVector(WidthCm / 200.0f, 1.0f, HeightCm / 155.0f));
        Window->Tags.Add(TEXT("R138_MuseumInteractive"));
        Window->Tags.Add(TEXT("MuseumBreakableGlass"));
    }

    void SpawnMuseumDoor(UWorld& World, const FVector& HingeLocation, const float Yaw,
        const FVector& Scale, AActor* Owner, const TCHAR* RoleTag)
    {
        FActorSpawnParameters Params;
        Params.Owner = Owner;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AOCInteractableDoor* Door = World.SpawnActor<AOCInteractableDoor>(
            AOCInteractableDoor::StaticClass(), FTransform(FRotator(0.0f, Yaw, 0.0f), HingeLocation), Params);
        if (!Door) return;
        Door->SetActorScale3D(Scale);
        Door->Tags.Add(TEXT("R138_MuseumInteractive"));
        Door->Tags.Add(FName(RoleTag));
    }
}

bool UOCR138MuseumInteractiveArchitectureSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR138MuseumInteractiveArchitectureSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) UpgradeMuseum(*World);
        }), R138MuseumDelaySeconds, false);
}

AActor* UOCR138MuseumInteractiveArchitectureSubsystem::FindR137MuseumActor(UWorld& World) const
{
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        if (AActor* Actor = *It; Actor && Actor->ActorHasTag(TEXT("R137_MuseumPhotoModel")))
        {
            return Actor;
        }
    }
    return nullptr;
}

void UOCR138MuseumInteractiveArchitectureSubsystem::SuppressSolidPrototype(AActor& MuseumActor) const
{
    static const TSet<FName> NamesToSuppress =
    {
        TEXT("R137Museum_BrickBody"),
        TEXT("R137Museum_BlueGreyTimber"),
        TEXT("R137Museum_WindowGlass"),
        TEXT("R137Museum_WindowGrilles"),
        TEXT("R137Museum_CarvedPaleTrim"),
        TEXT("R137Museum_GreyDoors")
    };

    TInlineComponentArray<UActorComponent*> Components;
    MuseumActor.GetComponents(Components);
    int32 Hidden = 0;
    for (UActorComponent* Component : Components)
    {
        UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component);
        if (!Primitive || !NamesToSuppress.Contains(Primitive->GetFName())) continue;
        Primitive->SetVisibility(false, true);
        Primitive->SetHiddenInGame(true, true);
        Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        ++Hidden;
    }

    UE_LOG(LogTemp, Display, TEXT("R13.8 museum: suppressed %d solid/static R13.7 prototype components."), Hidden);
}

void UOCR138MuseumInteractiveArchitectureSubsystem::BuildSegmentedArchitecture(UWorld& World) const
{
    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube || !Basic) return;

    AActor* Architecture = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!Architecture) return;
    Architecture->SetReplicates(false);
    Architecture->Tags.Add(TEXT("R138_MuseumHighFidelityArchitecture"));

    USceneComponent* Root = NewObject<USceneComponent>(Architecture,
        MakeUniqueObjectName(Architecture, USceneComponent::StaticClass(), FName(TEXT("R138MuseumArchitectureRoot"))));
    if (!Root)
    {
        Architecture->Destroy();
        return;
    }
    Architecture->SetRootComponent(Root);
    Architecture->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UMaterialInstanceDynamic* Brick = MakeMuseumMID(Architecture, Basic, TEXT("R138MuseumMID_Brick"),
        FLinearColor(0.49f, 0.205f, 0.095f, 1.0f));
    UMaterialInstanceDynamic* Interior = MakeMuseumMID(Architecture, Basic, TEXT("R138MuseumMID_Interior"),
        FLinearColor(0.66f, 0.63f, 0.56f, 1.0f));
    UMaterialInstanceDynamic* Timber = MakeMuseumMID(Architecture, Basic, TEXT("R138MuseumMID_Timber"),
        FLinearColor(0.31f, 0.37f, 0.39f, 1.0f));
    UMaterialInstanceDynamic* Pale = MakeMuseumMID(Architecture, Basic, TEXT("R138MuseumMID_PaleTrim"),
        FLinearColor(0.82f, 0.78f, 0.66f, 1.0f));
    UMaterialInstanceDynamic* DarkMetal = MakeMuseumMID(Architecture, Basic, TEXT("R138MuseumMID_Grille"),
        FLinearColor(0.24f, 0.27f, 0.27f, 1.0f));
    UMaterialInstanceDynamic* Concrete = MakeMuseumMID(Architecture, Basic, TEXT("R138MuseumMID_Concrete"),
        FLinearColor(0.47f, 0.47f, 0.43f, 1.0f));

    UInstancedStaticMeshComponent* Trim = MakeDetailISM(Architecture, Root, Cube, Pale,
        TEXT("R138Museum_WindowAndGableTrim"));
    UInstancedStaticMeshComponent* Grilles = MakeDetailISM(Architecture, Root, Cube, DarkMetal,
        TEXT("R138Museum_ReferenceGrilles"));

    const TArray<FOpeningX> FrontOpenings =
    {
        { -650.0f, 140.0f, 132.5f, 337.5f },
        { -355.0f, 140.0f, 132.5f, 337.5f },
        {    0.0f, 300.0f,  70.0f, 340.0f },
        {  355.0f, 140.0f, 132.5f, 337.5f },
        {  650.0f, 140.0f, 132.5f, 337.5f }
    };
    const TArray<FOpeningX> RearOpenings =
    {
        { -650.0f, 140.0f, 132.5f, 337.5f },
        { -330.0f, 140.0f, 132.5f, 337.5f },
        {    0.0f, 140.0f, 132.5f, 337.5f },
        {  330.0f, 140.0f, 132.5f, 337.5f },
        {  650.0f, 140.0f, 132.5f, 337.5f }
    };

    BuildWallAlongX(Architecture, Root, Cube, Brick, TEXT("FrontWall"), Museum,
        -HalfDepthY, -HalfLengthX, HalfLengthX, WallBottomZ, WallTopZ, FrontOpenings);
    BuildWallAlongX(Architecture, Root, Cube, Brick, TEXT("RearWall"), Museum,
         HalfDepthY, -HalfLengthX, HalfLengthX, WallBottomZ, WallTopZ, RearOpenings);

    const TArray<FOpeningY> RightOpenings =
    {
        { -215.0f, 140.0f, 132.5f, 337.5f },
        {  190.0f, 150.0f,  70.0f, 345.0f }
    };
    BuildWallAlongY(Architecture, Root, Cube, Brick, TEXT("RightGableWall"), Museum,
        HalfLengthX, -HalfDepthY, HalfDepthY, WallBottomZ, WallTopZ, RightOpenings);

    const TArray<FOpeningY> LeftOpenings =
    {
        { 120.0f, 190.0f, 70.0f, 330.0f }
    };
    BuildWallAlongY(Architecture, Root, Cube, Brick, TEXT("LeftWall"), Museum,
        -HalfLengthX, -HalfDepthY, HalfDepthY, WallBottomZ, WallTopZ, LeftOpenings);

    // Walkable ground floor. The interior plan is deliberately conservative until interior references arrive.
    AddSection(Architecture, Root, Cube, Concrete, TEXT("GroundFloor"),
        Museum + FVector(0.0f, 0.0f, 76.0f), FVector(1660.0f, 800.0f, 12.0f));
    // Pass 30: no speculative interior partitions. Runtime showed these pale slabs as false walls that
    // trapped/obscured the player. Interior room geometry must wait for actual interior references.
    UE_LOG(LogTemp, Display, TEXT("PASS30_MUSEUM_SPECULATIVE_INTERIOR_REMOVED"));

    // Central blue-grey upper room. It is now a hollow shell rather than a window painted onto a solid box.
    constexpr float UpperBottom = 390.0f;
    constexpr float UpperTop = 650.0f;
    constexpr float UpperHalfX = 285.0f;
    constexpr float UpperFrontY = -270.0f;
    constexpr float UpperRearY = 200.0f;
    const TArray<FOpeningX> UpperFrontOpenings =
    {
        { -190.0f, 105.0f, 437.5f, 602.5f },
        {    0.0f, 105.0f, 437.5f, 602.5f },
        {  190.0f, 105.0f, 437.5f, 602.5f }
    };
    BuildWallAlongX(Architecture, Root, Cube, Timber, TEXT("UpperFront"), Museum,
        UpperFrontY, -UpperHalfX, UpperHalfX, UpperBottom, UpperTop, UpperFrontOpenings);
    BuildWallAlongX(Architecture, Root, Cube, Timber, TEXT("UpperRear"), Museum,
        UpperRearY, -UpperHalfX, UpperHalfX, UpperBottom, UpperTop, {});
    const TArray<FOpeningY> UpperSideOpenings =
    {
        { -115.0f, 105.0f, 437.5f, 602.5f },
        {  105.0f, 105.0f, 437.5f, 602.5f }
    };
    BuildWallAlongY(Architecture, Root, Cube, Timber, TEXT("UpperRight"), Museum,
        UpperHalfX, UpperFrontY, UpperRearY, UpperBottom, UpperTop, UpperSideOpenings);
    BuildWallAlongY(Architecture, Root, Cube, Timber, TEXT("UpperLeft"), Museum,
        -UpperHalfX, UpperFrontY, UpperRearY, UpperBottom, UpperTop, {});
    AddSection(Architecture, Root, Cube, Interior, TEXT("UpperFloor"),
        Museum + FVector(0.0f, -35.0f, 397.0f), FVector(540.0f, 440.0f, 14.0f));

    // Photo-matched glazed entrance vestibule. Door and side glass are gameplay actors, frame is static.
    constexpr float VestibuleHalfX = 260.0f;
    constexpr float VestibuleFrontY = -670.0f;
    constexpr float VestibuleRearY = -420.0f;
    const TArray<FOpeningX> VestibuleFrontOpenings =
    {
        { -190.0f, 120.0f, 125.0f, 335.0f },
        {    0.0f, 220.0f,  70.0f, 340.0f },
        {  190.0f, 120.0f, 125.0f, 335.0f }
    };
    BuildWallAlongX(Architecture, Root, Cube, Timber, TEXT("VestibuleFront"), Museum,
        VestibuleFrontY, -VestibuleHalfX, VestibuleHalfX, WallBottomZ, WallTopZ, VestibuleFrontOpenings);
    BuildWallAlongY(Architecture, Root, Cube, Timber, TEXT("VestibuleLeft"), Museum,
        -VestibuleHalfX, VestibuleFrontY, VestibuleRearY, WallBottomZ, WallTopZ,
        { { -545.0f, 150.0f, 125.0f, 335.0f } });
    BuildWallAlongY(Architecture, Root, Cube, Timber, TEXT("VestibuleRight"), Museum,
         VestibuleHalfX, VestibuleFrontY, VestibuleRearY, WallBottomZ, WallTopZ,
        { { -545.0f, 150.0f, 125.0f, 335.0f } });
    AddSection(Architecture, Root, Cube, Concrete, TEXT("VestibuleFloor"),
        Museum + FVector(0.0f, -545.0f, 76.0f), FVector(500.0f, 230.0f, 12.0f));

    // Glazed side veranda visible in the supplied side/front reference set.
    constexpr float VerandaOuterX = -1105.0f;
    constexpr float VerandaInnerX = -850.0f;
    AddSection(Architecture, Root, Cube, Brick, TEXT("VerandaLowerOuter"),
        Museum + FVector(VerandaOuterX, 125.0f, 115.0f), FVector(24.0f, 550.0f, 90.0f));
    AddSection(Architecture, Root, Cube, Concrete, TEXT("VerandaFloor"),
        Museum + FVector((VerandaOuterX + VerandaInnerX) * 0.5f, 125.0f, 76.0f),
        FVector(255.0f, 550.0f, 12.0f));
    for (const float Y : { -80.0f, 60.0f, 200.0f, 340.0f })
    {
        AddSection(Architecture, Root, Cube, Timber,
            FString::Printf(TEXT("VerandaPost_%d"), FMath::RoundToInt(Y + 100.0f)),
            Museum + FVector(VerandaOuterX, Y - 70.0f, 260.0f), FVector(24.0f, 18.0f, 290.0f), false);
    }

    // Rebuild window trim/grilles from the current twenty-photo set rather than retaining the older generic grid.
    for (const float X : { -650.0f, -355.0f, 355.0f, 650.0f })
        AddFrontWindowDecor(Trim, Grilles, Museum, X, -434.0f, 235.0f, 140.0f, 205.0f);
    for (const float X : { -650.0f, -330.0f, 0.0f, 330.0f, 650.0f })
        AddFrontWindowDecor(Trim, Grilles, Museum, X, 434.0f, 235.0f, 140.0f, 205.0f);
    AddSideWindowDecor(Trim, Grilles, Museum, 864.0f, -215.0f, 235.0f, 140.0f, 205.0f);
    for (const float X : { -190.0f, 0.0f, 190.0f })
        AddFrontWindowDecor(Trim, Grilles, Museum, X, -278.0f, 520.0f, 105.0f, 165.0f);
    for (const float Y : { -115.0f, 105.0f })
        AddSideWindowDecor(Trim, Grilles, Museum, 293.0f, Y, 520.0f, 105.0f, 165.0f);

    UE_LOG(LogTemp, Display,
        TEXT("R13.8 museum architecture: segmented enterable shell built at MuseumAnchor; structural components carry stable Section:* tags for future RPG/Chaos damage."));
}

void UOCR138MuseumInteractiveArchitectureSubsystem::SpawnInteractiveOpenings(UWorld& World) const
{
    if (World.GetNetMode() == NM_Client) return;

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        if (AActor* Actor = *It; Actor && Actor->ActorHasTag(TEXT("R138_MuseumInteractive")) &&
            FVector::DistSquared2D(Actor->GetActorLocation(), Museum) < FMath::Square(2500.0f))
        {
            return;
        }
    }

    AActor* Owner = FindR137MuseumActor(World);

    // The photographed main entrance is a true two-leaf door. Mirrored actor yaw gives opposite hinge motion.
    SpawnMuseumDoor(World, Museum + FVector(-108.0f, -678.0f, 70.0f), 0.0f,
        FVector(0.83f, 1.0f, 1.15f), Owner, TEXT("MuseumMainDoorLeft"));
    SpawnMuseumDoor(World, Museum + FVector(108.0f, -678.0f, 70.0f), 180.0f,
        FVector(0.83f, 1.0f, 1.15f), Owner, TEXT("MuseumMainDoorRight"));

    // Tall side/service door on the gable wall.
    SpawnMuseumDoor(World, Museum + FVector(858.0f, 115.0f, 70.0f), 90.0f,
        FVector(1.10f, 1.0f, 1.17f), Owner, TEXT("MuseumServiceDoor"));

    for (const float X : { -650.0f, -355.0f, 355.0f, 650.0f })
        SpawnMuseumWindow(World, Museum + FVector(X, -434.0f, 235.0f), 0.0f, 140.0f, 205.0f, Owner);
    for (const float X : { -650.0f, -330.0f, 0.0f, 330.0f, 650.0f })
        SpawnMuseumWindow(World, Museum + FVector(X, 434.0f, 235.0f), 180.0f, 140.0f, 205.0f, Owner);
    SpawnMuseumWindow(World, Museum + FVector(864.0f, -215.0f, 235.0f), 90.0f, 140.0f, 205.0f, Owner);

    for (const float X : { -190.0f, 0.0f, 190.0f })
        SpawnMuseumWindow(World, Museum + FVector(X, -278.0f, 520.0f), 0.0f, 105.0f, 165.0f, Owner);
    for (const float Y : { -115.0f, 105.0f })
        SpawnMuseumWindow(World, Museum + FVector(293.0f, Y, 520.0f), 90.0f, 105.0f, 165.0f, Owner);

    // Glazed entrance side lights.
    SpawnMuseumWindow(World, Museum + FVector(-190.0f, -678.0f, 230.0f), 0.0f, 120.0f, 210.0f, Owner);
    SpawnMuseumWindow(World, Museum + FVector(190.0f, -678.0f, 230.0f), 0.0f, 120.0f, 210.0f, Owner);
    SpawnMuseumWindow(World, Museum + FVector(-268.0f, -545.0f, 230.0f), -90.0f, 150.0f, 210.0f, Owner);
    SpawnMuseumWindow(World, Museum + FVector(268.0f, -545.0f, 230.0f), 90.0f, 150.0f, 210.0f, Owner);

    // Four glass bays on the photographed side veranda.
    for (const float Y : { -80.0f, 60.0f, 200.0f, 340.0f })
        SpawnMuseumWindow(World, Museum + FVector(-1117.0f, Y, 235.0f), -90.0f, 115.0f, 195.0f, Owner);

    UE_LOG(LogTemp, Display,
        TEXT("R13.8 museum interaction: replicated two-leaf main door, service door and per-window breakable glass spawned."));
}

void UOCR138MuseumInteractiveArchitectureSubsystem::UpgradeMuseum(UWorld& World)
{
    AActor* R137Museum = FindR137MuseumActor(World);
    if (!R137Museum)
    {
        UE_LOG(LogTemp, Warning, TEXT("R13.8 museum architecture skipped: R13.7 museum actor was not found."));
        return;
    }

    SuppressSolidPrototype(*R137Museum);
    BuildSegmentedArchitecture(World);
    SpawnInteractiveOpenings(World);
}
