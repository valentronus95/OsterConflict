#include "OCMenuAudioProfile.h"
const TArray<TObjectPtr<USoundBase>>& UOCMenuAudioProfile::GetSet(EOCMenuAudioEvent Event) const
{
    switch(Event){case EOCMenuAudioEvent::Hover:return Hover;case EOCMenuAudioEvent::Back:return Back;case EOCMenuAudioEvent::Confirm:return Confirm;case EOCMenuAudioEvent::Error:return Error;case EOCMenuAudioEvent::OpenPanel:return OpenPanel;case EOCMenuAudioEvent::ClosePanel:return ClosePanel;case EOCMenuAudioEvent::Click:default:return Click;}
}
