from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parent/'OsterConflict'
checks={
 'Source/OsterConflict/Public/OCLobbyTypes.h':['EOCChatChannel','EOCSquadOrderType','FOCChatMessage','FOCSquadOrder','OCSquadName'],
 'Source/OsterConflict/Public/OCPlayerState.h':['bBotPlayer','SquadId','bSquadLeader','bLobbyReady'],
 'Source/OsterConflict/Private/OCGameMode.cpp':['PreLogin','SERVER_FULL_HUMANS','MaintainPopulation','DesiredBots','SelectBotToRemove','AssignSquadServer','RouteChatMessage','SubmitSquadOrder'],
 'Source/OsterConflict/Private/OCPlayerController.cpp':['SayGlobal','SayTeam','SaySquad','SquadAttack','SquadDefend','SquadMoveHere','SquadRegroup','ClientReceiveChat','ServerSendChat','IA_DeploymentToggle'],
 'Source/OsterConflict/Private/OCAIController.cpp':['GetSquadOrderFor','EOCSquadOrderType::Move','EOCSquadOrderType::AttackObjective'],
 'Source/OsterConflict/Private/OCHUD.cpp':['PRE-GAME / DEPLOYMENT','SERVER        %d HUMAN + %d BOT','SQUAD ORDER','[LEADER]'],
 'Source/OsterConflict/Private/OCEnterableHouse.cpp':['BuildHouseholdProps','AddSofa','fridge','PC tower','laptop','ClutterCount'],
 'Docs/SESSION_14A_README_UA.md':['HumanCount','SayGlobal','household'],
 'Docs/S14A_TEST_MATRIX.md':['SERVER_FULL_HUMANS','Squad chat','Household']
}
missing=[]
for rel, markers in checks.items():
    p=ROOT/rel
    if not p.exists(): missing.append(f'missing file {rel}'); continue
    t=p.read_text(encoding='utf-8')
    for m in markers:
        if m not in t: missing.append(f'{rel}: missing marker {m}')
# generated.h must be final include in UHT headers
for p in (ROOT/'Source/OsterConflict/Public').glob('*.h'):
    t=p.read_text(encoding='utf-8')
    if '.generated.h"' in t:
        inc=[line.strip() for line in t.splitlines() if line.strip().startswith('#include')]
        gen=[x for x in inc if '.generated.h"' in x]
        if gen and inc[-1]!=gen[-1]: missing.append(f'{p.name}: generated.h is not last include')
# RPC declarations may be implemented in split controller bridge .cpp files.
# Search the complete Private source set instead of assuming every AOCPlayerController implementation
# must remain in the monolithic OCPlayerController.cpp.
pc_h=(ROOT/'Source/OsterConflict/Public/OCPlayerController.h').read_text(encoding='utf-8')
private_dir=ROOT/'Source/OsterConflict/Private'
private_cpp_text='\n'.join(
    p.read_text(encoding='utf-8', errors='replace')
    for p in private_dir.glob('*.cpp')
)
for name in re.findall(r'UFUNCTION\(Server, Reliable\)\s+void\s+(\w+)\(',pc_h):
    if f'{name}_Implementation(' not in private_cpp_text: missing.append(f'RPC implementation missing: {name}')
if missing:
    print('S14A structural verification: FAIL')
    print('\n'.join(' - '+x for x in missing)); sys.exit(1)
print('S14A structural verification: PASS')
print(f'Checked {len(checks)} S14A files and hybrid population/squad/chat/interior markers.')
