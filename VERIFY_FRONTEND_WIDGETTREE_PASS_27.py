from pathlib import Path

root = Path(__file__).resolve().parent
cpp = (root / 'OsterConflict/Source/OsterConflict/Private/OCR13FrontendMenuSubsystem.cpp').read_text(encoding='utf-8')
root_cpp = (root / 'OsterConflict/Source/OsterConflict/Private/OCGameUIRootWidget.cpp').read_text(encoding='utf-8')
launcher = (root / 'RUN_R14_CURRENT_GAMEPLAY.cmd').read_text(encoding='utf-8')

errors = []

def need(text: str, token: str, label: str):
    if token not in text:
        errors.append(f'missing {label}: {token}')

def forbid(text: str, token: str, label: str):
    if token in text:
        errors.append(f'forbidden {label}: {token}')

# The stable native root is the lifecycle authority: it constructs source-built UMG through WidgetTree.
need(root_cpp, 'WidgetTree->ConstructWidget<UTextBlock>', 'native root WidgetTree text construction')
need(root_cpp, 'WidgetTree->ConstructWidget<UEditableTextBox>', 'native root WidgetTree field construction')

need(cpp, '#include "Blueprint/WidgetTree.h"', 'WidgetTree include')
need(cpp, 'UWidgetTree* Tree = Root->WidgetTree;', 'root WidgetTree acquisition')
need(cpp, 'R13FrontendMakeMenuText(UWidgetTree* Tree', 'WidgetTree text helper')
need(cpp, 'R13FrontendMakeMenuButton(UWidgetTree* Tree', 'WidgetTree button helper')
need(cpp, 'R13FrontendMakeField(UWidgetTree* Tree', 'WidgetTree field helper')
for token, label in [
    ('Tree->ConstructWidget<UBorder>', 'border construction'),
    ('Tree->ConstructWidget<UImage>', 'image construction'),
    ('Tree->ConstructWidget<UVerticalBox>', 'vertical box construction'),
    ('Tree->ConstructWidget<USizeBox>', 'size box construction'),
    ('Tree->ConstructWidget<UButton>', 'button construction'),
    ('Tree->ConstructWidget<UTextBlock>', 'text construction'),
    ('Tree->ConstructWidget<UEditableTextBox>', 'editable field construction'),
    ('PASS27_FRONTEND_WIDGETTREE_OWNED', 'runtime ownership marker'),
    ('PASS27_FRONTEND_WIDGETTREE_MISSING', 'missing-tree diagnostic marker'),
    ('PASS26_FRONTEND_ACTION_FENCE', 'Pass 26 action fence compatibility'),
    ('Primary->OnClicked.AddDynamic', 'OnClicked primary compatibility'),
]:
    need(cpp, token, label)

forbid(cpp, 'NewObject<', 'direct R13 UWidget NewObject construction')
forbid(cpp, 'LegacyFrontend->RemoveFromParent()', 'post-RebuildWidget legacy frontend detach')

need(launcher, '[CRASH-DIAGNOSTICS] Unreal exited with code %GAME_RC%.', 'automatic crash diagnostic header')
need(launcher, '/C:"PASS27_"', 'Pass 27 crash marker extraction')
need(launcher, 'Get-Content -LiteralPath $env:PLAYTEST_LOG -Tail 180', 'automatic gameplay log tail')

if errors:
    print('FRONTEND WIDGETTREE PASS 27: FAIL')
    for error in errors:
        print('[FAIL]', error)
    raise SystemExit(1)

print('FRONTEND WIDGETTREE PASS 27: SUCCESS')
print('- R13 overlay widgets use the same WidgetTree ownership path as UOCGameUIRootWidget')
print('- no direct NewObject<UWidget> construction remains in the frontend subsystem')
print('- native legacy frontend stays attached/collapsed instead of being structurally detached')
print('- any future non-zero Unreal exit automatically prints frontend markers and the gameplay-log tail')
print('STATUS: SOURCE CONTRACT ONLY; local UE 5.8 runtime confirmation is still required')