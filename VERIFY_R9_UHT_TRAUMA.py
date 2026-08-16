from pathlib import Path
import re, sys
ROOT=Path(__file__).resolve().parent
TYPES=ROOT/'OsterConflict/Source/OsterConflict/Public/OCTraumaTypes.h'
COMP=ROOT/'OsterConflict/Source/OsterConflict/Public/OCCombatVisualComponent.h'
text=TYPES.read_text(encoding='utf-8')
comp=COMP.read_text(encoding='utf-8')
checks=[]
def ck(name, ok):
    checks.append((name,bool(ok)))
    print(('PASS' if ok else 'FAIL')+': '+name)
ck('Trauma Sequence is Blueprint-supported int32', 'UPROPERTY(BlueprintReadOnly) int32 Sequence = 0;' in text)
ck('No Blueprint-exposed uint16 remains in OCTraumaTypes', not re.search(r'UPROPERTY\([^\n]*Blueprint[^\n]*\)\s*uint16\b', text))
ck('ServerSequence matches int32 replicated sequence', 'int32 ServerSequence = 0;' in comp)
ck('LastRenderedSequence matches int32 replicated sequence', 'int32 LastRenderedSequence = 0;' in comp)
# Broad reflected-source guard for the exact class of UHT error we hit.
bad=[]
for h in (ROOT/'OsterConflict/Source').rglob('*.h'):
    t=h.read_text(encoding='utf-8', errors='ignore')
    for m in re.finditer(r'UPROPERTY\([^\n]*(?:BlueprintReadOnly|BlueprintReadWrite)[^\n]*\)\s*uint16\b', t):
        bad.append(str(h.relative_to(ROOT)))
ck('No Blueprint-exposed uint16 UPROPERTY in project headers', not bad)
if bad: print('  offenders:', *bad, sep='\n  - ')
if not all(ok for _,ok in checks): sys.exit(1)
print(f'R9 UHT trauma verifier: PASS ({len(checks)} checks)')
