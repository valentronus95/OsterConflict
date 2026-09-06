#include "OCReferenceDrivenResidentialValidationSubsystem.h"

#include "OCGameMode.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

namespace
{
    constexpr float ValidationDelaySeconds = 2.0f;

    bool IsRejectedGenericFamily(const FName Name)
    {
        return Name == TEXT("Buildings") ||
            Name == TEXT("ResidentialRoofs") ||
            Name == TEXT("ResidentialDetails") ||
            Name == TEXT("WoodFences") ||
            Name == TEXT("MetalFences") ||
            Name == TEXT("LightSheetFences");
    }

    bool ContainsRejectedPresentationToken(const FString& Value)
    {
        static const TCHAR* RejectedTokens[] =
        {
            TEXT("/AdvancedVillagePack/"),
            TEXT("AdvancedVillagePack"),
            TEXT("OCEnterableHouse"),
            TEXT("DarkTower"),
            TEXT("SteepRoofTower"),
            TEXT("SteepRoof"),
            TEXT("Shack"),
            TEXT("Tower")
        };

        for (const TCHAR* Token : RejectedTokens)
        {
            if (Value.Contains(Token, ESearchCase::IgnoreCase)) return true;
        }
        return false;
    }
}

bool UOCReferenceDrivenResidentialValidationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCReferenceDrivenResidentialValidationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    ValidationWorld = &InWorld;
    InWorld.GetTimerManager().SetTimer(
        ValidationTimer,
        this,
        &UOCReferenceDrivenResidentialValidationSubsystem::ValidateReferenceDrivenResidentialWorld,
        ValidationDelaySeconds,
        false);

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_SCHEDULED delay_s=%.1f mutation=0 periodic_scan=0"),
        ValidationDelaySeconds);
}

void UOCReferenceDrivenResidentialValidationSubsystem::Deinitialize()
{
    if (UWorld* World = ValidationWorld.Get())
    {
        World->GetTimerManager().ClearTimer(ValidationTimer);
    }
    ValidationWorld.Reset();
    Super::Deinitialize();
}

void UOCReferenceDrivenResidentialValidationSubsystem::ValidateReferenceDrivenResidentialWorld()
{
    UWorld* World = ValidationWorld.Get();
    if (!World) return;

    int32 GenericBuildingInstances = 0;
    int32 GenericRoofInstances = 0;
    int32 GenericDetailInstances = 0;
    int32 GenericPrivateFenceInstances = 0;
    int32 RejectedNamedActors = 0;
    int32 RejectedMeshComponents = 0;

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor || Actor->IsActorBeingDestroyed()) continue;

        const FString ActorIdentity = Actor->GetName() + TEXT("|") + Actor->GetClass()->GetPathName();
        if (ContainsRejectedPresentationToken(ActorIdentity))
        {
            ++RejectedNamedActors;
        }

        TInlineComponentArray<UInstancedStaticMeshComponent*> InstancedComponents;
        Actor->GetComponents(InstancedComponents);
        for (UInstancedStaticMeshComponent* Component : InstancedComponents)
        {
            if (!Component || !IsRejectedGenericFamily(Component->GetFName())) continue;

            const int32 Count = Component->GetInstanceCount();
            const FName Name = Component->GetFName();
            if (Name == TEXT("Buildings")) GenericBuildingInstances += Count;
            else if (Name == TEXT("ResidentialRoofs")) GenericRoofInstances += Count;
            else if (Name == TEXT("ResidentialDetails")) GenericDetailInstances += Count;
            else GenericPrivateFenceInstances += Count;
        }

        TInlineComponentArray<UStaticMeshComponent*> MeshComponents;
        Actor->GetComponents(MeshComponents);
        for (UStaticMeshComponent* Component : MeshComponents)
        {
            if (!Component) continue;
            FString MeshIdentity = Component->GetName();
            if (const UStaticMesh* Mesh = Component->GetStaticMesh())
            {
                MeshIdentity += TEXT("|");
                MeshIdentity += Mesh->GetPathName();
            }
            if (ContainsRejectedPresentationToken(MeshIdentity))
            {
                ++RejectedMeshComponents;
            }
        }
    }

    const bool bReady = GenericBuildingInstances == 0 &&
        GenericRoofInstances == 0 &&
        GenericDetailInstances == 0 &&
        GenericPrivateFenceInstances == 0 &&
        RejectedNamedActors == 0 &&
        RejectedMeshComponents == 0;

    if (bReady)
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_READY genericBuildings=0 genericRoofs=0 genericDetails=0 genericPrivateFences=0 rejectedNamedActors=0 rejectedMeshComponents=0 mutation=0 periodic_scan=0"));
    }
    else
    {
        UE_LOG(LogTemp, Error,
            TEXT("PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RUNTIME_FAIL genericBuildings=%d genericRoofs=%d genericDetails=%d genericPrivateFences=%d rejectedNamedActors=%d rejectedMeshComponents=%d mutation=0 primary_authoring_fix_required=1"),
            GenericBuildingInstances,
            GenericRoofInstances,
            GenericDetailInstances,
            GenericPrivateFenceInstances,
            RejectedNamedActors,
            RejectedMeshComponents);
    }
}
