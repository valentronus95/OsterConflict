# S18B First Build / RC Gate

## Gate A — Toolchain
- UE 5.8 source build доступний через `UE_ROOT`.
- `RunUBT.bat`, `RunUAT.bat`, `UnrealEditor-Cmd.exe` існують.
- Windows SDK / MSVC toolchain приймається Unreal Turnkey/UBT.

## Gate B — Compile
- `OsterConflictEditor Win64 Development` PASS.
- UHT не має fatal/reflection errors.
- `OsterConflictClient` PASS.
- `OsterConflictServer` PASS.

## Gate C — Project-owned map
- Editor script створює `/Game/Maps/OsterConflict_Runtime`.
- `.umap` знаходиться в `Content/Maps`.
- Runtime content, як і раніше, будується серверним `AOCGameMode`, поки production art map не замінить source-only world constructor.

## Gate D — Automation
Командний test set: `OsterConflict.Release`.
Мінімум:
- build fingerprint;
- georeference origin;
- faction display names.

## Gate E — Cook / package
- Windows Client Development BuildCookRun PASS.
- Windows Dedicated Server Development BuildCookRun PASS.
- `.exe` та `.pak`/IoStore containers існують.
- post-build SHA-256 manifest створений.

## Gate F — Packaged smoke
- Server запускається на 7777.
- Два Client process живуть після connect/warm-up.
- `?AutoDeploy=1` використовується лише для smoke automation.
- Немає Fatal error / Assertion failed у зібраних логах.

## Gate G — RC acceptance після першої збірки
До наступного milestone обов'язково записати фактичні, а не оцінені:
- client package size;
- server package size;
- clean compile duration;
- cook/package duration;
- startup time;
- 2-client smoke result;
- 8/16 population soak result;
- peak RAM server/client;
- average server frame time;
- net bandwidth baseline;
- crash/assert count.
