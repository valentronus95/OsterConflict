#include "OCLocalInboxWeaponOverrideSubsystem.h"

#include "OCLocalInboxRuntimeSubsystem.h"
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
    constexpr float DesiredM16LengthCm = 100.0f;

    template <typename TMesh, typename TComponent>
    TComponent* AddVisual(AOCWeapon_AssaultRifle* Weapon, USceneComponent* Root, TMesh* Mesh,
        const FName BaseName)
    {
        if (!Weapon || !Root || !Mesh) return nullptr;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        const float Longest = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
        if (Longest <= 1.0f) return nullptr;

        const FName UniqueName = MakeUniqueObjectName(Weapon, TComponent::StaticClass(), BaseName);
        TComponent* Visual = NewObject<TComponent>(Weapon, UniqueName);
        if (!Visual) return nullptr;
        const float UniformScale = DesiredM16LengthCm / Longest;
        Visual->SetupAttachment(Root);
        Visual->SetRelativeLocation(-Bounds.Origin * UniformScale);
        Visual->SetRelativeRotation(FRotator::ZeroRotator);
        Visual->SetRelativeScale3D(FVector(UniformScale));
        Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Visual->SetGenerateOverlapEvents(false);
        Visual->SetCanEverAffectNavigation(false);
        Visual->SetCastShadow(true);
        Visual->ComponentTags.Add(FName(TEXT("OC_LocalInboxM16Visual")));
        Weapon->AddInstanceComponent(Visual);
        Visual->RegisterComponent();
        return Visual;
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
    if (!UOCLocalInboxRuntimeSubsystem::ResolveFirstAssetObjectPathForCategory(TEXT("M16_M4"), M16ObjectPath)) return;

    for (TActorIterator<AOCWeapon_AssaultRifle> It(&InWorld); It; ++It)
    {
        ApplyM16Visual(*It);
    }

    ActorSpawnedHandle = InWorld.AddOnActorSpawnedHandler(
        FOnActorSpawned::FDelegate::CreateUObject(this, &UOCLocalInboxWeaponOverrideSubsystem::HandleActorSpawned));
    UE_LOG(LogTemp, Display, TEXT("PASS45_LOCAL_M16_OVERRIDE_READY source=%s spawn_hook=1"), *M16ObjectPath);
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
    AOCWeapon_AssaultRifle* Weapon = Cast<AOCWeapon_AssaultRifle>(Actor);
    UWorld* World = GetWorld();
    if (!Weapon || !World) return;

    const TWeakObjectPtr<AOCWeapon_AssaultRifle> WeakWeapon(Weapon);
    World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, WeakWeapon]()
    {
        if (AOCWeapon_AssaultRifle* LiveWeapon = WeakWeapon.Get()) ApplyM16Visual(LiveWeapon);
    }));
}

void UOCLocalInboxWeaponOverrideSubsystem::ApplyM16Visual(AOCWeapon_AssaultRifle* Weapon)
{
    if (!Weapon || M16ObjectPath.IsEmpty() || Weapon->ActorHasTag(FName(TEXT("OC_LocalM16Bound")))) return;
    USceneComponent* Root = Weapon->GetRootComponent();
    if (!Root) return;

    // Keep collision/gameplay components alive but stop every old mesh from rendering before adding the user's model.
    TInlineComponentArray<UStaticMeshComponent*> StaticComponents;
    Weapon->GetComponents(StaticComponents);
    for (UStaticMeshComponent* Component : StaticComponents)
    {
        if (Component && !Component->ComponentHasTag(FName(TEXT("OC_LocalInboxM16Visual"))))
        {
            Component->SetVisibility(false, true);
            Component->SetHiddenInGame(true, true);
        }
    }
    TInlineComponentArray<USkeletalMeshComponent*> SkeletalComponents;
    Weapon->GetComponents(SkeletalComponents);
    for (USkeletalMeshComponent* Component : SkeletalComponents)
    {
        if (Component && !Component->ComponentHasTag(FName(TEXT("OC_LocalInboxM16Visual"))))
        {
            Component->SetVisibility(false, true);
            Component->SetHiddenInGame(true, true);
        }
    }

    bool bBound = false;
    if (UStaticMesh* StaticMesh = LoadObject<UStaticMesh>(nullptr, *M16ObjectPath))
    {
        if (UStaticMeshComponent* Visual = AddVisual<UStaticMesh, UStaticMeshComponent>(
            Weapon, Root, StaticMesh, FName(TEXT("LocalInboxM16Static"))))
        {
            Visual->SetStaticMesh(StaticMesh);
            bBound = true;
        }
    }
    else if (USkeletalMesh* SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, *M16ObjectPath))
    {
        if (USkeletalMeshComponent* Visual = AddVisual<USkeletalMesh, USkeletalMeshComponent>(
            Weapon, Root, SkeletalMesh, FName(TEXT("LocalInboxM16Skeletal"))))
        {
            Visual->SetSkeletalMeshAsset(SkeletalMesh);
            bBound = true;
        }
    }

    if (bBound)
    {
        Weapon->Tags.Add(FName(TEXT("OC_LocalM16Bound")));
        UE_LOG(LogTemp, Display, TEXT("PASS45_LOCAL_M16_RUNTIME_BOUND weapon=%s asset=%s"),
            *Weapon->GetName(), *M16ObjectPath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PASS45_LOCAL_M16_RUNTIME_FAIL asset=%s"), *M16ObjectPath);
    }
}
