from pathlib import Path
import re
ROOT=Path(__file__).resolve().parent
P=ROOT/'OsterConflict'
required=[
 'Source/OsterConflict/Public/OCPlayerUserSettings.h',
 'Source/OsterConflict/Private/OCPlayerUserSettings.cpp',
 'Source/OsterConflict/Public/OCGameUIRootWidget.h',
 'Source/OsterConflict/Private/OCGameUIRootWidget.cpp',
 'Source/OsterConflict/Public/OCPlayerController.h',
 'Source/OsterConflict/Private/OCPlayerController.cpp',
 'Source/OsterConflict/Private/OCR13DeploymentSelectionBridge.cpp',
 'Source/OsterConflict/Public/OCCharacter.h',
 'Source/OsterConflict/Private/OCCharacter.cpp',
 'Source/OsterConflict/Private/OCHUD.cpp',
 'Source/OsterConflict/Public/OCAudioUserSettings.h',
 'Source/OsterConflict/Private/OCAudioUserSettings.cpp',
 'Docs/SESSION_17B_README_UA.md','Docs/SETTINGS_ARCHITECTURE_S17B.md','Docs/S17B_TEST_MATRIX.md',
 'Scripts/RUN_S17B_FRONTEND_CLIENT.bat','Config/DefaultGame.ini'
]
for rel in required:
    if not (P/rel).exists(): raise SystemExit(f'MISSING {rel}')
alltext='\n'.join((P/r).read_text(errors='ignore') for r in required if (P/r).suffix in {'.h','.cpp','.md','.ini','.bat'})
markers=[
 'UOCPlayerUserSettings','Config=GameUserSettings','UGameUserSettings','SettingsTitle','GraphicsTitle','AudioMixerTitle','ControlsTitle','InterfaceTitle','AccessibilityTitle',
 'SetScreenResolution','SetFullscreenMode','SetVSyncEnabled','SetDynamicResolutionEnabled','SetFrameRateLimit','SetResolutionScaleValueEx','SetOverallScalabilityLevel',
 'SetViewDistanceQuality','SetShadowQuality','SetTextureQuality','SetVisualEffectQuality','SetFoliageQuality','SetPostProcessingQuality','SetAntiAliasingQuality','SetShadingQuality','SetGlobalIlluminationQuality','SetReflectionQuality','SetLandscapeQuality',
 'MouseSensitivity','AimSensitivityMultiplier','bInvertMouseY','FieldOfView','HUDScale','bShowFPS','bShowPing','bShowCrosshair','bShowHitMarker',
 'GoreLevel','bSubtitles','bReduceFlashes','CameraShakeScale','ColorVisionMode',
 'UnmapAllKeysFromAction','MapKey','PendingRebindAction','ResetDefaults','SaveBack','Cancel','ReloadConfig','SetToDefaults',
 'AudioMasterEnabled','AudioWeaponsEnabled','AudioAmbienceEnabled','MenuMusicEnabled','DynamicRange','OutputSpatial',
 'oc.GoreLevel','ProjectVersion=0.0.'
]
missing=[m for m in markers if m not in alltext]
if missing: raise SystemExit('MISSING MARKERS: '+', '.join(missing))

# staged reset must not save immediately
for rel,name in [('Source/OsterConflict/Private/OCPlayerUserSettings.cpp','ResetPlayerDefaults'),('Source/OsterConflict/Private/OCAudioUserSettings.cpp','ResetAudioDefaults')]:
    txt=(P/rel).read_text(errors='ignore')
    m=re.search(rf'void\s+\w+::{name}\s*\(\)\s*\{{(.*?)\n\}}',txt,re.S)
    if not m: raise SystemExit(f'Missing function {name}')
    if 'SaveConfig' in m.group(1): raise SystemExit(f'{name} must be staged, not auto-save')

# generated.h last include in touched UHT headers
for rel in ['Source/OsterConflict/Public/OCPlayerUserSettings.h','Source/OsterConflict/Public/OCGameUIRootWidget.h','Source/OsterConflict/Public/OCPlayerController.h','Source/OsterConflict/Public/OCCharacter.h']:
    lines=(P/rel).read_text(errors='ignore').splitlines()
    inc=[x.strip() for x in lines if x.strip().startswith('#include')]
    if not inc or 'generated.h' not in inc[-1]: raise SystemExit(f'generated.h order: {rel}')

# server RPC declarations may be split across focused controller implementation translation units
h=(P/'Source/OsterConflict/Public/OCPlayerController.h').read_text(errors='ignore')
cpp='\n'.join([
    (P/'Source/OsterConflict/Private/OCPlayerController.cpp').read_text(errors='ignore'),
    (P/'Source/OsterConflict/Private/OCR13DeploymentSelectionBridge.cpp').read_text(errors='ignore'),
])
rpcs=re.findall(r'UFUNCTION\(Server, Reliable\)\s+void\s+(\w+)\s*\(',h)
for rpc in rpcs:
    if f'{rpc}_Implementation' not in cpp: raise SystemExit(f'RPC implementation missing: {rpc}')

# delimiter sanity for all project C++
for path in list((P/'Source/OsterConflict').rglob('*.h'))+list((P/'Source/OsterConflict').rglob('*.cpp')):
    text=path.read_text(errors='ignore')
    # counts are deliberately simple and complement per-milestone verifiers
    for a,b in [('(',')'),('{','}'),('[',']')]:
        if text.count(a)!=text.count(b): raise SystemExit(f'Delimiter mismatch {a}{b}: {path.relative_to(P)}')

print(f'S17B structural verification: PASS\nChecked {len(required)} required files, {len(markers)} settings/persistence markers and {len(rpcs)} PlayerController server RPCs.')
