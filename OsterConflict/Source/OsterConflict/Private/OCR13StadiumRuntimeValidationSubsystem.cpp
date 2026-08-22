#include "OCR13StadiumRuntimeValidationSubsystem.h"

#include "OCGameMode.h"
#include "OCGeoReference.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

namespace
{
    const FName AuthoritativeTag(TEXT("R13_StadionOsterAuthoritative"));

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

    bool HasTextComponent(AActor* Actor, const FName Name)
    {
        if (!Actor) return false;
        TInlineComponentArray<UTextRenderComponent*> Components;
        Actor->GetComponents(Components);
        for (const UTextRenderComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return true;
        }
        return false;
    }
}

bool UOCR13StadiumRuntimeValidationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

TStatId UOCR13StadiumRuntimeValidationSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCR13StadiumRuntimeValidationSubsystem, STATGROUP_Tickables);
}

void UOCR13StadiumRuntimeValidationSubsystem::FailValidation(const FString& Reason)
{
    if (bFinished) return;
    bFinished = true;
    UE_LOG(LogTemp, Error, TEXT("PASS9_STADION_OSTER_RUNTIME_FAIL reason=%s"), *Reason);
}

void UOCR13StadiumRuntimeValidationSubsystem::Tick(float DeltaTime)
{
    if (bFinished) return;

    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld()) return;
    if (!World->GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = World->GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    ElapsedSeconds += FMath::Max(0.0f, DeltaTime);
    if (ElapsedSeconds < 1.0f) return;

    AActor* StadiumActor = nullptr;
    int32 StadiumActorCount = 0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor && Actor->ActorHasTag(AuthoritativeTag))
        {
            StadiumActor = Actor;
            ++StadiumActorCount;
        }
    }

    if (StadiumActorCount == 0)
    {
        if (ElapsedSeconds < 8.0f) return;
        FailValidation(TEXT("authoritative_site_actor_missing"));
        return;
    }
    if (StadiumActorCount != 1 || !StadiumActor)
    {
        FailValidation(FString::Printf(TEXT("authoritative_site_actor_count_%d"), StadiumActorCount));
        return;
    }

    struct FRequiredISM
    {
        FName Name;
        int32 MinInstances;
    };

    const FRequiredISM RequiredComponents[] =
    {
        { TEXT("StadionOsterMainPitch"), 1 },
        { TEXT("StadionOsterRunningSurface"), 4 },
        { TEXT("StadionOsterPitchLines"), 10 },
        { TEXT("StadionOsterSportsMetal"), 12 },
        { TEXT("StadionOsterFootpaths"), 2 },
        { TEXT("StadionOsterEntranceBlue"), 1 },
        { TEXT("StadionOsterEntranceYellow"), 1 },
        { TEXT("StadionOsterHouses01"), 1 },
        { TEXT("StadionOsterHouses02"), 1 },
        { TEXT("StadionOsterTrees01"), 1 },
        { TEXT("StadionOsterTrees04"), 1 },
        { TEXT("StadionOsterFences01"), 1 },
        { TEXT("StadionOsterFences03"), 1 },
    };

    int32 TotalRequiredInstances = 0;
    for (const FRequiredISM& Required : RequiredComponents)
    {
        UInstancedStaticMeshComponent* Component = FindISM(StadiumActor, Required.Name);
        if (!Component)
        {
            FailValidation(FString::Printf(TEXT("component_missing_%s"), *Required.Name.ToString()));
            return;
        }
        const int32 Count = Component->GetInstanceCount();
        if (Count < Required.MinInstances)
        {
            FailValidation(FString::Printf(TEXT("component_%s_instances_%d_lt_%d"),
                *Required.Name.ToString(), Count, Required.MinInstances));
            return;
        }
        TotalRequiredInstances += Count;
    }

    if (FindISM(StadiumActor, TEXT("StadionOsterGrassApron")))
    {
        FailValidation(TEXT("obsolete_giant_grass_apron_returned"));
        return;
    }

    if (!HasTextComponent(StadiumActor, TEXT("StadionOsterEntranceText")))
    {
        FailValidation(TEXT("entrance_text_missing"));
        return;
    }

    UInstancedStaticMeshComponent* Pitch = FindISM(StadiumActor, TEXT("StadionOsterMainPitch"));
    FTransform PitchTransform;
    if (!Pitch || !Pitch->GetInstanceTransform(0, PitchTransform, true))
    {
        FailValidation(TEXT("pitch_transform_unavailable"));
        return;
    }

    const FOCGeoReferencePoint StadiumGeo = FOCGeoReference::Stadium();
    const FVector ExpectedXY = FOCGeoReference::ToLocalCm(StadiumGeo.Latitude, StadiumGeo.Longitude, 0.0);
    const FVector PitchLocation = PitchTransform.GetLocation();
    const float XYError = FVector2D(PitchLocation.X - ExpectedXY.X, PitchLocation.Y - ExpectedXY.Y).Size();
    if (XYError > 150.0f)
    {
        FailValidation(FString::Printf(TEXT("pitch_georef_error_cm_%.1f"), XYError));
        return;
    }

    FHitResult GroundHit;
    FCollisionQueryParams GroundParams(SCENE_QUERY_STAT(OCStadiumRuntimeValidationGround), false);
    GroundParams.AddIgnoredActor(StadiumActor);
    const FVector GroundStart(ExpectedXY.X, ExpectedXY.Y, PitchLocation.Z + 8000.0f);
    const FVector GroundEnd(ExpectedXY.X, ExpectedXY.Y, PitchLocation.Z - 8000.0f);
    if (!World->LineTraceSingleByChannel(GroundHit, GroundStart, GroundEnd, ECC_Visibility, GroundParams))
    {
        FailValidation(TEXT("terrain_ground_trace_failed"));
        return;
    }

    const float ExpectedPitchZ = GroundHit.ImpactPoint.Z + 12.0f;
    const float ZError = FMath::Abs(PitchLocation.Z - ExpectedPitchZ);
    if (ZError > 45.0f)
    {
        FailValidation(FString::Printf(TEXT("pitch_terrain_z_error_cm_%.1f"), ZError));
        return;
    }

    bool bFoundWorldSector = false;
    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        AOCWorldSectorOster* Sector = *It;
        if (!Sector) continue;
        bFoundWorldSector = true;

        for (const FName LegacyName : { FName(TEXT("StadiumGeometry")), FName(TEXT("StadiumDetails")) })
        {
            if (UInstancedStaticMeshComponent* Legacy = FindISM(Sector, LegacyName))
            {
                if (Legacy->IsVisible())
                {
                    FailValidation(FString::Printf(TEXT("legacy_visible_%s"), *LegacyName.ToString()));
                    return;
                }
            }
        }
    }

    if (!bFoundWorldSector)
    {
        FailValidation(TEXT("world_sector_missing"));
        return;
    }

    bFinished = true;
    UE_LOG(LogTemp, Display,
        TEXT("PASS9_STADION_OSTER_READY actors=1 components=%d pitchXYErrorCm=%.1f pitchZErrorCm=%.1f georef=(%.6f,%.6f)"),
        TotalRequiredInstances, XYError, ZError, StadiumGeo.Latitude, StadiumGeo.Longitude);
}
