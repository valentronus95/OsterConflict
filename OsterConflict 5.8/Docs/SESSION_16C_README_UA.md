# S16C — Character / faction / animation source milestone

## Що реалізовано

S16C створює арт- і анімаційний каркас для персонажів без залежності від готових сторонніх `.uasset`.

- 4 візуальні архетипи фракцій: UA Special Unit, Masked Fighters, US Rangers Style, Insurgents.
- У матчі активні тільки 2 фракції, які вибираються сервером: `?Team1Faction=UA?Team2Faction=Masked`.
- `PlayerState` реплікує faction + deterministic appearance seed.
- Однаковий backend працює для людей і AI-ботів.
- `UOCCharacterVisualProfile` — DataAsset-схема для third-person body, FPS arms, AnimBP, gear і montage assets.
- `UOCCharacterVisualComponent` — presentation layer, що застосовує профіль і відтворює one-shot animation events.
- `UOCCharacterAnimInstance` — C++-міст для locomotion / ADS / reload / downed / death / vehicle параметрів.
- FPS arms мають окремий SkeletalMeshComponent.
- До імпорту production art існує source-only proxy body + proxy FPS arms із Engine BasicShapes.
- Фракції відрізняються proxy tint, а варіанти спорядження детермінуються seed.
- Fire / reload / revive / downed / death мають мережеві cosmetic event hooks.

## Що ще НЕ є фінальним

- Немає production skeletal meshes людей.
- Немає готових рук зі шкірою/рукавицями та weapon poses.
- Немає фінального Animation Blueprint / Blend Space / Aim Offset assets.
- Немає IK Rig / IK Retargeter assets і foot/hand IK pass.
- Немає production cloth/hair simulation.
- Proxy персонаж є лише технічною заміною, щоб у source-only білді не було невидимих Pawn.

## Ціль наступного content pass

Імпорт одного спільного humanoid skeleton, чотирьох modular character families, FPS arms, animation library і Animation Blueprint, після чого source-only proxy автоматично вимикається профілем.
