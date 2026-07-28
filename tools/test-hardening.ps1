$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$plugin = Get-Content -LiteralPath (Join-Path $root 'plugin.cpp') -Raw
$clipGuard = Get-Content -LiteralPath (Join-Path $root 'src\clip_guard.cpp') -Raw
$volumeConsole = Get-Content -LiteralPath (Join-Path $root 'src\volume_console.cpp') -Raw
$canvas = Get-Content -LiteralPath (Join-Path $root 'src\canvas_openai.cpp') -Raw
$installer = Get-Content -LiteralPath (Join-Path $root 'installer\AccessibleOBSStudio.iss') -Raw
$allSource = $plugin + $clipGuard + $volumeConsole + $canvas

function Assert-True {
    param([bool]$Condition, [string]$Message)
    if (-not $Condition) {
        throw $Message
    }
}

$retiredName = 'Peak' + 'Guard'
Assert-True ($allSource -notmatch ('(?i)' + $retiredName)) 'The retired feature name is present.'
Assert-True ($allSource -notmatch 'invokeMethod\(obsMainWindow') 'Queued work still uses the OBS window as its lifetime context.'
Assert-True ($clipGuard -match 'CG_MAX_HISTORY_EVENTS') 'ClipGuard history is not bounded.'
Assert-True ($clipGuard -match 'sessionSafeguards') 'ClipGuard safeguards are not preserved across meter restarts.'
Assert-True ($clipGuard -match 'pendingEvents') 'Audio callbacks do not use the bounded pending-event buffer.'
Assert-True ($volumeConsole -match 'refreshTimer_->setInterval\(500\)') 'The Volume Console has no real-time refresh.'
Assert-True ($volumeConsole -match 'std::max\(\{0,entry.maximumValue,value\}\)') 'Positive source gain is not preserved during refresh.'
Assert-True ($canvas -match '256ull\*1024\*1024') 'Canvas capture memory is not bounded.'
Assert-True ($plugin -match 'CancelNetworkRequests\(\)') 'Network requests are not cancelled during shutdown.'
Assert-True ($installer -match 'HasValidMicrosoftSignature') 'Downloaded prerequisites are not signature checked.'
Assert-True ($installer -match '1\.1\.0-test\.1') 'The installer is not clearly identified as a test build.'

Write-Host 'Hardening source invariants passed.'
