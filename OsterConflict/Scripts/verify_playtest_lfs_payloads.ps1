param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectRoot
)

$ErrorActionPreference = 'Stop'
$ProjectRoot = [System.IO.Path]::GetFullPath($ProjectRoot)

$Roots = @(
    (Join-Path $ProjectRoot 'OsterConflict\Content\AK-47'),
    (Join-Path $ProjectRoot 'OsterConflict\Content\R13\Weapons'),
    (Join-Path $ProjectRoot 'OsterConflict\Content\PN_FoliageCollection')
)

$Bad = New-Object System.Collections.Generic.List[string]
$MissingRoots = New-Object System.Collections.Generic.List[string]

foreach ($Root in $Roots) {
    if (-not (Test-Path -LiteralPath $Root)) {
        $MissingRoots.Add($Root)
        continue
    }

    Get-ChildItem -LiteralPath $Root -Recurse -File -ErrorAction Stop |
        Where-Object { $_.Length -lt 1024 } |
        ForEach-Object {
            $FirstLine = Get-Content -LiteralPath $_.FullName -TotalCount 1 -ErrorAction SilentlyContinue
            if ($FirstLine -eq 'version https://git-lfs.github.com/spec/v1') {
                $Bad.Add($_.FullName)
            }
        }
}

if ($MissingRoots.Count -gt 0) {
    Write-Host '[STOP] Required playtest asset roots are missing after Git LFS pull:' -ForegroundColor Red
    $MissingRoots | ForEach-Object { Write-Host ('  ' + $_) }
    exit 11
}

if ($Bad.Count -gt 0) {
    Write-Host '[STOP] Unhydrated Git LFS model files remain:' -ForegroundColor Red
    $Bad | Select-Object -First 20 | ForEach-Object { Write-Host ('  ' + $_) }
    exit 12
}

Write-Host '[ASSETS] Weapon and foliage LFS payloads are hydrated.'
exit 0
