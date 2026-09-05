param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectDir
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression.FileSystem

$ProjectDir = [System.IO.Path]::GetFullPath($ProjectDir)
$RepoRoot = Split-Path -Parent $ProjectDir
$Inbox = Join-Path $RepoRoot 'models_game_OC'
$ContentRoot = Join-Path $ProjectDir 'Content'
$PluginsRoot = Join-Path $ProjectDir 'Plugins'
$StateRoot = Join-Path $ProjectDir 'Saved\LocalModelInbox'
$ExtractRoot = Join-Path $StateRoot 'Extracted'
$ManifestPath = Join-Path $StateRoot 'prepared_sources.json'

New-Item -ItemType Directory -Path $StateRoot -Force | Out-Null
New-Item -ItemType Directory -Path $ExtractRoot -Force | Out-Null

$modelExtensions = @('.fbx', '.glb', '.gltf', '.obj', '.dae', '.blend')
$imageExtensions = @('.png', '.tga', '.jpg', '.jpeg', '.bmp', '.exr', '.dds')
$uePackageExtensions = @('.uasset', '.umap', '.uexp', '.ubulk', '.uptnl')

function Get-Category([string]$Text) {
    $v = $Text.ToLowerInvariant()
    # Keep source ASCII-only because Windows PowerShell 5.1 treats UTF-8-without-BOM as ANSI.
    # .NET regex understands \uXXXX escapes, so Ukrainian filename matching is preserved.
    if ($v -match 'hud|heads.?up|crosshair|reticle|minimap|health.?bar|ammo.?ui|overlay|interface|compass|scope.?ui') { return 'HUD_UI' }
    if ($v -match 'btr.?4|bucephal|\u0431\u0443\u0446\u0435\u0444') { return 'BTR4' }
    if ($v -match 'hmmwv|humvee|hummer') { return 'HMMWV' }
    if ($v -match '(^|[^a-z0-9])m2([^a-z0-9]|$)|browning|50.?cal') { return 'M2' }
    if ($v -match 'm249|minimi') { return 'M249' }
    if ($v -match 'remington|870') { return 'REMINGTON870' }
    if ($v -match 'm16|m4a1|(^|[^a-z0-9])m4([^a-z0-9]|$)') { return 'M16_M4' }
    if ($v -match 'ak.?47|(^|[^a-z0-9])akm([^a-z0-9]|$)') { return 'AK47' }
    if ($v -match '(^|[^a-z0-9])mp5([^a-z0-9]|$)') { return 'MP5' }
    if ($v -match 'm?1911') { return 'M1911' }
    if ($v -match 'm700|remington.?700') { return 'M700' }
    if ($v -match '(^|[^a-z0-9])m14([^a-z0-9]|$)') { return 'M14' }
    if ($v -match 'mac.?10') { return 'MAC10' }
    if ($v -match 'tec.?9') { return 'TEC9' }
    if ($v -match 'lever|winchester') { return 'LEVER_ACTION' }
    if ($v -match 'rpg|launcher|rocket') { return 'LAUNCHER' }
    if ($v -match 'rifle|weapon|gun|pistol|shotgun|smg|sniper') { return 'WEAPON_OTHER' }
    if ($v -match 'pickup|pick.?up|technical|hilux|truck') { return 'PICKUP' }
    if ($v -match 'skin|character|soldier|human|mannequin|uniform|operator|fighter|\u043f\u0435\u0440\u0441\u043e\u043d\u0430|\u0441\u043e\u043b\u0434\u0430\u0442|\u0441\u043a\u0456\u043d|\u043b\u044e\u0434\u0438\u043d') { return 'CHARACTER_SKIN' }
    if ($v -match 'tree|foliage|grass|bush|vegetation|plant|flower|mushroom|treestump|\u0434\u0435\u0440\u0435\u0432|\u043a\u0443\u0449|\u0442\u0440\u0430\u0432\u0430') { return 'FOLIAGE' }
    if ($v -match 'prop|furniture|chair|table|barrel|crate|fence|bridge|lamp|light|bench|ladder|plank|wheel|whell|bowl|cauldron|kettle|mug|spoon|bucket|pot|sack|cart|axe|boat|well|torch|hay|log|stone|\u043c\u0435\u0431\u043b|\u043f\u0440\u043e\u043f|\u043f\u0430\u0440\u043a\u0430\u043d') { return 'PROP' }
    if ($v -match 'river|canal|stream|pond|waterway') { return 'WATER_WORLD' }
    if ($v -match 'road|sidewalk|pavement|pathway') { return 'ROAD_WORLD' }
    if ($v -match 'terrain|ground|landscape|mud|moss|field') { return 'GROUND_WORLD' }
    if ($v -match 'building|house|home|hut|roof|wall|porch|balcony|shed|hovel|tower|museum|silpo|stadium|culture|college|street|town|village|\u0431\u0443\u0434\u0438\u043d|\u043c\u0443\u0437\u0435\u0439|\u0441\u0442\u0430\u0434\u0456\u043e\u043d|\u0432\u0443\u043b\u0438\u0446') { return 'BUILDING_WORLD' }
    return 'UNCLASSIFIED'
}

function Test-SafeZipEntry([string]$EntryName, [string]$Destination) {
    if ([string]::IsNullOrWhiteSpace($EntryName)) { return $false }
    $normalized = $EntryName.Replace('\', '/')
    if ($normalized.StartsWith('/') -or $normalized -match '^[A-Za-z]:') { return $false }
    if ($normalized.Split('/') | Where-Object { $_ -eq '..' }) { return $false }
    $root = [System.IO.Path]::GetFullPath($Destination)
    if (-not $root.EndsWith([System.IO.Path]::DirectorySeparatorChar)) { $root += [System.IO.Path]::DirectorySeparatorChar }
    $candidate = [System.IO.Path]::GetFullPath((Join-Path $Destination $normalized))
    return $candidate.StartsWith($root, [System.StringComparison]::OrdinalIgnoreCase)
}

function Expand-SafeZip([System.IO.FileInfo]$Archive, [string]$Destination) {
    $zip = [System.IO.Compression.ZipFile]::OpenRead($Archive.FullName)
    try {
        foreach ($entry in $zip.Entries) {
            if (-not (Test-SafeZipEntry -EntryName $entry.FullName -Destination $Destination)) {
                throw ('unsafe_zip_path:' + $entry.FullName)
            }
        }
    }
    finally { $zip.Dispose() }

    New-Item -ItemType Directory -Path $Destination -Force | Out-Null
    Expand-Archive -LiteralPath $Archive.FullName -DestinationPath $Destination -Force
}

function Get-DeployTarget([string]$SourcePath, [string]$StageRoot) {
    # Path.GetRelativePath is unavailable in Windows PowerShell 5.1 / .NET Framework.
    $root = [System.IO.Path]::GetFullPath($StageRoot)
    if (-not $root.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
        $root += [System.IO.Path]::DirectorySeparatorChar
    }
    $sourceFull = [System.IO.Path]::GetFullPath($SourcePath)
    if (-not $sourceFull.StartsWith($root, [System.StringComparison]::OrdinalIgnoreCase)) {
        return $null
    }
    $relative = $sourceFull.Substring($root.Length).Replace('\', '/')
    $parts = $relative.Split('/')

    $contentIndex = [Array]::IndexOf($parts, 'Content')
    if ($contentIndex -ge 0 -and $contentIndex -lt ($parts.Length - 1)) {
        $after = ($parts[($contentIndex + 1)..($parts.Length - 1)] -join [System.IO.Path]::DirectorySeparatorChar)
        return [ordered]@{ kind='Content'; target=(Join-Path $ContentRoot $after); package_relative=$after }
    }

    $pluginsIndex = [Array]::IndexOf($parts, 'Plugins')
    if ($pluginsIndex -ge 0 -and $pluginsIndex -lt ($parts.Length - 1)) {
        $after = ($parts[($pluginsIndex + 1)..($parts.Length - 1)] -join [System.IO.Path]::DirectorySeparatorChar)
        return [ordered]@{ kind='Plugin'; target=(Join-Path $PluginsRoot $after); package_relative=$after }
    }

    # Content-only packs such as AdvancedVillagePack.zip often start directly with PackName/Meshes/...
    # Preserve that root exactly under /Game so internal package references remain valid.
    if ($relative -notmatch '(?i)^(Config|Source|Binaries|Intermediate|Saved|Documentation|Docs)/' -and
        ([System.IO.Path]::GetExtension($relative).ToLowerInvariant() -in $uePackageExtensions)) {
        return [ordered]@{ kind='Content'; target=(Join-Path $ContentRoot ($relative.Replace('/', [System.IO.Path]::DirectorySeparatorChar))); package_relative=$relative }
    }

    return $null
}

function Copy-PackageFile([string]$Source, $DeployInfo, [System.Collections.Generic.List[string]]$Conflicts) {
    $target = $DeployInfo.target
    $parent = Split-Path -Parent $target
    New-Item -ItemType Directory -Path $parent -Force | Out-Null

    if (Test-Path -LiteralPath $target) {
        $sourceHash = (Get-FileHash -LiteralPath $Source -Algorithm SHA256).Hash
        $targetHash = (Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash
        if ($sourceHash -ne $targetHash) {
            $Conflicts.Add(($target + ' <= ' + $Source))
            return 'CONFLICT'
        }
        return 'ALREADY_PRESENT'
    }

    Copy-Item -LiteralPath $Source -Destination $target -Force
    return 'COPIED'
}

$manifest = [ordered]@{
    schema = 'oster-conflict-pass45-all-local-inbox-prepared-v1'
    inbox = $Inbox
    archives = @()
    loose_sources = @()
    ue_packages = @()
    raw_models = @()
    hud_images = @()
    conflicts = @()
    status = 'IN_PROGRESS'
}
$conflicts = New-Object 'System.Collections.Generic.List[string]'

if (-not (Test-Path -LiteralPath $Inbox)) {
    $manifest.status = 'NO_INBOX'
    $manifest | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $ManifestPath -Encoding UTF8
    Write-Host ('[ALL INBOX] No models_game_OC folder found: ' + $Inbox)
    exit 0
}

# Loose files are part of the same contract as archived files.
foreach ($file in (Get-ChildItem -LiteralPath $Inbox -Recurse -File -ErrorAction SilentlyContinue | Where-Object { $_.Extension.ToLowerInvariant() -ne '.zip' })) {
    $ext = $file.Extension.ToLowerInvariant()
    $category = Get-Category $file.FullName
    $record = [ordered]@{ source=$file.FullName; category=$category; extension=$ext; archive=$null }
    $manifest.loose_sources += $record

    # Loose UE-ready packages obey the same rule as ZIP content: deploy them into the real project
    # while preserving their /Game-relative path. Dropping a .uasset beside a ZIP must not leave it inert.
    if ($uePackageExtensions -contains $ext) {
        $deploy = Get-DeployTarget -SourcePath $file.FullName -StageRoot $Inbox
        if ($deploy) {
            $copyResult = Copy-PackageFile -Source $file.FullName -DeployInfo $deploy -Conflicts $conflicts
            $manifest.ue_packages += [ordered]@{
                source=$file.FullName; archive=$null; category=$category; extension=$ext;
                deploy_kind=$deploy.kind; target=$deploy.target; package_relative=$deploy.package_relative; result=$copyResult
            }
        }
        continue
    }

    if ($modelExtensions -contains $ext) { $manifest.raw_models += $record }
    if (($imageExtensions -contains $ext) -and $category -eq 'HUD_UI') { $manifest.hud_images += $record }
}

$archives = Get-ChildItem -LiteralPath $Inbox -Recurse -File -Filter '*.zip' -ErrorAction SilentlyContinue | Sort-Object FullName
$archiveQueue = New-Object System.Collections.Queue
foreach ($archive in $archives) { $archiveQueue.Enqueue([ordered]@{ file=$archive; depth=0; parent=$null }) }
$seenArchiveHashes = @{}
$archiveOrdinal = 0

while ($archiveQueue.Count -gt 0) {
    $item = $archiveQueue.Dequeue()
    $archive = $item.file
    $hash = (Get-FileHash -LiteralPath $archive.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($seenArchiveHashes.ContainsKey($hash)) { continue }
    $seenArchiveHashes[$hash] = $true

    if ($item.depth -gt 4) {
        $manifest.archives += [ordered]@{
            archive=$archive.FullName; sha256=$hash; status='NESTED_DEPTH_LIMIT';
            error='nested_zip_depth_limit_exceeded'; depth=$item.depth; parent=$item.parent
        }
        Write-Host ('[ALL INBOX] STOP nested ZIP depth limit exceeded: ' + $archive.FullName) -ForegroundColor Red
        continue
    }

    $archiveOrdinal++
    $stage = Join-Path $ExtractRoot ($hash.Substring(0,16))
    $marker = Join-Path $stage '.oc_extracted_ok'

    try {
        if (-not (Test-Path -LiteralPath $marker)) {
            if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
            Expand-SafeZip -Archive $archive -Destination $stage
            Set-Content -LiteralPath $marker -Value $hash -Encoding ASCII
        }
    }
    catch {
        $manifest.archives += [ordered]@{ archive=$archive.FullName; sha256=$hash; status='UNSAFE_OR_UNREADABLE'; error=$_.Exception.Message; depth=$item.depth }
        Write-Host ('[ALL INBOX] STOP archive: ' + $archive.FullName + ' ' + $_.Exception.Message) -ForegroundColor Red
        continue
    }

    $archiveRecord = [ordered]@{ archive=$archive.FullName; sha256=$hash; status='EXTRACTED'; stage=$stage; depth=$item.depth; parent=$item.parent }
    $manifest.archives += $archiveRecord

    foreach ($nested in (Get-ChildItem -LiteralPath $stage -Recurse -File -Filter '*.zip' -ErrorAction SilentlyContinue)) {
        $archiveQueue.Enqueue([ordered]@{ file=$nested; depth=($item.depth + 1); parent=$archive.FullName })
    }

    foreach ($source in (Get-ChildItem -LiteralPath $stage -Recurse -File -ErrorAction SilentlyContinue)) {
        if ($source.Name -eq '.oc_extracted_ok') { continue }
        $ext = $source.Extension.ToLowerInvariant()
        $category = Get-Category ($archive.Name + ' ' + $source.FullName)

        if ($uePackageExtensions -contains $ext) {
            $deploy = Get-DeployTarget -SourcePath $source.FullName -StageRoot $stage
            if ($deploy) {
                $copyResult = Copy-PackageFile -Source $source.FullName -DeployInfo $deploy -Conflicts $conflicts
                $manifest.ue_packages += [ordered]@{
                    source=$source.FullName; archive=$archive.FullName; category=$category; extension=$ext;
                    deploy_kind=$deploy.kind; target=$deploy.target; package_relative=$deploy.package_relative; result=$copyResult
                }
            }
            continue
        }

        if ($modelExtensions -contains $ext) {
            $manifest.raw_models += [ordered]@{ source=$source.FullName; archive=$archive.FullName; category=$category; extension=$ext }
            continue
        }

        if (($imageExtensions -contains $ext) -and $category -eq 'HUD_UI') {
            $manifest.hud_images += [ordered]@{ source=$source.FullName; archive=$archive.FullName; category=$category; extension=$ext }
        }
    }
}

$manifest.conflicts = @($conflicts)
$unsafeCount = @($manifest.archives | Where-Object { $_.status -ne 'EXTRACTED' }).Count
if ($unsafeCount -gt 0) { $manifest.status = 'UNSAFE_ARCHIVE_PRESENT' }
elseif ($conflicts.Count -gt 0) { $manifest.status = 'PACKAGE_CONFLICT' }
else { $manifest.status = 'PASS' }

$manifest | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath $ManifestPath -Encoding UTF8
Write-Host ('[ALL INBOX] prepared archives=' + $manifest.archives.Count + ' raw_models=' + $manifest.raw_models.Count + ' ue_package_files=' + $manifest.ue_packages.Count + ' hud_images=' + $manifest.hud_images.Count)
Write-Host ('[ALL INBOX] manifest=' + $ManifestPath)

if ($unsafeCount -gt 0) { exit 40 }
if ($conflicts.Count -gt 0) {
    Write-Host ('[ALL INBOX] STOP: package path conflicts=' + $conflicts.Count + '. Refusing to overwrite unrelated project assets.') -ForegroundColor Red
    exit 41
}
exit 0