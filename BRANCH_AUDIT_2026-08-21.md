# OsterConflict branch audit — 2026-08-21

Purpose: establish which branch is the coherent current game and stop using branch age or launcher labels as a proxy for visual correctness.

## Result

`main` remains the integration branch. No other branch is a complete newer replacement for the whole game. Several older/diverged branches preserve specific good visual fixes or staging work; those fixes must be forward-ported selectively instead of switching the whole project back to those branches.

The 2026-08-21 visual regression was real and had three independent causes:

1. The current option-5 launcher was opening `Sandbox / Test Range` directly, while the earlier accepted screenshots came from the normal frontend -> TEAM gameplay path.
2. `OCVisualEnvironment.cpp` in main had regressed to a 70000 intensity sun and foggy/warm settings. The accepted R13 playtest branch still contained the verified neutral daylight baseline: intensity 4, white sun, blue Earth-like atmosphere and no height fog.
3. The AK-47 first-person axis correction was lost during the production-weapon refactor. History commit `7809756d175c3c61c6d201d6927978552562bff6` explicitly established that the Fab AK long axis is Y while the game attach convention is X-forward and used a -90 degree yaw for equipped first-person presentation.

These three issues were forward-ported/fixed in current main instead of rolling the project back.

## Branches inspected

### Integration / backups
- `main` — current integration target.
- `backup/accidental-r13-main-20260820-2340` — older backup, no unique commits ahead of current main.
- `backup/main-before-landmark-separation-20260821-0033` — older backup, no unique commits ahead of current main.
- `backup/main-before-r13-recovery-20260820-2253` — older backup, no unique commits ahead of current main.

### R13 / R14 gameplay and model integration
- `r13-content-gameplay-pass` — diverged staging branch. Preserves accepted neutral daylight and old normal-game launcher behavior. Its latest imported weapon/audio/menu payload changes were promoted to main; do not switch the whole game back to this branch.
- `feat/r13-weapon-variants` — behind main; no complete newer game state.
- `feat/r14-production-models` — behind main; production integration is represented in current main.
- `feat/model-assets-integration` — behind main; no complete newer game state.
- `feat/import-hmmwv-btr4-m2` — behind main; HMMWV/BTR-4/M2 integration was merged into main lineage.
- `feat/restore-production-model-packs` — diverged asset-recovery staging. Key production content is present in main; verified AK production asset path matches the restored pack. Do not merge wholesale.
- `feat/restore-world-model-packs` — diverged world-asset recovery staging. Current main contains recovered environment/model runtime systems; preserve branch as recovery source.
- `restore-location-content-packs` — diverged location-content recovery staging; not a complete replacement for main.

### Recovered world systems
- `feat/recovered-environment-runtime` — diverged historical recovery work; current main contains `OCRecoveredEnvironmentSubsystem.cpp`.
- `feat/recovered-foliage-runtime` — diverged historical recovery work; current main contains `OCRecoveredFoliageSubsystem.cpp`.
- `feat/recovered-roadside-props-runtime` — diverged historical recovery work; current main contains `OCRecoveredRoadsidePropsSubsystem.cpp`.
- `fix/r13-runtime-geography-forward-port` — diverged recovery/fix branch; current main carries the integrated geography/runtime direction and should receive selective fixes, not a wholesale rollback.

### Museum
- `museum-oster` — behind current main.
- `codex/oster-museum-photo-site` — behind current main.
- `r13-museum-fidelity-staging` — diverged museum staging with unique historical commits; current main contains newer OCR137–OCR145 museum subsystems. Preserve as reference, do not replace main wholesale.
- `fix/r137-museum-site-cleanup` — diverged cleanup branch; current main contains the corresponding museum runtime validation/site replacement systems.

### Silpo
- `silpo-oster` — diverged Silpo staging; current main contains OCR140–OCR143 Silpo model/detail/interior/facade systems and dedicated geo ownership.
- `codex/oster-silpo-photo-site` — behind current main.

### Stadium
- `stadion-oster` — diverged stadium staging with unique historical commits/reference material. Current main contains the current stadium surface subsystem and hard georeference. Preserve this branch as source/reference; do not merge wholesale because its older shared-map changes can regress later landmark separation.

### Older location slice
- `r12-krushelnytska-visual-slice` — behind current main.

## Current launch contract

- `START_HERE.cmd` option 5: normal current game path. Opens frontend; `START / LOCAL GAME` enters the TEAM gameplay session. This is the canonical visual/gameplay test.
- `START_HERE.cmd` option 9: Sandbox / Test Range diagnostic. This is only for location debugging and must not be treated as the normal-game visual baseline.

## Rule going forward

Do not restore an entire old branch because one screenshot looks better. First identify the exact visual/runtime regression in history, forward-port that fix into `main`, build current source, and keep newer location/model/gameplay integration intact.
