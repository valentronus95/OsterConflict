@echo off
setlocal EnableExtensions
cd /d "%~dp0"

if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "EDITOR_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"
set "IMPORT_SCRIPT=%~dp0OsterConflict\Scripts\R13\IMPORT_R13_CONTENT.py"
set "RAW=%~dp0OsterConflict\Content\Raw\R13"
set "WEAPONS=%RAW%\Weapons\Kenney"
set "AUDIO=%RAW%\Audio"
set "UI=%RAW%\UI"
set "CACHE=%TEMP%\OsterConflict_R13_CC0"

 echo ============================================================
 echo OSTER CONFLICT R13 - DOWNLOAD + IMPORT CONTENT
 echo ============================================================
 echo Downloads selected CC0 weapon models, CC0 combat audio and a
 echo licensed real Oster photo, then imports them into Unreal.
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
where curl.exe >nul 2>nul || (
  echo [ERROR] curl.exe not found.
  pause
  exit /b 4
)
if not exist "%EDITOR_CMD%" (
  echo [ERROR] UnrealEditor-Cmd.exe not found at %EDITOR_CMD%
  pause
  exit /b 5
)

if exist "%CACHE%" rmdir /s /q "%CACHE%"
mkdir "%WEAPONS%" 2>nul
mkdir "%AUDIO%" 2>nul
mkdir "%UI%" 2>nul

set "GIT_LFS_SKIP_SMUDGE=1"
echo [1/5] Cloning public CC0 asset source...
git clone --depth 1 --filter=blob:none https://github.com/series-ai/jam-ready-assets.git "%CACHE%"
if errorlevel 1 goto :fail

 echo [2/5] Downloading selected Kenney weapon models through Git LFS...
git -C "%CACHE%" lfs pull --include="3D/weapons/kenney-weapon-pack/Models/OBJ format/machinegun.obj,3D/weapons/kenney-weapon-pack/Models/OBJ format/machinegun.mtl,3D/weapons/kenney-weapon-pack/Models/OBJ format/pistol.obj,3D/weapons/kenney-weapon-pack/Models/OBJ format/pistol.mtl,3D/weapons/kenney-weapon-pack/Models/OBJ format/shotgun.obj,3D/weapons/kenney-weapon-pack/Models/OBJ format/shotgun.mtl,3D/weapons/kenney-weapon-pack/Models/OBJ format/sniper.obj,3D/weapons/kenney-weapon-pack/Models/OBJ format/sniper.mtl,3D/weapons/kenney-weapon-pack/Models/OBJ format/uzi.obj,3D/weapons/kenney-weapon-pack/Models/OBJ format/uzi.mtl,3D/weapons/kenney-weapon-pack/Models/OBJ format/rocketlauncherModern.obj,3D/weapons/kenney-weapon-pack/Models/OBJ format/rocketlauncherModern.mtl,3D/weapons/kenney-weapon-pack/Models/OBJ format/grenade.obj,3D/weapons/kenney-weapon-pack/Models/OBJ format/grenade.mtl" --exclude=""
if errorlevel 1 goto :fail

for %%F in (machinegun pistol shotgun sniper uzi rocketlauncherModern grenade) do (
  copy /y "%CACHE%\3D\weapons\kenney-weapon-pack\Models\OBJ format\%%F.obj" "%WEAPONS%\%%F.obj" >nul || goto :fail
  if exist "%CACHE%\3D\weapons\kenney-weapon-pack\Models\OBJ format\%%F.mtl" copy /y "%CACHE%\3D\weapons\kenney-weapon-pack\Models\OBJ format\%%F.mtl" "%WEAPONS%\%%F.mtl" >nul
)
copy /y "%CACHE%\3D\weapons\kenney-weapon-pack\License.txt" "%WEAPONS%\LICENSE_KENNEY_CC0.txt" >nul

 echo [3/5] Downloading CC0 combat audio...
curl.exe -L --fail --retry 2 "https://opengameart.org/sites/default/files/gunfire_sfx.wav" -o "%AUDIO%\gunfire_sfx.wav" || goto :fail
curl.exe -L --fail --retry 2 "https://opengameart.org/sites/default/files/gunreload1.wav" -o "%AUDIO%\gunreload1.wav" || goto :fail
curl.exe -L --fail --retry 2 "https://opengameart.org/sites/default/files/assaultriflereload1_0.wav" -o "%AUDIO%\assaultriflereload1.wav" || goto :fail
curl.exe -L --fail --retry 2 "https://opengameart.org/sites/default/files/shotguncock_0.wav" -o "%AUDIO%\shotguncock.wav" || goto :fail
curl.exe -L --fail --retry 2 "https://opengameart.org/sites/default/files/player_hit.wav" -o "%AUDIO%\player_hit.wav" || goto :fail
curl.exe -L --fail --retry 2 "https://opengameart.org/sites/default/files/snd_bullethit.wav" -o "%AUDIO%\snd_bullethit.wav" || goto :fail
curl.exe -L --fail --retry 2 "https://opengameart.org/sites/default/files/snd_throw1.wav" -o "%AUDIO%\snd_throw1.wav" || goto :fail
curl.exe -L --fail --retry 2 "https://opengameart.org/sites/default/files/dull_explosion.wav" -o "%AUDIO%\dull_explosion.wav" || goto :fail
>"%AUDIO%\LICENSES.txt" echo Combat audio sources are CC0. Gunfire: iamoneabe / OpenGameArt. Reloads: SpringySpringo / OpenGameArt. Impact/throw/explosion: Spring Spring / OpenGameArt.

 echo [4/5] Downloading real Oster menu background from Wikimedia Commons...
curl.exe -L --fail --retry 2 "https://commons.wikimedia.org/wiki/Special:Redirect/file/Oster_2.jpg" -o "%UI%\Oster_Menu_BG.jpg"
if errorlevel 1 goto :fail
>"%UI%\ATTRIBUTION.txt" echo Oster 2.jpg - riverbank of the Oster River, author Barow, Wikimedia Commons, CC BY-SA 4.0. Source: https://commons.wikimedia.org/wiki/File:Oster_2.jpg

 echo [5/5] Importing R13 assets into Unreal...
"%EDITOR_CMD%" "%PROJECT%" -run=pythonscript -script="%IMPORT_SCRIPT%" -unattended -nop4 -NullRHI -NoSplash -UTF8Output
if errorlevel 1 goto :fail

 echo.
echo ============================================================
echo R13 CONTENT IMPORT: PASS
echo Assets: /Game/R13/Weapons  /Game/R13/Audio  /Game/R13/UI
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
