# S18A — Optimization / QA / Release Preparation

Статус: source milestone.

## Мета
S18A не додає новий режим гри. Він переводить існуючий прототип у стан, який можна профілювати, навантажувати та готувати до першої реальної Windows-компіляції.

## Реалізовано
- Server performance profiles через URL option `?PerfProfile=LowCPU|Balanced|Quality`.
- Профіль не змінює правила матчу або максимальну кількість людей; він керує лише допустимою ціною AI/corpse presentation.
- AI think interval масштабується серверним profile multiplier.
- Balanced corpse cap знижено до 16, LowCPU — до 10, Quality — до 20.
- `PerfReport` через PlayerController: клієнт просить authoritative snapshot у сервера, сервер логгує та повертає звіт власнику.
- Static/slow replicated actors отримали нижчі NetUpdateFrequency / MinNetUpdateFrequency та NetCullDistanceSquared.
- Door tick тепер event-driven: двері не тикають щокадру, коли вже стоять у відкритому/закритому положенні.
- Ambient zones тикають 5 разів/с замість кожного кадру; на dedicated server tick повністю вимикається.
- Capture point replication frequency обмежена окремо від її серверного capture tick.
- Додані scripts для 8/16 population, LowCPU server і Unreal Insights trace запуску.
- Доданий source/release tree audit та повний runner попередніх verifier-ів.

## Що навмисно НЕ оголошено завершеним
- World Partition/HLOD не можуть бути фактично збудовані без project-owned `.umap` і Unreal Editor. Поточний source-only world proxy вже використовує instancing, але це не заміна HLOD build.
- Replication Graph не вмикається навмання. Для 8–16 гравців спочатку треба зняти Networking Insights trace; лише якщо relevancy/actor gathering стане bottleneck, підключати Replication Graph.
- Фінальні LOD/Nanite/texture streaming budgets залежать від справжніх art assets, яких у source milestone ще немає.
- Ніяких “60 FPS confirmed” без packaged build і профілювання на реальній машині.

## Команди QA
У консолі клієнта:

`PerfReport`

Сервер поверне рядок на кшталт:

`PERF [Balanced] Humans=4 Bots=12 Characters=16 Vehicles=12 Capture=3 ... CorpseBudget=16 AIThinkScale=1.00`

Для UE built-in profiling використовувати Unreal Insights / Networking Insights, `stat unit`, `stat fps`, `stat net`, а для CSV capture — `CsvProfile Start` / `CsvProfile Stop`.
