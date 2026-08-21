# TACTICAL MAP 2.0 — UE 5.8 RUNTIME ACCEPTANCE

Status before runtime test: `CODED_UNTESTED`

This checklist is the acceptance gate for `UI-TACTICAL-MAP-001`. Source/CI success is not runtime proof. A failed item keeps the feature out of `VERIFIED RUNTIME` and out of `main`.

## A. Launch and input ownership

- [ ] Launch through the normal `START_HERE.cmd` path and reach actual gameplay.
- [ ] Press `M` once: Tactical Map opens exactly once, with no duplicate/flickering layer.
- [ ] Press `M` again: Tactical Map closes exactly once.
- [ ] While the map is open, player movement/look input is blocked and mouse cursor is available.
- [ ] After closing the map, WASD, mouse look and normal gameplay input are restored immediately.
- [ ] `V` remains the DeployTrap key and `M` does not deploy a trap.
- [ ] Map does not open over frontend menu, deployment panel, admin panel, settings or active chat input.

## B. World image and alignment

- [ ] Background is an actual top-down capture of the current gameplay world, not a generated/static fake map.
- [ ] Museum is visible at the same relative world position as in gameplay.
- [ ] Stadium is visible at the same relative world position as in gameplay.
- [ ] Park is visible at the same relative world position as in gameplay.
- [ ] Center is visible at the same relative world position as in gameplay.
- [ ] Silpo is visible at the same relative world position as in gameplay.
- [ ] Separately-owned landmark actors are present in the map capture and are not missing merely because they are outside `AOCWorldSectorOster` component ownership.
- [ ] North indicator/orientation agrees with actual player/world movement.
- [ ] Player marker reaches the correct edge/sector when physically moving toward that area in the world.
- [ ] No obvious horizontal mirror, 90/180 degree rotation error or landmark offset exists between capture and overlays.

## C. Player and viewport interaction

- [ ] Player marker updates continuously while the pawn moves.
- [ ] Player heading marker rotates consistently with world yaw.
- [ ] Mouse wheel zooms around the cursor location, not only around screen center.
- [ ] Zoom cannot go below the full-map view or beyond the intended maximum.
- [ ] LMB drag pans the map.
- [ ] Pan is clamped: no empty background can be dragged into view beyond the map edges.
- [ ] Zoom/pan do not desynchronize POI, objective, player, squad or ping overlays from the captured world image.

## D. Squad and vehicle markers

Use at least two clients or one human + a valid replicated squad member/bot where the same network state is exercised.

- [ ] Only members with the same `TeamId` and `SquadId` appear as squad markers.
- [ ] A same-team member from another squad does not appear as a squad marker.
- [ ] An enemy team member does not appear as a friendly squad marker.
- [ ] On-foot squad member uses the `●` marker at the real character world position.
- [ ] When that squad member enters a vehicle, the marker changes to `▣` and follows the actual vehicle position.
- [ ] Leaving/despawning/changing squad removes the stale marker.
- [ ] Local player can open/keep the map while possessing an `AOCVehicleBase` vehicle pawn.
- [ ] Closing the map while in a vehicle restores driving/look input correctly.

## E. Capture objectives and squad orders

- [ ] Every live `AOCCapturePoint` appears automatically; the map is not limited to hardcoded A/B/C UI coordinates.
- [ ] A/B/C, when present in the Oster gameplay sector, are positioned on their actual capture-point actors.
- [ ] Neutral/capturing/owned/contested state updates on the marker from replicated capture-point state.
- [ ] Capture progress displayed on the map agrees with the actual objective state.
- [ ] `MOVE` squad order appears at its actual `WorldLocation`.
- [ ] `REGROUP` squad order appears at its actual `WorldLocation`.
- [ ] `ATTACK <ObjectiveId>` resolves to the matching live capture-point actor, not world origin `(0,0)`.
- [ ] `DEFEND <ObjectiveId>` resolves to the matching live capture-point actor, not world origin `(0,0)`.
- [ ] Invalid/unresolved objective ID produces no fake marker at the center/origin.

## F. Network tactical ping

Use two clients in the same team+squad, plus preferably a third client in a different squad/team.

- [ ] RMB on the map converts the clicked map point to a plausible world coordinate.
- [ ] Sender sees the accepted squad ping.
- [ ] Second client in the same team+squad sees the same ping at the same world location.
- [ ] Different squad does not receive the ping.
- [ ] Enemy team does not receive the ping.
- [ ] Ping label identifies the issuer.
- [ ] Ping remains aligned through zoom and pan.
- [ ] Ping expires after approximately 8 seconds using synchronized server time.
- [ ] Rapid RMB spam is server-throttled and does not flood recipients.
- [ ] Obviously invalid/far/out-of-contract ping coordinates are rejected server-side.
- [ ] A player with no valid team/squad still gets the local sandbox fallback marker without attempting squad routing.

## G. Capture/runtime stability

- [ ] Opening the map does not crash or produce an invalid/black render target.
- [ ] Map capture happens on open and is not visibly recaptured every frame.
- [ ] Normal gameplay frame pacing does not collapse while the map is open.
- [ ] Repeated open/close cycles do not leak duplicate widgets, capture actors or input mapping contexts.
- [ ] Respawn/repossess still leaves `M` mapped once and only once.
- [ ] Enter vehicle → open map → close map → exit vehicle → continue on foot works without stuck input.
- [ ] Opening/closing chat/settings/deployment before or after the map does not leave cursor/input mode stuck.

## H. Evidence required for acceptance

Record at minimum:

1. UE 5.8 build result and exact commit SHA.
2. One screenshot of the full map with recognizable Oster landmarks.
3. One screenshot after zoom/pan proving overlays stay aligned.
4. One screenshot or video showing a squad member transition to vehicle marker.
5. Two-client evidence of the same squad ping at the same location.
6. Objective state/order evidence for at least one capture point.
7. Relevant log excerpt if any warning/error appears.

Acceptance result must be written as one of:

- `VERIFIED RUNTIME` — every blocking item above passes on the tested SHA.
- `CODED_UNTESTED` — no runtime evidence yet.
- `IN_PROGRESS` — runtime produced at least one reproducible failure.
