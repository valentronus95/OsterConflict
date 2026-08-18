param(
    [Parameter(Mandatory=$true)][string]$ProjectRoot
)

$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest

$ProjectRoot=(Resolve-Path $ProjectRoot).Path
$ContentRoot=Join-Path $ProjectRoot 'Content'
$PackagingConfig=Join-Path $ProjectRoot 'Config\DefaultGame.ini'

if(-not(Test-Path $ContentRoot)){
    Write-Host ('[ERROR] Content directory not found: ' + $ContentRoot) -ForegroundColor Red
    exit 12
}
if(-not(Test-Path $PackagingConfig)){
    Write-Host ('[ERROR] Packaging config not found: ' + $PackagingConfig) -ForegroundColor Red
    exit 12
}

function Test-GitLfsPointer([string]$Path){
    if(-not(Test-Path -LiteralPath $Path)){ return $false }
    $Item=Get-Item -LiteralPath $Path
    # Git LFS pointer files are tiny text files (~130 bytes). Avoid reading normal binary assets at all.
    if($Item.Length -gt 512){ return $false }
    try {
        $Bytes=[System.IO.File]::ReadAllBytes($Item.FullName)
        $Text=[System.Text.Encoding]::ASCII.GetString($Bytes)
        return $Text.StartsWith('version https://git-lfs.github.com/spec/v1')
    } catch {
        return $false
    }
}

$RawConfig=Get-Content $PackagingConfig -Raw
$Matches=[regex]::Matches($RawConfig,'DirectoriesToAlwaysCook=\(Path="([^"]+)"\)')
if($Matches.Count -eq 0){
    Write-Host '[ERROR] No DirectoriesToAlwaysCook entries were found in DefaultGame.ini.' -ForegroundColor Red
    exit 12
}

$MissingDirs=New-Object System.Collections.Generic.List[string]
$EmptyDirs=New-Object System.Collections.Generic.List[string]
$Pointers=New-Object System.Collections.Generic.List[string]
$CheckedAssets=0

foreach($Match in $Matches){
    $GamePath=$Match.Groups[1].Value
    if(-not $GamePath.StartsWith('/Game/')){ continue }

    $Relative=$GamePath.Substring(6).Replace('/',[System.IO.Path]::DirectorySeparatorChar)
    $LocalDir=Join-Path $ContentRoot $Relative
    if(-not(Test-Path -LiteralPath $LocalDir -PathType Container)){
        $MissingDirs.Add($GamePath)
        continue
    }

    $Assets=@(Get-ChildItem -LiteralPath $LocalDir -Recurse -File -ErrorAction Stop |
        Where-Object { $_.Extension -in @('.uasset','.umap') })
    if($Assets.Count -eq 0){
        $EmptyDirs.Add($GamePath)
        continue
    }

    foreach($Asset in $Assets){
        ++$CheckedAssets
        if(Test-GitLfsPointer $Asset.FullName){
            $Pointers.Add($Asset.FullName.Substring($ProjectRoot.Length).TrimStart('\'))
        }
    }
}

if($MissingDirs.Count -gt 0 -or $EmptyDirs.Count -gt 0 -or $Pointers.Count -gt 0){
    Write-Host ''
    Write-Host '============================================================' -ForegroundColor Yellow
    Write-Host 'R13 GAMEPLAY LAUNCH BLOCKED: GIT LFS CONTENT IS NOT READY' -ForegroundColor Yellow
    Write-Host '============================================================' -ForegroundColor Yellow
    foreach($Item in $MissingDirs){ Write-Host ('[MISSING COOK DIR] ' + $Item) -ForegroundColor Red }
    foreach($Item in $EmptyDirs){ Write-Host ('[EMPTY COOK DIR] ' + $Item) -ForegroundColor Red }
    foreach($Item in $Pointers){ Write-Host ('[LFS POINTER] ' + $Item) -ForegroundColor Red }
    Write-Host ''
    Write-Host 'Run Git LFS download for this repository (for example: git lfs pull), then retry.' -ForegroundColor Yellow
    Write-Host 'Do not replace the pointer files manually.' -ForegroundColor Yellow
    exit 13
}

Write-Host ('R13 Git LFS payloads: PASS (' + $CheckedAssets + ' runtime-cooked UE assets hydrated).') -ForegroundColor Green
exit 0
