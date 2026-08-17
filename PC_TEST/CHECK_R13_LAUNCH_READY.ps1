param(
    [Parameter(Mandatory=$true)][string]$ProjectRoot
)

$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest

$ProjectRoot=(Resolve-Path $ProjectRoot).Path
$EditorDll=Join-Path $ProjectRoot 'Binaries\Win64\UnrealEditor-OsterConflict.dll'
$SourceRoot=Join-Path $ProjectRoot 'Source'
$ImportState=Join-Path $ProjectRoot 'Content\Raw\R13\R13_IMPORT_STATE.txt'
$WeaponRoot=Join-Path $ProjectRoot 'Content\R13\Weapons'
$UIRoot=Join-Path $ProjectRoot 'Content\R13\UI'

if(-not(Test-Path $EditorDll)){
    Write-Host '[ERROR] Editor module is not built yet.' -ForegroundColor Red
    Write-Host 'Run START_HERE option 1 first.' -ForegroundColor Yellow
    exit 4
}

$DllItem=Get-Item $EditorDll
$LatestSource=Get-ChildItem $SourceRoot -Recurse -File -ErrorAction Stop |
    Where-Object { $_.Extension -in @('.h','.cpp') } |
    Sort-Object LastWriteTimeUtc -Descending |
    Select-Object -First 1

if($LatestSource -and $LatestSource.LastWriteTimeUtc -gt $DllItem.LastWriteTimeUtc){
    Write-Host ''
    Write-Host '============================================================' -ForegroundColor Yellow
    Write-Host 'R13 GAMEPLAY LAUNCH BLOCKED: C++ BUILD IS STALE' -ForegroundColor Yellow
    Write-Host '============================================================' -ForegroundColor Yellow
    Write-Host ('Newest source: ' + $LatestSource.FullName)
    Write-Host ('Source time : ' + $LatestSource.LastWriteTime)
    Write-Host ('Editor DLL  : ' + $DllItem.LastWriteTime)
    Write-Host 'Run START_HERE option 1, then return to option 4.' -ForegroundColor Yellow
    exit 9
}

$Missing=New-Object System.Collections.Generic.List[string]
$StateOk=$false
if(Test-Path $ImportState){
    $StateOk=((Get-Content $ImportState -Raw).Trim() -eq 'R13_MUSEUM_WEAPONS_V2')
}
if(-not $StateOk){ $Missing.Add('R13 content state: R13_MUSEUM_WEAPONS_V2') }

foreach($Name in @('machinegun','pistol','shotgun','sniper','uzi','rocketlauncherModern','grenade')){
    $Path=Join-Path $WeaponRoot ($Name + '.uasset')
    if(-not(Test-Path $Path)){ $Missing.Add('weapon asset: ' + $Name + '.uasset') }
}
$MenuBackground=Join-Path $UIRoot 'Oster_Menu_BG.uasset'
if(-not(Test-Path $MenuBackground)){ $Missing.Add('menu background: Oster_Menu_BG.uasset') }

if($Missing.Count -gt 0){
    Write-Host ''
    Write-Host '============================================================' -ForegroundColor Yellow
    Write-Host 'R13 GAMEPLAY LAUNCH BLOCKED: REQUIRED ART IS MISSING OR STALE' -ForegroundColor Yellow
    Write-Host '============================================================' -ForegroundColor Yellow
    foreach($Item in $Missing){ Write-Host ('[MISSING] ' + $Item) -ForegroundColor Red }
    Write-Host 'Run START_HERE option 8 first.' -ForegroundColor Yellow
    Write-Host 'The importer now verifies the museum background and all runtime-required weapon assets.'
    exit 7
}

Write-Host 'R13 launch readiness: PASS (compiled source + current required art).' -ForegroundColor Green
exit 0
