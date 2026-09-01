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
- Latest exact substantive source-verified head: `7c59b7a653c25aea67e44bb57f13ba9b16faefb7`.
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
- Bolt mechanical cue: CONTENT GAP; a real CC0 bolt-action donor is now provenance-pinned but its audio payload is not yet repository-owned/imported.
- Lever mechanical cue: CONTENT GAP; a real CC0 lever-action donor is now provenance-pinned but its audio payload is not yet repository-owned/imported.
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
- Runtime verdict remains **RUNTIME REJECTED 2026-08-31**. No local UE 5.8 run was performed in this cycle.
- PR #94 remains OPEN / UNMERGED; `main` is unchanged.
- Local uncommitted user changes were not touched.
- Item 16 remains unchecked. Official progress remains **22/36 = 61.1%**.

## Work cycle — 2026-09-01 item 16 real manual-action audio provenance continuation

- Start head: `f2d31158a2d2324c2c1d3e4a5576b3cab799cb61`.
- Pre-work live audit confirmed `main@bca00f4046700f383af9f1742cc24b6a62401b1a`, PR #94 OPEN / UNMERGED / mergeable, exact-head CI green, canonical checklist `22/36 = 61.1%`, and no active reservation.
- Item 16 remained the first factual unchecked item, so no later checklist item was used to bypass the content blocker.
- `411383707521639707fb29ee85d4ece6f4174700` — add `PASS45_MANUAL_ACTION_AUDIO_PROVENANCE.md` with two real CC0 mechanical donor contracts:
  - lever-action donor: Freesound `C-V / Lever action cocking.wav`, described by the source as a real .22 lever-action rifle action;
  - bolt-action donor: Freesound `rammbostein / Mosin Nagant Bolt.wav`, described by the source as a real bolt-action rifle action.
- Provenance explicitly limits identity claims: the lever donor is not proof of exact Stein/Marlin/Model-1894 identity and the Mosin donor is not an M700 recording.
- The public CC0 `Free Firearm Sound Library` / `buddingmonkey/FreeFirearmsSFXLibrary` mirror was audited at `beb2f4041f3d6740fa0aeaf0e71159bd65a78c1b`; its prepared sheet is shot-oriented, so weapon/folder names alone are explicitly forbidden as evidence of a mechanical action track.
- Repository `.gitattributes` confirms `*.wav` is Git LFS-controlled. The provenance contract forbids bypassing LFS or treating a URL/source page as committed runtime content.
- `99395228236274b4046b578baa93713c339cb3f8` — add `VERIFY_PASS45_MANUAL_ACTION_AUDIO_PROVENANCE.py`.
- `708f6793b4fdf9759d193f1e004f514f0a2a2c60` — add dedicated `Pass 45 manual-action audio provenance` GitHub workflow.
- `2f2386e0ec2f19ab6d1490c64224d0f8aae4397e` — wire the new guard into `RUN_ALL_VERIFY.py`.
- The first dedicated guard run correctly failed only because three verifier literals did not include the manifest's Markdown emphasis/capitalization; no production regression was present.
- `7c59b7a653c25aea67e44bb57f13ba9b16faefb7` — align those guard literals with the pinned provenance text.
- Exact CI for substantive head `7c59b7a653c25aea67e44bb57f13ba9b16faefb7`:
  - `Pass 45 manual-action audio provenance` run `33478079624` — **SUCCESS**.
  - `Source verification` run `33478079300` — **SUCCESS**, including the cumulative structural/regression suite and the new provenance guard.
  - `Pass 45 weapon audio fallback` run `33478079468` — **SUCCESS**.
  - `Runtime recovery Pass 45` run `33478079386`, `Pass 45 strict runtime acceptance harness` run `33478079598`, `Pass 45 stale runtime retirement` run `33478079161`, `R14 weapon profile contracts` run `33478079482`, `Pass 45 weapon firing and drop physics` run `33478079455`, and the returned related source workflows completed **SUCCESS**.
- Runtime wiring remains deliberately fail-closed: `PumpCycle` still uses tracked `shotguncock`; `BoltCycle` and `LeverCycle` are not assigned merely because candidate source pages exist.
- Actual bolt/lever audio bytes still need compliant repository acquisition/import before runtime wiring; accepted M700/870/Lever moving-part animation sequences also remain absent.
- Runtime verdict remains **RUNTIME REJECTED 2026-08-31**. No local UE 5.8 runtime acceptance was performed.
- PR #94 remains OPEN / UNMERGED; `main` is unchanged.
- Local uncommitted user changes were not touched.
- Item 16 remains unchecked. Official progress remains **22/36 = 61.1%**.

## Next factual work

1. Acquire the pinned CC0 bolt/lever payloads through repository-compliant content/LFS intake, then import and wire them without overstating exact weapon identity.
2. Obtain/commit accepted M700/870/Lever moving-part or skeletal action sequences.
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
