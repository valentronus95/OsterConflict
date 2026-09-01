# PASS45 Runtime Recovery — Persistent Work History

This is the current human-readable checkpoint index for `PASS45_RUNTIME_RECOVERY_TZ.md`.

Earlier detailed work history through the previous checkpoint is preserved verbatim in:

`PASS45_RUNTIME_RECOVERY_HISTORY_ARCHIVE_PRE_2026-09-01.md`

Archived source blob: `1a56e4bad9f342523b7980ac9d2e0691871d1295`.

Git history remains the raw source of truth. This file records the current integration state and the newest substantive work cycle so future PASS45 sessions can continue without replaying completed work.

## Canonical ownership

- Canonical TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`
- Active integration branch: `fix/pass45-runtime-rejection-material-closure-20260826`
- Active integration PR: **#94**
- Target branch: `main`
- `main`: `bca00f4046700f383af9f1742cc24b6a62401b1a`
- Latest factual local verdict: **RUNTIME REJECTED 2026-08-31**.
- Latest exact substantive source-verified head: `5bc61e13b93cd8844167d8f1e667e307d66529a8`.
- Merge rule: PR #94 remains OPEN / UNMERGED until a current-head local UE 5.8 full runtime test passes import, build, gameplay, automated evidence gates and direct screenshot acceptance.
- Official canonical checklist accounting: **22/36 = 61.1%**, **38.9% remaining**.
- Source-only work on runtime-dependent items does not increase that percentage.

## Current open focus

The first canonical unchecked item remains item 16: accepted authored M700/Remington 870/Lever Action moving-part or skeletal manual-action presentation plus factual bolt/pump/lever mechanical audio and UE 5.8 acceptance.

Current factual content state:

- M700 exact manual-action animation: CONTENT GAP.
- Remington 870 exact manual-action animation: CONTENT GAP.
- Lever Action exact manual-action animation: CONTENT GAP.
- Pump mechanical cue: tracked `R13/Audio/shotguncock` is available as source fallback.
- Bolt mechanical cue: CONTENT GAP.
- Lever mechanical cue: CONTENT GAP.
- Isolated Draft PR #95 was audited and does not currently contain accepted M700/Lever action sequences or bolt/lever mechanical content suitable for direct port.

## Work cycle — 2026-09-01 item 16 physical fallback retirement + stale-rule cleanup

- Start head: `408ec28f11f85ef0d1b5df9deae1db43188c0613`.
- Branch / PR: `fix/pass45-runtime-rejection-material-closure-20260826` / #94.
- Production/source commits:
  - `35cf8a537e2ab01b74de0724d262fae33ea091fe` — remove the whole-weapon/arms manual-action transform fallback from first-person runtime.
  - `5c200a1fff17f7e77f81f4da0c5cb2e894c9c2af` — remove obsolete manual-action interpolation state.
  - `e299bebb5f2c9590b5dbead2bb49c990b355b73a` — remove manual-action transform fields from first-person weapon profiles.
  - `e6c0cc7d69e8a7911c49f9a7ee41404446752638` — make M700/870/Lever profiles authored-animation-only.
  - `a40dbc1497d07ef78d488ae02fa0d839a7459faa` — update the regression guard to require physical fallback retirement.
- Governance/docs commits:
  - `5345df61160ba4bfaff2dd3f4710eb37597cbd8a` — synchronize `OSTER_CONFLICT_WORK_LEDGER.md` with the new source truth.
  - `20a8c07cf5f8c7a73450d8bc3bb5cf458f3c2d06` — forward-port `PASS45_RUNTIME_RECOVERY_TZ.md` so it no longer describes the retired procedural fallback as current behavior.
  - `f8a7b8be24725e403c2d5f47dfa9a2d4793bd619` — rotate the oversized history ledger without loss: prior full history is preserved verbatim at `PASS45_RUNTIME_RECOVERY_HISTORY_ARCHIVE_PRE_2026-09-01.md` using the original Git blob.
- Runtime source behavior now observes authoritative replicated `bActionCycling`, routes action audio, and may start only an exact compatible authored `ManualActionAnimationObjectPath` on the production skeletal weapon.
- If authored action content is absent or incompatible, the presentation preserves baseline weapon/arms transforms and emits an explicit content-gap/failure marker. No whole-transform substitute is applied.
- Retired presentation elements include `PASS45_MANUAL_ACTION_PROCEDURAL_FALLBACK_ACTIVE`, `ActionCycleStartTime`, `bAuthoredManualActionActive`, `ManualActionWeaponLocation/Rotation`, `ManualActionArmsLocation/Rotation` and `bManualActionCueDeclared`.
- `VERIFY_PASS45_WEAPON_ACTION_MATRIX.py` now rejects resurrection of those retired fields/state/markers while preserving the authored bridge, exact authoritative timings, required empty M700/870/Lever sequence slots, tracked PumpCycle cue and explicit Bolt/Lever gaps.
- Cumulative verification exposed two stale historical date assertions rather than production regressions:
  - `8b531743e9b4c8f172689503c2c06fba18ee65de` — `VERIFY_PASS45_WEAPON_MUZZLE_DROP_PHYSICS.py` now distinguishes the current factual `RUNTIME REJECTED 2026-08-31` verdict from the 2026-08-27 latest committed rendered screenshot evidence.
  - `5bc61e13b93cd8844167d8f1e667e307d66529a8` — `VERIFY_PASS45_GRENADE_SMOKE_PRIMITIVE_RETIREMENT.py` now uses the same current-verdict semantics instead of forcing the canonical TZ back to 2026-08-27.
- Exact CI for substantive head `5bc61e13b93cd8844167d8f1e667e307d66529a8`:
  - `Source verification` run `33473392252` / job `99747540325` — **SUCCESS**, including the full structural/regression suite.
  - `Pass 45 grenade smoke primitive retirement` run `33473392220` — **SUCCESS**.
  - `Pass 45 weapon firing and drop physics` run `33473392292` — **SUCCESS**.
  - `Runtime recovery Pass 45`, `Pass 45 strict runtime acceptance harness`, `Pass 45 stale runtime retirement`, `R14 weapon profile contracts`, `Pass 45 weapon audio fallback`, `Pass 45 pre-tick sector startup guard`, `Pass 45 tree startup deferred` and the returned related source gates also completed **SUCCESS** or were still non-failing scheduler work at this checkpoint; runtime status is not promoted from any of them.
- Runtime verdict remains **RUNTIME REJECTED 2026-08-31**. No local UE 5.8 run was performed in this cycle.
- PR #94 remains OPEN / UNMERGED; `main` is unchanged.
- Local uncommitted user changes were not touched.
- Item 16 remains unchecked. Official progress remains **22/36 = 61.1%**.

## Next factual work

1. Keep the retired whole-transform manual-action path from returning.
2. Obtain/commit accepted M700/870/Lever action sequences and factual bolt/lever mechanical audio without inventing substitutes.
3. Validate skeleton compatibility, action timing, sound audibility and direct first-person visuals in UE 5.8.
4. Independently, the broader runtime recovery still requires a current-head Quick Normal/full runtime acceptance run before any runtime promotion.

## Status vocabulary

- `SOURCE-CODED`: implementation exists in source.
- `SOURCE-VERIFIED`: relevant source/CI verification passed for the exact indexed head.
- `CONTENT GAP`: required authored content is absent/unaccepted.
- `RUNTIME PENDING`: no current-head local UE 5.8 acceptance exists.
- `RUNTIME REJECTED`: factual local runtime evidence rejected the state.
- `RUNTIME ACCEPTED`: current-head local UE 5.8 import/build/gameplay/evidence/direct visual acceptance passed.
- `MERGED`: exact accepted head was merged to `main` and merge SHA is recorded.
