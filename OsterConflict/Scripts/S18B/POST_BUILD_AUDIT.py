#!/usr/bin/env python3
from __future__ import annotations
import argparse, hashlib, json
from pathlib import Path


def sha256(path: Path) -> str:
    h=hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda:f.read(1024*1024), b''):
            h.update(chunk)
    return h.hexdigest()


def find_exe(root: Path, preferred: str) -> Path | None:
    # Never accept arbitrary *Client.exe/*Server.exe helpers (for example CrashReportClient) as game binaries.
    for name in (preferred, 'OsterConflict.exe'):
        hits=sorted(root.rglob(name))
        if hits:
            return hits[0]
    return None


def scan(root: Path, preferred: str):
    exe=find_exe(root, preferred)
    containers=[p for ext in ('*.pak','*.utoc','*.ucas') for p in root.rglob(ext)]
    return exe, sorted(set(containers))

ap=argparse.ArgumentParser()
ap.add_argument('--client', required=True)
ap.add_argument('--server', required=True)
ap.add_argument('--out', required=True)
args=ap.parse_args()
client=Path(args.client).resolve(); server=Path(args.server).resolve(); out=Path(args.out).resolve(); out.mkdir(parents=True,exist_ok=True)
for label,path in [('client',client),('server',server)]:
    if not path.exists(): raise SystemExit(f'MISSING {label} archive directory: {path}')

client_exe,client_data=scan(client,'OsterConflictClient.exe')
server_exe,server_data=scan(server,'OsterConflictServer.exe')
if not client_exe: raise SystemExit('CLIENT EXE NOT FOUND')
if not server_exe: raise SystemExit('SERVER EXE NOT FOUND')
if not client_data: raise SystemExit('CLIENT COOKED DATA CONTAINER NOT FOUND (.pak/.utoc/.ucas)')
if not server_data: raise SystemExit('SERVER COOKED DATA CONTAINER NOT FOUND (.pak/.utoc/.ucas)')

files=[client_exe,server_exe]+client_data+server_data
manifest=[]
for p in files:
    manifest.append({'path':str(p),'bytes':p.stat().st_size,'sha256':sha256(p)})
report={
    'status':'PASS',
    'client_exe':str(client_exe),
    'server_exe':str(server_exe),
    'client_bytes':sum(p.stat().st_size for p in client.rglob('*') if p.is_file()),
    'server_bytes':sum(p.stat().st_size for p in server.rglob('*') if p.is_file()),
    'artifacts':manifest,
}
(out/'S18B_BUILD_MANIFEST.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
with (out/'S18B_SHA256.txt').open('w',encoding='utf-8') as f:
    for x in manifest: f.write(f"{x['sha256']}  {x['path']}\n")
print('POST_BUILD_AUDIT: PASS')
print(json.dumps({k:v for k,v in report.items() if k!='artifacts'},indent=2))
