# Pass 31 — gameplay input recovery

Runtime symptom being addressed: after deployment a local player can possess an `AOCCharacter` yet remain unable to move/look if the possession occurs while deployment/frontend UI input locks or stale high-priority Enhanced Input mappings are still being released.

Source changes:
- broaden `UOCVehicleExitInputRecoverySubsystem` from vehicle-only recovery to all local character possession transitions;
- keep polling after possession if an intentional UI lock is still active, instead of missing the one frame where the pawn changes;
- rebuild Enhanced Input mappings once the UI is genuinely released;
- reset stack-based move/look ignore state and restore `FInputModeGameOnly`;
- retain vehicle-exit recovery and make the same path cover initial deployment and respawn;
- add `PASS31_GAMEPLAY_INPUT_READY` runtime evidence;
- add `VERIFY_GAMEPLAY_INPUT_PASS_31.py` to the root regression suite.

Acceptance remains runtime-dependent: a local UE 5.8 playtest must confirm WASD + mouse movement after spawn outside the museum.
