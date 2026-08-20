@echo off
setlocal EnableExtensions EnableDelayedExpansion
chcp 65001 >nul

set "PROJECT_DIR=%~dp0"
set "REPO_DIR=%PROJECT_DIR%.."
set "ZIP_PATH=%~1"

if defined ZIP_PATH if not exist "%ZIP_PATH%" set "ZIP_PATH="

if not defined ZIP_PATH (
    for %%F in (
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
    echo Drag the downloaded models ZIP onto this CMD file and run it again.
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
  "$btrZip=Get-ChildItem $stage -Recurse -File -Filter '*.zip' ^| Where-Object {$_.Name -match '(?i)btr'} ^| Select-Object -First 1;" ^
  "if(-not $m2){throw 'M2 GLB not found in upload'}; if(-not $hmmwv){throw 'HMMWV GLB not found in upload'}; if(-not $btrZip){throw 'BTR ZIP not found in upload'};" ^
  "$m2Dst=Join-Path $project 'SourceAssets\Production\Weapons\M2'; $hDst=Join-Path $project 'SourceAssets\Production\Vehicles\HMMWV'; $bDst=Join-Path $project 'SourceAssets\Production\Vehicles\BTR4'; $tDst=Join-Path $bDst 'Textures';" ^
  "New-Item $m2Dst,$hDst,$bDst,$tDst -ItemType Directory -Force ^| Out-Null;" ^
  "Copy-Item $m2.FullName (Join-Path $m2Dst 'm2_50cal_machinegun_cc0.glb') -Force;" ^
  "Copy-Item $hmmwv.FullName (Join-Path $hDst 'ukrainian_hmmwv_mk_19.glb') -Force;" ^
  "$bStage=Join-Path $stage 'BTR4_outer'; New-Item $bStage -ItemType Directory -Force ^| Out-Null; Expand-Archive -LiteralPath $btrZip.FullName -DestinationPath $bStage -Force;" ^
  "$nested=Get-ChildItem $bStage -Recurse -File -Filter '*.zip' ^| Select-Object -First 1;" ^
  "$sourceStage=$bStage; if($nested){$sourceStage=Join-Path $stage 'BTR4_source'; New-Item $sourceStage -ItemType Directory -Force ^| Out-Null; Expand-Archive -LiteralPath $nested.FullName -DestinationPath $sourceStage -Force};" ^
  "$fbx=Get-ChildItem $sourceStage -Recurse -File -Filter '*.fbx' ^| Select-Object -First 1; if(-not $fbx){$fbx=Get-ChildItem $bStage -Recurse -File -Filter '*.fbx' ^| Select-Object -First 1}; if(-not $fbx){throw 'BTR FBX not found'};" ^
  "Copy-Item $fbx.FullName (Join-Path $bDst 'BTR4_Bucephalus.fbx') -Force;" ^
  "$wanted=@('Bahnya_low_albedo.png','Koleso_low_albedo.png','Korpus_low_albedo.png','Windows_low_albedo.png','interior.png','tire.png');" ^
  "foreach($name in $wanted){$tex=Get-ChildItem $sourceStage -Recurse -File -Filter $name ^| Sort-Object Length -Descending ^| Select-Object -First 1; if(-not $tex){$tex=Get-ChildItem $bStage -Recurse -File -Filter $name ^| Sort-Object Length -Descending ^| Select-Object -First 1}; if(-not $tex){throw ('BTR texture missing: '+$name)}; Copy-Item $tex.FullName (Join-Path $tDst $name) -Force};" ^
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
    git push origin HEAD || goto :git_error
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
git diff --cached --quiet
if errorlevel 1 (
    git commit -m "Import HMMWV M2 and BTR-4 production Unreal assets" || goto :git_error
    git push origin HEAD || goto :git_error
) else (
    echo Unreal production assets already committed.
)

echo.
echo ============================================================
echo PASS: source + Unreal production assets are committed/pushed.
echo ============================================================
popd
exit /b 0

:git_error
echo ERROR: git commit/push failed.
popd
exit /b 8
