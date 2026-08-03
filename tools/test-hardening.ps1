$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$plugin = Get-Content -LiteralPath (Join-Path $root 'plugin.cpp') -Raw
$audibleMeter = Get-Content -LiteralPath (Join-Path $root 'src\audible_meter.cpp') -Raw
$soundDoctor = Get-Content -LiteralPath (Join-Path $root 'src\sound_doctor.cpp') -Raw
$focusNavigation = Get-Content -LiteralPath (Join-Path $root 'src\focus_navigation.cpp') -Raw
$shortcutEditor = Get-Content -LiteralPath (Join-Path $root 'src\shortcut_editor.cpp') -Raw
$qtInterface = Get-Content -LiteralPath (Join-Path $root 'src\qt_interface.cpp') -Raw
$localizedUi = Get-Content -LiteralPath (Join-Path $root 'src\localized_ui.cpp') -Raw
$volumeConsole = Get-Content -LiteralPath (Join-Path $root 'src\volume_console.cpp') -Raw
$canvas = Get-Content -LiteralPath (Join-Path $root 'src\canvas_openai.cpp') -Raw
$installer = Get-Content -LiteralPath (Join-Path $root 'installer\AccessibleStudio.iss') -Raw
$allSource = $plugin + $audibleMeter + $soundDoctor + $focusNavigation + $shortcutEditor + $qtInterface + $localizedUi + $volumeConsole + $canvas + $installer

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
Assert-True (($audibleMeter -match 'prefaderPriorityActive') -and ($audibleMeter -match 'BeginInputPriority') -and ($audibleMeter -match 'HandleInputPriority') -and ($audibleMeter -match 'if\(prefaderPriorityActive\.load\(\)&&HandleInputPriority\(\)\)return')) 'Latched pre-fader priority over all other warnings is incomplete.'
Assert-True (($audibleMeter -match 'Accessible Volume Console cannot correct this') -and ($audibleMeter -match 'microphone, audio interface, Windows input level, or source application') -and ($audibleMeter -match 'setDefaultButton\(QMessageBox::Yes\)') -and ($audibleMeter -match 'setEscapeButton\(QMessageBox::No\)')) 'The pre-fader warning decision dialog is incomplete or does not explain the external adjustment.'
Assert-True (($audibleMeter -match 'audible-meter-input-decisions\.json') -and ($audibleMeter -match 'identitySha256') -and ($audibleMeter -match 'signalReference') -and ($audibleMeter -match 'signalFile\.lastModified') -and ($audibleMeter -match 'inputIgnored')) 'Per-source pre-fader suppression is not persistently identified.'
Assert-True (($audibleMeter -match 'current->inputIgnored=true;ResetAlert\(current->inputAlert\)') -and ($audibleMeter -match 'static MeterZone FocusedZone\(\).*s->outputZone')) 'A pre-fader opt-out can interfere with post-fader Console measurements.'
Assert-True (($audibleMeter -match 'type!=QStringLiteral\(\"ffmpeg_source\"\)') -and ($audibleMeter -match 'type!=QStringLiteral\(\"vlc_source\"\)')) 'Direct OBS media playback is still checked for prefade warnings.'
Assert-True (($audibleMeter -match 'if\(s->checkPrefader&&!s->inputIgnored\)UpdateAlert\(s->inputAlert') -and ($audibleMeter -match 'if\(!consoleOpen\.load\(\)\)UpdateAlert\(s->outputAlert') -and ($audibleMeter -match 'if\(consoleOpen\.load\(\)\)\{MeterZone z=FocusedZone\(\)') -and ($volumeConsole -match 'AudibleMeterRequestConsoleOpen\(\)')) 'The Volume Console does not keep pre-fader detection queued while pausing output warnings.'
Assert-True (($audibleMeter -match 'silentMs>=2000') -and ($audibleMeter -match 'silentMs>=12000') -and ($audibleMeter -match 'silentMs>=22000') -and ($audibleMeter -match 'No signal from the monitored source\.') -and ($audibleMeter -match 'AM_INPUT_SILENT_DIALOG')) 'The agreed pre-fader silence notification and decision sequence is incomplete.'
Assert-True (($audibleMeter -match 'priorityHealthySince') -and ($audibleMeter -match 'warningExposureMs\.load\(\)') -and ($audibleMeter -match 'ResolveInputPriority\(\)') -and ($audibleMeter -match 'inputResolvedDialog') -and ($audibleMeter -match 'AM_INPUT_UNAVAILABLE')) 'Pre-fader priority does not require sustained healthy input, acknowledged resolution, or safe source-change handling.'
Assert-True (($audibleMeter -match 'InputPriorityPhase::AwaitingDecision.*SetTone\(AudibleTone::None\)') -and ($audibleMeter -match 'inputPriorityPhase=InputPriorityPhase::Adjusting') -and ($audibleMeter -match 'AM_INPUT_CONSOLE_DECISION')) 'The pre-fader decision can beep before Yes or cannot intercept a Console-opening request.'
Assert-True (($audibleMeter -match 'AudibleMeterConsoleOpened\(\)\{consoleOpen=true;ResetOutputAlerts\(\)') -and ($audibleMeter -match 'AudibleMeterConsoleClosed\(\)\{consoleOpen=false;consoleFocusedUuid\.clear\(\);ResetOutputAlerts\(\)')) 'Output-warning exposure is not reset at Volume Console boundaries.'
Assert-True (($qtInterface -match 'AdvancedSoundSettingsDialog') -and ($qtInterface -match 'OutputWarningLevel') -and ($qtInterface -match 'TimeAboveWarningLevel') -and ($qtInterface -match 'PeakSafetyMargin') -and ($qtInterface -match 'MaximumAutomaticReduction') -and ($qtInterface -notmatch 'QDoubleSpinBox \*warningDelay')) 'The Audible Meter page in Advanced Sound Settings is incomplete.'
Assert-True (($qtInterface -match 'RestoreAudibleMeterDefaults') -and ($qtInterface -match 'RestoreSoundDoctorDefaults') -and ($localizedUi -match 'Audible Meter settings will be restored to default') -and ($localizedUi -match 'Sound Doctor settings will be restored to default') -and ($qtInterface -match 'setDefaultButton\(yes\)') -and ($qtInterface -match 'setEscapeButton\(no\)')) 'The Advanced Sound Settings restore-default actions or confirmations are incomplete.'
Assert-True (($qtInterface -match 'PreFaderStopListWidget') -and ($qtInterface -match 'Qt::Key_Delete') -and ($qtInterface -match 'RemoveFromStopList') -and ($audibleMeter -match 'SaveAudibleMeterInputStopList')) 'The pre-fader stop list is not manageable from Advanced Sound Settings.'
Assert-True ($audibleMeter -match 'LoudestUuid') 'Audible Meter cannot identify the loudest source.'
Assert-True ($audibleMeter -match 'selectedUuid=consoleFocusedUuid') 'Starting Audible Meter does not preserve the source already focused in the Console.'
Assert-True (($audibleMeter -match 'COLLECTION_CHANGING=35') -and ($audibleMeter -match 'collectionSwitchInProgress=true;automaticLevelUndo\.clear\(\);CloseInputWarningDialog\(\);ClearInputPriority\(\);SetTone\(AudibleTone::None\);SuspendMeters\(\)')) 'Audible Meter does not release source references before a scene collection unload.'
Assert-True ($audibleMeter -match 'if\(event==COLLECTION_CHANGED\)collectionSwitchInProgress=false') 'Audible Meter does not resume after a scene collection change.'
Assert-True ($audibleMeter -match 'if\(event==COLLECTION_CHANGING\)ShutdownVolumeConsole\(\)') 'The Volume Console can retain sources during a scene collection unload.'
Assert-True (($audibleMeter -match 'ToneAmplitude') -and ($audibleMeter -match 'environmentDb') -and ($audibleMeter -match '0\.45')) 'Automatic warning volume is not dynamically capped.'
Assert-True ($volumeConsole -match 'AudibleMeterPreferredConsoleSource\(\)') 'The Volume Console does not request the loudest source when opened.'
Assert-True (($audibleMeter -match 'MostProblematicUuid') -and ($audibleMeter -match 'warningSource=MostProblematicUuid\(\)')) 'The Volume Console does not prioritize the source responsible for an active post-fader warning.'
Assert-True (($audibleMeter -match 'AM_DEFAULT_PEAK_MARGIN_DB=3\.0') -and ($audibleMeter -match 'AM_DEFAULT_MAX_REDUCTION_DB=12\.0') -and ($audibleMeter -match 'peakTarget=outputWarningDb\.load\(\)-peakSafetyMarginDb\.load\(\)') -and ($audibleMeter -notmatch 'TYPICAL_TARGET') -and ($audibleMeter -match 'ReduceWarningSources\(\)') -and ($audibleMeter -match 'playingTone==AudibleTone::InputWarning') -and ($audibleMeter -match 'outputAlert\.automaticallyCorrected=true')) 'Shift+I automatic warning-source reduction is not using the peak-only algorithm.'
Assert-True (($audibleMeter -match 'RestoreAutomaticLevelCorrection\(\)') -and ($audibleMeter -match 'modifiers==\(Qt::ControlModifier\|Qt::ShiftModifier\)') -and ($audibleMeter -match 'std::abs\(current-change\.after\)<=tolerance')) 'Automatic level correction does not have a guarded one-level undo.'
Assert-True (($audibleMeter -match 'k->key\(\)==Qt::Key_I&&modifiers==Qt::ShiftModifier') -and ($audibleMeter -match 'EditableFocus\(QApplication::focusWidget\(\)\)')) 'Shift+I can be intercepted while the user is typing.'
Assert-True (($volumeConsole -match 'QEvent::ShortcutOverride') -and ($volumeConsole -match 'entry\.slider->value\(\)\+step')) 'The Volume Console does not reliably claim and apply fader arrow keys.'
Assert-True (($volumeConsole -notmatch 'if\(isMinimized\(\)\)showNormal\(\);else if\(!isVisible\(\)\)show\(\);raise\(\);activateWindow\(\);FocusIndex') -and ($volumeConsole -match 'QTimer::singleShot\(0,dialog')) 'The warning-source fader focus is not deferred until the Console is active.'
Assert-True ($audibleMeter -notmatch 'OpenVolumeConsole') 'Audible Meter still opens the Volume Console automatically.'
Assert-True ($audibleMeter -notmatch 'filter_(create|add|remove|set|enable|disable)') 'Audible Meter must not manipulate filters.'
Assert-True (($plugin -match 'accessible_studio\.audible_meter') -and ($plugin -notmatch 'simple_audible_meter|diagnostic_audible_meter|focus_audible_meter_report')) 'Audible Meter command registration is incorrect.'
Assert-True ($focusNavigation -match 'audibleMeterHotkey,AUDIBLE_METER_NAME,\"OBS_KEY_I\",OBS_MOD_CONTROL\}') 'Audible Meter does not default to Ctrl+I.'
Assert-True (($plugin -match 'accessible_studio\.sound_doctor') -and ($plugin -match 'SoundDoctorHotkey') -and ($focusNavigation -match 'soundDoctorHotkey,SOUND_DOCTOR_NAME,\"OBS_KEY_D\",OBS_MOD_CONTROL\|OBS_MOD_SHIFT')) 'Sound Doctor does not have the dedicated Ctrl+Shift+D shortcut.'
Assert-True (($soundDoctor -match 'MONITOR_SECONDS=120') -and ($soundDoctor -match 'DEFAULT_MIN_ACTIVE_SECONDS=45\.0') -and ($soundDoctor -match 'std::try_to_lock')) 'Sound Doctor monitoring is not bounded or its callback can block.'
Assert-True (($soundDoctor -match 'CreateWindowEx\(WS_EX_APPWINDOW\|WS_EX_CONTROLPARENT') -and ($soundDoctor -match 'CreateCoreWebView2Controller\(parent') -and ($soundDoctor -notmatch 'QWebEngine')) 'Sound Doctor results are not hosted in a native WebView2 window like Canvas Description.'
Assert-True (($soundDoctor -match 'WM_SETFOCUS') -and ($soundDoctor -match 'ActivateKeyboardWindow\(resultsWindow,resultsWindow\)') -and ($soundDoctor -match 'SetFocus\(resultsWindow\)') -and ($soundDoctor -match "getElementById\('heading'\)\.focus")) 'Sound Doctor does not transfer initial keyboard and screen-reader focus into WebView2.'
Assert-True (($soundDoctor -match 'QDialogButtonBox::RejectRole') -and ($soundDoctor -match 'dialog\.exec\(\)')) 'Sound Doctor does not provide a conventional Escape-safe introductory dialog.'
Assert-True (($soundDoctor -match 'compressor_filter') -and ($soundDoctor -match 'limiter_filter') -and ($soundDoctor -match 'sound_doctor_managed') -and ($soundDoctor -match 'Sound Doctor – Compressor') -and ($soundDoctor -match 'Sound Doctor – Limiter')) 'Sound Doctor filter inspection or ownership marking is incomplete.'
Assert-True (($soundDoctor -match 'spread>=minimumSpread') -and ($soundDoctor -match 'spread>=14\.0\?maximumRatio:spread>=10\.0\?std::min\(2\.5,maximumRatio\):2\.0') -and ($soundDoctor -notmatch 'spread>=10\.0&&source->peakEvents') -and ($soundDoctor -match 'limiterAllSources\|\|MicrophoneType')) 'Sound Doctor compressor or limiter recommendation policy is not adaptive and preventive.'
Assert-True (($soundDoctor -match 'expectedFilterFingerprint') -and ($soundDoctor -match 'FilterFingerprint\(lookup\.source\)!=item\.expectedFilterFingerprint') -and ($soundDoctor -match 'HasFilter\(lookup\.source,type\)')) 'Sound Doctor does not revalidate sources and filter chains before applying changes.'
Assert-True (($soundDoctor -match 'Kind::Compressor\?2:3') -and ($soundDoctor -match 'attack_time\",6') -and ($soundDoctor -match 'release_time\",100') -and ($soundDoctor -match 'release_time\",60')) 'Sound Doctor does not place or configure managed filters conservatively.'
Assert-True (($soundDoctor -match 'add_undo_redo') -and ($soundDoctor -match 'RemoveApplied') -and ($soundDoctor -match 'RedoApplied')) 'Sound Doctor changes do not provide OBS undo and redo.'
Assert-True (($soundDoctor -match 'COLLECTION_CHANGING=35') -and ($soundDoctor -match 'DestroyWindow\(SoundDoctor::resultsWindow\)')) 'Sound Doctor can retain stale recommendations across scene-collection changes.'
Assert-True (($soundDoctor -match 'OutputsActive\(\)') -and ($soundDoctor -match 'showResults&&OutputsActive\(\)') -and ($soundDoctor -match 'WAITING_FOR_OUTPUTS') -and ($soundDoctor -match 'resultsDeferred') -and ($soundDoctor -match 'SD_STREAMING_STOPPED=3') -and ($soundDoctor -match 'SD_RECORDING_STOPPED=7')) 'Sound Doctor can display recommendations during streaming or recording.'
Assert-True (($soundDoctor -match 'Sound Doctor is currently working\. Do you want to interrupt this process and discard the results\?') -and ($soundDoctor -match 'Don’t show this message again') -and ($soundDoctor -match 'SoundDoctorInterruptWarningSuppressed') -and ($soundDoctor -match 'yes->setDefault\(true\)') -and ($soundDoctor -match 'Stop\(false\)')) 'Sound Doctor cannot be safely interrupted with the agreed confirmation and persistent bypass.'
Assert-True (($soundDoctor -match 'static void Tick\(\)\{if\(!running\.load\(\)\)return;Refresh\(\);if\(promptOpen\)return;') -and ($soundDoctor -match 'if\(running\.load\(\)\)\{PromptInterruption\(\);return;\}')) 'Sound Doctor can finish behind its interruption dialog or cannot route a repeated activation to interruption.'
Assert-True (($soundDoctor -notmatch 'Q(File|SaveFile)|CreateFile|WriteFile') -and ($soundDoctor -notmatch 'source_(set_volume|update|set_enabled)')) 'Sound Doctor records session data or changes user-owned source/filter settings.'
Assert-True (($qtInterface -match 'audio->addAction') -and ($qtInterface -match 'video->addAction') -and ($qtInterface -match 'AdvancedSoundSettings') -and ($qtInterface -match 'ManageOpenAiApiKeys') -and ($qtInterface -match 'OpenUserManual') -and ($qtInterface -match 'QDesktopServices::openUrl')) 'The discoverable Tools menu or localized user-manual command is incomplete.'
Assert-True (($localizedUi -match "static QString LAccessibleLabel\(LocalText id\)") -and ($localizedUi -match "remove\(u'&'\)") -and ($qtInterface -match 'search->setAccessibleName\(LAccessibleLabel\(LocalText::FindCommands\)\)')) 'The Find Commands field can expose its label mnemonic through its accessible name.'
Assert-True (($qtInterface -notmatch 'audibleSettings=buttons->addButton') -and ($qtInterface -notmatch 'apiSettings=buttons->addButton') -and ($qtInterface -match 'AdvancedSoundSettingsDialog') -and ($qtInterface -match 'ShowSettings\(\)')) 'Settings commands were not removed from the Shortcut Editor while retaining their menu dialogs.'
Assert-True (($qtInterface -match 'QListWidget \*navigation_') -and ($qtInterface -match 'QStackedWidget \*pages_') -and ($qtInterface -match 'QDialogButtonBox::Apply\|QDialogButtonBox::Cancel\|QDialogButtonBox::Ok') -and ($qtInterface -match 'SoundDoctorMinimumCompressionSpreadDb')) 'Advanced Sound Settings does not follow the native OBS list-and-pages settings pattern.'
Assert-True (($plugin -match 'QPointer<QMenu> accessibleToolsMenu') -and ($plugin -match 'QPointer<QAction> audibleMeterMenuAction') -and ($qtInterface -match 'audibleMeterMenuAction->setCheckable\(true\)') -and ($audibleMeter -match 'audibleMeterMenuAction->setChecked\(true\)') -and ($audibleMeter -match 'audibleMeterMenuAction->setChecked\(false\)')) 'The Audible Meter menu state is not synchronized.'
Assert-True (($qtInterface -match 'README\.%1\.html') -and ($qtInterface -match 'QStringLiteral\(\"en-US\"\)') -and ($shortcutEditor -match 'ShowAccessibleToolsMenu\(\)')) 'The localized manual fallback or menu-opening command is incomplete.'
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
Assert-True (($canvas -match 'visualChecker\?4000:') -and ($canvas -match 'QStringLiteral\("reasoning"\).*QStringLiteral\("effort"\).*QStringLiteral\("low"\)')) 'Visual Checker does not use low reasoning with a 4,000-token output allowance.'
Assert-True ($plugin -match 'CancelNetworkRequests\(\)') 'Network requests are not cancelled during shutdown.'
Assert-True ($installer -match 'HasValidMicrosoftSignature') 'Downloaded prerequisites are not signature checked.'
Assert-True (($installer -match '#define AppVersion "1\.1\.2"') -and ($installer -match 'OutputBaseFilename=AccessibleStudio-1\.1\.2-Setup') -and ($installer -notmatch '1\.1\.2-test')) 'The installer is not clearly identified as the final 1.1.2 build.'
Assert-True (($installer -match 'IndependentProjectNotice') -and ($installer -match 'BDA542EA-4E63-4F03-9F5B-B7A8CD8E470B') -and ($installer -match 'RemoveAccessibleObsStudioLegacy\.ps1') -and ($installer -match '6934DC32-5675-4735-B08A-0DED7B2CBD79')) 'The Accessible Studio transition notice, legacy cleanup, or new installer identity is incomplete.'

Write-Host 'Hardening source invariants passed.'
