OSTER CONFLICT — R11 VISUAL FOUNDATION — UE 5.8

ЦЕ ПОВНИЙ АРХІВ R11 ДЛЯ ТЕСТУ НА WINDOWS.
Не накладай його поверх R10. Розпакуй R11 в окрему нову папку.

ПЕРШИЙ ЗАПУСК
1) Закрий старий OsterConflict в Unreal Editor.
2) Розпакуй архів, наприклад у D:\OsterConflict_R11\
3) Запусти START_HERE.cmd.
4) Обери 1 — Compile Editor/Game with installed UE 5.8.
   Скрипт автоматично шукає Launcher UE 5.8 у C:\Program Files\Epic Games\UE_5.8.
5) Після BUILD SUCCESSFUL знову відкрий START_HERE.cmd.
6) Обери 4 — Launch R11 local listen-server visual test.
   На чистому архіві пункт 4 сам створить OsterConflict_Runtime.umap і запустить локальний listen server.

ЩО ВИПРАВЛЕНО В R11
- runtime daylight: сонце, SkyAtmosphere, SkyLight і fog;
- освітлення реплікується клієнтам;
- source-only світ отримав семантичні кольори/матеріали замість суцільної сіро-чорної сцени;
- біля обох баз додано видиму тестову геометрію: дорога, укриття, огорожа, покриття;
- authoring/reference markers приховані під час гри;
- зброя більше не один Cube: з базових UE мешів збирається впізнавана форма автомата/пістолета/SMG/LMG/снайперської/рушниці/launcher;
- постріл, трасер і impact більше не малюються DrawDebug кубиками/лініями; використовуються короткоживучі scene FX і світло;
- source-only руки/кисті та proxy-тіло стали менш кубічними;
- транспорт отримав базову військову/цивільну палітру;
- Full validation працює через Build.bat і розрізняє Launcher UE та source build;
- dedicated-server compile/package на Launcher UE не видається за підтримуваний: для Launcher використовується listen-server тест.

ВАЖЛИВО
R11 — це VISUAL FOUNDATION для source-only прототипу. У ньому немає стороннього AAA asset pack, фотограмметрії Остра або готових ліцензованих моделей зброї. Завдання R11 — прибрати чорний екран, debug-примітиви та однотипні куби, зберігши існуючу gameplay/network логіку як основу для наступного art/content проходу.

ЯКЩО КОМПІЛЯЦІЯ ВПАЛА
Не копіюй Binaries/Intermediate із R10. Запусти START_HERE -> 3 для clean validation або передай останній лог/TEST_RESULTS ZIP для аналізу.

ДЛЯ FULL VALIDATION
Успішна Automation-перевірка додатково підтверджується файлом PC_TEST\TEST_RESULTS\<timestamp>\AutomationReports\index.json.
