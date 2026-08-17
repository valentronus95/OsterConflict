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
    UStaticMesh* LoadArtMesh(const TCHAR* Path)
    {
        UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Path);
        if (!Mesh)
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
        Component->SetCastShadow(true);
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
        // Houses and trees in the authored R12 street slice already have dedicated art. Grass is intentionally not
        // excluded: the previous exclusion removed the proxy grass globally and then supplied no replacement here.
        return FMath::Abs(Location.X + 3400.0f) < 7000.0f &&
            Location.Y > -14500.0f && Location.Y < 17500.0f;
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

    void AddHouseReplacements(UInstancedStaticMeshComponent* Proxy, UInstancedStaticMeshComponent* HouseA,
        UInstancedStaticMeshComponent* HouseB, UStaticMesh* MeshA, UStaticMesh* MeshB, int32& OutCount)
    {
        if (!Proxy || (!HouseA && !HouseB)) return;
        for (int32 Index = 0; Index < Proxy->GetInstanceCount(); ++Index)
        {
            FTransform ProxyTransform;
            if (!Proxy->GetInstanceTransform(Index, ProxyTransform, true)) continue;
            FVector Location = ProxyTransform.GetLocation();
            if (IsInsideR12KrushelnytskaSlice(Location)) continue;
            Location.Z = 0.0f;

            const bool bUseA = (Index % 3) != 1;
            UInstancedStaticMeshComponent* Target = bUseA ? HouseA : HouseB;
            UStaticMesh* Mesh = bUseA ? MeshA : MeshB;
            if (!Target || !Mesh) continue;

            const float Scale = HouseScaleForProxy(ProxyTransform, Mesh);
            const float Yaw = ProxyTransform.Rotator().Yaw + ((Index % 5) - 2) * 1.5f;
            Target->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), Location, FVector(Scale)), true);
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

            UInstancedStaticMeshComponent* Target = TreeFamilies[Index % TreeFamilies.Num()];
            if (!Target) continue;
            const float Variation = 0.90f + 0.06f * static_cast<float>(Index % 5);
            const float Scale = BaseScale * Variation;
            Target->AddInstance(FTransform(FRotator(0.0f, static_cast<float>((Index * 37) % 360), 0.0f),
                Location, FVector(Scale)), true);
            ++OutCount;
        }
    }

    void AddGrassReplacements(UInstancedStaticMeshComponent* Proxy,
        const TArray<UInstancedStaticMeshComponent*>& GrassFamilies, int32& OutCount)
    {
        if (!Proxy || GrassFamilies.Num() == 0) return;

        // Five-by-five coverage is still cheap with ISM, but reads as actual ground vegetation instead of nine
        // isolated tufts. Do not exclude the R12 slice: its source grass proxy is valid topology data for R13 too.
        constexpr float Fractions[] = { -0.42f, -0.21f, 0.0f, 0.21f, 0.42f };

        for (int32 Index = 0; Index < Proxy->GetInstanceCount(); ++Index)
        {
            FTransform ProxyTransform;
            if (!Proxy->GetInstanceTransform(Index, ProxyTransform, true)) continue;
            const FVector Center = ProxyTransform.GetLocation();

            const FVector ProxyScale = ProxyTransform.GetScale3D();
            const float Width = FMath::Max(500.0f, FMath::Abs(ProxyScale.X) * 100.0f);
            const float Depth = FMath::Max(500.0f, FMath::Abs(ProxyScale.Y) * 100.0f);

            int32 Local = 0;
            for (float FX : Fractions)
            {
                for (float FY : Fractions)
                {
                    UInstancedStaticMeshComponent* Target = GrassFamilies[(Index + Local) % GrassFamilies.Num()];
                    ++Local;
                    if (!Target) continue;

                    const float JitterX = static_cast<float>(((Index * 17 + Local * 7) % 9) - 4) * Width * 0.010f;
                    const float JitterY = static_cast<float>(((Index * 11 + Local * 13) % 9) - 4) * Depth * 0.010f;
                    FVector Location = Center + FVector(FX * Width + JitterX, FY * Depth + JitterY, 3.0f - Center.Z);
                    const float Scale = 0.88f + 0.08f * static_cast<float>((Index + Local) % 5);
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
    if (!World) return false;
    return World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE;
}

void UOCR13WholeOsterArtSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    // R12 creates its real Krushelnytska slice shortly after GameMode BeginPlay. Run after it, then use the source
    // proxy transforms as topology data for real whole-city residential/vegetation replacements.
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
    UStaticMesh* Grass01 = LoadArtMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_GrassPatch_Var01.SM_GrassPatch_Var01"));
    UStaticMesh* Grass02 = LoadArtMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_GrassPatch_Var02.SM_GrassPatch_Var02"));
    UStaticMesh* Grass03 = LoadArtMesh(TEXT("/Game/AdvancedVillagePack/Meshes/SM_GrassPatch_Var03.SM_GrassPatch_Var03"));

    if (!House01 && !House02 && !Tree01 && !Grass01)
    {
        UE_LOG(LogTemp, Warning, TEXT("R13 whole-Oster art: AdvancedVillagePack not available; preserving proxy topology."));
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

    UInstancedStaticMeshComponent* House01ISM = MakeISM(ArtRoot, Root, House01, TEXT("R13_House01"), true);
    UInstancedStaticMeshComponent* House02ISM = MakeISM(ArtRoot, Root, House02, TEXT("R13_House02"), true);

    TArray<UInstancedStaticMeshComponent*> TreeFamilies = {
        MakeISM(ArtRoot, Root, Tree01, TEXT("R13_Tree01"), true),
        MakeISM(ArtRoot, Root, Tree02, TEXT("R13_Tree02"), true),
        MakeISM(ArtRoot, Root, Tree03, TEXT("R13_Tree03"), true),
        MakeISM(ArtRoot, Root, Tree04, TEXT("R13_Tree04"), true),
        MakeISM(ArtRoot, Root, Tree05, TEXT("R13_Tree05"), true),
    };
    TreeFamilies.Remove(nullptr);

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
    int32 GrassCount = 0;

    AddHouseReplacements(FindISM(WorldSector, TEXT("Buildings")), House01ISM, House02ISM,
        House01, House02, HouseCount);
    AddTreeReplacements(FindISM(WorldSector, TEXT("TreeTrunks")), TreeFamilies, 1.00f, TreeCount);
    AddTreeReplacements(FindISM(WorldSector, TEXT("SovietPoplarTrunks")), TreeFamilies, 1.18f, TreeCount);
    AddTreeReplacements(FindISM(WorldSector, TEXT("BirchTrunks")), TreeFamilies, 1.02f, TreeCount);
    AddTreeReplacements(FindISM(WorldSector, TEXT("PineTrunks")), TreeFamilies, 1.08f, TreeCount);
    AddGrassReplacements(FindISM(WorldSector, TEXT("GrassMown")), GrassFamilies, GrassCount);
    AddGrassReplacements(FindISM(WorldSector, TEXT("GrassRough")), GrassFamilies, GrassCount);
    AddGrassReplacements(FindISM(WorldSector, TEXT("GrassWetland")), GrassFamilies, GrassCount);

    // Hide a proxy family only after a real replacement family actually produced instances. This prevents a missing
    // asset or an empty source family from turning entire parts of the map into featureless ground.
    if (HouseCount > 0)
    {
        HideProxy(WorldSector, TEXT("Buildings"));
        HideProxy(WorldSector, TEXT("ResidentialRoofs"));
        HideProxy(WorldSector, TEXT("ResidentialDetails"));
    }
    if (TreeCount > 0)
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
        TEXT("R13 whole-Oster art: real houses=%d trees=%d grass patches=%d; hidden rejected fantasy families=%d."),
        HouseCount, TreeCount, GrassCount, HiddenFantasyFamilies);
}
