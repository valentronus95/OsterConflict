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
    constexpr float CollegeAccessRepairDelaySeconds = 2.40f;
    constexpr float CollegeYawDegrees = 1.0f;
    constexpr float FrontFenceY = -2450.0f;
    constexpr float FrontFenceZ = 110.0f;
    constexpr float FrontFenceGapCenterX = 900.0f;
    constexpr float FrontFenceGapWidthCm = 3400.0f;
    constexpr float LeftFenceCenterX = -3000.0f;
    constexpr float LeftFenceLengthCm = 4400.0f;
    constexpr float RightFenceCenterX = 3900.0f;
    constexpr float RightFenceLengthCm = 2600.0f;
    const FVector LegacyFrontFenceScale(104.0f, 0.45f, 2.20f);

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

    int32 FindLegacyFrontFenceInstance(UInstancedStaticMeshComponent* Fences, const FVector& ExpectedCenter)
    {
        if (!Fences) return INDEX_NONE;
        for (int32 Index = Fences->GetInstanceCount() - 1; Index >= 0; --Index)
        {
            FTransform Transform;
            if (!Fences->GetInstanceTransform(Index, Transform, false)) continue;
            if (!Transform.GetLocation().Equals(ExpectedCenter, 4.0f)) continue;
            if (!Transform.GetScale3D().Equals(LegacyFrontFenceScale, 0.02f)) continue;
            return Index;
        }
        return INDEX_NONE;
    }

    UInstancedStaticMeshComponent* BuildSplitFence(AOCWorldSectorOster* Sector,
        UInstancedStaticMeshComponent* SourceFences, const FVector& College)
    {
        if (!Sector || !SourceFences || !SourceFences->GetStaticMesh() || !Sector->GetRootComponent()) return nullptr;

        UInstancedStaticMeshComponent* Split = NewObject<UInstancedStaticMeshComponent>(
            Sector, TEXT("R13_CollegeFrontFenceSplit"));
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
            College + FVector(LeftFenceCenterX, FrontFenceY, FrontFenceZ),
            FVector(LeftFenceLengthCm / 100.0f, 0.45f, 2.20f)), false);
        Split->AddInstance(FTransform(FRotator(0.0f, CollegeYawDegrees, 0.0f),
            College + FVector(RightFenceCenterX, FrontFenceY, FrontFenceZ),
            FVector(RightFenceLengthCm / 100.0f, 0.45f, 2.20f)), false);
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

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) RepairCollegeEntrance(*World);
        }), CollegeAccessRepairDelaySeconds, false);
}

void UOCR13CollegeAccessRepairSubsystem::RepairCollegeEntrance(UWorld& World)
{
    AOCWorldSectorOster* Sector = FindOsterSector(World);
    if (!Sector || Sector->ActorHasTag(TEXT("R13_CollegeAccessRepairApplied"))) return;

    UInstancedStaticMeshComponent* SourceFences = FindFenceComponent(Sector);
    if (!SourceFences) return;

    const FVector College = AOCWorldSectorOster::CollegeAnchor();
    const FVector LegacyFenceCenter = College + FVector(0.0f, FrontFenceY, FrontFenceZ);
    const int32 LegacyIndex = FindLegacyFrontFenceInstance(SourceFences, LegacyFenceCenter);
    if (LegacyIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("R13 college access repair: legacy 104m front fence instance not found; no replacement applied."));
        return;
    }

    UInstancedStaticMeshComponent* SplitFence = BuildSplitFence(Sector, SourceFences, College);
    if (!SplitFence) return;

    if (!SourceFences->RemoveInstance(LegacyIndex))
    {
        SplitFence->DestroyComponent();
        UE_LOG(LogTemp, Warning,
            TEXT("R13 college access repair: failed to remove legacy front fence; split replacement rolled back."));
        return;
    }

    Sector->Tags.Add(TEXT("R13_CollegeAccessRepairApplied"));
    UE_LOG(LogTemp, Display,
        TEXT("R13 college access repair: removed the stair-crossing 104m front fence and installed two BlockAll segments with a 3.4m opening centered on X+900; side/rear fences untouched."));

    static_cast<void>(FrontFenceGapCenterX);
    static_cast<void>(FrontFenceGapWidthCm);
}
