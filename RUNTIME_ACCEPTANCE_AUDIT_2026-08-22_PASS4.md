# Oster Conflict — Runtime acceptance audit, pass 4

Date: 2026-08-22
Status: CODED_UNTESTED until local UE 5.8 build/playtest.

## Authoritative runtime evidence

Latest user screenshots still show four rejected results:

1. Deployment START visibly stalls before gameplay.
2. Actual player spawn is still observed in the empty test field instead of beside the Museum test hub.
3. M1911 / multiple rack weapons and BTR still appear as primitive or proxy geometry in the observed runtime.
4. Visible shot/tracer presentation originates below or away from the actual barrel.

These screenshots outrank earlier code-only claims.

## Source audit findings

### START stall

`OCDenseGroundFoliageSubsystem` previously populated the entire Oster coverage synchronously around gameplay transition. Current source now batches 96 cells per timer tick, so the largest identified deployment-frame vegetation workload is no longer one blocking operation. `OCDeploymentLoadingSubsystem` remains wired through `UICommitDeployment()` and provides the 0–100 transition overlay. This is still runtime-unverified.

### Spawn

Current `OCTeamSpawnPoint` BASE placement is already tied to `AOCWorldSectorOster::MuseumAnchor()`. Primary BASE is approximately 17 m from the Museum anchor, ground-snapped, and the 11-class rack follows the primary BASE. The latest screenshot predates acceptance of this exact source result, therefore status remains CODED_UNTESTED rather than VERIFIED.

### Weapon models

The repository does contain the restored AK/R13 weapon families under Git LFS, including skeletal M1911, MP5, M700, M14, MAC-10, TEC-9 and Lever Action assets. The normal launcher already hydrates their LFS payloads, but previously did not prove that Unreal could actually open the required assets before gameplay.

Pass 4 adds a fresh-process weapon asset gate. Normal gameplay is blocked unless Unreal opens the required AK/R13 visuals plus real R13 machine-gun, shotgun and launcher fallbacks. This prevents another normal test from silently continuing with Engine/BasicShapes weapon boxes when an LFS/asset load fails.

Exact M249 and Remington 870 production identity remains a separate content gap. A real R13 fallback is acceptable only as a fallback and must not be labelled exact production content.

### HMMWV / M2 / BTR

The canonical vehicle source files remain local-only. `prepare_local_production_sources.ps1` recovers the user's existing HMMWV/M2/BTR downloads from Downloads/Desktop or known ZIPs, the importer creates the canonical `/Game/Production/...` assets, and a second fresh Unreal process must reopen all three before normal gameplay starts. Runtime size/material/ground-contact acceptance remains open.

### Muzzle / tracer

The concrete tracer bug was identified in source. The multicast creates a short streak near the hit target. The old visual resolver only rebased starts located within 90 cm of the first-person camera, so the target-side tracer start failed that test and never moved to the muzzle.

Pass 4 accepts a target-side point only when it lies on the local player's aim ray, then resolves the visible weapon muzzle from authored muzzle/barrel sockets first and visible barrel/muzzle geometry second. A rebased local streak is capped to 9 m so it starts from the barrel without becoming a full laser beam.

## CI finding

The previous full Source verification failure was also audited. Stadion verification and the earlier structural suites passed. The failure came from `VERIFY_R10_CXX_BATCH_FIX.py` globally forbidding a local variable spelling (`if (UVerticalBoxSlot* Slot`) even in unrelated namespace helpers. The real `OCGameUIRootWidget.cpp` shadow check already exists separately. Pass 4 removes only that cross-file false positive and retains the actual file-specific C4458 guard.

## Acceptance gate before VERIFIED RUNTIME

The next local UE 5.8 run must prove all of the following in the same current-main build:

- deployment START shows the loading transition and does not stall/jump;
- player actually appears beside Museum and the weapon rack is beside the same spawn;
- M1911 and the other required weapon visuals are real meshes, not boxes;
- BTR/HMMWV/M2 are canonical imported visuals, not proxy geometry;
- muzzle flash and the local tracer begin at the visible barrel end;
- minimap, reverse steering, gunner entry and grass regressions are checked at the same time.

Until that run, every pass-4 item remains CODED_UNTESTED.
