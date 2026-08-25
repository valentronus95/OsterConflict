#include "OCR140MuseumFacadeDetailSubsystem.h"

#include "OCMuseumBreakableWindow.h"
#include "OCGameMode.h"
#include "OCInteractableDoor.h"
#include "OCMuseumServiceDoubleDoor.h"
#include "OCWorldSectorOster.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr float R140FacadeDelaySeconds = 5.75f;
    constexpr float MuseumRadiusCm = 2600.0f;

    AActor* FindMuseumOwner(UWorld& World)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (Actor && Actor->ActorHasTag(TEXT("R137_MuseumPhotoModel"))) return Actor;
        }
        return nullptr;
    }

    UMaterialInstanceDynamic* MakeMID(AActor* Owner, UMaterialInterface* Base,
        const TCHAR* SemanticName, const FLinearColor& Color)
    {
        if (!Owner || !Base) return nullptr;
        const FName UniqueName = MakeUniqueObjectName(
            Owner, UMaterialInstanceDynamic::StaticClass(), FName(SemanticName));
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Owner, UniqueName);
        if (Material) Material->SetVectorParameterValue(TEXT("Color"), Color);
        return Material;
    }

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        UMaterialInterface* Material, const TCHAR* SemanticName, const bool bCollision = false)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        const FName UniqueName = MakeUniqueObjectName(
            Owner, UInstancedStaticMeshComponent::StaticClass(), FName(SemanticName));
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, UniqueName);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material) Component->SetMaterial(0, Material);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(FName(bCollision ? TEXT("BlockAll") : TEXT("NoCollision")));
        Component->SetCollisionEnabled(bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(bCollision);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Center,
        const FVector& SizeCm, const FRotator& Rotation = FRotator::ZeroRotator)
    {
        if (!Component || SizeCm.GetMin() <= 0.0f) return;
        Component->AddInstance(FTransform(Rotation, Center, SizeCm / 100.0f), true);
    }

    void AddFitted(UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh,
        const FVector& Center, const FVector& DesiredSizeCm, const float YawDegrees)
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
        const FVector Location = Center - Rotation.RotateVector(Bounds.Origin * Scale);
        Component->AddInstance(FTransform(Rotation, Location, Scale), true);
    }

    void AddServiceGable(UInstancedStaticMeshComponent* Timber, UInstancedStaticMeshComponent* Trim,
        UInstancedStaticMeshComponent* Dentils, const FVector& Museum)
    {
        // REF-20: red/brown timber gable sits above the same end wall as the service double door.
        // The middle courses are split so the upper three-panel window is a true opening rather than glass on timber.
        for (int32 Course = 0; Course < 8; ++Course)
        {
            const float Z = 424.0f + static_cast<float>(Course) * 36.0f;
            const float TotalWidthY = 790.0f - static_cast<float>(Course) * 86.0f;
            const bool bCrossesWindow = Z >= 510.0f && Z <= 630.0f;
            if (!bCrossesWindow)
            {
                AddBox(Timber, Museum + FVector(871.0f, 0.0f, Z), FVector(18.0f, TotalWidthY, 30.0f));
                continue;
            }

            constexpr float WindowWidthY = 245.0f;
            const float SideWidth = FMath::Max(0.0f, (TotalWidthY - WindowWidthY) * 0.5f);
            if (SideWidth > 4.0f)
            {
                const float OffsetY = WindowWidthY * 0.5f + SideWidth * 0.5f;
                AddBox(Timber, Museum + FVector(871.0f, -OffsetY, Z), FVector(18.0f, SideWidth, 30.0f));
                AddBox(Timber, Museum + FVector(871.0f,  OffsetY, Z), FVector(18.0f, SideWidth, 30.0f));
            }
        }

        // Vertical board rhythm visible on the large gable.
        for (int32 Index = -5; Index <= 5; ++Index)
        {
            const float Y = static_cast<float>(Index) * 58.0f;
            const float Height = 210.0f - FMath::Abs(static_cast<float>(Index)) * 18.0f;
            AddBox(Trim, Museum + FVector(883.0f, Y, 560.0f), FVector(7.0f, 8.0f, Height));
        }

        // Pale carved horizontal band and dense brick dentils below the timber gable.
        AddBox(Trim, Museum + FVector(884.0f, 0.0f, 412.0f), FVector(10.0f, 825.0f, 18.0f));
        for (int32 Index = -9; Index <= 9; ++Index)
        {
            const float Y = static_cast<float>(Index) * 40.0f;
            AddBox(Dentils, Museum + FVector(881.0f, Y, 382.0f), FVector(24.0f, 20.0f, 32.0f));
            AddBox(Trim, Museum + FVector(887.0f, Y + 10.0f, 414.0f), FVector(7.0f, 18.0f, 12.0f),
                FRotator(0.0f, 0.0f, (Index % 2 == 0) ? 18.0f : -18.0f));
        }

        // Corner pilasters and window sill echo the photographed brick relief.
        AddBox(Dentils, Museum + FVector(880.0f, -405.0f, 235.0f), FVector(45.0f, 55.0f, 305.0f));
        AddBox(Dentils, Museum + FVector(880.0f,  405.0f, 235.0f), FVector(45.0f, 55.0f, 305.0f));
        AddBox(Dentils, Museum + FVector(889.0f, -215.0f, 123.0f), FVector(25.0f, 180.0f, 22.0f));

        // Upper window surround. The breakable glass itself is spawned separately on authority.
        AddBox(Trim, Museum + FVector(889.0f, -129.0f, 575.0f), FVector(10.0f, 16.0f, 132.0f));
        AddBox(Trim, Museum + FVector(889.0f,  129.0f, 575.0f), FVector(10.0f, 16.0f, 132.0f));
        AddBox(Trim, Museum + FVector(889.0f, 0.0f, 647.0f), FVector(10.0f, 274.0f, 14.0f));
        AddBox(Trim, Museum + FVector(889.0f, 0.0f, 503.0f), FVector(10.0f, 274.0f, 14.0f));
        AddBox(Trim, Museum + FVector(890.0f, -43.0f, 575.0f), FVector(9.0f, 8.0f, 120.0f));
        AddBox(Trim, Museum + FVector(890.0f,  43.0f, 575.0f), FVector(9.0f, 8.0f, 120.0f));
    }

    void AddGableWindowGrille(UInstancedStaticMeshComponent* Grilles, const FVector& Museum)
    {
        const FVector Center = Museum + FVector(897.0f, 0.0f, 575.0f);
        // REF-20 shows the same fan-like security grille language as the ground-floor windows.
        AddBox(Grilles, Center + FVector(0.0f, 0.0f, -51.0f), FVector(5.0f, 218.0f, 5.0f));
        for (const float Roll : { -58.0f, -35.0f, 0.0f, 35.0f, 58.0f })
        {
            AddBox(Grilles, Center + FVector(0.0f, 0.0f, -8.0f), FVector(5.0f, 122.0f, 5.0f),
                FRotator(0.0f, 0.0f, Roll));
        }
    }

    void AddFacadeUtilities(UInstancedStaticMeshComponent* Plaques, UInstancedStaticMeshComponent* Utilities,
        UInstancedStaticMeshComponent* Trim, const FVector& Museum)
    {
        // REF-02/REF-08/REF-12: memorial plaque and clustered electrical boxes on the entrance-side brick facade.
        AddBox(Plaques, Museum + FVector(505.0f, -440.0f, 255.0f), FVector(92.0f, 8.0f, 58.0f));
        for (int32 Index = 0; Index < 4; ++Index)
        {
            AddBox(Utilities, Museum + FVector(325.0f + Index * 38.0f, -443.0f, 163.0f + (Index % 2) * 9.0f),
                FVector(26.0f, 10.0f, 32.0f));
        }
        AddBox(Utilities, Museum + FVector(455.0f, -444.0f, 194.0f), FVector(8.0f, 8.0f, 126.0f));

        // Window-sill projections visible in close references.
        for (const float X : { -650.0f, -355.0f, 355.0f, 650.0f })
        {
            AddBox(Trim, Museum + FVector(X, -444.0f, 124.0f), FVector(176.0f, 32.0f, 18.0f));
        }
    }

    void AddAddressNumber(AActor* DetailActor, USceneComponent* Root, const FVector& Museum)
    {
        if (!DetailActor || !Root) return;
        UTextRenderComponent* Number = NewObject<UTextRenderComponent>(DetailActor,
            MakeUniqueObjectName(DetailActor, UTextRenderComponent::StaticClass(), FName(TEXT("R140Museum_Address30"))));
        if (!Number) return;
        Number->SetupAttachment(Root);
        Number->SetText(FText::FromString(TEXT("30")));
        Number->SetHorizontalAlignment(EHTA_Center);
        Number->SetWorldSize(24.0f);
        Number->SetTextRenderColor(FColor(40, 40, 38, 255));
        Number->SetRelativeLocation(Museum + FVector(235.0f, -690.0f, 343.0f));
        Number->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
        Number->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        DetailActor->AddInstanceComponent(Number);
        Number->RegisterComponent();
    }

    void SpawnUpperGableWindow(UWorld& World, AActor* Owner, const FVector& Museum)
    {
        if (World.GetNetMode() == NM_Client) return;
        for (TActorIterator<AOCBreakableWindow> It(&World); It; ++It)
        {
            AOCBreakableWindow* Window = *It;
            if (Window && Window->ActorHasTag(TEXT("MuseumUpperGableWindow")) &&
                FVector::DistSquared2D(Window->GetActorLocation(), Museum) < FMath::Square(MuseumRadiusCm))
            {
                return;
            }
        }

        FActorSpawnParameters Params;
        Params.Owner = Owner;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AOCMuseumBreakableWindow* Window = World.SpawnActor<AOCMuseumBreakableWindow>(
            AOCMuseumBreakableWindow::StaticClass(),
            FTransform(FRotator(0.0f, 90.0f, 0.0f), Museum + FVector(889.0f, 0.0f, 575.0f), FVector(1.18f, 1.0f, 0.80f)),
            Params);
        if (!Window) return;
        Window->Tags.Add(TEXT("R138_MuseumInteractive"));
        Window->Tags.Add(TEXT("R140_MuseumInteractive"));
        Window->Tags.Add(TEXT("MuseumUpperGableWindow"));
    }

    void ReplaceServiceDoor(UWorld& World, AActor* Owner, const FVector& Museum)
    {
        if (World.GetNetMode() == NM_Client) return;
        for (TActorIterator<AOCMuseumServiceDoubleDoor> It(&World); It; ++It)
        {
            AOCMuseumServiceDoubleDoor* Door = *It;
            if (Door && Door->ActorHasTag(TEXT("MuseumServiceDoubleDoor")) &&
                FVector::DistSquared2D(Door->GetActorLocation(), Museum) < FMath::Square(MuseumRadiusCm))
            {
                return;
            }
        }

        TArray<TWeakObjectPtr<AOCInteractableDoor>> PrototypeDoors;
        for (TActorIterator<AOCInteractableDoor> It(&World); It; ++It)
        {
            AOCInteractableDoor* Door = *It;
            if (Door && Door->ActorHasTag(TEXT("MuseumServiceDoor")) &&
                FVector::DistSquared2D(Door->GetActorLocation(), Museum) < FMath::Square(MuseumRadiusCm))
            {
                PrototypeDoors.Add(Door);
            }
        }

        FActorSpawnParameters Params;
        Params.Owner = Owner;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AOCMuseumServiceDoubleDoor* FinalDoor = World.SpawnActor<AOCMuseumServiceDoubleDoor>(
            AOCMuseumServiceDoubleDoor::StaticClass(),
            FTransform(FRotator(0.0f, 90.0f, 0.0f), Museum + FVector(872.0f, 115.0f, 70.0f)), Params);
        if (!FinalDoor) return;

        FinalDoor->Tags.Add(TEXT("R138_MuseumInteractive"));
        FinalDoor->Tags.Add(TEXT("R140_MuseumInteractive"));
        FinalDoor->Tags.Add(TEXT("MuseumServiceDoubleDoor"));
        for (const TWeakObjectPtr<AOCInteractableDoor>& WeakDoor : PrototypeDoors)
        {
            if (AOCInteractableDoor* Door = WeakDoor.Get()) Door->Destroy();
        }
    }
}

bool UOCR140MuseumFacadeDetailSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR140MuseumFacadeDetailSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;
    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ApplyFacadeDetail(*World);
        }), R140FacadeDelaySeconds, false);
}

void UOCR140MuseumFacadeDetailSubsystem::ApplyFacadeDetail(UWorld& World) const
{
    const FVector Museum = AOCWorldSectorOster::MuseumAnchor();
    AActor* MuseumOwner = FindMuseumOwner(World);
    if (!MuseumOwner)
    {
        UE_LOG(LogTemp, Warning, TEXT("R14.0 museum facade skipped: R13.7 museum owner not found."));
        return;
    }

    bool bFacadeExists = false;
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        if (AActor* Actor = *It; Actor && Actor->ActorHasTag(TEXT("R140_MuseumFacadeDetail")))
        {
            bFacadeExists = true;
            break;
        }
    }

    if (!bFacadeExists)
    {
        UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
        UStaticMesh* RoofMesh = LoadObject<UStaticMesh>(nullptr,
            TEXT("/Game/Modular_Rural_Cabin/Meshes/Modular/Roof_Both_Ends_4m.Roof_Both_Ends_4m"));
        UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
            TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
        UMaterialInterface* MetalRoof = LoadObject<UMaterialInterface>(nullptr,
            TEXT("/Game/Modular_Rural_Cabin/Materials/Instances/Metal_Roof.Metal_Roof"));
        if (!Cube || !Basic) return;

        AActor* DetailActor = World.SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
        if (!DetailActor) return;
        DetailActor->SetReplicates(false);
        DetailActor->Tags.Add(TEXT("R140_MuseumFacadeDetail"));

        USceneComponent* Root = NewObject<USceneComponent>(DetailActor,
            MakeUniqueObjectName(DetailActor, USceneComponent::StaticClass(), FName(TEXT("R140MuseumFacadeRoot"))));
        if (!Root)
        {
            DetailActor->Destroy();
            return;
        }
        DetailActor->SetRootComponent(Root);
        DetailActor->AddInstanceComponent(Root);
        Root->RegisterComponent();

        UMaterialInstanceDynamic* TimberMat = MakeMID(DetailActor, Basic, TEXT("R140MuseumMID_GableTimber"),
            FLinearColor(0.46f, 0.19f, 0.105f, 1.0f));
        UMaterialInstanceDynamic* PaleMat = MakeMID(DetailActor, Basic, TEXT("R140MuseumMID_CarvedTrim"),
            FLinearColor(0.79f, 0.75f, 0.63f, 1.0f));
        UMaterialInstanceDynamic* BrickDarkMat = MakeMID(DetailActor, Basic, TEXT("R140MuseumMID_DentilBrick"),
            FLinearColor(0.36f, 0.12f, 0.055f, 1.0f));
        UMaterialInstanceDynamic* GrilleMat = MakeMID(DetailActor, Basic, TEXT("R140MuseumMID_Grille"),
            FLinearColor(0.43f, 0.48f, 0.47f, 1.0f));
        UMaterialInstanceDynamic* PlaqueMat = MakeMID(DetailActor, Basic, TEXT("R140MuseumMID_MemorialPlaque"),
            FLinearColor(0.13f, 0.22f, 0.24f, 1.0f));
        UMaterialInstanceDynamic* UtilityMat = MakeMID(DetailActor, Basic, TEXT("R140MuseumMID_Utility"),
            FLinearColor(0.72f, 0.72f, 0.67f, 1.0f));

        UInstancedStaticMeshComponent* Timber = MakeISM(DetailActor, Root, Cube, TimberMat,
            TEXT("R140Museum_ServiceGableTimber"));
        UInstancedStaticMeshComponent* Trim = MakeISM(DetailActor, Root, Cube, PaleMat,
            TEXT("R140Museum_CarvedTrim"));
        UInstancedStaticMeshComponent* Dentils = MakeISM(DetailActor, Root, Cube, BrickDarkMat,
            TEXT("R140Museum_BrickDentils"));
        UInstancedStaticMeshComponent* Grilles = MakeISM(DetailActor, Root, Cube, GrilleMat,
            TEXT("R140Museum_GableGrille"));
        UInstancedStaticMeshComponent* Plaques = MakeISM(DetailActor, Root, Cube, PlaqueMat,
            TEXT("R140Museum_Plaques"));
        UInstancedStaticMeshComponent* Utilities = MakeISM(DetailActor, Root, Cube, UtilityMat,
            TEXT("R140Museum_Utilities"));

        AddServiceGable(Timber, Trim, Dentils, Museum);
        AddGableWindowGrille(Grilles, Museum);
        AddFacadeUtilities(Plaques, Utilities, Trim, Museum);
        AddAddressNumber(DetailActor, Root, Museum);

        if (RoofMesh)
        {
            UInstancedStaticMeshComponent* Canopy = MakeISM(DetailActor, Root, RoofMesh,
                MetalRoof ? MetalRoof : Basic, TEXT("R140Museum_ServiceCanopy"));
            AddFitted(Canopy, RoofMesh, Museum + FVector(985.0f, 115.0f, 390.0f),
                FVector(250.0f, 255.0f, 105.0f), 90.0f);
        }

        UE_LOG(LogTemp, Display,
            TEXT("PASS45_MUSEUM_R140_DETAIL_ONLY_READY late_r137_suppression=0 instance_removal=0; service gable/canopy/final door and upper glass authored directly from references."));
    }

    ReplaceServiceDoor(World, MuseumOwner, Museum);
    SpawnUpperGableWindow(World, MuseumOwner, Museum);
}
