#include "OCR13WholeOsterArtSubsystem.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    struct FHouseArtFamily
    {
        UInstancedStaticMeshComponent* Component = nullptr;
        UStaticMesh* Mesh = nullptr;
    };

    struct FTreeArtFamily
    {
        UInstancedStaticMeshComponent* Component = nullptr;
        UStaticMesh* Mesh = nullptr;
    };

    UStaticMesh* LoadArtMesh(const TCHAR* Path, bool bWarn = true)
    {
        UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Path);
        if (!Mesh && bWarn)
        {
            UE_LOG(LogTemp, Warning, TEXT("R13 whole-Oster art: missing mesh %s"), Path);
        }
        return Mesh;
    }

    UStaticMesh* LoadFirstAvailableGrass(const TCHAR* Preferred, const TCHAR* Fallback)
    {
        if (UStaticMesh* Mesh = LoadArtMesh(Preferred, false)) return Mesh;
        return LoadArtMesh(Fallback, true);
    }

    bool IsUsableHouseMesh(UStaticMesh* Mesh)
    {
        if (!Mesh) return false;
        const FVector Size = Mesh->GetBounds().BoxExtent * 2.0f;
        return Size.X >= 300.0f && Size.Y >= 300.0f && Size.Z >= 220.0f &&
            Size.X <= 5000.0f && Size.Y <= 5000.0f && Size.Z <= 2200.0f;
    }

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        const FName Name, bool bCollision)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCastShadow(bCollision);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    UInstancedStaticMeshComponent* FindISM(AActor* Actor, const FName Name)
    {
        if (!Actor) return nullptr;
        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Actor->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return Component;
        }
        return nullptr;
    }

    void HideProxy(AActor* Actor, const FName Name)
    {
        if (UPrimitiveComponent* Primitive = FindObjectFast<UPrimitiveComponent>(Actor, Name))
        {
            Primitive->SetVisibility(false, true);
            Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }

    bool IsInsideR12KrushelnytskaSlice(const FVector& Location)
    {
        return FMath::Abs(Location.X + 3400.0f) < 7000.0f &&
            Location.Y > -14500.0f && Location.Y < 17500.0f;
    }

    bool IsMuseumGarden(const FVector& Location)
    {
        // Museum is the local georeference origin. Keep a distinct mature-tree character around the landmark.
        return Location.Size2D() <= 10500.0f;
    }

    float HouseScaleForProxy(const FTransform& ProxyTransform, UStaticMesh* HouseMesh)
    {
        if (!HouseMesh) return 1.0f;
        const FVector ProxyScale = ProxyTransform.GetScale3D();
        const float DesiredX = FMath::Max(400.0f, FMath::Abs(ProxyScale.X) * 100.0f);
        const float DesiredY = FMath::Max(400.0f, FMath::Abs(ProxyScale.Y) * 100.0f);
        const FVector MeshSize = HouseMesh->GetBounds().BoxExtent * 2.0f;
        const float MeshFootprint = FMath::Sqrt(FMath::Max(1.0f, MeshSize.X * MeshSize.Y));
        const float DesiredFootprint = FMath::Sqrt(DesiredX * DesiredY);
        return FMath::Clamp(DesiredFootprint / MeshFootprint, 0.65f, 2.25f);
    }

    float GrassScaleForCell(UInstancedStaticMeshComponent* Target, float CellSize, int32 Variant)
    {
        if (!Target || !Target->GetStaticMesh()) return 1.0f;
        const FVector MeshSize = Target->GetStaticMesh()->GetBounds().BoxExtent * 2.0f;
        const float MeshFootprint = FMath::Max(1.0f, FMath::Max(MeshSize.X, MeshSize.Y));
        const float DesiredFootprint = FMath::Clamp(CellSize * 0.84f, 105.0f, 285.0f);
        const float Variation = 0.88f + 0.045f * static_cast<float>(Variant % 6);
        return FMath::Clamp((DesiredFootprint / MeshFootprint) * Variation, 0.30f, 7.0f);
    }

    void AddFlatProxyReplacements(UInstancedStaticMeshComponent* Proxy,
        UInstancedStaticMeshComponent* Target, UStaticMesh* TargetMesh, float ZOffset, int32& OutCount)
    {
        if (!Proxy || !Target || !TargetMesh) return;

        const FVector TargetSize = TargetMesh->GetBounds().BoxExtent * 2.0f;
        if (TargetSize.X <= KINDA_SMALL_NUMBER || TargetSize.Y <= KINDA_SMALL_NUMBER) return;

        for (int32 Index = 0; Index < Proxy->GetInstanceCount(); ++Index)
        {
            FTransform ProxyTransform;
            if (!Proxy->GetInstanceTransform(Index, ProxyTransform, true)) continue;

            const FVector ProxyScale = ProxyTransform.GetScale3D();
            const float DesiredX = FMath::Max(100.0f, FMath::Abs(ProxyScale.X) * 100.0f);
            const float DesiredY = FMath::Max(100.0f, FMath::Abs(ProxyScale.Y) * 100.0f);
            const float ScaleX = DesiredX / TargetSize.X;
            const float ScaleY = DesiredY / TargetSize.Y;
            const float ScaleZ = FMath::Clamp(FMath::Min(ScaleX, ScaleY), 0.35f, 2.5f);

            FVector Location = ProxyTransform.GetLocation();
            Location.Z += ZOffset;
            const FVector TargetScale(ScaleX, ScaleY, ScaleZ);
            Location -= TargetMesh->GetBounds().Origin * TargetScale;

            Target->AddInstance(FTransform(
                FRotator(0.0f, ProxyTransform.Rotator().Yaw, 0.0f),
                Location, TargetScale), true);
            ++OutCount;
        }
    }

    void AddHouseReplacements(UInstancedStaticMeshComponent* Proxy,
        const TArray<FHouseArtFamily>& HouseFamilies, int32& OutCount)
    {
        if (!Proxy || HouseFamilies.Num() == 0) return;

        for (int32 Index = 0; Index < Proxy->GetInstanceCount(); ++Index)
        {
            FTransform ProxyTransform;
            if (!Proxy->GetInstanceTransform(Index, ProxyTransform, true)) continue;
            FVector Location = ProxyTransform.GetLocation();
            if (IsInsideR12KrushelnytskaSlice(Location)) continue;

            const FHouseArtFamily& Family = HouseFamilies[(Index * 5 + Index / 3) % HouseFamilies.Num()];
            if (!Family.Component || !Family.Mesh) continue;

            const float BaseScale = HouseScaleForProxy(ProxyTransform, Family.Mesh);
            const float Variation = 0.96f + 0.02f * static_cast<float>(Index % 5);
            const float Scale = BaseScale * Variation;
            const float Yaw = ProxyTransform.Rotator().Yaw + static_cast<float>((Index % 7) - 3) * 1.35f;

            // Ground by actual mesh bounds instead of assuming every house author used the same pivot.
            const FBoxSphereBounds Bounds = Family.Mesh->GetBounds();
            const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
            Location.Z = -LocalBottom * Scale;

            Family.Component->AddInstance(
                FTransform(FRotator(0.0f, Yaw, 0.0f), Location, FVector(Scale)), true);
            ++OutCount;
        }
    }

    void AddTreeReplacements(UInstancedStaticMeshComponent* Proxy,
        const TArray<UInstancedStaticMeshComponent*>& TreeFamilies, float BaseScale, int32& OutCount)
    {
        if (!Proxy || TreeFamilies.Num() == 0) return;
        for (int32 Index = 0; Index < Proxy->GetInstanceCount(); ++Index)
        {
            FTransform ProxyTransform;
            if (!Proxy->GetInstanceTransform(Index, ProxyTransform, true)) continue;
            FVector Location = ProxyTransform.GetLocation();
            if (IsInsideR12KrushelnytskaSlice(Location)) continue;
            Location.Z = 0.0f;

            int32 FamilyIndex = Index % TreeFamilies.Num();
            float LocalBaseScale = BaseScale;
            if (IsMuseumGarden(Location))
            {
                // Mature general tree stock remains slightly larger around the landmark garden. Conifers are handled
                // by a dedicated height-matched pine family below, so this no longer pretends a deciduous mesh is pine.
                FamilyIndex = FMath::Max(0, TreeFamilies.Num() - 1 - (Index % FMath::Min(2, TreeFamilies.Num())));
                LocalBaseScale *= 1.10f;
            }

            UInstancedStaticMeshComponent* Target = TreeFamilies[FamilyIndex];
            if (!Target) continue;
            const float Variation = 0.90f + 0.055f * static_cast<float>(Index % 5);
            const float Scale = LocalBaseScale * Variation;
            Target->AddInstance(FTransform(
                FRotator(0.0f, static_cast<float>((Index * 37) % 360), 0.0f), Location, FVector(Scale)), true);
            ++OutCount;
        }
    }

    void AddHeightMatchedTreeReplacements(UInstancedStaticMeshComponent* Proxy,
        const TArray<FTreeArtFamily>& Families, const float BaseHeightCm, int32& OutCount)
    {
        if (!Proxy || Families.Num() == 0) return;

        for (int32 Index = 0; Index < Proxy->GetInstanceCount(); ++Index)
        {
            FTransform ProxyTransform;
            if (!Proxy->GetInstanceTransform(Index, ProxyTransform, true)) continue;
            FVector Location = ProxyTransform.GetLocation();
            if (IsInsideR12KrushelnytskaSlice(Location)) continue;

            const FTreeArtFamily& Family = Families[Index % Families.Num()];
            if (!Family.Component || !Family.Mesh) continue;

            const FBoxSphereBounds Bounds = Family.Mesh->GetBounds();
            const FVector MeshSize = Bounds.BoxExtent * 2.0f;
            if (MeshSize.Z <= 10.0f) continue;

            float DesiredHeight = BaseHeightCm * (0.91f + 0.045f * static_cast<float>(Index % 5));
            if (IsMuseumGarden(Location)) DesiredHeight *= 1.10f;
            const float Scale = FMath::Clamp(DesiredHeight / MeshSize.Z, 0.30f, 4.0f);
            const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
            Location.Z = -LocalBottom * Scale;

            Family.Component->AddInstance(FTransform(
                FRotator(0.0f, static_cast<float>((Index * 53 + 17) % 360), 0.0f),
                Location, FVector(Scale)), true);
            ++OutCount;
        }
    }

    void AddGrassReplacements(UInstancedStaticMeshComponent* Proxy,
        const TArray<UInstancedStaticMeshComponent*>& GrassFamilies, int32& OutCount)
    {
        if (!Proxy || GrassFamilies.Num() == 0) return;

        for (int32 Index = 0; Index < Proxy->GetInstanceCount(); ++Index)
        {
            FTransform ProxyTransform;
            if (!Proxy->GetInstanceTransform(Index, ProxyTransform, true)) continue;
            const FVector Center = ProxyTransform.GetLocation();
            const FVector ProxyScale = ProxyTransform.GetScale3D();
            const float Width = FMath::Max(500.0f, FMath::Abs(ProxyScale.X) * 100.0f);
            const float Depth = FMath::Max(500.0f, FMath::Abs(ProxyScale.Y) * 100.0f);

            // R13.4 density pass: scale the foliage grid with the authored lawn/rough patch size instead of
            // painting every patch with the same sparse 5x5 stamp. Bounds keep instance counts predictable.
            const int32 CellsX = FMath::Clamp(FMath::CeilToInt(Width / 850.0f), 5, 9);
            const int32 CellsY = FMath::Clamp(FMath::CeilToInt(Depth / 850.0f), 5, 9);
            const float CellWidth = Width / static_cast<float>(CellsX);
            const float CellDepth = Depth / static_cast<float>(CellsY);
            const float CellSize = FMath::Min(CellWidth, CellDepth);
            const FQuat PatchRotation = FQuat(FRotator(0.0f, ProxyTransform.Rotator().Yaw, 0.0f));

            int32 Local = 0;
            for (int32 X = 0; X < CellsX; ++X)
            {
                for (int32 Y = 0; Y < CellsY; ++Y)
                {
                    UInstancedStaticMeshComponent* Target = GrassFamilies[(Index + Local) % GrassFamilies.Num()];
                    ++Local;
                    if (!Target) continue;

                    const float FX = ((static_cast<float>(X) + 0.5f) / static_cast<float>(CellsX)) - 0.5f;
                    const float FY = ((static_cast<float>(Y) + 0.5f) / static_cast<float>(CellsY)) - 0.5f;
                    const float JitterX = static_cast<float>(((Index * 17 + Local * 7) % 11) - 5) * CellWidth * 0.035f;
                    const float JitterY = static_cast<float>(((Index * 11 + Local * 13) % 11) - 5) * CellDepth * 0.035f;
                    const FVector LocalOffset(FX * Width * 0.94f + JitterX, FY * Depth * 0.94f + JitterY, 0.0f);
                    FVector Location = Center + PatchRotation.RotateVector(LocalOffset);
                    Location.Z = 3.0f;

                    const float Scale = GrassScaleForCell(Target, CellSize, Index + Local);
                    Target->AddInstance(FTransform(
                        FRotator(0.0f, static_cast<float>(((Index * 29) + Local * 47) % 360), 0.0f),
                        Location, FVector(Scale)), true);
                    ++OutCount;
                }
            }
        }
    }

    bool IsRejectedFantasySliceProp(const FName Name)
    {
        const FString Value = Name.ToString();
        return Value.StartsWith(TEXT("R12_Fence")) || Value == TEXT("R12_StreetLights");
    }
}

bool UOCR13WholeOsterArtSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13WholeOsterArtSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ApplyWholeOsterBridge(*World);
        }), 0.80f, false);
}

void UOCR13WholeOsterArtSubsystem::ApplyWholeOsterBridge(UWorld& World)
{
    AActor* WorldSector = nullptr;
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("OCWorldSectorOster")))
        {
            WorldSector = Actor;
            break;
        }
    }
    if (!WorldSector) return;

    UStaticMesh* House01 = LoadArtMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var01.SM_House_Var01"));
    UStaticMesh* House02 = LoadArtMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var02.SM_House_Var02"));

    // AdvancedVillage includes several optional full-house/extension meshes. Only accept candidates whose bounds
    // are genuinely building-sized; small attachment meshes are ignored instead of being scaled into absurdity.
    UStaticMesh* House01Extra03 = LoadArtMesh(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var01_Extra03.SM_House_Var01_Extra03"), false);
    UStaticMesh* House01Extra05 = LoadArtMesh(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var01_Extra05.SM_House_Var01_Extra05"), false);
    UStaticMesh* House01Extra07 = LoadArtMesh(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var01_Extra07.SM_House_Var01_Extra07"), false);
    UStaticMesh* House02Extra = LoadArtMesh(
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var02_Extra.SM_House_Var02_Extra"), false);

    UStaticMesh* Tree01 = LoadArtMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var01.SM_Tree_Var01"));
    UStaticMesh* Tree02 = LoadArtMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var02.SM_Tree_Var02"));
    UStaticMesh* Tree03 = LoadArtMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var03.SM_Tree_Var03"));
    UStaticMesh* Tree04 = LoadArtMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var04.SM_Tree_Var04"));
    UStaticMesh* Tree05 = LoadArtMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var05.SM_Tree_Var05"));

    // R13.4 species pass: source-authored pine topology now maps to actual bundled conifers instead of random
    // AdvancedVillage deciduous meshes. Height matching keeps the result stable even when source assets use different units.
    UStaticMesh* Pine01 = LoadArtMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_01.SM_Pine_Tree_01"), false);
    UStaticMesh* Pine03 = LoadArtMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_03.SM_Pine_Tree_03"), false);
    UStaticMesh* Pine05 = LoadArtMesh(
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Foliage/SM_Pine_Tree_05.SM_Pine_Tree_05"), false);

    UStaticMesh* RoadMesh = LoadArtMesh(
        TEXT("/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Street_01/SM_Urb_Roa_Street_01.SM_Urb_Roa_Street_01"));
    UStaticMesh* SidewalkMesh = LoadArtMesh(
        TEXT("/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Sidewalk_01/SM_Urb_Roa_Sidewalk_01.SM_Urb_Roa_Sidewalk_01"));

    // Prefer the committed PN foliage collection. AdvancedVillage remains a fallback so old archives still work.
    UStaticMesh* Grass01 = LoadFirstAvailableGrass(
        TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_01_mesh.grass_01_01_mesh"),
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_GrassPatch_Var01.SM_GrassPatch_Var01"));
    UStaticMesh* Grass02 = LoadFirstAvailableGrass(
        TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_02_mesh.grass_01_02_mesh"),
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_GrassPatch_Var02.SM_GrassPatch_Var02"));
    UStaticMesh* Grass03 = LoadFirstAvailableGrass(
        TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/grass_01_03_mesh.grass_01_03_mesh"),
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_GrassPatch_Var03.SM_GrassPatch_Var03"));

    if (!House01 && !House02 && !Tree01 && !Pine01 && !Grass01 && !RoadMesh && !SidewalkMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("R13 whole-Oster art: environment art unavailable; preserving proxy topology."));
        return;
    }

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);
    ArtRoot->SetActorEnableCollision(true);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_WholeOsterArtRoot"));
    if (!Root) return;
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    TArray<FHouseArtFamily> HouseFamilies;
    auto AddHouseFamily = [&](UStaticMesh* Mesh, const FName Name)
    {
        if (!IsUsableHouseMesh(Mesh)) return;
        if (UInstancedStaticMeshComponent* Component = MakeISM(ArtRoot, Root, Mesh, Name, true))
        {
            FHouseArtFamily Family;
            Family.Component = Component;
            Family.Mesh = Mesh;
            HouseFamilies.Add(Family);
        }
    };
    AddHouseFamily(House01, TEXT("R13_House01"));
    AddHouseFamily(House02, TEXT("R13_House02"));
    AddHouseFamily(House01Extra03, TEXT("R13_House01Extra03"));
    AddHouseFamily(House01Extra05, TEXT("R13_House01Extra05"));
    AddHouseFamily(House01Extra07, TEXT("R13_House01Extra07"));
    AddHouseFamily(House02Extra, TEXT("R13_House02Extra"));

    UInstancedStaticMeshComponent* RoadsISM = MakeISM(ArtRoot, Root, RoadMesh, TEXT("R13_Roads"), true);
    UInstancedStaticMeshComponent* SidewalksISM = MakeISM(ArtRoot, Root, SidewalkMesh, TEXT("R13_Sidewalks"), true);
    if (RoadsISM) RoadsISM->SetCastShadow(false);
    if (SidewalksISM) SidewalksISM->SetCastShadow(false);

    TArray<UInstancedStaticMeshComponent*> TreeFamilies = {
        MakeISM(ArtRoot, Root, Tree01, TEXT("R13_Tree01"), true),
        MakeISM(ArtRoot, Root, Tree02, TEXT("R13_Tree02"), true),
        MakeISM(ArtRoot, Root, Tree03, TEXT("R13_Tree03"), true),
        MakeISM(ArtRoot, Root, Tree04, TEXT("R13_Tree04"), true),
        MakeISM(ArtRoot, Root, Tree05, TEXT("R13_Tree05"), true),
    };
    TreeFamilies.Remove(nullptr);

    TArray<FTreeArtFamily> PineFamilies;
    auto AddPineFamily = [&](UStaticMesh* Mesh, const FName Name)
    {
        if (!Mesh) return;
        if (UInstancedStaticMeshComponent* Component = MakeISM(ArtRoot, Root, Mesh, Name, true))
        {
            FTreeArtFamily Family;
            Family.Component = Component;
            Family.Mesh = Mesh;
            PineFamilies.Add(Family);
        }
    };
    AddPineFamily(Pine01, TEXT("R13_Pine01"));
    AddPineFamily(Pine03, TEXT("R13_Pine03"));
    AddPineFamily(Pine05, TEXT("R13_Pine05"));

    TArray<UInstancedStaticMeshComponent*> GrassFamilies = {
        MakeISM(ArtRoot, Root, Grass01, TEXT("R13_Grass01"), false),
        MakeISM(ArtRoot, Root, Grass02, TEXT("R13_Grass02"), false),
        MakeISM(ArtRoot, Root, Grass03, TEXT("R13_Grass03"), false),
    };
    GrassFamilies.Remove(nullptr);
    for (UInstancedStaticMeshComponent* Grass : GrassFamilies)
    {
        if (Grass) Grass->SetCastShadow(false);
    }

    int32 HouseCount = 0;
    int32 TreeCount = 0;
    int32 PineCount = 0;
    int32 GrassCount = 0;
    int32 RoadCount = 0;
    int32 SidewalkCount = 0;

    AddFlatProxyReplacements(FindISM(WorldSector, TEXT("Roads")), RoadsISM, RoadMesh, 1.0f, RoadCount);
    AddFlatProxyReplacements(FindISM(WorldSector, TEXT("Sidewalks")), SidewalksISM, SidewalkMesh, 1.0f, SidewalkCount);
    AddHouseReplacements(FindISM(WorldSector, TEXT("Buildings")), HouseFamilies, HouseCount);
    AddTreeReplacements(FindISM(WorldSector, TEXT("TreeTrunks")), TreeFamilies, 1.00f, TreeCount);
    AddTreeReplacements(FindISM(WorldSector, TEXT("SovietPoplarTrunks")), TreeFamilies, 1.18f, TreeCount);
    AddTreeReplacements(FindISM(WorldSector, TEXT("BirchTrunks")), TreeFamilies, 1.02f, TreeCount);
    AddHeightMatchedTreeReplacements(FindISM(WorldSector, TEXT("PineTrunks")), PineFamilies, 1750.0f, PineCount);
    AddGrassReplacements(FindISM(WorldSector, TEXT("GrassMown")), GrassFamilies, GrassCount);
    AddGrassReplacements(FindISM(WorldSector, TEXT("GrassRough")), GrassFamilies, GrassCount);
    AddGrassReplacements(FindISM(WorldSector, TEXT("GrassWetland")), GrassFamilies, GrassCount);

    if (RoadCount > 0) HideProxy(WorldSector, TEXT("Roads"));
    if (SidewalkCount > 0) HideProxy(WorldSector, TEXT("Sidewalks"));
    if (HouseCount > 0)
    {
        HideProxy(WorldSector, TEXT("Buildings"));
        HideProxy(WorldSector, TEXT("ResidentialRoofs"));
        HideProxy(WorldSector, TEXT("ResidentialDetails"));
    }
    if (TreeCount > 0 || PineCount > 0)
    {
        const FName TreeProxyFamilies[] = {
            TEXT("TreeTrunks"), TEXT("TreeCrowns"), TEXT("SovietPoplarTrunks"), TEXT("SovietPoplarCrowns"),
            TEXT("BirchTrunks"), TEXT("BirchCrowns"), TEXT("PineTrunks"), TEXT("PineCrowns")
        };
        for (const FName Name : TreeProxyFamilies) HideProxy(WorldSector, Name);
    }
    if (GrassCount > 0)
    {
        HideProxy(WorldSector, TEXT("GrassMown"));
        HideProxy(WorldSector, TEXT("GrassRough"));
        HideProxy(WorldSector, TEXT("GrassWetland"));
    }

    int32 HiddenFantasyFamilies = 0;
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;
        TInlineComponentArray<UPrimitiveComponent*> Components;
        Actor->GetComponents(Components);
        for (UPrimitiveComponent* Primitive : Components)
        {
            if (!Primitive || !IsRejectedFantasySliceProp(Primitive->GetFName())) continue;
            Primitive->SetVisibility(false, true);
            Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            ++HiddenFantasyFamilies;
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13.4 whole-Oster art: roads=%d sidewalks=%d houses=%d (%d viable house families) deciduous=%d pines=%d (%d pine families) grass instances=%d; hidden rejected fantasy families=%d."),
        RoadCount, SidewalkCount, HouseCount, HouseFamilies.Num(), TreeCount, PineCount, PineFamilies.Num(),
        GrassCount, HiddenFantasyFamilies);
}
