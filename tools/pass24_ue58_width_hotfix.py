from pathlib import Path

CPP = Path('OsterConflict/Source/OsterConflict/Private/OCR13FrontendMenuSubsystem.cpp')
VERIFY = Path('VERIFY_FRONTEND_SLATE_TRANSITION_PASS_24.py')

cpp = CPP.read_text(encoding='utf-8')
verify = VERIFY.read_text(encoding='utf-8')

old_cpp = '''        Field->SetWidgetStyle(FieldStyle);\n        Field->SetMinimumDesiredWidth(420.0f);\n'''
new_cpp = '''        Field->SetWidgetStyle(FieldStyle);\n'''
if cpp.count(old_cpp) != 1:
    raise SystemExit(f'expected exactly one unsupported width call block, found {cpp.count(old_cpp)}')
cpp = cpp.replace(old_cpp, new_cpp)

old_need = "    ('Field->SetMinimumDesiredWidth(420.0f);', 'stable field width'),\n"
if verify.count(old_need) != 1:
    raise SystemExit(f'expected exactly one verifier width requirement, found {verify.count(old_need)}')
verify = verify.replace(old_need, '')

old_forbid = "    ('FLinearColor(0.0f, 0.0f, 0.0f, 0.36f)', 'old translucent setup panel'),\n"
new_forbid = old_forbid + "    ('Field->SetMinimumDesiredWidth(', 'unsupported UEditableTextBox width API'),\n"
if verify.count(old_forbid) != 1:
    raise SystemExit(f'expected exactly one verifier forbid anchor, found {verify.count(old_forbid)}')
verify = verify.replace(old_forbid, new_forbid)

CPP.write_text(cpp, encoding='utf-8')
VERIFY.write_text(verify, encoding='utf-8')
print('PASS24 UE5.8 width hotfix applied')
