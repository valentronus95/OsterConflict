# PASS45 GATE D / C / H SOURCE CLOSURE — 2026-08-26

Status: **CODED_UNTESTED / RUNTIME REJECTED 2026-08-25 REMAINS AUTHORITATIVE**

Branch: `fix/pass45-runtime-rejection-material-closure-20260826`
PR: #94
Current code milestone: `a306042bb54e254004c8896fba81148786543945`
Source workflows: **39/39 PASS**

## Gate D — Museum / Culture House identity

The previous landmark separation validator only rejected generic `Buildings/Landmark*` geometry inside canonical parcels. It did not prove that an authoritative Culture House actor itself was absent from the Museum parcel.

Current source truth:

- R13.7 Museum source is a residential Solonyna-house silhouette: brick body, metal gable roof, wooden upper volume/veranda/detail treatment;
- R13.7 does not encode a six-column civic facade;
- R14.6 Culture House explicitly owns the six-column facade and is tagged `R146_CultureHouseAuthoritative`;
- canonical geo anchors remain distinct: Museum `50.948239, 30.883865`, Culture House `50.948694, 30.881435`.

`OCR146LandmarkSeparationSubsystem` is still validation-only (`mutation=0`) and now additionally requires:

- exactly one `R137_MuseumPhotoModel` owner;
- exactly one `R146_CultureHouseAuthoritative` owner;
- Museum authored instances at Museum > 0;
- Culture authored instances at Culture House > 0;
- Culture authored instances at Museum = 0;
- Museum authored instances at Culture House = 0;
- exactly six Culture House column shafts;
- Museum/Culture separation >= 100 m;
- Culture owner anchor error <= 100 cm.

Runtime markers:

- `PASS45_LANDMARK_IDENTITY_VALIDATION_READY`;
- `PASS45_LANDMARK_IDENTITY_VALIDATION_FAIL`.

The strict runtime evidence verifier requires READY and rejects FAIL.

## Gate C — recovery thermal cap

The launcher already requested `-ExecCmds="t.MaxFPS 60"`, but a command-line string is not proof that UE retained the requested cap.

`UOCPerformanceSampleSubsystem` now validates the live runtime CVar after actual gameplay possession:

- obtains `t.MaxFPS` through `IConsoleManager`;
- requires actual value `60.0` with a narrow 0.5 tolerance;
- never lowers render scale or graphics quality as part of this check.

Runtime markers:

- `PASS45_THERMAL_CAP_RUNTIME_READY requested_fps=60 actual_tmaxfps=...`;
- `PASS45_THERMAL_CAP_RUNTIME_FAIL ...`.

The strict runtime evidence verifier requires READY and rejects FAIL.

## Gate H — actual fullscreen viewport

The launcher request `-fullscreen` is no longer accepted as proof by itself.

After gameplay possession, `UOCPerformanceSampleSubsystem` queries the live `UGameViewportClient::IsFullScreenViewport()` state.

Runtime markers:

- `PASS45_FULLSCREEN_RUNTIME_READY`;
- `PASS45_FULLSCREEN_RUNTIME_FAIL`.

The strict runtime evidence verifier requires READY and rejects FAIL.

## CI forward-port

- `VERIFY_WORLD_GEOMETRY_STABILITY_PASS_12.py` now protects Museum/Culture source identity and authoritative cross-parcel validation.
- `.github/workflows/world-geometry-stability-pass-12.yml` now triggers on Museum, Culture House and geo-reference source changes.
- `VERIFY_PLAYFLOW_PERFORMANCE_PASS_14.py` protects live `t.MaxFPS` and fullscreen evidence code.
- `.github/workflows/playflow-performance-pass-14.yml` now triggers on the sampler header and `RUN_R14_CURRENT_GAMEPLAY.cmd` as well as its implementation.
- `VERIFY_RUNTIME_RUNAWAY_HEAT_PASS_38.py` now actually protects the thermal cap instead of checking only historical bounded-lifecycle items.
- `VERIFY_PASS45_RUNTIME_EVIDENCE_LOG.py` requires Gate D/C/H READY markers and rejects their FAIL markers.
- `VERIFY_PASS45_STRICT_RUNTIME_ACCEPTANCE_HARNESS.py` protects those strict-evidence requirements.

## Source verification

At `a306042bb54e254004c8896fba81148786543945` the current PR matrix is **39/39 PASS**, including:

- `Source verification`;
- `World geometry stability pass 12`;
- `Playflow and performance pass 14`;
- `Runtime runaway heat Pass 38`;
- `Landmark shell ownership Pass 21`;
- `Pass 45 strict runtime acceptance harness`;
- `Runtime recovery Pass 45`.

## Runtime truth

None of these source markers prove the current local UE build renders correctly.

Pass45 remains **RUNTIME REJECTED 2026-08-25** until `START_HERE.cmd -> 2. ПОВНИЙ RUNTIME-ТЕСТ` proves:

- the actual runtime emits the new READY markers with no corresponding FAIL;
- the viewport is visibly correct/fullscreen in the target environment;
- the observed FPS is capped as intended without severe progressive heating;
- Museum and Culture House are visibly distinct in screenshots;
- all remaining Gate A–J visual/interaction requirements pass.
