# Windows toolchain для S18B

S18B dedicated-server workflow розрахований на **UE 5.8 source build**, бо project має окремий `OsterConflictServer.Target.cs`.

Не фіксуй вручну версії MSVC/Windows SDK у ТЗ: Unreal 5.8/Turnkey/UBT повинен сам підтвердити сумісний toolchain на build-машині. Це захищає проєкт від застарілого номера SDK у документації.

Очікувана структура:

```text
%UE_ROOT%\Engine\Build\BatchFiles\RunUBT.bat
%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat
%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe
```

Build pipeline навмисно завершується помилкою, якщо ці файли відсутні.
