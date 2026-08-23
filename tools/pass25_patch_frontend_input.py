from pathlib import Path

HDR = Path('OsterConflict/Source/OsterConflict/Public/OCR13FrontendMenuSubsystem.h')
CPP = Path('OsterConflict/Source/OsterConflict/Private/OCR13FrontendMenuSubsystem.cpp')

hdr = HDR.read_text(encoding='utf-8')
cpp = CPP.read_text(encoding='utf-8')

old_hdr = '''    bool bPendingHostedStart = false;\n    bool bPendingNetworkConnect = false;\n    bool bGameplayStarted = false;\n'''
new_hdr = '''    bool bPendingHostedStart = false;\n    bool bPendingNetworkConnect = false;\n    bool bMenuInputArmed = false; // Pass 25: SetInputMode is armed once, never reset every Tick\n    bool bGameplayStarted = false;\n'''
if hdr.count(old_hdr) != 1:
    raise SystemExit(f'header anchor count={hdr.count(old_hdr)}')
hdr = hdr.replace(old_hdr, new_hdr)

old_ensure = '''    if (ActiveRoot.Get() == Root && MenuBox.IsValid() && MenuPanel.IsValid())\n    {\n        ActiveController = PC;\n        return;\n    }\n'''
new_ensure = '''    if (ActiveRoot.Get() == Root && MenuBox.IsValid() && MenuPanel.IsValid())\n    {\n        if (ActiveController.Get() != PC) bMenuInputArmed = false;\n        ActiveController = PC;\n        return;\n    }\n'''
if cpp.count(old_ensure) != 1:
    raise SystemExit(f'ensure anchor count={cpp.count(old_ensure)}')
cpp = cpp.replace(old_ensure, new_ensure)

old_reset = '''    bPendingHostedStart = false;\n    bPendingNetworkConnect = false;\n    bGameplayStarted = false;\n'''
new_reset = '''    bPendingHostedStart = false;\n    bPendingNetworkConnect = false;\n    bMenuInputArmed = false;\n    bGameplayStarted = false;\n'''
if cpp.count(old_reset) != 1:
    raise SystemExit(f'reset anchor count={cpp.count(old_reset)}')
cpp = cpp.replace(old_reset, new_reset)

old_force = '''void UOCR13FrontendMenuSubsystem::ForceMenuInput()\n{\n    AOCPlayerController* PC = ActiveController.Get();\n    if (!PC || !MenuBox.IsValid()) return;\n\n    PC->ResetIgnoreMoveInput();\n    PC->ResetIgnoreLookInput();\n    PC->SetIgnoreMoveInput(true);\n    PC->SetIgnoreLookInput(true);\n    PC->bShowMouseCursor = true;\n    PC->bEnableClickEvents = true;\n    PC->bEnableMouseOverEvents = true;\n\n    FInputModeUIOnly Mode;\n    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);\n    // Pass 24: do not recreate/focus an SWidget every Tick. Mouse input remains UI-only;\n    // focus is acquired naturally by the clicked control.\n    PC->SetInputMode(Mode);\n}\n\nvoid UOCR13FrontendMenuSubsystem::ReleaseMenuInput()\n{\n    AOCPlayerController* PC = ActiveController.Get();\n    if (!PC) return;\n\n    PC->ResetIgnoreMoveInput();\n    PC->ResetIgnoreLookInput();\n    PC->bShowMouseCursor = false;\n    PC->bEnableClickEvents = false;\n    PC->bEnableMouseOverEvents = false;\n    if (PC->PlayerInput) PC->PlayerInput->FlushPressedKeys();\n    PC->SetInputMode(FInputModeGameOnly());\n}\n'''
new_force = '''void UOCR13FrontendMenuSubsystem::ForceMenuInput()\n{\n    AOCPlayerController* PC = ActiveController.Get();\n    if (!PC || !MenuBox.IsValid()) return;\n\n    // Pass 25: OnClicked fires on mouse release. Re-applying SetInputMode every world Tick\n    // can reset Slate mouse capture between press and release, leaving every button visually\n    // present but inert. Arm UI input once per menu/controller lifecycle instead.\n    if (bMenuInputArmed) return;\n\n    PC->ResetIgnoreMoveInput();\n    PC->ResetIgnoreLookInput();\n    PC->SetIgnoreMoveInput(true);\n    PC->SetIgnoreLookInput(true);\n    PC->bShowMouseCursor = true;\n    PC->bEnableClickEvents = true;\n    PC->bEnableMouseOverEvents = true;\n\n    FInputModeUIOnly Mode;\n    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);\n    PC->SetInputMode(Mode);\n    bMenuInputArmed = true;\n    UE_LOG(LogTemp, Display, TEXT("PASS25_MENU_INPUT_ARMED"));\n}\n\nvoid UOCR13FrontendMenuSubsystem::ReleaseMenuInput()\n{\n    bMenuInputArmed = false;\n    AOCPlayerController* PC = ActiveController.Get();\n    if (!PC) return;\n\n    PC->ResetIgnoreMoveInput();\n    PC->ResetIgnoreLookInput();\n    PC->bShowMouseCursor = false;\n    PC->bEnableClickEvents = false;\n    PC->bEnableMouseOverEvents = false;\n    if (PC->PlayerInput) PC->PlayerInput->FlushPressedKeys();\n    PC->SetInputMode(FInputModeGameOnly());\n}\n'''
if cpp.count(old_force) != 1:
    raise SystemExit(f'force/release anchor count={cpp.count(old_force)}')
cpp = cpp.replace(old_force, new_force)

HDR.write_text(hdr, encoding='utf-8')
CPP.write_text(cpp, encoding='utf-8')
print('PASS25 frontend input patch applied')
