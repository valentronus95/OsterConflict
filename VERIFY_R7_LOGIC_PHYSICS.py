from pathlib import Path
ROOT=Path(__file__).resolve().parent
P=ROOT/'OsterConflict'
checks=[]
def req(cond,msg):
    if not cond: raise SystemExit('R7 LOGIC/PHYSICS VERIFY FAIL: '+msg)
    checks.append(msg)
def t(rel): return (P/rel).read_text(encoding='utf-8',errors='ignore')

def rt(rel): return (ROOT/rel).read_text(encoding='utf-8',errors='ignore')

charh=t('Source/OsterConflict/Public/OCCharacter.h')
charcpp=t('Source/OsterConflict/Private/OCCharacter.cpp')
weapon=t('Source/OsterConflict/Private/OCWeaponBase.cpp')
vehh=t('Source/OsterConflict/Public/OCVehicleBase.h')
veh=t('Source/OsterConflict/Private/OCVehicleBase.cpp')
btr=t('Source/OsterConflict/Private/OCBTR.cpp')
ai=t('Source/OsterConflict/Private/OCAIController.cpp')
windowh=t('Source/OsterConflict/Public/OCBreakableWindow.h')
window=t('Source/OsterConflict/Private/OCBreakableWindow.cpp')
desh=t('Source/OsterConflict/Public/OCDestructibleProp.h')
des=t('Source/OsterConflict/Private/OCDestructibleProp.cpp')
combat=t('Source/OsterConflict/Private/OCCombatVisualComponent.cpp')
gateh=t('Source/OsterConflict/Public/OCInteractableGate.h')
gate=t('Source/OsterConflict/Private/OCInteractableGate.cpp')
grenh=t('Source/OsterConflict/Public/OCGrenadeProjectile.h')
gren=t('Source/OsterConflict/Private/OCGrenadeProjectile.cpp')
psh=t('Source/OsterConflict/Public/OCPlayerState.h')
psc=t('Source/OsterConflict/Private/OCPlayerState.cpp')
gm=t('Source/OsterConflict/Private/OCGameMode.cpp')
vsp_h=t('Source/OsterConflict/Public/OCVehicleSpawnPoint.h')
vsp=t('Source/OsterConflict/Private/OCVehicleSpawnPoint.cpp')
armed=t('Source/OsterConflict/Private/OCArmedVehicleBase.cpp')

req('float CrouchSpeed = 250.0f;' in charh,'explicit crouch speed exists')
req('MaxWalkSpeedCrouched = IsDowned() ? DownedCrawlSpeed : CrouchSpeed' in charcpp,'downed/crouch speed uses crouched movement cap')
req('ResolveCharacterDamageMultiplier' in weapon and 'return 2.0f' in weapon,'source-proxy head/neck gameplay multiplier exists')
req('HitZoneMultiplier' in weapon,'hit-zone multiplier applied to gameplay damage')

req('ReplicatedUsing=OnRep_Open' in gateh,'gate open state has OnRep')
req('bStartWithTickEnabled = false' in gate,'gate is not ticking while idle')
req('SetActorTickEnabled(false)' in gate,'gate disables tick after interpolation')

req('void ResetServer();' in windowh,'window exposes round reset')
req('ECC_WorldStatic, ECR_Block' in window and 'ECC_Pawn, ECR_Ignore' in window,'window cosmetic shards collide with world but not pawns')
req('GetNetMode() == NM_DedicatedServer' in window,'window cosmetic shard physics skipped on dedicated server')
req('ApplyIntactPresentation' in window,'window can restore intact presentation')

req('void ResetServer();' in desh,'destructible exposes round reset')
req('TransientChunks' in desh and 'TransientChunks.Reset()' in des,'destructible cosmetic chunks are tracked/cleared')
req('GetNetMode() == NM_DedicatedServer' in des,'destructible cosmetic chunks skipped on dedicated server')
req('SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore)' in des,'destructible debris ignores pawn collision')
req('SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore)' in combat,'dismemberment chunks ignore pawn collision')

req('GetActorUpVector() * SteeringInput' in veh,'vehicle steering torque uses vehicle-local up axis')
req('Clamp(FMath::Abs(ForwardSpeed) / 800.0f, 0.0f, 1.0f)' in veh,'stationary vehicle no longer receives forced minimum pivot torque')
req('DamagePowerScale' in veh and 'DamageGripScale' in veh,'vehicle damage stages affect driving physics')
req('CanDealDamage(EventInstigator, this)' in veh,'vehicle damage honors friendly-fire authority policy')
req('FindSafeExitLocationForCharacter' in vehh and 'FindTeleportSpot' in veh,'driver/gunner exit resolves a non-blocking teleport spot')
req('FindSafeExitLocationForCharacter(Character, 1.0f, bForced)' in armed,'gunner uses safe exit resolver')

req('ClearSuspensionPointsLocal' in vehh,'derived vehicle can replace suspension contacts')
req('ClearSuspensionPointsLocal();' in btr,'BTR replaces 4-point base suspension')
req(btr.count('AddSuspensionPointLocal(') >= 2 and 'PrimaryAxleWheelPositions' in btr and 'MidAxleWheelPositions' in btr,'BTR uses four axles/eight suspension contacts')
req('WheelVisuals.Add(ExtraWheels[I])' in btr,'extra BTR wheels participate in wheel animation')

req('#include "OCSmokeCloud.h"' in ai,'AI knows smoke gameplay actor')
req('GetSmokeRadiusCm()' in ai and 'Closest = Start + Segment * Alpha' in ai,'AI line-of-sight is blocked by smoke volume')

req('FragPhysicsImpulse' in grenh and 'MaxImpulseBodyMassKg' in grenh,'bounded frag physics impulse tuning exists')

req('ApplyBoundedPhysicsImpulseServer' in gren and 'AddImpulseAtLocation' in gren,'frag explosion applies bounded impulse to simulated light bodies')
req('Component->GetMass() > MaxImpulseBodyMassKg' in gren,'explosion impulse excludes heavy bodies by mass budget')

req('ResetRoundStatsServer' in psh and 'SetScore(0.0f)' in psc,'round reset clears K/D/R/score')
for marker in ['AOCInteractableDoor','AOCInteractableGate','AOCInteractableLight','AOCBreakableWindow','AOCDestructibleProp','AOCSmokeCloud','AOCDeployableTrap']:
    req(marker in gm,f'round reset references {marker}')
req('ForceExitGunnerServer' in gm and 'ForceExitDriverServer' in gm,'round reset clears vehicle occupants before pawn restart')
req('ResetForRoundServer' in vsp_h and 'GetWorldTimerManager().ClearTimer(RespawnTimerHandle)' in vsp,'vehicle spawn point has deterministic round reset')
req('RoundVehicles' in gm and 'SpawnPoint->ResetForRoundServer()' in gm,'round restart removes old vehicles/wrecks and regenerates fleet')

req('LocalZ <= -28.0f' in weapon, 'lower-limb proxy zone does not swallow lower torso')
req('HealthComponent->IsAlive()) && !bIsSprinting' in charcpp, 'server reload requires alive state')
req('CurrentWeapon && (!HealthComponent || HealthComponent->IsAlive())' in charcpp, 'server fire-mode mutation requires alive state')
req('Candidate && HasClearLineOfSightTo(Bot, Candidate)' in ai, 'perceived AI targets are filtered by gameplay LOS/smoke')
req('OCFragImpulseLOS' in gren and 'OcclusionHit.GetActor() != Actor' in gren, 'frag physics impulse respects blocking LOS')
start=rt('START_HERE.cmd')
req(any(token in start for token in ['R13 CONTENT + GAMEPLAY PASS','R11 VISUAL FOUNDATION','R8 TARGET RULES FIX']),'current kit retains R7 logic/physics hardened baseline')
print(f'R7 LOGIC/PHYSICS BASELINE: PASS ({len(checks)} checks)')
