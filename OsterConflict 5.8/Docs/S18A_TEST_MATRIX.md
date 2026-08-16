# S18A — Test matrix

1. **8 population / Balanced** — сервер `Population=8`, перевірити AI/objectives/vehicles, `PerfReport`, 10 хв без накопичення actors.
2. **16 population / Balanced** — 16 total, 20 хв, Conquest round restart x2.
3. **16 population / LowCPU** — порівняти server frame time і поведінку AI з Balanced.
4. **8 humans + 8 bots** — люди витісняють filler bots, немає population oscillation.
5. **16 humans** — filler bots = 0, 17-й human відхиляється `SERVER_FULL_HUMANS`.
6. **Corpse stress** — багато смертей/revive; cap відповідає profile, respawn не видаляє свіжий corpse миттєво.
7. **Door stress** — 50+ дверей idle: tick disabled; під час open/close tick активний і після завершення знову off.
8. **Ambient dedicated server** — AOCAmbientAudioZone tick off на server, ambience працює на clients.
9. **Destruction stress** — windows/destructibles не створюють довгоживучий replicated debris.
10. **Vehicle stress** — усі vehicle spawns + driver/gunner, `stat net`, відсутній runaway traffic.
11. **Networking Insights** — trace 8 і 16 population; знайти top Actors/RPC/Properties, зберегти baseline до наступних змін.
12. **Timing Insights** — перевірити CPU spikes AI, physics, UI та map construction.
13. **CSV capture** — 60+ секунд активного бою; порівняти повторні збірки однаковим сценарієм.
14. **Frontend/settings regression** — S17A/S17B UI не ламається після perf changes.
15. **ZIP extract regression** — запустити всі `VERIFY_S04.py` … `VERIFY_S18A.py` із розпакованого milestone.
