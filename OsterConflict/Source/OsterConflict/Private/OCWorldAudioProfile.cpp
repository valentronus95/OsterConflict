#include "OCWorldAudioProfile.h"

const TArray<TObjectPtr<USoundBase>>& UOCWorldAudioProfile::GetEventSet(EOCWorldAudioEvent Event) const
{
    switch (Event)
    {
        case EOCWorldAudioEvent::DoorOpen: return DoorOpen;
        case EOCWorldAudioEvent::DoorClose: return DoorClose;
        case EOCWorldAudioEvent::GateOpen: return GateOpen;
        case EOCWorldAudioEvent::GateClose: return GateClose;
        case EOCWorldAudioEvent::LightOn: return LightOn;
        case EOCWorldAudioEvent::LightOff: return LightOff;
        case EOCWorldAudioEvent::WindowBreak: return WindowBreak;
        case EOCWorldAudioEvent::DestructionWood: return DestructionWood;
        case EOCWorldAudioEvent::DestructionMetal: return DestructionMetal;
        case EOCWorldAudioEvent::DestructionMasonry: return DestructionMasonry;
        case EOCWorldAudioEvent::ExplosionSmall: return ExplosionSmall;
        case EOCWorldAudioEvent::ExplosionLarge: return ExplosionLarge;
        case EOCWorldAudioEvent::Debris: return Debris;
        case EOCWorldAudioEvent::InteractionGeneric:
        default: return InteractionGeneric;
    }
}
