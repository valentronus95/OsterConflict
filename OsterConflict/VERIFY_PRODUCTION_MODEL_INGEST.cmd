@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "PROJECT_DIR=%~dp0"
set "REPO_DIR=%PROJECT_DIR%.."
set "FAILED=0"

pushd "%REPO_DIR%" || exit /b 2

where git >nul 2>nul || (
    echo ERROR: git.exe is not available in PATH.
    popd
    exit /b 3
)

rem Source files must be real local binaries, not tiny unsmudged LFS pointer files.
call :require_local_binary "OsterConflict\SourceAssets\Production\Vehicles\HMMWV\ukrainian_hmmwv_mk_19.glb" 1024
call :require_local_binary "OsterConflict\SourceAssets\Production\Weapons\M2\m2_50cal_machinegun_cc0.glb" 1024
call :require_local_binary "OsterConflict\SourceAssets\Production\Vehicles\BTR4\BTR4_Bucephalus.fbx" 1024
call :require_local_binary "OsterConflict\SourceAssets\Production\Vehicles\BTR4\Textures\Bahnya_low_albedo.png" 1024
call :require_local_binary "OsterConflict\SourceAssets\Production\Vehicles\BTR4\Textures\Koleso_low_albedo.png" 1024
call :require_local_binary "OsterConflict\SourceAssets\Production\Vehicles\BTR4\Textures\Korpus_low_albedo.png" 1024
call :require_local_binary "OsterConflict\SourceAssets\Production\Vehicles\BTR4\Textures\Windows_low_albedo.png" 1024
call :require_local_binary "OsterConflict\SourceAssets\Production\Vehicles\BTR4\Textures\interior.png" 1024
call :require_local_binary "OsterConflict\SourceAssets\Production\Vehicles\BTR4\Textures\tire.png" 1024

rem Canonical Unreal products must exist after the Editor import.
call :require_local_binary "OsterConflict\Content\Production\Vehicles\HMMWV\SM_HMMWV_UA.uasset" 1024
call :require_local_binary "OsterConflict\Content\Production\Weapons\M2\SM_M2_Browning.uasset" 1024
call :require_local_binary "OsterConflict\Content\Production\Vehicles\BTR4\SM_BTR4_Bucephalus.uasset" 1024

rem Every production binary extension used by this ingest must resolve to the LFS filter.
call :require_lfs_attr "OsterConflict\SourceAssets\Production\Vehicles\HMMWV\ukrainian_hmmwv_mk_19.glb"
call :require_lfs_attr "OsterConflict\SourceAssets\Production\Weapons\M2\m2_50cal_machinegun_cc0.glb"
call :require_lfs_attr "OsterConflict\SourceAssets\Production\Vehicles\BTR4\BTR4_Bucephalus.fbx"
call :require_lfs_attr "OsterConflict\SourceAssets\Production\Vehicles\BTR4\Textures\Korpus_low_albedo.png"
call :require_lfs_attr "OsterConflict\Content\Production\Vehicles\HMMWV\SM_HMMWV_UA.uasset"
call :require_lfs_attr "OsterConflict\Content\Production\Weapons\M2\SM_M2_Browning.uasset"
call :require_lfs_attr "OsterConflict\Content\Production\Vehicles\BTR4\SM_BTR4_Bucephalus.uasset"

rem Source binaries are committed before the Unreal import. Verify Git stores LFS pointers, not raw binaries.
call :require_head_lfs_pointer "OsterConflict/SourceAssets/Production/Vehicles/HMMWV/ukrainian_hmmwv_mk_19.glb"
call :require_head_lfs_pointer "OsterConflict/SourceAssets/Production/Weapons/M2/m2_50cal_machinegun_cc0.glb"
call :require_head_lfs_pointer "OsterConflict/SourceAssets/Production/Vehicles/BTR4/BTR4_Bucephalus.fbx"
call :require_head_lfs_pointer "OsterConflict/SourceAssets/Production/Vehicles/BTR4/Textures/Bahnya_low_albedo.png"
call :require_head_lfs_pointer "OsterConflict/SourceAssets/Production/Vehicles/BTR4/Textures/Koleso_low_albedo.png"
call :require_head_lfs_pointer "OsterConflict/SourceAssets/Production/Vehicles/BTR4/Textures/Korpus_low_albedo.png"
call :require_head_lfs_pointer "OsterConflict/SourceAssets/Production/Vehicles/BTR4/Textures/Windows_low_albedo.png"
call :require_head_lfs_pointer "OsterConflict/SourceAssets/Production/Vehicles/BTR4/Textures/interior.png"
call :require_head_lfs_pointer "OsterConflict/SourceAssets/Production/Vehicles/BTR4/Textures/tire.png"

rem Unreal assets are staged immediately before this validator runs. Verify the index contains LFS pointers.
call :require_index_lfs_pointer "OsterConflict/Content/Production/Vehicles/HMMWV/SM_HMMWV_UA.uasset"
call :require_index_lfs_pointer "OsterConflict/Content/Production/Weapons/M2/SM_M2_Browning.uasset"
call :require_index_lfs_pointer "OsterConflict/Content/Production/Vehicles/BTR4/SM_BTR4_Bucephalus.uasset"

if not "%FAILED%"=="0" (
    echo.
    echo ============================================================
    echo FAILED: production model ingest verification found errors.
    echo Do not mark the production-model PR ready for merge.
    echo ============================================================
    popd
    exit /b 1
)

echo.
echo ============================================================
echo PASS: production source binaries, Unreal assets and LFS pointers verified.
echo ============================================================
popd
exit /b 0

:require_local_binary
set "CHECK_PATH=%~1"
set "MIN_SIZE=%~2"
if not exist "%CHECK_PATH%" (
    echo ERROR: missing file: %CHECK_PATH%
    set "FAILED=1"
    goto :eof
)
for %%F in ("%CHECK_PATH%") do set "CHECK_SIZE=%%~zF"
if !CHECK_SIZE! LSS !MIN_SIZE! (
    echo ERROR: file is suspiciously small ^(!CHECK_SIZE! bytes^): %CHECK_PATH%
    set "FAILED=1"
) else (
    echo PASS: local binary !CHECK_SIZE! bytes: %CHECK_PATH%
)
goto :eof

:require_lfs_attr
set "CHECK_PATH=%~1"
set "ATTR_VALUE="
for /f "tokens=3" %%A in ('git check-attr filter -- "%CHECK_PATH%" 2^>nul') do set "ATTR_VALUE=%%A"
if /I not "!ATTR_VALUE!"=="lfs" (
    echo ERROR: Git LFS filter is not active for: %CHECK_PATH%
    set "FAILED=1"
) else (
    echo PASS: LFS attribute: %CHECK_PATH%
)
goto :eof

:require_head_lfs_pointer
set "CHECK_PATH=%~1"
git show "HEAD:%CHECK_PATH%" 2>nul | findstr /C:"version https://git-lfs.github.com/spec/v1" >nul
if errorlevel 1 (
    echo ERROR: HEAD does not contain an LFS pointer for: %CHECK_PATH%
    set "FAILED=1"
) else (
    echo PASS: HEAD LFS pointer: %CHECK_PATH%
)
goto :eof

:require_index_lfs_pointer
set "CHECK_PATH=%~1"
git show ":%CHECK_PATH%" 2>nul | findstr /C:"version https://git-lfs.github.com/spec/v1" >nul
if errorlevel 1 (
    echo ERROR: staged index does not contain an LFS pointer for: %CHECK_PATH%
    set "FAILED=1"
) else (
    echo PASS: staged LFS pointer: %CHECK_PATH%
)
goto :eof
