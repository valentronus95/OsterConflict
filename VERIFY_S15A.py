from pathlib import Path
import re, sys
root=Path(__file__).resolve().parent/'OsterConflict'
required=[
'Source/OsterConflict/Public/OCAudioTypes.h',
'Source/OsterConflict/Public/OCWeaponAudioProfile.h',
'Source/OsterConflict/Private/OCWeaponAudioProfile.cpp',
'Source/OsterConflict/Public/OCWeaponAudioComponent.h',
'Source/OsterConflict/Private/OCWeaponAudioComponent.cpp',
'Docs/SESSION_15A_README_UA.md','Docs/WEAPON_AUDIO_ARCHITECTURE_S15A.md',
'Docs/AUDIO_ASSET_MANIFEST_S15A.md','Docs/S15A_TEST_MATRIX.md','AudioSources/README_UA.md']
missing=[x for x in required if not (root/x).exists()]
if missing:
 print('Missing:',*missing,sep='\n  '); sys.exit(1)
checks={
'Audio profile':('Source/OsterConflict/Public/OCWeaponAudioProfile.h','UOCWeaponAudioProfile'),
'Outdoor shot':('Source/OsterConflict/Public/OCWeaponAudioProfile.h','ShotNearOutdoor'),
'Indoor shot':('Source/OsterConflict/Public/OCWeaponAudioProfile.h','ShotNearIndoor'),
'Suppressed shot':('Source/OsterConflict/Public/OCWeaponAudioProfile.h','ShotSuppressedOutdoor'),
'Distant tails':('Source/OsterConflict/Public/OCWeaponAudioProfile.h','DistantTails'),
'Bullet cracks':('Source/OsterConflict/Public/OCWeaponAudioProfile.h','BulletCracks'),
'Audio component':('Source/OsterConflict/Public/OCWeaponAudioComponent.h','UOCWeaponAudioComponent'),
'Acoustic detector':('Source/OsterConflict/Private/OCWeaponAudioComponent.cpp','DetectEnvironmentAt'),
'Bullet closest segment':('Source/OsterConflict/Private/OCWeaponAudioComponent.cpp','ClosestPointOnSegment'),
'Dedicated guard':('Source/OsterConflict/Private/OCWeaponAudioComponent.cpp','NM_DedicatedServer'),
'Audio debug':('Source/OsterConflict/Private/OCWeaponAudioComponent.cpp','oc.Audio.Debug'),
'Shot multicast':('Source/OsterConflict/Public/OCWeaponBase.h','MulticastShotAudio'),
'State multicast':('Source/OsterConflict/Public/OCWeaponBase.h','MulticastWeaponStateAudio'),
'Dry fire':('Source/OsterConflict/Private/OCWeaponBase.cpp','LastServerDryFireTime'),
'Reload start':('Source/OsterConflict/Private/OCWeaponBase.cpp','EOCWeaponAudioEvent::ReloadStart'),
'Reload end':('Source/OsterConflict/Private/OCWeaponBase.cpp','EOCWeaponAudioEvent::ReloadEnd'),
'Fire mode':('Source/OsterConflict/Private/OCWeaponBase.cpp','EOCWeaponAudioEvent::FireModeSwitch'),
'Impact audio':('Source/OsterConflict/Private/OCWeaponBase.cpp','HandleImpactLocal'),
'Profile on definition':('Source/OsterConflict/Public/OCWeaponDefinition.h','AudioProfile'),
'License rule':('Docs/AUDIO_ASSET_MANIFEST_S15A.md','Не копіювати raw audio'),
}
for name,(f,tok) in checks.items():
 if tok not in (root/f).read_text(errors='ignore'):
  print('FAIL marker',name,f,tok); sys.exit(1)
# generated include order
for h in (root/'Source/OsterConflict/Public').glob('*.h'):
 text=h.read_text(errors='ignore')
 if 'generated.h' not in text: continue
 inc=[l.strip() for l in text.splitlines() if l.strip().startswith('#include')]
 gi=[i for i,l in enumerate(inc) if '.generated.h"' in l]
 if not gi or gi[-1]!=len(inc)-1:
  print('FAIL generated.h order',h.name);sys.exit(1)
# delimiters
def strip_cpp(text):
 text=re.sub(r'/\*.*?\*/','',text,flags=re.S); text=re.sub(r'//.*','',text)
 text=re.sub(r'"(?:\\.|[^"\\])*"','""',text); text=re.sub(r"'(?:\\.|[^'\\])*'","''",text); return text
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
print('S15A structural verification: PASS')
print(f'Checked {len(required)} required files, {len(checks)} markers and {len(files)} C++ headers/sources.')
