# OSTER CONFLICT — RUNTIME ACCEPTANCE PASS 6

Date: 2026-08-22  
Branch: `fix/runtime-acceptance-pass-6-20260822`  
Baseline: `main` at `539dcf420940097ca56e28058e2d548595dab6ca`  
Overall status: **IN_PROGRESS / CODED_UNTESTED**  
Merge status: **DO NOT MERGE TO `main` UNTIL SOURCE CHECKS + UE RUNTIME ACCEPTANCE ARE GREEN**

## 1. Acceptance scope

This pass is intentionally one runtime acceptance pass rather than a collection of disconnected cosmetic patches.

Required runtime outcomes:

1. Actual spawn/respawn beside the canonical Oster Museum hub, not the old map-edge `BASE`.
2. All 11 weapon pickup classes beside the actual museum spawn.
3. Production weapon models correctly oriented; imported materials preserved; no white primitive stand-ins.
4. Visible muzzle/tracer FX starts at the actual firing weapon barrel/muzzle. Camera trace authority may remain camera-based.
5. Compact always-on minimap and compact chat; `M` remains full tactical map; `Y` team and `U` global chat.
6. Production BTR-4E/Bucephalus and pickup/HMMWV with mounted M2; obsolete proxy geometry must not render or collide.
7. No invisible collision left behind by hidden visual proxies.
8. Denser real grass without returning to the old synchronous deployment stall.
9. No black/grey/legacy-menu blink during frontend → gameplay travel.

## 2. Confirmed root causes

### 2.1 Old map-edge BASE has a separate world owner

`AOCWorldSectorOster` still constructs `BuildGameplayBases()` around the legacy centers approximately `(-104000,-92000)` and `(104000,92000)`. Moving `AOCTeamSpawnPoint` therefore cannot remove the old world geometry by itself.

Pass 6 adds one explicit runtime correction owner, `UOCRuntimeAcceptancePass6Subsystem`, which removes only instances belonging to the known world-sector component families inside the two obsolete base zones. This is a compatibility cleanup for the current generated sector and must still be visually accepted in UE runtime.

### 2.2 R13 weapon presentation was skeletal-centric

Runtime validation already establishes that most restored R13 weapon production visuals are `StaticMesh` assets. `UOCFirstPersonWeaponPresentationSubsystem` previously discovered `OC_ProductionWeaponVisual` only through `USkeletalMeshComponent`, so the actual static production models were outside part of the presentation path.

Pass 6 now discovers production visuals through `UPrimitiveComponent`, preserving a skeletal-only branch only for animation sequences that genuinely require a compatible skeleton. Production static weapons are normalized once from their longest authored axis to gameplay `+X` and valid imported materials are retained.

### 2.3 Visible shot FX was not bound robustly to the actual firing weapon

The authoritative hit trace may start from the camera, while old visible FX rebasing could select the local first pawn/weapon and could fail for target-side tracer segments or remote players.

Pass 6 resolves the firing character/`CurrentWeapon` from camera proximity or the shot aim ray, then resolves a production visual muzzle socket or a production-component bounds fallback. Muzzle and tracer visuals use that weapon-local origin while gameplay trace authority remains unchanged.

### 2.4 Frontend travel used a persistent viewport render flag

`UOCR13UIViewportStabilizerSubsystem` could toggle `UGameViewportClient::bDisableWorldRendering` and restore legacy widgets during the short pawn-less async travel gap. Since the viewport client survives world travel, this is an unsafe owner for transient frontend visibility.

Pass 6 keeps world rendering enabled and holds the existing opaque R13 menu blocker/background through the pawn-less startup shell until the gameplay world owns the transition.

## 3. Code changes in this pass

| Area | Change | Status |
|---|---|---|
| Minimap | Normal-play map reduced to 184/172 px footprint, smaller marker | CODED_UNTESTED |
| Chat | Panel reduced to 360×190, last 5 messages, `Y/U` retained | CODED_UNTESTED |
| Grass | 900 cm grid, 4–6 grass clumps/cell, batched population retained | CODED_UNTESTED |
| Menu travel | Removed active persistent viewport-render suppression; pawn-less shell isolation retained | CODED_UNTESTED |
| BTR-4E | Production shell retained; obsolete visual proxy components fully inert: hidden + no collision/overlap/nav/shadow | CODED_UNTESTED |
| Pickup/HMMWV | Production shell proxies fully inert; M2/fallback long axis normalized; old primitive turret/barrel disabled | CODED_UNTESTED |
| M2 muzzle | Production M2 165 cm visual uses front-bound muzzle placement (82.5 cm from centered pivot) | CODED_UNTESTED |
| Legacy BASE | Explicit pass-6 subsystem removes instances around the two obsolete generated base centers | CODED_UNTESTED |
| Static weapons | Production StaticMesh visuals normalized once to X-forward, valid imported materials preserved | CODED_UNTESTED |
| FP presentation | Production visual lookup generalized from skeletal-only to primitive/static-or-skeletal | CODED_UNTESTED |
| Muzzle/tracer | Actual firing `CurrentWeapon` resolved; production socket/bounds used for visible muzzle/tracer origin | CODED_UNTESTED |
| Source CI | Added `VERIFY_RUNTIME_ACCEPTANCE_PASS_6.py` + dedicated workflow | SOURCE CHECK PENDING |

## 4. Commits recorded during pass 6

- `e3dbdd45b4d6ce305d001f53b93cbf938bee83c7` — compact minimap
- `13771c57b89339dbf3900d9b136316c1f3245dd4` — compact chat
- `6169de08f64bd760a09aa06dd80469a8f570cd85` — denser batched foliage
- `aad366adb5804810873bae1cdb7be87ef3758507` — frontend travel isolation
- `26a80bac28d8b79469792aaa97afe1eff93ac679` — BTR production proxy cleanup
- `4bfa7d91ca86d337537f857ac38a8ff08d2322a9` — pickup/M2 production presentation
- `796a5c4a6df57ce0005e983364c508854bea281c` — pass-6 subsystem header
- `a8debc39ae64a85734d834a42f4d41660577bc9b` / `3a4ca2dcd06e81f9ba87319cd17f0011b00c3607` — legacy BASE + static weapon normalization
- `cae24185985c8efeac89528c22b714b5a17c3420` — firing-weapon muzzle/tracer binding
- `550310554f394478589809ab90c1435f50d398ab` — mesh-agnostic FP header
- `87a02b0014d14d7649203e51a243eb226844e7b4` / `869014e9b2fa024964dad0613a270d3b6944d10b` — StaticMesh-aware FP implementation + immediate field-name correction
- `7ed31962d65a73d43c1f35f50f5716828b92e00e` — pass-6 source verifier
- `231b23b047e4ebc223d457a4c6a556a7aec5b50b` — pass-6 workflow

## 5. Existing source facts preserved

`AOCTeamSpawnPoint` already uses `AOCWorldSectorOster::MuseumAnchor()` and its primary base rack enumerates exactly 11 gameplay classes:

1. Assault Rifle
2. SMG
3. Pistol
4. Sniper
5. Shotgun
6. LMG
7. M14
8. MAC-10
9. TEC-9
10. Lever Action
11. Anti-Armor Launcher

Pass 6 does not replace this rack with another owner. It removes the competing legacy world-base presentation and corrects the production visual paths around it.

## 6. Source-contract gate

`VERIFY_RUNTIME_ACCEPTANCE_PASS_6.py` checks only structural source guarantees:

- museum spawn + complete 11-class rack;
- explicit legacy-base cleanup owner;
- StaticMesh-aware first-person production visual lookup;
- actual firing-weapon muzzle/tracer rebasing;
- compact map/chat dimensions and hotkeys;
- production BTR/M2 contracts and inert proxy collision;
- denser no-collision foliage;
- frontend travel isolation without the old active viewport-render toggle.

A green source-contract run must **not** be relabeled `VERIFIED RUNTIME`.

## 7. Required UE runtime acceptance before merge

The branch remains **IN_PROGRESS** until an actual UE 5.8 launch/playtest proves all of the following in one coherent build:

- Deployment and respawn visibly land at the museum hub.
- The old blocky edge `BASE` is absent.
- All 11 rack weapons are present and pickable near spawn.
- Each visible production weapon is upright/forward in pickup and first-person presentation; no white primitive replacement is visible.
- Imported weapon materials/textures render correctly. Any missing texture/content is treated as an asset gap, not hidden by code color.
- Muzzle flash originates at the barrel end for local and observed remote shots.
- Tracer is a short thin streak, not a yellow round projectile or full laser.
- Minimap/chat are compact and do not cover normal gameplay.
- BTR-4E and pickup/HMMWV/M2 use production shells with no proxy visuals or invisible proxy collision.
- Grass is visibly denser while deployment remains responsive.
- Repeated frontend → START → gameplay transitions show no black/grey/legacy-menu blink.

Until that evidence exists, every changed runtime item in this document remains **CODED_UNTESTED** and the overall pass remains **IN_PROGRESS**.
