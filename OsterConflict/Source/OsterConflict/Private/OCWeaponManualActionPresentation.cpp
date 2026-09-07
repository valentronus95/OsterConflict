#include "OCWeaponBase.h"

#include "OCCharacter.h"
#include "OCWeaponAudioComponent.h"

void AOCWeaponBase::OnRep_ActionCycling()
{
    if (!bActionCycling || !WeaponAudioComponent)
    {
        return;
    }

    // The locally controlled weapon emits its mechanical cue from the first-person presentation
    // transition so standalone/listen-server play is covered too. This replication callback is
    // only the remote-listener path and therefore must not double-play for the local owner.
    const AOCCharacter* OwnerCharacter = Cast<AOCCharacter>(GetOwner());
    if (!OwnerCharacter || OwnerCharacter->IsLocallyControlled())
    {
        return;
    }

    const int32 EventSeed = AmmoInMagazine * 31 + static_cast<int32>(Tuning.ActionType) * 101;
    WeaponAudioComponent->HandleStateEventLocal(
        EOCWeaponAudioEvent::ManualActionCycle, GetActorLocation(), EventSeed);
}
