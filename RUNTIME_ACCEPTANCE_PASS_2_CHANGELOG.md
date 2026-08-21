# Runtime acceptance pass 2 changelog

- Fixed main-menu jump by removing duplicate geometry ownership from the viewport stabilizer.
- Reasserted museum-adjacent BASE placement in `AOCTeamSpawnPoint::BeginPlay` for serialized PlayerStarts.
- Kept dense foliage subsystem alive while frontend is active and populate once gameplay becomes ready.
- Rebased local tracer and muzzle presentation from camera trace origin to visible current-weapon geometry.
- Normal gameplay now gates on the full real HMMWV + M2 + BTR-4 production importer.
- Dedicated M2/BTR import helpers reject authored approximations for acceptance runs.
- Added `VERIFY_RUNTIME_ACCEPTANCE_PASS_2.py` and focused GitHub Actions contract workflow.

All changes remain `CODED_UNTESTED` until a local UE 5.8 Windows build/playtest confirms runtime behavior.
