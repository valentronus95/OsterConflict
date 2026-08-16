from pathlib import Path
import re, sys
root=Path(__file__).resolve().parent
proj=root/'OsterConflict'
src=proj/'Source/OsterConflict'
required=[
'Public/OCGameplayMode.h','Public/OCOrdnanceTypes.h','Public/OCGrenadeProjectile.h','Private/OCGrenadeProjectile.cpp',
'Public/OCSmokeCloud.h','Private/OCSmokeCloud.cpp','Public/OCDeployableTrap.h','Private/OCDeployableTrap.cpp',
'Public/OCAntiArmorLauncher.h','Private/OCAntiArmorLauncher.cpp','Public/OCAntiArmorProjectile.h','Private/OCAntiArmorProjectile.cpp',
'Public/OCInteractableLight.h','Private/OCInteractableLight.cpp','Public/OCInteractableGate.h','Private/OCInteractableGate.cpp',
'Public/OCPlayerController.h','Private/OCPlayerController.cpp','Public/OCGameState.h','Private/OCGameState.cpp',
'Public/OCGameMode.h','Private/OCGameMode.cpp','Public/OCCharacter.h','Private/OCCharacter.cpp','Private/OCHUD.cpp',
]
missing=[f for f in required if not (src/f).exists()]
if missing:
    print('MISSING FILES',missing); sys.exit(1)
markers={
'Public/OCGameplayMode.h':['Sandbox / Test Range','SpawnWeaponRack','TeleportCollege'],
'Public/OCOrdnanceTypes.h':['Fragmentation','Smoke','Flash','AntiArmorGame','ObjectiveTrap'],
'Private/OCGameMode.cpp':['ParseOption(Options, TEXT("Mode"))','EOCGameplayMode::Sandbox','if (bSandboxMode) return;','IsSandboxGodMode()'],
'Private/OCPlayerController.cpp':['EKeys::F10','Spawn all weapons','AOCAntiArmorLauncher::StaticClass()','Toggle god mode','ResetInteractables','TeleportMuseum'],
'Private/OCCharacter.cpp':['EKeys::F','EKeys::Four','EKeys::M','EKeys::N','ServerThrowSelectedGrenade_Implementation','ServerDeploySelectedTrap_Implementation','%15','E  REPAIR VEHICLE'],
'Private/OCGrenadeProjectile.cpp':['ApplyRadialDamageWithFalloff','AOCSmokeCloud','ApplyFlashServer','LineTraceSingleByChannel'],
'Private/OCDeployableTrap.cpp':['DOREPLIFETIME(AOCDeployableTrap, TrapPreset)','IsEngineer()','UOCAntiArmorDamageType','SmokeTrap','FlashTrap'],
'Private/OCAntiArmorLauncher.cpp':['OC Anti-Armor Launcher','EOCWeaponClass::Launcher','AOCAntiArmorProjectile'],
'Private/OCAntiArmorProjectile.cpp':['UOCAntiArmorDamageType::StaticClass()','ApplyPointDamage','ApplyRadialDamageWithFalloff'],
'Private/OCEnterableHouse.cpp':['AOCInteractableLight::StaticClass()','AOCInteractableGate::StaticClass()'],
'Private/OCHUD.cpp':['SANDBOX / TEST RANGE','F10 ADMIN PANEL','DrawSandboxAdminPanel','GetFlashEffectAlpha'],
'Private/OCVehicleBase.cpp':['RepairVehicleServer'],
}
count=0
for rel, toks in markers.items():
    s=(src/rel).read_text(errors='ignore')
    for tok in toks:
        count+=1
        if tok not in s:
            print(f'MISSING MARKER {tok!r} in {rel}'); sys.exit(1)
# Character server RPC declarations must have implementations.
h=(src/'Public/OCCharacter.h').read_text(errors='ignore')
c=(src/'Private/OCCharacter.cpp').read_text(errors='ignore')
rpcs=re.findall(r'UFUNCTION\s*\(\s*Server[^)]*\)\s*(?:\n\s*)?(?:virtual\s+)?[\w:<>,*&\s]+\s+(\w+)\s*\(',h)
missing_rpc=[n for n in rpcs if f'{n}_Implementation' not in c]
if missing_rpc:
    print('MISSING CHARACTER RPC IMPLEMENTATIONS',missing_rpc);sys.exit(1)
# Lightweight delimiter balance ignoring strings/comments enough for accidental patch breakage.
for p in list(src.glob('Public/*.h'))+list(src.glob('Private/*.cpp')):
    text=p.read_text(errors='ignore')
    stripped=re.sub(r'//.*?$|/\*.*?\*/|"(?:\\.|[^"\\])*"', '', text, flags=re.M|re.S)
    for a,b in [('(',')'),('{','}'),('[',']')]:
        bal=0
        for ch in stripped:
            if ch==a: bal+=1
            elif ch==b: bal-=1
            if bal<0:
                print('DELIMITER UNDERFLOW',p.name,a,b);sys.exit(1)
        if bal!=0:
            print('DELIMITER IMBALANCE',p.name,a,b,bal);sys.exit(1)
# Docs and scripts.
for f in ['Docs/SESSION_12_README_UA.md','Docs/ORDNANCE_SANDBOX_ARCHITECTURE_S12.md','Docs/S12_TEST_MATRIX.md',
          'Scripts/RUN_S12_SANDBOX_SERVER_7777.bat','Scripts/RUN_S12_SANDBOX_CLIENT.bat','Scripts/RUN_S12_SANDBOX_SOLO.bat']:
    if not (proj/f).exists(): print('MISSING',f);sys.exit(1)
print('S12 structural verification: PASS')
print(f'Checked {len(required)} required files and {count} S12 functionality markers; {len(rpcs)} Character server RPCs present.')
