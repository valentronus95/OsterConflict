#include "OCR13OsterResidentialArchitectureSubsystem.h"

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

namespace
{
    constexpr float ArchitectureDelaySeconds = 1.95f;
    constexpr int32 MaxResidentialArchitectureHouses = 96;

    struct FHouseSeed
    {
        FVector Location = FVector::ZeroVector;
        float Yaw = 0.0f;
        FVector SourceFootprintCm = FVector(1000.0f, 750.0f, 340.0f);
        uint32 StableIndex = 0;
    };

    bool IsGenericHouseComponent(const FName Name)
    {
        return Name == TEXT("R13_House01") || Name == TEXT("R13_House02") ||
            Name == TEXT("R12_House01") || Name == TEXT("R12_House02");
    }

    bool IsLegacyHouseExtraComponent(const FName Name)
    {
        const FString Value = Name.ToString();
        return Value.StartsWith(TEXT("R13_House01Extra")) || Value.StartsWith(TEXT("R13_House02Extra"));
    }

    bool IsReservedLandmarkArea(const FVector& Location)
    {
        struct FReservedArea
        {
            FVector Center;
            float RadiusCm;
        };
        const FReservedArea Reserved[] = {
            { AOCWorldSectorOster::MuseumAnchor(), 10800.0f },
            { AOCWorldSectorOster::ParkAnchor(), 13200.0f },
            { AOCWorldSectorOster::CollegeAnchor(), 10800.0f },
            { AOCWorldSectorOster::StadiumAnchor(), 12200.0f },
        };
        for (const FReservedArea& Area : Reserved)
        {
            if (FVector::DistSquared2D(Location, Area.Center) <= FMath::Square(Area.RadiusCm)) return true;
        }
        return false;
    }

    UMaterialInstanceDynamic* MakeColorMaterial(AActor* Owner, UMaterialInterface* Base,
        const FName Name, const FLinearColor& Color)
    {
        if (!Owner || !Base) return nullptr;
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Owner, Name);
        if (Material) Material->SetVectorParameterValue(TEXT("Color"), Color);
        return Material;
    }

    UInstancedStaticMeshComponent* MakeVisualISM(AActor* Owner, USceneComponent* Root,
        UStaticMesh* Mesh, UMaterialInterface* Material, const FName Name, const bool bCastShadow,
        const int32 CullEndCm = 105000)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material)
        {
            const int32 Slots = FMath::Max(1, Mesh->GetStaticMaterials().Num());
            for (int32 SlotIndex = 0; SlotIndex < Slots; ++SlotIndex) Component->SetMaterial(SlotIndex, Material);
        }
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(bCastShadow);
        Component->SetCullDistances(0, CullEndCm);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void GatherAndHideGenericHouses(UWorld& World, TArray<FHouseSeed>& OutSeeds)
    {
        uint32 StableIndex = 0;
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (!Actor) continue;

            TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
            Actor->GetComponents(Components);
            for (UInstancedStaticMeshComponent* Component : Components)
            {
                if (!Component || !IsGenericHouseComponent(Component->GetFName())) continue;
                UStaticMesh* Mesh = Component->GetStaticMesh();
                if (!Mesh) continue;

                const FVector NativeSize = Mesh->GetBounds().BoxExtent * 2.0f;
                for (int32 Index = 0; Index < Component->GetInstanceCount() &&
                    OutSeeds.Num() < MaxResidentialArchitectureHouses; ++Index)
                {
                    FTransform Transform;
                    if (!Component->GetInstanceTransform(Index, Transform, true)) continue;

                    FHouseSeed Seed;
                    Seed.Location = Transform.GetLocation();
                    Seed.Location.Z = 0.0f;
                    Seed.Yaw = Transform.Rotator().Yaw;
                    const FVector InstanceScale = Transform.GetScale3D().GetAbs();
                    Seed.SourceFootprintCm = FVector(
                        NativeSize.X * InstanceScale.X,
                        NativeSize.Y * InstanceScale.Y,
                        NativeSize.Z * InstanceScale.Z);
                    Seed.StableIndex = StableIndex++;
                    if (!IsReservedLandmarkArea(Seed.Location)) OutSeeds.Add(Seed);
                }

                // Keep the generic house mesh only as invisible collision/navigation/placement metadata.
                // Do not disable collision: movement and line traces still rely on these already-tested structures.
                Component->SetVisibility(false, true);
                Component->SetHiddenInGame(true, true);
            }
        }
    }

    void HideLegacyHouseExtras(UWorld& World)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (!Actor) continue;
            TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
            Actor->GetComponents(Components);
            for (UInstancedStaticMeshComponent* Component : Components)
            {
                if (!Component || !IsLegacyHouseExtraComponent(Component->GetFName())) continue;
                Component->SetVisibility(false, true);
                Component->SetHiddenInGame(true, true);
            }
        }
    }

    void AddBox(UInstancedStaticMeshComponent* Target, const FVector& WorldLocation,
        const FVector& SizeCm, const float Yaw)
    {
        if (!Target) return;
        Target->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), WorldLocation, SizeCm / 100.0f), true);
    }

    FVector LocalToWorld(const FVector& HouseLocation, const float HouseYaw, const FVector& LocalOffset)
    {
        return HouseLocation + FRotator(0.0f, HouseYaw, 0.0f).RotateVector(LocalOffset);
    }

    void AddFittedGroundProp(UInstancedStaticMeshComponent* Target, UStaticMesh* Mesh,
        const FVector& HouseLocation, const float HouseYaw, const FVector& LocalOffset,
        const FVector2D& DesiredFootprintCm, const float DesiredHeightCm)
    {
        if (!Target || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) return;

        const FVector Scale(
            FMath::Clamp(DesiredFootprintCm.X / NativeSize.X, 0.18f, 6.0f),
            FMath::Clamp(DesiredFootprintCm.Y / NativeSize.Y, 0.18f, 6.0f),
            FMath::Clamp(DesiredHeightCm / NativeSize.Z, 0.18f, 6.0f));
        FVector Location = LocalToWorld(HouseLocation, HouseYaw, LocalOffset);
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        Location.Z = -LocalBottom * Scale.Z;
        const FVector PivotCorrection = FRotator(0.0f, HouseYaw, 0.0f).RotateVector(
            FVector(-Bounds.Origin.X * Scale.X, -Bounds.Origin.Y * Scale.Y, 0.0f));
        Location += PivotCorrection;
        Target->AddInstance(FTransform(FRotator(0.0f, HouseYaw, 0.0f), Location, Scale), true);
    }

    void AddFittedRoof(UInstancedStaticMeshComponent* Target, UStaticMesh* Mesh,
        const FVector& HouseLocation, const float HouseYaw, const float WidthCm, const float DepthCm,
        const float BodyHeightCm)
    {
        if (!Target || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) return;

        const float ScaleX = FMath::Clamp((WidthCm + 80.0f) / NativeSize.X, 0.18f, 7.0f);
        const float ScaleY = FMath::Clamp((DepthCm + 100.0f) / NativeSize.Y, 0.18f, 7.0f);
        const float ScaleZ = FMath::Clamp(FMath::Min(ScaleX, ScaleY), 0.22f, 4.2f);
        const FVector Scale(ScaleX, ScaleY, ScaleZ);

        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        FVector LocalPivotCorrection(-Bounds.Origin.X * Scale.X, -Bounds.Origin.Y * Scale.Y,
            BodyHeightCm - LocalBottom * Scale.Z);
        const FVector Location = HouseLocation +
            FRotator(0.0f, HouseYaw, 0.0f).RotateVector(LocalPivotCorrection);
        Target->AddInstance(FTransform(FRotator(0.0f, HouseYaw, 0.0f), Location, Scale), true);
    }

    void AddWindow(UInstancedStaticMeshComponent* Trim, UInstancedStaticMeshComponent* Glass,
        const FVector& HouseLocation, const float HouseYaw, const FVector& LocalCenter,
        const float WidthCm, const float HeightCm, const bool bSideWall)
    {
        const float FacingYaw = HouseYaw + (bSideWall ? 90.0f : 0.0f);
        const FVector TrimSize = bSideWall
            ? FVector(10.0f, WidthCm + 18.0f, HeightCm + 18.0f)
            : FVector(WidthCm + 18.0f, 10.0f, HeightCm + 18.0f);
        const FVector GlassSize = bSideWall
            ? FVector(7.0f, WidthCm, HeightCm)
            : FVector(WidthCm, 7.0f, HeightCm);
        AddBox(Trim, LocalToWorld(HouseLocation, HouseYaw, LocalCenter), TrimSize, FacingYaw);

        FVector GlassOffset = LocalCenter;
        GlassOffset.Y -= bSideWall ? 0.0f : 3.0f;
        GlassOffset.X -= bSideWall ? 3.0f : 0.0f;
        AddBox(Glass, LocalToWorld(HouseLocation, HouseYaw, GlassOffset), GlassSize, FacingYaw);
    }
}

bool UOCR13OsterResidentialArchitectureSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13OsterResidentialArchitectureSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    // Wait until source/whole-city/infill/environment passes have authored all house placement metadata and old
    // companion extras. Then replace only presentation while retaining the tested invisible collision structures.
    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ApplyOsterResidentialArchitecture(*World);
        }), ArchitectureDelaySeconds, false);
}

void UOCR13OsterResidentialArchitectureSubsystem::ApplyOsterResidentialArchitecture(UWorld& World)
{
    TArray<FHouseSeed> Houses;
    GatherAndHideGenericHouses(World, Houses);
    HideLegacyHouseExtras(World);
    if (Houses.IsEmpty()) return;

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* RoofMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Roof_Both_Ends_4m.Roof_Both_Ends_4m"));
    UStaticMesh* PorchMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Porch_4x4m.Porch_4x4m"));
    UStaticMesh* DoorMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Door_01.Door_01"));
    UMaterialInterface* BasicMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    UMaterialInterface* RoofMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Metal_Roof.Metal_Roof"));
    if (!Cube || !BasicMaterial || !RoofMesh) return;

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_OsterResidentialArchitectureRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UMaterialInstanceDynamic* BrickMaterials[] = {
        MakeColorMaterial(ArtRoot, BasicMaterial, TEXT("R13_OsterBrickRed"), FLinearColor(0.43f, 0.17f, 0.095f, 1.0f)),
        MakeColorMaterial(ArtRoot, BasicMaterial, TEXT("R13_OsterBrickOrange"), FLinearColor(0.50f, 0.235f, 0.12f, 1.0f)),
        MakeColorMaterial(ArtRoot, BasicMaterial, TEXT("R13_OsterBrickBrown"), FLinearColor(0.34f, 0.145f, 0.085f, 1.0f)),
        MakeColorMaterial(ArtRoot, BasicMaterial, TEXT("R13_OsterBrickMuted"), FLinearColor(0.47f, 0.28f, 0.19f, 1.0f)),
    };
    UMaterialInstanceDynamic* PlinthMaterial = MakeColorMaterial(
        ArtRoot, BasicMaterial, TEXT("R13_OsterHousePlinth"), FLinearColor(0.065f, 0.07f, 0.065f, 1.0f));
    UMaterialInstanceDynamic* TrimMaterial = MakeColorMaterial(
        ArtRoot, BasicMaterial, TEXT("R13_OsterWindowTrim"), FLinearColor(0.73f, 0.72f, 0.65f, 1.0f));
    UMaterialInstanceDynamic* GlassMaterial = MakeColorMaterial(
        ArtRoot, BasicMaterial, TEXT("R13_OsterWindowGlass"), FLinearColor(0.055f, 0.085f, 0.10f, 1.0f));

    TArray<UInstancedStaticMeshComponent*> BrickBodies;
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(BrickMaterials); ++Index)
    {
        if (UInstancedStaticMeshComponent* Component = MakeVisualISM(
            ArtRoot, Root, Cube, BrickMaterials[Index], FName(*FString::Printf(TEXT("R13_OsterBrickHouse%02d"), Index + 1)), true))
        {
            BrickBodies.Add(Component);
        }
    }
    UInstancedStaticMeshComponent* Plinths = MakeVisualISM(
        ArtRoot, Root, Cube, PlinthMaterial, TEXT("R13_OsterHousePlinths"), true);
    UInstancedStaticMeshComponent* WindowTrim = MakeVisualISM(
        ArtRoot, Root, Cube, TrimMaterial, TEXT("R13_OsterHouseWindowTrim"), true);
    UInstancedStaticMeshComponent* WindowGlass = MakeVisualISM(
        ArtRoot, Root, Cube, GlassMaterial, TEXT("R13_OsterHouseWindowGlass"), false);
    UInstancedStaticMeshComponent* Roofs = MakeVisualISM(
        ArtRoot, Root, RoofMesh, RoofMaterial, TEXT("R13_OsterGreyPitchedRoofs"), true);
    UInstancedStaticMeshComponent* Porches = MakeVisualISM(
        ArtRoot, Root, PorchMesh, nullptr, TEXT("R13_OsterHousePorches"), true);
    UInstancedStaticMeshComponent* Doors = MakeVisualISM(
        ArtRoot, Root, DoorMesh, nullptr, TEXT("R13_OsterHouseDoors"), true);

    if (BrickBodies.IsEmpty() || !Roofs)
    {
        ArtRoot->Destroy();
        return;
    }

    int32 Added = 0;
    for (const FHouseSeed& Seed : Houses)
    {
        const int32 Variant = static_cast<int32>(Seed.StableIndex % 4u);
        UInstancedStaticMeshComponent* BrickBody = BrickBodies[Variant % BrickBodies.Num()];

        const float SourceWidth = FMath::Max(Seed.SourceFootprintCm.X, Seed.SourceFootprintCm.Y);
        const float SourceDepth = FMath::Min(Seed.SourceFootprintCm.X, Seed.SourceFootprintCm.Y);
        const float WidthCm = FMath::Clamp(SourceWidth * (0.92f + 0.025f * Variant), 860.0f, 1380.0f);
        const float DepthCm = FMath::Clamp(SourceDepth * (0.92f + 0.035f * ((Variant + 1) % 3)), 620.0f, 940.0f);
        const float BodyHeightCm = 315.0f + 22.0f * static_cast<float>(Variant);
        const float PlinthHeightCm = 64.0f + 8.0f * static_cast<float>(Variant % 2);

        const FVector BodyCenter = Seed.Location + FVector(0.0f, 0.0f, BodyHeightCm * 0.5f);
        AddBox(BrickBody, BodyCenter, FVector(WidthCm, DepthCm, BodyHeightCm), Seed.Yaw);
        AddBox(Plinths, Seed.Location + FVector(0.0f, 0.0f, PlinthHeightCm * 0.5f),
            FVector(WidthCm + 8.0f, DepthCm + 8.0f, PlinthHeightCm), Seed.Yaw);
        AddFittedRoof(Roofs, RoofMesh, Seed.Location, Seed.Yaw, WidthCm, DepthCm, BodyHeightCm);

        const float FrontY = -DepthCm * 0.5f - 6.0f;
        const float WindowZ = 175.0f;
        const float WindowWidth = FMath::Clamp(WidthCm * 0.14f, 118.0f, 165.0f);
        const float WindowHeight = 128.0f;
        const float DoorX = WidthCm * (Variant % 2 == 0 ? 0.28f : -0.28f);

        AddWindow(WindowTrim, WindowGlass, Seed.Location, Seed.Yaw,
            FVector(-WidthCm * 0.23f, FrontY, WindowZ), WindowWidth, WindowHeight, false);
        AddWindow(WindowTrim, WindowGlass, Seed.Location, Seed.Yaw,
            FVector( WidthCm * 0.02f, FrontY, WindowZ), WindowWidth, WindowHeight, false);
        if (WidthCm >= 1080.0f)
        {
            AddWindow(WindowTrim, WindowGlass, Seed.Location, Seed.Yaw,
                FVector(-WidthCm * 0.42f, FrontY, WindowZ), WindowWidth * 0.90f, WindowHeight, false);
        }

        AddWindow(WindowTrim, WindowGlass, Seed.Location, Seed.Yaw,
            FVector(-WidthCm * 0.5f - 6.0f, -DepthCm * 0.12f, WindowZ),
            FMath::Clamp(DepthCm * 0.19f, 112.0f, 150.0f), WindowHeight, true);

        if (Doors && DoorMesh)
        {
            AddFittedGroundProp(Doors, DoorMesh, Seed.Location, Seed.Yaw,
                FVector(DoorX, FrontY - 2.0f, 0.0f), FVector2D(105.0f, 18.0f), 220.0f);
        }
        if (Porches && PorchMesh && (Variant != 3 || (Seed.StableIndex % 3u) == 0u))
        {
            AddFittedGroundProp(Porches, PorchMesh, Seed.Location, Seed.Yaw,
                FVector(DoorX, FrontY - 118.0f, 0.0f), FVector2D(270.0f, 210.0f), 46.0f);
        }

        ++Added;
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13.6 Oster residential architecture: replaced visible generic village prefabs with %d low brick homes, grey pitched roofs, dark plinths, simple windows/doors/porches; legacy collision retained invisibly."),
        Added);
}
