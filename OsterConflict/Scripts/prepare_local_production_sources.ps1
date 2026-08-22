param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectDir
)

$ErrorActionPreference = 'Stop'

$ProjectDir = [System.IO.Path]::GetFullPath($ProjectDir)
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
    Write-Host ('[SOURCE] Restored: ' + $Target)
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
    Write-Host ('[SOURCE] Expanding local model archive: ' + $Archive.FullName)
    Expand-Archive -LiteralPath $Archive.FullName -DestinationPath $Destination -Force
}

$missing = Missing-Targets
if ($missing.Count -eq 0) {
    Write-Host '[SOURCE] PASS: local HMMWV, M2 Browning, BTR-4 source and BTR textures are already present.'
    exit 0
}

Write-Host ('[SOURCE] Missing local production source(s): ' + ($missing -join ', '))
Write-Host '[SOURCE] Searching previously downloaded model files and archives. Nothing is uploaded or committed.'

# Search the canonical source tree too. This catches original downloads copied into the project under
# their source filenames. The three common Windows folders cover local intake without crawling the profile.
$searchRoots = @(
    $SourceRoot,
    (Join-Path $env:USERPROFILE 'Downloads'),
    (Join-Path $env:USERPROFILE 'Desktop'),
    (Join-Path $env:USERPROFILE 'Documents')
) | Where-Object { $_ -and (Test-Path -LiteralPath $_) } | Select-Object -Unique

$hmmwv = Find-FirstMatchingFile -Roots $searchRoots -ExactName 'ukrainian_hmmwv_mk_19.glb' -Regex '(?i)(hmmwv|humvee).*\.glb$'
$m2 = Find-FirstMatchingFile -Roots $searchRoots -ExactName 'm2_50cal_machinegun_cc0.glb' -Regex '(?i)(m2|50.?cal).*\.glb$'
$btr = Find-FirstMatchingFile -Roots $searchRoots -ExactName 'BTR4_Bucephalus.fbx' -Regex '(?i)(btr.?4|bucephalus).*\.fbx$'

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
    foreach ($root in $searchRoots) {
        foreach ($name in $preferredNames) {
            $candidate = Join-Path $root $name
            if (Test-Path -LiteralPath $candidate) {
                $archives += Get-Item -LiteralPath $candidate
            }
        }

        $archives += Get-ChildItem -LiteralPath $root -Recurse -File -Filter '*.zip' -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -match '(?i)(oster|vehicle|model|модел|btr|hmmwv|m2)' }
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

        # Some user packages contain the BTR package as a ZIP inside the outer archive.
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
        Copy-IfMissing -Target $HmmwvTarget -Source (Find-FirstMatchingFile -Roots $roots -ExactName 'ukrainian_hmmwv_mk_19.glb' -Regex '(?i)(hmmwv|humvee).*\.glb$')
        Copy-IfMissing -Target $M2Target -Source (Find-FirstMatchingFile -Roots $roots -ExactName 'm2_50cal_machinegun_cc0.glb' -Regex '(?i)(m2|50.?cal).*\.glb$')
        Copy-IfMissing -Target $BtrTarget -Source (Find-FirstMatchingFile -Roots $roots -ExactName 'BTR4_Bucephalus.fbx' -Regex '(?i)(btr.?4|bucephalus).*\.fbx$')
        Restore-BtrTexturesFromRoots -Roots $roots
    }
}

$missing = Missing-Targets
if ($missing.Count -gt 0) {
    Write-Host ''
    Write-Host ('[SOURCE] STOP: still missing ' + ($missing -join ', ')) -ForegroundColor Red
    Write-Host '[SOURCE] Expected local files:'
    Write-Host ('  ' + $HmmwvTarget)
    Write-Host ('  ' + $M2Target)
    Write-Host ('  ' + $BtrTarget)
    foreach ($textureName in $BtrTextures) {
        Write-Host ('  ' + (Join-Path $BtrTextureTarget $textureName))
    }
    Write-Host '[SOURCE] Put the original model ZIP/files in Downloads/Desktop/Documents or SourceAssets, then run START_HERE.cmd again.'
    exit 20
}

Write-Host '[SOURCE] PASS: required production model sources and BTR textures are now available locally.'
exit 0
