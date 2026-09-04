param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectDir
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression.FileSystem

$ProjectDir = [System.IO.Path]::GetFullPath($ProjectDir)
$RepoRoot = Split-Path -Parent $ProjectDir
$Inbox = Join-Path $RepoRoot 'models_game_OC'
$ReportDir = Join-Path $ProjectDir 'Saved\LocalModelInbox'
$ReportPath = Join-Path $ReportDir 'asset_inventory.json'

if (-not (Test-Path -LiteralPath $ReportDir)) {
    New-Item -ItemType Directory -Path $ReportDir -Force | Out-Null
}

$modelExtensions = @('.fbx', '.glb', '.gltf', '.obj', '.dae', '.blend')
$textureExtensions = @('.png', '.tga', '.jpg', '.jpeg', '.bmp', '.exr', '.dds')

function Get-Category([string]$Text) {
    $value = $Text.ToLowerInvariant()
    $categories = New-Object System.Collections.Generic.List[string]

    if ($value -match 'btr.?4|bucephal|буцеф') { $categories.Add('BTR4') }
    if ($value -match 'hmmwv|humvee|hummer') { $categories.Add('HMMWV') }
    if ($value -match '(^|[^a-z0-9])m2([^a-z0-9]|$)|browning|50.?cal') { $categories.Add('M2') }
    if ($value -match 'm249|minimi') { $categories.Add('M249') }
    if ($value -match 'remington|870') { $categories.Add('REMINGTON870') }
    if ($value -match 'm16|m4a1|(^|[^a-z0-9])m4([^a-z0-9]|$)') { $categories.Add('M16_M4') }
    if ($value -match 'pickup|pick.?up|technical|hilux') { $categories.Add('PICKUP') }
    if ($value -match 'skin|character|soldier|human|mannequin|uniform|персона|солдат|скін') { $categories.Add('CHARACTER_SKIN') }
    if ($value -match 'building|house|home|museum|silpo|stadium|culture|college|street|будин|музей|стадіон|вулиц') { $categories.Add('BUILDING_WORLD') }
    if ($value -match 'tree|foliage|grass|bush|vegetation|дерев|кущ|трава') { $categories.Add('FOLIAGE') }
    if ($value -match 'prop|furniture|chair|table|barrel|crate|мебл|проп') { $categories.Add('PROP') }

    if ($categories.Count -eq 0) { $categories.Add('UNCLASSIFIED') }
    return @($categories | Sort-Object -Unique)
}

function Test-SafeZipEntry([string]$EntryName, [string]$StageRoot) {
    if ([string]::IsNullOrWhiteSpace($EntryName)) { return $false }
    $normalized = $EntryName.Replace('\', '/')
    if ($normalized.StartsWith('/') -or $normalized -match '^[A-Za-z]:') { return $false }

    $segments = $normalized.Split('/')
    if ($segments | Where-Object { $_ -eq '..' }) { return $false }

    $candidate = [System.IO.Path]::GetFullPath((Join-Path $StageRoot $normalized))
    $root = [System.IO.Path]::GetFullPath($StageRoot)
    if (-not $root.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
        $root += [System.IO.Path]::DirectorySeparatorChar
    }
    return $candidate.StartsWith($root, [System.StringComparison]::OrdinalIgnoreCase)
}

$inventory = [ordered]@{
    schema = 'oster-conflict-pass45-local-model-inbox-v1'
    inbox = $Inbox
    archive_count = 0
    unsafe_archive_count = 0
    archives = @()
}

if (-not (Test-Path -LiteralPath $Inbox)) {
    $inventory.status = 'NO_INBOX'
    $inventory | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $ReportPath -Encoding UTF8
    Write-Host ('[MODEL INBOX] No local inbox found: ' + $Inbox)
    Write-Host ('[MODEL INBOX] Report: ' + $ReportPath)
    exit 0
}

$archives = Get-ChildItem -LiteralPath $Inbox -Recurse -File -Filter '*.zip' -ErrorAction SilentlyContinue | Sort-Object FullName
$inventory.archive_count = @($archives).Count

foreach ($archive in $archives) {
    $record = [ordered]@{
        archive = $archive.FullName
        sha256 = (Get-FileHash -LiteralPath $archive.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        safe = $true
        categories = @()
        model_entries = @()
        texture_entries = @()
        unsupported_entries = @()
        error = $null
    }

    $stageRoot = Join-Path $ReportDir ('safety_probe_' + $record.sha256.Substring(0, 12))
    try {
        $zip = [System.IO.Compression.ZipFile]::OpenRead($archive.FullName)
        try {
            foreach ($entry in $zip.Entries) {
                if (-not (Test-SafeZipEntry -EntryName $entry.FullName -StageRoot $stageRoot)) {
                    $record.safe = $false
                    $record.error = 'unsafe_zip_path:' + $entry.FullName
                    break
                }

                $extension = [System.IO.Path]::GetExtension($entry.FullName).ToLowerInvariant()
                if ($modelExtensions -contains $extension) {
                    $record.model_entries += $entry.FullName
                    $record.categories += Get-Category ($archive.Name + ' ' + $entry.FullName)
                }
                elseif ($textureExtensions -contains $extension) {
                    $record.texture_entries += $entry.FullName
                }
            }
        }
        finally {
            $zip.Dispose()
        }
    }
    catch {
        $record.safe = $false
        $record.error = 'zip_open_error:' + $_.Exception.Message
    }

    if ($record.model_entries.Count -eq 0) {
        $record.categories += Get-Category $archive.Name
    }
    $record.categories = @($record.categories | Sort-Object -Unique)

    if (-not $record.safe) {
        $inventory.unsafe_archive_count++
        Write-Warning ('[MODEL INBOX] UNSAFE/UNREADABLE archive kept unresolved: ' + $archive.FullName + ' ' + $record.error)
    }
    else {
        Write-Host ('[MODEL INBOX] ' + $archive.Name + ' models=' + $record.model_entries.Count + ' textures=' + $record.texture_entries.Count + ' categories=' + ($record.categories -join ','))
    }

    $inventory.archives += $record
}

$inventory.status = if ($inventory.unsafe_archive_count -gt 0) { 'UNSAFE_ARCHIVE_PRESENT' } else { 'PASS' }
$inventory | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $ReportPath -Encoding UTF8
Write-Host ('[MODEL INBOX] Inventory report: ' + $ReportPath)

if ($inventory.unsafe_archive_count -gt 0) {
    Write-Host ('[MODEL INBOX] STOP: unsafe/unreadable archives=' + $inventory.unsafe_archive_count) -ForegroundColor Red
    exit 40
}

Write-Host ('[MODEL INBOX] PASS: audited all local ZIP archives=' + $inventory.archive_count)
exit 0
