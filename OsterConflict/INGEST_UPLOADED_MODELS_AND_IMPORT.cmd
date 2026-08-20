@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
set "REPO_DIR=%PROJECT_DIR%.."
set "ZIP_PATH=%~1"
set "TARGET_BRANCH=feat/import-hmmwv-btr4-m2"

if defined ZIP_PATH if not exist "%ZIP_PATH%" set "ZIP_PATH="

if not defined ZIP_PATH (
    for %%F in (
        "%USERPROFILE%\Downloads\OsterConflict_vehicle_assets_ready.zip"
        "%USERPROFILE%\Desktop\OsterConflict_vehicle_assets_ready.zip"
        "%REPO_DIR%\OsterConflict_vehicle_assets_ready.zip"
        "%PROJECT_DIR%OsterConflict_vehicle_assets_ready.zip"
        "%USERPROFILE%\Downloads\моделі.zip"
        "%USERPROFILE%\Desktop\моделі.zip"
        "%REPO_DIR%\моделі.zip"
        "%PROJECT_DIR%моделі.zip"
    ) do (
        if not defined ZIP_PATH if exist "%%~F" set "ZIP_PATH=%%~F"
    )
)

if not defined ZIP_PATH (
    echo ERROR: models ZIP was not found.
    echo Drag OsterConflict_vehicle_assets_ready.zip or the original models ZIP onto this CMD file and run it again.
    exit /b 2
)

pushd "%REPO_DIR%" || exit /b 3

where git >nul 2>nul || (
    echo ERROR: git.exe is not available in PATH.
    popd
    exit /b 4
)

git lfs version >nul 2>nul || (
    echo ERROR: Git LFS is not installed/available.
    popd
    exit /b 5
)

git lfs install >nul 2>nul

rem Never let this ingest accidentally commit production binaries straight to main
rem or carry unrelated tracked edits from an older local session into the asset branch.
git diff --quiet
if errorlevel 1 goto :dirty_tree
git diff --cached --quiet
if errorlevel 1 goto :dirty_tree

git remote get-url origin >nul 2>nul || (
    echo ERROR: git remote 'origin' is not configured.
    popd
    exit /b 9
)

git fetch origin "%TARGET_BRANCH%" || goto :git_error

for /f "delims=" %%B in ('git branch --show-current') do set "CURRENT_BRANCH=%%B"
if /I not "!CURRENT_BRANCH!"=="%TARGET_BRANCH%" (
    git show-ref --verify --quiet "refs/heads/%TARGET_BRANCH%"
    if errorlevel 1 (
        git switch --track -c "%TARGET_BRANCH%" "origin/%TARGET_BRANCH%" || goto :git_error
    ) else (
        git switch "%TARGET_BRANCH%" || goto :git_error
    )
)

git pull --ff-only origin "%TARGET_BRANCH%" || goto :git_error

for /f "delims=" %%B in ('git branch --show-current') do set "CURRENT_BRANCH=%%B"
if /I not "!CURRENT_BRANCH!"=="%TARGET_BRANCH%" (
    echo ERROR: safety check failed. Current branch is !CURRENT_BRANCH!, expected %TARGET_BRANCH%.
    popd
    exit /b 10
)

echo PASS: ingest is locked to branch %TARGET_BRANCH%.

set "OC_ZIP=%ZIP_PATH%"
set "OC_PROJECT=%PROJECT_DIR%"

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference='Stop';" ^
  "$zip=$env:OC_ZIP; $project=$env:OC_PROJECT;" ^
  "$stage=Join-Path $project 'Saved\UploadedModelsIntake';" ^
  "if(Test-Path $stage){Remove-Item $stage -Recurse -Force}; New-Item $stage -ItemType Directory -Force ^| Out-Null;" ^
  "Expand-Archive -LiteralPath $zip -DestinationPath $stage -Force;" ^
  "$m2=Get-ChildItem $stage -Recurse -File -Filter '*.glb' ^| Where-Object {$_.Name -match '(?i)(50cal|m2)'} ^| Select-Object -First 1;" ^
  "$hmmwv=Get-ChildItem $stage -Recurse -File -Filter '*.glb' ^| Where-Object {$_.Name -match '(?i)(hmmwv|humvee)'} ^| Select-Object -First 1;" ^
  "$btrFbx=Get-ChildItem $stage -Recurse -File -Filter '*.fbx' ^| Where-Object {$_.Name -match '(?i)(btr|bucephalus)'} ^| Select-Object -First 1;" ^
  "$btrZip=Get-ChildItem $stage -Recurse -File -Filter '*.zip' ^| Where-Object {$_.Name -match '(?i)btr'} ^| Select-Object -First 1;" ^
  "if(-not $m2){throw 'M2 GLB not found in upload'}; if(-not $hmmwv){throw 'HMMWV GLB not found in upload'}; if(-not $btrFbx -and -not $btrZip){throw 'BTR FBX/ZIP not found in upload'};" ^
  "$m2Dst=Join-Path $project 'SourceAssets\Production\Weapons\M2'; $hDst=Join-Path $project 'SourceAssets\Production\Vehicles\HMMWV'; $bDst=Join-Path $project 'SourceAssets\Production\Vehicles\BTR4'; $tDst=Join-Path $bDst 'Textures';" ^
  "New-Item $m2Dst,$hDst,$bDst,$tDst -ItemType Directory -Force ^| Out-Null;" ^
  "Copy-Item $m2.FullName (Join-Path $m2Dst 'm2_50cal_machinegun_cc0.glb') -Force;" ^
  "Copy-Item $hmmwv.FullName (Join-Path $hDst 'ukrainian_hmmwv_mk_19.glb') -Force;" ^
  "$sourceStage=$stage; $bStage=$null; $fbx=$btrFbx;" ^
  "if(-not $fbx){" ^
  "  $bStage=Join-Path $stage 'BTR4_outer'; New-Item $bStage -ItemType Directory -Force ^| Out-Null; Expand-Archive -LiteralPath $btrZip.FullName -DestinationPath $bStage -Force;" ^
  "  $nested=Get-ChildItem $bStage -Recurse -File -Filter '*.zip' ^| Select-Object -First 1;" ^
  "  $sourceStage=$bStage; if($nested){$sourceStage=Join-Path $stage 'BTR4_source'; New-Item $sourceStage -ItemType Directory -Force ^| Out-Null; Expand-Archive -LiteralPath $nested.FullName -DestinationPath $sourceStage -Force};" ^
  "  $fbx=Get-ChildItem $sourceStage -Recurse -File -Filter '*.fbx' ^| Select-Object -First 1; if(-not $fbx){$fbx=Get-ChildItem $bStage -Recurse -File -Filter '*.fbx' ^| Select-Object -First 1};" ^
  "};" ^
  "if(-not $fbx){throw 'BTR FBX not found'}; Copy-Item $fbx.FullName (Join-Path $bDst 'BTR4_Bucephalus.fbx') -Force;" ^
  "$wanted=@('Bahnya_low_albedo.png','Koleso_low_albedo.png','Korpus_low_albedo.png','Windows_low_albedo.png','interior.png','tire.png');" ^
  "foreach($name in $wanted){$tex=Get-ChildItem $sourceStage -Recurse -File -Filter $name ^| Sort-Object Length -Descending ^| Select-Object -First 1; if(-not $tex -and $bStage){$tex=Get-ChildItem $bStage -Recurse -File -Filter $name ^| Sort-Object Length -Descending ^| Select-Object -First 1}; if(-not $tex){$tex=Get-ChildItem $stage -Recurse -File -Filter $name ^| Sort-Object Length -Descending ^| Select-Object -First 1}; if(-not $tex){throw ('BTR texture missing: '+$name)}; Copy-Item $tex.FullName (Join-Path $tDst $name) -Force};" ^
  "Write-Host 'PASS: source assets unpacked into SourceAssets/Production.'"

if errorlevel 1 (
    echo ERROR: failed to unpack/normalize uploaded model archive.
    popd
    exit /b 6
)

git add .gitattributes "OsterConflict/SourceAssets/Production"
git diff --cached --quiet
if errorlevel 1 (
    git commit -m "Add HMMWV M2 and BTR-4 production source assets" || goto :git_error
    git push origin "HEAD:%TARGET_BRANCH%" || goto :git_error
) else (
    echo Source assets already committed; continuing to Unreal import.
)

call "OsterConflict\IMPORT_PRODUCTION_VEHICLES_UE58.cmd"
if errorlevel 1 (
    echo ERROR: Unreal production import failed. Source assets remain safely committed.
    popd
    exit /b 7
)

git add "OsterConflict/Content/Production"

rem A successful Unreal process exit is not enough. Verify the canonical products exist,
rem are real local binaries and are represented by LFS pointers before we create the asset commit.
call "OsterConflict\VERIFY_PRODUCTION_MODEL_INGEST.cmd"
if errorlevel 1 (
    echo ERROR: production model verification failed after Unreal import.
    echo Source assets remain committed on %TARGET_BRANCH%, but generated assets were not committed.
    popd
    exit /b 12
)

git diff --cached --quiet
if errorlevel 1 (
    git commit -m "Import HMMWV M2 and BTR-4 production Unreal assets" || goto :git_error
    git push origin "HEAD:%TARGET_BRANCH%" || goto :git_error
) else (
    echo Unreal production assets already committed.
)

echo.
echo ============================================================
echo PASS: source + Unreal production assets are verified, committed and pushed.
echo Branch: %TARGET_BRANCH%
echo ============================================================
popd
exit /b 0

:dirty_tree
echo ERROR: tracked local changes are present before ingest.
echo Commit/revert the unrelated tracked changes first; ingest was not started.
popd
exit /b 11

:git_error
echo ERROR: git branch/fetch/commit/push operation failed.
popd
exit /b 8
