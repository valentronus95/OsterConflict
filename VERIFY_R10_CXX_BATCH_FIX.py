from pathlib import Path
import re
ROOT = Path(__file__).resolve().parent
P = ROOT / 'OsterConflict'
SRC = P / 'Source' / 'OsterConflict'
checks = []

def require(name, cond):
    if not cond:
        raise SystemExit(f'FAIL: {name}')
    checks.append(name)
    print(f'PASS: {name}')

def read(rel):
    return (SRC / rel).read_text(encoding='utf-8', errors='replace')

ai_h = read('Public/OCAIController.h')
ai_cpp = read('Private/OCAIController.cpp')
gm_h = read('Public/OCGameMode.h')
gm_cpp = read('Private/OCGameMode.cpp')
ps_h = read('Public/OCPlayerState.h')
armed = read('Private/OCArmedVehicleBase.cpp')
btr = read('Private/OCBTR.cpp')
ui_h = read('Public/OCGameUIRootWidget.h')
ui_cpp = read('Private/OCGameUIRootWidget.cpp')
grenade = read('Private/OCGrenadeProjectile.cpp')
pc = read('Private/OCPlayerController.cpp')
deployment_bridge = read('Private/OCR13DeploymentSelectionBridge.cpp')

require('AI Role shadow renamed', 'EOCPlayerRole BotRole' in ai_h and 'EOCPlayerRole Role' not in ai_h)
require('AI Character shadow renamed', 'AOCCharacter* SensedCharacter' in ai_cpp and 'AOCCharacter* Character = Cast<AOCCharacter>(Actor)' not in ai_cpp)
require('GameMode bot Role shadow renamed', 'SpawnSingleBot(EOCTeam Team, EOCPlayerRole BotRole)' in gm_h and 'const EOCPlayerRole BotRole' in gm_cpp)
require('GameMode parsed Role local renamed', 'FString RoleOption' in gm_cpp and 'FString Role =' not in gm_cpp)
require('PlayerState exposes IsEngineer', 'bool IsEngineer() const' in ps_h)
require('Armed vehicle has BoxComponent complete type', '#include "Components/BoxComponent.h"' in armed)
require('Armed vehicle attachment uses raw scene pointer', 'TurretPivot->SetupAttachment(PhysicsBody.Get())' in armed)
require('Turret damage type fallback is non-ambiguous', 'AppliedDamageType = TurretDamageTypeClass;' in armed and '? TurretDamageTypeClass :' not in armed)
require('BTR has FDamageEvent definition include', '#include "Engine/DamageEvents.h"' in btr)
require('Grenade has full CameraComponent include', '#include "Camera/CameraComponent.h"' in grenade)

# The original C4458 issue was inside UOCGameUIRootWidget, whose UObject hierarchy already exposes Slot-related names.
# A local variable called Slot in unrelated helper/subsystem classes is legal, so keep this regression guard scoped.
require('UI Slot shadow names removed', not re.search(r'\b(?:UVerticalBoxSlot|UCanvasPanelSlot|UHorizontalBoxSlot)\*\s+Slot\b', ui_cpp))
require('UI slider out parameter uses raw temporary bridge', 'TObjectPtr<UTextBlock>& OutValueText' not in ui_h and 'UTextBlock* ValueText = nullptr' in ui_cpp)
require('PlayerController Character shadow removed', 'AOCCharacter* Character = Cast<AOCCharacter>(GetPawn())' not in pc and 'AOCCharacter* ControlledCharacter' in pc)

# AOCGameMode inherits AGameModeBase::GameState. With the project's warning policy, declaring a local GameState inside
# AOCGameMode methods is C4458 and is a hard compile error. Keep the deployment bridge on an unambiguous local name.
require('Deployment GameState shadow removed',
        'const AOCGameState* CurrentGameState = GetGameState<AOCGameState>()' in deployment_bridge and
        'const AOCGameState* GameState = GetGameState<AOCGameState>()' not in deployment_bridge)
require('Deployment roster uses renamed GameState local', 'CurrentGameState->PlayerArray' in deployment_bridge)

for rel in ['Public/OCBreakableWindow.h','Public/OCInteractableDoor.h','Public/OCInteractableGate.h','Public/OCInteractableLight.h']:
    require(f'{rel} declares WorldAudioComponent', 'TObjectPtr<UOCWorldAudioComponent> WorldAudioComponent' in read(rel))

all_cpp = '\n'.join(p.read_text(encoding='utf-8', errors='replace') for p in SRC.rglob('*.cpp'))
require('No deprecated direct NetUpdateFrequency writes', not re.search(r'(?m)^\s*NetUpdateFrequency\s*=', all_cpp))
require('No deprecated direct MinNetUpdateFrequency writes', not re.search(r'(?m)^\s*MinNetUpdateFrequency\s*=', all_cpp))
require('No deprecated direct NetCullDistanceSquared writes', not re.search(r'(?m)^\s*NetCullDistanceSquared\s*=', all_cpp))

# Exact non-contextual R9 compiler blocker signatures should be gone from all source.
forbidden = [
    'EOCPlayerRole Role, EOCBotDifficulty Difficulty',
    'TurretPivot->SetupAttachment(PhysicsBody);',
    'TurretDamageTypeClass ? TurretDamageTypeClass :',
]
joined = '\n'.join(p.read_text(encoding='utf-8', errors='replace') for p in SRC.rglob('*') if p.suffix in {'.h','.cpp'})
for token in forbidden:
    require(f'R9 blocker removed: {token}', token not in joined)

print(f'R10 C++ batch-fix verifier: PASS ({len(checks)} checks)')
