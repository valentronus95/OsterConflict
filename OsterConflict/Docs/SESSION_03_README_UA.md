# Oster Conflict — Session 03
## Multiplayer Test Harness / PlayerState / Scoreboard

### Що додано

- `AOCPlayerState`
  - replicated `Kills`;
  - replicated `Deaths`;
  - стандартний replicated `Score` через `APlayerState`;
  - ping через `GetPingInMilliseconds()`.
- `AOCPlayerController`
  - TAB scoreboard;
  - `ConnectToServer <IP:PORT>`;
  - `SetNickname <NAME>`;
  - `DisconnectFromServer`;
  - окремий Enhanced Input Mapping Context, який працює навіть коли Pawn тимчасово відсутній.
- `AOCGameMode`
  - `PlayerControllerClass = AOCPlayerController`;
  - `PlayerStateClass = AOCPlayerState`;
  - join/leave logging;
  - nickname з travel option `?Name=`;
  - серверний облік kills/deaths/score;
  - kill = +100 score;
  - suicide/environmental death не дає kill іншому гравцю.
- `AOCHUD`
  - TAB таблиця;
  - PLAYER / K / D / SCORE / PING;
  - сортування за kills, потім score, потім deaths;
  - локальний гравець підсвічується;
  - локальний ping також показується у верхньому правому куті.
- `UOCHealthComponent`
  - сервер запам'ятовує останнього damage instigator для attribution смерті.

### Керування

- `TAB` — тримати таблицю результатів.
- Решта керування залишається з S02.

### Консольні команди прототипу

Відкрити консоль Unreal (`~`) і використати:

```text
ConnectToServer 127.0.0.1:7777
SetNickname Alpha
DisconnectFromServer
```

Для прямого URL-підключення:

```text
open 127.0.0.1:7777?Name=Alpha
```

### Тест на 2 клієнти в Editor

У Play settings:

1. Number of Players = 2 або більше.
2. Net Mode = Play As Client.
3. Увімкнути окремий dedicated server.
4. Запустити PIE.
5. На обох клієнтах перевірити TAB.
6. Підстрелити одного гравця іншим і перевірити K/D/Score після respawn.

### Тест packaged Client/Server

Після Development Server / Development Client build Unreal розміщує виконувані файли у `Binaries/Win64`. Далі:

```text
Scripts/RUN_S03_SERVER_7777.bat
Scripts/RUN_S03_CLIENT_ALPHA.bat
Scripts/RUN_S03_CLIENT_BRAVO.bat
```

Dedicated server за замовчуванням використовує порт 7777; у скрипті порт вказаний явно.

### Межі S03

Ця сесія ще НЕ додає:

- команди;
- Conquest;
- squad logic;
- kill feed;
- assists;
- matchmaking/server browser;
- Steam/EOS sessions;
- persistence статистики.

Це наступні модулі, а не причина перетворювати S03 на нескінченну яму.
