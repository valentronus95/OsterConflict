#include "OCMuseumCoreRecoverySubsystem.h"

#include "OCGameMode.h"
#include "OCR138MuseumInteractiveArchitectureSubsystem.h"
#include "OCR139MuseumMainDoorReplacementSubsystem.h"
#include "OCR140MuseumFacadeDetailSubsystem.h"
#include "OCR141MuseumWindowReplacementSubsystem.h"
#include "OCR142MuseumEntranceDetailSubsystem.h"
#include "OCR143MuseumSiteVegetationSubsystem.h"
#include "OCR144MuseumRearExteriorDetailSubsystem.h"
#include "OCR145MuseumTreeLayoutSubsystem.h"
#include "OCTeamSpawnPoint.h"
#include "OCWorldSectorOster.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float RecoveryDelaySeconds = 0.65f;
    constexpr float MuseumNoSpawnRadiusCm = 3000.0f;
    constexpr float MuseumNearbyBaseRadiusCm = 6000.0f;

    const FName MuseumPrototypeTag(TEXT("R137_MuseumPhotoModel"));
    const FName MuseumArchitectureTag(TEXT("R138_MuseumHighFidelityArchitecture"));
    const FName RecoveryCarrierTag(TEXT("PASS35_MuseumRecoveryCarrier"));

    int32 CountActorsWithTag(UWorld& World, const FName Tag)
    {
        int32 Count = 0;
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            const AActor* Actor = *It;
            if (Actor && !Actor->IsActorBeingDestroyed() && Actor->ActorHasTag(Tag)) ++Count;
        }
        return Count;
    }

    AActor* FindActorWithTag(UWorld& World, const FName Tag)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (Actor && !Actor->IsActorBeingDestroyed() && Actor->ActorHasTag(Tag)) return Actor;
        }
        return nullptr;
    }

    UMaterialInstanceDynamic* MakeRecoveryMID(
        AActor* Owner,
        UMaterialInterface* Base,
        const FName Name,
        const FLinearColor& Color)
    {
        if (!Owner || !Base) return nullptr;
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Owner, Name);
        if (Material) Material->SetVectorParameterValue(TEXT("Color"), Color);
        return Material;
    }

    UStaticMeshComponent* AddRecoveryBox(
        AActor* Owner,
        USceneComponent* Root,
        UStaticMesh* Cube,
        UMaterialInterface* Material,
        const FName Name,
        const FVector& Location,
        const FVector& SizeCm,
        const FRotator& Rotation = FRotator::ZeroRotator,
        const bool bCollision = false)
    {
        if (!Owner || !Root || !Cube || SizeCm.GetMin() <= 0.0f) return nullptr;
        UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Cube);
        if (Material) Component->SetMaterial(0, Material);
        Component->SetRelativeLocation(Location);
        Component->SetRelativeRotation(Rotation);
        Component->SetRelativeScale3D(SizeCm / 100.0f);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(bCollision);
        Component->SetCastShadow(false);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    UStaticMeshComponent* AddRecoveryFittedMesh(
        AActor* Owner,
        USceneComponent* Root,
        UStaticMesh* Mesh,
        UMaterialInterface* Material,
        const FName Name,
        const FVector& Center,
        const FVector& DesiredSizeCm,
        const float YawDegrees = 0.0f)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) return nullptr;

        const FVector Scale(
            DesiredSizeCm.X / NativeSize.X,
            DesiredSizeCm.Y / NativeSize.Y,
            DesiredSizeCm.Z / NativeSize.Z);
        const FQuat Rotation = FRotator(0.0f, YawDegrees, 0.0f).Quaternion();
        const FVector Location = Center - Rotation.RotateVector(Bounds.Origin * Scale);

        UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material) Component->SetMaterial(0, Material);
        Component->SetRelativeLocation(Location);
        Component->SetRelativeRotation(Rotation.Rotator());
        Component->SetRelativeScale3D(Scale);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(false);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void BuildRecoveryPresentation(AActor& Carrier, USceneComponent& Root)
    {
        UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
            TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
        if (!Cube || !Basic) return;

        UMaterialInstanceDynamic* Plinth = MakeRecoveryMID(
            &Carrier, Basic, TEXT("PASS35MuseumMID_Plinth"), FLinearColor(0.07f, 0.065f, 0.06f, 1.0f));
        UMaterialInstanceDynamic* RoofFallback = MakeRecoveryMID(
            &Carrier, Basic, TEXT("PASS35MuseumMID_Roof"), FLinearColor(0.16f, 0.19f, 0.20f, 1.0f));

        AddRecoveryBox(
            &Carrier, &Root, Cube, Plinth, TEXT("PASS35Museum_RecoveryPlinth"),
            FVector(0.0f, 0.0f, 35.0f), FVector(1760.0f, 900.0f, 70.0f), FRotator::ZeroRotator, true);

        UStaticMesh* RoofMesh = LoadObject<UStaticMesh>(nullptr,
            TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Roof_Both_Ends_4m.Roof_Both_Ends_4m"));
        UMaterialInterface* MetalRoof = LoadObject<UMaterialInterface>(nullptr,
            TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Metal_Roof.Metal_Roof"));

        if (RoofMesh)
        {
            AddRecoveryFittedMesh(
                &Carrier, &Root, RoofMesh, MetalRoof ? MetalRoof : RoofFallback,
                TEXT("PASS35Museum_RecoveryRoofMain"),
                FVector(0.0f, 0.0f, 505.0f), FVector(1840.0f, 1010.0f, 270.0f));
            AddRecoveryFittedMesh(
                &Carrier, &Root, RoofMesh, MetalRoof ? MetalRoof : RoofFallback,
                TEXT("PASS35Museum_RecoveryRoofUpper"),
                FVector(0.0f, -35.0f, 690.0f), FVector(660.0f, 570.0f, 190.0f));
            AddRecoveryFittedMesh(
                &Carrier, &Root, RoofMesh, MetalRoof ? MetalRoof : RoofFallback,
                TEXT("PASS35Museum_RecoveryRoofEntrance"),
                FVector(0.0f, -535.0f, 415.0f), FVector(610.0f, 340.0f, 145.0f));
            UE_LOG(LogTemp, Display, TEXT("PASS35_MUSEUM_RECOVERY_PRESENTATION_READY roof=authored_asset"));
            return;
        }

        // Last-resort silhouette only. The real roof asset is always attempted first; these thin slabs exist
        // solely so a missing optional LFS presentation asset can never erase the entire museum from gameplay.
        for (const float Side : { -1.0f, 1.0f })
        {
            AddRecoveryBox(
                &Carrier, &Root, Cube, RoofFallback,
                FName(*FString::Printf(TEXT("PASS35Museum_RecoveryRoofMain_%s"), Side < 0.0f ? TEXT("L") : TEXT("R"))),
                FVector(0.0f, Side * 235.0f, 505.0f), FVector(1840.0f, 555.0f, 24.0f),
                FRotator(0.0f, 0.0f, Side * 18.0f));
            AddRecoveryBox(
                &Carrier, &Root, Cube, RoofFallback,
                FName(*FString::Printf(TEXT("PASS35Museum_RecoveryRoofUpper_%s"), Side < 0.0f ? TEXT("L") : TEXT("R"))),
                FVector(0.0f, -35.0f + Side * 130.0f, 690.0f), FVector(660.0f, 315.0f, 20.0f),
                FRotator(0.0f, 0.0f, Side * 20.0f));
        }
        UE_LOG(LogTemp, Warning, TEXT("PASS35_MUSEUM_RECOVERY_PRESENTATION_READY roof=fallback_slabs"));
    }

    AActor* EnsurePrototypeCarrier(UWorld& World)
    {
        if (AActor* Existing = FindActorWithTag(World, MuseumPrototypeTag)) return Existing;

        FActorSpawnParameters Params;
        Params.Name = MakeUniqueObjectName(&World, AActor::StaticClass(), FName(TEXT("R137_MuseumRecoveryCarrier")));
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AActor* Carrier = World.SpawnActor<AActor>(
            AActor::StaticClass(),
            FTransform(FRotator::ZeroRotator, AOCWorldSectorOster::MuseumAnchor()),
            Params);
        if (!Carrier) return nullptr;

        Carrier->SetReplicates(false);
        Carrier->Tags.Add(MuseumPrototypeTag);
        Carrier->Tags.Add(RecoveryCarrierTag);

        USceneComponent* Root = NewObject<USceneComponent>(
            Carrier,
            MakeUniqueObjectName(Carrier, USceneComponent::StaticClass(), FName(TEXT("Pass35MuseumCarrierRoot"))));
        if (!Root)
        {
            Carrier->Destroy();
            return nullptr;
        }
        Root->SetMobility(EComponentMobility::Static);
        Carrier->SetRootComponent(Root);
        Carrier->AddInstanceComponent(Root);
        Root->RegisterComponent();

        BuildRecoveryPresentation(*Carrier, *Root);

        UE_LOG(LogTemp, Warning,
            TEXT("PASS35_MUSEUM_OWNER_CARRIER_RECOVERED location=%s reason=R137_owner_missing"),
            *AOCWorldSectorOster::MuseumAnchor().ToCompactString());
        return Carrier;
    }

    void ReplayMuseumDetails(UWorld& World)
    {
        const bool bAuthority = World.GetNetMode() != NM_Client;

        if (bAuthority)
        {
            if (UOCR139MuseumMainDoorReplacementSubsystem* Stage =
                World.GetSubsystem<UOCR139MuseumMainDoorReplacementSubsystem>())
            {
                Stage->RunAuthoritativeDetailNow(World);
            }
        }
        if (UOCR140MuseumFacadeDetailSubsystem* Stage = World.GetSubsystem<UOCR140MuseumFacadeDetailSubsystem>())
            Stage->RunAuthoritativeDetailNow(World);
        if (bAuthority)
        {
            if (UOCR141MuseumWindowReplacementSubsystem* Stage =
                World.GetSubsystem<UOCR141MuseumWindowReplacementSubsystem>())
            {
                Stage->RunAuthoritativeDetailNow(World);
            }
        }
        if (UOCR142MuseumEntranceDetailSubsystem* Stage = World.GetSubsystem<UOCR142MuseumEntranceDetailSubsystem>())
            Stage->RunAuthoritativeDetailNow(World);
        if (UOCR143MuseumSiteVegetationSubsystem* Stage = World.GetSubsystem<UOCR143MuseumSiteVegetationSubsystem>())
            Stage->RunAuthoritativeDetailNow(World);
        if (UOCR144MuseumRearExteriorDetailSubsystem* Stage = World.GetSubsystem<UOCR144MuseumRearExteriorDetailSubsystem>())
            Stage->RunAuthoritativeDetailNow(World);
        if (UOCR145MuseumTreeLayoutSubsystem* Stage = World.GetSubsystem<UOCR145MuseumTreeLayoutSubsystem>())
            Stage->RunAuthoritativeDetailNow(World);

        UE_LOG(LogTemp, Display, TEXT("PASS35_MUSEUM_DETAIL_REPLAY_COMPLETE"));
    }

    void LogBaseDistances(UWorld& World)
    {
        const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
        float BestTeamOneCm = TNumericLimits<float>::Max();
        float BestTeamTwoCm = TNumericLimits<float>::Max();

        for (TActorIterator<AOCTeamSpawnPoint> It(&World); It; ++It)
        {
            const AOCTeamSpawnPoint* Point = *It;
            if (!Point || !Point->IsBaseSpawn()) continue;
            const float DistanceCm = FVector::Dist2D(Point->GetActorLocation(), Museum);
            if (Point->GetTeamId() == EOCTeam::TeamOne) BestTeamOneCm = FMath::Min(BestTeamOneCm, DistanceCm);
            if (Point->GetTeamId() == EOCTeam::TeamTwo) BestTeamTwoCm = FMath::Min(BestTeamTwoCm, DistanceCm);
        }

        const bool bTeamOneReady = BestTeamOneCm >= MuseumNoSpawnRadiusCm && BestTeamOneCm <= MuseumNearbyBaseRadiusCm;
        const bool bTeamTwoReady = BestTeamTwoCm >= MuseumNoSpawnRadiusCm && BestTeamTwoCm <= MuseumNearbyBaseRadiusCm;
        if (bTeamOneReady && bTeamTwoReady)
        {
            UE_LOG(LogTemp, Display,
                TEXT("PASS35_MUSEUM_BASE_DISTANCE_READY team1_m=%.1f team2_m=%.1f min_m=%.1f max_m=%.1f"),
                BestTeamOneCm / 100.0f,
                BestTeamTwoCm / 100.0f,
                MuseumNoSpawnRadiusCm / 100.0f,
                MuseumNearbyBaseRadiusCm / 100.0f);
        }
        else
        {
            UE_LOG(LogTemp, Error,
                TEXT("PASS35_MUSEUM_BASE_DISTANCE_FAIL team1_m=%.1f team2_m=%.1f"),
                BestTeamOneCm == TNumericLimits<float>::Max() ? -1.0f : BestTeamOneCm / 100.0f,
                BestTeamTwoCm == TNumericLimits<float>::Max() ? -1.0f : BestTeamTwoCm / 100.0f);
        }
    }
}

bool UOCMuseumCoreRecoverySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCMuseumCoreRecoverySubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    InWorld.GetTimerManager().SetTimer(
        RecoveryTimer,
        this,
        &UOCMuseumCoreRecoverySubsystem::EnsureMuseumCore,
        RecoveryDelaySeconds,
        false);
}

void UOCMuseumCoreRecoverySubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(RecoveryTimer);
    Super::Deinitialize();
}

void UOCMuseumCoreRecoverySubsystem::EnsureMuseumCore()
{
    UWorld* World = GetWorld();
    if (!World) return;

    int32 PrototypeOwners = CountActorsWithTag(*World, MuseumPrototypeTag);
    int32 ArchitectureOwners = CountActorsWithTag(*World, MuseumArchitectureTag);

    if (ArchitectureOwners == 0)
    {
        if (PrototypeOwners == 0 && !EnsurePrototypeCarrier(*World))
        {
            UE_LOG(LogTemp, Error, TEXT("PASS35_MUSEUM_CORE_FAIL reason=carrier_spawn_failed"));
            return;
        }

        if (UOCR138MuseumInteractiveArchitectureSubsystem* Architecture =
            World->GetSubsystem<UOCR138MuseumInteractiveArchitectureSubsystem>())
        {
            Architecture->RunAuthoritativeUpgradeNow(*World);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PASS35_MUSEUM_CORE_FAIL reason=R138_subsystem_missing"));
            return;
        }

        ReplayMuseumDetails(*World);
        PrototypeOwners = CountActorsWithTag(*World, MuseumPrototypeTag);
        ArchitectureOwners = CountActorsWithTag(*World, MuseumArchitectureTag);
    }

    LogBaseDistances(*World);

    if (PrototypeOwners == 1 && ArchitectureOwners == 1)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS35_MUSEUM_CORE_READY prototypeOwners=%d architectureOwners=%d anchor=%s recoveryCarrier=%d"),
            PrototypeOwners,
            ArchitectureOwners,
            *AOCWorldSectorOster::MuseumAnchor().ToCompactString(),
            FindActorWithTag(*World, RecoveryCarrierTag) ? 1 : 0);
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS35_MUSEUM_CORE_FAIL prototypeOwners=%d architectureOwners=%d"),
            PrototypeOwners,
            ArchitectureOwners);
    }
}
