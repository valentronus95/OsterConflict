[CmdletBinding()]
param(
    [switch]$Force,
    [switch]$SkipLargeFpsKit
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$IntakeRoot = Join-Path $ProjectRoot "SourceAssets\ThirdParty\Gameplay"
$LocalBase = if ($env:LOCALAPPDATA) { $env:LOCALAPPDATA } else { [IO.Path]::GetTempPath() }
$CacheRoot = Join-Path $LocalBase "OsterConflict\GameplayContentCache\2026-08-28"
$ReceiptPath = Join-Path $IntakeRoot "LOCAL_CONTENT_RECEIPT.csv"

New-Item -ItemType Directory -Force -Path $IntakeRoot | Out-Null
New-Item -ItemType Directory -Force -Path $CacheRoot | Out-Null

function Write-Step([string]$Message) {
    Write-Host "[OC Gameplay Intake] $Message"
}

function Get-RelativePathCompat([string]$Root, [string]$FullPath) {
    $rootFull = [IO.Path]::GetFullPath($Root).TrimEnd([char]'\', [char]'/') + [IO.Path]::DirectorySeparatorChar
    $fileFull = [IO.Path]::GetFullPath($FullPath)
    if (-not $fileFull.StartsWith($rootFull, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Path '$fileFull' is outside intake root '$rootFull'."
    }
    return $fileFull.Substring($rootFull.Length).Replace("\", "/")
}

function Assert-Sha256([string]$Path, [string]$ExpectedSha256) {
    if ([string]::IsNullOrWhiteSpace($ExpectedSha256)) {
        return
    }
    $actual = (Get-FileHash -Algorithm SHA256 -LiteralPath $Path).Hash.ToUpperInvariant()
    $expected = $ExpectedSha256.ToUpperInvariant()
    if ($actual -ne $expected) {
        throw "SHA-256 mismatch for '$Path'. Expected $expected, got $actual"
    }
}

function Get-RemoteFile {
    param(
        [Parameter(Mandatory=$true)][string]$Id,
        [Parameter(Mandatory=$true)][string]$Url,
        [Parameter(Mandatory=$true)][string]$Destination,
        [string]$ExpectedSha256 = ""
    )

    $parent = Split-Path -Parent $Destination
    New-Item -ItemType Directory -Force -Path $parent | Out-Null

    if ((Test-Path -LiteralPath $Destination) -and -not $Force) {
        try {
            Assert-Sha256 -Path $Destination -ExpectedSha256 $ExpectedSha256
            Write-Step "$Id already present; verified/skipped."
            return
        }
        catch {
            Write-Step "$Id existing file failed verification; downloading again."
        }
    }

    $temp = "$Destination.download"
    Remove-Item -Force -ErrorAction SilentlyContinue -LiteralPath $temp
    Write-Step "Downloading $Id"
    Invoke-WebRequest -UseBasicParsing -Uri $Url -OutFile $temp -MaximumRedirection 10
    Assert-Sha256 -Path $temp -ExpectedSha256 $ExpectedSha256
    Move-Item -Force -LiteralPath $temp -Destination $Destination
}

function Reset-Directory([string]$Path) {
    if (Test-Path -LiteralPath $Path) {
        Remove-Item -Recurse -Force -LiteralPath $Path
    }
    New-Item -ItemType Directory -Force -Path $Path | Out-Null
}

function Copy-Tree {
    param(
        [Parameter(Mandatory=$true)][string]$Source,
        [Parameter(Mandatory=$true)][string]$Destination
    )
    if (-not (Test-Path -LiteralPath $Source)) {
        throw "Expected extracted source is missing: $Source"
    }
    if ((Test-Path -LiteralPath $Destination) -and $Force) {
        Remove-Item -Recurse -Force -LiteralPath $Destination
    }
    New-Item -ItemType Directory -Force -Path $Destination | Out-Null
    Copy-Item -Recurse -Force -Path (Join-Path $Source "*") -Destination $Destination
}

$armsCommit = "c80a452e680c5a27c9e936176eb17e446869529a"
$armsArchive = Join-Path $CacheRoot "FPS-Arms-3D-$armsCommit.zip"
Get-RemoteFile -Id "OC-INTAKE-ARMS-001" -Url "https://github.com/Ayush-Mohanty/FPS-Arms-3D/archive/$armsCommit.zip" -Destination $armsArchive
$armsExtract = Join-Path $CacheRoot "FPS-Arms-3D-$armsCommit"
if ($Force -or -not (Test-Path -LiteralPath $armsExtract)) {
    Reset-Directory $armsExtract
    Expand-Archive -LiteralPath $armsArchive -DestinationPath $armsExtract -Force
}
$armsRoot = Get-ChildItem -LiteralPath $armsExtract -Directory | Select-Object -First 1
if (-not $armsRoot) { throw "FPS-Arms-3D archive did not contain a root directory." }
Copy-Tree -Source (Join-Path $armsRoot.FullName "Models") -Destination (Join-Path $IntakeRoot "FPSArms3D\Models")

if (-not $SkipLargeFpsKit) {
    $kitCommit = "a19b7458a593598211c95ec46ef4eb4b6d1f94d7"
    $kitArchive = Join-Path $CacheRoot "fps-asset-kit-$kitCommit.zip"
    Get-RemoteFile -Id "OC-INTAKE-FPSKIT-001" -Url "https://github.com/petroulacl/fps-asset-kit/archive/$kitCommit.zip" -Destination $kitArchive
    $kitExtract = Join-Path $CacheRoot "fps-asset-kit-$kitCommit"
    if ($Force -or -not (Test-Path -LiteralPath $kitExtract)) {
        Reset-Directory $kitExtract
        Expand-Archive -LiteralPath $kitArchive -DestinationPath $kitExtract -Force
    }
    $kitRoot = Get-ChildItem -LiteralPath $kitExtract -Directory | Select-Object -First 1
    if (-not $kitRoot) { throw "fps-asset-kit archive did not contain a root directory." }
    Copy-Tree -Source (Join-Path $kitRoot.FullName "sfx") -Destination (Join-Path $IntakeRoot "FPSAssetKit\sfx")
    Copy-Tree -Source (Join-Path $kitRoot.FullName "weapons") -Destination (Join-Path $IntakeRoot "FPSAssetKit\weapons")
}
else {
    Write-Step "Skipping OC-INTAKE-FPSKIT-001 because -SkipLargeFpsKit was supplied."
}

$hardLinesCommit = "7c1f90b0295030e60cf9ea731371156466ddb181"
$hardBase = "https://raw.githubusercontent.com/yegors/hard-lines/$hardLinesCommit/public/audio"
$hardAssets = @(
    @{ Id="OC-INTAKE-SHOTGUN-001-A"; Url="$hardBase/sg-report-0.wav"; Relative="HardLines\Shotgun\sg-report-0.wav" },
    @{ Id="OC-INTAKE-SHOTGUN-001-B"; Url="$hardBase/sg-report-1.wav"; Relative="HardLines\Shotgun\sg-report-1.wav" },
    @{ Id="OC-INTAKE-FOOTSTEP-001-A"; Url="$hardBase/step-hard-0.wav"; Relative="HardLines\Footsteps\step-hard-0.wav" },
    @{ Id="OC-INTAKE-FOOTSTEP-001-B"; Url="$hardBase/step-hard-1.wav"; Relative="HardLines\Footsteps\step-hard-1.wav" },
    @{ Id="OC-INTAKE-FOOTSTEP-001-C"; Url="$hardBase/step-hard-2.wav"; Relative="HardLines\Footsteps\step-hard-2.wav" },
    @{ Id="OC-INTAKE-FOOTSTEP-001-D"; Url="$hardBase/step-hard-3.wav"; Relative="HardLines\Footsteps\step-hard-3.wav" }
)
foreach ($asset in $hardAssets) {
    Get-RemoteFile -Id $asset.Id -Url $asset.Url -Destination (Join-Path $IntakeRoot $asset.Relative)
}

$borderCommit = "38d6afa442a06bbd414f21ac05c0f1b4d08bd705"
$engineBase = "https://raw.githubusercontent.com/yashimosh/border-run/$borderCommit/public/sfx/engine"
$engineFiles = @("loop_0.wav", "loop_1_0.wav", "loop_2_0.wav", "loop_3_0.wav", "loop_4_0.wav", "loop_5_0.wav")
foreach ($name in $engineFiles) {
    Get-RemoteFile -Id "OC-INTAKE-ENGINE-001-$name" -Url "$engineBase/$name" -Destination (Join-Path $IntakeRoot "BorderRun\VehicleEngine\$name")
}

Get-RemoteFile -Id "OC-INTAKE-AMBIENCE-001" -Url "https://opengameart.org/sites/default/files/Forest_Ambience.mp3" -Destination (Join-Path $IntakeRoot "OpenGameArt\Ambience\Forest_Ambience.mp3") -ExpectedSha256 "9850AA1D0D5D66BD9C5DAF8BB77C6D852E01F2F4DE22F283BD5621E8BED13B75"
Get-RemoteFile -Id "OC-INTAKE-FIRE-001" -Url "https://opengameart.org/sites/default/files/fire.wav" -Destination (Join-Path $IntakeRoot "OpenGameArt\Ambience\fire.wav") -ExpectedSha256 "85CA0CC60D0C037FFF8B185E31AD1FCDBDA6CE45EEE17C3EE1318D1B8F59E330"

Get-RemoteFile -Id "FPS-Arms-3D nested AK license" -Url "https://raw.githubusercontent.com/Ayush-Mohanty/FPS-Arms-3D/$armsCommit/Models/fps_ak-74m_animations/license.txt" -Destination (Join-Path $IntakeRoot "FPSArms3D\AK74M_LICENSE_CC_BY_4.0.txt")
Get-RemoteFile -Id "FPS-Arms-3D repository license" -Url "https://raw.githubusercontent.com/Ayush-Mohanty/FPS-Arms-3D/$armsCommit/LICENSE" -Destination (Join-Path $IntakeRoot "FPSArms3D\REPOSITORY_LICENSE.txt")
Get-RemoteFile -Id "hard-lines audio provenance" -Url "https://raw.githubusercontent.com/yegors/hard-lines/$hardLinesCommit/public/audio/README.md" -Destination (Join-Path $IntakeRoot "HardLines\AUDIO_PROVENANCE.md")
Get-RemoteFile -Id "border-run audio provenance" -Url "https://raw.githubusercontent.com/yashimosh/border-run/$borderCommit/CREDITS.md" -Destination (Join-Path $IntakeRoot "BorderRun\CREDITS.md")
Get-RemoteFile -Id "fps-asset-kit source README" -Url "https://raw.githubusercontent.com/petroulacl/fps-asset-kit/a19b7458a593598211c95ec46ef4eb4b6d1f94d7/README.md" -Destination (Join-Path $IntakeRoot "FPSAssetKit\SOURCE_README.md")

Write-Step "Writing SHA-256 receipt"
$receiptRows = Get-ChildItem -LiteralPath $IntakeRoot -Recurse -File | Where-Object { $_.FullName -ne $ReceiptPath } | Sort-Object FullName | ForEach-Object {
    [pscustomobject]@{
        RelativePath = Get-RelativePathCompat -Root $IntakeRoot -FullPath $_.FullName
        Bytes = $_.Length
        Sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $_.FullName).Hash.ToUpperInvariant()
    }
}
$receiptRows | Export-Csv -NoTypeInformation -Encoding UTF8 -LiteralPath $ReceiptPath
Write-Step "DONE"
Write-Host "Intake root: $IntakeRoot"
Write-Host "Local archive cache: $CacheRoot"
Write-Host "Receipt: $ReceiptPath"
Write-Host "No production Unreal asset was replaced automatically. Retarget/import/runtime validation is the next gate."
