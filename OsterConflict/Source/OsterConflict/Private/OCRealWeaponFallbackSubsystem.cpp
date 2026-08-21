#include "OCRealWeaponFallbackSubsystem.h"

#include "OCGameMode.h"
#include "OCWeaponBase.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    const FName RealFallbackTag(TEXT("OC_RealMeshFallbackApplied"));
    const FName ProductionVisualTag(TEXT("OC_ProductionWeaponVisual"));
    const FName RealFallbackComponentTag(TEXT("OC_RealFallbackWeaponVisual"));

    bool HasProductionVisual(const AOCWeaponBase& Weapon)
    {
        TArray<UActorComponent*> Components;
        Weapon.GetComponents(Components);
        for (const UActorComponent* Component : Components)
        {
            if (IsValid(Component) && Component->ComponentHasTag(ProductionVisualTag)) return true;
        }
        return false;
    }
}

bool UOCRealWeaponFallbackSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCRealWeaponFallbackSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    GenericMachineGun = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/R13/Weapons/machinegun.machinegun"));
    GenericPistol = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/R13/Weapons/pistol.pistol"));
    GenericSMG = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/R13/Weapons/uzi.uzi"));
    GenericShotgun = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/R13/Weapons/shotgun.shotgun"));

    InWorld.GetTimerManager().SetTimer(
        RefreshTimer,
        this,
        &UOCRealWeaponFallbackSubsystem::RefreshWeaponFallbacks,
        0.25f,
        true,
        0.20f);
}

void UOCRealWeaponFallbackSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(RefreshTimer);
    GenericMachineGun = nullptr;
    GenericPistol = nullptr;
    GenericSMG = nullptr;
    GenericShotgun = nullptr;
    Super::Deinitialize();
}

void UOCRealWeaponFallbackSubsystem::RefreshWeaponFallbacks()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<AOCWeaponBase> It(World); It; ++It)
    {
        AOCWeaponBase* Weapon = *It;
        if (!IsValid(Weapon) || Weapon->IsActorBeingDestroyed()) continue;
        if (Weapon->ActorHasTag(RealFallbackTag) || HasProductionVisual(*Weapon)) continue;

        const FString Name = Weapon->GetWeaponDisplayName();
        if (Name.Equals(TEXT("M249"), ESearchCase::IgnoreCase))
        {
            ApplyRealFallback(*Weapon, GenericMachineGun.Get(), 104.0f, TEXT("R13 generic machinegun"));
        }
        else if (Name.Equals(TEXT("M1911"), ESearchCase::IgnoreCase))
        {
            ApplyRealFallback(*Weapon, GenericPistol.Get(), 24.0f, TEXT("R13 generic pistol"));
        }
        else if (Name.Equals(TEXT("MAC-10"), ESearchCase::IgnoreCase) || Name.Equals(TEXT("MAC10"), ESearchCase::IgnoreCase))
        {
            ApplyRealFallback(*Weapon, GenericSMG.Get(), 31.0f, TEXT("R13 generic SMG"));
        }
        else if (Name.Equals(TEXT("Remington 870"), ESearchCase::IgnoreCase))
        {
            ApplyRealFallback(*Weapon, GenericShotgun.Get(), 103.0f, TEXT("R13 generic shotgun"));
        }
    }
}

bool UOCRealWeaponFallbackSubsystem::ApplyRealFallback(
    AOCWeaponBase& Weapon,
    UStaticMesh* Mesh,
    float DesiredLengthCm,
    const TCHAR* FallbackLabel)
{
    USceneComponent* RootComponent = Weapon.GetRootComponent();
    if (!IsValid(&Weapon) || Weapon.IsActorBeingDestroyed() || !IsValid(Mesh) || !IsValid(RootComponent))
    {
        return false;
    }

    const FBoxSphereBounds Bounds = Mesh->GetBounds();
    const FVector NativeSize = Bounds.BoxExtent * 2.0f;
    const float NativeLength = FMath::Max3(NativeSize.X, NativeSize.Y, NativeSize.Z);
    if (NativeLength <= 1.0f) return false;

    // Hide every old static source-only primitive before the real imported fallback is registered.
    TArray<UStaticMeshComponent*> StaticComponents;
    Weapon.GetComponents<UStaticMeshComponent>(StaticComponents);
    for (UStaticMeshComponent* Existing : StaticComponents)
    {
        if (!IsValid(Existing) || Existing->ComponentHasTag(RealFallbackComponentTag)) continue;
        Existing->SetVisibility(false, true);
        Existing->SetHiddenInGame(true, true);
        Existing->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    const FName ComponentName = MakeUniqueObjectName(
        &Weapon,
        UStaticMeshComponent::StaticClass(),
        FName(TEXT("OC_RealWeaponFallback")));
    UStaticMeshComponent* Visual = NewObject<UStaticMeshComponent>(&Weapon, ComponentName);
    if (!IsValid(Visual)) return false;

    Visual->SetupAttachment(RootComponent);
    Visual->SetStaticMesh(Mesh);
    Visual->SetRelativeLocation(FVector::ZeroVector);
    Visual->SetRelativeRotation(FRotator::ZeroRotator);
    Visual->SetRelativeScale3D(FVector(DesiredLengthCm / NativeLength));
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Visual->SetGenerateOverlapEvents(false);
    Visual->SetCanEverAffectNavigation(false);
    Visual->ComponentTags.Add(RealFallbackComponentTag);
    Weapon.AddInstanceComponent(Visual);
    Visual->RegisterComponent();

    Weapon.Tags.Add(RealFallbackTag);
    UE_LOG(LogTemp, Warning,
        TEXT("Weapon '%s' exact production visual is unavailable; primitive hidden and %s real-mesh fallback applied. Production verification remains OPEN."),
        *Weapon.GetWeaponDisplayName(), FallbackLabel);
    return true;
}
