# S17A — Rich UI / Lobby / Deployment / Chat

## Статус
Source milestone. Реалізований UMG/C++ frontend без обов'язкових Widget Blueprint `.uasset`.

## Що додано
- C++ `UOCGameUIRootWidget`, який створює WidgetTree у runtime.
- `-Frontend` режим для direct-connect екрана.
- Username + IP:Port connect.
- Escape: frontend/menu overlay.
- Deployment: Team 1/2, роль, squad, Base/A/B/C spawn, Ready/Deploy.
- Серверна перевірка team switching; після deploy mid-life team hop заборонений.
- Запит на Base/A/B/C респавн із safe fallback, якщо forward point втрачено/заблоковано.
- TAB: UMG scoreboard з Human/Bot, squad, role, K/D/R/Score/Ping.
- T: chat input; ALL/TEAM/SQUAD channel cycle; Enter send.
- F10 Sandbox: UMG admin panel поверх існуючого server-authoritative admin backend.
- Старий Canvas HUD лишився fallback; багаті UI-панелі не дублюються, якщо UMG root активний.

## Обмеження
- Це не фінальний арт UI. Немає фінальних іконок, картографічної текстури, анімацій меню, локалізації та controller/gamepad navigation polish.
- Server browser/matchmaking не входить у S17A; direct IP/port залишається першою мережевою моделлю.
- Audio/graphics/control settings будуть у S17B.
- Реальна компіляція UE 5.8 у цьому середовищі недоступна; milestone пройшов source/static verification.
