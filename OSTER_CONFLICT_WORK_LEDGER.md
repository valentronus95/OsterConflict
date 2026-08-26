# OSTER CONFLICT — WORK LEDGER

> Current authoritative work state. Latest explicit user requirement + latest factual local UE runtime/build evidence always override older source/verifier claims.

## 1. Current context — 2026-08-26

- Repository: `valentronus95/OsterConflict`.
- Integrated `main` baseline: `69f0f8005ffc4518fcb413a6202eb3e51c21fd1f`.
- Active corrective branch: `fix/pass45-runtime-rejection-material-closure-20260826`.
- Active PR: **#94 — `Pass45: close Stein R3 fresh-load and strict runtime acceptance gaps`**.
- PR state: **OPEN / UNMERGED**.
- Canonical active TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Source implementation head before this ledger synchronization: `36d07919d62c95b79b9e739d1a12c8b50dcca187`.
- On that source head, GitHub `Source verification` completed **SUCCESS**, including the full structural/regression suite; `R14 weapon profile contracts` also completed **SUCCESS**. Remaining workflow runs for that head were still completing when this ledger was synchronized.
- Latest factual gameplay evidence: `RUNTIME_EVIDENCE/2026-08-26_PASS45_REJECTED/`.
- Historical rejected evidence retained separately: `RUNTIME_EVIDENCE/2026-08-25_PASS45_REJECTED/` and `RUNTIME_EVIDENCE/2026-08-24_PASS44_REJECTED/`.
- UE target: 5.8.x / Windows.
- Canonical user launcher: `START_HERE.cmd` only.
- Hard map reference: `REFERENCE_PHOTOS/map_extent/oster_central_playable_area_20260824.jpg`.
- Current status token: **PASS 45 ACTIVE / RUNTIME REJECTED 2026-08-26 / WEAPON FIRING + MANUAL ACTION + ADS VALIDATION ARCHITECTURE CODED_UNTESTED / LOCAL UE RUNTIME PENDING**.

## 2. Status rules

- `IN_PROGRESS` — implementation/content closure is incomplete.
- `CODED_UNTESTED` — source correction exists but factual local UE build/runtime has not accepted it.
- `CONTENT GAP` — required production content is absent/unverified; never fake READY.
- `AUDIO CONTENT GAP` — routing exists but accepted authored sound content is absent/unverified.
- `RUNTIME REJECTED` — factual local gameplay disproved the result.
- `VERIFIED BUILD` — factual local UBT/UE build succeeds.
- `VERIFIED RUNTIME` — factual local UE/user playtest proves behavior/appearance.
- Green source CI is not UE compile/runtime acceptance.
- Mesh-load success is weaker than authored material/texture truth.
- A historical verifier may never force a rejected owner/behavior back into current source.
- No profile/material/audio slot becomes READY merely because a source field exists.

## 3. Latest authoritative runtime — 2026-08-26

The latest factual local playtest reaches gameplay but **Pass 45 remains rejected**.

Confirmed improvements that must not regress:

- gameplay launches;
- the previous near-black world corruption is no longer the dominant rendered state;
- multiple firearm production meshes/materials render recognizably;
- HMMWV visual forward direction is improved enough to drive forward normally;
- recovery cap is approximately 60 FPS in the captured run;
- production HMMWV/BTR/M2 intake reaches gameplay.

Remaining factual runtime failures/gaps include:

- weapon firing/recoil/audio/muzzle presentation still requires current-head proof;
- some weapon audio/content remains absent or prototype-grade;
- dropped-weapon physics requires current-head proof;
- launcher visible production presentation remains unaccepted;
- Museum/Culture House/Silpo identity/separation remains visually rejected/unaccepted;
- rejected vegetation/visual-fidelity families remain open;
- M2 station hierarchy/camera remains open;
- HMMWV >=80 km/h road target remains open;
- BTR white/default material persistence, forward axis and remote-operator view remain open;
- final fullscreen/thermal/tactical-map/direct screenshot gates remain open.

**Current factual verdict: RUNTIME REJECTED 2026-08-26.**

## 4. Confirmed corrective source work on active branch

Everything below is still **CODED_UNTESTED** unless explicitly marked as source-CI evidence only.

### 4.1 Stale runtime owner retirement

Physically retired rejected/destructive/inert owners include the old world-production visual owner, late Museum recovery/visibility rebuild owners, duplicate landmark cleanup owners, retired palette compatibility owner and the stale completion audit that attempted to require rejected behavior back.

Guardrails:

- one runtime responsibility has one mutating owner;
- validation-only systems may observe and fail, not repair primary authoring late;
- `VERIFY_PASS45_STALE_RUNTIME_RETIREMENT.py` protects retired ownership;
- historical verifiers are forward-ported rather than used to resurrect old READY markers.

### 4.2 Museum / landmark ownership

Current source contract retains:

- one coordinated startup sequence;
- R13.7 as the single visible Museum exterior owner;
- R13.8 as hidden collision/interactivity/final glass rather than a competing visible shell;
- validation-only layer/separation checks;
- no late `Destroy`/`RemoveInstance` cleanup used to disguise a primary landmark-authoring error.

Runtime Museum/Culture/Silpo visual identity still requires factual screenshots.

### 4.3 Vehicle BASE teleport correction

Current source validates only initial character deployment near Museum BASE. Ordinary `character -> vehicle -> character` possession transitions are not treated as fresh BASE deployment.

Markers retain `vehicle_revalidation=0`; civilian/HMMWV/BTR enter-drive-exit still require local runtime proof with `museum_respawn_path=0`.

### 4.4 HMMWV / BTR / M2 transforms and material ownership

- HMMWV/BTR production visual fitting uses proportional/uniform scaling rather than independent XYZ deformation.
- M2 uses grounded/bounds-based mount alignment.
- default M2 gunner vertical direction was corrected at source.
- production vehicle authored materials are no longer intentionally overwritten by legacy base tint.
- BTR production material intake is revisioned; stale `.uasset` presence is not proof of current authored-material truth.

Runtime still owns acceptance for proportions, white/default material absence, forward axis and turret/camera behavior.

### 4.5 Display / thermal recovery

- normal route no longer forces `-windowed`;
- recovery route targets fullscreen;
- normal recovery cap remains approximately 60 FPS;
- native render scale is not intentionally lowered to fake performance.

### 4.6 Strict runtime acceptance route

Canonical full-test route remains:

`START_HERE -> 2. ПОВНИЙ RUNTIME-ТЕСТ -> RUN_R14_MAIN_RUNTIME_ACCEPTANCE.cmd -> RUN_R14_PLAYFLOW_PERFORMANCE_ACCEPTANCE.cmd -> RUN_R14_CURRENT_GAMEPLAY.cmd`

`RUN_R14_CURRENT_GAMEPLAY.cmd` is the single actual gameplay-process owner. Automated PASS still leaves visual acceptance pending; direct screenshots remain mandatory.

### 4.7 Reference-driven residential retirement

- normal runtime no longer intentionally spawns the generic `AOCEnterableHouse` family;
- generic `BuildResidentialBlocks()`/Krushelnytska house-fence filler was retired while reference-driven POI geometry remains;
- `VERIFY_PASS45_REFERENCE_DRIVEN_RESIDENTIAL_RETIREMENT.py` guards against resurrection.

The rejected dark tower/shack and generic-family absence still require factual runtime proof.

### 4.8 Weapon material/import closure

- Stein weapon intake uses R3 deterministic authored material generation from committed PNG sources;
- a separate fresh UE process validates material/texture dependencies;
- negative UnrealEditor commandlet return codes cannot escape the launcher as a false PASS;
- required weapons use **exact production OR explicit real-mesh fallback**;
- white/default/BasicShape material, zero used textures or missing local authored dependencies are hard failures;
- exact Remington 870 / M249 / M16-M4 payload gaps remain explicit `CONTENT GAP` where applicable;
- production HMMWV/M2/BTR import remains revisioned/fresh-load validated.

Rendered local UE appearance remains authoritative.

### 4.9 Weapon firing / recoil / drop continuation — 2026-08-26

Source correction now includes:

- view-ray hit authority separated from production-muzzle presentation;
- muzzle flash/tracer/shot audio originate from the visible production weapon rather than camera `TraceOrigin`;
- launcher projectile/FX/audio use production muzzle and failed projectile spawn does not consume ammo;
- bounded server cadence tolerance for timer jitter;
- legacy Character `LocalFireFeedbackTimerHandle`, duplicate held-input recoil path and old local recovery owner physically retired;
- confirmed camera recoil/crosshair/camera-shake ownership tied to server-accepted shot presentation;
- deliberate dropped weapons use authority rigid-body collision/gravity/replicated movement while authored rack pickups are not globally forced into simulation.

Guard: `VERIFY_PASS45_WEAPON_MUZZLE_DROP_PHYSICS.py`.

### 4.10 Weapon action / Burst3 / manual-action continuation — 2026-08-26

Source model now separates selector position from mechanical action:

- supported fire modes are data-driven per exact weapon;
- finite authoritative `Burst3` sequencing exists as an opt-in architecture, while no current production weapon falsely enables it;
- `FOCWeaponTuning::ManualActionCycleSeconds` is explicit;
- M700 bolt = `1.10 s`, Remington 870 pump = `0.72 s`, Lever Action .45-70 = `0.85 s`;
- replicated `bActionCycling` blocks another shot/reload/selector mutation until authoritative completion;
- first-person procedural bolt/pump/lever cues consume the replicated gate and authoritative duration; they own **no second gameplay timer**;
- marker: `PASS45_MANUAL_ACTION_PRESENTATION_READY ... replicated_gate=1 second_gameplay_timer=0`;
- manual mechanical audio routes by exact action through `BoltCycle`, `PumpCycle`, `LeverCycle`;
- local and remote action-audio paths are separated to avoid intentional double playback;
- empty sound sets remain explicit `AUDIO CONTENT GAP`.

Still pending: authored/exact moving-part presentation where assets support it, real accepted mechanical sound content and local UE timing/feel verification.

Guard: `VERIFY_PASS45_WEAPON_ACTION_MATRIX.py`.

### 4.11 Per-weapon ADS validation architecture — 2026-08-26

Source now refuses to treat one generic ADS transform as factual calibration:

- every current weapon id resolves through an explicit first-person profile;
- profiles expose `ADSRearSightSocket`, `ADSFrontSightSocket`, `ADSOpticSocket` and separate `bADSCalibrated` truth;
- all current weapon profiles intentionally remain `bADSCalibrated=false` until exact UE 5.8 sight evidence exists;
- entering ADS invokes fail-visible alignment diagnostics;
- uncalibrated profile marker: `PASS45_ADS_PROFILE_UNCALIBRATED ... no_fake_ready=1`;
- calibrated-but-invalid production visual/socket emits `PASS45_ADS_ALIGNMENT_FAIL`;
- valid calibrated profile can emit `PASS45_ADS_ALIGNMENT_SAMPLE` with camera-vs-sight angular error and camera-line offset;
- `oc.Weapon.ADS.Debug 1` draws camera and authored sight-axis rays for calibration evidence;
- diagnostics own no gameplay timer or ballistic mutation;
- `VERIFY_PASS45_WEAPON_ADS_ALIGNMENT.py` is included in cumulative `RUN_ALL_VERIFY.py`.

Source implementation head `36d07919d62c95b79b9e739d1a12c8b50dcca187` passed GitHub `Source verification` and `R14 weapon profile contracts`. This is source-contract evidence only. Exact per-weapon sight sockets/offsets and local UE visual approval remain pending.

## 5. Active requirements

| ID | Requirement | Status | Current action |
|---|---|---|---|
| STALE-OWNER-001 | Old code/verifiers must not overwrite or resurrect newer runtime behavior | CODED_UNTESTED | Physical retirement + validation guards retained. |
| PERF-COLLAPSE-001 | No severe FPS/thermal collapse | CODED_UNTESTED | ~60 FPS recovery cap; final mixed soak pending. |
| VIS-BLACK-WORLD-001 | No black ground/world corruption | CODED_UNTESTED | Latest run improved; current-head screenshot proof still required. |
| LOC-MUSEUM-001 | Correct visible Oster Local History Museum | CODED_UNTESTED | Single exterior owner; runtime identity proof pending. |
| LOC-CULTURE-001 | Culture House separate from Museum | IN_PROGRESS | Separate anchor/validation exists; visible proof pending. |
| LOC-SILPO-001 | Silpo one correct site owner | CODED_UNTESTED | Startup ownership retained; visible proof pending. |
| VIS-GENERIC-RESIDENTIAL-001 | No rejected generic house/fence/tower family | CODED_UNTESTED | Source owners retired; runtime absence proof pending. |
| GAME-VEHICLE-TELEPORT-001 | Vehicle enter/drive/exit never returns to Museum | CODED_UNTESTED | Initial-only BASE logic; local civilian/HMMWV/BTR test pending. |
| VEH-HMMWV-001 | Correct HMMWV proportions/orientation and >=80 km/h road target | IN_PROGRESS | Proportional fit source-coded; speed/calibration runtime pending. |
| VEH-BTR-001 | BTR proportions/material/forward/remote operator correct | IN_PROGRESS | Material intake + proportional source path exists; runtime white/axis/operator gaps open. |
| VEH-M2-001 | Coherent M2 mount, 360° yaw, correct camera/pitch | IN_PROGRESS | Pitch/mount source work partial; final hierarchy/runtime pending. |
| WEAPON-MATERIAL-001 | Required available weapons use authored material + texture deps | CODED_UNTESTED | R3/fresh-load path coded; rendered rack proof pending. |
| WEAPON-FIRING-001 | Ammo/recoil/muzzle/audio count agree on factual shots | CODED_UNTESTED | Confirmed-shot ownership source-coded; local firing matrix pending. |
| WEAPON-DROP-001 | Deliberate weapon drops fall/collide/settle and replicate | CODED_UNTESTED | Rigid-body source path coded; local physics proof pending. |
| WEAPON-ACTION-001 | Exact selectors + deterministic Burst3/manual cycles | CODED_UNTESTED | Finite Burst3 + replicated manual gate + procedural cue source-coded. |
| WEAPON-AUDIO-001 | No accepted silent weapon; exact mechanical action audio | IN_PROGRESS / CONTENT GAP | Routing exists; authored sound population/audit pending. |
| WEAPON-ADS-001 | Per-weapon sight alignment is factual, not generic | IN_PROGRESS / CODED_UNTESTED | Fail-visible socket architecture + diagnostics coded; all exact profiles remain uncalibrated. |
| UI-TACTICAL-MAP-001 | `M` matches compact central-Oster topology | CODED_UNTESTED | Runtime screenshot required. |
| VIS-TREES-001 | No primitive/fantasy visible tree family | IN_PROGRESS / CONTENT GAP | Replacement/reference work still required. |
| ASSET-M16-M4-001 | M16/M4 production visuals | CONTENT GAP | No verified exact payload; do not claim connected. |

## 6. Behavior that must not return

1. runtime-rejected world-production visual owner or equivalent renamed behavior;
2. late Museum recovery/rebuild/polling ownership;
3. duplicate landmark destroy/repair owners;
4. retired weapon palette mutation/compatibility owner;
5. verifier/workflow requiring a retired owner/READY marker;
6. ordinary vehicle possession treated as BASE deployment;
7. non-uniform XYZ fitting of production HMMWV/BTR;
8. forced normal `-windowed` launch;
9. uncapped normal recovery route;
10. implicit normal-game bot autofill;
11. historical 2.4 km gameplay/tactical map;
12. grey/BasicShape material repair presented as production readiness;
13. late scene mutation used to hide primary authoring errors;
14. generic residential/fence filler resurrected as Oster-authentic content;
15. direct `START_HERE -> PLAYFLOW` full-test bypass of strict acceptance;
16. duplicate strict production-vehicle import owners;
17. held-input recoil timer resurrected beside confirmed-shot recoil;
18. low RPM used as a substitute for bolt/pump/lever action state;
19. a second first-person timer owning manual-action gameplay state;
20. `bADSCalibrated=true` without exact sight reference + UE evidence;
21. generic ADS/FOV-only presentation described as factual sight alignment.

## 7. Current execution order

1. [x] Preserve latest runtime evidence and mark **RUNTIME REJECTED 2026-08-26**.
2. [x] Retain stale-owner retirement and one-owner architecture.
3. [x] Retain daylight/material stability and initial-character-only BASE recovery.
4. [x] Retain proportional HMMWV/BTR and material-owner protections.
5. [x] Stein R3 authored-material + independent fresh-load source path.
6. [x] Source-code factual-shot muzzle/recoil/drop corrections and retire duplicate local recoil owner.
7. [x] Source-code data-driven selector/action model and finite Burst3 architecture.
8. [x] Source-code authoritative M700/870/Lever post-shot cycle gate.
9. [x] Source-code replicated-gate procedural manual-action presentation + exact Bolt/Pump/Lever audio routing.
10. [x] Source-code fail-visible per-weapon ADS profile/sight-reference architecture + alignment diagnostics.
11. [x] Add manual-action and ADS guards to cumulative source verification.
12. [ ] Replace procedural manual-action cue with accepted authored moving-part/skeletal presentation where assets support it.
13. [ ] Populate/accept real bolt/pump/lever mechanical sound content and close general silent-weapon audio gaps.
14. [ ] Inspect each exact production weapon in local UE 5.8, author/confirm rear/front/optic references and calibrate ADS transforms; only factual profiles may set `bADSCalibrated=true`.
15. [ ] Remove visible primitive weapon/pickup/launcher fallbacks from accepted runtime.
16. [ ] Replace grenade models/throw/smoke presentation.
17. [ ] Correct Museum/Culture House/Silpo visible identity/separation.
18. [ ] Replace rejected vegetation family and raise world/material/LOD fidelity.
19. [ ] Complete HMMWV M2 hierarchy/360°/camera and >=80 km/h road tuning.
20. [ ] Close BTR white material/forward-axis/remote-operator gaps.
21. [ ] Validate fullscreen, ~60 FPS, native render scale and 10-minute mixed thermal soak.
22. [ ] Validate tactical map screenshot.
23. [ ] Current-head `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` import + build + gameplay + automated gates + direct screenshots.
24. [ ] Merge PR #94 only after factual current-head runtime acceptance.

## 8. Next factual runtime gates

### Build / import

- current branch/head matches origin;
- Stein R3 authored import + independent fresh load succeeds;
- production HMMWV/M2/BTR import/fresh load succeeds;
- UBT/UE 5.8 build exits 0;
- no required available weapon reports placeholder material or zero authored texture dependency.

### Weapons

- 10-shot semi count agreement;
- full-auto hold has no ghost recoil after authoritative firing stops;
- release has no artificial downward kick;
- empty-mag hold creates no shot recoil/audio;
- muzzle/tracer/projectile begins at visible production muzzle;
- drop from standing/walking/running falls, collides and settles;
- M700/870/Lever action cycle visually follows authoritative state;
- action audio is actually populated/audible or remains explicit content gap;
- every accepted weapon receives dedicated hip/ADS screenshots;
- `oc.Weapon.ADS.Debug 1` evidence is used when calibrating sight axes;
- no uncalibrated ADS profile is described as READY.

### World / landmarks

- no black-world corruption;
- Museum visually reads as Museum, not Culture House;
- Culture House and Silpo visibly separate;
- rejected generic house/tower/tree families absent;
- no major white/default/proxy core visuals.

### Vehicles

- civilian/HMMWV/BTR enter-drive-exit never returns to Museum;
- HMMWV forward axis and >=80 km/h target accepted without broken handling;
- M2 ring/shield/weapon hierarchy coherent with 360° yaw and correct gunner view;
- BTR has no white/default region before/after possession, drives forward correctly and uses remote operator optic/monitor logic.

### Performance / display / map

- intended fullscreen/borderless;
- ~60 FPS recovery cap, native render scale retained;
- >=30 FPS sampled floor;
- 10-minute mixed soak without severe progressive collapse;
- tactical map remains compact, north-up and correctly separates Museum/Culture/Silpo/Stadium.

## 9. Historical truth retained

- Pass44 2026-08-24 remains historically **RUNTIME REJECTED**.
- Pass45 2026-08-25 local build/import rejection remains factual chronology even though later corrective builds reached gameplay.
- Earlier 2026-08-25 runtime rejection remains preserved separately.
- None of those historical failures may be erased by later source fixes, and none of their stale workaround owners gain authority merely because they once existed.

## 10. Current verdict

**PASS 45 = ACTIVE / RUNTIME REJECTED 2026-08-26.**

PR #94 remains **OPEN / UNMERGED**.

Source verification success on implementation head `36d07919d62c95b79b9e739d1a12c8b50dcca187` proves structural/source-contract consistency only. Manual-action presentation/audio routing and ADS validation architecture are **CODED_UNTESTED**; exact authored action content, per-weapon ADS calibration and all factual local UE 5.8 acceptance remain open.
