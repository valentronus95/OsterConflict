param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectDir
)

$ErrorActionPreference = 'Stop'

$ProjectDir = [System.IO.Path]::GetFullPath($ProjectDir)
$RepoRoot = Split-Path -Parent $ProjectDir
$LocalModelInbox = Join-Path $RepoRoot 'models_game_OC'
$SourceRoot = Join-Path $ProjectDir 'SourceAssets\Production'
$CacheRoot = Join-Path $ProjectDir 'Saved\LocalProductionSourceIntake'

$HmmwvTarget = Join-Path $SourceRoot 'Vehicles\HMMWV\ukrainian_hmmwv_mk_19.glb'
$M2Target = Join-Path $SourceRoot 'Weapons\M2\m2_50cal_machinegun_cc0.glb'
$BtrTarget = Join-Path $SourceRoot 'Vehicles\BTR4\BTR4_Bucephalus.fbx'
$BtrTextureTarget = Join-Path $SourceRoot 'Vehicles\BTR4\Textures'

$BtrTextures = @(
    'Bahnya_low_albedo.png',
    'Koleso_low_albedo.png',
    'Korpus_low_albedo.png',
    'Windows_low_albedo.png',
    'interior.png',
    'tire.png'
)

function Ensure-Parent([string]$Path) {
    $parent = Split-Path -Parent $Path
    if (-not (Test-Path -LiteralPath $parent)) {
        New-Item -ItemType Directory -Path $parent -Force | Out-Null
    }
}

function Missing-Targets {
    $missing = @()
    if (-not (Test-Path -LiteralPath $HmmwvTarget)) { $missing += 'HMMWV GLB' }
    if (-not (Test-Path -LiteralPath $M2Target)) { $missing += 'M2 Browning GLB' }
    if (-not (Test-Path -LiteralPath $BtrTarget)) { $missing += 'BTR-4 FBX' }
    foreach ($textureName in $BtrTextures) {
        if (-not (Test-Path -LiteralPath (Join-Path $BtrTextureTarget $textureName))) {
            $missing += ('BTR texture ' + $textureName)
        }
    }
    return $missing
}

function Find-FirstMatchingFile {
    param(
        [string[]]$Roots,
        [string]$ExactName,
        [string]$Regex
    )

    foreach ($root in $Roots) {
        if (-not $root -or -not (Test-Path -LiteralPath $root)) { continue }

        $exact = Get-ChildItem -LiteralPath $root -Recurse -File -Filter $ExactName -ErrorAction SilentlyContinue |
            Sort-Object Length -Descending |
            Select-Object -First 1
        if ($exact) { return $exact }

        $match = Get-ChildItem -LiteralPath $root -Recurse -File -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -match $Regex } |
            Sort-Object Length -Descending |
            Select-Object -First 1
        if ($match) { return $match }
    }

    return $null
}

function Copy-IfMissing {
    param(
        [string]$Target,
        $Source
    )

    if ((Test-Path -LiteralPath $Target) -or -not $Source) { return }
    Ensure-Parent $Target
    Copy-Item -LiteralPath $Source.FullName -Destination $Target -Force
    Write-Host ('[SOURCE] Restored: ' + $Target + ' <= ' + $Source.FullName)
}

function Restore-BtrTexturesFromRoots {
    param([string[]]$Roots)

    if (-not (Test-Path -LiteralPath $BtrTextureTarget)) {
        New-Item -ItemType Directory -Path $BtrTextureTarget -Force | Out-Null
    }
    foreach ($textureName in $BtrTextures) {
        $textureTarget = Join-Path $BtrTextureTarget $textureName
        if (Test-Path -LiteralPath $textureTarget) { continue }
        $texture = Find-FirstMatchingFile -Roots $Roots -ExactName $textureName -Regex ('(?i)^' + [regex]::Escape($textureName) + '$')
        if ($texture) {
            Copy-Item -LiteralPath $texture.FullName -Destination $textureTarget -Force
            Write-Host ('[SOURCE] Restored BTR texture: ' + $textureName)
        }
    }
}

function Expand-ArchiveSafe {
    param(
        [System.IO.FileInfo]$Archive,
        [string]$Destination
    )

    if (-not $Archive) { return }
    if (Test-Path -LiteralPath $Destination) {
        Remove-Item -LiteralPath $Destination -Recurse -Force
    }
    New-Item -ItemType Directory -Path $Destination -Force | Out-Null
    Write-Host ('[SOURCE] Expanding audited local model archive: ' + $Archive.FullName)
    Expand-Archive -LiteralPath $Archive.FullName -DestinationPath $Destination -Force
}

function Find-BtrFbxInNamedArchive {
    param(
        [System.IO.FileInfo]$Archive,
        [string]$Stage
    )

    if (-not $Archive -or -not (Test-Path -LiteralPath $Stage)) { return $null }

    $targeted = Find-FirstMatchingFile -Roots @($Stage) -ExactName 'BTR4_Bucephalus.fbx' -Regex '(?i)(btr.?4|bucephalus|буцеф).*\.fbx$'
    if ($targeted) { return $targeted }

    # The archive context is authoritative for a generic source.fbx/model.fbx filename only when
    # the archive itself is clearly BTR-4/Bucephalus-labelled. Never take a generic FBX from an unrelated ZIP.
    if ($Archive.Name -match '(?i)(btr.?4|bucephalus|буцеф)') {
        $genericFbx = Get-ChildItem -LiteralPath $Stage -Recurse -File -Filter '*.fbx' -ErrorAction SilentlyContinue |
            Sort-Object Length -Descending |
            Select-Object -First 1
        if ($genericFbx) {
            Write-Host ('[SOURCE] BTR-labelled archive contains generic FBX: ' + $genericFbx.FullName)
            return $genericFbx
        }
    }

    return $null
}

$missing = Missing-Targets
if ($missing.Count -eq 0) {
    Write-Host '[SOURCE] PASS: local HMMWV, M2 Browning, BTR-4 source and BTR textures are already present.'
    exit 0
}

Write-Host ('[SOURCE] Missing local production source(s): ' + ($missing -join ', '))
Write-Host '[SOURCE] Searching the dedicated models_game_OC inbox first, then existing local download locations. Nothing is uploaded or committed.'

$searchRoots = @(
    $LocalModelInbox,
    $SourceRoot,
    (Join-Path $env:USERPROFILE 'Downloads'),
    (Join-Path $env:USERPROFILE 'Desktop'),
    (Join-Path $env:USERPROFILE 'Documents')
) | Where-Object { $_ -and (Test-Path -LiteralPath $_) } | Select-Object -Unique

$hmmwv = Find-FirstMatchingFile -Roots $searchRoots -ExactName 'ukrainian_hmmwv_mk_19.glb' -Regex '(?i)(hmmwv|humvee|hummer).*\.glb$'
$m2 = Find-FirstMatchingFile -Roots $searchRoots -ExactName 'm2_50cal_machinegun_cc0.glb' -Regex '(?i)(m2|browning|50.?cal).*\.glb$'
$btr = Find-FirstMatchingFile -Roots $searchRoots -ExactName 'BTR4_Bucephalus.fbx' -Regex '(?i)(btr.?4|bucephalus|буцеф).*\.fbx$'

Copy-IfMissing -Target $HmmwvTarget -Source $hmmwv
Copy-IfMissing -Target $M2Target -Source $m2
Copy-IfMissing -Target $BtrTarget -Source $btr
Restore-BtrTexturesFromRoots -Roots $searchRoots

$missing = Missing-Targets
if ($missing.Count -gt 0) {
    $preferredNames = @(
        'OsterConflict_vehicle_assets_ready.zip',
        'моделі.zip'
    )

    $archives = @()

    # The dedicated inbox is explicit user intent. Inspect every ZIP there, not only archives whose
    # filenames happen to contain an old hard-coded vehicle keyword.
    if (Test-Path -LiteralPath $LocalModelInbox) {
        $archives += Get-ChildItem -LiteralPath $LocalModelInbox -Recurse -File -Filter '*.zip' -ErrorAction SilentlyContinue
    }

    # Outside the inbox remain conservative: only known/preferred production-model archive names are considered.
    foreach ($root in ($searchRoots | Where-Object { $_ -ne $LocalModelInbox })) {
        foreach ($name in $preferredNames) {
            $candidate = Join-Path $root $name
            if (Test-Path -LiteralPath $candidate) {
                $archives += Get-Item -LiteralPath $candidate
            }
        }

        $archives += Get-ChildItem -LiteralPath $root -Recurse -File -Filter '*.zip' -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -match '(?i)(oster|vehicle|model|модел|btr|hmmwv|m2|browning)' }
    }

    $archives = $archives | Sort-Object FullName -Unique
    $archiveIndex = 0
    foreach ($archive in $archives) {
        if ((Missing-Targets).Count -eq 0) { break }

        $archiveIndex++
        $stage = Join-Path $CacheRoot ('archive_' + $archiveIndex)
        try {
            Expand-ArchiveSafe -Archive $archive -Destination $stage
        }
        catch {
            Write-Warning ('Could not expand ' + $archive.FullName + ': ' + $_.Exception.Message)
            continue
        }

        $nestedIndex = 0
        foreach ($nested in (Get-ChildItem -LiteralPath $stage -Recurse -File -Filter '*.zip' -ErrorAction SilentlyContinue)) {
            $nestedIndex++
            try {
                Expand-ArchiveSafe -Archive $nested -Destination (Join-Path $stage ('nested_' + $nestedIndex))
            }
            catch {
                Write-Warning ('Could not expand nested archive ' + $nested.FullName + ': ' + $_.Exception.Message)
            }
        }

        $roots = @($stage)
        Copy-IfMissing -Target $HmmwvTarget -Source (Find-FirstMatchingFile -Roots $roots -ExactName 'ukrainian_hmmwv_mk_19.glb' -Regex '(?i)(hmmwv|humvee|hummer).*\.glb$')
        Copy-IfMissing -Target $M2Target -Source (Find-FirstMatchingFile -Roots $roots -ExactName 'm2_50cal_machinegun_cc0.glb' -Regex '(?i)(m2|browning|50.?cal).*\.glb$')
        if (-not (Test-Path -LiteralPath $BtrTarget)) {
            Copy-IfMissing -Target $BtrTarget -Source (Find-BtrFbxInNamedArchive -Archive $archive -Stage $stage)
        }
        Restore-BtrTexturesFromRoots -Roots $roots
    }
}

$missing = Missing-Targets
if ($missing.Count -gt 0) {
    Write-Host ''
    Write-Host ('[SOURCE] CONTENT GAP: still missing ' + ($missing -join ', ')) -ForegroundColor Yellow
    Write-Host '[SOURCE] Expected canonical local files:'
    Write-Host ('  ' + $HmmwvTarget)
    Write-Host ('  ' + $M2Target)
    Write-Host ('  ' + $BtrTarget)
    foreach ($textureName in $BtrTextures) {
        Write-Host ('  ' + (Join-Path $BtrTextureTarget $textureName))
    }
    Write-Host '[SOURCE] Other inbox models remain in the inventory for their own gameplay/world integration pass; they are never silently called READY.'
    exit 20
}

Write-Host '[SOURCE] PASS: required production vehicle model sources and BTR textures are now available locally.'
exit 0
