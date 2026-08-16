from pathlib import Path
import re, sys
root=Path(__file__).resolve().parent/'OsterConflict'
required=[
'Source/OsterConflict/Public/OCAudioUserSettings.h','Source/OsterConflict/Private/OCAudioUserSettings.cpp',
'Source/OsterConflict/Public/OCWorldAudioProfile.h','Source/OsterConflict/Private/OCWorldAudioProfile.cpp',
'Source/OsterConflict/Public/OCWorldAudioComponent.h','Source/OsterConflict/Private/OCWorldAudioComponent.cpp',
'Source/OsterConflict/Public/OCAmbientAudioZone.h','Source/OsterConflict/Private/OCAmbientAudioZone.cpp',
'Source/OsterConflict/Public/OCCharacterAudioProfile.h','Source/OsterConflict/Private/OCCharacterAudioProfile.cpp',
'Source/OsterConflict/Public/OCCharacterAudioComponent.h','Source/OsterConflict/Private/OCCharacterAudioComponent.cpp',
'Source/OsterConflict/Public/OCVehicleAudioProfile.h','Source/OsterConflict/Private/OCVehicleAudioProfile.cpp',
'Source/OsterConflict/Public/OCVehicleAudioComponent.h','Source/OsterConflict/Private/OCVehicleAudioComponent.cpp',
'Source/OsterConflict/Public/OCMenuAudioProfile.h','Source/OsterConflict/Private/OCMenuAudioProfile.cpp',
'Source/OsterConflict/Public/OCMenuAudioSubsystem.h','Source/OsterConflict/Private/OCMenuAudioSubsystem.cpp',
'Docs/SESSION_15B_README_UA.md','Docs/WORLD_AUDIO_ARCHITECTURE_S15B.md','Docs/AUDIO_UX_BENCHMARK_S15B.md',
'Docs/AUDIO_ASSET_MANIFEST_S15B.md','Docs/S15B_TEST_MATRIX.md']
missing=[x for x in required if not (root/x).exists()]
if missing:
 print('Missing:',*missing,sep='\n  ');sys.exit(1)
checks={
'Audio master setting':('Source/OsterConflict/Public/OCAudioUserSettings.h','MasterVolume'),
'Audio checkboxes':('Source/OsterConflict/Public/OCAudioUserSettings.h','bMenuMusicEnabled'),
'Dynamic range':('Source/OsterConflict/Public/OCAudioTypes.h','EOCDynamicRange'),
'Output mode':('Source/OsterConflict/Public/OCAudioTypes.h','EOCAudioOutputMode'),
'World semantic audio':('Source/OsterConflict/Public/OCWorldAudioComponent.h','MulticastWorldAudio'),
'Door audio hook':('Source/OsterConflict/Private/OCInteractableDoor.cpp','EOCWorldAudioEvent::DoorOpen'),
'Gate audio hook':('Source/OsterConflict/Private/OCInteractableGate.cpp','EOCWorldAudioEvent::GateOpen'),
'Light audio hook':('Source/OsterConflict/Private/OCInteractableLight.cpp','EOCWorldAudioEvent::LightOn'),
'Window audio hook':('Source/OsterConflict/Private/OCBreakableWindow.cpp','EOCWorldAudioEvent::WindowBreak'),
'Explosion audio hook':('Source/OsterConflict/Private/OCGrenadeProjectile.cpp','EOCWorldAudioEvent::ExplosionSmall'),
'Destruction audio':('Source/OsterConflict/Private/OCDestructibleProp.cpp','EOCWorldAudioEvent::DestructionMetal'),
'Ambient zone':('Source/OsterConflict/Public/OCAmbientAudioZone.h','AOCAmbientAudioZone'),
'Ambient birds':('Source/OsterConflict/Public/OCWorldAudioProfile.h','Birds'),
'Ambient yard animals':('Source/OsterConflict/Public/OCWorldAudioProfile.h','YardAnimals'),
'Four ambient zones':('Source/OsterConflict/Private/OCGameMode.cpp','AmbientSeeds'),
'Character footsteps':('Source/OsterConflict/Private/OCCharacterAudioComponent.cpp','PlayFootstep'),
'Pain audio':('Source/OsterConflict/Private/OCCharacterAudioComponent.cpp','PainHeavy'),
'Vehicle engine':('Source/OsterConflict/Public/OCVehicleAudioProfile.h','EngineInteriorLoop'),
'Vehicle skid':('Source/OsterConflict/Private/OCVehicleAudioComponent.cpp','IsHandbrakeApplied'),
'Vehicle collisions':('Source/OsterConflict/Private/OCVehicleBase.cpp','PlayCollisionServer'),
'Menu music':('Source/OsterConflict/Private/OCMenuAudioSubsystem.cpp','StartMenuMusic'),
'UI events':('Source/OsterConflict/Public/OCMenuAudioProfile.h','OpenPanel'),
'Weapon bus integration':('Source/OsterConflict/Private/OCWeaponAudioComponent.cpp','EOCAudioBus::Weapons'),
'Benchmark doc':('Docs/AUDIO_UX_BENCHMARK_S15B.md','Arma Reforger'),
'Priority policy':('Docs/WORLD_AUDIO_ARCHITECTURE_S15B.md','Priority order')}
for name,(f,tok) in checks.items():
 text=(root/f).read_text(errors='ignore')
 if tok not in text:
  print('FAIL marker',name,f,tok);sys.exit(1)
# Unreal generated include must be last include in UHT headers
for h in (root/'Source/OsterConflict/Public').glob('*.h'):
 text=h.read_text(errors='ignore')
 if 'generated.h' not in text: continue
 inc=[l.strip() for l in text.splitlines() if l.strip().startswith('#include')]
 gi=[i for i,l in enumerate(inc) if '.generated.h"' in l]
 if not gi or gi[-1]!=len(inc)-1:
  print('FAIL generated.h order',h.name);sys.exit(1)
# Basic delimiter sanity after stripping strings/comments
def strip_cpp(text):
 text=re.sub(r'/\*.*?\*/','',text,flags=re.S); text=re.sub(r'//.*','',text)
 text=re.sub(r'"(?:\\.|[^"\\])*"','""',text); text=re.sub(r"'(?:\\.|[^'\\])*'","''",text);return text
files=list((root/'Source').rglob('*.cpp'))+list((root/'Source').rglob('*.h'))
for f in files:
 t=strip_cpp(f.read_text(errors='ignore'))
 for a,b in [('(',')'),('{','}'),('[',']')]:
  dep=0
  for ch in t:
   if ch==a: dep+=1
   elif ch==b:
    dep-=1
    if dep<0: print('FAIL delimiter',f,a+b);sys.exit(1)
  if dep!=0: print('FAIL delimiter balance',f,a+b,dep);sys.exit(1)
# Guard against the accidental undeclared identifier caught during S15B build-up
ambient=(root/'Source/OsterConflict/Private/OCAmbientAudioZone.cpp').read_text()
if 'bReplicates=true' not in ambient or 'SetReplicateMovement(false)' not in ambient:
 print('FAIL ambient zones spawned by server must replicate their configuration to clients');sys.exit(1)
settings=(root/'Source/OsterConflict/Private/OCAudioUserSettings.cpp').read_text()
if 'GetBusPercent' in settings and 'bMasterEnabled=bEnabled' in settings.split('int32 UOCAudioUserSettings::GetBusPercent',1)[1].split('float* UOCAudioUserSettings::GetMutableBusValue',1)[0]:
 print('FAIL settings mutation inside GetBusPercent');sys.exit(1)
print('S15B structural verification: PASS')
print(f'Checked {len(required)} required files, {len(checks)} markers and {len(files)} C++ headers/sources.')
