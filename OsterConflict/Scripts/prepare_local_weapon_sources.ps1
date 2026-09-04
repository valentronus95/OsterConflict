param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectDir
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression.FileSystem

$ProjectDir = [System.IO.Path]::GetFullPath($ProjectDir)
$RepoRoot = Split-Path -Parent $ProjectDir
$Inbox = Join-Path $RepoRoot 'models_game_OC'
$StageRoot = Join-Path $ProjectDir 'Saved\LocalProductionSourceIntake\Weapons'
$ManifestPath = Join-Path $StageRoot 'weapon_sources.json'

if (Test-Path -LiteralPath $StageRoot) {
    Remove-Item -LiteralPath $StageRoot -Recurse -Force
}
New-Item -ItemType Directory -Path $StageRoot -Force | Out-Null

$supportedModels = @('.fbx', '.glb')
$companionExtensions = @('.png', '.tga', '.jpg', '.jpeg', '.bmp', '.exr', '.dds', '.mtl', '.bin')

function Test-SafeZipEntry([string]$EntryName, [string]$Destination) {
    if ([string]::IsNullOrWhiteSpace($EntryName)) { return $false }
    $normalized = $EntryName.Replace('\', '/')
    if ($normalized.StartsWith('/') -or $normalized -match '^[A-Za-z]:') { return $false }
    if ($normalized.Split('/') | Where-Object { $_ -eq '..' }) { return $false }

    $root = [System.IO.Path]::GetFullPath($Destination)
    if (-not $root.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
        $root += [System.IO.Path]::DirectorySeparatorChar
    }
    $candidate = [System.IO.Path]::GetFullPath((Join-Path $Destination $normalized))
    return $candidate.StartsWith($root, [System.StringComparison]::OrdinalIgnoreCase)
}

function Get-WeaponKind([string]$Text) {
    $lower = $Text.ToLowerInvariant()
    if ($lower -match 'm249|minimi') { return 'M249' }
    if ($lower -match 'remington|870') { return 'Remington870' }
    return $null
}

function Expand-SafeArchive([System.IO.FileInfo]$Archive, [string]$Destination) {
    if (Test-Path -LiteralPath $Destination) {
        Remove-Item -LiteralPath $Destination -Recurse -Force
    }
    New-Item -ItemType Directory -Path $Destination -Force | Out-Null

    $zip = [System.IO.Compression.ZipFile]::OpenRead($Archive.FullName)
    try {
        foreach ($entry in $zip.Entries) {
            if (-not (Test-SafeZipEntry -EntryName $entry.FullName -Destination $Destination)) {
                throw ('Unsafe ZIP entry: ' + $entry.FullName)
            }
        }
    }
    finally {
        $zip.Dispose()
    }

    Expand-Archive -LiteralPath $Archive.FullName -DestinationPath $Destination -Force
}

function Find-BestModel([string]$Root, [string]$Kind) {
    if (-not (Test-Path -LiteralPath $Root)) { return $null }
    $regex = if ($Kind -eq 'M249') { '(?i)(m249|minimi)' } else { '(?i)(remington|870)' }

    $candidates = Get-ChildItem -LiteralPath $Root -Recurse -File -ErrorAction SilentlyContinue |
        Where-Object { $supportedModels -contains $_.Extension.ToLowerInvariant() }

    $named = $candidates | Where-Object { $_.FullName -match $regex } | Sort-Object Length -Descending | Select-Object -First 1
    if ($named) { return $named }

    return $null
}

$manifest = [ordered]@{
    schema = 'oster-conflict-pass45-local-production-weapons-v1'
    inbox = $Inbox
    M249 = $null
    Remington870 = $null
    gaps = @()
}

if (-not (Test-Path -LiteralPath $Inbox)) {
    $manifest.gaps += 'MODEL_INBOX_MISSING'
    $manifest | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $ManifestPath -Encoding UTF8
    Write-Host ('[WEAPON SOURCE] CONTENT GAP: local model inbox not found: ' + $Inbox)
    exit 0
}

# Loose files are accepted only when their own path identifies the exact weapon.
foreach ($kind in @('M249', 'Remington870')) {
    $loose = Find-BestModel -Root $Inbox -Kind $kind
    if ($loose) {
        $manifest[$kind] = $loose.FullName
        Write-Host ('[WEAPON SOURCE] ' + $kind + ' loose source: ' + $loose.FullName)
    }
}

$archiveIndex = 0
foreach ($archive in (Get-ChildItem -LiteralPath $Inbox -Recurse -File -Filter '*.zip' -ErrorAction SilentlyContinue | Sort-Object FullName)) {
    if ($manifest.M249 -and $manifest.Remington870) { break }

    $archiveIndex++
    $archiveKind = Get-WeaponKind $archive.Name
    $stage = Join-Path $StageRoot ('archive_' + $archiveIndex)

    try {
        Expand-SafeArchive -Archive $archive -Destination $stage
    }
    catch {
        Write-Warning ('[WEAPON SOURCE] Archive left unresolved: ' + $archive.FullName + ' ' + $_.Exception.Message)
        continue
    }

    foreach ($kind in @('M249', 'Remington870')) {
        if ($manifest[$kind]) { continue }
        $candidate = Find-BestModel -Root $stage -Kind $kind
        if (-not $candidate -and $archiveKind -eq $kind) {
            # A clearly labelled weapon archive may contain a generic source.fbx/model.glb filename.
            $candidate = Get-ChildItem -LiteralPath $stage -Recurse -File -ErrorAction SilentlyContinue |
                Where-Object { $supportedModels -contains $_.Extension.ToLowerInvariant() } |
                Sort-Object Length -Descending |
                Select-Object -First 1
        }
        if ($candidate) {
            $manifest[$kind] = $candidate.FullName
            Write-Host ('[WEAPON SOURCE] ' + $kind + ' archive source: ' + $candidate.FullName)
        }
    }
}

foreach ($kind in @('M249', 'Remington870')) {
    if (-not $manifest[$kind]) {
        $manifest.gaps += ($kind + '_SOURCE_MISSING')
        Write-Host ('[WEAPON SOURCE] CONTENT GAP: ' + $kind + ' exact local source not found.') -ForegroundColor Yellow
    }
}

$manifest | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $ManifestPath -Encoding UTF8
Write-Host ('[WEAPON SOURCE] Manifest: ' + $ManifestPath)
exit 0
