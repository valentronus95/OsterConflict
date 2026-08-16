from pathlib import Path
import subprocess,sys,re
ROOT=Path(__file__).resolve().parent; P=ROOT/'OsterConflict'
required=[
 'Source/OsterConflict/Public/OCGameInstance.h','Source/OsterConflict/Private/OCGameInstance.cpp',
 'Source/OsterConflict/Public/OCBuildVersion.h','Source/OsterConflict/Public/OCGameMode.h','Source/OsterConflict/Private/OCGameMode.cpp',
 'Source/OsterConflict/Public/OCPlayerController.h','Source/OsterConflict/Private/OCPlayerController.cpp',
 'Source/OsterConflict/Public/OCPlayerUserSettings.h','Source/OsterConflict/Private/OCGameUIRootWidget.cpp',
 'Config/DefaultEngine.ini','Docs/S18C_HARDENING_R1.md'
]
for r in required:
    if not (P/r).exists(): raise SystemExit('MISSING '+r)
text='\n'.join((P/r).read_text(errors='ignore') for r in required)
markers=[
 'GameInstanceClass=/Script/OsterConflict.OCGameInstance','NetServerMaxTickRate=30',
 'OnNetworkFailure().AddUObject','OnTravelFailure().AddUObject','VERSION_MISMATCH',
 'ProtocolOption','?Protocol=%d','CanUseSandboxAdmin','SandboxAdminAll','UE_BUILD_SHIPPING',
 'ClientSetSandboxAdminAllowed','SettingsSchemaVersion','CurrentSettingsSchemaVersion',
 'Voice Chat is P2'
]
missing=[m for m in markers if m not in text]
if missing: raise SystemExit('MISSING MARKERS: '+', '.join(missing))
# Sandbox mode alone must not be the admin predicate anymore.
pc=(P/'Source/OsterConflict/Private/OCPlayerController.cpp').read_text(errors='ignore')
admin=re.search(r'bool AOCPlayerController::IsSandboxAdmin\(\) const\s*\{(.*?)\n\}',pc,re.S)
if not admin or 'State->IsSandboxMode()' in admin.group(1): raise SystemExit('ADMIN POLICY STILL TRUSTS SANDBOX MODE ALONE')
# Runtime fingerprint intentionally remains S18B/18 in this hardening pass.
bv=(P/'Source/OsterConflict/Public/OCBuildVersion.h').read_text(errors='ignore')
for marker in ['Milestone = TEXT("S18B")','ProjectVersion = TEXT("0.0.18B-S18B")','NetworkProtocol = 18']:
    if marker not in bv: raise SystemExit('UNEXPECTED BUILD FINGERPRINT CHANGE: '+marker)
# Run existing S18C verifier as prerequisite.
run=subprocess.run([sys.executable,str(ROOT/'VERIFY_S18C.py')],cwd=ROOT,text=True,capture_output=True)
print(run.stdout,end='')
if run.returncode: print(run.stderr,end=''); raise SystemExit(run.returncode)
print('S18C HARDENING R1 structural verification: PASS')
