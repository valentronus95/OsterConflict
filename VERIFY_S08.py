from pathlib import Path
import re, sys

root = Path(__file__).resolve().parent / 'OsterConflict'
required = [
    'Source/OsterConflict/Public/OCInteractableActor.h',
    'Source/OsterConflict/Private/OCInteractableActor.cpp',
    'Source/OsterConflict/Public/OCInteractableDoor.h',
    'Source/OsterConflict/Private/OCInteractableDoor.cpp',
    'Source/OsterConflict/Public/OCBreakableWindow.h',
    'Source/OsterConflict/Private/OCBreakableWindow.cpp',
    'Source/OsterConflict/Public/OCEnterableHouse.h',
    'Source/OsterConflict/Private/OCEnterableHouse.cpp',
    'Source/OsterConflict/Public/OCWorldSectorOster.h',
    'Source/OsterConflict/Private/OCWorldSectorOster.cpp',
    'Source/OsterConflict/Public/OCCharacter.h',
    'Source/OsterConflict/Private/OCCharacter.cpp',
    'Source/OsterConflict/Private/OCGameMode.cpp',
    'Docs/SESSION_08_README_UA.md',
    'Docs/INTERACTION_ARCHITECTURE_S08.md',
    'Docs/S08_TEST_MATRIX.md',
    'Docs/ROADMAP_SESSIONS.md',
]
missing = [f for f in required if not (root / f).exists()]
if missing:
    print('Missing:', *missing, sep='\n - ')
    sys.exit(1)

markers = {
    'OCInteractableActor.h': [
        'GetInteractionPrompt', 'CanInteractServer', 'InteractServer', 'MaxInteractionDistance'
    ],
    'OCInteractableDoor.h': [
        'class OSTERCONFLICT_API AOCInteractableDoor', 'UPROPERTY(Replicated', 'bool bOpen = false'
    ],
    'OCInteractableDoor.cpp': [
        'DOREPLIFETIME(AOCInteractableDoor, bOpen)', 'E  OPEN DOOR', 'E  CLOSE DOOR',
        'bOpen = !bOpen', 'FMath::FInterpTo'
    ],
    'OCBreakableWindow.h': [
        'ReplicatedUsing=OnRep_Broken', 'bool bBroken = false', 'NetMulticast, Unreliable',
        'FVector_NetQuantizeNormal'
    ],
    'OCBreakableWindow.cpp': [
        'DOREPLIFETIME(AOCBreakableWindow, bBroken)', 'BreakServer', 'SetCollisionEnabled(ECollisionEnabled::NoCollision)',
        'MulticastBreakFX_Implementation', 'AddImpulse', 'DebrisLifetime'
    ],
    'OCEnterableHouse.cpp': [
        'SpawnActor<AOCInteractableDoor>', 'SpawnActor<AOCBreakableWindow>', 'BuildShell();', 'BuildInterior();',
        'BuildYard();', 'S08 ENTERABLE HOUSE'
    ],
    'OCWorldSectorOster.cpp': [
        'BuildSolomiiKrushelnytskoiStreet();', 'KrushelnytskaEnterableHouseAnchor',
        'SOLOMII KRUSHELNYTSKOI STREET / S08',
        'FVector(-43000.0f, 36000.0f, RoadZ)', 'FVector(-24200.0f, 37000.0f, RoadZ)'
    ],
    'OCCharacter.cpp': [
        'FindFocusedWorldInteractable', 'OCWorldInteractionTrace', 'Interactable->CanInteractServer(this)',
        'Interactable->InteractServer(this)', 'Interactable->GetInteractionPrompt(this)'
    ],
    'OCGameMode.cpp': [
        'SpawnActor<AOCEnterableHouse>', 'KrushelnytskaEnterableHouseAnchor()', 'KrushelnytskaEnterableHouseYaw()'
    ],
    'ROADMAP_SESSIONS.md': [
        'S08 — Остер Greybox, Sector B [ВИКОНАНО В ЦЬОМУ АРХІВІ]'
    ],
    'SESSION_08_README_UA.md': [
        'Родовий відмінок: **Остра**', 'AOCInteractableActor', 'AOCInteractableDoor', 'AOCBreakableWindow'
    ],
}

for name, needles in markers.items():
    paths = list(root.rglob(name))
    text = paths[0].read_text(errors='ignore') if paths else ''
    for needle in needles:
        if needle not in text:
            print(f'Missing marker {needle!r} in {name}')
            sys.exit(1)

# C++ delimiter sanity after removing comments and strings.
for p in list((root/'Source').rglob('*.h')) + list((root/'Source').rglob('*.cpp')):
    t = p.read_text(errors='ignore')
    x = re.sub(r'//.*', '', t)
    x = re.sub(r'/\*.*?\*/', '', x, flags=re.S)
    x = re.sub(r'"(?:\\.|[^"\\])*"', '""', x)
    for a, b in [('(', ')'), ('[', ']'), ('{', '}')]:
        depth = 0
        for ch in x:
            if ch == a:
                depth += 1
            elif ch == b:
                depth -= 1
                if depth < 0:
                    print('Delimiter underflow', p, a, b)
                    sys.exit(1)
        if depth:
            print('Delimiter mismatch', p, a, b, depth)
            sys.exit(1)

# generated.h must be the last include in reflected headers.
for p in (root/'Source/OsterConflict/Public').rglob('*.h'):
    lines = p.read_text(errors='ignore').splitlines()
    generated = [i for i, line in enumerate(lines) if '.generated.h"' in line]
    if generated:
        include_lines = [i for i, line in enumerate(lines) if line.strip().startswith('#include')]
        if generated[-1] != max(include_lines):
            print('generated.h is not last include:', p)
            sys.exit(1)

# Network regression: every Character/Controller Server RPC still has an implementation.
for hname in ['OCCharacter.h', 'OCPlayerController.h']:
    hp = root/'Source/OsterConflict/Public'/hname
    cp = root/'Source/OsterConflict/Private'/hname.replace('.h', '.cpp')
    ht, ct = hp.read_text(), cp.read_text()
    names = re.findall(r'UFUNCTION\(Server,[^\)]*\)\s*\n\s*void\s+(\w+)\s*\(', ht)
    for name in names:
        if f'{name}_Implementation' not in ct:
            print('Missing RPC implementation', name)
            sys.exit(1)

# New replication contracts must be paired with DOREPLIFETIME and multicast implementation.
door_h = (root/'Source/OsterConflict/Public/OCInteractableDoor.h').read_text()
door_cpp = (root/'Source/OsterConflict/Private/OCInteractableDoor.cpp').read_text()
window_h = (root/'Source/OsterConflict/Public/OCBreakableWindow.h').read_text()
window_cpp = (root/'Source/OsterConflict/Private/OCBreakableWindow.cpp').read_text()
if 'Replicated' not in door_h or 'DOREPLIFETIME(AOCInteractableDoor, bOpen)' not in door_cpp:
    print('Door replication contract incomplete')
    sys.exit(1)
if 'ReplicatedUsing=OnRep_Broken' not in window_h or 'DOREPLIFETIME(AOCBreakableWindow, bBroken)' not in window_cpp:
    print('Window replication contract incomplete')
    sys.exit(1)
if 'void AOCBreakableWindow::MulticastBreakFX_Implementation' not in window_cpp:
    print('Window multicast implementation missing')
    sys.exit(1)

# Interaction priority: revive must appear before world interaction, and world interaction before pickups.
char_cpp = (root/'Source/OsterConflict/Private/OCCharacter.cpp').read_text()
server_block = char_cpp[char_cpp.index('void AOCCharacter::ServerInteract_Implementation()'):char_cpp.index('void AOCCharacter::ServerCancelInteract_Implementation()')]
order = [server_block.find('FindClosestDownedCharacter'), server_block.find('FindFocusedWorldInteractable'), server_block.find('FindClosestWorldWeapon')]
if min(order) < 0 or order != sorted(order):
    print('S08 interaction priority regression:', order)
    sys.exit(1)

# The active game mode must still use the city sector, not the retired tiny arena.
gm = (root/'Source/OsterConflict/Private/OCGameMode.cpp').read_text(errors='ignore')
if 'SpawnActor<AOCTestArena>' in gm or 'SpawnPrototypeArena();' in gm:
    print('Old OCTestArena became active again')
    sys.exit(1)

print('S08 structural verification: PASS')
print(f'Checked {len(required)} required files and {sum(map(len, markers.values()))} S08 markers.')
