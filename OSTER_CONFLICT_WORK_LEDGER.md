# OSTER CONFLICT — WORK LEDGER

> Current authoritative work state. Latest explicit user requirement + latest factual local UE runtime/build evidence always override older source/verifier claims.

## 1. Current context — 2026-08-31

- Repository: `valentronus95/OsterConflict`.
- Integrated `main` baseline: `bca00f4046700f383af9f1742cc24b6a62401b1a`.
- Active corrective branch: `fix/pass45-runtime-rejection-material-closure-20260826`.
- Active PR: **#94 — `Pass45: runtime recovery, reference-backed world fidelity and strict acceptance`**.
- PR state: **OPEN / UNMERGED**.
- Canonical active TZ: `PASS45_RUNTIME_RECOVERY_TZ.md`.
- Latest substantive source head: `6cc2158d31f10ee50a1c5d3ca096b86b4b49022b` (`fix(pass45): distinguish pretick lifecycle from preview bootstrap`).
- Exact source CI for that substantive head completed **SUCCESS**, including full `Source verification`, pre-tick startup guard, Block0 ground guard, tree-startup deferral, runtime recovery and strict runtime-acceptance source contracts.
- Latest branch bookkeeping head before this ledger sync: `37c4775b1cc590bce79c647ed2037270ff32cc56` (`docs(pass45): record pretick startup recovery`).
- Latest factual local UE evidence is the 2026-08-31 Quick Normal startup rejection: incremental C++ build succeeded, then the direct game stayed black and entered rejected KiteDemo tree/material/static-mesh work.
- Latest rendered gameplay evidence pack remains `RUNTIME_EVIDENCE/2026-08-27_PASS45_REJECTED/`.
- Historical rejected evidence remains separate: `RUNTIME_EVIDENCE/2026-08-25_PASS45_REJECTED/` and `RUNTIME_EVIDENCE/2026-08-24_PASS44_REJECTED/`.
- UE target: 5.8.x / Windows.
- Canonical user launcher: `START_HERE.cmd` only.
- Hard map reference: `REFERENCE_PHOTOS/map_extent/oster_central_playable_area_20260824.jpg`.
- Current status token: **PASS 45 ACTIVE / RUNTIME REJECTED 2026-08-31 / PRE-TICK + SECOND START TREE-LOAD SOURCE RECOVERED / EXACT SUBSTANTIVE SOURCE HEAD CI SUCCESS / LOCAL UE RE-RUN PENDING**.

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

## 3. Latest authoritative runtime — 2026-08-31

The latest factual local attempt is a **startup rejection** newer than the 2026-08-27 rendered gameplay pack.

What factually happened:

- `START_HERE.cmd -> 1. ЗВИЧАЙНА ГРА` completed the incremental C++ build successfully;
- the direct game process opened a black window and did not reach usable gameplay/UI;
- the log reached the render/daylight readiness markers, then entered KiteDemo material/static-mesh work;
- the blocking path included `HillTree_02` build/wait work and UE 5.8 material diagnostics;
- therefore the latest factual verdict is still rejection even though earlier runs had reached gameplay.

The current source branch now contains a source-level recovery for both known startup causes: native world-sector tree loads are deferred/quarantined, the second Stadion Oster START-time tree load path is quarantined, and Block0 gets a canonical Oster sector before world-subsystem BeginPlay. None of this is runtime acceptance until a new local UE 5.8 launch proves it.

The earlier 2026-08-27 rendered improvements/failures remain authoritative for visual acceptance once startup is recovered. They include recognizable authored weapon meshes, improved HMMWV forward behavior and a ~60 FPS cap, but still reject/unaccept AK/hand ADS, M2, BTR, landmark/world visual fidelity, vegetation/material quality and final runtime gates.

**Current factual verdict: RUNTIME REJECTED 2026-08-31.**

### Pass 44 historical runtime rejection (retained fact)

**Pass 44 verdict: RUNTIME REJECTED.** The 2026-08-24 factual runtime disproved Pass44 as a complete solution: spawn/result framing was wrong, the map was still perceived as excessively large/empty, weapon visuals/materials were not production-ready, production-model claims were unreliable, and FPS could collapse severely. Pass45 supersedes Pass44 as the active corrective pass; this historical rejection may not be erased by later source fixes.

### Pass 44 behavior retained unless disproved

The following Pass44 decisions remain protected as non-regression because later evidence did not invalidate them:

- compact central-Oster hard extent: approximately 960×940 m, never restore the historical 2.4 km battlefield;
- normal local gameplay defaults to zero implicit filler bots unless explicitly requested;
- Museum BASE acceptance must be based on the actual live pawn, not source-only spawnpoint existence;
- tactical-map bounds follow the compact central-Oster reference rather than legacy peripheral component auto-fit;
- grey/BasicShape weapon material repair is forbidden; authored material gaps remain fail-visible;
- the retired Pass37 weapon palette compatibility owner stays physically deleted, not preserved as an inert shell.

Pass44 historical non-regression does **not** authorize resurrection of any owner/repair path that Pass45 physically retired.

## 4. Confirmed corrective source work on active branch

Everything below is still **CODED_UNTESTED** unless explicitly marked as source-CI evidence only.

### 4.1 Stale runtime owner retirement

Physically retired rejected/destructive/inert owners include the old world-production visual owner, late Museum recovery/visibility rebuild owners, duplicate landmark cleanup owners, retired palette compatibility owner and the stale completion audit that attempted to require rejected behavior back.

Guardrails:

- one runtime responsibility has one mutating owner;
- validation-only systems may observe and fail, not repair primary authoring late;
- `VERIFY_PASS45_STALE_RUNTIME_RETIREMENT.py` protects retired ownership;
- historical verifiers are forward-ported rather than used to resurrect old READY markers.

### 4.2 Museum / Culture House / Silpo identity ownership

Current source contract now includes:

- one coordinated startup sequence;
- R13.7 as the single visible Museum exterior owner;
- R13.8 as hidden collision/interactivity/final glass rather than a competing visible shell;
- R13.7 Museum source has no six-column civic signature;
- R14.6 Culture House is the sole six-column civic owner at its own canonical geo anchor;
- R14.0 owns the Silpo shell at the canonical Silpo site;
- R14.3 owns visible Silpo facade identity/sign at that same site, including explicit `Сільпо` text;
- validation-only layer/separation checks remain `mutation=0` and reject cross-parcel identity instead of repairing it late;
- strict runtime evidence and the focused landmark launcher require the factual `R14.3 Silpo facade identity pass built at` stage, preventing a shell-only Silpo false pass;
- `VERIFY_PASS45_LANDMARK_IDENTITY.py` and `.github/workflows/pass45-landmark-identity.yml` protect the source contract;
- no late `Destroy`/`RemoveInstance` cleanup is used to disguise a primary landmark-authoring error.

Runtime Museum/Culture/Silpo visual identity and photo fidelity still require factual UE screenshots. Structural ownership does not make their current BasicShape-heavy construction visually acceptable.

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

Exact per-weapon sight sockets/offsets and local UE visual approval remain pending.

### 4.12 Weapon audio silent-path closure — 2026-08-26

Source now closes the unassigned/empty near-shot silence path without calling temporary generic audio final content:

- exact assigned `UOCWeaponAudioProfile` event sets remain first priority;
- `UOCWeaponAudioComponent::EnsureRepositoryFallbackProfile()` lazily creates a transient event-local presentation fallback only when the requested set is empty;
- AK first prefers tracked `/Game/AK-47/.../AK47_Fire_Cue`, `Reload_Cue` and `AK47_Empty_Cue`;
- other missing shot events may temporarily use tracked `/Game/R13/Audio/gunfire_sfx`;
- tracked reload assets prevent an entirely empty temporary reload-start path;
- missing distant tail can reuse the factual near report at reduced volume;
- pump action can use tracked `/Game/R13/Audio/shotguncock`;
- bolt/lever exact mechanical audio remains explicit `AUDIO CONTENT GAP`;
- tracked `snd_bullethit` supplies a temporary impact fallback;
- fallback creation does not mutate ammo, damage, fire cadence, projectile/trace authority, transforms or action timing;
- source markers: `PASS45_WEAPON_AUDIO_FALLBACK_READY ... authoritative_mutation=0 runtime_acceptance=0` and `PASS45_WEAPON_AUDIO_CONTENT_GAP`;
- `VERIFY_PASS45_WEAPON_AUDIO_FALLBACK.py` plus `.github/workflows/pass45-weapon-audio-fallback.yml` guards this path.

Final per-weapon shot character, indoor/outdoor/distant variants, suppressor behavior, complete reload layers, bolt/lever mechanics and UE mix/audibility remain open.

### 4.13 Primitive weapon / grenade / smoke retirement and transactional throw — 2026-08-26

Source now fails closed instead of rendering known-bad primitive content:

- weapon/launcher BasicShape fallbacks are hidden before production-load failure can render them;
- valid runtime must emit `PASS45_PRIMITIVE_WEAPON_RUNTIME_READY` and must not emit `PASS45_VISIBLE_PRIMITIVE_WEAPON_FAIL`;
- real fallback meshes attach to the unscaled visual root while invisible physics/collision authority remains separate;
- grenade visible Engine sphere is replaced by the tracked R13 grenade mesh; missing production visual fails visibly instead of restoring a sphere;
- smoke BasicShape puff cluster is physically retired; missing authored smoke VFX remains explicit `CONTENT GAP`;
- grenade throw uses swept/overlap clearance, `DontSpawnIfColliding`, commits inventory only after factual projectile spawn and inherits carrier velocity;
- valid acceptance requires `PASS45_GRENADE_THROW_COMMIT_READY` and the presentation bridge marker;
- tracked authored `snd_throw1` now plays from that factual replicated successful-throw event, never from input-before-spawn; READY/CONTENT GAP markers keep runtime audibility fail-visible without moving gameplay authority into presentation;
- `VERIFY_PASS45_PRIMITIVE_WEAPON_RETIREMENT.py` and `VERIFY_PASS45_GRENADE_SMOKE_PRIMITIVE_RETIREMENT.py` protect these paths.

Rendered weapon/launcher/grenade appearance, authored smoke and first-person grenade animation remain local-runtime/content work.

### 4.14 Landmark identity false-pass closure — 2026-08-26

Source and acceptance automation now distinguish a correct parcel/shell from visible landmark identity:

- `VERIFY_PASS45_LANDMARK_IDENTITY.py` checks Museum/Culture/Silpo canonical anchors and identity ownership;
- Museum source is forbidden from carrying Culture House column identity or Silpo signage;
- Culture House is required to carry the six-column civic signature at its own site;
- R14.0 Silpo shell and R14.3 facade/sign identity must both target the canonical Silpo site;
- strict runtime evidence and `RUN_R21_LANDMARK_OWNERSHIP_RUNTIME_ACCEPTANCE.cmd` require the factual R14.3 facade identity stage;
- `.github/workflows/pass45-landmark-identity.yml` and cumulative `RUN_ALL_VERIFY.py` include the guard.

This is **CODED_UNTESTED**, not runtime visual acceptance.

### 4.15 Pre-tick sector and second START tree-load recovery — 2026-08-31

The 2026-08-31 black-window investigation exposed two additional startup source defects after the original constructor/CDO tree-load fix:

- Block0 observes world state in `UWorldSubsystem::OnWorldBeginPlay`, but the legacy Oster sector was created only later from GameMode `BeginPlay`, producing factual `oster_sector_count_0`;
- `UOCR13StadiumSurfaceSubsystem::OnWorldBeginPlay()` independently synchronously loaded `HillTree_02` and `ScotsPineTall_01`, recreating the heavy KiteDemo START dependency.

Current source recovery:

- `AOCGameModeRuntimeSafe::InitGame()` creates and tags the lightweight canonical `AOCWorldSectorOster` before world-subsystem BeginPlay;
- the legacy duplicate created during base GameMode BeginPlay is retired before first gameplay tick, leaving exactly one live sector;
- no KiteDemo tree load was moved into pre-tick startup;
- automatic Stadion Oster world-subsystem instantiation is quarantined with `UCLASS(Abstract)` until a safe delayed/content-repaired path is available;
- this intentionally leaves automatic stadium presentation as a visible `CONTENT GAP / RUNTIME PENDING`, not a false acceptance;
- `VERIFY_PASS45_PRETICK_SECTOR_STARTUP_GUARD.py` plus `.github/workflows/pass45-pretick-sector-startup-guard.yml` guards the lifecycle, Block0 fail-closed contract, stadium quarantine and normal-launcher opt-in absence;
- exact substantive source head `6cc2158d31f10ee50a1c5d3ca096b86b4b49022b` passed full `Source verification` and all relevant exact-head source gates.

A new local UE 5.8 Quick Normal run is the next factual gate. Until it succeeds, status remains **RUNTIME REJECTED 2026-08-31**.

## 5. Active requirements

| ID | Requirement | Status | Current action |
|---|---|---|---|
| STALE-OWNER-001 | Old code/verifiers must not overwrite or resurrect newer runtime behavior | CODED_UNTESTED | Physical retirement + validation guards retained. |
| PERF-COLLAPSE-001 | No severe FPS/thermal collapse | CODED_UNTESTED | ~60 FPS recovery cap; final mixed soak pending. |
| VIS-BLACK-WORLD-001 | No black ground/world corruption or unusable black startup | CODED_UNTESTED / RUNTIME REJECTED | Latest 2026-08-31 Quick Normal stayed black; startup source recovery is now source-verified and local re-run is required. |
| LOC-MUSEUM-001 | Correct visible Oster Local History Museum | CODED_UNTESTED | Museum identity source guarded; direct visual proof pending. |
| LOC-CULTURE-001 | Culture House separate from Museum | CODED_UNTESTED | Separate geo owner + six-column identity guard; direct visual proof pending. |
| LOC-SILPO-001 | Silpo one correct site owner and visible facade identity | CODED_UNTESTED | R14.0 shell + mandatory R14.3 `Сільпо` facade stage; direct screenshot proof pending. |
| VIS-GENERIC-RESIDENTIAL-001 | No rejected generic house/fence/tower family | CODED_UNTESTED | Source owners retired; runtime absence proof pending. |
| GAME-VEHICLE-TELEPORT-001 | Vehicle enter/drive/exit never returns to Museum | CODED_UNTESTED | Initial-only BASE logic; local civilian/HMMWV/BTR test pending. |
| VEH-HMMWV-001 | Correct HMMWV proportions/orientation and >=80 km/h road target | IN_PROGRESS | Proportional fit source-coded; speed/calibration runtime pending. |
| VEH-BTR-001 | BTR proportions/material/forward/remote operator correct | IN_PROGRESS | Material intake + proportional source path exists; runtime white/axis/operator gaps open. |
| VEH-M2-001 | Coherent M2 mount, 360° yaw, correct camera/pitch | IN_PROGRESS | Pitch/mount source work partial; final hierarchy/runtime pending. |
| WEAPON-MATERIAL-001 | Required available weapons use authored material + texture deps | CODED_UNTESTED | R3/fresh-load path coded; rendered rack proof pending. |
| WEAPON-FIRING-001 | Ammo/recoil/muzzle/audio count agree on factual shots | CODED_UNTESTED | Confirmed-shot ownership source-coded; local firing matrix pending. |
| WEAPON-DROP-001 | Deliberate weapon drops fall/collide/settle and replicate | CODED_UNTESTED | Rigid-body source path coded; local physics proof pending. |
| WEAPON-ACTION-001 | Exact selectors + deterministic Burst3/manual cycles | CODED_UNTESTED | Finite Burst3 + replicated manual gate + procedural cue source-coded. |
| WEAPON-AUDIO-001 | No accepted silent weapon; exact per-weapon/mechanical audio | IN_PROGRESS / CODED_UNTESTED / CONTENT GAP | Source-level silence fallback coded; final authored identity/mix and bolt/lever content pending. |
| WEAPON-ADS-001 | Per-weapon sight alignment is factual, not generic | IN_PROGRESS / CODED_UNTESTED | Fail-visible socket architecture + diagnostics coded; all exact profiles remain uncalibrated. |
| WEAPON-PRIMITIVE-001 | No visible BasicShape weapon/pickup/launcher fallback in accepted runtime | CODED_UNTESTED | Source fail-closed retirement complete; rendered zero-primitive proof pending. |
| GRENADE-VISUAL-001 | No primitive grenade/smoke and no inventory loss on failed throw | CODED_UNTESTED / CONTENT GAP | Primitive retirement + transactional throw + authored throw audio coded; authored hand animation, flash VFX and runtime visual proof pending. |
| UI-TACTICAL-MAP-001 | `M` matches compact central-Oster topology | CODED_UNTESTED | Runtime screenshot required. |
| VIS-TREES-001 | No primitive/fantasy visible tree family | CODED_UNTESTED / CONTENT GAP / RUNTIME PENDING | Final KiteDemo tree identities remain quarantined from normal startup after UE 5.8 material/static-mesh rejection; no constructor/CDO or stadium START sync-load may restore them. Runtime material/LOD repair and direct visual acceptance remain open. |
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
21. generic ADS/FOV-only presentation described as factual sight alignment;
22. an unassigned/empty shot profile silently swallowing an otherwise accepted factual shot;
23. temporary generic weapon audio fallback being called final per-weapon authored sound acceptance;
24. a shell-only Silpo being accepted without the visible R14.3 facade/sign identity stage;
25. a six-column Culture House signature appearing at the Museum parcel.

## 7. Current execution order

1. [x] Preserve latest runtime evidence and mark **RUNTIME REJECTED 2026-08-31**.
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
12. [x] Close source-level silent-shot path with event-local repository audio fallback and dedicated guard/workflow.
13. [ ] Replace procedural manual-action cue with accepted authored moving-part/skeletal presentation where assets support it.
14. [ ] Replace temporary generic audio fallback with accepted exact per-weapon shot/reload/distant/mechanical profiles and close bolt/lever audio gaps.
15. [ ] Inspect each exact production weapon in local UE 5.8, confirm rear/front/optic references and calibrate ADS transforms; only factual profiles may set `bADSCalibrated=true`.
16. [x] Source-retire visible primitive weapon/pickup/launcher fallbacks and make runtime acceptance fail closed. **Rendered proof pending.**
17. [x] Source-retire primitive grenade/smoke visuals and make grenade throw transactional/safe. **Authored smoke and throw audio are source-wired; authored hand animation, flash VFX and runtime proof remain pending.**
18. [x] Source-close Museum/Culture House/Silpo identity/separation and require the R14.3 Silpo facade/sign stage. **Visual runtime proof pending.**
19. [ ] Replace rejected vegetation family and raise world/material/LOD fidelity. **Normal startup now quarantines the rejected KiteDemo tree/material chain; the pre-tick sector and second stadium START-load source blockers are source-verified fixed. Tree material/LOD repair and direct UE 5.8 visual acceptance remain required.**
20. [ ] Complete HMMWV M2 hierarchy/360°/camera and >=80 km/h road tuning.
21. [ ] Close BTR white material/forward-axis/remote-operator gaps.
22. [ ] Validate fullscreen, ~60 FPS, native render scale and 10-minute mixed thermal soak.
23. [ ] Validate tactical map screenshot.
24. [ ] Current-head `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` import + build + gameplay + automated gates + direct screenshots. **Before this full gate, the immediate local blocker is a new Quick Normal launch proving a usable first frame.**
25. [ ] Merge PR #94 only after factual current-head runtime acceptance.

## 8. Next factual runtime gates

### Build / import

- current branch/head matches origin;
- Stein R3 authored import + independent fresh load succeeds;
- production HMMWV/M2/BTR import/fresh load succeeds;
- UBT/UE 5.8 build exits 0;
- no required available weapon reports placeholder material or zero authored texture dependency.

### Startup

- Quick Normal reaches a usable first frame instead of a permanent black window;
- no `PASS45_BLOCK0_PRETICK_GROUND_FAIL` / `oster_sector_count_0`;
- normal launcher does not opt into `Pass45LoadKiteDemoTrees`;
- no synchronous `HillTree_02` / `ScotsPineTall_01` stadium START-time load path returns;
- one canonical Oster sector exists before first gameplay tick.

### Weapons

- 10-shot semi count agreement;
- full-auto hold has no ghost recoil after authoritative firing stops;
- release has no artificial downward kick;
- empty-mag hold creates no shot recoil/audio;
- muzzle/tracer/projectile begins at visible production muzzle;
- drop from standing/walking/running falls, collides and settles;
- M700/870/Lever action cycle visually follows authoritative state;
- source fallback means a missing profile cannot silently erase the shot, but exact final sound identity remains required;
- action audio is actually populated/audible or remains explicit content gap;
- every accepted weapon receives dedicated hip/ADS screenshots;
- `oc.Weapon.ADS.Debug 1` evidence is used when calibrating sight axes;
- no uncalibrated ADS profile is described as READY;
- no visible Cube/Cylinder/BasicShape weapon fallback remains on an accepted rack/equipped/drop state.

### World / landmarks

- no black-world corruption;
- Museum visually reads as Museum, not Culture House;
- Culture House and Silpo visibly separate;
- Silpo visibly shows the correct facade identity/sign at its canonical site;
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

## 9. Preserved local UE build/import rejection — 2026-08-25

Status of this historical attempt: **LOCAL UE BUILD REJECTED**. This is preserved as factual chronology even though a later corrected build reached gameplay.

- UE 5.8/MSVC rejected the tactical-map `FVector2D` road table with **C2131** while it was `constexpr`; source now uses a normal `const` table.
- UE 5.8 Interchange rejected the deprecated `auto_detect_mesh_type` property during HMMWV/M2 GLB intake; current importer explicitly forces StaticMesh through the supported Interchange API.
- These corrections do not erase the failed attempt.
- A later run reaching gameplay does not erase the earlier build/import rejection or the newer Pass45 runtime rejection.

Current build/import correction state remains **CODED_UNTESTED** until a current-head local UE 5.8 import/build exits successfully.

## 10. Historical truth retained

- Pass44 2026-08-24 remains historically **RUNTIME REJECTED**.
- Pass45 2026-08-25 local build/import rejection remains factual chronology even though later corrective builds reached gameplay.
- Earlier 2026-08-25 and 2026-08-27 runtime rejections remain preserved separately.
- The 2026-08-31 Quick Normal black-window rejection is the latest factual local verdict until superseded by a new current-head UE 5.8 run.
- None of those historical failures may be erased by later source fixes, and none of their stale workaround owners gain authority merely because they once existed.

## 11. Current verdict

**PASS 45 = ACTIVE / RUNTIME REJECTED 2026-08-31 / STARTUP SOURCE RECOVERY SOURCE-VERIFIED / LOCAL UE RE-RUN PENDING.**

PR #94 remains **OPEN / UNMERGED**.
