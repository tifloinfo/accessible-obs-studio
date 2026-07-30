$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$plugin = Get-Content -LiteralPath (Join-Path $root 'plugin.cpp') -Raw
$audibleMeter = Get-Content -LiteralPath (Join-Path $root 'src\audible_meter.cpp') -Raw
$focusNavigation = Get-Content -LiteralPath (Join-Path $root 'src\focus_navigation.cpp') -Raw
$qtInterface = Get-Content -LiteralPath (Join-Path $root 'src\qt_interface.cpp') -Raw
$localizedUi = Get-Content -LiteralPath (Join-Path $root 'src\localized_ui.cpp') -Raw
$volumeConsole = Get-Content -LiteralPath (Join-Path $root 'src\volume_console.cpp') -Raw
$canvas = Get-Content -LiteralPath (Join-Path $root 'src\canvas_openai.cpp') -Raw
$installer = Get-Content -LiteralPath (Join-Path $root 'installer\AccessibleOBSStudio.iss') -Raw
$allSource = $plugin + $audibleMeter + $focusNavigation + $qtInterface + $localizedUi + $volumeConsole + $canvas + $installer

function Assert-True {
    param([bool]$Condition, [string]$Message)
    if (-not $Condition) {
        throw $Message
    }
}

$retiredName = 'Clip' + 'Guard'
$retiredFileName = $retiredName.Insert(4, '_').ToLowerInvariant() + '.cpp'
Assert-True ($allSource -notmatch ('(?i)' + $retiredName)) 'The retired feature name is present.'
Assert-True (-not (Test-Path -LiteralPath (Join-Path $root ('src\' + $retiredFileName)))) 'The retired source file is still present.'
Assert-True ($allSource -notmatch 'invokeMethod\(obsMainWindow') 'Queued work still uses the OBS window as its lifetime context.'
Assert-True ($audibleMeter -notmatch '(?i)SimpleAudible|DiagnosticAudible|AudibleMeterMode') 'Removed Audible Meter modes are still present.'
Assert-True (($audibleMeter -match 'AM_DEFAULT_WARNING_MS=1500') -and ($audibleMeter -match 'warningExposureMs\.load\(\)')) 'Audible Meter does not use the configurable 1.5-second default exposure.'
Assert-True ($audibleMeter -match 'std::try_to_lock') 'The audio callback can block on UI state.'
Assert-True ($audibleMeter -notmatch '(?i)report|history|QTreeWidget') 'Removed Diagnostic report or history code is still present.'
Assert-True (($audibleMeter -match 'Qt::Key_I') -and ($audibleMeter -match 'Qt::Key_H') -and ($audibleMeter -match 'Qt::Key_J') -and ($audibleMeter -match 'Qt::Key_K') -and ($audibleMeter -match 'Qt::Key_L')) 'Audible Meter on-the-fly commands are incomplete.'
Assert-True (($audibleMeter -match 'QLineEdit') -and ($audibleMeter -match 'QTextEdit') -and ($audibleMeter -match 'QPlainTextEdit')) 'Audible Meter letter commands are not protected in editors.'
Assert-True ($audibleMeter -match 'if\(inputActive\)SetTone\(AudibleTone::InputWarning,environmentDb\);else if\(outputActive\)') 'Input danger does not take priority over output warnings.'
Assert-True (($audibleMeter -match 'Do you wish to adjust prefade for this source\?') -and ($audibleMeter -match 'setDefaultButton\(QMessageBox::Yes\)') -and ($audibleMeter -match 'setEscapeButton\(QMessageBox::No\)')) 'The pre-fader warning decision dialog is incomplete.'
Assert-True (($audibleMeter -match 'audible-meter-input-decisions\.json') -and ($audibleMeter -match 'identitySha256') -and ($audibleMeter -match 'signalReference') -and ($audibleMeter -match 'signalFile\.lastModified') -and ($audibleMeter -match 'inputIgnored')) 'Per-source pre-fader suppression is not persistently identified.'
Assert-True (($audibleMeter -match 'current->inputIgnored=true;ResetAlert\(current->inputAlert\)') -and ($audibleMeter -match 'static MeterZone FocusedZone\(\).*s->outputZone')) 'A pre-fader opt-out can interfere with post-fader Console measurements.'
Assert-True (($audibleMeter -match 'type!=QStringLiteral\(\"ffmpeg_source\"\)') -and ($audibleMeter -match 'type!=QStringLiteral\(\"vlc_source\"\)')) 'Direct OBS media playback is still checked for prefade warnings.'
Assert-True (($audibleMeter -match 'if\(consoleOpen\.load\(\)\)\{MeterZone z=FocusedZone\(\)') -and ($audibleMeter -match 'audibleWarningsEnabled\.load\(\)&&!consoleOpen\.load\(\)')) 'The Volume Console does not exclusively own meter tones and suspend warning exposure.'
Assert-True (($audibleMeter -match 'AudibleMeterConsoleOpened\(\)\{consoleOpen=true;ResetAllAlerts\(\)') -and ($audibleMeter -match 'AudibleMeterConsoleClosed\(\)\{consoleOpen=false;consoleFocusedUuid\.clear\(\);ResetAllAlerts\(\)')) 'Warning exposure is not reset at Volume Console boundaries.'
Assert-True (($qtInterface -match 'QDoubleSpinBox \*warningDelay') -and ($qtInterface -match 'setRange\(0\.1,30\.0\)') -and ($qtInterface -match 'SaveAudibleMeterWarningSeconds')) 'The test-build warning-delay field is incomplete.'
Assert-True ($audibleMeter -match 'LoudestUuid') 'Audible Meter cannot identify the loudest source.'
Assert-True ($audibleMeter -match 'selectedUuid=consoleFocusedUuid') 'Starting Audible Meter does not preserve the source already focused in the Console.'
Assert-True (($audibleMeter -match 'COLLECTION_CHANGING=35') -and ($audibleMeter -match 'collectionSwitchInProgress=true;CloseInputWarningDialog\(\);SetTone\(AudibleTone::None\);SuspendMeters\(\)')) 'Audible Meter does not release source references before a scene collection unload.'
Assert-True ($audibleMeter -match 'if\(event==COLLECTION_CHANGED\)collectionSwitchInProgress=false') 'Audible Meter does not resume after a scene collection change.'
Assert-True ($audibleMeter -match 'if\(event==COLLECTION_CHANGING\)ShutdownVolumeConsole\(\)') 'The Volume Console can retain sources during a scene collection unload.'
Assert-True (($audibleMeter -match 'ToneAmplitude') -and ($audibleMeter -match 'environmentDb') -and ($audibleMeter -match '0\.45')) 'Automatic warning volume is not dynamically capped.'
Assert-True ($volumeConsole -match 'AudibleMeterPreferredConsoleSource\(\)') 'The Volume Console does not request the loudest source when opened.'
Assert-True ($audibleMeter -notmatch 'OpenVolumeConsole') 'Audible Meter still opens the Volume Console automatically.'
Assert-True ($audibleMeter -notmatch 'filter_(create|add|remove|set|enable|disable)') 'Audible Meter must not manipulate filters.'
Assert-True (($plugin -match 'accessible_obs_studio\.audible_meter') -and ($plugin -notmatch 'simple_audible_meter|diagnostic_audible_meter|focus_audible_meter_report')) 'Audible Meter command registration is incorrect.'
Assert-True ($focusNavigation -match 'audibleMeterHotkey,AUDIBLE_METER_NAME,\"OBS_KEY_I\",OBS_MOD_CONTROL\}') 'Audible Meter does not default to Ctrl+I.'
Assert-True (($focusNavigation -match 'SHORTCUT_DEFAULTS_SCHEMA') -and ($focusNavigation -match 'repairEmpty=initializeDefaults')) 'Empty upgraded shortcuts are not repaired across all defaults.'
Assert-True (($plugin -match 'NUMBERED_SCENE_NAMES') -and ($plugin -match 'numberedSceneHotkeys')) 'Numbered scene hotkeys are not registered.'
Assert-True (($focusNavigation -match 'frontend_scenes\(&scenes\)') -and ($focusNavigation -match 'frontend_set_scene\(target\)') -and ($focusNavigation -match 'sceneKeys') -and ($focusNavigation -match 'OBS_MOD_ALT')) 'Alt+number scene switching is incomplete.'
Assert-True ($plugin -match 'InitializeAccessibilityAlerts\(\);QueueProfileReview\(\);return true') 'Shortcut defaults still depend entirely on OBS finished-loading event timing.'
Assert-True ($plugin -match 'DescriptionPaletteColor') 'Canvas WebView colors are not sourced from the OBS Qt palette.'
Assert-True ($plugin -match 'ApplicationPaletteChange') 'Canvas WebView colors do not update after an OBS theme change.'
Assert-True ($plugin -match 'put_DefaultBackgroundColor') 'Canvas WebView can flash a background that does not match OBS.'
Assert-True ($volumeConsole -match 'refreshTimer_->setInterval\(500\)') 'The Volume Console has no real-time refresh.'
Assert-True ($volumeConsole -match 'std::max\(\{0,entry.maximumValue,value\}\)') 'Positive source gain is not preserved during refresh.'
Assert-True ($canvas -match '256ull\*1024\*1024') 'Canvas capture memory is not bounded.'
Assert-True ($plugin -match 'CancelNetworkRequests\(\)') 'Network requests are not cancelled during shutdown.'
Assert-True ($installer -match 'HasValidMicrosoftSignature') 'Downloaded prerequisites are not signature checked.'
Assert-True ($installer -match '1\.1\.0-test\.4') 'The installer is not clearly identified as the current test build.'

Write-Host 'Hardening source invariants passed.'
