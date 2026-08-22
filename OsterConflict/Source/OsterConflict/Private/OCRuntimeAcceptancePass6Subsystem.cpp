#include "OCRuntimeAcceptancePass6Subsystem.h"

#include "OCWeaponBase.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
    const FName ProductionWeaponVisualTag(TEXT("OC_ProductionWeaponVisual"));
    const FName Pass6AxisNormalizedTag(TEXT("OC_Pass6AxisNormalized"));

    constexpr float LegacyBaseCleanupRadiusCm = 11000.0f;
    const FVector LegacyBaseCenters[] =
    {
        FVector(-104000.0f, -92000.0f, 0.0f),
        FVector( 104000.0f,  92000.0f, 0.0f)
    };

    bool IsLegacyBaseOwnedComponent(const UInstancedStaticMeshComponent* Component)
    {
        if (!Component) return false;
        const FString Name = Component->GetName();
        static const TCHAR* OwnedNames[] =
        {
            TEXT("Sidewalks"),
            TEXT("Roads"),
            TEXT("Buildings"),
            TEXT("ResidentialRoofs"),
            TEXT("ParkDetails"),
            TEXT("MetalFences")
        };
        for (const TCHAR* OwnedName : OwnedNames)
        {
            if (Name.Equals(OwnedName, ESearchCase::IgnoreCase) || Name.StartsWith(OwnedName, ESearchCase::IgnoreCase))
            {
                return true;
            }
        }
        return false;
    }

    FQuat ResolveLongAxisToForward(const FVector& NativeSize)
    {
        FVector NativeForward = FVector::ForwardVector;
        if (NativeSize.Y >= NativeSize.X && NativeSize.Y >= NativeSize.Z)
        {
            NativeForward = FVector::RightVector;
        }
        else if (NativeSize.Z >= NativeSize.X && NativeSize.Z >= NativeSize.Y)
        {
            NativeForward = FVector::UpVector;
        }
        return FQuat::FindBetweenNormals(NativeForward, FVector::ForwardVector);
    }

    void MakeHiddenStaticGeometryInert(AOCWeaponBase& Weapon)
    {
        TInlineComponentArray<UStaticMeshComponent*> Components;
        Weapon.GetComponents(Components);
        for (UStaticMeshComponent* Component : Components)
        {
            if (!Component || Component->ComponentHasTag(ProductionWeaponVisualTag)) continue;
            if (!Component->IsVisible())
            {
                Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                Component->SetGenerateOverlapEvents(false);
                Component->SetCanEverAffectNavigation(false);
                Component->SetCastShadow(false);
            }
        }
    }
}

bool UOCRuntimeAcceptancePass6Subsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCRuntimeAcceptancePass6Subsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    ApplyAcceptanceCorrections();
    InWorld.GetTimerManager().SetTimer(
        AcceptanceTimer,
        this,
        &UOCRuntimeAcceptancePass6Subsystem::ApplyAcceptanceCorrections,
        0.25f,
        true,
        0.25f);
}

void UOCRuntimeAcceptancePass6Subsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AcceptanceTimer);
    }
    Super::Deinitialize();
}

void UOCRuntimeAcceptancePass6Subsystem::ApplyAcceptanceCorrections()
{
    if (!bLegacyBaseCleanupComplete)
    {
        RemoveLegacyGameplayBaseInstances();
    }
    NormalizeProductionStaticWeapons();
}

void UOCRuntimeAcceptancePass6Subsystem::RemoveLegacyGameplayBaseInstances()
{
    UWorld* World = GetWorld();
    if (!World) return;

    bool bFoundSector = false;
    int32 RemovedInstances = 0;
    const float RadiusSquared = FMath::Square(LegacyBaseCleanupRadiusCm);

    for (TActorIterator<AOCWorldSectorOster> It(World); It; ++It)
    {
        AOCWorldSectorOster* Sector = *It;
        if (!IsValid(Sector)) continue;
        bFoundSector = true;

        TInlineComponentArray<UInstancedStaticMeshComponent*> Components;
        Sector->GetComponents(Components);
        for (UInstancedStaticMeshComponent* Component : Components)
        {
            if (!IsLegacyBaseOwnedComponent(Component)) continue;

            for (int32 InstanceIndex = Component->GetInstanceCount() - 1; InstanceIndex >= 0; --InstanceIndex)
            {
                FTransform InstanceTransform;
                if (!Component->GetInstanceTransform(InstanceIndex, InstanceTransform, true)) continue;

                const FVector Location = InstanceTransform.GetLocation();
                bool bInsideLegacyBase = false;
                for (const FVector& BaseCenter : LegacyBaseCenters)
                {
                    const FVector2D Delta(Location.X - BaseCenter.X, Location.Y - BaseCenter.Y);
                    if (Delta.SizeSquared() <= RadiusSquared)
                    {
                        bInsideLegacyBase = true;
                        break;
                    }
                }

                if (bInsideLegacyBase && Component->RemoveInstance(InstanceIndex))
                {
                    ++RemovedInstances;
                }
            }
        }
    }

    if (!bFoundSector) return;

    bLegacyBaseCleanupComplete = true;
    UE_LOG(LogTemp, Display,
        TEXT("Pass 6 legacy BASE cleanup complete: removed %d source-only instance(s) around the two obsolete gameplay-base centers."),
        RemovedInstances);
}

void UOCRuntimeAcceptancePass6Subsystem::NormalizeProductionStaticWeapons()
{
    UWorld* World = GetWorld();
    if (!World) return;

    UMaterialInterface* NeutralBaseMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    for (TActorIterator<AOCWeaponBase> It(World); It; ++It)
    {
        AOCWeaponBase* Weapon = *It;
        if (!IsValid(Weapon) || Weapon->IsActorBeingDestroyed()) continue;

        MakeHiddenStaticGeometryInert(*Weapon);

        TInlineComponentArray<UStaticMeshComponent*> Components;
        Weapon->GetComponents(Components);
        for (UStaticMeshComponent* Component : Components)
        {
            if (!Component || !Component->ComponentHasTag(ProductionWeaponVisualTag) ||
                Component->ComponentHasTag(Pass6AxisNormalizedTag))
            {
                continue;
            }

            UStaticMesh* Mesh = Component->GetStaticMesh();
            if (!Mesh) continue;

            const FBoxSphereBounds Bounds = Mesh->GetBounds();
            const FVector NativeSize = Bounds.BoxExtent * 2.0f;
            if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) continue;

            const FQuat AxisCorrection = ResolveLongAxisToForward(NativeSize);
            const FVector Scale = Component->GetRelativeScale3D();
            const FVector ScaledOrigin(
                Bounds.Origin.X * Scale.X,
                Bounds.Origin.Y * Scale.Y,
                Bounds.Origin.Z * Scale.Z);

            // Restored R13 assets frequently carry SKM_* names despite being StaticMesh and several are
            // authored with their long axis on Y/Z. Normalize that long axis once at the production component;
            // the weapon actor can then use the same X-forward gameplay/pickup convention as the rest of the game.
            Component->SetRelativeRotation(AxisCorrection.Rotator());
            Component->SetRelativeLocation(-AxisCorrection.RotateVector(ScaledOrigin));
            Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Component->SetGenerateOverlapEvents(false);
            Component->SetCanEverAffectNavigation(false);

            // Preserve every valid imported material. Only an actually empty slot receives a dark neutral
            // emergency material, preventing the old bright-white proxy look without hiding missing asset data.
            UMaterialInstanceDynamic* NeutralMID = nullptr;
            const int32 MaterialCount = Component->GetNumMaterials();
            for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
            {
                if (Component->GetMaterial(MaterialIndex) || !NeutralBaseMaterial) continue;
                if (!NeutralMID)
                {
                    NeutralMID = UMaterialInstanceDynamic::Create(NeutralBaseMaterial, Component);
                    if (NeutralMID)
                    {
                        NeutralMID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.055f, 0.060f, 0.058f, 1.0f));
                    }
                }
                if (NeutralMID) Component->SetMaterial(MaterialIndex, NeutralMID);
            }

            Component->ComponentTags.AddUnique(Pass6AxisNormalizedTag);
            UE_LOG(LogTemp, Display,
                TEXT("Pass 6 normalized production StaticMesh weapon '%s' (%s) to X-forward; imported materials preserved."),
                *Weapon->GetWeaponDisplayName(), *Mesh->GetName());
        }
    }
}
