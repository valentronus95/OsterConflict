#include "OCLocalInboxWeaponOverrideSubsystem.h"

#include "OCAntiArmorLauncher.h"
#include "OCLocalInboxRuntimeSubsystem.h"
#include "OCWeaponBase.h"
#include "OCWeaponVariants.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    const FName LocalVisualTag(TEXT("OC_LocalInboxWeaponVisual"));
    const FName LocalBoundTag(TEXT("OC_LocalInboxWeaponBound"));

    template <typename TMesh, typename TComponent>
    TComponent* AddVisual(AOCWeaponBase* Weapon, USceneComponent* Root, TMesh* Mesh,
        const FName BaseName, const float DesiredLengthCm)
    {
        if (!Weapon || !Root || !Mesh) return nullptr;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        const float Longest = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
        if (Longest <= 1.0f) return nullptr;

        const FName UniqueName = MakeUniqueObjectName(Weapon, TComponent::StaticClass(), BaseName);
        TComponent* Visual = NewObject<TComponent>(Weapon, UniqueName);
        if (!Visual) return nullptr;

        const float UniformScale = DesiredLengthCm / Longest;
        Visual->SetupAttachment(Root);
        Visual->SetRelativeLocation(-Bounds.Origin * UniformScale);
        Visual->SetRelativeRotation(FRotator::ZeroRotator);
        Visual->SetRelativeScale3D(FVector(UniformScale));
        Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Visual->SetGenerateOverlapEvents(false);
        Visual->SetCanEverAffectNavigation(false);
        Visual->SetCastShadow(true);
        Visual->ComponentTags.Add(LocalVisualTag);
        Weapon->AddInstanceComponent(Visual);
        Visual->RegisterComponent();
        return Visual;
    }

    void HideOldWeaponPresentation(AOCWeaponBase* Weapon)
    {
        if (!Weapon) return;

        TInlineComponentArray<UStaticMeshComponent*> StaticComponents;
        Weapon->GetComponents(StaticComponents);
        for (UStaticMeshComponent* Component : StaticComponents)
        {
            if (Component && !Component->ComponentHasTag(LocalVisualTag))
            {
                Component->SetVisibility(false, false);
                Component->SetHiddenInGame(true, false);
            }
        }

        TInlineComponentArray<USkeletalMeshComponent*> SkeletalComponents;
        Weapon->GetComponents(SkeletalComponents);
        for (USkeletalMeshComponent* Component : SkeletalComponents)
        {
            if (Component && !Component->ComponentHasTag(LocalVisualTag))
            {
                Component->SetVisibility(false, false);
                Component->SetHiddenInGame(true, false);
            }
        }
    }
}

bool UOCLocalInboxWeaponOverrideSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCLocalInboxWeaponOverrideSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    int32 BoundAtStart = 0;
    for (TActorIterator<AOCWeaponBase> It(&InWorld); It; ++It)
    {
        const bool bBefore = It->ActorHasTag(LocalBoundTag);
        ApplyLocalVisual(*It);
        if (!bBefore && It->ActorHasTag(LocalBoundTag)) ++BoundAtStart;
    }

    ActorSpawnedHandle = InWorld.AddOnActorSpawnedHandler(
        FOnActorSpawned::FDelegate::CreateUObject(this, &UOCLocalInboxWeaponOverrideSubsystem::HandleActorSpawned));
    UE_LOG(LogTemp, Display, TEXT("PASS45_LOCAL_WEAPON_OVERRIDE_READY initial_bound=%d spawn_hook=1"), BoundAtStart);
}

void UOCLocalInboxWeaponOverrideSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        if (ActorSpawnedHandle.IsValid()) World->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
    }
    ActorSpawnedHandle.Reset();
    Super::Deinitialize();
}

void UOCLocalInboxWeaponOverrideSubsystem::HandleActorSpawned(AActor* Actor)
{
    AOCWeaponBase* Weapon = Cast<AOCWeaponBase>(Actor);
    UWorld* World = GetWorld();
    if (!Weapon || !World) return;

    const TWeakObjectPtr<AOCWeaponBase> WeakWeapon(Weapon);
    World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, WeakWeapon]()
    {
        if (AOCWeaponBase* LiveWeapon = WeakWeapon.Get()) ApplyLocalVisual(LiveWeapon);
    }));
}

bool UOCLocalInboxWeaponOverrideSubsystem::ResolveVisualForWeapon(AOCWeaponBase* Weapon,
    FString& OutObjectPath, float& OutDesiredLengthCm, FString& OutCategory) const
{
    if (!Weapon) return false;

    auto TryCategory = [&](const TCHAR* Category, const float LengthCm)
    {
        FString Path;
        if (!UOCLocalInboxRuntimeSubsystem::ResolveFirstAssetObjectPathForCategory(Category, Path)) return false;
        OutObjectPath = MoveTemp(Path);
        OutDesiredLengthCm = LengthCm;
        OutCategory = Category;
        return true;
    };

    if (Cast<AOCWeapon_AssaultRifle>(Weapon))
    {
        return TryCategory(TEXT("M16_M4"), 100.0f) || TryCategory(TEXT("AK47"), 88.0f);
    }
    if (Cast<AOCWeapon_SMG>(Weapon)) return TryCategory(TEXT("MP5"), 68.0f);
    if (Cast<AOCWeapon_Pistol>(Weapon)) return TryCategory(TEXT("M1911"), 23.0f);
    if (Cast<AOCWeapon_Sniper>(Weapon)) return TryCategory(TEXT("M700"), 112.0f);
    if (Cast<AOCWeapon_Shotgun>(Weapon)) return TryCategory(TEXT("REMINGTON870"), 100.0f);
    if (Cast<AOCWeapon_LMG>(Weapon)) return TryCategory(TEXT("M249"), 104.0f);
    if (Cast<AOCWeapon_M14>(Weapon)) return TryCategory(TEXT("M14"), 112.0f);
    if (Cast<AOCWeapon_Mac10>(Weapon)) return TryCategory(TEXT("MAC10"), 30.0f);
    if (Cast<AOCWeapon_Tec9>(Weapon)) return TryCategory(TEXT("TEC9"), 32.0f);
    if (Cast<AOCWeapon_LeverAction>(Weapon)) return TryCategory(TEXT("LEVER_ACTION"), 105.0f);
    if (Cast<AOCAntiArmorLauncher>(Weapon)) return TryCategory(TEXT("LAUNCHER"), 105.0f);
    return false;
}

void UOCLocalInboxWeaponOverrideSubsystem::ApplyLocalVisual(AOCWeaponBase* Weapon)
{
    if (!Weapon || Weapon->ActorHasTag(LocalBoundTag)) return;

    FString ObjectPath;
    FString Category;
    float DesiredLengthCm = 100.0f;
    if (!ResolveVisualForWeapon(Weapon, ObjectPath, DesiredLengthCm, Category)) return;

    USceneComponent* Root = Weapon->GetRootComponent();
    if (!Root) return;

    bool bBound = false;
    if (UStaticMesh* StaticMesh = LoadObject<UStaticMesh>(nullptr, *ObjectPath))
    {
        if (UStaticMeshComponent* Visual = AddVisual<UStaticMesh, UStaticMeshComponent>(
            Weapon, Root, StaticMesh, FName(TEXT("LocalInboxWeaponStatic")), DesiredLengthCm))
        {
            Visual->SetStaticMesh(StaticMesh);
            bBound = true;
        }
    }
    else if (USkeletalMesh* SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, *ObjectPath))
    {
        if (USkeletalMeshComponent* Visual = AddVisual<USkeletalMesh, USkeletalMeshComponent>(
            Weapon, Root, SkeletalMesh, FName(TEXT("LocalInboxWeaponSkeletal")), DesiredLengthCm))
        {
            Visual->SetSkeletalMeshAsset(SkeletalMesh);
            bBound = true;
        }
    }

    if (!bBound)
    {
        UE_LOG(LogTemp, Error, TEXT("PASS45_LOCAL_WEAPON_RUNTIME_FAIL category=%s asset=%s"),
            *Category, *ObjectPath);
        return;
    }

    HideOldWeaponPresentation(Weapon);
    Weapon->Tags.Add(LocalBoundTag);
    UE_LOG(LogTemp, Display, TEXT("PASS45_LOCAL_WEAPON_RUNTIME_BOUND weapon=%s category=%s asset=%s"),
        *Weapon->GetName(), *Category, *ObjectPath);
}
