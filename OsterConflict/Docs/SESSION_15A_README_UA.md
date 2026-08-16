# Oster Conflict — S15A Weapon Audio

Статус: source milestone.

S15A додає мережевий weapon-audio framework без включення чужих аудіофайлів. Сервер підтверджує постріл/перезарядку/стан, після чого клієнти локально вибирають near/indoor/distant/ballistic presentation.

## Реалізовано
- UOCWeaponAudioProfile (PrimaryDataAsset) для звукових палітр.
- UOCWeaponAudioComponent на кожній зброї.
- Outdoor / SemiIndoor / Indoor класифікація джерела.
- Near report / suppressed report / distant tail.
- Локальна механіка затвора/спуску.
- Reload start/end/cancel, dry-fire, fire-mode switch, equip/drop hooks.
- Bullet crack біля слухача через найближчу точку hitscan-сегмента.
- Surface impact audio для Flesh/Glass/Wood/Metal/Masonry/Dirt.
- Dedicated server нічого не програє.
- `oc.Audio.Debug 1` показує аудіо-події в development build.

## Важливо
S15A не містить фінальних SoundWave/MetaSound assets. Профілі звуку мають бути заповнені оригінальними, власно згенерованими або належно ліцензованими файлами. Аудіо Arma/Battlefield не копіюється.
