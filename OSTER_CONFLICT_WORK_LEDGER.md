# OSTER CONFLICT — WORK LEDGER

> Постійний журнал фактичного стану `main`. Runtime-скрін/лог/playtest завжди має пріоритет над code-only твердженням.

## 1. Поточний контекст

- Repository: `valentronus95/OsterConflict`
- Active branch: `main`
- UE target: 5.8.x Windows
- Project: `OsterConflict/OsterConflict.uproject`
- User-facing launcher: **тільки `START_HERE.cmd`**.
- `RUN_*.cmd` — внутрішні helper scripts. Не створювати новий user-facing launcher під кожну R-версію.
- Persistent evidence: `RUNTIME_AUDIT_2026-08-21.md`, `LEGACY_BLOCKOUT_AUDIT_2026-08-21.md`, `RUNTIME_PLAYTEST_AUDIT_2026-08-21_1744.md`.
- User playtest 2026-08-21 17:44 + пакет `Oster-photo-bag.docx` є authoritative evidence для поточного runtime.
- Не створювати нові декоративні R15/R16 layers, доки поточний runtime backlog не закритий.

## 2. Статусні правила

- `IN_PROGRESS` — runtime уже показав проблему або fix ще не підтверджено/не завершено.
- `CODED_UNTESTED` — source fix є, але UE runtime його ще не підтвердив.
- `VERIFIED BUILD` — конкретний compile blocker підтверджено усуненим фактичним наступним build/run.
- `VERIFIED RUNTIME` — тільки після фактичного UE runtime/playtest.
- Один landmark/site = один authoritative placement owner; cleanup не замінює ownership.
- Generic fallback не може видаватися за exact production asset.
- Authored game-visual asset може закрити source-side visual gap лише як `CODED_UNTESTED`; runtime scale/pivot/material/placement acceptance все одно обов’язковий.

## 3. Активні вимоги

| ID | Вимога | Repeat | Status | Фактичний стан / що лишилось |
|---|---|---:|---|---|
| UI-BOOT-001 | Splash → main menu без чорної паузи | 1 | CODED_UNTESTED | `65377c4c...`, `7edaf9e4...`: доданий MoviePlayer startup loading screen над важким `OsterConflict_Runtime`, щоб platform splash не провалювався в чорний engine frame. Потрібен UE 5.8 startup acceptance. |
| UI-MENU-001 | Головне меню стабільне | ≥3 | IN_PROGRESS | Main menu в останньому run виглядає нормально. Потрібен повторний runtime після boot/travel змін, щоб переконатися, що presentation не регреснув. |
| UI-TRAVEL-001 | START має один зрозумілий перехід у deployment/game | ≥3 | CODED_UNTESTED | Current deployment CTA уже `ПОЯВИТИСЯ`, не другий `СТАРТ`; frontend source утримує approved background через local travel (`bLocalTravelPending`). Старий runtime gray/double-START evidence лишається authoritative до нового playtest. |
| UI-CHAT-001 | Team chat `Y`, global chat `U`, панель прихована без вводу | 1 | CODED_UNTESTED | `d0f7c323...`: runtime chat layer. Legacy `ChatPanel` приховується; `Y` Team, `U` Global, `Enter` send/close, `Esc` close. Потрібен runtime acceptance. |
| GAME-SPAWN-001 | Нормальний фактичний spawn, не порожнє поле | ≥3 | CODED_UNTESTED | `45bf9fd5...`: старі BASE точки на краях blockout більше не authoritative. Team One/Two BASE перенесені на тротуари central east-west corridor, дивляться в бік міста та snap-яться до collision surface. |
| GAME-WEAPONS-001 | 11 pickup classes біля фактичного spawn | ≥3 | CODED_UNTESTED | `45bf9fd5...`: біля primary BASE кожної команди runtime rack з 11 класів: AR, SMG, Pistol, Sniper, Shotgun, LMG, M14, MAC-10, TEC-9, Lever Action, Anti-Armor Launcher. |
| VIS-FP-001 | Production/real weapon visuals без primitive boxes | ≥3 | IN_PROGRESS | AK та частина exact R13 weapon meshes існують. `6eeb4aec...` прибрав стартову 0.20 s primitive паузу для real-mesh fallback. Exact M249/Remington production assets досі відсутні, runtime acceptance pending. |
| UI-TACTICAL-MAP-001 | `M` tactical map без конфлікту | 1 | CODED_UNTESTED | PR #18: `M` тепер canonical Tactical Map key через Enhanced Input; `OCCharacter` default DeployTrap = `V`, HUD показує `V DEPLOY`; map-owned input lock скидається навіть при blocking UI, drag має mouse capture/release. Sync `63406c0...` включає `main f02cdef...`; Tactical Map CI #87 і key/input contract CI #93 — PASS. Потрібні фактичний UE 5.8 build і runtime acceptance. |
| GAME-VEHICLE-INPUT-001 | Після exit з авто повертаються WASD/sprint/mouse | 1 | IN_PROGRESS | Recovery coded; повторний enter→exit acceptance у новому run ще не зафіксовано. |
| VEH-PICKUP-001 | Pickup/HMMWV має M2 Browning без proxy geometry | ≥2 | CODED_UNTESTED | `1370a101...` прибирає Cube/Cylinder і використовує real R13 machinegun лише як diagnostic fallback. `66e902d4...` додав authored external-only M2 game-visual GLB generator; `d8c13771...` M2-only importer створює canonical `/Game/Production/Weapons/M2/SM_M2_Browning`, причому локальний downloaded `m2_50cal_machinegun_cc0.glb` має пріоритет. `4de7123c...`, `a76d0403...`, `17c094fa...` автоматизують імпорт перед normal/Sandbox run. Потрібні UE import + scale/pivot/muzzle/gunner acceptance. |
| VEH-PICKUP-SPEED-001 | Pickup max speed 120 км/год | 1 | CODED_UNTESTED | `c9ea15f4...`: server/standalone runtime speed contract ставить 120 км/год, forward cap + assist force. Потрібен speed test. |
| ASSET-BTR-001 | BTR production model без green box/proxy | ≥2 | CODED_UNTESTED | `6b89bcff...` додав authored external-only 8×8 BTR-4 game-visual GLB generator; `8f3dac18...` імпортує локальний user FBX, якщо він присутній, інакше генерує repo-safe GLB у canonical `/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus`. `122ad4fd...`, `5d6cf9e9...`, `7abcb72d...` автоматизують імпорт перед normal/Sandbox run. Локальний FBX лишається dev-only до перевірки ліцензії; потрібні UE import + scale/ground/wheel/camera acceptance. |
| VEH-BTR-SPEED-001 | BTR max speed 90 км/год | 1 | CODED_UNTESTED | `c9ea15f4...`: server/standalone runtime speed contract ставить 90 км/год. Потрібен speed test. |
| ASSET-CHARACTER-001 | Production character/skins | ≥2 | IN_PROGRESS | Останній runtime показав повну real character model, але бойовий profile/скіни ще не прийняті як final. |
| DEBUG-FLIGHT-001 | Керований spectator/free-fly test mode | 1 | IN_PROGRESS | Gameplay debugger/spectator-like view не є готовим dev free-fly contract. |
| LOC-MUSEUM-001 | Museum окремо від Silpo/Culture | ≥4 | IN_PROGRESS | `65c5cac7...` + `f36eab1a...`: landmark startup coordinator скасовує legacy delayed timers і запускає authoritative Museum stages одним startup pass зі збереженням authority-only replacements. Runtime separation ще не підтверджена. |
| LOC-SILPO-001 | Silpo лише на своїй реальній локації | ≥4 | IN_PROGRESS | Той самий startup coordinator прибирає 5.35 s late rebuild path source-side. Exterior/layout все ще потребує runtime acceptance проти останнього регресійного screenshot. |
| LOC-CULTURE-001 | Culture House лише на своїй реальній локації | ≥4 | IN_PROGRESS | Canonical geo owner + startup coordinator source-side є; runtime acceptance і остаточне видалення foreign geometry pending. |
| LOC-STADIUM-001 | Stadion Oster georeferenced, правильно орієнтований | ≥3 | CODED_UNTESTED | `061a69b4...`: прибрана 154×112 m green Cube apron; stadium Z тепер snap-иться до фактичної collision surface/ground trace, один stadium owner збережено. Фото-географічна орієнтація ще потребує runtime перевірки. |
| LOC-TERRAIN-001 | Реальний relief, не плоска площина | ≥3 | IN_PROGRESS / DATA BLOCKED | Authoritative base `Ground` досі є приблизно 2400×2400 m Engine Cube. Репозиторій не містить підтвердженого heightmap/Landscape elevation data. Не вигадувати «реальний рельєф» формулою; потрібні фактичні terrain data/assets. |
| VIS-HOUSES-001 | Реальні canonical houses замість коробок | ≥3 | CODED_UNTESTED | `92ff812d...`: legacy `Buildings` Cube presentation приховано, collision-core лишено; існуючі `SM_House_Var01/02` лишаються visual owner. Потрібен overview acceptance. |
| VIS-GRASS-001 | Натуральне покриття травою | 1 | CODED_UNTESTED | `92ff812d...` приховав Cube grass tiles; `c67744a2...` вимкнув duplicate recovered foliage owner; `2d40fa53...` зробив DenseGroundFoliage єдиним owner і ущільнив grid 22 m → 13.5 m, 2–4 clumps/cell; `5e451eb4...` виправив real `SM_Plant` fallback. |
| VIS-FLICKER-001 | Без distant flicker/z-fighting/late rebuild | ≥3 | IN_PROGRESS | `65c5cac7...`/`f36eab1a...` source-side згорнули Museum/Silpo/Culture late rebuilds; `92ff812d...` прибрав visible box/grass overlaps; `82dcb4fd...` прибрав orphan roadside owner. Distant runtime acceptance ще потрібний. |
| LEGACY-BLOCKOUT-001 | Legacy blockout не перекриває current locations/assets | ≥2 | IN_PROGRESS | Raw-coordinate unfinished-building owners прибрані (`d6c0ec4f...`, `e610cd5f...`); `82dcb4fd...` прибрав orphan props; `92ff812d...` приховав visible legacy Buildings/grass presentation. Загальний runtime overview ще не прийнятий. |
| VIS-FENCES-001 | Реальні паркани без stretching | 1 | CODED_UNTESTED | Current `OCAssetModelDecorator::AddFenceLine()` вже tiles real `Fence_Old_1_2m` секціями приблизно по 195 cm, а не розтягує один mesh. Потрібен лише runtime visual acceptance/placement check. |
| VIS-STREETLIGHT-001 | Imported streetlight | 1 | CODED_UNTESTED | Source coded; current playtest не був acceptance check. |

## 4. Технічні фікси / build evidence

| ID | Результат | Status | Commit / доказ |
|---|---|---|---|
| BUILD-UE58-001 | Старий UE 5.8 include blocker | VERIFIED BUILD | `37a3a6d5...`. |
| BUILD-TACTICAL-MAP-001 | `C4458 Slot hides class member` | VERIFIED BUILD | Fix `bb7d49b5...`; попередній playtest дійшов до gameplay. |
| CRASH-OBJECTNAME-001 | Bus-station object-name crash | VERIFIED RUNTIME FOR REPORTED CRASH | Старий name collision не повторився. |
| ASSET-LFS-PREFLIGHT-001 | Git LFS hydration | VERIFIED FOR PREFLIGHT | `54b8c2cd...`; launchers мають hydration guard. |
| CRASH-WEAPON-FALLBACK-001 | `Pure virtual` у `ApplyRealFallback()` | RUNTIME DID NOT RECUR IN LATEST RUN | GC-safe refs `0213b53b...` + validity guards `6c13f70f...`. |
| VEHICLE-EXIT-RECOVERY-001 | Restore input stack after vehicle exit | CODED_UNTESTED | `30cae92a...`, `7c4d9138...`. |
| TACTICAL-MAP-SOURCE-001 | `M` map, `V` trap canonical | CODED_UNTESTED | PR #18: `599d488...` input-lock/drag safety; `b603621...` constructor + HUD key consistency; `33be351...` UE 5.8 build helper hardening; `63406c0...` sync з `main f02cdef...`. Tactical Map source/input CI #93 — PASS; реальний UE 5.8 compile/runtime ще не підтверджено. |
| LANDMARK-STARTUP-001 | Museum/Silpo/Culture без late startup rebuild | CODED_UNTESTED | `29948d4f...`, `65c5cac7...`, `f36eab1a...`; coordinator clears legacy timers and runs authoritative stages immediately. |
| LAUNCHER-UX-001 | Один user launcher | VERIFIED ENTRY POINT | `START_HERE.cmd` лишається єдиним user-facing entry point; нові `RUN_*` тільки internal helper. |
| CHAT-RUNTIME-001 | Hidden-by-default gameplay chat + `Y/U` channels | CODED_UNTESTED | `d0f7c323...`. |
| VEH-SPEED-RUNTIME-001 | Pickup 120 / BTR 90 speed contracts | CODED_UNTESTED | `c9ea15f4...`. |
| MOUNTED-GUN-FALLBACK-001 | Не показувати Cube/Cylinder замість M2 | CODED_UNTESTED | `1370a101...`; exact/canonical M2 first, real R13 machinegun fallback second. |
| M2-AUTHORED-VISUAL-001 | Purpose-built external M2 game visual | CODED_UNTESTED | `66e902d4...`; pure-Python GLB generator, external receiver/barrel/jacket/spade grips/sights/pintle only, no internal/manufacturing model. |
| M2-IMPORT-001 | Canonical M2 production import | CODED_UNTESTED | `d8c13771...`: local downloaded GLB wins; authored GLB generated if absent; target `/Game/Production/Weapons/M2/SM_M2_Browning`. |
| M2-LAUNCH-INTEGRATION-001 | M2 import runs before normal/Sandbox playtest | CODED_UNTESTED | `4de7123c...`, `a76d0403...`, `17c094fa...`; import failure does not block game and falls back honestly. |
| BTR4-AUTHORED-VISUAL-001 | Purpose-built external 8×8 BTR-4 game visual | CODED_UNTESTED | `6b89bcff...`; pure-Python GLB generator with hull, 8 wheels, external turret/cannon silhouette, hatches/optics/details; no engineering/manufacturing model. |
| BTR4-IMPORT-001 | Canonical BTR-4 production import | CODED_UNTESTED | `8f3dac18...`: local user FBX wins when present; authored GLB generated if absent; target `/Game/Production/Vehicles/BTR4/SM_BTR4_Bucephalus`. |
| BTR4-LAUNCH-INTEGRATION-001 | BTR-4 import runs before normal/Sandbox playtest | CODED_UNTESTED | `122ad4fd...`, `5d6cf9e9...`, `7abcb72d...`; runtime acceptance checklist includes scale/ground/8 wheels/camera/no green proxy. |
| WEAPON-FALLBACK-PRESENTATION-001 | Real weapon fallback без стартової primitive паузи | CODED_UNTESTED | `6eeb4aec...`. |
| BASE-SPAWN-RUNTIME-001 | BASE spawn перенесений із blockout edge у town corridor | CODED_UNTESTED | `45bf9fd5...`. |
| BASE-WEAPON-RACK-001 | 11 weapon classes біля actual BASE | CODED_UNTESTED | `45bf9fd5...`. |
| STADIUM-GROUND-001 | Прибрана giant green stadium slab + terrain snap | CODED_UNTESTED | `061a69b4...`. |
| HOUSE-PROXY-PRESENTATION-001 | Real houses visible, box collision-core hidden | CODED_UNTESTED | `92ff812d...`. |
| FOLIAGE-OWNERSHIP-001 | Один ground-cover owner без Cube grass | CODED_UNTESTED | `92ff812d...`, `c67744a2...`, `2d40fa53...`, `5e451eb4...`. |
| ORPHAN-ROADSIDE-001 | Прибрані props від уже видаленої недобудови | CODED_UNTESTED | `82dcb4fd...`. |
| BOOT-LOADING-001 | Deliberate startup presentation замість black gap | CODED_UNTESTED | `65377c4c...`, `7edaf9e4...`. |

## 5. Останній фактичний user run — 2026-08-21 17:44

Підтверджено runtime:
- splash Oster Conflict показується;
- є проміжковий чорний екран перед main menu;
- main menu повернулося і виглядає нормально;
- START мав незрозумілий gray/double-START flow;
- gameplay запускається без попереднього `Pure virtual` crash;
- AK real first-person visual працює; reload animation + shot smoke є;
- player character real model видно;
- Silpo interior існує;
- pickup можна бачити/використовувати;
- BTR, pickup gun, weapon/world visuals, terrain, houses і landmark layout у цьому run залишались неприйнятними;
- великий team chat panel був постійно видимий;
- distant flicker був;
- map largely flat, grass sparse;
- Museum/Silpo/Culture separation runtime не була досягнута;
- stadium/photo-geography fidelity не була досягнута.

Пакет доказів: user `Oster-photo-bag.docx`, 13 screenshots цього run.

Усі source fixes після цього run залишаються `CODED_UNTESTED`/`IN_PROGRESS`, доки новий UE 5.8 playtest їх не підтвердить.

## 6. Наступна черга

Порядок user request зберігається; завершені source-pass не повторювати без нового evidence:

1. Chat `Y/U`, pickup 120, BTR 90, M2/BTR authored canonical import paths: source-side coded; M249/Remington exact visuals open; усе чекає runtime acceptance.
2. Spawn/test contract: source fix `45bf9fd5...`, чекає runtime.
3. Museum/Silpo/Culture + legacy boxes: startup late-rebuild coordinator і частина legacy cleanup coded; чекає runtime separation evidence.
4. Stadium/houses/grass/fences: source fixes coded; **terrain relief лишається DATA BLOCKED без heightmap/Landscape data**.
5. Flicker: основні підтверджені overlap/late-rebuild джерела прибрані; потрібен distant runtime check.
6. Boot/travel: startup MoviePlayer + current `ПОЯВИТИСЯ` flow coded; потрібен normal-launch acceptance.
7. Source consistency cleanup `M` tactical map / `V` engineer trap, без нового input owner.
8. Новий UE 5.8 playtest. Статуси підвищувати лише після runtime evidence.

**Заборона:** ніяких нових декоративних R15/R16 layers до закриття цього backlog.