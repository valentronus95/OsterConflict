#include "OCR138MuseumInteractiveArchitectureSubsystem.h"

#include "OCBreakableWindow.h"
#include "OCGameMode.h"
#include "OCInteractableDoor.h"
#include "OCWorldSectorOster.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float R138MuseumDelaySeconds = 1.10f;
    constexpr float WallBottomZ = 70.0f;
    constexpr float WallTopZ = 390.0f;
    constexpr float WallThickness = 28.0f;
    constexpr float HalfLengthX = 850.0f;
    constexpr float HalfDepthY = 420.0f;

    const FName CollisionOwnerTag(TEXT("R138_MuseumInteractionCollision"));

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

    UStaticMeshComponent* AddCollisionSection(
        AActor* Owner,
        USceneComponent* Root,
        UStaticMesh* Cube,
        const FString& SectionId,
        const FVector& Center,
        const FVector& SizeCm,
        const bool bCollision = true)
    {
        if (!Owner || !Root || !Cube || SizeCm.GetMin() <= 0.0f) return nullptr;

        const FName BaseName(*FString::Printf(TEXT("R138Collision_%s"), *SectionId));
        UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(
            Owner,
            MakeUniqueObjectName(Owner, UStaticMeshComponent::StaticClass(), BaseName));
        if (!Component) return nullptr;

        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Cube);
        Component->SetRelativeLocation(Center);
        Component->SetRelativeScale3D(SizeCm / 100.0f);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(bCollision);
        Component->SetVisibility(false, true);
        Component->SetHiddenInGame(true, true);
        Component->SetCastShadow(false);
        Component->ComponentTags.Add(TEXT("MuseumInteractionCollision"));
        Component->ComponentTags.Add(FName(*FString::Printf(TEXT("Section:%s"), *SectionId)));
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void BuildWallAlongX(
        AActor* Owner,
        USceneComponent* Root,
        UStaticMesh* Cube,
        const FString& Prefix,
        const FVector& Origin,
        const float Y,
        const float MinX,
        const float MaxX,
        const float BottomZ,
        const float TopZ,
        TArray<FOpeningX> Openings)
    {
        Openings.Sort([](const FOpeningX& A, const FOpeningX& B) { return A.CenterX < B.CenterX; });
        float Cursor = MinX;
        int32 SectionIndex = 0;

        for (const FOpeningX& Opening : Openings)
        {
            const float Left = FMath::Clamp(Opening.CenterX - Opening.Width * 0.5f, MinX, MaxX);
            const float Right = FMath::Clamp(Opening.CenterX + Opening.Width * 0.5f, MinX, MaxX);

            if (Left > Cursor + 1.0f)
            {
                const float Width = Left - Cursor;
                AddCollisionSection(
                    Owner, Root, Cube,
                    FString::Printf(TEXT("%s_Column_%02d"), *Prefix, SectionIndex++),
                    Origin + FVector(Cursor + Width * 0.5f, Y, (BottomZ + TopZ) * 0.5f),
                    FVector(Width, WallThickness, TopZ - BottomZ));
            }

            const float OpeningBottom = FMath::Clamp(Opening.BottomZ, BottomZ, TopZ);
            const float OpeningTop = FMath::Clamp(Opening.TopZ, BottomZ, TopZ);
            if (OpeningBottom > BottomZ + 1.0f)
            {
                AddCollisionSection(
                    Owner, Root, Cube,
                    FString::Printf(TEXT("%s_Sill_%02d"), *Prefix, SectionIndex),
                    Origin + FVector(Opening.CenterX, Y, (BottomZ + OpeningBottom) * 0.5f),
                    FVector(Right - Left, WallThickness, OpeningBottom - BottomZ));
            }
            if (TopZ > OpeningTop + 1.0f)
            {
                AddCollisionSection(
                    Owner, Root, Cube,
                    FString::Printf(TEXT("%s_Header_%02d"), *Prefix, SectionIndex),
                    Origin + FVector(Opening.CenterX, Y, (OpeningTop + TopZ) * 0.5f),
                    FVector(Right - Left, WallThickness, TopZ - OpeningTop));
            }
            Cursor = FMath::Max(Cursor, Right);
        }

        if (MaxX > Cursor + 1.0f)
        {
            const float Width = MaxX - Cursor;
            AddCollisionSection(
                Owner, Root, Cube,
                FString::Printf(TEXT("%s_Column_%02d"), *Prefix, SectionIndex),
                Origin + FVector(Cursor + Width * 0.5f, Y, (BottomZ + TopZ) * 0.5f),
                FVector(Width, WallThickness, TopZ - BottomZ));
        }
    }

    void BuildWallAlongY(
        AActor* Owner,
        USceneComponent* Root,
        UStaticMesh* Cube,
        const FString& Prefix,
        const FVector& Origin,
        const float X,
        const float MinY,
        const float MaxY,
        const float BottomZ,
        const float TopZ,
        TArray<FOpeningY> Openings)
    {
        Openings.Sort([](const FOpeningY& A, const FOpeningY& B) { return A.CenterY < B.CenterY; });
        float Cursor = MinY;
        int32 SectionIndex = 0;

        for (const FOpeningY& Opening : Openings)
        {
            const float Low = FMath::Clamp(Opening.CenterY - Opening.Width * 0.5f, MinY, MaxY);
            const float High = FMath::Clamp(Opening.CenterY + Opening.Width * 0.5f, MinY, MaxY);

            if (Low > Cursor + 1.0f)
            {
                const float Width = Low - Cursor;
                AddCollisionSection(
                    Owner, Root, Cube,
                    FString::Printf(TEXT("%s_Column_%02d"), *Prefix, SectionIndex++),
                    Origin + FVector(X, Cursor + Width * 0.5f, (BottomZ + TopZ) * 0.5f),
                    FVector(WallThickness, Width, TopZ - BottomZ));
            }

            const float OpeningBottom = FMath::Clamp(Opening.BottomZ, BottomZ, TopZ);
            const float OpeningTop = FMath::Clamp(Opening.TopZ, BottomZ, TopZ);
            if (OpeningBottom > BottomZ + 1.0f)
            {
                AddCollisionSection(
                    Owner, Root, Cube,
                    FString::Printf(TEXT("%s_Sill_%02d"), *Prefix, SectionIndex),
                    Origin + FVector(X, Opening.CenterY, (BottomZ + OpeningBottom) * 0.5f),
                    FVector(WallThickness, High - Low, OpeningBottom - BottomZ));
            }
            if (TopZ > OpeningTop + 1.0f)
            {
                AddCollisionSection(
                    Owner, Root, Cube,
                    FString::Printf(TEXT("%s_Header_%02d"), *Prefix, SectionIndex),
                    Origin + FVector(X, Opening.CenterY, (OpeningTop + TopZ) * 0.5f),
                    FVector(WallThickness, High - Low, TopZ - OpeningTop));
            }
            Cursor = FMath::Max(Cursor, High);
        }

        if (MaxY > Cursor + 1.0f)
        {
            const float Width = MaxY - Cursor;
            AddCollisionSection(
                Owner, Root, Cube,
                FString::Printf(TEXT("%s_Column_%02d"), *Prefix, SectionIndex),
                Origin + FVector(X, Cursor + Width * 0.5f, (BottomZ + TopZ) * 0.5f),
                FVector(WallThickness, Width, TopZ - BottomZ));
        }
    }

    void SpawnMuseumWindow(
        UWorld& World,
        const FVector& Location,
        const float Yaw,
        const float WidthCm,
        const float HeightCm,
        AActor* Owner)
    {
        FActorSpawnParameters Params;
        Params.Owner = Owner;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AOCBreakableWindow* Window = World.SpawnActor<AOCBreakableWindow>(
            AOCBreakableWindow::StaticClass(),
            FTransform(FRotator(0.0f, Yaw, 0.0f), Location),
            Params);
        if (!Window) return;

        Window->SetActorScale3D(FVector(WidthCm / 200.0f, 1.0f, HeightCm / 155.0f));
        Window->Tags.Add(TEXT("R138_MuseumInteractive"));
        Window->Tags.Add(TEXT("MuseumBreakableGlass"));
    }

    void SpawnMuseumDoor(
        UWorld& World,
        const FVector& HingeLocation,
        const float Yaw,
        const FVector& Scale,
        AActor* Owner,
        const TCHAR* RoleTag)
    {
        FActorSpawnParameters Params;
        Params.Owner = Owner;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AOCInteractableDoor* Door = World.SpawnActor<AOCInteractableDoor>(
            AOCInteractableDoor::StaticClass(),
            FTransform(FRotator(0.0f, Yaw, 0.0f), HingeLocation),
            Params);
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
    InWorld.GetTimerManager().SetTimer(
        Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) UpgradeMuseum(*World);
        }),
        R138MuseumDelaySeconds,
        false);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_MUSEUM_R138_INTERACTION_SCHEDULED delay=%.2f visible_shell_owner=R137"),
        R138MuseumDelaySeconds);
}

AActor* UOCR138MuseumInteractiveArchitectureSubsystem::FindR137MuseumActor(UWorld& World) const
{
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor && !Actor->IsActorBeingDestroyed() && Actor->ActorHasTag(TEXT("R137_MuseumPhotoModel")))
        {
            return Actor;
        }
    }
    return nullptr;
}

void UOCR138MuseumInteractiveArchitectureSubsystem::ReleaseR137StructuralCollision(AActor& MuseumActor) const
{
    static const TSet<FName> CollisionTransferredToR138 =
    {
        TEXT("R137Museum_Plinth"),
        TEXT("R137Museum_BrickBody"),
        TEXT("R137Museum_BlueGreyTimber"),
        TEXT("R137Museum_GreyDoors"),
        TEXT("R137Museum_StepsAndSlabs"),
        TEXT("R137Museum_RearAnnex")
    };

    int32 Released = 0;
    TInlineComponentArray<UPrimitiveComponent*> Components;
    MuseumActor.GetComponents(Components);
    for (UPrimitiveComponent* Component : Components)
    {
        if (!Component || !CollisionTransferredToR138.Contains(Component->GetFName())) continue;
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetCanEverAffectNavigation(false);
        ++Released;
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_MUSEUM_R137_VISIBLE_OWNER_PRESERVED released_collision_components=%d visibility_mutation=0 material_mutation=0"),
        Released);
}

void UOCR138MuseumInteractiveArchitectureSubsystem::BuildInteractionCollisionArchitecture(UWorld& World) const
{
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Existing = *It;
        if (Existing && !Existing->IsActorBeingDestroyed() && Existing->ActorHasTag(CollisionOwnerTag))
        {
            UE_LOG(LogTemp, Display,
                TEXT("PASS45_MUSEUM_R138_COLLISION_ONLY_READY collision_owner=1 visible_components=0 duplicate_build=0"));
            return;
        }
    }

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!Cube)
    {
        UE_LOG(LogTemp, Error, TEXT("PASS45_MUSEUM_R138_COLLISION_ONLY_FAIL reason=cube_missing"));
        return;
    }

    AActor* CollisionOwner = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!CollisionOwner)
    {
        UE_LOG(LogTemp, Error, TEXT("PASS45_MUSEUM_R138_COLLISION_ONLY_FAIL reason=owner_spawn_failed"));
        return;
    }

    CollisionOwner->SetReplicates(false);
    CollisionOwner->Tags.Add(CollisionOwnerTag);

    USceneComponent* Root = NewObject<USceneComponent>(
        CollisionOwner,
        MakeUniqueObjectName(CollisionOwner, USceneComponent::StaticClass(), FName(TEXT("R138MuseumCollisionRoot"))));
    if (!Root)
    {
        CollisionOwner->Destroy();
        UE_LOG(LogTemp, Error, TEXT("PASS45_MUSEUM_R138_COLLISION_ONLY_FAIL reason=root_create_failed"));
        return;
    }

    Root->SetMobility(EComponentMobility::Static);
    CollisionOwner->SetRootComponent(Root);
    CollisionOwner->AddInstanceComponent(Root);
    Root->RegisterComponent();

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();

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

    BuildWallAlongX(CollisionOwner, Root, Cube, TEXT("FrontWall"), Museum,
        -HalfDepthY, -HalfLengthX, HalfLengthX, WallBottomZ, WallTopZ, FrontOpenings);
    BuildWallAlongX(CollisionOwner, Root, Cube, TEXT("RearWall"), Museum,
         HalfDepthY, -HalfLengthX, HalfLengthX, WallBottomZ, WallTopZ, RearOpenings);

    BuildWallAlongY(CollisionOwner, Root, Cube, TEXT("RightGableWall"), Museum,
        HalfLengthX, -HalfDepthY, HalfDepthY, WallBottomZ, WallTopZ,
        {
            { -215.0f, 140.0f, 132.5f, 337.5f },
            {  190.0f, 150.0f,  70.0f, 345.0f }
        });
    BuildWallAlongY(CollisionOwner, Root, Cube, TEXT("LeftWall"), Museum,
        -HalfLengthX, -HalfDepthY, HalfDepthY, WallBottomZ, WallTopZ,
        { { 120.0f, 190.0f, 70.0f, 330.0f } });

    AddCollisionSection(CollisionOwner, Root, Cube, TEXT("GroundFloor"),
        Museum + FVector(0.0f, 0.0f, 76.0f), FVector(1660.0f, 800.0f, 12.0f));

    constexpr float UpperBottom = 390.0f;
    constexpr float UpperTop = 650.0f;
    constexpr float UpperHalfX = 285.0f;
    constexpr float UpperFrontY = -270.0f;
    constexpr float UpperRearY = 200.0f;

    BuildWallAlongX(CollisionOwner, Root, Cube, TEXT("UpperFront"), Museum,
        UpperFrontY, -UpperHalfX, UpperHalfX, UpperBottom, UpperTop,
        {
            { -190.0f, 105.0f, 437.5f, 602.5f },
            {    0.0f, 105.0f, 437.5f, 602.5f },
            {  190.0f, 105.0f, 437.5f, 602.5f }
        });
    BuildWallAlongX(CollisionOwner, Root, Cube, TEXT("UpperRear"), Museum,
        UpperRearY, -UpperHalfX, UpperHalfX, UpperBottom, UpperTop, {});
    BuildWallAlongY(CollisionOwner, Root, Cube, TEXT("UpperRight"), Museum,
        UpperHalfX, UpperFrontY, UpperRearY, UpperBottom, UpperTop,
        {
            { -115.0f, 105.0f, 437.5f, 602.5f },
            {  105.0f, 105.0f, 437.5f, 602.5f }
        });
    BuildWallAlongY(CollisionOwner, Root, Cube, TEXT("UpperLeft"), Museum,
        -UpperHalfX, UpperFrontY, UpperRearY, UpperBottom, UpperTop, {});
    AddCollisionSection(CollisionOwner, Root, Cube, TEXT("UpperFloor"),
        Museum + FVector(0.0f, -35.0f, 397.0f), FVector(540.0f, 440.0f, 14.0f));

    constexpr float VestibuleHalfX = 260.0f;
    constexpr float VestibuleFrontY = -670.0f;
    constexpr float VestibuleRearY = -420.0f;
    BuildWallAlongX(CollisionOwner, Root, Cube, TEXT("VestibuleFront"), Museum,
        VestibuleFrontY, -VestibuleHalfX, VestibuleHalfX, WallBottomZ, WallTopZ,
        {
            { -190.0f, 120.0f, 125.0f, 335.0f },
            {    0.0f, 220.0f,  70.0f, 340.0f },
            {  190.0f, 120.0f, 125.0f, 335.0f }
        });
    BuildWallAlongY(CollisionOwner, Root, Cube, TEXT("VestibuleLeft"), Museum,
        -VestibuleHalfX, VestibuleFrontY, VestibuleRearY, WallBottomZ, WallTopZ,
        { { -545.0f, 150.0f, 125.0f, 335.0f } });
    BuildWallAlongY(CollisionOwner, Root, Cube, TEXT("VestibuleRight"), Museum,
         VestibuleHalfX, VestibuleFrontY, VestibuleRearY, WallBottomZ, WallTopZ,
        { { -545.0f, 150.0f, 125.0f, 335.0f } });
    AddCollisionSection(CollisionOwner, Root, Cube, TEXT("VestibuleFloor"),
        Museum + FVector(0.0f, -545.0f, 76.0f), FVector(500.0f, 230.0f, 12.0f));

    constexpr float VerandaOuterX = -1105.0f;
    constexpr float VerandaInnerX = -850.0f;
    AddCollisionSection(CollisionOwner, Root, Cube, TEXT("VerandaLowerOuter"),
        Museum + FVector(VerandaOuterX, 125.0f, 115.0f), FVector(24.0f, 550.0f, 90.0f));
    AddCollisionSection(CollisionOwner, Root, Cube, TEXT("VerandaFloor"),
        Museum + FVector((VerandaOuterX + VerandaInnerX) * 0.5f, 125.0f, 76.0f),
        FVector(255.0f, 550.0f, 12.0f));

    AddCollisionSection(CollisionOwner, Root, Cube, TEXT("RearAnnex"),
        Museum + FVector(1020.0f, 235.0f, 160.0f), FVector(430.0f, 470.0f, 250.0f));

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_MUSEUM_R138_COLLISION_ONLY_READY collision_owner=1 visible_components=0 authored_material_writes=0 visible_shell_duplication=0"));
}

void UOCR138MuseumInteractiveArchitectureSubsystem::SpawnInteractiveOpenings(UWorld& World) const
{
    if (World.GetNetMode() == NM_Client) return;

    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor && Actor->ActorHasTag(TEXT("R138_MuseumInteractive")) &&
            FVector::DistSquared2D(Actor->GetActorLocation(), Museum) < FMath::Square(2500.0f))
        {
            UE_LOG(LogTemp, Display,
                TEXT("PASS45_MUSEUM_INTERACTIVE_OPENINGS_READY duplicate_spawn=0 existing=1"));
            return;
        }
    }

    AActor* Owner = FindR137MuseumActor(World);

    SpawnMuseumDoor(World, Museum + FVector(-108.0f, -678.0f, 70.0f), 0.0f,
        FVector(0.83f, 1.0f, 1.15f), Owner, TEXT("MuseumMainDoorLeft"));
    SpawnMuseumDoor(World, Museum + FVector(108.0f, -678.0f, 70.0f), 180.0f,
        FVector(0.83f, 1.0f, 1.15f), Owner, TEXT("MuseumMainDoorRight"));
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

    SpawnMuseumWindow(World, Museum + FVector(-190.0f, -678.0f, 230.0f), 0.0f, 120.0f, 210.0f, Owner);
    SpawnMuseumWindow(World, Museum + FVector(190.0f, -678.0f, 230.0f), 0.0f, 120.0f, 210.0f, Owner);
    SpawnMuseumWindow(World, Museum + FVector(-268.0f, -545.0f, 230.0f), -90.0f, 150.0f, 210.0f, Owner);
    SpawnMuseumWindow(World, Museum + FVector(268.0f, -545.0f, 230.0f), 90.0f, 150.0f, 210.0f, Owner);

    for (const float Y : { -80.0f, 60.0f, 200.0f, 340.0f })
        SpawnMuseumWindow(World, Museum + FVector(-1117.0f, Y, 235.0f), -90.0f, 115.0f, 195.0f, Owner);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_MUSEUM_INTERACTIVE_OPENINGS_READY duplicate_spawn=0 existing=0"));
}

void UOCR138MuseumInteractiveArchitectureSubsystem::UpgradeMuseum(UWorld& World)
{
    AActor* R137Museum = FindR137MuseumActor(World);
    if (!R137Museum)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_FAIL reason=R137_visible_owner_missing"));
        return;
    }

    ReleaseR137StructuralCollision(*R137Museum);
    BuildInteractionCollisionArchitecture(World);
    SpawnInteractiveOpenings(World);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_MUSEUM_SINGLE_VISIBLE_OWNER_READY visible_owner=R137 interaction_owner=R138 collision_only=1 visibility_mutation=0 material_mutation=0 visible_shell_duplication=0"));
}
