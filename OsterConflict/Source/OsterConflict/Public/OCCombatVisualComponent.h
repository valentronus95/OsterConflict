#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OCTraumaTypes.h"
#include "OCCombatVisualComponent.generated.h"

class AOCCharacter;
class UDamageType;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * S14B visual-trauma layer.
 * The server decides zone/severity/dismemberment result. Clients render blood, hit reactions and local debris.
 * It deliberately does NOT replicate every blood decal, ragdoll body or detached chunk.
 */
UCLASS(ClassGroup=(OsterConflict), meta=(BlueprintSpawnableComponent))
class OSTERCONFLICT_API UOCCombatVisualComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOCCombatVisualComponent();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** Called after authoritative point damage has been applied. */
    void RecordPointTraumaServer(float Damage, const FVector& HitLocation, const FVector& ShotDirection,
        FName BoneName, EOCWeaponClass WeaponClass, TSubclassOf<UDamageType> DamageTypeClass, bool bFatal);

    /** Called after authoritative blast damage. */
    void RecordRadialTraumaServer(float Damage, const FVector& BlastOrigin,
        TSubclassOf<UDamageType> DamageTypeClass, bool bFatal);

    /** Called by Character when life state becomes Dead. Safe for bleed-out/give-up with no fresh impact. */
    void HandleDeathServer();

    UFUNCTION(BlueprintPure, Category="Trauma")
    const FOCReplicatedTraumaEvent& GetLastTraumaEvent() const { return LastTraumaEvent; }

    UFUNCTION(BlueprintPure, Category="Trauma")
    EOCBodyZone GetLastBodyZone() const { return LastTraumaEvent.BodyZone; }

    UFUNCTION(BlueprintPure, Category="Trauma")
    EOCBloodSeverity GetLastBloodSeverity() const { return LastTraumaEvent.BloodSeverity; }

    /** Final art hook: AnimBP/Niagara/decal system can implement this without changing gameplay code. */
    UFUNCTION(BlueprintImplementableEvent, Category="Trauma|Presentation")
    void BP_PlayTraumaEvent(const FOCReplicatedTraumaEvent& Event);

    /** Final art hook fired once when the owner becomes a corpse. */
    UFUNCTION(BlueprintImplementableEvent, Category="Trauma|Presentation")
    void BP_PlayDeathPresentation(const FOCReplicatedTraumaEvent& Event);

protected:
    UPROPERTY(ReplicatedUsing=OnRep_LastTraumaEvent, VisibleInstanceOnly, BlueprintReadOnly, Category="Trauma")
    FOCReplicatedTraumaEvent LastTraumaEvent;

    UPROPERTY(EditDefaultsOnly, Category="Trauma|Blood", meta=(ClampMin="0.0"))
    float BloodDebugLifetime = 4.0f;

    UPROPERTY(EditDefaultsOnly, Category="Trauma|Corpse", meta=(ClampMin="1.0"))
    float LocalChunkLifetime = 12.0f;

    UPROPERTY(EditDefaultsOnly, Category="Trauma|Corpse", meta=(ClampMin="0.0"))
    float RagdollImpulseScale = 45.0f;

    // PASS45 Gate K fail-closed default: the legacy detached-limb path still uses an Engine BasicShape Cylinder.
    // Keep it disabled in production defaults until authored limb/chunk content replaces that source-only proxy.
    // Authoritative damage, trauma severity and ragdoll behavior remain active; this only suppresses rejected art.
    UPROPERTY(EditDefaultsOnly, Category="Trauma|Dismemberment")
    bool bAllowDismemberment = false;

    UFUNCTION()
    void OnRep_LastTraumaEvent();

private:
    int32 ServerSequence = 0;
    int32 LastRenderedSequence = 0;
    TArray<TWeakObjectPtr<UStaticMeshComponent>> LocalChunks;

    EOCBodyZone ResolveBodyZone(const FVector& HitLocation, FName BoneName) const;
    EOCBloodSeverity ResolveBloodSeverity(float Damage, EOCBodyZone Zone, EOCWeaponClass WeaponClass,
        bool bExplosive, bool bFatal) const;
    void ResolveDismemberment(FOCReplicatedTraumaEvent& Event) const;
    void RenderTraumaEventLocal(const FOCReplicatedTraumaEvent& Event);
    void ApplyDeathPhysicsLocal(const FOCReplicatedTraumaEvent& Event);
    void SpawnDismembermentChunksLocal(const FOCReplicatedTraumaEvent& Event);
    void SpawnSingleChunkLocal(EOCDismembermentPart Part, const FVector& BaseLocation, const FVector& ImpulseDirection);
    static bool IsExplosiveDamageClass(TSubclassOf<UDamageType> DamageTypeClass);
    static int32 GoreLevelForLocalMachine();
};
