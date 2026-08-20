# OSTER CONFLICT — R14 WEAPON ANIMATION REQUIREMENTS

Оновлено: 2026-08-20
Гілка: `feat/r14-production-models`

Цей файл визначає, яких саме animation assets бракує після інвентаризації поточного Content. Він не є списком випадкових Marketplace/Fab пакетів: новий asset приймається тільки якщо закриває конкретну прогалину, має зрозумілу ліцензію і може бути retargeted/attached без заміни authoritative gameplay logic.

## Вже підтверджено

- AK-47 має окремі weapon animation sequences `AK-47_Fire_W` і `AK-47_Reload_W`, уже підключені в `UOCFirstPersonWeaponPresentationSubsystem`.
- `SampleAnimationPack/Animations/Rifle` дає rifle idle/ADS/walk fallback, але в перевіреному каталозі немає named Fire/Reload sequences.
- Перевірені R13 Stein weapon folders містять skeletal meshes/accessories, але не окремі weapon Fire/Reload animation assets.
- Remington 870 і M249 production visuals зараз static meshes, тому реальний pump/bolt/belt/magazine movement потребує articulated/skeletal replacement або окремих рухомих компонентів.
- `OC_RPG1` тепер має Kenney CC0 `rocketlauncherModern` static production visual, але first-person grip/reload presentation ще не калібровані.
- R14 тепер має code-level animation coverage registry: `OCWeaponAnimationProfiles.h/.cpp`. У ньому оголошені всі 11 поточних weapon ID; порожній asset path означає реальну відсутність authored animation, а не мовчазний fallback.
- `OCWeaponPresentationProfileTests.cpp` перевіряє 11/11 grip + animation profile declarations, канонічні AK Fire/Reload paths, завантаження оголошених animation assets і skeleton compatibility AK-47.
- `.github/workflows/r14-weapon-profile-contracts.yml` захищає цю матрицю на source-CI. Це не заміняє UE 5.8 runtime/visual validation, яку свідомо відкладаємо до спільного ноутбучного прогону після подальшого доопрацювання гілок.

## Обов'язкова матриця

| Weapon ID | Модель | Мінімальний animation set | Стан |
|---|---|---|---|
| `OC_AR1` | AK-47 | fire, reload, idle/ADS, recoil integration | fire/reload present; code contract present; visual timing validation pending |
| `OC_SMG1` | MP5 | fire, ADS fire, tactical reload, empty reload, charge/bolt if mesh supports | MISSING; explicit empty code profile |
| `OC_PST1` | M1911 | fire, ADS fire, tactical reload, empty reload, slide/chamber | MISSING; explicit empty code profile |
| `OC_SNP1` | M700 | fire, bolt cycle, reload, ADS transition/idle | MISSING; explicit empty code profile |
| `OC_SG1` | Remington 870 | fire, pump cycle, shell insert loop, reload start/end, empty state | MISSING; current visual static; articulated requirement encoded |
| `OC_LMG1` | M249 | fire/bolt, belt or feed movement, box/belt reload, empty reload | MISSING; current visual static; articulated requirement encoded |
| `R13_M14` | M14 | fire, ADS fire, tactical/empty reload | MISSING; explicit empty code profile |
| `R13_MAC10` | MAC-10 | fire, reload, empty reload, bolt/charging if supported | MISSING; explicit empty code profile |
| `R13_TEC9` | TEC-9 | fire, reload, empty reload/bolt if supported | MISSING; explicit empty code profile |
| `R13_LEVER4570` | Lever Action .45-70 | fire, lever cycle, per-round reload or weapon-authentic reload | MISSING; explicit empty code profile |
| `OC_RPG1` | Kenney rocketlauncherModern | shoulder idle/ADS, fire recoil/backblast presentation, reload/equip | MISSING; explicit empty code profile |

## Arms / character animation requirements

Для кожної категорії weapon animation має існувати узгоджений hands/arms pass:

1. right-hand grip + trigger alignment;
2. left-hand support position;
3. magazine/pump/bolt/lever interaction;
4. hip idle;
5. ADS idle;
6. fire reaction;
7. reload hands;
8. sprint/low-ready;
9. jump/fall/land compatibility;
10. equip/unequip;
11. interruption on weapon switch, death and vehicle entry;
12. third-person reload/fire representation or compatible upper-body montage.

## Правила придбання / завантаження

- Не купувати/завантажувати великий animation pack тільки через кількість animations.
- До імпорту підтвердити: license, Unreal Engine version, skeleton/rig, source format, retargetability, first-person arms compatibility.
- Перевага asset-у, який покриває кілька наших реальних категорій: rifle + pistol + SMG + shotgun + sniper.
- Пакет для Epic Mannequin/Manny не вважається plug-and-play для `QuantumCharacter`; потрібен retarget test.
- Weapon skeletal animation і arms animation перевіряються окремо. Сумісна анімація рук не означає сумісність із skeleton конкретної зброї.
- Не підміняти gameplay reload timing косметичною анімацією. Animation Notifies повинні синхронізувати presentation з уже authoritative ammo/reload logic.
- Нові animation assets тримати в контрольованому namespace і не розкидати по root Content.
- Новий animation path спочатку вноситься в `OCWeaponAnimationProfiles.cpp`, після чого проходить automation/runtime validation. Не хардкодити нові Fire/Reload paths у випадкових subsystem/class файлах.

## Acceptance gate для нового animation asset

Новий набір допускається до R14 лише після:

- source/license verified;
- exact skeleton/retarget target documented;
- fire/reload sequences load in UE 5.8;
- no skeleton mismatch warnings;
- hands do not clip through production weapon mesh in FP;
- remote third-person representation remains valid;
- weapon switch/reload interruption restores state;
- dedicated server remains cosmetic-asset independent;
- cook/package succeeds;
- entry added to `R14_MODEL_REGISTRY.md` and `R14_MODEL_INTEGRATION_HISTORY.md`.
