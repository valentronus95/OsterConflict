# S18A — Packaging readiness / blockers

## Уже є
- Game / Editor / Client / Server targets.
- Windows target platform у `.uproject`.
- Dedicated-server scripts і direct-connect client flow.
- Source-only gameplay map constructor.
- Release QA scripts/verifiers.

## До першого справжнього RC build обов'язково
1. Встановити Unreal Engine 5.8 toolchain / сумісний source build для Server target.
2. Відкрити проєкт і виконати повну C++/UHT компіляцію.
3. Створити project-owned map `.umap`; `/Engine/Maps/Entry` не є фінальною картою продукту.
4. Перенести production open-world content у World Partition level.
5. Створити HLOD Layers і реально збудувати HLOD commandlet/editor build.
6. Додати production collision/NavMesh bounds/data layers.
7. Імпортувати ліцензовані/оригінальні character/weapon/vehicle/audio/VFX assets.
8. Зробити cook audit: missing references, redirectors, unused assets, shader compile errors.
9. Package Development Client + Server; тільки після проходження QA робити Shipping client.
10. Запустити 8/16 load tests у packaged builds, а не лише PIE.

## Release gate
Немає релізного статусу, якщо хоча б один пункт нижче не пройдений:
- clean compile;
- clean cook;
- client/server connect;
- 2 complete Conquest rounds;
- Sandbox smoke test;
- 16-population soak test;
- Networking Insights baseline;
- crash-free 30 minute test;
- settings persistence;
- no missing/copyright-unclear production assets.
