#include "OCRecoveredRoadsidePropsSubsystem.h"

#include "OCGameMode.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

namespace
{
    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root,
        UStaticMesh* Mesh, const FName Name)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(TEXT("NoCollision"));
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCullDistances(0, 55000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddFitted(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        const FVector& Location, const FVector& DesiredSizeCm, const float YawDegrees)
    {
        if (!Component || !Mesh) return;
        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const FVector NativeSize = Bounds.BoxExtent * 2.0f;
        if (NativeSize.X <= 1.0f || NativeSize.Y <= 1.0f || NativeSize.Z <= 1.0f) return;

        const FVector Scale(
            DesiredSizeCm.X / NativeSize.X,
            DesiredSizeCm.Y / NativeSize.Y,
            DesiredSizeCm.Z / NativeSize.Z);
        const FQuat Rotation = FRotator(0.0f, YawDegrees, 0.0f).Quaternion();
        const FVector FittedLocation = Location - Rotation.RotateVector(Bounds.Origin * Scale);
        Component->AddInstance(FTransform(Rotation, FittedLocation, Scale), true);
    }
}

bool UOCRecoveredRoadsidePropsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCRecoveredRoadsidePropsSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (InWorld.GetNetMode() == NM_DedicatedServer) return;
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    InWorld.GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) Populate(*World);
        }));
}

void UOCRecoveredRoadsidePropsSubsystem::Populate(UWorld& World)
{
    if (bPopulated) return;
    bPopulated = true;

    UStaticMesh* WheelbarrowMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_RoadsideConstruction/Assets/MS/3D/Ind_Con_Wheelbarrow_Worn_01/SM_Ind_Con_Wheelbarrow_Worn_01.SM_Ind_Con_Wheelbarrow_Worn_01"));
    UStaticMesh* GravelMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_RoadsideConstruction/Assets/MS/3D/Ind_Con_Pile_Gravel_Crushed_01/SM_Ind_Con_Pile_Gravel_Crushed_01.SM_Ind_Con_Pile_Gravel_Crushed_01"));
    UStaticMesh* CableWheelMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_RoadsideConstruction/Assets/MS/3D/Ind_Con_CableWheel_Wood_L_01/SM_Ind_Con_CableWheel_Wood_L_01.SM_Ind_Con_CableWheel_Wood_L_01"));
    UStaticMesh* ShovelMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_RoadsideConstruction/Assets/MS/3D/Ind_Min_Tool_Shovel_Old_01/SM_Ind_Min_Tool_Shovel_Old_01.SM_Ind_Min_Tool_Shovel_Old_01"));
    UStaticMesh* ToolboxMesh = LoadObject<UStaticMesh>(nullptr,
        TEXT("/Game/Scene_RoadsideConstruction/Assets/MS/3D/Ind_Old_Toolbox_Metal_Green_02/SM_Ind_Old_Toolbox_Metal_Green_02.SM_Ind_Old_Toolbox_Metal_Green_02"));

    if (!WheelbarrowMesh && !GravelMesh && !CableWheelMesh && !ShovelMesh && !ToolboxMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Recovered roadside-props pass skipped: selected meshes were not loadable."));
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = TEXT("OC_RecoveredRoadsideProps");
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Actor = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
    if (!Actor) return;
    Actor->Tags.Add(TEXT("OC_RecoveredRoadsideProps"));

    USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("RecoveredRoadsidePropsRoot"));
    Actor->AddInstanceComponent(Root);
    Root->RegisterComponent();
    Actor->SetRootComponent(Root);

    UInstancedStaticMeshComponent* Wheelbarrows = MakeISM(Actor, Root, WheelbarrowMesh, TEXT("RecoveredWheelbarrows"));
    UInstancedStaticMeshComponent* GravelPiles = MakeISM(Actor, Root, GravelMesh, TEXT("RecoveredGravelPiles"));
    UInstancedStaticMeshComponent* CableWheels = MakeISM(Actor, Root, CableWheelMesh, TEXT("RecoveredCableWheels"));
    UInstancedStaticMeshComponent* Shovels = MakeISM(Actor, Root, ShovelMesh, TEXT("RecoveredShovels"));
    UInstancedStaticMeshComponent* Toolboxes = MakeISM(Actor, Root, ToolboxMesh, TEXT("RecoveredToolboxes"));

    // Dress only the restored unfinished-building shell. Nothing is scattered through named
    // Oster locations, and these props remain visual-only until the site is inspected in UE.
    const FVector Site(-69000.0f, 64500.0f, 0.0f);

    AddFitted(Wheelbarrows, WheelbarrowMesh, Site + FVector(-1550.0f, -950.0f, 0.0f),
        FVector(145.0f, 75.0f, 85.0f), 28.0f);
    AddFitted(Wheelbarrows, WheelbarrowMesh, Site + FVector(1450.0f, 1250.0f, 0.0f),
        FVector(140.0f, 72.0f, 82.0f), -61.0f);

    AddFitted(GravelPiles, GravelMesh, Site + FVector(-1850.0f, 1100.0f, 0.0f),
        FVector(230.0f, 210.0f, 85.0f), 17.0f);
    AddFitted(GravelPiles, GravelMesh, Site + FVector(1950.0f, -1200.0f, 0.0f),
        FVector(190.0f, 175.0f, 70.0f), -24.0f);

    AddFitted(CableWheels, CableWheelMesh, Site + FVector(1650.0f, 850.0f, 0.0f),
        FVector(175.0f, 90.0f, 175.0f), 72.0f);
    AddFitted(Shovels, ShovelMesh, Site + FVector(-1120.0f, 720.0f, 0.0f),
        FVector(24.0f, 18.0f, 145.0f), -16.0f);
    AddFitted(Toolboxes, ToolboxMesh, Site + FVector(-680.0f, -780.0f, 0.0f),
        FVector(62.0f, 28.0f, 25.0f), 11.0f);

    UE_LOG(LogTemp, Display,
        TEXT("Recovered construction site now uses wheelbarrows, gravel, cable wheel, shovel and toolbox models."));
}
