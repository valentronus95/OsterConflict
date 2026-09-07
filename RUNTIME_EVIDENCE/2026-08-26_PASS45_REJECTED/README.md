# PASS 45 — LOCAL UE 5.8 RUNTIME EVIDENCE — 2026-08-26

Verdict: **RUNTIME REJECTED 2026-08-26**

This evidence supersedes the 2026-08-25 gameplay rejection as the latest factual rendered runtime observation. It does **not** mean all previous failures persist unchanged: several material and performance improvements are visible, but the build is still not acceptable.

![2026-08-26 runtime screenshot contact sheet](PASS45_RUNTIME_2026-08-26_SCREENSHOTS.svg)

## Confirmed improvements

- The run reaches actual gameplay.
- Large near-black world corruption from the previous rejected run is no longer the dominant rendered state in these screenshots.
- Several firearm meshes/materials now render as recognizable authored assets, including AK-family/M14/MP5/Lever Action examples.
- HMMWV forward orientation is now coherent enough to drive forward normally.
- Runtime HUD reports a stable 60 FPS in the supplied screenshots.
- The laptop still warms, but the user did not report the previous runaway 100–156 FPS behavior in this run.

These improvements must be retained, but none is sufficient for runtime acceptance.

## Screenshot map

1. Weapon rack: several real weapon meshes are visible, but primitive box/cylinder pickup visuals remain.
2. Marked weapon rack defects: multiple pickup objects are still low-detail primitive geometry.
3. M14 in first person: authored weapon is visible; environment remains visually flat/prototype-grade.
4. Firing view: visible shot/tracer origin is offset below/away from the expected muzzle line.
5. MAC-10 pickup: world pickup appears extremely small/poorly readable at interaction distance.
6. Primitive/blob environment geometry near MAC-10 interaction.
7. Lever Action authored mesh is visible, while large grey blob/tree proxy geometry remains nearby.
8. Anti-Armor Launcher first-person visual is still a cylinder/box primitive rather than a recognizable authored launcher.
9. Museum/Culture-House identity failure: six-column civic facade is still present at/inside the Museum presentation; landmark separation remains visually rejected.
10. Vegetation remains rejected: malformed thick trunks, low-detail crowns and repeated primitive-looking silhouettes.
11. MP5 ADS/first-person view with HMMWV ahead; sight presentation still requires per-weapon alignment validation.
12. HMMWV/M2: Browning/roof shield/mount are not assembled as one coherent turret station; mount geometry is visibly wrong.
13. HMMWV rear driving view; orientation now reads correctly, HUD shows 14 km/h in this moment.
14. Vehicle driving view at 65 km/h; gameplay top-speed calibration remains below requested HMMWV target.
15. M2 gunner view: weapon/shield geometry clips or leaves major obstructing black planes; the Browning itself is poorly presented.
16. M2 mount close-up: components appear detached/floating/exploded above the HMMWV turret shield.
17. BTR-4 side view: production mesh/camouflage is substantially better, but a large white/default material region remains on the upper/front hull.
18. BTR-4 driving view: possession/runtime state can make a major portion of the hull appear white/default; vehicle forward-axis/orientation also remains suspect from user observation.

## User-observed runtime defects attached to this evidence

- Several weapons have no firing audio.
- Holding LMB can stop actual firing while camera shake/recoil continues; releasing LMB can drive the camera downward.
- Projectile/tracer/muzzle origin appears below the barrel on multiple weapons.
- Weapon recoil, camera recoil and firing-state lifetime are not correctly separated.
- Fire-mode behavior is inconsistent; weapons that should be semi-only, select-fire, bolt-action, pump-action or full-auto are not consistently constrained by their actual class/variant.
- No coherent fire-mode selector/UI exists where a select-fire weapon supports multiple modes.
- ADS/sight alignment is inconsistent and does not reliably line up the camera, rear sight/optic, front sight and muzzle axis.
- Dropped/pickup weapons can remain floating instead of falling, colliding, settling and sleeping on the ground.
- Grenade model, throw presentation and smoke VFX are prototype-grade/unacceptable.
- Museum and Culture House remain visually mixed/overlapped; the Silpo sign/identity has not migrated to the actual Silpo site.
- Existing trees remain visibly unacceptable and must be replaced rather than cosmetically retinted.
- HMMWV requested road gameplay speed is at least 80 km/h.
- HMMWV M2 station must be one coherent ring/shield/weapon/gunner assembly in yaw; the selected project turret configuration requires full 360-degree traverse, with weapon elevation handled inside the cradle.
- M2 gunner camera must show a usable mounted weapon view without clipping/floating shield pieces; firing must not create a persistent downward camera drift.
- BTR-4 still has a white/default material failure, can become substantially white after entering it, drives with the visual/physics forward direction mismatched, and has an illogical turret camera.
- BTR-4E/BM-7 Parus gameplay must use an interior remote-operator optic/monitor presentation rather than pretending the operator is physically looking through the external weapon module.
- Graphics are now brighter and somewhat improved, but still visibly prototype-grade: flat ground, crude proxy geometry, weak material detail, low-quality vegetation and landmark composition. A stable 60 FPS does not justify keeping this visual state.

## External factual references used for the next implementation contract

- Heckler & Koch MP5 product specification: MP5 configurations support semi-auto/full-auto/3-round burst modes. https://www.heckler-koch.com/en/Products/Military%20and%20Law%20Enforcement/Submachine%20guns/MP5
- HK military/LE catalog documents MP5 trigger groups including safe/single/full-auto and burst configurations. https://hk-usa.com/wp-content/uploads/HK-USA-MILITARY-LE-COMBINED-CATALOG1.pdf
- U.S. Army M14 field manual documents M14 rifles modified/configured for automatic fire; exact in-game M14 fire modes must follow the specific asset/configuration rather than a generic rifle rule. https://rdl.train.army.mil/catalog-ws/view/100.ATSC/710C9E3A-308E-49A1-A22E-CECA92C3F89A-1274548901937/fm23-8/fm23_8.pdf
- MAC-10 operation/maintenance documentation describes semi/full selector operation for selective-fire versions. https://nazarian.no/images/wep/176_mac1011Manual.pdf
- Intratec TEC-9 manual describes the production TEC-9 as a semi-automatic pistol. https://www.survivorlibrary.com/library/intratec_tec9_sub_machinegun.pdf
- Remington lists Model 700 as a bolt-action rifle family and Model 870 as a pump-action shotgun family. https://www.remarms.com/support/owners-manuals-%28remarms%29
- U.S. Army M249 documentation defines the M249 as a fully automatic weapon. https://rdl.train.army.mil/catalog-ws/view/100.ATSC/4BEFBB3D-7A12-4E22-97A6-50E841CF90C3-1495568587177/tc3_22x249.pdf
- AM General HMMWV performance documentation gives a 70 mph / 113 km/h maximum-speed figure for the documented M1151/M1152/M1165 family sheet. The game does not need to force that exact maximum, but a hard ceiling below the requested 80 km/h target is not acceptable. https://amgeneral.com/wp-content/uploads/2019/12/M1167.pdf
- U.S. Marine Corps machine-gun manual documents M2 .50 cal use on HMMWV armament carriers and vehicle mounts. Project-specific ring/shield traversal remains a gameplay/configuration requirement and must be validated against the chosen modeled mount. https://www.trngcmd.marines.mil/Portals/207/Docs/TBS/MCWP%203-15.1%20Machine%20Guns%20and%20Machine%20Gun%20Gunnery.pdf
- BTR-4E reference material describes the BM-7 Parus as a Remote Weapon Station and the gunner/commander as observing/operating from within the hull using day/night sighting/fire-control equipment. This supports an interior monitor/optic gameplay presentation rather than an exposed external-gunner camera. https://shared.akamai.steamstatic.com/store_item_assets/steam/apps/1502380/manuals/CM_Black_Sea_Manual.pdf?t=1728289513

## Runtime truth

This evidence does not authorize merging PR #94. The latest factual verdict is **RUNTIME REJECTED 2026-08-26** until a later current-head UE 5.8 full runtime test corrects and visually proves the failures recorded here.