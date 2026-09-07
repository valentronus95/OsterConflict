#!/usr/bin/env python3
from pathlib import Path

TARGET = Path('PASS45_REMINGTON870_PMAG_SPATIAL_PARTITION_AUDIT.py')
if not TARGET.is_file():
    raise SystemExit('PASS45 REMINGTON870 PMAG SPATIAL PARTITION CONTRACT: FAIL\n[FAIL] audit missing')
text = TARGET.read_text(encoding='utf-8')
required = (
    'EXPECTED_SHA256 = "147aa6a0b167ba3f0806ad19a1cb6cc8790a0d541079f54d2e0fa8cf829954a2"',
    'MIN_COMPONENTS_PER_SIDE = 4',
    'MIN_SEPARATION = 0.25',
    'partition_axis": "Y"',
    'direct_pmag_as_pump_mapping": "REJECTED"',
    'fore_end_group_identity": "UNPROVEN"',
    'standalone_pump_clip": "UNPROVEN"',
    'production_cutover": False',
    'runtime_acceptance": False',
    'item16_checked": False',
)
missing = [needle for needle in required if needle not in text]
if missing:
    raise SystemExit('PASS45 REMINGTON870 PMAG SPATIAL PARTITION CONTRACT: FAIL\n[FAIL] missing=' + repr(missing))
for forbidden in ('fore_end_group_identity": "PROVEN"', 'production_cutover": True', 'item16_checked": True'):
    if forbidden in text:
        raise SystemExit('PASS45 REMINGTON870 PMAG SPATIAL PARTITION CONTRACT: FAIL\n[FAIL] forbidden=' + forbidden)
print('PASS45 REMINGTON870 PMAG SPATIAL PARTITION CONTRACT: PASS')
print('exact_donor=1 direct_mapping_rejected=1 semantic_acceptance=0 production_cutover=0 runtime_acceptance=0 item16_checked=0')
