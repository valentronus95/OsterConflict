#include "OCCombatVisualComponent.h"
#include "OCTransientVisualFX.h"

#include "OCCharacter.h"
#include "OCDamageTypes.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HAL/IConsoleManager.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include <initializer_list>

namespace
{
    TAutoConsoleVariable<int32> CVarOCGoreLevel(
        TEXT("oc.GoreLevel"),
        2,
        TEXT("Local gore presentation: 0=Off, 1=Reduced, 2=Full. Gameplay outcomes are unchanged."),
        ECVF_Default);

    int32 SeverityRank(EOCBloodSeverity Severity)
    {
        return static_cast<int32>(Severity);
    }

    EOCBloodSeverity RankToSeverity(int32 Rank)
    {
        return static_cast<EOCBloodSeverity>(FMath::Clamp(Rank, 0, 4));
    }

    bool NameContainsAny(const FString& Name, std::initializer_list<const TCHAR*> Tokens)
    {
        for (const TCHAR* Token : Tokens)
        {
            if (Name.Contains(Token, ESearchCase::IgnoreCase)) return true;
        }
        return false;
    }
}

UOCCombatVisualComponent::UOCCombatVisualComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UOCCombatVisualComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UOCCombatVisualComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UOCCombatVisualComponent, LastTraumaEvent);
}

bool UOCCombatVisualComponent::IsExplosiveDamageClass(TSubclassOf<UDamageType> DamageTypeClass)
{
    if (!DamageTypeClass) return false;
    UClass* DamageClass = DamageTypeClass.Get();
    return DamageClass && (DamageClass->IsChildOf(UOCExplosiveDamageType::StaticClass()) ||
        DamageClass->IsChildOf(UOCAntiArmorDamageType::StaticClass()) ||
        DamageClass->IsChildOf(UOCVehicleCannonDamageType::StaticClass()));
}

int32 UOCCombatVisualComponent::GoreLevelForLocalMachine()
{
    return FMath::Clamp(CVarOCGoreLevel.GetValueOnGameThread(), 0, 2);
}

EOCBodyZone UOCCombatVisualComponent::ResolveBodyZone(const FVector& HitLocation, FName BoneName) const
{
    const FString Bone = BoneName.ToString().ToLower();
    if (!Bone.IsEmpty())
    {
        if (NameContainsAny(Bone, {TEXT("head"), TEXT("neck") })) return EOCBodyZone::HeadNeck;
        if (NameContainsAny(Bone, {TEXT("pelvis"), TEXT("hip") })) return EOCBodyZone::Pelvis;
        if (NameContainsAny(Bone, {TEXT("upperarm_l"), TEXT("lowerarm_l"), TEXT("hand_l"), TEXT("arm_l") })) return EOCBodyZone::LeftArm;
        if (NameContainsAny(Bone, {TEXT("upperarm_r"), TEXT("lowerarm_r"), TEXT("hand_r"), TEXT("arm_r") })) return EOCBodyZone::RightArm;
        if (NameContainsAny(Bone, {TEXT("thigh_l"), TEXT("calf_l"), TEXT("foot_l"), TEXT("leg_l") })) return EOCBodyZone::LeftLeg;
        if (NameContainsAny(Bone, {TEXT("thigh_r"), TEXT("calf_r"), TEXT("foot_r"), TEXT("leg_r") })) return EOCBodyZone::RightLeg;
        if (NameContainsAny(Bone, {TEXT("spine"), TEXT("clavicle"), TEXT("chest") })) return EOCBodyZone::Torso;
    }

    const AOCCharacter* Character = Cast<AOCCharacter>(GetOwner());
    if (!Character) return EOCBodyZone::Unknown;

    const FVector Local = Character->GetActorTransform().InverseTransformPosition(HitLocation);
    if (Local.Z > 63.0f) return EOCBodyZone::HeadNeck;
    if (Local.Z > 15.0f) return EOCBodyZone::Torso;
    if (Local.Z > -15.0f) return EOCBodyZone::Pelvis;
    if (Local.Z > -60.0f) return Local.Y < 0.0f ? EOCBodyZone::LeftLeg : EOCBodyZone::RightLeg;
    return EOCBodyZone::Unknown;
}

EOCBloodSeverity UOCCombatVisualComponent::ResolveBloodSeverity(float Damage, EOCBodyZone Zone,
    EOCWeaponClass WeaponClass, bool bExplosive, bool bFatal) const
{
    int32 Rank = 0;
    if (Damage >= 8.0f) Rank = 1;
    if (Damage >= 22.0f) Rank = 2;
    if (Damage >= 45.0f) Rank = 3;
    if (Damage >= 85.0f) Rank = 4;

    if (Zone == EOCBodyZone::HeadNeck) ++Rank;
    if (WeaponClass == EOCWeaponClass::Shotgun || WeaponClass == EOCWeaponClass::SniperRifle || WeaponClass == EOCWeaponClass::LMG) ++Rank;
    if (bExplosive) Rank += 2;
    if (bFatal && Rank < 3) Rank = 3;
    return RankToSeverity(Rank);
}

void UOCCombatVisualComponent::ResolveDismemberment(FOCReplicatedTraumaEvent& Event) const
{
    Event.DismembermentSeverity = EOCDismembermentSeverity::None;
    Event.DismembermentMask = 0;
    if (!bAllowDismemberment || !Event.bFatal || Event.BloodSeverity != EOCBloodSeverity::Extreme)
    {
        return;
    }

    const bool bHighEnergyBallistic = Event.WeaponClass == EOCWeaponClass::Shotgun || Event.WeaponClass == EOCWeaponClass::SniperRifle;
    if (!Event.bExplosive && !bHighEnergyBallistic)
    {
        return;
    }

    auto AddPart = [&Event](EOCDismembermentPart Part)
    {
        Event.DismembermentMask |= static_cast<uint8>(Part);
    };

    switch (Event.BodyZone)
    {
        case EOCBodyZone::LeftArm: AddPart(EOCDismembermentPart::LeftArm); break;
        case EOCBodyZone::RightArm: AddPart(EOCDismembermentPart::RightArm); break;
        case EOCBodyZone::LeftLeg: AddPart(EOCDismembermentPart::LeftLeg); break;
        case EOCBodyZone::RightLeg: AddPart(EOCDismembermentPart::RightLeg); break;
        default: break;
    }

    if (Event.bExplosive && Event.Damage >= 120.0f)
    {
        AddPart(EOCDismembermentPart::LeftArm);
        AddPart(EOCDismembermentPart::RightArm);
        Event.DismembermentSeverity = EOCDismembermentSeverity::MultiPart;
    }
    if (Event.bExplosive && Event.Damage >= 180.0f)
    {
        AddPart(EOCDismembermentPart::LeftLeg);
        AddPart(EOCDismembermentPart::RightLeg);
        Event.DismembermentSeverity = EOCDismembermentSeverity::Catastrophic;
    }
    else if (Event.DismembermentMask != 0 && Event.DismembermentSeverity == EOCDismembermentSeverity::None)
    {
        const uint8 Mask = Event.DismembermentMask;
        const int32 Count = ((Mask & 1) ? 1 : 0) + ((Mask & 2) ? 1 : 0) + ((Mask & 4) ? 1 : 0) + ((Mask & 8) ? 1 : 0);
        Event.DismembermentSeverity = Count > 1 ? EOCDismembermentSeverity::MultiPart : EOCDismembermentSeverity::SingleLimb;
    }
}

void UOCCombatVisualComponent::RecordPointTraumaServer(float Damage, const FVector& HitLocation,
    const FVector& ShotDirection, FName BoneName, EOCWeaponClass WeaponClass,
    TSubclassOf<UDamageType> DamageTypeClass, bool bFatal)
{
    AActor* Owner = GetOwner();
    if (!Owner || !Owner->HasAuthority() || Damage <= 0.0f) return;

    FOCReplicatedTraumaEvent Event;
    Event.Sequence = ++ServerSequence;
    Event.HitLocation = HitLocation;
    Event.IncomingDirection = ShotDirection.GetSafeNormal();
    Event.BoneName = BoneName;
    Event.BodyZone = ResolveBodyZone(HitLocation, BoneName);
    Event.WeaponClass = WeaponClass;
    Event.Damage = Damage;
    Event.bExplosive = IsExplosiveDamageClass(DamageTypeClass);
    Event.bFatal = bFatal;
    Event.BloodSeverity = ResolveBloodSeverity(Damage, Event.BodyZone, WeaponClass, Event.bExplosive, bFatal);
    ResolveDismemberment(Event);

    LastTraumaEvent = Event;
    Owner->ForceNetUpdate();
    RenderTraumaEventLocal(LastTraumaEvent);
}

void UOCCombatVisualComponent::RecordRadialTraumaServer(float Damage, const FVector& BlastOrigin,
    TSubclassOf<UDamageType> DamageTypeClass, bool bFatal)
{
    AActor* Owner = GetOwner();
    if (!Owner || !Owner->HasAuthority() || Damage <= 0.0f) return;

    FOCReplicatedTraumaEvent Event;
    Event.Sequence = ++ServerSequence;
    Event.HitLocation = Owner->GetActorLocation();
    Event.IncomingDirection = (Owner->GetActorLocation() - BlastOrigin).GetSafeNormal();
    Event.BodyZone = EOCBodyZone::Torso;
    Event.WeaponClass = EOCWeaponClass::Launcher;
    Event.Damage = Damage;
    Event.bExplosive = true;
    Event.bFatal = bFatal;
    Event.BloodSeverity = ResolveBloodSeverity(Damage, Event.BodyZone, Event.WeaponClass, true, bFatal);
    ResolveDismemberment(Event);

    LastTraumaEvent = Event;
    Owner->ForceNetUpdate();
    RenderTraumaEventLocal(LastTraumaEvent);
}

void UOCCombatVisualComponent::HandleDeathServer()
{
    AActor* Owner = GetOwner();
    if (!Owner || !Owner->HasAuthority()) return;

    if (!LastTraumaEvent.bFatal)
    {
        FOCReplicatedTraumaEvent Event = LastTraumaEvent;
        Event.Sequence = ++ServerSequence;
        Event.HitLocation = Owner->GetActorLocation();
        Event.IncomingDirection = FVector::BackwardVector;
        Event.bFatal = true;
        Event.DismembermentSeverity = EOCDismembermentSeverity::None;
        Event.DismembermentMask = 0;
        LastTraumaEvent = Event;
        Owner->ForceNetUpdate();
    }

    RenderTraumaEventLocal(LastTraumaEvent);
    ApplyDeathPhysicsLocal(LastTraumaEvent);
}

void UOCCombatVisualComponent::OnRep_LastTraumaEvent()
{
    RenderTraumaEventLocal(LastTraumaEvent);
    if (LastTraumaEvent.bFatal)
    {
        ApplyDeathPhysicsLocal(LastTraumaEvent);
    }
}

void UOCCombatVisualComponent::RenderTraumaEventLocal(const FOCReplicatedTraumaEvent& Event)
{
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;
    if (Event.Sequence == 0 || Event.Sequence == LastRenderedSequence) return;
    LastRenderedSequence = Event.Sequence;

    BP_PlayTraumaEvent(Event);

    const int32 GoreLevel = GoreLevelForLocalMachine();
    if (GoreLevel <= 0) return;

    const float Radius = GoreLevel == 1 ? 4.0f + SeverityRank(Event.BloodSeverity) * 2.0f
                                       : 6.0f + SeverityRank(Event.BloodSeverity) * 3.0f;
    if (AOCTransientVisualFX* BloodHit = GetWorld()->SpawnActor<AOCTransientVisualFX>(
        AOCTransientVisualFX::StaticClass(), Event.HitLocation, FRotator::ZeroRotator))
    {
        BloodHit->ConfigureImpact(Event.HitLocation, -Event.IncomingDirection.GetSafeNormal(),
            FLinearColor(0.24f, 0.005f, 0.003f), Radius, 0.16f);
    }
}

void UOCCombatVisualComponent::ApplyDeathPhysicsLocal(const FOCReplicatedTraumaEvent& Event)
{
    if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer) return;
    AOCCharacter* Character = Cast<AOCCharacter>(GetOwner());
    if (!Character) return;

    BP_PlayDeathPresentation(Event);

    USkeletalMeshComponent* Mesh = Character->GetMesh();
    if (Mesh && Mesh->GetSkeletalMeshAsset() && Mesh->GetPhysicsAsset())
    {
        Mesh->SetCollisionProfileName(TEXT("Ragdoll"));
        Mesh->SetAllBodiesSimulatePhysics(true);
        Mesh->SetSimulatePhysics(true);
        Mesh->WakeAllRigidBodies();
        const FVector Impulse = Event.IncomingDirection.GetSafeNormal() * Event.Damage * RagdollImpulseScale;
        Mesh->AddImpulseAtLocation(Impulse, Event.HitLocation, Event.BoneName);
    }

    if (GoreLevelForLocalMachine() >= 2 && Event.DismembermentMask != 0)
    {
        SpawnDismembermentChunksLocal(Event);
    }

    // R11 deliberately drops the long-lived DrawDebug blood circle. Production decals/particles can replace it;
    // the source build should never present editor debug geometry as final combat art.
}

void UOCCombatVisualComponent::SpawnDismembermentChunksLocal(const FOCReplicatedTraumaEvent& Event)
{
    const FVector Base = Event.HitLocation;
    const FVector ImpulseDirection = Event.IncomingDirection.GetSafeNormal();
    const EOCDismembermentPart Parts[] = {
        EOCDismembermentPart::LeftArm, EOCDismembermentPart::RightArm,
        EOCDismembermentPart::LeftLeg, EOCDismembermentPart::RightLeg
    };

    AOCCharacter* Character = Cast<AOCCharacter>(GetOwner());
    USkeletalMeshComponent* Mesh = Character ? Character->GetMesh() : nullptr;

    for (EOCDismembermentPart Part : Parts)
    {
        if ((Event.DismembermentMask & static_cast<uint8>(Part)) == 0) continue;
        SpawnSingleChunkLocal(Part, Base, ImpulseDirection);

        if (Mesh && Mesh->GetSkeletalMeshAsset())
        {
            const FName Bone = Part == EOCDismembermentPart::LeftArm ? FName(TEXT("upperarm_l")) :
                Part == EOCDismembermentPart::RightArm ? FName(TEXT("upperarm_r")) :
                Part == EOCDismembermentPart::LeftLeg ? FName(TEXT("thigh_l")) : FName(TEXT("thigh_r"));
            if (Mesh->GetBoneIndex(Bone) != INDEX_NONE)
            {
                Mesh->HideBoneByName(Bone, PBO_Term);
            }
        }
    }
}

void UOCCombatVisualComponent::SpawnSingleChunkLocal(EOCDismembermentPart Part, const FVector& BaseLocation,
    const FVector& ImpulseDirection)
{
    AActor* Owner = GetOwner();
    if (!Owner || !GetWorld()) return;

    UStaticMesh* LimbMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (!LimbMesh) return;

    UStaticMeshComponent* Chunk = NewObject<UStaticMeshComponent>(Owner);
    if (!Chunk) return;

    Chunk->RegisterComponent();
    Chunk->SetStaticMesh(LimbMesh);
    if (UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
    {
        if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMaterial, Chunk))
        {
            MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.19f, 0.012f, 0.008f));
            Chunk->SetMaterial(0, MID);
        }
    }
    Chunk->SetCollisionProfileName(TEXT("PhysicsActor"));
    Chunk->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    Chunk->SetWorldLocation(BaseLocation + FVector(FMath::FRandRange(-12.0f, 12.0f), FMath::FRandRange(-12.0f, 12.0f), FMath::FRandRange(-8.0f, 16.0f)));
    const bool bLeg = Part == EOCDismembermentPart::LeftLeg || Part == EOCDismembermentPart::RightLeg;
    Chunk->SetWorldScale3D(bLeg ? FVector(0.18f, 0.14f, 0.52f) : FVector(0.14f, 0.14f, 0.42f));
    Chunk->SetSimulatePhysics(true);
    Chunk->SetEnableGravity(true);
    Chunk->AddImpulse((ImpulseDirection + FVector(FMath::FRandRange(-0.35f,0.35f), FMath::FRandRange(-0.35f,0.35f), 0.35f)).GetSafeNormal() * 18000.0f);
    Chunk->SetCullDistance(4500.0f);
    LocalChunks.Add(Chunk);

    FTimerHandle CleanupHandle;
    TWeakObjectPtr<UStaticMeshComponent> WeakChunk = Chunk;
    GetWorld()->GetTimerManager().SetTimer(CleanupHandle, FTimerDelegate::CreateLambda([WeakChunk]()
    {
        if (WeakChunk.IsValid()) WeakChunk->DestroyComponent();
    }), LocalChunkLifetime, false);
}
