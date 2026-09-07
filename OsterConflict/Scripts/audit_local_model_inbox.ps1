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

    # Keep source ASCII-only because Windows PowerShell 5.1 treats UTF-8-without-BOM as ANSI.
    # .NET regex understands \uXXXX escapes, so Ukrainian filename matching is preserved.
    if ($value -match 'btr.?4|bucephal|\u0431\u0443\u0446\u0435\u0444') { $categories.Add('BTR4') }
    if ($value -match 'hmmwv|humvee|hummer') { $categories.Add('HMMWV') }
    if ($value -match '(^|[^a-z0-9])m2([^a-z0-9]|$)|browning|50.?cal') { $categories.Add('M2') }
    if ($value -match 'm249|minimi') { $categories.Add('M249') }
    if ($value -match 'remington|870') { $categories.Add('REMINGTON870') }
    if ($value -match 'm16|m4a1|(^|[^a-z0-9])m4([^a-z0-9]|$)') { $categories.Add('M16_M4') }
    if ($value -match '(^|[^a-z0-9])ak[ _.-]?74(m)?([^a-z0-9]|$)') { $categories.Add('AK74') }
    if ($value -match '(^|[^a-z0-9])ak[ _.-]?47([^a-z0-9]|$)|(^|[^a-z0-9])akm([^a-z0-9]|$)') { $categories.Add('AK47') }
    if ($value -match '(^|[^a-z0-9])ar[ _.-]?15([^a-z0-9]|$)') { $categories.Add('AR15') }
    if ($value -match 'ballista') { $categories.Add('BALLISTA') }
    if ($value -match 'kar[ _.-]?98(k)?') { $categories.Add('KAR98') }
    if ($value -match 'makarov') { $categories.Add('MAKAROV') }
    if ($value -match '(^|[^a-z0-9])m[ _.-]?72([^a-z0-9]|$)|m72[ _.-]?law') { $categories.Add('M72') }
    if ($value -match '(^|[^a-z0-9])mp5([^a-z0-9]|$)') { $categories.Add('MP5') }
    if ($value -match 'm?1911') { $categories.Add('M1911') }
    if ($value -match 'm700|remington.?700') { $categories.Add('M700') }
    if ($value -match '(^|[^a-z0-9])m14([^a-z0-9]|$)') { $categories.Add('M14') }
    if ($value -match 'mac.?10') { $categories.Add('MAC10') }
    if ($value -match 'tec.?9') { $categories.Add('TEC9') }
    if ($value -match 'lever|winchester') { $categories.Add('LEVER_ACTION') }
    if ($value -match 'rpg|rocket[ _.-]?launcher|grenade[ _.-]?launcher|bazooka|panzerfaust|at4') { $categories.Add('LAUNCHER_GENERIC') }
    if ($value -match 'scar|hk[ _.-]?416|g36|famas|(^|[^a-z0-9])aug([^a-z0-9]|$)|galil|acr|assault[ _.-]?rifle') { $categories.Add('ASSAULT_GENERIC') }
    if ($value -match 'sniper|dragunov|(^|[^a-z0-9])svd([^a-z0-9]|$)|(^|[^a-z0-9])awp([^a-z0-9]|$)|m24|m40|barrett') { $categories.Add('SNIPER_GENERIC') }
    if ($value -match 'shotgun|mossberg|benelli|spas[ _.-]?12|saiga[ _.-]?12') { $categories.Add('SHOTGUN_GENERIC') }
    if ($value -match 'submachine|(^|[^a-z0-9])smg([^a-z0-9]|$)|(^|[^a-z0-9])uzi([^a-z0-9]|$)|p90|ump[ _.-]?45|vector') { $categories.Add('SMG_GENERIC') }
    if ($value -match 'pistol|handgun|glock|beretta|desert[ _.-]?eagle|deagle|sig[ _.-]?sauer|usp') { $categories.Add('PISTOL_GENERIC') }
    if ($value -match 'light[ _.-]?machine[ _.-]?gun|machine[ _.-]?gun|(^|[^a-z0-9])lmg([^a-z0-9]|$)|rpk|pkm') { $categories.Add('LMG_GENERIC') }
    if ($value -match 'rifle|carbine|weapon|gun') { $categories.Add('WEAPON_OTHER') }
    if ($value -match 'pickup|pick.?up|technical|hilux|truck') { $categories.Add('PICKUP') }
    if ($value -match 'skin|character|soldier|human|mannequin|uniform|operator|fighter|\u043f\u0435\u0440\u0441\u043e\u043d\u0430|\u0441\u043e\u043b\u0434\u0430\u0442|\u0441\u043a\u0456\u043d|\u043b\u044e\u0434\u0438\u043d') { $categories.Add('CHARACTER_SKIN') }
    if ($value -match 'building|house|home|hut|roof|wall|porch|balcony|shed|tower|museum|silpo|stadium|culture|college|street|town|village|\u0431\u0443\u0434\u0438\u043d|\u043c\u0443\u0437\u0435\u0439|\u0441\u0442\u0430\u0434\u0456\u043e\u043d|\u0432\u0443\u043b\u0438\u0446') { $categories.Add('BUILDING_WORLD') }
    if ($value -match 'tree|foliage|grass|bush|vegetation|plant|flower|\u0434\u0435\u0440\u0435\u0432|\u043a\u0443\u0449|\u0442\u0440\u0430\u0432\u0430') { $categories.Add('FOLIAGE') }
    if ($value -match 'prop|furniture|chair|table|barrel|crate|fence|bridge|lamp|bench|ladder|plank|wheel|bucket|sack|cart|axe|boat|well|torch|hay|log|stone|\u043c\u0435\u0431\u043b|\u043f\u0440\u043e\u043f|\u043f\u0430\u0440\u043a\u0430\u043d') { $categories.Add('PROP') }
    if ($value -match 'hud|heads.?up|crosshair|reticle|minimap|health.?bar|ammo.?ui|overlay|interface|compass|scope.?ui') { $categories.Add('HUD_UI') }

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
    schema = 'oster-conflict-pass45-local-model-inbox-v2'
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

if ($inventory.unsafe_archive_count -gt 0) {
    $inventory.status = 'UNSAFE_ARCHIVE_PRESENT'
}
else {
    $inventory.status = 'PASS'
}
$inventory | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $ReportPath -Encoding UTF8
Write-Host ('[MODEL INBOX] Inventory report: ' + $ReportPath)

if ($inventory.unsafe_archive_count -gt 0) {
    Write-Host ('[MODEL INBOX] STOP: unsafe/unreadable archives=' + $inventory.unsafe_archive_count) -ForegroundColor Red
    exit 40
}

Write-Host ('[MODEL INBOX] PASS: audited all local ZIP archives=' + $inventory.archive_count)
exit 0
