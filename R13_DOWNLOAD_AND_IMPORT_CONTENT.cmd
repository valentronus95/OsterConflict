@echo off
setlocal EnableExtensions
cd /d "%~dp0"

if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "EDITOR_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"
set "IMPORT_SCRIPT=%~dp0OsterConflict\Scripts\R13\IMPORT_R13_CONTENT.py"
set "RAW=%~dp0OsterConflict\Content\Raw\R13"
set "WEAPONS=%RAW%\Weapons\Kenney"
set "UI=%RAW%\UI"
set "CACHE=%TEMP%\OsterConflict_R13_CC0"

 echo ============================================================
 echo OSTER CONFLICT R13 - DOWNLOAD + IMPORT CONTENT
 echo ============================================================
 echo This downloads selected CC0 Kenney weapon assets and one
 echo licensed Wikimedia Commons Oster photo, then imports them to UE.
 echo.

where git >nul 2>nul || (
  echo [ERROR] Git not found.
  pause
  exit /b 2
)
where git-lfs >nul 2>nul || (
  echo [ERROR] Git LFS not found.
  pause
  exit /b 3
)
if not exist "%EDITOR_CMD%" (
  echo [ERROR] UnrealEditor-Cmd.exe not found at %EDITOR_CMD%
  pause
  exit /b 4
)

if exist "%CACHE%" rmdir /s /q "%CACHE%"
mkdir "%WEAPONS%" 2>nul
mkdir "%UI%" 2>nul

set "GIT_LFS_SKIP_SMUDGE=1"
echo [1/4] Cloning public CC0 asset source...
git clone --depth 1 --filter=blob:none https://github.com/series-ai/jam-ready-assets.git "%CACHE%"
if errorlevel 1 goto :fail

 echo [2/4] Downloading selected weapon models through Git LFS...
git -C "%CACHE%" lfs pull --include="3D/weapons/kenney-weapon-pack/Models/OBJ format/machinegun.obj,3D/weapons/kenney-weapon-pack/Models/OBJ format/machinegun.mtl,3D/weapons/kenney-weapon-pack/Models/OBJ format/pistol.obj,3D/weapons/kenney-weapon-pack/Models/OBJ format/pistol.mtl,3D/weapons/kenney-weapon-pack/Models/OBJ format/shotgun.obj,3D/weapons/kenney-weapon-pack/Models/OBJ format/shotgun.mtl,3D/weapons/kenney-weapon-pack/Models/OBJ format/sniper.obj,3D/weapons/kenney-weapon-pack/Models/OBJ format/sniper.mtl,3D/weapons/kenney-weapon-pack/Models/OBJ format/uzi.obj,3D/weapons/kenney-weapon-pack/Models/OBJ format/uzi.mtl,3D/weapons/kenney-weapon-pack/Models/OBJ format/rocketlauncherModern.obj,3D/weapons/kenney-weapon-pack/Models/OBJ format/rocketlauncherModern.mtl,3D/weapons/kenney-weapon-pack/Models/OBJ format/grenade.obj,3D/weapons/kenney-weapon-pack/Models/OBJ format/grenade.mtl" --exclude=""
if errorlevel 1 goto :fail

for %%F in (machinegun pistol shotgun sniper uzi rocketlauncherModern grenade) do (
  copy /y "%CACHE%\3D\weapons\kenney-weapon-pack\Models\OBJ format\%%F.obj" "%WEAPONS%\%%F.obj" >nul || goto :fail
  if exist "%CACHE%\3D\weapons\kenney-weapon-pack\Models\OBJ format\%%F.mtl" copy /y "%CACHE%\3D\weapons\kenney-weapon-pack\Models\OBJ format\%%F.mtl" "%WEAPONS%\%%F.mtl" >nul
)
copy /y "%CACHE%\3D\weapons\kenney-weapon-pack\License.txt" "%WEAPONS%\LICENSE_KENNEY_CC0.txt" >nul

 echo [3/4] Downloading Oster menu background from Wikimedia Commons...
where curl.exe >nul 2>nul || (
  echo [ERROR] curl.exe not found.
  goto :fail
)
curl.exe -L --fail --retry 2 "https://commons.wikimedia.org/wiki/Special:Redirect/file/%%D0%%9E%%D1%%81%%D1%%82%%D0%%B5%%D1%%80-%%D0%%AE%%D1%%80%%D1%%8C%%D0%%B5%%D0%%B2%%D0%%B0_%%D0%%91%%D0%%BE%%D0%%B6%%D0%%BD%%D0%%B8%%D1%%86%%D0%%B0_%%D0%%A4%%D0%%BE%%D1%%82%%D0%%BE_03.jpg" -o "%UI%\Oster_Menu_BG.jpg"
if errorlevel 1 goto :fail
>"%UI%\ATTRIBUTION.txt" echo Oster - Yuriieva Bozhnytsia Photo 03, Wikimedia Commons. CC BY-SA 4.0. Source: https://commons.wikimedia.org/wiki/File:Остер-Юрьева_Божница_Фото_03.jpg

 echo [4/4] Importing R13 assets into Unreal...
"%EDITOR_CMD%" "%PROJECT%" -run=pythonscript -script="%IMPORT_SCRIPT%" -unattended -nop4 -NullRHI -NoSplash -UTF8Output
if errorlevel 1 goto :fail

 echo.
echo ============================================================
echo R13 CONTENT IMPORT: PASS
echo Imported assets are under /Game/R13/Weapons and /Game/R13/UI.
echo ============================================================
if exist "%CACHE%" rmdir /s /q "%CACHE%"
pause
exit /b 0

:fail
echo.
echo [ERROR] R13 content import failed. See messages above.
if exist "%CACHE%" rmdir /s /q "%CACHE%"
pause
exit /b 10
