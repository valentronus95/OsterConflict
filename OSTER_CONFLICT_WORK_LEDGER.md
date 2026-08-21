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
- Новий user playtest 2026-08-21 17:44 + пакет `Oster-photo-bag.docx` є authoritative evidence для поточного runtime.
- Не створювати нові декоративні R15/R16 layers, доки поточний runtime backlog не закритий.

## 2. Статусні правила

- `IN_PROGRESS` — runtime уже показав проблему або fix ще не підтверджено.
- `CODED_UNTESTED` — source fix є, але UE runtime його ще не підтвердив.
- `VERIFIED BUILD` — конкретний compile blocker підтверджено усуненим фактичним наступним build/run.
- `VERIFIED RUNTIME` — тільки після фактичного UE runtime/playtest.
- Один landmark/site = один authoritative placement owner; cleanup не замінює ownership.
- Відсутній source/production asset не можна вважати готовим через proxy або generic fallback; це лишається asset blocker до фактичного імпорту.

## 3. Активні вимоги

| ID | Вимога | Repeat | Status | Фактичний стан / що лишилось |
|---|---|---:|---|---|
| UI-BOOT-001 | Splash → main menu без чорної паузи | 1 | IN_PROGRESS | Normal launch показує splash Oster Conflict, потім проміжковий чорний екран, потім main menu. Треба зробити безперервний loading/frontend presentation. |
| UI-MENU-001 | Головне меню стабільне | ≥3 | IN_PROGRESS | Main menu знову є і візуально виглядає нормально, але START дає сірий transition/фон і flow сприймається як повторний START. |
| UI-TRAVEL-001 | START має один зрозумілий перехід у deployment/game | ≥3 | IN_PROGRESS | Після першого START весь екран/фон сіріє, потім deployment step 4 теж має кнопку `СТАРТ`, через що виглядає як подвійний запуск. Потрібно розділити loading і deploy action, перейменувати final deploy CTA. |
| UI-CHAT-001 | Team chat `Y`, global chat `U`, панель прихована без вводу | 1 | CODED_UNTESTED | `d0f7c323...`: доданий runtime chat layer. Legacy `ChatPanel` приховується; `Y` відкриває Team, `U` Global, `Enter` надсилає/закриває, `Esc` закриває. Потрібен UE 5.8 runtime acceptance. |
| GAME-SPAWN-001 | Нормальний фактичний spawn, не порожнє поле | ≥3 | CODED_UNTESTED | `45bf9fd5...`: старі BASE точки на краях blockout більше не authoritative. Team One/Two BASE перенесені на тротуари центрального east-west corridor, дивляться в бік міста та snap-яться до collision surface. Потрібен normal gameplay acceptance. |
| GAME-WEAPONS-001 | 11 pickup classes біля фактичного spawn | ≥3 | CODED_UNTESTED | `45bf9fd5...`: біля primary BASE кожної команди створюється один runtime rack з 11 класів: AR, SMG, Pistol, Sniper, Shotgun, LMG, M14, MAC-10, TEC-9, Lever Action, Anti-Armor Launcher. Потрібен pickup test. |
| VIS-FP-001 | Production/real weapon visuals без primitive boxes | ≥3 | IN_PROGRESS | AK та частина exact R13 weapon meshes уже реально існують. `6eeb4aec...` прибрав 0.20 s затримку real-mesh fallback для M249/M1911/MAC-10/Remington 870, але exact production M249/Remington assets відсутні й runtime ще не перевірений. |
| UI-TACTICAL-MAP-001 | `M` tactical map без конфлікту | 1 | IN_PROGRESS | Source: `M` map, trap `V`; новий playtest не дав runtime acceptance evidence для карти. |
| GAME-VEHICLE-INPUT-001 | Після exit з авто повертаються WASD/sprint/mouse | 1 | IN_PROGRESS | Recovery coded; новий run показує driving/vehicles, але повторний enter→exit acceptance не зафіксовано. |
| VEH-PICKUP-001 | Pickup/HMMWV має M2 Browning без proxy geometry | ≥2 | IN_PROGRESS / ASSET BLOCKED | `SM_Pickup` реально є і використовується. Exact M2 `/Game/Production/...` відсутній, source FBX у Git теж відсутній. `1370a101...` замінює Cube/Cylinder на реальний R13 machine-gun mesh як чесно позначений temporary fallback; exact M2 все ще не готовий. |
| VEH-PICKUP-SPEED-001 | Pickup max speed 120 км/год | 1 | CODED_UNTESTED | `c9ea15f4...`: серверний runtime speed contract ставить 120 км/год, forward cap + assist force біля старої rigid-body physics. Потрібен фактичний speed test. |
| ASSET-BTR-001 | BTR production model без green box/proxy | ≥2 | IN_PROGRESS / ASSET BLOCKED | Повторний Content/SourceAssets inventory: exact BTR-4 production mesh у Git відсутній; є лише metadata з очікуваним `BTR4_Bucephalus.fbx`, самого FBX немає. Не підміняти BTR цивільним або вигаданим asset path. |
| VEH-BTR-SPEED-001 | BTR max speed 90 км/год | 1 | CODED_UNTESTED | `c9ea15f4...`: серверний runtime speed contract ставить 90 км/год, forward cap + assist force. Потрібен фактичний speed test. |
| ASSET-CHARACTER-001 | Production character/skins | ≥2 | IN_PROGRESS | Новий runtime уже показує повну real character model (джинси/кепка/бронежилет), тобто asset path працює хоча б для одного profile. Але це ще не прийнятий бойовий skin/profile і не всі персонажі перевірені. |
| DEBUG-FLIGHT-001 | Керований spectator/free-fly test mode | 1 | IN_PROGRESS | User випадково відкрив Unreal gameplay debugger/spectator-like view через клавішу й побачив персонажа збоку. Не вважати готовим flight mode. Потрібен окремий зрозумілий dev free-fly contract без debugger overlay. |
| LOC-MUSEUM-001 | Museum окремо від Silpo/Culture | ≥4 | IN_PROGRESS | `b11a2133...`: окремий 4.95 s museum site-cleanup subsystem вимкнений як дубльований late owner. Основний R13.7 photo model усе ще має 5.10 s build delay, тому late rebuild проблема ще не закрита. |
| LOC-SILPO-001 | Silpo лише на своїй реальній локації | ≥4 | IN_PROGRESS | Interior уже існує, але exterior знову сирий/regressed: parking/signage/banners/фасад втрачено або перекрито. Runtime separation від Museum/Culture не підтверджена; R14.0 Silpo build досі стартує пізно. |
| LOC-CULTURE-001 | Culture House лише на своїй реальній локації | ≥4 | IN_PROGRESS | R14.6 Culture House має окремий canonical geo owner, але runtime acceptance ще немає. Foreign generic/legacy geometry навколо site має бути остаточно прибрана без startup races. |
| LOC-STADIUM-001 | Stadion Oster georeferenced, правильно орієнтований | ≥3 | IN_PROGRESS | Runtime показує box/blockout і криве розміщення. Сам anchor недостатній: потрібні footprint, door/front orientation, road/forest relation та relief по фото. |
| LOC-TERRAIN-001 | Реальний relief, не плоска площина | ≥3 | IN_PROGRESS | Новий runtime показує майже суцільну плоску зелену площину. Segmented terrain source не дає потрібного результату. |
| VIS-HOUSES-001 | Реальні canonical houses замість коробок | ≥3 | IN_PROGRESS | Новий overview показує десятки білих/сірих box houses. Потрібно прибрати visible legacy blockout і використовувати imported/photographic house assets. |
| VIS-GRASS-001 | Натуральне покриття травою | 1 | IN_PROGRESS | Runtime: трава лише окремими рідкими `клубочками`; потрібен landscape/foliage coverage, не випадкові clumps. |
| VIS-FLICKER-001 | Без distant flicker/z-fighting/late rebuild | ≥3 | IN_PROGRESS | `b11a2133...` прибрав один підтверджений 4.95 s museum mutation path. Інші delayed landmark layers і overlap/z-fighting ще треба прибрати. |
| LEGACY-BLOCKOUT-001 | Legacy blockout не перекриває current locations/assets | ≥2 | IN_PROGRESS | `d6c0ec4f...` прибрав raw-coordinate unfinished-building shell із recovered environment; `e610cd5f...` вимкнув другого owner цього ж site. Загальний visible blockout по карті ще не закритий. |
| VIS-FENCES-001 | Реальні паркани без stretching | 1 | IN_PROGRESS | Real assets є, tiling/placement pending. |
| VIS-STREETLIGHT-001 | Imported streetlight | 1 | CODED_UNTESTED | Source coded; current playtest не був acceptance check. |

## 4. Технічні фікси / build evidence

| ID | Результат | Status | Commit / доказ |
|---|---|---|---|
| BUILD-UE58-001 | Старий UE 5.8 include blocker | VERIFIED BUILD | `37a3a6d5...`. |
| BUILD-TACTICAL-MAP-001 | `C4458 Slot hides class member` | VERIFIED BUILD | Fix `bb7d49b5...`; новий playtest дійшов до повного gameplay. |
| CRASH-OBJECTNAME-001 | Bus-station object-name crash | VERIFIED RUNTIME FOR REPORTED CRASH | Старий name collision не повторився. |
| ASSET-LFS-PREFLIGHT-001 | Git LFS hydration | VERIFIED FOR PREFLIGHT | `54b8c2cd...`; launches проходять hydration. |
| CRASH-WEAPON-FALLBACK-001 | `Pure virtual` у `ApplyRealFallback()` | RUNTIME DID NOT RECUR IN LATEST RUN | GC-safe refs `0213b53b...` + validity guards `6c13f70f...`. Останній довгий gameplay run не впав цим crash; visual fallback issue лишається відкритим. |
| VEHICLE-EXIT-RECOVERY-001 | Restore input stack after vehicle exit | CODED_UNTESTED | `30cae92a...`, `7c4d9138...`. |
| TACTICAL-MAP-SOURCE-001 | `M` map, `V` trap | CODED_UNTESTED | `a6d31480...`, `d9d36c1...`, build fix `bb7d49b5...`. |
| LANDMARK-EXCLUSION-001 | Museum/Silpo/Culture cleanup guard | RUNTIME INSUFFICIENT | `dc07e098...`, `c248578a...`; latest playtest still shows mixed/box geometry, so guard is not a final ownership solution. |
| LAUNCHER-UX-001 | Один user launcher | VERIFIED ENTRY POINT | `START_HERE.cmd` працює як єдиний вхід; normal main menu реально відкрився. Інші RUN scripts лишаються internal. |
| CHAT-RUNTIME-001 | Hidden-by-default gameplay chat + `Y/U` channels | CODED_UNTESTED | `d0f7c323...`; окремий runtime chat owner, legacy persistent panel гаситься. |
| VEH-SPEED-RUNTIME-001 | Pickup 120 / BTR 90 speed contracts | CODED_UNTESTED | `c9ea15f4...`; server/standalone runtime enforcement, без claim `VERIFIED`. |
| MOUNTED-GUN-FALLBACK-001 | Не показувати Cube/Cylinder замість M2 | CODED_UNTESTED | `1370a101...`; exact M2 first, real R13 machine-gun fallback second, окремий fallback tag. |
| WEAPON-FALLBACK-PRESENTATION-001 | Real weapon fallback без стартової primitive паузи | CODED_UNTESTED | `6eeb4aec...`; fallback застосовується одразу у `OnWorldBeginPlay`, timer лишився для пізніх spawn. |
| BASE-SPAWN-RUNTIME-001 | BASE spawn перенесений із blockout edge у town corridor | CODED_UNTESTED | `45bf9fd5...`; ground snap + inward facing + two base candidates/team. |
| BASE-WEAPON-RACK-001 | 11 weapon classes біля actual BASE | CODED_UNTESTED | `45bf9fd5...`; один primary rack на команду, weapons dropped to world surface. |
| LEGACY-RECOVERED-BUILDING-001 | Прибрана недоказана raw-coordinate недобудова | CODED_UNTESTED | `d6c0ec4f...`, `e610cd5f...`; shell прибрано, detail owner disabled, forest-road assets лишені. |
| MUSEUM-LATE-CLEANUP-001 | Окремий 4.95 s museum cleanup owner вимкнений | CODED_UNTESTED | `b11a2133...`; cleanup залишається в основному museum build path, без другого late subsystem. |

## 5. Останній фактичний user run — 2026-08-21 17:44

Підтверджено runtime:
- splash Oster Conflict показується;
- є проміжковий чорний екран перед main menu;
- main menu повернулося і виглядає нормально;
- START має незрозумілий gray/double-START flow;
- gameplay запускається без попереднього `Pure virtual` crash;
- AK real first-person visual працює; reload animation + shot smoke є;
- player character real model уже видно;
- Silpo interior існує;
- pickup можна бачити/використовувати;
- BTR, pickup gun, weapon/world visuals, terrain, houses і landmark layout залишаються неприйнятними;
- великий team chat panel постійно видимий;
- distant flicker є;
- map largely flat, grass sparse;
- Museum/Silpo/Culture separation runtime не досягнута;
- stadium/photo-geography fidelity не досягнута.

Пакет доказів: user `Oster-photo-bag.docx`, 13 screenshots цього run.

## 6. Наступна черга

Порядок зафіксований user request і не переставляється:

1. Chat `Y/U` + pickup 120 / BTR 90 + real assets: source fixes зроблені частково; exact BTR-4/M2 заблоковані відсутніми source assets, решта чекає runtime acceptance.
2. Spawn/test contract: **source fix зроблено `45bf9fd5...`**, чекає runtime acceptance.
3. Museum/Silpo/Culture + confirmed legacy boxes: робота активна; 4.95 s museum cleanup owner і raw-coordinate recovered building pair уже прибрані. Далі прибрати 5.10/5.35 s late landmark rebuilds і завершити single-owner separation.
4. Stadion, terrain/relief, canonical houses, grass coverage.
5. Distant flicker після duplicate geometry removal.
6. Boot/travel polish: black gap + double-START confusion після закриття поточного gameplay/location priority.
7. Новий UE 5.8 playtest. Статуси підвищувати лише після runtime evidence.

**Заборона:** ніяких нових декоративних R15/R16 layers до закриття цього backlog.