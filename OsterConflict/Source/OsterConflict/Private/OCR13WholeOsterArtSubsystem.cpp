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
                FRotator(0.0f, ProxyTransform.Rotator().Yaw, 0.0f), Location, TargetScale), true);
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

            // The complete house meshes remain the structural owner. R13EnvironmentDressing adds their companion
            // Extra meshes later, so an attachment can never be mistaken for an entire house here.
            const FHouseArtFamily& Family = HouseFamilies[(Index * 5 + Index / 3) % HouseFamilies.Num()];
            if (!Family.Component || !Family.Mesh) continue;

            const float BaseScale = HouseScaleForProxy(ProxyTransform, Family.Mesh);
            const float Variation = 0.96f + 0.02f * static_cast<float>(Index % 5);
            const float Scale = BaseScale * Variation;
            const float Yaw = ProxyTransform.Rotator().Yaw + static_cast<float>((Index % 7) - 3) * 1.35f;

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
                FRotator(0.0f, static_cast<float>((Index * 53 + 17) % 360), 0.0f), Location, FVector(Scale)), true);
            ++OutCount;
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

    UStaticMesh* Tree01 = LoadArtMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var01.SM_Tree_Var01"));
    UStaticMesh* Tree02 = LoadArtMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var02.SM_Tree_Var02"));
    UStaticMesh* Tree03 = LoadArtMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var03.SM_Tree_Var03"));
    UStaticMesh* Tree04 = LoadArtMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var04.SM_Tree_Var04"));
    UStaticMesh* Tree05 = LoadArtMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var05.SM_Tree_Var05"));

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

    if (!House01 && !House02 && !Tree01 && !Pine01 && !RoadMesh && !SidewalkMesh)
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
        if (!Mesh) return;
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

    int32 HouseCount = 0;
    int32 TreeCount = 0;
    int32 PineCount = 0;
    int32 RoadCount = 0;
    int32 SidewalkCount = 0;

    AddFlatProxyReplacements(FindISM(WorldSector, TEXT("Roads")), RoadsISM, RoadMesh, 1.0f, RoadCount);
    AddFlatProxyReplacements(FindISM(WorldSector, TEXT("Sidewalks")), SidewalksISM, SidewalkMesh, 1.0f, SidewalkCount);
    AddHouseReplacements(FindISM(WorldSector, TEXT("Buildings")), HouseFamilies, HouseCount);
    AddTreeReplacements(FindISM(WorldSector, TEXT("TreeTrunks")), TreeFamilies, 1.00f, TreeCount);
    AddTreeReplacements(FindISM(WorldSector, TEXT("SovietPoplarTrunks")), TreeFamilies, 1.18f, TreeCount);
    AddTreeReplacements(FindISM(WorldSector, TEXT("BirchTrunks")), TreeFamilies, 1.02f, TreeCount);
    AddHeightMatchedTreeReplacements(FindISM(WorldSector, TEXT("PineTrunks")), PineFamilies, 1750.0f, PineCount);

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

    // EnvironmentDressing owns adaptive PN grass/ground plants. Hide source cube proxies here but do not render a
    // second grass carpet, otherwise the two delayed passes double density and hurt both readability and performance.
    HideProxy(WorldSector, TEXT("GrassMown"));
    HideProxy(WorldSector, TEXT("GrassRough"));
    HideProxy(WorldSector, TEXT("GrassWetland"));

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
        TEXT("R13.4 whole-Oster art: roads=%d sidewalks=%d houses=%d deciduous=%d pines=%d; adaptive grass delegated to EnvironmentDressing; hidden rejected fantasy families=%d."),
        RoadCount, SidewalkCount, HouseCount, TreeCount, PineCount, HiddenFantasyFamilies);
}
