# S18C test matrix

1. Source preflight passes outside Unreal.
2. Every UHT header has the exact generated include and it is last.
3. Every Server/Client/NetMulticast UFUNCTION has an `_Implementation` body.
4. Game/Editor/Client/Server target files exist.
5. Build.cs contains required runtime/UI/AI modules.
6. All Python scripts compile with host Python.
7. Source archive has no Binaries/Intermediate/Saved/DDC.
8. Windows preflight rejects an engine that is not UE 5.8.
9. Windows preflight verifies UBT/UAT/UnrealEditor-Cmd and checks MSVC when `vswhere` is present.
10. First build errors can be classified as UHT/C++/LINK/COOK/PACKAGE/RUNTIME by `ANALYZE_BUILD_LOG.py`.
11. S18B build/package scripts remain present and unchanged as the actual build path.
12. Full S04-S18C verifier suite passes from an extracted ZIP copy.
