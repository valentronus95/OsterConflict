#include "OCGameRecoveryStadiumActivationSubsystem.h"

#include "OCGameMode.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "UObject/SoftObjectPath.h"

namespace
{
    const TCHAR* StadiumPresentationPaths[] =
    {
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Plane_1x1.SM_Plane_1x1"),
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Meshes/SM_Curb_1.SM_Curb_1"),
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_Pack_V1/Mesh/SM_Sign_1.SM_Sign_1"),
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Grass_Inst.M_Grass_Inst"),
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_Pack_V1/Materials/Instances/M_Color_1_Inst.M_Color_1_Inst"),
        TEXT("/Game/Mega_Street_Props_Pack/Street_Props_pack_V2/Materials/Instances/M_Metal_3_Inst.M_Metal_3_Inst"),
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var01.SM_House_Var01"),
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_House_Var02.SM_House_Var02"),
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var01.SM_Fence_Var01"),
        TEXT("/Game/AdvancedVillagePack/Meshes/SM_Fence_Var03.SM_Fence_Var03"),
        TEXT("/Game/KiteDemo/Environments/Trees/HillTree_02/HillTree_02.HillTree_02"),
        TEXT("/Game/KiteDemo/Environments/Trees/ScotsPineTall_01/ScotsPineTall_01.ScotsPineTall_01"),
        TEXT("/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Ground_01/SM_Urb_Roa_Ground_01.SM_Urb_Roa_Ground_01"),
        TEXT("/Game/Scene_RoadsideConstruction/Assets/Custom/Urb_Roa_Sidewalk_01/SM_Urb_Roa_Sidewalk_01.SM_Urb_Roa_Sidewalk_01"),
    };

    TArray<FSoftObjectPath> BuildStadiumPreloadPaths()
    {
        TArray<FSoftObjectPath> Paths;
        Paths.Reserve(UE_ARRAY_COUNT(StadiumPresentationPaths));
        for (const TCHAR* Path : StadiumPresentationPaths)
        {
            Paths.Emplace(Path);
        }
        return Paths;
    }

    bool AreStadiumAssetsResolved(FString& OutMissingAsset)
    {
        for (const TCHAR* Path : StadiumPresentationPaths)
        {
            const FSoftObjectPath AssetPath(Path);
            if (!AssetPath.ResolveObject())
            {
                OutMissingAsset = AssetPath.ToString();
                return false;
            }
        }
        OutMissingAsset.Reset();
        return true;
    }

    bool HasAuthoritativeStadiumActor(UWorld& World)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            const AActor* Actor = *It;
            if (Actor && Actor->ActorHasTag(TEXT("R13_StadionOsterAuthoritative")))
            {
                return true;
            }
        }
        return false;
    }
}

void UOCGameRecoveryStadiumActivationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    // Deliberately bypass UOCR13StadiumSurfaceSubsystem::OnWorldBeginPlay: that historical path performs
    // synchronous LoadObject calls. The concrete recovery owner reaches ApplyStadiumSurface only after preload.
    UWorldSubsystem::OnWorldBeginPlay(InWorld);

    bPreloadRequested = false;
    bPreloadFailed = false;
    bPresentationReady = false;

    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    BeginStadiumPreload();
}

void UOCGameRecoveryStadiumActivationSubsystem::BeginStadiumPreload()
{
    if (bPreloadRequested || bPresentationReady) return;
    bPreloadRequested = true;

    const TArray<FSoftObjectPath> Paths = BuildStadiumPreloadPaths();
    PreloadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
        Paths,
        FStreamableDelegate::CreateUObject(this,
            &UOCGameRecoveryStadiumActivationSubsystem::CompleteStadiumPreload));

    UE_LOG(LogTemp, Display,
        TEXT("GAME_RECOVERY_STADIUM_ASYNC_PRELOAD_BEGIN assets=%d pre_spawn=1 sync_gameplay_loads=0 runtime_acceptance=0"),
        Paths.Num());

    if (!PreloadHandle.IsValid())
    {
        bPreloadFailed = true;
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_STADIUM_ASYNC_PRELOAD_FAIL reason=invalid_handle authored_stadium_visible=0 sync_fallback=0 runtime_acceptance=0"));
    }
}

void UOCGameRecoveryStadiumActivationSubsystem::CompleteStadiumPreload()
{
    if (!PreloadHandle.IsValid() || !PreloadHandle->HasLoadCompleted())
    {
        bPreloadFailed = true;
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_STADIUM_ASYNC_PRELOAD_FAIL reason=handle_not_complete authored_stadium_visible=0 sync_fallback=0 runtime_acceptance=0"));
        PreloadHandle.Reset();
        return;
    }

    FString MissingAsset;
    if (!AreStadiumAssetsResolved(MissingAsset))
    {
        bPreloadFailed = true;
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_STADIUM_ASYNC_PRELOAD_FAIL reason=asset_not_resolved asset=%s authored_stadium_visible=0 sync_fallback=0 runtime_acceptance=0"),
            *MissingAsset);
        PreloadHandle.Reset();
        return;
    }

    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld())
    {
        bPreloadFailed = true;
        PreloadHandle.Reset();
        return;
    }

    ApplyStadiumSurface(*World);
    bPresentationReady = HasAuthoritativeStadiumActor(*World);

    if (bPresentationReady)
    {
        UE_LOG(LogTemp, Display,
            TEXT("GAME_RECOVERY_STADIUM_PRESENTATION_READY assets=%d async_preload=1 sync_gameplay_loads=0 canonical_owner=R13_StadionOsterAuthoritative runtime_acceptance=0"),
            UE_ARRAY_COUNT(StadiumPresentationPaths));
    }
    else
    {
        bPreloadFailed = true;
        UE_LOG(LogTemp, Error,
            TEXT("GAME_RECOVERY_STADIUM_PRESENTATION_FAIL reason=authoritative_actor_missing sync_fallback=0 runtime_acceptance=0"));
    }

    PreloadHandle.Reset();
}

void UOCGameRecoveryStadiumActivationSubsystem::Deinitialize()
{
    if (PreloadHandle.IsValid())
    {
        PreloadHandle->CancelHandle();
        PreloadHandle.Reset();
    }
    bPreloadRequested = false;
    bPreloadFailed = false;
    bPresentationReady = false;
    Super::Deinitialize();
}
