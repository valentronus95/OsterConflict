[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$RepoRoot = Split-Path -Parent $ProjectRoot
$ContentRoot = Join-Path $ProjectRoot "Content"
$ReportRoot = Join-Path $ProjectRoot "SourceAssets\ThirdParty\LocalInventory"
New-Item -ItemType Directory -Force -Path $ReportRoot | Out-Null

Write-Host "[OC Local Inventory] Scanning Oster Content"
$contentRows = @()
if (Test-Path -LiteralPath $ContentRoot) {
    $contentRows = Get-ChildItem -LiteralPath $ContentRoot -Directory | Sort-Object Name | ForEach-Object {
        $files = @(Get-ChildItem -LiteralPath $_.FullName -Recurse -File -ErrorAction SilentlyContinue)
        [pscustomobject]@{
            Folder = $_.Name
            FileCount = $files.Count
            Bytes = ($files | Measure-Object -Property Length -Sum).Sum
            UAssetCount = @($files | Where-Object Extension -eq ".uasset").Count
            UMapCount = @($files | Where-Object Extension -eq ".umap").Count
        }
    }
}
$contentRows | Export-Csv -NoTypeInformation -Encoding UTF8 -LiteralPath (Join-Path $ReportRoot "OSTER_CONTENT_TOPLEVEL.csv")

Write-Host "[OC Local Inventory] Capturing git changes"
$gitStatusPath = Join-Path $ReportRoot "GIT_STATUS_PORCELAIN.txt"
$gitStatus = & git -C $RepoRoot status --porcelain=v1 --untracked-files=all 2>&1
$gitStatus | Set-Content -Encoding UTF8 -LiteralPath $gitStatusPath

Write-Host "[OC Local Inventory] Scanning external Unreal projects"
$docs = [Environment]::GetFolderPath("MyDocuments")
$unrealProjectsRoot = Join-Path $docs "Unreal Projects"
$projectRows = @()
if (Test-Path -LiteralPath $unrealProjectsRoot) {
    $uprojects = @(Get-ChildItem -LiteralPath $unrealProjectsRoot -Recurse -Filter *.uproject -File -ErrorAction SilentlyContinue)
    foreach ($uproject in $uprojects) {
        $root = Split-Path -Parent $uproject.FullName
        $content = Join-Path $root "Content"
        $files = @()
        if (Test-Path -LiteralPath $content) {
            $files = @(Get-ChildItem -LiteralPath $content -Recurse -File -ErrorAction SilentlyContinue)
        }
        $projectRows += [pscustomobject]@{
            Project = [IO.Path]::GetFileNameWithoutExtension($uproject.Name)
            UProject = $uproject.FullName
            ContentPath = $content
            ContentExists = (Test-Path -LiteralPath $content)
            FileCount = $files.Count
            Bytes = ($files | Measure-Object -Property Length -Sum).Sum
            UAssetCount = @($files | Where-Object Extension -eq ".uasset").Count
            UMapCount = @($files | Where-Object Extension -eq ".umap").Count
        }
    }
}
$projectRows | Sort-Object Project | Export-Csv -NoTypeInformation -Encoding UTF8 -LiteralPath (Join-Path $ReportRoot "EXTERNAL_UNREAL_PROJECTS.csv")

$summaryPath = Join-Path $ReportRoot "SUMMARY.txt"
$changedCount = @($gitStatus | Where-Object { -not [string]::IsNullOrWhiteSpace($_) }).Count
$totalContentFiles = ($contentRows | Measure-Object -Property FileCount -Sum).Sum
$totalContentBytes = ($contentRows | Measure-Object -Property Bytes -Sum).Sum
@(
    "Oster Conflict local gameplay-content inventory",
    "GeneratedUtc=$([DateTime]::UtcNow.ToString('o'))",
    "RepoRoot=$RepoRoot",
    "ProjectRoot=$ProjectRoot",
    "ContentRoot=$ContentRoot",
    "GitChangedLines=$changedCount",
    "ContentTopLevelFolders=$($contentRows.Count)",
    "ContentFiles=$totalContentFiles",
    "ContentBytes=$totalContentBytes",
    "ExternalUnrealProjects=$($projectRows.Count)",
    "",
    "Detected target projects:",
    ($projectRows | Where-Object { $_.Project -match 'SuperSimple|GameAnimation|InteractionSystem' } | ForEach-Object { "- $($_.Project) | $($_.ContentPath) | files=$($_.FileCount) bytes=$($_.Bytes)" })
) | Set-Content -Encoding UTF8 -LiteralPath $summaryPath

Write-Host "[OC Local Inventory] DONE"
Write-Host "Report root: $ReportRoot"
Write-Host "Git changed lines: $changedCount"
Write-Host "External Unreal projects: $($projectRows.Count)"
