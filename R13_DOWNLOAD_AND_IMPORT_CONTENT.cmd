@echo off
setlocal EnableExtensions
cd /d "%~dp0"

if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "EDITOR_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT=%~dp0OsterConflict\OsterConflict.uproject"
set "IMPORT_SCRIPT=%~dp0OsterConflict\Scripts\R13\IMPORT_R13_CONTENT.py"
set "RAW=%~dp0OsterConflict\Content\Raw\R13"
set "WEAPONS=%RAW%\Weapons\Kenney"
set "STEIN=%RAW%\Weapons\SteinClassicWeapons\WeaponsPack"
set "AUDIO=%RAW%\Audio"
set "UI=%RAW%\UI"
set "IMPORT_STATE=%RAW%\R13_IMPORT_STATE.txt"
set "CACHE=%TEMP%\OsterConflict_R13_CC0"

rem Museum source label: Будинок Солонини, Остер.JPG
rem Legacy state R13_MUSEUM_WEAPONS_V2 is superseded by R13_STEIN_WEAPONS_V3.
rem Wikimedia Special:Redirect/file intermittently returns 404 to curl even while the file exists.
rem Use the canonical upload.wikimedia.org object URL and keep a second real museum exterior as fallback.
set "MUSEUM_URL_PRIMARY=https://upload.wikimedia.org/wikipedia/commons/b/bd/%%D0%%91%%D1%%83%%D0%%B4%%D0%%B8%%D0%%BD%%D0%%BE%%D0%%BA_%%D0%%A1%%D0%%BE%%D0%%BB%%D0%%BE%%D0%%BD%%D0%%B8%%D0%%BD%%D0%%B8%%2C_%%D0%%9E%%D1%%81%%D1%%82%%D0%%B5%%D1%%80.JPG"
set "MUSEUM_URL_FALLBACK=https://upload.wikimedia.org/wikipedia/commons/7/71/%%D0%%91%%D1%%83%%D0%%B4%%D0%%B8%%D0%%BD%%D0%%BE%%D0%%BA_%%D1%%96_%%D1%%81%%D0%%B0%%D0%%B4%%D0%%B8%%D0%%B1%%D0%%B0_%%D0%%B3%%D0%%B5%%D0%%BD%%D0%%B5%%D1%%80%%D0%%B0%%D0%%BB-%%D0%%BB%%D0%%B5%%D0%%B9%%D1%%82%%D0%%B5%%D0%%BD%%D0%%B0_%%D0%%92.%%D0%%9A.%%D0%%A1%%D0%%BE%%D0%%BB%%D0%%BE%%D0%%BD%%D0%%B8%%D0%%BD%%D0%%B8_%%D0%%B2_%%D0%%9E%%D1%%81%%D1%%82%%D1%%80%%D1%%96.jpg"

 echo ============================================================
 echo OSTER CONFLICT R13 - DOWNLOAD + IMPORT CONTENT
 echo ============================================================
 echo Downloads fallback CC0 content and imports the committed
 echo Stein Classic Weapons CC0 pack plus audio and Oster museum art.
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
if not exist "%STEIN%\1911\SKM_1911.fbx" (
  echo [ERROR] Stein Classic Weapons source pack is missing.
  echo Expected: %STEIN%\1911\SKM_1911.fbx
  echo Pull the current r13-content-gameplay-pass branch first.
  pause
  exit /b 6
)

if exist "%IMPORT_STATE%" del /q "%IMPORT_STATE%"
if exist "%CACHE%" rmdir /s /q "%CACHE%"
mkdir "%WEAPONS%" 2>nul
mkdir "%AUDIO%" 2>nul
mkdir "%UI%" 2>nul

set "GIT_LFS_SKIP_SMUDGE=1"
echo [1/5] Cloning public CC0 fallback asset source...
git clone --depth 1 --filter=blob:none https://github.com/series-ai/jam-ready-assets.git "%CACHE%"
if errorlevel 1 goto :fail

 echo [2/5] Downloading selected Kenney fallback weapon models through Git LFS...
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

 echo [4/5] Downloading Oster local-history museum exterior for the menu background...
if exist "%UI%\Oster_Menu_BG.jpg" del /q "%UI%\Oster_Menu_BG.jpg"
curl.exe -L --fail --retry 3 --retry-all-errors --connect-timeout 20 -A "OsterConflict-R13/1.0" "%MUSEUM_URL_PRIMARY%" -o "%UI%\Oster_Menu_BG.jpg"
if errorlevel 1 (
  echo [WARN] Primary Wikimedia museum image failed; trying verified fallback exterior...
  if exist "%UI%\Oster_Menu_BG.jpg" del /q "%UI%\Oster_Menu_BG.jpg"
  curl.exe -L --fail --retry 3 --retry-all-errors --connect-timeout 20 -A "OsterConflict-R13/1.0" "%MUSEUM_URL_FALLBACK%" -o "%UI%\Oster_Menu_BG.jpg"
  if errorlevel 1 goto :fail
)
for %%I in ("%UI%\Oster_Menu_BG.jpg") do if %%~zI LSS 50000 (
  echo [ERROR] Museum background download is unexpectedly small: %%~zI bytes.
  goto :fail
)
>"%UI%\ATTRIBUTION.txt" echo Oster museum exterior - Wikimedia Commons, Oster museum of local history. Preserve the selected file's author and license attribution from its Wikimedia Commons source page when distributing.

 echo [5/5] Importing R13 assets into Unreal, including Stein Classic Weapons...
"%EDITOR_CMD%" "%PROJECT%" -run=pythonscript -script="%IMPORT_SCRIPT%" -unattended -nop4 -NullRHI -NoSplash -UTF8Output
if errorlevel 1 goto :fail

>"%IMPORT_STATE%" echo R13_STEIN_WEAPONS_V3

echo.
echo ============================================================
echo R13 CONTENT IMPORT: PASS
echo Assets: /Game/R13/Weapons  /Game/R13/Audio  /Game/R13/UI
echo Stein 1911, AK47, LeverAction, M14, M700, MP5, Mac10 and Tec9 verified.
echo ============================================================
if exist "%CACHE%" rmdir /s /q "%CACHE%"
pause
exit /b 0

:fail
echo.
echo [ERROR] R13 content import failed. See messages above.
if exist "%IMPORT_STATE%" del /q "%IMPORT_STATE%"
if exist "%CACHE%" rmdir /s /q "%CACHE%"
pause
exit /b 10
