from pathlib import Path

root = Path(__file__).resolve().parent
cpp_path = root / "OsterConflict/Source/OsterConflict/Private/OCR13FrontendMenuSubsystem.cpp"
launcher_path = root / "RUN_R14_CURRENT_GAMEPLAY.cmd"
ledger_path = root / "OSTER_CONFLICT_WORK_LEDGER.md"

cpp = cpp_path.read_text(encoding="utf-8")
launcher = launcher_path.read_text(encoding="utf-8")
ledger = ledger_path.read_text(encoding="utf-8")


def replace_once(text: str, old: str, new: str, label: str) -> str:
    if old not in text:
        raise SystemExit(f"PASS27 patch failed: missing {label}")
    return text.replace(old, new, 1)


# The native root explicitly builds all UMG widgets through WidgetTree->ConstructWidget.
# The R13 overlay was the exception: it created live UWidgets with NewObject after the root Slate
# tree already existed. Move the overlay onto the same ownership/lifetime path as the stable root.
cpp = replace_once(
    cpp,
    '#include "OCR13FrontendMenuSubsystem.h"\n',
    '#include "OCR13FrontendMenuSubsystem.h"\n\n#include "Blueprint/WidgetTree.h"\n',
    "WidgetTree include",
)
cpp = replace_once(cpp, 'UTextBlock* R13FrontendMakeMenuText(UObject* Outer,', 'UTextBlock* R13FrontendMakeMenuText(UWidgetTree* Tree,', 'text helper signature')
cpp = replace_once(cpp, '        UTextBlock* Block = NewObject<UTextBlock>(Outer);', '        if (!Tree) return nullptr;\n        UTextBlock* Block = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());', 'text helper construction')
cpp = replace_once(cpp, 'UButton* R13FrontendMakeMenuButton(UObject* Outer,', 'UButton* R13FrontendMakeMenuButton(UWidgetTree* Tree,', 'button helper signature')
cpp = replace_once(cpp, '        if (!Outer || !Parent) return nullptr;\n\n        USizeBox* Size = NewObject<USizeBox>(Outer);\n        UButton* Button = NewObject<UButton>(Outer);\n        UTextBlock* Text = R13FrontendMakeMenuText(Outer, Label, 16, true);', '        if (!Tree || !Parent) return nullptr;\n\n        USizeBox* Size = Tree->ConstructWidget<USizeBox>(USizeBox::StaticClass());\n        UButton* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass());\n        UTextBlock* Text = R13FrontendMakeMenuText(Tree, Label, 16, true);', 'button helper construction')
cpp = replace_once(cpp, 'UEditableTextBox* R13FrontendMakeField(UObject* Outer,', 'UEditableTextBox* R13FrontendMakeField(UWidgetTree* Tree,', 'field helper signature')
cpp = replace_once(cpp, '        if (!Outer || !Parent) return nullptr;\n        UEditableTextBox* Field = NewObject<UEditableTextBox>(Outer);', '        if (!Tree || !Parent) return nullptr;\n        UEditableTextBox* Field = Tree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass());', 'field helper construction')

cpp = replace_once(
    cpp,
    'void UOCR13FrontendMenuSubsystem::BuildFrontend(UOCGameUIRootWidget* Root, AOCPlayerController* PC)\n{\n    if (!Root || !PC) return;\n\n    UCanvasPanel* Canvas = Cast<UCanvasPanel>(Root->GetWidgetFromName(TEXT("OC_UI_Root")));',
    'void UOCR13FrontendMenuSubsystem::BuildFrontend(UOCGameUIRootWidget* Root, AOCPlayerController* PC)\n{\n    if (!Root || !PC) return;\n\n    UWidgetTree* Tree = Root->WidgetTree;\n    if (!Tree)\n    {\n        UE_LOG(LogTemp, Error, TEXT("PASS27_FRONTEND_WIDGETTREE_MISSING"));\n        return;\n    }\n\n    UCanvasPanel* Canvas = Cast<UCanvasPanel>(Root->GetWidgetFromName(TEXT("OC_UI_Root")));',
    "BuildFrontend tree acquisition",
)

replacements = {
    'NewObject<UBorder>(Root, TEXT("R13_MenuWorldBlocker"))': 'Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("R13_MenuWorldBlocker"))',
    'NewObject<UImage>(Root, TEXT("R13_MenuBackground"))': 'Tree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("R13_MenuBackground"))',
    'NewObject<UBorder>(Root, TEXT("R13_MenuShade"))': 'Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("R13_MenuShade"))',
    'NewObject<UBorder>(Root, TEXT("R13_MenuPanel"))': 'Tree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("R13_MenuPanel"))',
    'NewObject<UVerticalBox>(Root, TEXT("R13_PlayerFrontend"))': 'Tree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("R13_PlayerFrontend"))',
    'NewObject<UBorder>(Root)': 'Tree->ConstructWidget<UBorder>(UBorder::StaticClass())',
    'NewObject<UVerticalBox>(Root, TEXT("R13_FrontendFields"))': 'Tree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("R13_FrontendFields"))',
    'R13FrontendMakeMenuText(Root,': 'R13FrontendMakeMenuText(Tree,',
    'R13FrontendMakeMenuButton(Root,': 'R13FrontendMakeMenuButton(Tree,',
    'R13FrontendMakeField(\n        Root,': 'R13FrontendMakeField(\n        Tree,',
}
for old, new in replacements.items():
    if old not in cpp:
        raise SystemExit(f"PASS27 patch failed: missing token {old}")
    cpp = cpp.replace(old, new)

if 'NewObject<' in cpp:
    raise SystemExit('PASS27 patch failed: OCR13 frontend still contains NewObject widget construction')

cpp = replace_once(
    cpp,
    '    ApplyPage();\n}\n\nvoid UOCR13FrontendMenuSubsystem::ApplyPage()',
    '    ApplyPage();\n    UE_LOG(LogTemp, Display, TEXT("PASS27_FRONTEND_WIDGETTREE_OWNED"));\n}\n\nvoid UOCR13FrontendMenuSubsystem::ApplyPage()',
    "runtime ownership marker",
)

# On any future non-zero Unreal exit, surface the actual gameplay log automatically. No more blind
# Crash Reporter screenshots while the useful PASS markers sit in a different file.
launcher = replace_once(
    launcher,
    'set "GAME_RC=%ERRORLEVEL%"\n\nif "%IS_ACCEPTANCE%"=="1" (',
    'set "GAME_RC=%ERRORLEVEL%"\n\nif not "%GAME_RC%"=="0" (\n  echo.\n  echo [CRASH-DIAGNOSTICS] Unreal exited with code %GAME_RC%.\n  echo [CRASH-DIAGNOSTICS] Relevant frontend markers:\n  if exist "%PLAYTEST_LOG%" findstr /C:"PASS27_" /C:"PASS26_" /C:"PASS25_" /C:"PASS24_" /C:"R13 frontend:" "%PLAYTEST_LOG%"\n  echo [CRASH-DIAGNOSTICS] Last 180 gameplay-log lines:\n  if exist "%PLAYTEST_LOG%" powershell -NoProfile -Command "Get-Content -LiteralPath $env:PLAYTEST_LOG -Tail 180"\n)\n\nif "%IS_ACCEPTANCE%"=="1" (',
    "automatic crash diagnostics",
)

ledger = ledger.replace('Active correction branch: `fix/frontend-slate-crash-pass-26-20260823` → `main`', 'Active correction branch: `fix/frontend-widgettree-pass-27-20260823` → `main`')
ledger = ledger.replace('User playtest 2026-08-23 на `main` commit `6d1ff2605573c4a1cdcf51e132ac56f986db216a` є authoritative evidence для frontend crash: UE 5.8 assertion у Slate/SlateCore після Pass 25, `Array index out of bounds: -808103970 into an array of size 0`.', 'User playtest 2026-08-23 повторив той самий Slate/SlateCore assertion уже на `main` commit `f2d397f8b9a2348576dcf96b0c20522a8a8c8d8f` після Pass 26: `Array index out of bounds: -808103970 into an array of size 0`. Pass 26 lifecycle fence не усунув crash, тому ця runtime evidence має пріоритет над зеленим CI.')
ledger = ledger.replace('| UI-MENU-001 | Головне меню стабільне | ≥4 | IN_PROGRESS | 2026-08-23 runtime на Pass 25 (`6d1ff260...`) зібрався і відкрив frontend, але frontend interaction завершилась assertion crash у Slate/SlateCore: negative array index into size 0. Pass 26 source hardening coded; потрібен новий UE 5.8 build/runtime. |', '| UI-MENU-001 | Головне меню стабільне | ≥5 | IN_PROGRESS | 2026-08-23 runtime повторив той самий Slate/SlateCore array assertion вже на Pass 26 (`f2d397f8...`). Новий concrete suspect: R13 overlay створював live UMG widgets через `NewObject` після побудови native `WidgetTree`, тоді як стабільний `UOCGameUIRootWidget` всюди використовує `WidgetTree->ConstructWidget`. Pass 27 переводить весь R13 frontend на WidgetTree ownership; runtime acceptance обов’язковий. |')
ledger = ledger.replace('| CRASH-FRONTEND-SLATE-20260823 | Pass 25 frontend click → Slate/SlateCore array assertion | CODED_UNTESTED | Pass 26: усі `OnClicked` лише ставлять action у чергу; виконання fenced мінімум на наступний engine frame; legacy suppression більше не кожен Tick; presentation/pause invalidation dedupe. Runtime acceptance pending. |', '| CRASH-FRONTEND-SLATE-20260823 | Frontend interaction → Slate/SlateCore array assertion | CODED_UNTESTED | Pass 26 runtime FAILED: exact assertion repeated on `f2d397f8...`. Pass 27 aligns the R13 overlay with the native root lifecycle by constructing every UMG widget through `Root->WidgetTree->ConstructWidget`; direct `NewObject<UWidget>` construction is forbidden in this frontend. Launcher also auto-prints PASS markers + last 180 gameplay-log lines on non-zero exit. Runtime acceptance pending. |')

cpp_path.write_text(cpp, encoding="utf-8")
launcher_path.write_text(launcher, encoding="utf-8")
ledger_path.write_text(ledger, encoding="utf-8")

print("PASS27 PATCH APPLIED")
print("- OCR13 frontend widgets now belong to UOCGameUIRootWidget::WidgetTree")
print("- direct NewObject<UWidget> construction removed from the R13 frontend")
print("- crash launcher automatically surfaces gameplay-log markers and tail")
print("- ledger records Pass 26 runtime failure and Pass 27 as CODED_UNTESTED")