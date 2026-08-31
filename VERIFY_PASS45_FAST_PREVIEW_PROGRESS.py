from pathlib import Path

ROOT = Path(__file__).resolve().parent
launcher = (ROOT / 'RUN_PASS45_FAST_PREVIEW.cmd').read_text(encoding='utf-8')
progress = (ROOT / 'OsterConflict/Scripts/PASS45_FAST_PREVIEW_PROGRESS.ps1').read_text(encoding='utf-8')

required_launcher = [
    'PASS45_FAST_PREVIEW_PROGRESS.ps1',
    'PASS45_FAST_PREVIEW_PROGRESS.state',
    '-LogPath "%PREVIEW_LOG%"',
    '-StatePath "%PROGRESS_STATE%"',
    'exit_code=%GAME_RC%',
    'PREVIEW ONLY',
]
for needle in required_launcher:
    assert needle in launcher, f'missing Fast Preview launcher progress contract: {needle}'

required_progress = [
    'System.Windows.Forms.ProgressBar',
    "'5%'",
    'Startup stage estimate',
    'Elapsed:',
    'Last UE activity:',
    'Building static mesh',
    'Building texture',
    'Waiting for static meshes',
    'PASS27_FRONTEND_WIDGETTREE_OWNED',
    '100',
    '90+ seconds',
    'exit_code=',
    'Fast Preview remains preview-only',
]
for needle in required_progress:
    assert needle in progress, f'missing visible startup progress contract: {needle}'

assert 'RUNTIME ACCEPTED' not in progress, 'progress UI must never claim runtime acceptance'
assert 'runtime acceptance remains unchanged' in launcher, 'Fast Preview must remain acceptance-neutral'

print('PASS45 Fast Preview visible startup progress: PASS')
