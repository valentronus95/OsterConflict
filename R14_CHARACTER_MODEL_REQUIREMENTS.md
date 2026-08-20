# OSTER CONFLICT — R14 CHARACTER MODEL REQUIREMENTS

Оновлено: 2026-08-20
Гілка: `feat/r14-production-models`

Цей документ фіксує реальний стан персонажів і критерії Stage 2. Він не оголошує готовими візуали, яких у Content немає або які не пройшли UE 5.8 visual validation.

## 1. Поточна production-база

Поточний runtime має чотири visual faction archetypes:

- `UASpecialUnit` — UA Special Unit;
- `MaskedFighters` — Masked Fighters;
- `USRangers` — US Rangers Style;
- `Insurgents` — Insurgents.

Усі чотири зараз технічно використовують одну базу:

- third-person body: `/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter`;
- first-person arms: `/Game/QuantumCharacter/Mesh/Modules/SKM_Arms`.

Тому на поточному етапі **жодна faction не вважається production-унікальною по body/arms**. Це явно зафіксовано в `OCCharacterProductionProfiles.h/.cpp` через `bFactionUniqueBody=false` і `bFactionUniqueArms=false`.

## 2. Реальні gameplay-ролі

У authoritative `EOCPlayerRole` вже існують:

| Role | Поточне GearClass правило | Поточна проблема |
|---|---|---|
| `Rifleman` | `Standard`, з seeded шансом `Light` | немає власного гарантовано впізнаваного role-візуалу |
| `Medic` | `Standard` | візуально може збігатися зі Standard Rifleman |
| `Engineer` | `Heavy` | той самий базовий Heavy visual, що й Support |
| `Support` | `Heavy` | той самий базовий Heavy visual, що й Engineer |

R14 **не змінює баланс, loadout або мережеву роль** заради косметики. Role visual profile тільки описує зовнішній вигляд, який має відповідати вже реплікованому `AOCPlayerState::PlayerRole`.

## 3. Вже наявні QuantumCharacter assets

Перевірені production-кандидати:

| ID | Canonical object path | Type | Runtime зараз |
|---|---|---|---|
| Body | `/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter.SKM_QuantumCharacter` | Skeletal | так |
| BodyNoHead | `/Game/QuantumCharacter/Mesh/SKM_QuantumCharacter_NoHead.SKM_QuantumCharacter_NoHead` | Skeletal | ні, кандидат для modular head pass |
| Arms | `/Game/QuantumCharacter/Mesh/Modules/SKM_Arms.SKM_Arms` | Skeletal | так |
| Head | `/Game/QuantumCharacter/Mesh/Modules/SKM_Head.SKM_Head` | Skeletal | ні |
| BulletproofBeige | `/Game/QuantumCharacter/Mesh/Modules/SKM_Bulletproof_Bege.SKM_Bulletproof_Bege` | Skeletal | так |
| DropsBeige | `/Game/QuantumCharacter/Mesh/Modules/SKM_Drops_1_Bege.SKM_Drops_1_Bege` | Skeletal | так |
| HolsterHardBeige | `/Game/QuantumCharacter/Mesh/Modules/SKM_Holster_Hard_Bege.SKM_Holster_Hard_Bege` | Skeletal | так |
| Jeans | `/Game/QuantumCharacter/Mesh/Modules/SKM_Jeans.SKM_Jeans` | Skeletal | ні |
| BackPatch | `/Game/QuantumCharacter/Mesh/Modules/SKM_Patch_Back.SKM_Patch_Back` | Skeletal | ні |
| RolledUpBlueShirt | `/Game/QuantumCharacter/Mesh/Modules/SKM_Shirt_RolledUp_Blue.SKM_Shirt_RolledUp_Blue` | Skeletal | ні |
| CapBeige | `/Game/QuantumCharacter/Mesh/Modules/SM_Cap_Bege.SM_Cap_Bege` | Static | так |

Наявність asset у Content не означає автоматично, що він без clipping/skin-weight/material проблем сумісний з поточним runtime. Для unused assets це лише verified source candidate до UE visual pass.

## 4. Stage 2 target: faction differentiation

Перед merge кожна faction повинна мати production-approved візуальний набір, який:

1. не покладається лише на DisplayName або proxy tint;
2. читається по silhouette/gear/materials у third person;
3. не створює плутанини між opposing teams;
4. не ламає skeleton/AnimBP/PhysicsAsset;
5. підтримує death/downed/revive, crouch, sprint, jump/fall та vehicle entry;
6. не змінює authoritative gameplay properties;
7. має перевірений source/license для всіх зовнішніх assets.

Допускається спільний skeleton і навіть спільна технічна body-base, якщо faction реально відрізняється production-approved modules/materials/headgear. `bFactionUniqueBody` не треба штучно ставити `true`, якщо відмінність зроблена gear/material layer.

## 5. Stage 2 target: role differentiation

Rifleman, Medic, Engineer і Support мають бути візуально розпізнавані **всередині своєї faction**, але role differentiation не повинна перекривати team/faction readability.

Мінімальний acceptance:

- `Medic`: окремий production-approved medical identifier/gear set, не лише назва у UI;
- `Engineer`: окремий tool/utility silhouette або gear combination;
- `Support`: окремий heavy/ammo-support silhouette або gear combination, не тотожний Engineer;
- `Rifleman`: базовий combat silhouette з дозволеною seeded Light/Standard варіативністю.

Конкретні кольори/символи не вважаються затвердженими до фактичного art pass. R14 не вигадує їх у коді без asset evidence.

## 6. Animation requirements

Поточна production character subsystem має базові locomotion sequences idle/walk/run/fall. `UOCCharacterVisualProfile` уже передбачає Fire/Reload/Revive/Downed/Death montage slots, але Stage 2 не вважається завершеним, доки production character set не перевірений у всіх цих станах.

Обов'язково перевірити:

- locomotion skeleton compatibility;
- upper-body weapon pose та hand alignment;
- fire/reload interaction з R14 weapon presentation;
- downed/revive/death transitions;
- gear не відривається і не залишається у старому pose;
- first-person arms не дублюються у third person;
- owner/no-owner visibility коректна у multiplayer.

## 7. Source / automation gates

R14 додає:

- `OCCharacterProductionProfiles.h/.cpp` — code-level faction, role та module registry;
- `OCCharacterProductionProfileTests.cpp` — UE automation contract для 4 factions, 4 authoritative roles, body/arms та audited modules;
- `.github/workflows/r14-character-model-contracts.yml` — source-level regression gate.

Source CI не дорівнює UE runtime validation. Фактичний UE 5.8 compile/runtime/visual/cook pass буде виконано пізніше одним консолідованим запуском на ноутбуці разом з іншими R14/локаційними змінами.

## 8. Definition of Done для Stage 2

Stage 2 можна закрити лише коли:

- 4/4 faction profiles production-distinct у фактичній грі;
- 4/4 gameplay roles мають чітку role readability;
- Engineer і Support більше не виглядають одним і тим самим Heavy preset;
- всі використані modules/materials мають exact canonical paths і source/license status;
- skeleton/material/LOD/PhysicsAsset validation пройдені;
- locomotion/combat/downed/death/vehicle states перевірені;
- multiplayer owner/remote visibility перевірена;
- Windows UE 5.8 compile/runtime/visual/cook PASS;
- `R14_MODEL_REGISTRY.md` та `R14_MODEL_INTEGRATION_HISTORY.md` синхронізовані з фактичним результатом.
