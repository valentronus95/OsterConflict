#include "OCPass45ImportedWeaponBridgeSubsystem.h"

#include "OCGameMode.h"
#include "OCPass45LocalAssetResolver.h"
#include "OCWeaponBase.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    const FName ProductionVisualTag(TEXT("OC_ProductionWeaponVisual"));
    const FName RealFallbackComponentTag(TEXT("OC_RealFallbackWeaponVisual"));
    const FName LocalBridgeTag(TEXT("OC_PASS45_LOCAL_IMPORTED_WEAPON"));
    constexpr int32 MaxRefreshPasses = 8;

    bool HasProductionVisual(const AOCWeaponBase& Weapon)
    {
        TArray<UPrimitiveComponent*> Components;
        Weapon.GetComponents<UPrimitiveComponent>(Components);
        for (const UPrimitiveComponent* Component : Components)
        {
            if (Component && Component->ComponentHasTag(ProductionVisualTag) && Component->IsVisible())
            {
                return true;
            }
        }
        return false;
    }

    void RetireTemporaryRealFallbacks(AOCWeaponBase& Weapon)
    {
        TArray<UActorComponent*> Components;
        Weapon.GetComponents(Components);
        for (UActorComponent* Component : Components)
        {
            if (Component && Component->ComponentHasTag(RealFallbackComponentTag))
            {
                Component->DestroyComponent();
            }
        }
    }

    UStaticMesh* ResolveExactLocalVisual(const FString& DisplayName)
    {
        if (DisplayName.Equals(TEXT("M1911"), ESearchCase::IgnoreCase))
        {
            return OCPass45FindLocalStaticMesh(
                { FName(TEXT("/Game/colt-m1911")) },
                { TEXT("1911"), TEXT("colt"), TEXT("pistol") });
        }
        if (DisplayName.Equals(TEXT("Remington 870"), ESearchCase::IgnoreCase))
        {
            return OCPass45FindLocalStaticMesh(
                { FName(TEXT("/Game/shotgun")) },
                { TEXT("shotgun"), TEXT("remington"), TEXT("weapon") });
        }
        if (DisplayName.Equals(TEXT("AK-47"), ESearchCase::IgnoreCase))
        {
            return OCPass45FindLocalStaticMesh(
                { FName(TEXT("/Game/AK-47")) },
                { TEXT("ak-47"), TEXT("ak47"), TEXT("weapon") });
        }
        return nullptr;
    }

    float DesiredLengthFor(const FString& DisplayName)
    {
        if (DisplayName.Equals(TEXT("M1911"), ESearchCase::IgnoreCase)) return 23.0f;
        if (DisplayName.Equals(TEXT("Remington 870"), ESearchCase::IgnoreCase)) return 100.0f;
        if (DisplayName.Equals(TEXT("AK-47"), ESearchCase::IgnoreCase)) return 88.0f;
        return 80.0f;
    }

    bool ApplyLocalVisual(AOCWeaponBase& Weapon, UStaticMesh* Mesh)
    {
        if (!Mesh || HasProductionVisual(Weapon)) return false;
        USceneComponent* VisualRoot = Weapon.GetWeaponVisualRoot();
        if (!VisualRoot) return false;

        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        const float NativeLength = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
        if (NativeLength <= 1.0f) return false;

        const float DesiredLength = DesiredLengthFor(Weapon.GetWeaponDisplayName());
        const float UniformScale = DesiredLength / NativeLength;

        UStaticMeshComponent* Visual = NewObject<UStaticMeshComponent>(
            &Weapon,
            MakeUniqueObjectName(&Weapon, UStaticMeshComponent::StaticClass(), FName(TEXT("Pass45LocalImportedWeaponVisual"))));
        if (!Visual) return false;

        Visual->SetupAttachment(VisualRoot);
        Visual->SetStaticMesh(Mesh);
        Visual->SetRelativeLocation(-Bounds.Origin * UniformScale);
        Visual->SetRelativeRotation(FRotator::ZeroRotator);
        Visual->SetRelativeScale3D(FVector(UniformScale));
        Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Visual->SetGenerateOverlapEvents(false);
        Visual->SetCanEverAffectNavigation(false);
        Visual->SetCastShadow(true);
        Visual->SetHiddenInGame(false, true);
        Visual->SetVisibility(true, true);
        Visual->ComponentTags.AddUnique(ProductionVisualTag);
        Visual->ComponentTags.AddUnique(LocalBridgeTag);
        Weapon.AddInstanceComponent(Visual);
        Visual->RegisterComponent();

        RetireTemporaryRealFallbacks(Weapon);
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_LOCAL_IMPORTED_WEAPON_READY weapon=%s asset=%s desired_length_cm=%.1f production_visual=1 temporary_fallback_retired=1 runtime_acceptance=0"),
            *Weapon.GetWeaponDisplayName(), *Mesh->GetPathName(), DesiredLength);
        return true;
    }
}

bool UOCPass45ImportedWeaponBridgeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && World->GetNetMode() != NM_DedicatedServer &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCPass45ImportedWeaponBridgeSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    InWorld.GetTimerManager().SetTimer(
        RefreshTimer,
        this,
        &UOCPass45ImportedWeaponBridgeSubsystem::RefreshWeapons,
        0.35f,
        true,
        0.08f);
}

void UOCPass45ImportedWeaponBridgeSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(RefreshTimer);
    RefreshPass = 0;
    Super::Deinitialize();
}

void UOCPass45ImportedWeaponBridgeSubsystem::RefreshWeapons()
{
    UWorld* World = GetWorld();
    if (!World) return;
    ++RefreshPass;

    int32 ExactCandidates = 0;
    int32 Applied = 0;
    for (TActorIterator<AOCWeaponBase> It(World); It; ++It)
    {
        AOCWeaponBase* Weapon = *It;
        if (!Weapon || Weapon->IsActorBeingDestroyed() || HasProductionVisual(*Weapon)) continue;

        UStaticMesh* LocalMesh = ResolveExactLocalVisual(Weapon->GetWeaponDisplayName());
        if (!LocalMesh) continue;
        ++ExactCandidates;
        if (ApplyLocalVisual(*Weapon, LocalMesh)) ++Applied;
    }

    if (Applied > 0)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_LOCAL_IMPORTED_WEAPON_BRIDGE_PASS pass=%d exact_candidates=%d applied=%d wrong_identity_substitution=0"),
            RefreshPass, ExactCandidates, Applied);
    }

    if (RefreshPass >= MaxRefreshPasses)
    {
        World->GetTimerManager().ClearTimer(RefreshTimer);
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_LOCAL_IMPORTED_WEAPON_BRIDGE_STOPPED passes=%d permanent_scan=0 wrong_identity_substitution=0"),
            RefreshPass);
    }
}
