from pathlib import Path

path = Path('OsterConflict/Source/OsterConflict/Private/OCR13MuseumStadiumPhotoFidelitySubsystem.cpp')
text = path.read_text(encoding='utf-8')

old_include = '#include "TimerManager.h"\n'
new_include = '#include "TimerManager.h"\n#include "UObject/UObjectGlobals.h"\n'
if old_include not in text:
    raise SystemExit('expected TimerManager include not found')
if '#include "UObject/UObjectGlobals.h"' not in text:
    text = text.replace(old_include, new_include, 1)

old_material = '        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Owner, Name);'
new_material = '''        const FName MaterialObjectName = MakeUniqueObjectName(\n            Owner, UMaterialInstanceDynamic::StaticClass(), FName(*(Name.ToString() + TEXT("_MID"))));\n        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Owner, MaterialObjectName);'''
if text.count(old_material) != 1:
    raise SystemExit(f'expected exactly one material creation line, found {text.count(old_material)}')
text = text.replace(old_material, new_material, 1)

old_component = '        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);'
new_component = '''        const FName ComponentObjectName = MakeUniqueObjectName(\n            Owner, UInstancedStaticMeshComponent::StaticClass(), Name);\n        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, ComponentObjectName);'''
if text.count(old_component) != 1:
    raise SystemExit(f'expected exactly one ISM creation line, found {text.count(old_component)}')
text = text.replace(old_component, new_component, 1)

required = [
    'FName(*(Name.ToString() + TEXT("_MID")))',
    'MakeUniqueObjectName(\n            Owner, UInstancedStaticMeshComponent::StaticClass(), Name)',
    'TEXT("R13_MuseumPhotoConcrete")',
    'TEXT("R13_MuseumPhotoGasPipe")',
]
for marker in required:
    if marker not in text:
        raise SystemExit(f'missing post-patch marker: {marker}')

path.write_text(text, encoding='utf-8')
print('R13 runtime UObject naming hotfix applied successfully')
