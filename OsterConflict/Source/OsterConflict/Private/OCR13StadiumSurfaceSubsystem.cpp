#include "OCR13StadiumSurfaceSubsystem.h"

#include "OCGameMode.h"
#include "OCGeoReference.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
    // Macro placement is hard-georeferenced. Internal offsets remain photo/map-derived local data so the site can be
    // refined without moving the whole stadium. Long-axis yaw comes from the user's north-up landmark maps.
    constexpr float FieldYawDegrees = -21.5f;
    constexpr float FieldLengthCm = 10500.0f;
    constexpr float FieldWidthCm = 6800.0f;
    constexpr float SiteRadiusCm = 9000.0f;
    constexpr float LegacyFenceCleanupRadiusCm = 8800.0f;

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

    void RemoveInstancesNear(UInstancedStaticMeshComponent* Component, const FVector& Center, const float RadiusCm)
    {
        if (!Component) return;

        TArray<int32> ToRemove;
        FTransform Transform;
        const float RadiusSq = FMath::Square(RadiusCm);
        for (int32 Index = 0; Index < Component->GetInstanceCount(); ++Index)
        {
            if (Component->GetInstanceTransform(Index, Transform, true)
                && FVector::DistSquared2D(Transform.GetLocation(), Center) <= RadiusSq)
            {
                ToRemove.Add(Index);
            }
        }

        for (int32 RemoveIndex = ToRemove.Num() - 1; RemoveIndex >= 0; --RemoveIndex)
        {
            Component->RemoveInstance(ToRemove[RemoveIndex]);
        }
        Component->MarkRenderStateDirty();
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
        UStaticMesh* Mesh = Target->GetStaticMesh();
        if (!Mesh) return;

        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f) return;

        const bool bNativeLongAxisY = NativeSize.Y > NativeSize.X * 1.05f;
        FVector Scale(1.0f);
        float Yaw = FieldYawDegrees + LocalYawDegrees;
        if (bNativeLongAxisY)
        {
            Scale.X = SizeCm.Y / NativeSize.X;
            Scale.Y = SizeCm.X / NativeSize.Y;
            Yaw -= 90.0f;
        }
        else
        {
            Scale.X = SizeCm.X / NativeSize.X;
            Scale.Y = SizeCm.Y / NativeSize.Y;
        }
        Scale.Z = NativeSize.Z > 1.0f
            ? FMath::Max(0.05f, SizeCm.Z / NativeSize.Z)
            : 1.0f;

        const FQuat RotationQuat = FRotator(0.0f, Yaw, 0.0f).Quaternion();
        const FVector ScaledBoundsOrigin(
            Bounds.Origin.X * Scale.X,
            Bounds.Origin.Y * Scale.Y,
            Bounds.Origin.Z * Scale.Z);
        const FVector Location = SitePoint(SiteCenter, LocalCenter) - RotationQuat.RotateVector(ScaledBoundsOrigin);
        Target->AddInstance(FTransform(RotationQuat, Location, Scale), true);
    }

    void AddAuthoredSurfaceLocal(UInstancedStaticMeshComponent* Target, UStaticMesh* Mesh,
        const FVector& SiteCenter, const FVector& LocalCenter, const FVector& DesiredSizeCm,
        const float LocalYawDegrees = 0.0f)
    {
        if (!Target || !Mesh) return;

        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f) return;

        const bool bNativeLongAxisY = NativeSize.Y > NativeSize.X * 1.05f;
        FVector Scale(1.0f);
        float Yaw = FieldYawDegrees + LocalYawDegrees;
        if (bNativeLongAxisY)
        {
            Scale.X = DesiredSizeCm.Y / NativeSize.X;
            Scale.Y = DesiredSizeCm.X / NativeSize.Y;
            Yaw -= 90.0f;
        }
        else
        {
            Scale.X = DesiredSizeCm.X / NativeSize.X;
            Scale.Y = DesiredSizeCm.Y / NativeSize.Y;
        }
        Scale.Z = NativeSize.Z > 1.0f
            ? FMath::Max(0.05f, DesiredSizeCm.Z / NativeSize.Z)
            : 1.0f;

        const FQuat RotationQuat = FRotator(0.0f, Yaw, 0.0f).Quaternion();
        const FVector ScaledBoundsOrigin(
            Bounds.Origin.X * Scale.X,
            Bounds.Origin.Y * Scale.Y,
            Bounds.Origin.Z * Scale.Z);
        const FVector Location = SitePoint(SiteCenter, LocalCenter) - RotationQuat.RotateVector(ScaledBoundsOrigin);
        Target->AddInstance(FTransform(RotationQuat, Location, Scale), true);
    }

    void AddPathPolyline(UInstancedStaticMeshComponent* Target, const FVector& SiteCenter,
        const TArray<FVector>& LocalPoints, const float WidthCm, const float ZCm)
    {
        if (!Target || LocalPoints.Num() < 2) return;

        for (int32 Index = 1; Index < LocalPoints.Num(); ++Index)
        {
            const FVector A = LocalPoints[Index - 1];
            const FVector B = LocalPoints[Index];
            const FVector Delta = B - A;
            const float Length = FVector2D(Delta.X, Delta.Y).Size();
            if (Length <= 1.0f) continue;

            FVector Mid = (A + B) * 0.5f;
            Mid.Z = ZCm;
            const float LocalYaw = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
            AddBoxLocal(Target, SiteCenter, Mid, FVector(Length + 24.0f, WidthCm, 4.0f), LocalYaw);
        }
    }

    void AddAuthoredPathPolyline(UInstancedStaticMeshComponent* Target, UStaticMesh* Mesh,
        const FVector& SiteCenter, const TArray<FVector>& LocalPoints, const float WidthCm, const float ZCm)
    {
        if (!Target || !Mesh || LocalPoints.Num() < 2) return;

        for (int32 Index = 1; Index < LocalPoints.Num(); ++Index)
        {
            const FVector A = LocalPoints[Index - 1];
            const FVector B = LocalPoints[Index];
            const FVector Delta = B - A;
            const float Length = FVector2D(Delta.X, Delta.Y).Size();
            if (Length <= 1.0f) continue;

            FVector Mid = (A + B) * 0.5f;
            Mid.Z = ZCm;
            const float LocalYaw = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
            AddAuthoredSurfaceLocal(Target, Mesh, SiteCenter, Mid,
                FVector(Length + 24.0f, WidthCm, 4.0f), LocalYaw);
        }
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
        const FVector Scale(ScaleXY, ScaleXY, HeightScale);

        float Yaw = FieldYawDegrees + LocalYawDegrees;
        if (!bLongX) Yaw += 90.0f;
        FVector Location = SitePoint(SiteCenter, LocalCenter);
        const float LocalBottom = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        Location.Z -= LocalBottom * HeightScale;
        Target->AddInstance(FTransform(FRotator(0.0f, Yaw, 0.0f), Location, Scale), true);
    }

    void AddEntranceText(AActor* SiteActor, USceneComponent* Root, const FVector& Stadium)
    {
        if (!SiteActor || !Root) return;

        UTextRenderComponent* EntranceText = NewObject<UTextRenderComponent>(SiteActor, TEXT("StadionOsterEntranceText"));
        if (!EntranceText) return;

        EntranceText->SetupAttachment(Root);
        EntranceText->SetMobility(EComponentMobility::Static);
        EntranceText->SetText(FText::FromString(TEXT("СТАДІОН ОСТЕР")));
        EntranceText->SetWorldSize(72.0f);
        EntranceText->SetTextRenderColor(FColor(244, 205, 36));
        EntranceText->SetWorldLocation(SitePoint(Stadium, FVector(-6170.0f, 5002.0f, 92.0f)));
        EntranceText->SetWorldRotation(FRotator(0.0f, FieldYawDegrees - 90.0f, 0.0f));
        EntranceText->SetCastShadow(false);
        EntranceText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        EntranceText->SetGenerateOverlapEvents(false);
        EntranceText->SetCanEverAffectNavigation(false);
        SiteActor->AddInstanceComponent(EntranceText);
        EntranceText->RegisterComponent();
    }

    void HideLegacyStadiumVisuals(UWorld& World, const FVector& Stadium)
    {
        for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
        {
            AOCWorldSectorOster* Sector = *It;
            if (!Sector) continue;

            for (const FName ComponentName : { FName(TEXT("StadiumGeometry")), FName(TEXT("StadiumDetails")) })
            {
                if (UInstancedStaticMeshComponent* Component = FindISM(Sector, ComponentName))
                {
                    Component->SetVisibility(false, true);
                    Component->SetHiddenInGame(true, true);
                }
            }

            RemoveInstancesNear(FindISM(Sector, FName(TEXT("Fences"))), Stadium, LegacyFenceCleanupRadiusCm);
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

    ApplyStadiumSurface(InWorld);
}

void UOCR13StadiumSurfaceSubsystem::ApplyStadiumSurface(UWorld& World)
{
    if (bApplied) return;

    UStaticMesh* PitchSurfaceMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Plane_1x1.SM_Plane_1x1"));
    UStaticMesh* StadiumBarMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Meshes/SM_Curb_1.SM_Curb_1"));
    UStaticMesh* StadiumSignMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_Pack_V1/Mesh/SM_Sign_1.SM_Sign_1"));
    UMaterialInterface* TurfMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Grass_Inst.M_Grass_Inst"));
    UMaterialInterface* ColorBaseMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_Pack_V1/Materials/Instances/M_Color_1_Inst.M_Color_1_Inst"));
    UMaterialInterface* SportsMetalMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Metal_3_Inst.M_Metal_3_Inst"));

    UStaticMesh* House01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var01.SM_House_Var01"));
    UStaticMesh* House02 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var02.SM_House_Var02"));
    UStaticMesh* Fence01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var01.SM_Fence_Var01"));
    UStaticMesh* Fence03 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var03.SM_Fence_Var03"));
    UStaticMesh* Tree01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/KiteDemo/Environments/Trees/HillTree_02/HillTree_02.HillTree_02"));
    UStaticMesh* Tree04 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01"));
    UStaticMesh* TrackSurfaceMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Ground_01/SM_Urb_Roa_Ground_01.SM_Urb_Roa_Ground_01"));
    UStaticMesh* FootpathSurfaceMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Sidewalk_01/SM_Urb_Roa_Sidewalk_01.SM_Urb_Roa_Sidewalk_01"));
    if (!PitchSurfaceMesh || !StadiumBarMesh || !StadiumSignMesh
        || !TurfMaterial || !ColorBaseMaterial || !SportsMetalMaterial
        || !TrackSurfaceMesh || !FootpathSurfaceMesh)
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_STADIUM_AUTHORED_SURFACE_CONTENT_GAP pitch_loaded=%d bar_loaded=%d sign_loaded=%d turf_material_loaded=%d color_material_loaded=%d metal_material_loaded=%d track_loaded=%d footpath_loaded=%d gate_k_complete=0"),
            PitchSurfaceMesh ? 1 : 0,
            StadiumBarMesh ? 1 : 0,
            StadiumSignMesh ? 1 : 0,
            TurfMaterial ? 1 : 0,
            ColorBaseMaterial ? 1 : 0,
            SportsMetalMaterial ? 1 : 0,
            TrackSurfaceMesh ? 1 : 0,
            FootpathSurfaceMesh ? 1 : 0);
        return;
    }

    const FOCGeoReferencePoint StadiumGeo = FOCGeoReference::Stadium();
    FVector Stadium = FOCGeoReference::ToLocalCm(StadiumGeo.Latitude, StadiumGeo.Longitude, 0.0);

    // Georeference provides XY. Runtime terrain owns Z. The old Z=0 assumption could put the entire
    // stadium slab above/below the actual ground, making the site look like a floating blockout.
    FHitResult GroundHit;
    FCollisionQueryParams GroundParams(SCENE_QUERY_STAT(OCStadiumGround), false);
    const FVector GroundStart(Stadium.X, Stadium.Y, 10000.0f);
    const FVector GroundEnd(Stadium.X, Stadium.Y, -5000.0f);
    if (World.LineTraceSingleByChannel(GroundHit, GroundStart, GroundEnd, ECC_Visibility, GroundParams))
    {
        Stadium.Z = GroundHit.ImpactPoint.Z;
    }

    HideLegacyStadiumVisuals(World, Stadium);

    AActor* SiteActor = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!SiteActor) return;
    SiteActor->SetReplicates(false);
    SiteActor->SetActorEnableCollision(true);
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

    UMaterialInstanceDynamic* LineMat = MakeColor(SiteActor, ColorBaseMaterial,
        TEXT("StadionOsterLineMat"), FLinearColor(0.93f, 0.93f, 0.89f, 1.0f));
    UMaterialInstanceDynamic* BlueMat = MakeColor(SiteActor, ColorBaseMaterial,
        TEXT("StadionOsterBlueMat"), FLinearColor(0.015f, 0.24f, 0.62f, 1.0f));
    UMaterialInstanceDynamic* YellowMat = MakeColor(SiteActor, ColorBaseMaterial,
        TEXT("StadionOsterYellowMat"), FLinearColor(0.92f, 0.54f, 0.02f, 1.0f));

    UInstancedStaticMeshComponent* Turf = MakeISM(SiteActor, Root, PitchSurfaceMesh, TurfMaterial,
        TEXT("StadionOsterMainPitch"), false, false);
    UInstancedStaticMeshComponent* Track = MakeISM(SiteActor, Root, TrackSurfaceMesh, nullptr,
        TEXT("StadionOsterRunningSurface"), false, false);
    UInstancedStaticMeshComponent* Lines = MakeISM(SiteActor, Root, StadiumBarMesh, LineMat,
        TEXT("StadionOsterPitchLines"), false, false);
    UInstancedStaticMeshComponent* SportsMetal = MakeISM(SiteActor, Root, StadiumBarMesh, SportsMetalMaterial,
        TEXT("StadionOsterSportsMetal"), true, true);
    UInstancedStaticMeshComponent* Paths = MakeISM(SiteActor, Root, FootpathSurfaceMesh, nullptr,
        TEXT("StadionOsterFootpaths"), false, false);
    UInstancedStaticMeshComponent* SignBlue = MakeISM(SiteActor, Root, StadiumSignMesh, BlueMat,
        TEXT("StadionOsterEntranceBlue"), true, true);
    UInstancedStaticMeshComponent* SignYellow = MakeISM(SiteActor, Root, StadiumSignMesh, YellowMat,
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
        TEXT("StadionOsterFences01"), true, true, 60000);
    UInstancedStaticMeshComponent* Fences03 = MakeISM(SiteActor, Root, Fence03, nullptr,
        TEXT("StadionOsterFences03"), true, true, 60000);

    // The formal pitch now uses a tracked authored plane + grass material. Terrain/foliage remains visible
    // outside the pitch, avoiding the giant Engine BasicShape apron rejected in the playtest.
    AddBoxLocal(Turf, Stadium, FVector(0.0f, 0.0f, 12.0f), FVector(FieldLengthCm, FieldWidthCm, 8.0f));

    constexpr float TrackZ = 10.0f;
    AddAuthoredSurfaceLocal(Track, TrackSurfaceMesh, Stadium,
        FVector(0.0f, 3750.0f, TrackZ), FVector(11000.0f, 500.0f, 5.0f));
    AddAuthoredSurfaceLocal(Track, TrackSurfaceMesh, Stadium,
        FVector(0.0f,-3750.0f, TrackZ), FVector(11000.0f, 500.0f, 5.0f));
    AddAuthoredSurfaceLocal(Track, TrackSurfaceMesh, Stadium,
        FVector(5525.0f,0.0f, TrackZ), FVector(550.0f, 7000.0f, 5.0f));
    AddAuthoredSurfaceLocal(Track, TrackSurfaceMesh, Stadium,
        FVector(-5525.0f,0.0f,TrackZ), FVector(550.0f, 7000.0f, 5.0f));

    constexpr float LineZ = 18.0f;
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

        AddBoxLocal(SportsMetal, Stadium, FVector(GoalX, -366.0f, 122.0f), FVector(10.0f, 10.0f, 244.0f));
        AddBoxLocal(SportsMetal, Stadium, FVector(GoalX,  366.0f, 122.0f), FVector(10.0f, 10.0f, 244.0f));
        AddBoxLocal(SportsMetal, Stadium, FVector(GoalX, 0.0f, 244.0f), FVector(10.0f, 742.0f, 10.0f));
    }

    for (const FVector& LocalGoal : { FVector(-3300.0f, 4300.0f, 0.0f), FVector(2500.0f, 4350.0f, 0.0f) })
    {
        AddBoxLocal(SportsMetal, Stadium, LocalGoal + FVector(0.0f,-150.0f,90.0f), FVector(10.0f,10.0f,180.0f));
        AddBoxLocal(SportsMetal, Stadium, LocalGoal + FVector(0.0f, 150.0f,90.0f), FVector(10.0f,10.0f,180.0f));
        AddBoxLocal(SportsMetal, Stadium, LocalGoal + FVector(0.0f,0.0f,180.0f), FVector(10.0f,310.0f,10.0f));
    }

    for (const FVector& LocalPost : { FVector(-4200.0f, 4700.0f, 0.0f), FVector(700.0f, 4750.0f, 0.0f) })
    {
        AddBoxLocal(SportsMetal, Stadium, LocalPost + FVector(0.0f,0.0f,150.0f), FVector(14.0f,14.0f,300.0f));
        AddBoxLocal(SignBlue, Stadium, LocalPost + FVector(0.0f,0.0f,300.0f), FVector(22.0f,180.0f,110.0f));
        AddBoxLocal(SignYellow, Stadium, LocalPost + FVector(-15.0f,0.0f,300.0f), FVector(5.0f,160.0f,90.0f));
    }

    for (int32 Index = 0; Index < 5; ++Index)
    {
        const FVector Base(-900.0f + Index * 240.0f, 4650.0f + (Index % 2) * 160.0f, 0.0f);
        AddBoxLocal(SportsMetal, Stadium, Base + FVector(0.0f,-55.0f,105.0f), FVector(9.0f,9.0f,210.0f));
        AddBoxLocal(SportsMetal, Stadium, Base + FVector(0.0f, 55.0f,105.0f), FVector(9.0f,9.0f,210.0f));
        AddBoxLocal(SportsMetal, Stadium, Base + FVector(0.0f,0.0f,205.0f), FVector(9.0f,120.0f,9.0f));
    }

    const TArray<FVector> NorthPath =
    {
        FVector(-4700.0f, 5420.0f, 0.0f), FVector(-3000.0f, 5510.0f, 0.0f),
        FVector(-1200.0f, 5580.0f, 0.0f), FVector(700.0f, 5530.0f, 0.0f),
        FVector(2500.0f, 5460.0f, 0.0f), FVector(4550.0f, 5560.0f, 0.0f)
    };
    AddAuthoredPathPolyline(Paths, FootpathSurfaceMesh, Stadium, NorthPath, 112.0f, 8.0f);

    const TArray<FVector> EntrancePath =
    {
        FVector(-6900.0f, 5100.0f, 0.0f), FVector(-6200.0f, 4850.0f, 0.0f),
        FVector(-5650.0f, 4350.0f, 0.0f), FVector(-5200.0f, 3650.0f, 0.0f),
        FVector(-4900.0f, 2850.0f, 0.0f)
    };
    AddAuthoredPathPolyline(Paths, FootpathSurfaceMesh, Stadium, EntrancePath, 104.0f, 8.0f);

    AddBoxLocal(SignBlue, Stadium, FVector(-6550.0f, 5050.0f, 170.0f), FVector(70.0f, 95.0f, 340.0f));
    AddBoxLocal(SignBlue, Stadium, FVector(-5650.0f, 5050.0f, 105.0f), FVector(1800.0f, 80.0f, 210.0f));
    AddBoxLocal(SignYellow, Stadium, FVector(-5650.0f, 5006.0f, 105.0f), FVector(1650.0f, 6.0f, 26.0f));
    AddEntranceText(SiteActor, Root, Stadium);

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

    for (int32 Index = 0; Index < 12; ++Index)
    {
        const float X = -5500.0f + Index * 1000.0f;
        UInstancedStaticMeshComponent* Target = (Index % 4 == 0 && Fences03) ? Fences03 : Fences01;
        UStaticMesh* Mesh = (Target == Fences03) ? Fence03 : Fence01;
        AddFenceSegment(Target, Mesh, Stadium, FVector(X, 5850.0f, 0.0f), 900.0f, 0.0f);
    }

    bApplied = true;
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_STADIUM_TREE_INTAKE_WIRED deciduous=HillTree_02 pine=ScotsPineTall_01 primary_authoring=1 late_mutation=0 runtime_acceptance=0"));
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_STADIUM_SURFACE_PARTIAL_AUTHORED_READY pitch_mesh=SM_Plane_1x1 running_mesh=SM_Urb_Roa_Ground_01 line_mesh=SM_Curb_1 sports_mesh=SM_Curb_1 footpath_mesh=SM_Urb_Roa_Sidewalk_01 entrance_mesh=SM_Sign_1 tree_families=HillTree_02,ScotsPineTall_01 authored_surface_families=7 remaining_basicshape_families=0 bounds_aware_box_fit=1 gate_k_complete=0 runtime_acceptance=0"));
    UE_LOG(LogTemp, Display,
        TEXT("Stadion Oster authoritative site created: geo=(%.6f, %.6f), local=(%.1f, %.1f, %.1f), yaw=%.1f, field=%.0fx%.0f cm, radius=%.0f cm. Runtime ground Z sampled; giant grass apron removed; legacy stadium visuals/fences retired; references=REFERENCE_PHOTOS/stadion_oster."),
        StadiumGeo.Latitude, StadiumGeo.Longitude, Stadium.X, Stadium.Y, Stadium.Z, FieldYawDegrees,
        FieldLengthCm, FieldWidthCm, SiteRadiusCm);
}
