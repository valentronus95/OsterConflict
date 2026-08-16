from pathlib import Path
import re
ROOT=Path(__file__).resolve().parent
P=ROOT/'OsterConflict'
required=[
 'Source/OsterConflict/Private/OCGameUIRootWidget.cpp',
 'Source/OsterConflict/Public/OCGameUIRootWidget.h',
 'Source/OsterConflict/Private/OCGameInstance.cpp',
 'Source/OsterConflict/Private/OCPlayerController.cpp',
 'Config/Localization/Game_Gather.ini',
 'Scripts/S19C/RUN_LOCALIZATION_GATHER.ps1',
 'Docs/S19C_SOURCE_READINESS.md',
 'Docs/UI_TERMINOLOGY_GLOSSARY.csv',
]
for rel in required:
    if not (P/rel).exists(): raise SystemExit(f'MISSING {rel}')
ui=(P/'Source/OsterConflict/Private/OCGameUIRootWidget.cpp').read_text(errors='ignore')
pc=(P/'Source/OsterConflict/Private/OCPlayerController.cpp').read_text(errors='ignore')
gi=(P/'Source/OsterConflict/Private/OCGameInstance.cpp').read_text(errors='ignore')
loc=(P/'Config/Localization/Game_Gather.ini').read_text(errors='ignore')
markers=[
 'LOCTEXT_NAMESPACE','SetNavigationRuleExplicit','UpdateFocusForVisibleContext','Widget->SetFocus()',
 'EKeys::Gamepad_FaceButton_Right','Gamepad_Special_Right','UICloseAdmin','SettingsGraphicsButton',
 'FrontendConnectButton','DeploymentTeamOneButton','NativeCulture=uk-UA','CulturesToGenerate=en',
 'GatherTextFromSource','GenerateTextLocalizationResource','VERSION_MISMATCH'
]
alltext='\n'.join([ui,pc,gi,loc])
missing=[m for m in markers if m not in alltext]
if missing: raise SystemExit('MISSING S19C MARKERS: '+', '.join(missing))
# We expect a substantial gatherable source surface in the critical UMG layer.
loc_count=len(re.findall(r'\b(?:LOCTEXT|NSLOCTEXT)\s*\(',ui+gi))
if loc_count < 80: raise SystemExit(f'Localization coverage too small: {loc_count} gatherable literals')
# Literal FText::FromString is acceptable for raw address/debug data, not static menu labels.
static_literals=re.findall(r'FText::FromString\s*\(\s*TEXT\("([^"]+)"\)\s*\)',ui)
allowed={'127.0.0.1:7777'}
wrong=[x for x in static_literals if x not in allowed]
if wrong: raise SystemExit('NON-GATHERABLE STATIC UI TEXT: '+', '.join(wrong))
# Voice chat is intentionally reserved in backend but must not be instantiated as a P0/P1 settings row.
if re.search(r'VoiceChatVolumeSlider\s*=\s*MakeSliderRow',ui) or re.search(r'VoiceChatAudioCheck\s*=\s*MakeCheckRow',ui):
    raise SystemExit('Voice Chat placeholder exposed in P0/P1 UI')
# Critical network messages should be gatherable and native Ukrainian source in this milestone.
if 'LOCTEXT("VersionMismatch"' not in gi or 'несумісні' not in gi:
    raise SystemExit('Localized version mismatch UX missing')
print(f'S19C source verification: PASS\nGatherable critical literals: {loc_count}; explicit focus/navigation + gamepad Back/Menu contract present.')
