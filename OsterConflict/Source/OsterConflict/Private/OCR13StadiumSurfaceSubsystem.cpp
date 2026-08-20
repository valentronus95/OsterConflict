#include "OCR13StadiumSurfaceSubsystem.h"

#include "OCGameMode.h"
#include "OCGeoReference.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
    // Macro placement is hard-georeferenced. Internal offsets are photo/map-derived and stay explicit here so they
    // can be tightened without moving the entire site. The long-axis yaw is read from the user's north-up satellite
    // map; it is intentionally not inherited from the old museum-relative blockout.
    constexpr float FieldYawDegrees = -21.5f;
    constexpr float FieldLengthCm = 10500.0f;
    constexpr float FieldWidthCm = 6800.0f;
    constexpr float SiteRadiusCm = 9000.0f;

    FVector SitePoint(const FVector& SiteCenter, const FVector& LocalOffset)
    {
        return SiteCenter + FRotator(0.0f, FieldYawDegrees, 0.0f).RotateVector(LocalOffset);
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

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        UMaterialInterface* Material, const FName Name, const bool bCollision, const bool bCastShadow,
        const int32 CullEndCm = 100000)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material)
        {
            const int32 SlotCount = FMath::Max(1, Mesh->GetStaticMaterials().Num());
            for (int32 Slot = 0; Slot < SlotCount; ++Slot) Component->SetMaterial(Slot, Material);
        }
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(bCollision);
        Component->SetCastShadow(bCastShadow);
        if (CullEndCm > 0) Component->SetCullDistances(0, CullEndCm);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    UMaterialInstanceDynamic* MakeColor(AActor* Owner, UMaterialInterface* Base,
        const FName Name, const FLinearColor& Color)
    {
        if (!Owner || !Base) return nullptr;
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Owner, Name);
        if (Material) Material->SetVectorParameterValue(TEXT("Color"), Color);
        return Material;
    }

    void AddBoxLocal(UInstancedStaticMeshComponent* Target, const FVector& SiteCenter,
        const FVector& LocalCenter, const FVector& SizeCm, const float LocalYawDegrees = 0.0f)
    {
        if (!Target) return;
        Target->AddInstance(FTransform(
            FRotator(0.0f, FieldYawDegrees + LocalYawDegrees, 0.0f),
            SitePoint(SiteCenter, LocalCenter), SizeCm / 100.0f), true);
    }

    void AddUniformGroundedMesh(UInstancedStaticMeshComponent* Target, UStaticMesh* Mesh,
        const FVector& SiteCenter, const FVector& LocalCenter, const float DesiredHeightCm,
        const float LocalYawDegrees = 0.0f)
    {
        if (!Target || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.Z <= 1.0f) return;

        const float Scale = FMath::Clamp(DesiredHeightCm / NativeSize.Z, 0.25f, 4.0f);
        FVector Location = SitePoint(SiteCenter, LocalCenter);
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        Location.Z -= LocalBottom * Scale;
        Target->AddInstance(FTransform(
            FRotator(0.0f, FieldYawDegrees + LocalYawDegrees, 0.0f), Location, FVector(Scale)), true);
    }

    void AddFenceSegment(UInstancedStaticMeshComponent* Target, UStaticMesh* Mesh,
        const FVector& SiteCenter, const FVector& LocalCenter, const float DesiredLengthCm,
        const float LocalYawDegrees)
    {
        if (!Target || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) return;

        const bool bLongX = NativeSize.X >= NativeSize.Y;
        const float NativeLength = bLongX ? NativeSize.X : NativeSize.Y;
        const float ScaleXY = FMath::Clamp(DesiredLengthCm / NativeLength, 0.45f, 1.35f);
        const float HeightScale = FMath::Clamp(155.0f / NativeSize.Z, 0.40f, 1.60f);
        FVector Scale = bLongX
            ? FVector(ScaleXY, ScaleXY, HeightScale)
            : FVector(ScaleXY, ScaleXY, HeightScale);

        float Yaw = FieldYawDegrees + LocalYawDegrees;
        if (!bLongX) Yaw += 90.0f;
        FVector Location = SitePoint(SiteCenter, LocalCenter);
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        Location.Z -= LocalBottom * HeightScale;
        Target->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), Location, Scale), true);
    }

    void HideLegacyStadiumVisuals(UWorld& World)
    {
        for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
        {
            AOCWorldSectorOster* Sector = *It;
            if (!Sector) continue;

            // Keep collision/navigation contract alive while retiring the visible procedural blockout. The dedicated
            // site root below becomes the only player-facing stadium presentation owner on stadion-oster.
            for (const FName ComponentName : { FName(TEXT("StadiumGeometry")), FName(TEXT("StadiumDetails")) })
            {
                if (UInstancedStaticMeshComponent* Component = FindISM(Sector, ComponentName))
                {
                    Component->SetVisibility(false, true);
                    Component->SetHiddenInGame(true, true);
                }
            }
        }
    }
}

bool UOCR13StadiumSurfaceSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13StadiumSurfaceSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    // No delayed presentation repair here. The stadium is created once at world begin-play so it cannot visibly
    // rebuild itself several seconds after the player arrives.
    ApplyStadiumSurface(InWorld);
}

void UOCR13StadiumSurfaceSubsystem::ApplyStadiumSurface(UWorld& World)
{
    if (bApplied) return;

    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube || !BaseMaterial) return;

    // Existing imported Oster/rural assets. Missing optional meshes simply skip their layer; no giant primitive
    // fallback is substituted for houses, trees or fences.
    UStaticMesh* House01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var01.SM_House_Var01"));
    UStaticMesh* House02 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var02.SM_House_Var02"));
    UStaticMesh* Fence01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var01.SM_Fence_Var01"));
    UStaticMesh* Fence03 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var03.SM_Fence_Var03"));
    UStaticMesh* Tree01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var01.SM_Tree_Var01"));
    UStaticMesh* Tree04 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Tree_Var04.SM_Tree_Var04"));

    const FOCGeoReferencePoint StadiumGeo = FOCGeoReference::Stadium();
    const FVector Stadium = FOCGeoReference::ToLocalCm(StadiumGeo.Latitude, StadiumGeo.Longitude, 0.0);

    HideLegacyStadiumVisuals(World);

    AActor* SiteActor = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!SiteActor) return;
    SiteActor->SetReplicates(false);
    SiteActor->SetActorEnableCollision(false);
    SiteActor->Tags.Add(FName(TEXT("R13_StadionOsterAuthoritative")));

    USceneComponent* Root = NewObject<USceneComponent>(SiteActor, TEXT("R13_StadionOsterSiteRoot"));
    if (!Root)
    {
        SiteActor->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    SiteActor->SetRootComponent(Root);
    SiteActor->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UMaterialInstanceDynamic* GrassMat = MakeColor(SiteActor, BaseMaterial,
        TEXT("StadionOsterGrassMat"), FLinearColor(0.19f, 0.36f, 0.10f, 1.0f));
    UMaterialInstanceDynamic* TurfMat = MakeColor(SiteActor, BaseMaterial,
        TEXT("StadionOsterTurfMat"), FLinearColor(0.055f, 0.31f, 0.11f, 1.0f));
    UMaterialInstanceDynamic* LineMat = MakeColor(SiteActor, BaseMaterial,
        TEXT("StadionOsterLineMat"), FLinearColor(0.93f, 0.93f, 0.89f, 1.0f));
    UMaterialInstanceDynamic* MetalMat = MakeColor(SiteActor, BaseMaterial,
        TEXT("StadionOsterMetalMat"), FLinearColor(0.76f, 0.78f, 0.76f, 1.0f));
    UMaterialInstanceDynamic* DirtMat = MakeColor(SiteActor, BaseMaterial,
        TEXT("StadionOsterDirtMat"), FLinearColor(0.28f, 0.20f, 0.12f, 1.0f));
    UMaterialInstanceDynamic* BlueMat = MakeColor(SiteActor, BaseMaterial,
        TEXT("StadionOsterBlueMat"), FLinearColor(0.015f, 0.24f, 0.62f, 1.0f));
    UMaterialInstanceDynamic* YellowMat = MakeColor(SiteActor, BaseMaterial,
        TEXT("StadionOsterYellowMat"), FLinearColor(0.92f, 0.54f, 0.02f, 1.0f));

    UInstancedStaticMeshComponent* Grass = MakeISM(SiteActor, Root, Cube, GrassMat,
        TEXT("StadionOsterGrassApron"), false, false);
    UInstancedStaticMeshComponent* Turf = MakeISM(SiteActor, Root, Cube, TurfMat,
        TEXT("StadionOsterMainPitch"), false, false);
    UInstancedStaticMeshComponent* Lines = MakeISM(SiteActor, Root, Cube, LineMat,
        TEXT("StadionOsterPitchLines"), false, false);
    UInstancedStaticMeshComponent* SportsMetal = MakeISM(SiteActor, Root, Cube, MetalMat,
        TEXT("StadionOsterSportsMetal"), true, true);
    UInstancedStaticMeshComponent* Paths = MakeISM(SiteActor, Root, Cube, DirtMat,
        TEXT("StadionOsterFootpaths"), false, false);
    UInstancedStaticMeshComponent* SignBlue = MakeISM(SiteActor, Root, Cube, BlueMat,
        TEXT("StadionOsterEntranceBlue"), true, true);
    UInstancedStaticMeshComponent* SignYellow = MakeISM(SiteActor, Root, Cube, YellowMat,
        TEXT("StadionOsterEntranceYellow"), false, true);

    UInstancedStaticMeshComponent* Houses01 = MakeISM(SiteActor, Root, House01, nullptr,
        TEXT("StadionOsterHouses01"), false, true, 70000);
    UInstancedStaticMeshComponent* Houses02 = MakeISM(SiteActor, Root, House02, nullptr,
        TEXT("StadionOsterHouses02"), false, true, 70000);
    UInstancedStaticMeshComponent* Trees01 = MakeISM(SiteActor, Root, Tree01, nullptr,
        TEXT("StadionOsterTrees01"), false, true, 85000);
    UInstancedStaticMeshComponent* Trees04 = MakeISM(SiteActor, Root, Tree04, nullptr,
        TEXT("StadionOsterTrees04"), false, true, 85000);
    UInstancedStaticMeshComponent* Fences01 = MakeISM(SiteActor, Root, Fence01, nullptr,
        TEXT("StadionOsterFences01"), false, true, 60000);
    UInstancedStaticMeshComponent* Fences03 = MakeISM(SiteActor, Root, Fence03, nullptr,
        TEXT("StadionOsterFences03"), false, true, 60000);

    // Site apron keeps the photographed open grassy character around the formal pitch. The underlying source ground
    // remains collision-authoritative; this is deliberately presentation-only to avoid a duplicate floor.
    AddBoxLocal(Grass, Stadium, FVector(0.0f, 0.0f, 5.0f), FVector(15400.0f, 11200.0f, 6.0f));
    AddBoxLocal(Turf, Stadium, FVector(0.0f, 0.0f, 10.0f), FVector(FieldLengthCm, FieldWidthCm, 8.0f));

    constexpr float LineZ = 16.0f;
    constexpr float LineWidth = 12.0f;
    const float HalfLength = FieldLengthCm * 0.5f;
    const float HalfWidth = FieldWidthCm * 0.5f;
    AddBoxLocal(Lines, Stadium, FVector(0.0f, HalfWidth, LineZ), FVector(FieldLengthCm, LineWidth, 3.0f));
    AddBoxLocal(Lines, Stadium, FVector(0.0f,-HalfWidth, LineZ), FVector(FieldLengthCm, LineWidth, 3.0f));
    AddBoxLocal(Lines, Stadium, FVector( HalfLength,0.0f,LineZ), FVector(LineWidth, FieldWidthCm, 3.0f));
    AddBoxLocal(Lines, Stadium, FVector(-HalfLength,0.0f,LineZ), FVector(LineWidth, FieldWidthCm, 3.0f));
    AddBoxLocal(Lines, Stadium, FVector(0.0f,0.0f,LineZ), FVector(LineWidth, FieldWidthCm, 3.0f));

    for (const float Side : { -1.0f, 1.0f })
    {
        const float GoalX = Side * HalfLength;
        const float PenaltyX = Side * (HalfLength - 1650.0f);
        AddBoxLocal(Lines, Stadium, FVector(PenaltyX,0.0f,LineZ), FVector(LineWidth, 4030.0f, 3.0f));
        AddBoxLocal(Lines, Stadium, FVector((GoalX + PenaltyX) * 0.5f, 2015.0f, LineZ),
            FVector(1650.0f, LineWidth, 3.0f));
        AddBoxLocal(Lines, Stadium, FVector((GoalX + PenaltyX) * 0.5f,-2015.0f, LineZ),
            FVector(1650.0f, LineWidth, 3.0f));

        // 7.32 x 2.44 m full-size goal frame. Netting can be replaced by a dedicated mesh later without moving posts.
        AddBoxLocal(SportsMetal, Stadium, FVector(GoalX, -366.0f, 122.0f), FVector(10.0f, 10.0f, 244.0f));
        AddBoxLocal(SportsMetal, Stadium, FVector(GoalX,  366.0f, 122.0f), FVector(10.0f, 10.0f, 244.0f));
        AddBoxLocal(SportsMetal, Stadium, FVector(GoalX, 0.0f, 244.0f), FVector(10.0f, 742.0f, 10.0f));
    }

    // Photo pack shows smaller training goals and mixed open exercise space on the residential side of the site.
    for (const FVector& LocalGoal : { FVector(-3300.0f, 4300.0f, 0.0f), FVector(2500.0f, 4350.0f, 0.0f) })
    {
        AddBoxLocal(SportsMetal, Stadium, LocalGoal + FVector(0.0f,-150.0f,90.0f), FVector(10.0f,10.0f,180.0f));
        AddBoxLocal(SportsMetal, Stadium, LocalGoal + FVector(0.0f, 150.0f,90.0f), FVector(10.0f,10.0f,180.0f));
        AddBoxLocal(SportsMetal, Stadium, LocalGoal + FVector(0.0f,0.0f,180.0f), FVector(10.0f,310.0f,10.0f));
    }

    // Basketball posts visible repeatedly in the 2020 references. Boards use simple geometry for now; their exact
    // texture/art is a dedicated prop task, not a reason to invent an unrelated arena asset.
    for (const FVector& LocalPost : { FVector(-4200.0f, 4700.0f, 0.0f), FVector(700.0f, 4750.0f, 0.0f) })
    {
        AddBoxLocal(SportsMetal, Stadium, LocalPost + FVector(0.0f,0.0f,150.0f), FVector(14.0f,14.0f,300.0f));
        AddBoxLocal(SignBlue, Stadium, LocalPost + FVector(0.0f,0.0f,300.0f), FVector(22.0f,180.0f,110.0f));
        AddBoxLocal(SignYellow, Stadium, LocalPost + FVector(-15.0f,0.0f,300.0f), FVector(5.0f,160.0f,90.0f));
    }

    // Outdoor workout bars grouped rather than uniformly scattered. This matches the photographed local character.
    for (int32 Index = 0; Index < 5; ++Index)
    {
        const FVector Base(-900.0f + Index * 240.0f, 4650.0f + (Index % 2) * 160.0f, 0.0f);
        AddBoxLocal(SportsMetal, Stadium, Base + FVector(0.0f,-55.0f,105.0f), FVector(9.0f,9.0f,210.0f));
        AddBoxLocal(SportsMetal, Stadium, Base + FVector(0.0f, 55.0f,105.0f), FVector(9.0f,9.0f,210.0f));
        AddBoxLocal(SportsMetal, Stadium, Base + FVector(0.0f,0.0f,205.0f), FVector(9.0f,120.0f,9.0f));
    }

    // Narrow dirt footpaths from the photo set; deliberately not asphalt roads.
    AddBoxLocal(Paths, Stadium, FVector(0.0f, 5550.0f, 8.0f), FVector(9300.0f, 115.0f, 4.0f), -3.0f);
    AddBoxLocal(Paths, Stadium, FVector(-4700.0f, 4550.0f, 8.0f), FVector(3300.0f, 105.0f, 4.0f), 62.0f);

    // 2025 entrance landmark: recognizable blue vertical pylon plus blue/yellow horizontal sign. Lettering is kept
    // out of this first compile-safe pass; its prop/model will be refined against reference 07 without moving root.
    AddBoxLocal(SignBlue, Stadium, FVector(-6550.0f, 5050.0f, 170.0f), FVector(70.0f, 95.0f, 340.0f));
    AddBoxLocal(SignBlue, Stadium, FVector(-5650.0f, 5050.0f, 105.0f), FVector(1800.0f, 80.0f, 210.0f));
    AddBoxLocal(SignYellow, Stadium, FVector(-5650.0f, 5006.0f, 105.0f), FVector(1650.0f, 6.0f, 26.0f));

    // Irregular tree belt from the photos. Real imported tree meshes only; no primitive-tree fallback.
    const FVector TreeOffsets[] =
    {
        {-6900,-4550,0}, {-6100,-5200,0}, {-5000,-5600,0}, {-3650,-5750,0}, {-2100,-5900,0},
        {-300,-6000,0}, {1750,-5850,0}, {3500,-5700,0}, {5200,-5400,0}, {6800,-4600,0},
        {7200,-2700,0}, {7350,-500,0}, {7200,1750,0}, {7000,3500,0},
        {-7000,-2500,0}, {-7250,-300,0}, {-7100,1900,0}, {-6850,3500,0}
    };
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(TreeOffsets); ++Index)
    {
        UInstancedStaticMeshComponent* Target = (Index % 3 == 0 && Trees04) ? Trees04 : Trees01;
        UStaticMesh* Mesh = (Target == Trees04) ? Tree04 : Tree01;
        AddUniformGroundedMesh(Target, Mesh, Stadium, TreeOffsets[Index], 1350.0f + (Index % 5) * 150.0f,
            static_cast<float>((Index * 47) % 360));
    }

    // Residential edge seen behind the goals/training area. These are existing real house assets, not new blockout.
    const FVector HouseOffsets[] =
    {
        {-5200, 6500, 0}, {-3100, 6750, 0}, {-900, 6600, 0}, {1600, 6900, 0}, {3900, 6650, 0}
    };
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(HouseOffsets); ++Index)
    {
        UInstancedStaticMeshComponent* Target = (Index % 2 == 0 && Houses01) ? Houses01 : Houses02;
        UStaticMesh* Mesh = (Target == Houses01) ? House01 : House02;
        AddUniformGroundedMesh(Target, Mesh, Stadium, HouseOffsets[Index], 720.0f + (Index % 3) * 70.0f,
            5.0f + Index * 4.0f);
    }

    // Short real fence runs along the residential edge. Do not stretch one mesh across the whole boundary.
    for (int32 Index = 0; Index < 12; ++Index)
    {
        const float X = -5500.0f + Index * 1000.0f;
        UInstancedStaticMeshComponent* Target = (Index % 4 == 0 && Fences03) ? Fences03 : Fences01;
        UStaticMesh* Mesh = (Target == Fences03) ? Fence03 : Fence01;
        AddFenceSegment(Target, Mesh, Stadium, FVector(X, 5850.0f, 0.0f), 900.0f, 0.0f);
    }

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("Stadion Oster authoritative site created: geo=(%.6f, %.6f), local=(%.1f, %.1f), yaw=%.1f, field=%.0fx%.0f cm, radius=%.0f cm. Legacy stadium visuals hidden; reference pack is REFERENCE_PHOTOS/stadion_oster."),
        StadiumGeo.Latitude, StadiumGeo.Longitude, Stadium.X, Stadium.Y, FieldYawDegrees,
        FieldLengthCm, FieldWidthCm, SiteRadiusCm);
}
