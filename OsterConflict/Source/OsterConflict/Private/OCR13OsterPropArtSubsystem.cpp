#include "OCR13OsterPropArtSubsystem.h"

#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
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

    UInstancedStaticMeshComponent* FindISMInWorld(UWorld& World, const FName Name)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            if (UInstancedStaticMeshComponent* Component = FindISM(*It, Name)) return Component;
        }
        return nullptr;
    }

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh, const FName Name)
    {
        if (!Owner || !Root || !Mesh) return nullptr;

        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;

        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(TEXT("BlockAll"));
        Component->SetCastShadow(true);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void ClearFamilies(const TArray<UInstancedStaticMeshComponent*>& Families)
    {
        for (UInstancedStaticMeshComponent* Family : Families)
        {
            if (Family) Family->ClearInstances();
        }
    }

    bool IsUsableVerticalFencePanel(UStaticMesh* Mesh)
    {
        if (!Mesh) return false;

        const FVector Size = Mesh->GetBounds().BoxExtent * 2.0f;
        const float LongXY = FMath::Max(Size.X, Size.Y);
        const float ThinXY = FMath::Max(1.0f, FMath::Min(Size.X, Size.Y));

        // The roadside pack contains loose sheet-metal props as well as usable upright pieces.
        // Only accept a mesh whose authored Z dimension behaves like a vertical fence panel.
        // A sheet authored flat on the ground has a tiny Z extent and must never become a fence.
        return LongXY >= 50.0f && Size.Z >= 50.0f && Size.Z >= ThinXY * 1.25f;
    }

    bool AddFenceModules(UInstancedStaticMeshComponent* Proxy,
        const TArray<UInstancedStaticMeshComponent*>& Families, int32& OutAdded)
    {
        OutAdded = 0;
        if (!Proxy || Proxy->GetInstanceCount() <= 0 || Families.Num() == 0) return false;

        for (int32 ProxyIndex = 0; ProxyIndex < Proxy->GetInstanceCount(); ++ProxyIndex)
        {
            FTransform ProxyTransform;
            if (!Proxy->GetInstanceTransform(ProxyIndex, ProxyTransform, true))
            {
                ClearFamilies(Families);
                OutAdded = 0;
                return false;
            }

            const FVector ProxyScale = ProxyTransform.GetScale3D().GetAbs();
            const bool bDesiredLongX = ProxyScale.X >= ProxyScale.Y;
            const float DesiredLength = FMath::Max(100.0f,
                (bDesiredLongX ? ProxyScale.X : ProxyScale.Y) * 100.0f);
            const float DesiredHeight = FMath::Max(100.0f, ProxyScale.Z * 100.0f);

            UInstancedStaticMeshComponent* SampleFamily = Families[ProxyIndex % Families.Num()];
            UStaticMesh* SampleMesh = SampleFamily ? SampleFamily->GetStaticMesh() : nullptr;
            if (!SampleMesh)
            {
                ClearFamilies(Families);
                OutAdded = 0;
                return false;
            }

            const FVector SampleSize = SampleMesh->GetBounds().BoxExtent * 2.0f;
            const bool bSampleLongX = SampleSize.X >= SampleSize.Y;
            const float SampleLong = FMath::Max(1.0f, bSampleLongX ? SampleSize.X : SampleSize.Y);
            const float SampleHeight = FMath::Max(1.0f, SampleSize.Z);
            const float HeightScale = FMath::Clamp(DesiredHeight / SampleHeight, 0.55f, 2.20f);
            const float NaturalModuleLength = SampleLong * HeightScale;
            const int32 ModuleCount = FMath::Clamp(FMath::CeilToInt(DesiredLength / NaturalModuleLength), 1, 64);
            const float ModuleSpacing = DesiredLength / static_cast<float>(ModuleCount);

            const FVector DesiredAxisLocal = bDesiredLongX ? FVector::ForwardVector : FVector::RightVector;
            const FVector DesiredAxisWorld = ProxyTransform.GetRotation().RotateVector(DesiredAxisLocal).GetSafeNormal();

            for (int32 ModuleIndex = 0; ModuleIndex < ModuleCount; ++ModuleIndex)
            {
                UInstancedStaticMeshComponent* Target = Families[(ProxyIndex + ModuleIndex) % Families.Num()];
                UStaticMesh* Mesh = Target ? Target->GetStaticMesh() : nullptr;
                if (!Target || !Mesh)
                {
                    ClearFamilies(Families);
                    OutAdded = 0;
                    return false;
                }

                const FBoxSphereBounds Bounds = Mesh->GetBounds();
                const FVector MeshSize = Bounds.BoxExtent * 2.0f;
                const bool bMeshLongX = MeshSize.X >= MeshSize.Y;
                const float MeshLong = FMath::Max(1.0f, bMeshLongX ? MeshSize.X : MeshSize.Y);
                const float MeshHeight = FMath::Max(1.0f, MeshSize.Z);
                const float ZScale = FMath::Clamp(DesiredHeight / MeshHeight, 0.55f, 2.20f);

                FVector Scale(ZScale);
                if (bMeshLongX) Scale.X = ModuleSpacing / MeshLong;
                else Scale.Y = ModuleSpacing / MeshLong;

                FQuat Rotation = ProxyTransform.GetRotation();
                if (bDesiredLongX != bMeshLongX)
                {
                    Rotation = Rotation * FQuat(FVector::UpVector, FMath::DegreesToRadians(90.0f));
                }

                const float Along = -0.5f * DesiredLength + (static_cast<float>(ModuleIndex) + 0.5f) * ModuleSpacing;
                FVector Location = ProxyTransform.GetLocation() + DesiredAxisWorld * Along;
                Location -= Rotation.RotateVector(Bounds.Origin * Scale);

                Target->AddInstance(FTransform(Rotation, Location, Scale), true);
                ++OutAdded;
            }
        }
        return OutAdded > 0;
    }

    bool AddVerticalPropReplacements(UInstancedStaticMeshComponent* Proxy,
        UInstancedStaticMeshComponent* Target, int32& OutAdded)
    {
        OutAdded = 0;
        if (!Proxy || Proxy->GetInstanceCount() <= 0 || !Target || !Target->GetStaticMesh()) return false;

        UStaticMesh* SourceMesh = Proxy->GetStaticMesh();
        UStaticMesh* TargetMesh = Target->GetStaticMesh();
        const FBoxSphereBounds TargetBounds = TargetMesh->GetBounds();
        const float TargetHeight = FMath::Max(1.0f, TargetBounds.BoxExtent.Z * 2.0f);
        const float SourceBaseHeight = SourceMesh
            ? FMath::Max(1.0f, SourceMesh->GetBounds().BoxExtent.Z * 2.0f)
            : TargetHeight;

        for (int32 Index = 0; Index < Proxy->GetInstanceCount(); ++Index)
        {
            FTransform ProxyTransform;
            if (!Proxy->GetInstanceTransform(Index, ProxyTransform, true))
            {
                Target->ClearInstances();
                OutAdded = 0;
                return false;
            }

            const float DesiredHeight = SourceBaseHeight * FMath::Max(0.01f, FMath::Abs(ProxyTransform.GetScale3D().Z));
            const float UniformScale = FMath::Clamp(DesiredHeight / TargetHeight, 0.60f, 1.80f);
            const FVector Scale(UniformScale);
            const FRotator Rotation(0.0f, ProxyTransform.Rotator().Yaw, 0.0f);

            FVector Location = ProxyTransform.GetLocation();
            const float TargetBottom = TargetBounds.Origin.Z - TargetBounds.BoxExtent.Z;
            Location.Z -= TargetBottom * UniformScale;

            Target->AddInstance(FTransform(Rotation, Location, Scale), true);
            ++OutAdded;
        }
        return OutAdded == Proxy->GetInstanceCount();
    }

    void HideProxyIfFullyReplaced(UInstancedStaticMeshComponent* Proxy, const bool bComplete, const int32 Added)
    {
        if (!Proxy || !bComplete || Added <= 0) return;
        Proxy->SetVisibility(false, true);
        Proxy->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

bool UOCR13OsterPropArtSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13OsterPropArtSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ApplyPropBridge(*World);
        }), 1.05f, false);
}

void UOCR13OsterPropArtSubsystem::ApplyPropBridge(UWorld& World)
{
    AOCWorldSectorOster* WorldSector = nullptr;
    for (TActorIterator<AOCWorldSectorOster> It(&World); It; ++It)
    {
        WorldSector = *It;
        if (WorldSector) break;
    }
    if (!WorldSector) return;

    UInstancedStaticMeshComponent* WoodProxy = FindISM(WorldSector, TEXT("WoodFences"));
    UInstancedStaticMeshComponent* LightSheetProxy = FindISM(WorldSector, TEXT("LightSheetFences"));
    UInstancedStaticMeshComponent* StreetLightProxy = FindISMInWorld(World, TEXT("R12_StreetLights"));
    const bool bHasWoodProxy = WoodProxy && WoodProxy->GetInstanceCount() > 0;
    const bool bHasLightSheetProxy = LightSheetProxy && LightSheetProxy->GetInstanceCount() > 0;
    const bool bHasStreetLightProxy = StreetLightProxy && StreetLightProxy->GetInstanceCount() > 0;
    if (!bHasWoodProxy && !bHasLightSheetProxy && !bHasStreetLightProxy) return;

    UStaticMesh* Fence01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Fence_Old_1_2m.Fence_Old_1_2m"));
    UStaticMesh* Fence02 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Fence_Old_2_2m.Fence_Old_2_2m"));
    UStaticMesh* Fence03 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Fence_Old_3_2m.Fence_Old_3_2m"));

    UStaticMesh* Sheet01 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_RoadsideConstruction/Assets/MS/3D/Urb_Roa_Sheet_Metal_Rusty_01/SM_Urb_Roa_Sheet_Metal_Rusty_01.SM_Urb_Roa_Sheet_Metal_Rusty_01"));
    UStaticMesh* Sheet02 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_RoadsideConstruction/Assets/MS/3D/Urb_Roa_Sheet_Metal_Rusty_02/SM_Urb_Roa_Sheet_Metal_Rusty_02.SM_Urb_Roa_Sheet_Metal_Rusty_02"));
    UStaticMesh* Sheet03 = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_RoadsideConstruction/Assets/MS/3D/Urb_Roa_Sheet_Metal_Rusty_03/SM_Urb_Roa_Sheet_Metal_Rusty_03.SM_Urb_Roa_Sheet_Metal_Rusty_03"));

    UStaticMesh* PowerPoleLight = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Modular_Rural_Cabin/Meshes/Props/Power_Pole_Light.Power_Pole_Light"));

    if (!IsUsableVerticalFencePanel(Sheet01)) Sheet01 = nullptr;
    if (!IsUsableVerticalFencePanel(Sheet02)) Sheet02 = nullptr;
    if (!IsUsableVerticalFencePanel(Sheet03)) Sheet03 = nullptr;

    if (!Fence01 && !Fence02 && !Fence03 && !Sheet01 && !Sheet02 && !Sheet03 && !PowerPoleLight)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("R13 Oster props: bundled prop meshes unavailable or unsuitable; preserving source proxies."));
        return;
    }

    AActor* ArtRoot = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
    if (!ArtRoot) return;
    ArtRoot->SetReplicates(false);

    USceneComponent* Root = NewObject<USceneComponent>(ArtRoot, TEXT("R13_OsterPropArtRoot"));
    if (!Root)
    {
        ArtRoot->Destroy();
        return;
    }
    Root->SetMobility(EComponentMobility::Static);
    ArtRoot->SetRootComponent(Root);
    ArtRoot->AddInstanceComponent(Root);
    Root->RegisterComponent();

    TArray<UInstancedStaticMeshComponent*> FenceFamilies;
    if (UInstancedStaticMeshComponent* Family = MakeISM(ArtRoot, Root, Fence01, TEXT("R13_WoodFence01"))) FenceFamilies.Add(Family);
    if (UInstancedStaticMeshComponent* Family = MakeISM(ArtRoot, Root, Fence02, TEXT("R13_WoodFence02"))) FenceFamilies.Add(Family);
    if (UInstancedStaticMeshComponent* Family = MakeISM(ArtRoot, Root, Fence03, TEXT("R13_WoodFence03"))) FenceFamilies.Add(Family);

    TArray<UInstancedStaticMeshComponent*> LightSheetFamilies;
    if (UInstancedStaticMeshComponent* Family = MakeISM(ArtRoot, Root, Sheet01, TEXT("R13_LightSheetFence01"))) LightSheetFamilies.Add(Family);
    if (UInstancedStaticMeshComponent* Family = MakeISM(ArtRoot, Root, Sheet02, TEXT("R13_LightSheetFence02"))) LightSheetFamilies.Add(Family);
    if (UInstancedStaticMeshComponent* Family = MakeISM(ArtRoot, Root, Sheet03, TEXT("R13_LightSheetFence03"))) LightSheetFamilies.Add(Family);

    UInstancedStaticMeshComponent* PowerPoleLightISM =
        MakeISM(ArtRoot, Root, PowerPoleLight, TEXT("R13_KrushelnytskaPowerPoleLights"));

    int32 WoodAdded = 0;
    int32 LightSheetAdded = 0;
    int32 PowerPoleLightAdded = 0;
    const bool bWoodComplete = AddFenceModules(WoodProxy, FenceFamilies, WoodAdded);
    const bool bLightSheetComplete = AddFenceModules(LightSheetProxy, LightSheetFamilies, LightSheetAdded);
    const bool bPowerPoleLightComplete = AddVerticalPropReplacements(StreetLightProxy, PowerPoleLightISM, PowerPoleLightAdded);

    HideProxyIfFullyReplaced(WoodProxy, bWoodComplete, WoodAdded);
    HideProxyIfFullyReplaced(LightSheetProxy, bLightSheetComplete, LightSheetAdded);
    HideProxyIfFullyReplaced(StreetLightProxy, bPowerPoleLightComplete, PowerPoleLightAdded);

    if (!bWoodComplete && !bLightSheetComplete && !bPowerPoleLightComplete)
    {
        ArtRoot->Destroy();
    }

    UE_LOG(LogTemp, Display,
        TEXT("R13 Oster props: wood modules=%d complete=%d, light-sheet modules=%d complete=%d, Krushelnytska poles=%d complete=%d; source family hides only after complete replacement."),
        WoodAdded, bWoodComplete ? 1 : 0,
        LightSheetAdded, bLightSheetComplete ? 1 : 0,
        PowerPoleLightAdded, bPowerPoleLightComplete ? 1 : 0);
}
