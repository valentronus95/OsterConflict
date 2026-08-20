#include "OCR13ResidentialInfillFenceSubsystem.h"

#include "OCGameMode.h"

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
    constexpr float FenceDelaySeconds = 1.42f;
    constexpr float FrontFenceLengthCm = 3100.0f;
    constexpr float GateOpeningCm = 420.0f;
    constexpr float SideFenceLengthCm = 3300.0f;
    constexpr float FrontOffsetCm = 1640.0f;
    constexpr float SideOffsetCm = 1530.0f;

    struct FFenceFamily
    {
        UInstancedStaticMeshComponent* Component = nullptr;
        UStaticMesh* Mesh = nullptr;
    };

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        UMaterialInterface* Material, const FName Name, const bool bCollision)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material) Component->SetMaterial(0, Material);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(bCollision);
        Component->SetCastShadow(!bCollision);
        Component->SetCullDistances(0, 80000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        if (bCollision) Component->SetVisibility(false, true);
        return Component;
    }

    FFenceFamily MakeFamily(AActor* Owner, USceneComponent* Root, const TCHAR* AssetPath, const FName Name)
    {
        FFenceFamily Family;
        Family.Mesh = LoadObject<UStaticMesh>(nullptr, AssetPath);
        Family.Component = MakeISM(Owner, Root, Family.Mesh, nullptr, Name, false);
        return Family;
    }

    bool IsUsableVerticalPanel(UStaticMesh* Mesh)
    {
        if (!Mesh) return false;
        const FVector Size = Mesh->GetBounds().BoxExtent * 2.0f;
        const float LongXY = FMath::Max(Size.X, Size.Y);
        const float ThinXY = FMath::Max(1.0f, FMath::Min(Size.X, Size.Y));
        return LongXY >= 50.0f && Size.Z >= 50.0f && Size.Z >= ThinXY * 1.20f;
    }

    void GatherInfillHouseTransforms(UWorld& World, TArray<FTransform>& OutTransforms)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (!Actor || !Actor->GetRootComponent() ||
                Actor->GetRootComponent()->GetFName() != TEXT("R13_ResidentialInfillRoot"))
            {
                continue;
            }

            TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
            Actor->GetComponents(Components);
            for (UInstancedStaticMeshComponent* Component : Components)
            {
                if (!Component) continue;
                const FName Name = Component->GetFName();
                if (Name != TEXT("R13_House01") && Name != TEXT("R13_House02")) continue;
                for (int32 Index = 0; Index < Component->GetInstanceCount(); ++Index)
                {
                    FTransform Transform;
                    if (Component->GetInstanceTransform(Index, Transform, true)) OutTransforms.Add(Transform);
                }
            }
            break;
        }
    }

    void AddCollisionRun(UInstancedStaticMeshComponent* Collision, const FVector& GroundCenter,
        const float Yaw, const float LengthCm, const float HeightCm)
    {
        if (!Collision) return;
        FVector Center = GroundCenter;
        Center.Z = HeightCm * 0.5f;
        Collision->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), Center,
            FVector(LengthCm / 100.0f, 0.18f, HeightCm / 100.0f)), true);
    }

    void AddPanelRun(const TArray<FFenceFamily>& Families, const FVector& GroundCenter,
        const float Yaw, const float LengthCm, const float HeightCm, const int32 Seed)
    {
        if (Families.IsEmpty() || LengthCm <= 1.0f || HeightCm <= 1.0f) return;

        const FFenceFamily& Sample = Families[Seed % Families.Num()];
        if (!Sample.Component || !Sample.Mesh) return;
        const FVector SampleSize = Sample.Mesh->GetBounds().BoxExtent * 2.0f;
        const bool bSampleLongX = SampleSize.X >= SampleSize.Y;
        const float SampleLong = FMath::Max(1.0f, bSampleLongX ? SampleSize.X : SampleSize.Y);
        const float SampleHeight = FMath::Max(1.0f, SampleSize.Z);
        const float SampleHeightScale = FMath::Clamp(HeightCm / SampleHeight, 0.55f, 2.40f);
        const float NaturalModuleLength = FMath::Max(80.0f, SampleLong * SampleHeightScale);
        const int32 ModuleCount = FMath::Clamp(FMath::CeilToInt(LengthCm / NaturalModuleLength), 1, 24);
        const float ModuleSpacing = LengthCm / static_cast<float>(ModuleCount);
        const FQuat RunRotation(FRotator(0.0f, Yaw, 0.0f));
        const FVector RunAxis = RunRotation.RotateVector(FVector::ForwardVector);

        for (int32 ModuleIndex = 0; ModuleIndex < ModuleCount; ++ModuleIndex)
        {
            const FFenceFamily& Family = Families[(Seed + ModuleIndex) % Families.Num()];
            if (!Family.Component || !Family.Mesh) continue;

            const FBoxSphereBounds Bounds = Family.Mesh->GetBounds();
            const FVector MeshSize = Bounds.BoxExtent * 2.0f;
            const bool bMeshLongX = MeshSize.X >= MeshSize.Y;
            const float MeshLong = FMath::Max(1.0f, bMeshLongX ? MeshSize.X : MeshSize.Y);
            const float MeshHeight = FMath::Max(1.0f, MeshSize.Z);
            const float HeightScale = FMath::Clamp(HeightCm / MeshHeight, 0.55f, 2.40f);

            FVector Scale(HeightScale);
            if (bMeshLongX) Scale.X = ModuleSpacing / MeshLong;
            else Scale.Y = ModuleSpacing / MeshLong;

            FQuat Rotation = RunRotation;
            if (!bMeshLongX) Rotation = RunRotation * FQuat(FVector::UpVector, FMath::DegreesToRadians(90.0f));

            const float Along = -0.5f * LengthCm + (static_cast<float>(ModuleIndex) + 0.5f) * ModuleSpacing;
            FVector Location = GroundCenter + RunAxis * Along;
            const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
            Location.Z = -LocalBottom * Scale.Z + 2.0f;
            Family.Component->AddInstance(FTransform(Rotation, Location, Scale), true);
        }
    }

    void AddOpenMetalRun(UInstancedStaticMeshComponent* Pickets, UInstancedStaticMeshComponent* Rails,
        const FVector& GroundCenter, const float Yaw, const float LengthCm, const float HeightCm)
    {
        if (!Pickets || !Rails) return;
        const FQuat Rotation(FRotator(0.0f, Yaw, 0.0f));
        const FVector Axis = Rotation.RotateVector(FVector::ForwardVector);
        const int32 PicketCount = FMath::Clamp(FMath::CeilToInt(LengthCm / 105.0f) + 1, 2, 40);
        const float Step = LengthCm / static_cast<float>(FMath::Max(1, PicketCount - 1));

        for (int32 Index = 0; Index < PicketCount; ++Index)
        {
            const float Along = -0.5f * LengthCm + Step * static_cast<float>(Index);
            FVector Location = GroundCenter + Axis * Along;
            Location.Z = HeightCm * 0.5f;
            Pickets->AddInstance(FTransform(Rotation, Location,
                FVector(0.055f, 0.055f, HeightCm / 100.0f)), true);
        }

        constexpr float RailFractions[] = { 0.26f, 0.56f, 0.84f };
        for (const float Fraction : RailFractions)
        {
            FVector Location = GroundCenter;
            Location.Z = HeightCm * Fraction;
            Rails->AddInstance(FTransform(Rotation, Location,
                FVector(LengthCm / 100.0f, 0.065f, 0.065f)), true);
        }
    }
}

bool UOCR13ResidentialInfillFenceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13ResidentialInfillFenceSubsystem::OnWorldBeginPlay(UWorld& InWorld)
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
            if (UWorld* World = WeakWorld.Get()) BuildInfillFences(*World);
        }), FenceDelaySeconds, false);
}

void UOCR13ResidentialInfillFenceSubsystem::BuildInfillFences(UWorld& World)
{
    if (bApplied) return;

    TArray<FTransform> Houses;
    GatherInfillHouseTransforms(World, Houses);
    if (Houses.IsEmpty()) return;

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube) return;

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(true);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_ResidentialInfillFenceRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    auto AddFamily = [ArtRoot, Root](TArray<FFenceFamily>& Families, const TCHAR* AssetPath,
        const FName ComponentName, const bool bRequireVerticalPanel)
    {
        FFenceFamily Family = MakeFamily(ArtRoot, Root, AssetPath, ComponentName);
        if (!Family.Component || !Family.Mesh) return;
        if (bRequireVerticalPanel && !IsUsableVerticalPanel(Family.Mesh))
        {
            Family.Component->DestroyComponent();
            return;
        }
        Families.Add(Family);
    };

    TArray<FFenceFamily> WoodFamilies;
    AddFamily(WoodFamilies,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Fence_Old_1_2m.Fence_Old_1_2m"),
        TEXT("R13_InfillWoodFence01"), false);
    AddFamily(WoodFamilies,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Fence_Old_2_2m.Fence_Old_2_2m"),
        TEXT("R13_InfillWoodFence02"), false);
    AddFamily(WoodFamilies,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Fence_Old_3_2m.Fence_Old_3_2m"),
        TEXT("R13_InfillWoodFence03"), false);

    TArray<FFenceFamily> SheetFamilies;
    AddFamily(SheetFamilies,
        TEXT("/Game/Scene_RoadsideConstruction/Assets/MS/3D/Urb_Roa_Sheet_Metal_Rusty_01/SM_Urb_Roa_Sheet_Metal_Rusty_01.SM_Urb_Roa_Sheet_Metal_Rusty_01"),
        TEXT("R13_InfillSheetFence01"), true);
    AddFamily(SheetFamilies,
        TEXT("/Game/Scene_RoadsideConstruction/Assets/MS/3D/Urb_Roa_Sheet_Metal_Rusty_02/SM_Urb_Roa_Sheet_Metal_Rusty_02.SM_Urb_Roa_Sheet_Metal_Rusty_02"),
        TEXT("R13_InfillSheetFence02"), true);
    AddFamily(SheetFamilies,
        TEXT("/Game/Scene_RoadsideConstruction/Assets/MS/3D/Urb_Roa_Sheet_Metal_Rusty_03/SM_Urb_Roa_Sheet_Metal_Rusty_03.SM_Urb_Roa_Sheet_Metal_Rusty_03"),
        TEXT("R13_InfillSheetFence03"), true);

    UInstancedStaticMeshComponent* MetalPickets = MakeISM(
        ArtRoot, Root, Cube, nullptr, TEXT("R13_InfillMetalPickets"), false);
    UInstancedStaticMeshComponent* MetalRails = MakeISM(
        ArtRoot, Root, Cube, nullptr, TEXT("R13_InfillMetalRails"), false);
    UInstancedStaticMeshComponent* Collision = MakeISM(
        ArtRoot, Root, Cube, nullptr, TEXT("R13_InfillFenceCollision"), true);

    if (BaseMaterial && MetalPickets && MetalRails)
    {
        UMaterialInstanceDynamic* MetalMaterial = UMaterialInstanceDynamic::Create(
            BaseMaterial, ArtRoot, TEXT("R13_InfillFenceMetalMat"));
        if (MetalMaterial)
        {
            MetalMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.10f, 0.12f, 0.12f, 1.0f));
            MetalPickets->SetMaterial(0, MetalMaterial);
            MetalRails->SetMaterial(0, MetalMaterial);
        }
    }

    int32 FencedHouseCount = 0;
    int32 RunCount = 0;
    for (int32 HouseIndex = 0; HouseIndex < Houses.Num(); ++HouseIndex)
    {
        // Preserve some open lots instead of turning every house into an identical fortress.
        if ((HouseIndex % 7) == 0) continue;

        const FTransform& House = Houses[HouseIndex];
        const FVector HouseLocation = House.GetLocation();
        const float HouseYaw = House.Rotator().Yaw;
        const FQuat HouseRotation(FRotator(0.0f, HouseYaw, 0.0f));
        const float FenceHeight = 185.0f + static_cast<float>((HouseIndex % 4) * 15);

        const float FrontSegmentLength = (FrontFenceLengthCm - GateOpeningCm) * 0.5f;
        const float FrontCenterOffset = GateOpeningCm * 0.5f + FrontSegmentLength * 0.5f;
        const FVector FrontA = HouseLocation + HouseRotation.RotateVector(
            FVector(-FrontCenterOffset, -FrontOffsetCm, 0.0f));
        const FVector FrontB = HouseLocation + HouseRotation.RotateVector(
            FVector( FrontCenterOffset, -FrontOffsetCm, 0.0f));
        const FVector Side = HouseLocation + HouseRotation.RotateVector(
            FVector(-SideOffsetCm, 10.0f, 0.0f));

        const int32 Roll = HouseIndex % 20;
        enum class EFenceKind : uint8 { Wood, Metal, Sheet };
        EFenceKind Kind = EFenceKind::Wood;
        if (Roll >= 12 && Roll < 17) Kind = EFenceKind::Metal;
        else if (Roll >= 17) Kind = EFenceKind::Sheet;
        if (Kind == EFenceKind::Sheet && SheetFamilies.IsEmpty()) Kind = EFenceKind::Wood;
        if (Kind == EFenceKind::Wood && WoodFamilies.IsEmpty()) Kind = EFenceKind::Metal;

        auto AddRun = [&](const FVector& Center, const float Yaw, const float Length, const int32 Seed)
        {
            if (Kind == EFenceKind::Wood) AddPanelRun(WoodFamilies, Center, Yaw, Length, FenceHeight, Seed);
            else if (Kind == EFenceKind::Sheet) AddPanelRun(SheetFamilies, Center, Yaw, Length, FenceHeight, Seed);
            else AddOpenMetalRun(MetalPickets, MetalRails, Center, Yaw, Length, FenceHeight);
            AddCollisionRun(Collision, Center, Yaw, Length, FenceHeight);
            ++RunCount;
        };

        AddRun(FrontA, HouseYaw, FrontSegmentLength, HouseIndex * 7 + 1);
        AddRun(FrontB, HouseYaw, FrontSegmentLength, HouseIndex * 7 + 3);
        AddRun(Side, HouseYaw + 90.0f, SideFenceLengthCm, HouseIndex * 7 + 5);
        ++FencedHouseCount;
    }

    if (RunCount <= 0)
    {
        ArtRoot->Destroy();
        return;
    }

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("R13.4 residential infill fences: fenced houses=%d/%d runs=%d; weighted wood/open-metal/corrugated-sheet fronts keep %.0f cm gate openings; visual modules use hidden continuous collision."),
        FencedHouseCount, Houses.Num(), RunCount, GateOpeningCm);
}
