# R11.1 Launcher fix

Fixed two first-run regressions found on the real Launcher UE 5.8 installation:

1. `Run-Logged` used a parameter named `$Args`, colliding with PowerShell's automatic `$args` variable and allowing a child `powershell.exe` to open interactively instead of receiving its arguments. It is now `$ArgumentList`.
2. `PRELAUNCH_CHECK.ps1` still required source-build-only `RunUBT.bat`. Installed/Launcher UE 5.8 is now detected and validated with `Build.bat`, `RunUAT.bat`, and `UnrealEditor-Cmd.exe`; `RunUBT.bat` is required only for source builds.

Expected first-run flow: `START_HERE.cmd` -> `1` -> prelaunch PASS/WARN output -> toolchain preflight -> Compile Editor -> Compile Game -> `RESULT: PASS for Mode=Compile`.
