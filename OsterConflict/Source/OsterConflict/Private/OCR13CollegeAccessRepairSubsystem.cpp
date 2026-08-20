#include "OCR13CollegeAccessRepairSubsystem.h"

#include "OCGameMode.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

namespace
{
    constexpr float CollegeAccessInitialDelaySeconds = 0.10f;
    constexpr float CollegeAccessRetryDelaySeconds = 0.25f;
    constexpr int32 CollegeAccessMaxAttempts = 40;
    constexpr float CollegeYawDegrees = 1.0f;

    // South/front boundary: preserve the original 104 m span while opening the broad authored entrance stairs.
    constexpr float FrontFenceY = -2450.0f;
    constexpr float FrontFenceZ = 110.0f;
    constexpr float FrontFenceGapCenterX = 900.0f;
    constexpr float FrontFenceGapWidthCm = 3400.0f;
    constexpr float FrontLeftSegmentCenterX = -3000.0f;
    constexpr float FrontLeftSegmentLengthCm = 4400.0f;
    constexpr float FrontRightSegmentCenterX = 3900.0f;
    constexpr float FrontRightSegmentLengthCm = 2600.0f;
    const FVector LegacyFrontFenceScale(104.0f, 0.45f, 2.20f);

    // West/left boundary: the 6.6 m road plus its two sidewalks occupies 14.4 m total width around Y=0.
    // An 18 m gate leaves 1.8 m clearance on each side and preserves the original -2450..9250 cm outer span.
    constexpr float LeftFenceX = -5600.0f;
    constexpr float LeftFenceCenterY = 3400.0f;
    constexpr float LeftFenceZ = 110.0f;
    constexpr float LeftFenceGapCenterY = 0.0f;
    constexpr float LeftFenceGapWidthCm = 1800.0f;
    constexpr float LeftLowerSegmentCenterY = -1675.0f;
    constexpr float LeftLowerSegmentLengthCm = 1550.0f;
    constexpr float LeftUpperSegmentCenterY = 5075.0f;
    constexpr float LeftUpperSegmentLengthCm = 8350.0f;
    const FVector LegacyLeftFenceScale(0.45f, 117.0f, 2.20f);

    AOCWorldSectorOster* FindOsterSector(UWorld& World)
    {
        for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
        {
            if (AOCWorldSectorOster* Sector = *It) return Sector;
        }
        return nullptr;
    }

    UInstancedStaticMeshComponent* FindFenceComponent(AOCWorldSectorOster* Sector)
    {
        if (!Sector) return nullptr;
        TArray<UInstancedStaticMeshComponent*> Components;
        Sector->GetComponents<UInstancedStaticMeshComponent>(Components, false);
        const FName FencesName(TEXT("Fences"));
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (Component && Component->GetFName() == FencesName) return Component;
        }
        return nullptr;
    }

    int32 FindLegacyFenceInstance(UInstancedStaticMeshComponent* Fences, const FVector& ExpectedCenter,
        const FVector& ExpectedScale)
    {
        if (!Fences) return INDEX_NONE;
        for (int32 Index = Fences->GetInstanceCount() - 1; Index >= 0; --Index)
        {
            FTransform Transform;
            if (!Fences->GetInstanceTransform(Index, Transform, false)) continue;
            if (!Transform.GetLocation().Equals(ExpectedCenter, 4.0f)) continue;
            if (!Transform.GetScale3D().Equals(ExpectedScale, 0.02f)) continue;
            return Index;
        }
        return INDEX_NONE;
    }

    UInstancedStaticMeshComponent* BuildSplitFences(AOCWorldSectorOster* Sector,
        UInstancedStaticMeshComponent* SourceFences, const FVector& College)
    {
        if (!Sector || !SourceFences || !SourceFences->GetStaticMesh() || !Sector->GetRootComponent()) return nullptr;

        UInstancedStaticMeshComponent* Split = NewObject<UInstancedStaticMeshComponent>(
            Sector, TEXT("R13_CollegeAccessFenceSplits"));
        if (!Split) return nullptr;

        Split->SetupAttachment(Sector->GetRootComponent());
        Split->SetStaticMesh(SourceFences->GetStaticMesh());
        if (UMaterialInterface* Material = SourceFences->GetMaterial(0)) Split->SetMaterial(0, Material);
        Split->SetMobility(EComponentMobility::Static);
        Split->SetCollisionProfileName(SourceFences->GetCollisionProfileName());
        Split->SetCollisionEnabled(SourceFences->GetCollisionEnabled());
        Split->SetGenerateOverlapEvents(false);
        Split->SetCanEverAffectNavigation(true);
        Split->SetCastShadow(true);
        Sector->AddInstanceComponent(Split);
        Split->RegisterComponent();

        // Local-space instances match the source Fences component convention exactly.
        Split->AddInstance(FTransform(FRotator(0.0f, CollegeYawDegrees, 0.0f),
            College + FVector(FrontLeftSegmentCenterX, FrontFenceY, FrontFenceZ),
            FVector(FrontLeftSegmentLengthCm / 100.0f, 0.45f, 2.20f)), false);
        Split->AddInstance(FTransform(FRotator(0.0f, CollegeYawDegrees, 0.0f),
            College + FVector(FrontRightSegmentCenterX, FrontFenceY, FrontFenceZ),
            FVector(FrontRightSegmentLengthCm / 100.0f, 0.45f, 2.20f)), false);
        Split->AddInstance(FTransform(FRotator(0.0f, CollegeYawDegrees, 0.0f),
            College + FVector(LeftFenceX, LeftLowerSegmentCenterY, LeftFenceZ),
            FVector(0.45f, LeftLowerSegmentLengthCm / 100.0f, 2.20f)), false);
        Split->AddInstance(FTransform(FRotator(0.0f, CollegeYawDegrees, 0.0f),
            College + FVector(LeftFenceX, LeftUpperSegmentCenterY, LeftFenceZ),
            FVector(0.45f, LeftUpperSegmentLengthCm / 100.0f, 2.20f)), false);
        return Split;
    }
}

bool UOCR13CollegeAccessRepairSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13CollegeAccessRepairSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    // UWorldSubsystem::OnWorldBeginPlay is earlier than Actor::BeginPlay. Start quickly, then retry until the
    // sector actor has completed BeginPlay so the split segments inherit its final tinted fence material.
    ScheduleRepair(InWorld, 0);
}

void UOCR13CollegeAccessRepairSubsystem::ScheduleRepair(UWorld& World, const int32 AttemptIndex)
{
    if (AttemptIndex >= CollegeAccessMaxAttempts)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("R13 college access repair: gave up after %d startup attempts; legacy fences left unchanged."),
            CollegeAccessMaxAttempts);
        return;
    }

    const float DelaySeconds = AttemptIndex == 0
        ? CollegeAccessInitialDelaySeconds
        : CollegeAccessRetryDelaySeconds;
    TWeakObjectPtr<UWorld> WeakWorld(&World);
    FTimerHandle Timer;
    World.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld, AttemptIndex]()
        {
            if (UWorld* RetryWorld = WeakWorld.Get())
            {
                if (!RepairCollegeEntrance(*RetryWorld))
                {
                    ScheduleRepair(*RetryWorld, AttemptIndex + 1);
                }
            }
        }), DelaySeconds, false);
}

bool UOCR13CollegeAccessRepairSubsystem::RepairCollegeEntrance(UWorld& World)
{
    AOCWorldSectorOster* Sector = FindOsterSector(World);
    if (!Sector) return false;
    if (Sector->ActorHasTag(TEXT("R13_CollegeAccessRepairApplied"))) return true;
    if (!Sector->HasActorBegunPlay()) return false;

    UInstancedStaticMeshComponent* SourceFences = FindFenceComponent(Sector);
    if (!SourceFences) return false;

    const FVector College = AOCWorldSectorOster::CollegeAnchor();
    const FVector LegacyFrontCenter = College + FVector(0.0f, FrontFenceY, FrontFenceZ);
    const FVector LegacyLeftCenter = College + FVector(LeftFenceX, LeftFenceCenterY, LeftFenceZ);
    const int32 FrontIndex = FindLegacyFenceInstance(SourceFences, LegacyFrontCenter, LegacyFrontFenceScale);
    const int32 LeftIndex = FindLegacyFenceInstance(SourceFences, LegacyLeftCenter, LegacyLeftFenceScale);
    if (FrontIndex == INDEX_NONE || LeftIndex == INDEX_NONE || FrontIndex == LeftIndex) return false;

    FTransform FrontTransform;
    FTransform LeftTransform;
    if (!SourceFences->GetInstanceTransform(FrontIndex, FrontTransform, false)) return false;
    if (!SourceFences->GetInstanceTransform(LeftIndex, LeftTransform, false)) return false;

    UInstancedStaticMeshComponent* SplitFence = BuildSplitFences(Sector, SourceFences, College);
    if (!SplitFence) return false;

    const bool bFrontRemovedFirst = FrontIndex > LeftIndex;
    const int32 FirstIndex = bFrontRemovedFirst ? FrontIndex : LeftIndex;
    const int32 SecondIndex = bFrontRemovedFirst ? LeftIndex : FrontIndex;
    const FTransform FirstTransform = bFrontRemovedFirst ? FrontTransform : LeftTransform;

    if (!SourceFences->RemoveInstance(FirstIndex))
    {
        SplitFence->DestroyComponent();
        return false;
    }
    if (!SourceFences->RemoveInstance(SecondIndex))
    {
        SourceFences->AddInstance(FirstTransform, false);
        SplitFence->DestroyComponent();
        UE_LOG(LogTemp, Warning,
            TEXT("R13 college access repair: second legacy fence removal failed; first removal restored and split replacement rolled back."));
        return false;
    }

    Sector->Tags.Add(TEXT("R13_CollegeAccessRepairApplied"));
    UE_LOG(LogTemp, Display,
        TEXT("R13 college access repair: front 104m fence now has a 34m stair opening at X+900cm; west 117m fence now has an 18m vehicle+sidewalk gate at Y=0 with 1.8m side clearance; blocking/navigation preserved; north boundary untouched."));

    static_cast<void>(FrontFenceGapCenterX);
    static_cast<void>(FrontFenceGapWidthCm);
    static_cast<void>(LeftFenceGapCenterY);
    static_cast<void>(LeftFenceGapWidthCm);
    return true;
}
