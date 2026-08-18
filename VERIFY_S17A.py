from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parent
P=ROOT/'OsterConflict'
required=[
 'Source/OsterConflict/Public/OCGameUIRootWidget.h',
 'Source/OsterConflict/Private/OCGameUIRootWidget.cpp',
 'Source/OsterConflict/Public/OCPlayerController.h',
 'Source/OsterConflict/Private/OCPlayerController.cpp',
 'Source/OsterConflict/Private/OCR13DeploymentSelectionBridge.cpp',
 'Source/OsterConflict/Public/OCGameMode.h',
 'Source/OsterConflict/Private/OCGameMode.cpp',
 'Docs/SESSION_17A_README_UA.md','Docs/UI_ARCHITECTURE_S17A.md','Docs/S17A_TEST_MATRIX.md',
 'Scripts/RUN_S17A_FRONTEND_CLIENT.bat','Scripts/RUN_S17A_SERVER_7777.bat'
]
for rel in required:
    if not (P/rel).exists(): raise SystemExit(f'MISSING {rel}')

alltext='\n'.join((P/r).read_text(errors='ignore') for r in required if (P/r).suffix in {'.h','.cpp','.md','.cs','.bat'})
markers=[
 'UOCGameUIRootWidget','UMG','SlateCore','-Frontend','UIConnect','UIRequestTeam','UISelectSpawn','UIReadyDeploy',
 'ServerRequestTeam','ServerSetDeploymentSpawn','RequestTeamChange','GetRequestedDeploymentSpawn',
 'FindSafest','SpawnA','Scoreboard','ChannelFmt','SandboxAdmin','IA_ChatToggle','EKeys::T','EKeys::Escape',
 'FInputModeGameAndUI','FInputModeGameOnly','SERVER_FULL_HUMANS','ALL/TEAM/SQUAD'
]
missing=[m for m in markers if m not in alltext and m not in (P/'Source/OsterConflict/OsterConflict.Build.cs').read_text(errors='ignore')]
if missing: raise SystemExit('MISSING MARKERS: '+', '.join(missing))

# Server RPC declarations may be split across focused controller implementation translation units.
h=(P/'Source/OsterConflict/Public/OCPlayerController.h').read_text()
cpp='\n'.join([
    (P/'Source/OsterConflict/Private/OCPlayerController.cpp').read_text(errors='ignore'),
    (P/'Source/OsterConflict/Private/OCR13DeploymentSelectionBridge.cpp').read_text(errors='ignore'),
])
rpcs=re.findall(r'UFUNCTION\(Server, Reliable\)\s+void\s+(\w+)\s*\(',h)
for rpc in rpcs:
    if f'{rpc}_Implementation' not in cpp: raise SystemExit(f'RPC implementation missing: {rpc}')

# generated.h last include in UHT headers touched by S17A
for rel in ['Source/OsterConflict/Public/OCGameUIRootWidget.h','Source/OsterConflict/Public/OCPlayerController.h']:
    text=(P/rel).read_text().splitlines()
    inc=[x.strip() for x in text if x.strip().startswith('#include')]
    if not inc or 'generated.h' not in inc[-1]: raise SystemExit(f'generated.h order: {rel}')

# Basic delimiter sanity for touched C++
for rel in ['Source/OsterConflict/Private/OCGameUIRootWidget.cpp','Source/OsterConflict/Private/OCPlayerController.cpp','Source/OsterConflict/Private/OCR13DeploymentSelectionBridge.cpp','Source/OsterConflict/Private/OCGameMode.cpp']:
    text=(P/rel).read_text()
    # ignore strings/comments imperfectly but enough to catch accidental editing damage
    for a,b in [('(',')'),('{','}'),('[',']')]:
        if text.count(a)!=text.count(b): raise SystemExit(f'Delimiter mismatch {a}{b}: {rel} {text.count(a)} != {text.count(b)}')

print(f'S17A structural verification: PASS\nChecked {len(required)} required files, {len(markers)} UI/network markers and {len(rpcs)} PlayerController server RPCs.')
