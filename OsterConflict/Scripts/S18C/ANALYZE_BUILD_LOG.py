#!/usr/bin/env python3
from pathlib import Path
import argparse,re,sys
ap=argparse.ArgumentParser(); ap.add_argument('log'); args=ap.parse_args()
p=Path(args.log)
if not p.exists(): raise SystemExit(f'log not found: {p}')
t=p.read_text(errors='ignore')
patterns=[
 ('UHT',r'UnrealHeaderTool|LogCompile: Error|Error:\s+.*(?:UCLASS|USTRUCT|UFUNCTION|UPROPERTY)|\.generated\.h'),
 ('C++',r'\berror C\d{4}\b|\bfatal error C\d{4}\b|\berror: .*\.cpp|\berror: .*\.h|\berror: use of|\berror: no member named'),
 ('LINK',r'\bLNK\d{4}\b|linker command failed'),
 ('COOK',r'Cook failed|LogCook: Error|Unable to find package|Can.t find file'),
 ('PACKAGE',r'PackagingResults: Error|AutomationTool exiting with ExitCode=[1-9]'),
 ('RUNTIME',r'Fatal error:|Assertion failed:|Unhandled Exception'),
]
found=[]
for kind,pat in patterns:
    hits=re.findall(pat,t,re.I)
    if hits: found.append((kind,len(hits)))
print(f'BUILD LOG: {p}')
if not found:
    print('No known fatal/build-error markers detected.')
    sys.exit(0)
for kind,n in found: print(f'{kind}: {n} marker(s)')
# Print bounded context around first likely error line.
lines=t.splitlines()
for i,line in enumerate(lines):
    if re.search(r'\berror C\d{4}\b|fatal error|\berror:|LogCompile: Error|Assertion failed|LogCook: Error|PackagingResults: Error|LNK\d{4}|AutomationTool exiting with ExitCode=[1-9]',line,re.I):
        print('\nFIRST ERROR CONTEXT:')
        for x in lines[max(0,i-4):min(len(lines),i+10)]: print(x)
        break
sys.exit(2)
