param(
    [Parameter(Mandatory = $true)][string]$LogPath,
    [Parameter(Mandatory = $true)][string]$StatePath
)

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

$script:StartedAt = Get-Date
$script:LastLogWrite = $script:StartedAt
$script:ReadySeenAt = $null
$script:ExitSeenAt = $null

$form = New-Object System.Windows.Forms.Form
$form.Text = 'Oster Conflict - Loading'
$form.Size = New-Object System.Drawing.Size(650, 300)
$form.StartPosition = 'CenterScreen'
$form.FormBorderStyle = 'FixedDialog'
$form.MaximizeBox = $false
$form.MinimizeBox = $true
$form.TopMost = $true
$form.BackColor = [System.Drawing.Color]::FromArgb(18, 22, 28)
$form.ForeColor = [System.Drawing.Color]::White

$title = New-Object System.Windows.Forms.Label
$title.Text = 'OSTER CONFLICT'
$title.Font = New-Object System.Drawing.Font('Segoe UI', 25, [System.Drawing.FontStyle]::Bold)
$title.AutoSize = $false
$title.TextAlign = 'MiddleCenter'
$title.Location = New-Object System.Drawing.Point(20, 18)
$title.Size = New-Object System.Drawing.Size(595, 52)
$form.Controls.Add($title)

$percentLabel = New-Object System.Windows.Forms.Label
$percentLabel.Text = '5%'
$percentLabel.Font = New-Object System.Drawing.Font('Segoe UI', 20, [System.Drawing.FontStyle]::Bold)
$percentLabel.AutoSize = $false
$percentLabel.TextAlign = 'MiddleCenter'
$percentLabel.Location = New-Object System.Drawing.Point(20, 73)
$percentLabel.Size = New-Object System.Drawing.Size(595, 38)
$form.Controls.Add($percentLabel)

$progress = New-Object System.Windows.Forms.ProgressBar
$progress.Minimum = 0
$progress.Maximum = 100
$progress.Value = 5
$progress.Location = New-Object System.Drawing.Point(38, 116)
$progress.Size = New-Object System.Drawing.Size(559, 26)
$form.Controls.Add($progress)

$stageLabel = New-Object System.Windows.Forms.Label
$stageLabel.Text = 'Starting Unreal Engine...'
$stageLabel.Font = New-Object System.Drawing.Font('Segoe UI', 11)
$stageLabel.AutoSize = $false
$stageLabel.TextAlign = 'MiddleCenter'
$stageLabel.Location = New-Object System.Drawing.Point(30, 151)
$stageLabel.Size = New-Object System.Drawing.Size(575, 30)
$form.Controls.Add($stageLabel)

$detailLabel = New-Object System.Windows.Forms.Label
$detailLabel.Text = 'Startup stage estimate. Runtime acceptance is not implied.'
$detailLabel.Font = New-Object System.Drawing.Font('Segoe UI', 9)
$detailLabel.AutoSize = $false
$detailLabel.TextAlign = 'MiddleCenter'
$detailLabel.Location = New-Object System.Drawing.Point(30, 184)
$detailLabel.Size = New-Object System.Drawing.Size(575, 26)
$form.Controls.Add($detailLabel)

$timeLabel = New-Object System.Windows.Forms.Label
$timeLabel.Text = 'Elapsed: 00:00 | Last UE activity: 0 s ago'
$timeLabel.Font = New-Object System.Drawing.Font('Consolas', 9)
$timeLabel.AutoSize = $false
$timeLabel.TextAlign = 'MiddleCenter'
$timeLabel.Location = New-Object System.Drawing.Point(30, 215)
$timeLabel.Size = New-Object System.Drawing.Size(575, 24)
$form.Controls.Add($timeLabel)

function Set-Stage([int]$Percent, [string]$Stage, [string]$Detail) {
    $safe = [Math]::Max(0, [Math]::Min(100, $Percent))
    if ($safe -ge $progress.Value) { $progress.Value = $safe }
    if ($safe -ge [int]($percentLabel.Text.TrimEnd('%'))) { $percentLabel.Text = "$safe%" }
    $stageLabel.Text = $Stage
    if ($Detail) { $detailLabel.Text = $Detail }
}

$timer = New-Object System.Windows.Forms.Timer
$timer.Interval = 750
$timer.Add_Tick({
    $now = Get-Date
    $elapsed = $now - $script:StartedAt
    $lastAge = [Math]::Max(0, [int](($now - $script:LastLogWrite).TotalSeconds))
    $timeLabel.Text = ('Elapsed: {0:mm\:ss} | Last UE activity: {1} s ago' -f $elapsed, $lastAge)

    $text = ''
    if (Test-Path -LiteralPath $LogPath) {
        try {
            $item = Get-Item -LiteralPath $LogPath -ErrorAction Stop
            if ($item.LastWriteTime -gt $script:LastLogWrite) { $script:LastLogWrite = $item.LastWriteTime }
            $text = (Get-Content -LiteralPath $LogPath -Tail 500 -ErrorAction SilentlyContinue) -join "`n"
        } catch {}
    }

    if ($text -match 'PASS27_FRONTEND_WIDGETTREE_OWNED') {
        Set-Stage 100 'Ready. Opening game menu...' 'Frontend is constructed. Fast Preview remains preview-only.'
        if (-not $script:ReadySeenAt) { $script:ReadySeenAt = $now }
        if (($now - $script:ReadySeenAt).TotalSeconds -ge 2) {
            $timer.Stop()
            $form.Close()
            return
        }
    }
    elseif ($text -match 'Building static mesh|Building texture|Waiting for static meshes|LogTextureFormat|LogStaticMesh') {
        $assetLines = ([regex]::Matches($text, 'Building static mesh|Building texture|Waiting for static meshes')).Count
        $assetPercent = 55 + [Math]::Min(27, [int]($assetLines / 2))
        Set-Stage $assetPercent 'Preparing heavy models and textures...' 'Unreal is active. First cache build can take several minutes.'
    }
    elseif ($text -match 'PASS45_|PASS44_|PASS43_|PASS42_') {
        Set-Stage 48 'Creating Oster world and gameplay systems...' 'Source/runtime ownership checks are running.'
    }
    elseif ($text -match 'OsterConflict_Runtime') {
        Set-Stage 34 'Loading Oster runtime map...' 'Map packages and world actors are being created.'
    }
    elseif ($text -match 'LogInit|LogLoad|LogEngine') {
        Set-Stage 22 'Unreal Engine started...' 'Initializing renderer, engine modules and project content.'
    }
    elseif (Test-Path -LiteralPath $LogPath) {
        Set-Stage 12 'Waiting for Unreal startup log...' 'The game process has started.'
    }

    if ($lastAge -ge 90 -and -not $script:ReadySeenAt) {
        $detailLabel.Text = 'No new log lines for 90+ seconds. UE may be compiling a very large asset; if this persists, treat it as a stall.'
    }

    if (Test-Path -LiteralPath $StatePath) {
        $state = (Get-Content -LiteralPath $StatePath -ErrorAction SilentlyContinue) -join "`n"
        if ($state -match 'exit_code=(-?\d+)') {
            $exitCode = [int]$Matches[1]
            if (-not $script:ReadySeenAt) {
                if ($exitCode -eq 0) {
                    Set-Stage $progress.Value 'Game process closed before frontend-ready marker.' 'Preview ended before the menu was confirmed ready.'
                } else {
                    Set-Stage $progress.Value "Startup failed (exit code $exitCode)." 'Check the Fast Preview console/log for the exact error.'
                }
                if (-not $script:ExitSeenAt) { $script:ExitSeenAt = $now }
                if (($now - $script:ExitSeenAt).TotalSeconds -ge 12) {
                    $timer.Stop()
                    $form.Close()
                    return
                }
            }
        }
    }
})

$form.Add_Shown({ $form.Activate() })
$timer.Start()
[void]$form.ShowDialog()
$timer.Stop()
