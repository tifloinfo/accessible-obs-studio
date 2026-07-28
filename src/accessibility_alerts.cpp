// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Tiflo.Info

namespace {

using AlertText=std::array<const char*,6>;

static QString AText(const AlertText &text){return QString::fromUtf8(text[LanguageIndex()]);}

static const AlertText STATUS_INFORMATION_TITLE={"Status Information","Statusinformationen","Сведения о состоянии","Відомості про стан","Informations d’état","Información de estado"};
static const AlertText STATUS_INFORMATION_COMMAND={".Status Information",".Statusinformationen",".Сведения о состоянии",".Відомості про стан",".Informations d’état",".Información de estado"};
static const AlertText PAUSE_RECORDING_COMMAND={".Pause or resume recording",".Aufnahme pausieren oder fortsetzen",".Приостановить или возобновить запись",".Призупинити або продовжити запис",".Suspendre ou reprendre l’enregistrement",".Pausar o reanudar la grabación"};
static const AlertText STREAM_OFF={"Streaming off.","Streaming aus.","Трансляция выключена.","Трансляцію вимкнено.","Diffusion désactivée.","Emisión desactivada."};
static const AlertText STREAM_STARTING={"Streaming starting.","Streaming wird gestartet.","Трансляция запускается.","Трансляція запускається.","Démarrage de la diffusion.","Iniciando la emisión."};
static const AlertText STREAM_ON={"Streaming on.","Streaming an.","Трансляция включена.","Трансляцію ввімкнено.","Diffusion activée.","Emisión activada."};
static const AlertText STREAM_RECONNECTING={"Streaming reconnecting.","Streaming wird erneut verbunden.","Трансляция переподключается.","Трансляція перепідключається.","Reconnexion de la diffusion.","Reconectando la emisión."};
static const AlertText STREAM_STOPPING={"Streaming stopping.","Streaming wird beendet.","Трансляция останавливается.","Трансляція зупиняється.","Arrêt de la diffusion.","Deteniendo la emisión."};
static const AlertText STREAM_STARTED={"Streaming started.","Streaming gestartet.","Трансляция запущена.","Трансляцію розпочато.","Diffusion démarrée.","Emisión iniciada."};
static const AlertText STREAM_STOPPED={"Streaming stopped.","Streaming beendet.","Трансляция остановлена.","Трансляцію зупинено.","Diffusion arrêtée.","Emisión detenida."};
static const AlertText STREAM_DISCONNECTED={"Stream disconnected. Reconnecting.","Stream getrennt. Erneute Verbindung.","Трансляция прервана. Выполняется переподключение.","Трансляцію перервано. Виконується перепідключення.","Diffusion déconnectée. Reconnexion.","Emisión desconectada. Reconectando."};
static const AlertText STREAM_RECONNECTED={"Stream reconnected.","Stream erneut verbunden.","Трансляция переподключена.","Трансляцію перепідключено.","Diffusion reconnectée.","Emisión reconectada."};
static const AlertText RECORDING_OFF={"Recording off.","Recording aus.","Запись выключена.","Запис вимкнено.","Enregistrement désactivé.","Grabación desactivada."};
static const AlertText RECORDING_ON={"Recording on.","Recording an.","Запись включена.","Запис увімкнено.","Enregistrement activé.","Grabación activada."};
static const AlertText RECORDING_PAUSED_STATUS={"Recording paused.","Recording pausiert.","Запись приостановлена.","Запис призупинено.","Enregistrement suspendu.","Grabación en pausa."};
static const AlertText RECORDING_STARTED={"Recording started.","Recording gestartet.","Запись начата.","Запис розпочато.","Enregistrement démarré.","Grabación iniciada."};
static const AlertText RECORDING_STOPPED={"Recording stopped.","Recording beendet.","Запись остановлена.","Запис зупинено.","Enregistrement arrêté.","Grabación detenida."};
static const AlertText RECORDING_PAUSED={"Recording paused.","Recording pausiert.","Запись приостановлена.","Запис призупинено.","Enregistrement suspendu.","Grabación en pausa."};
static const AlertText RECORDING_RESUMED={"Recording resumed.","Recording fortgesetzt.","Запись возобновлена.","Запис продовжено.","Enregistrement repris.","Grabación reanudada."};
static const AlertText RECORDING_NOT_RUNNING={"Recording is not running.","Es läuft keine Aufnahme.","Запись не выполняется.","Запис не виконується.","Aucun enregistrement n’est en cours.","No hay ninguna grabación en curso."};
static const AlertText RECORDING_CANNOT_PAUSE={"Recording cannot be paused with the current settings.","Die Aufnahme kann mit den aktuellen Einstellungen nicht pausiert werden.","Запись нельзя приостановить с текущими настройками.","Запис неможливо призупинити з поточними налаштуваннями.","L’enregistrement ne peut pas être suspendu avec les paramètres actuels.","La grabación no se puede pausar con la configuración actual."};
static const AlertText RECORDING_CANNOT_RESUME={"Recording could not be resumed.","Die Aufnahme konnte nicht fortgesetzt werden.","Не удалось возобновить запись.","Не вдалося продовжити запис.","L’enregistrement n’a pas pu reprendre.","No se pudo reanudar la grabación."};
static const AlertText VIRTUAL_CAMERA_OFF={"Virtual camera off.","Virtuelle Kamera aus.","Виртуальная камера выключена.","Віртуальну камеру вимкнено.","Caméra virtuelle désactivée.","Cámara virtual desactivada."};
static const AlertText VIRTUAL_CAMERA_ON={"Virtual camera on.","Virtuelle Kamera an.","Виртуальная камера включена.","Віртуальну камеру ввімкнено.","Caméra virtuelle activée.","Cámara virtual activada."};
static const AlertText VIRTUAL_CAMERA_STARTED={"Virtual camera started.","Virtuelle Kamera gestartet.","Виртуальная камера запущена.","Віртуальну камеру запущено.","Caméra virtuelle démarrée.","Cámara virtual iniciada."};
static const AlertText VIRTUAL_CAMERA_STOPPED={"Virtual camera stopped.","Virtuelle Kamera beendet.","Виртуальная камера остановлена.","Віртуальну камеру зупинено.","Caméra virtuelle arrêtée.","Cámara virtual detenida."};
static const AlertText STUDIO_MODE_OFF={"Studio Mode off.","Studiomodus aus.","Студийный режим выключен.","Студійний режим вимкнено.","Mode Studio désactivé.","Modo Estudio desactivado."};
static const AlertText STUDIO_MODE_ON={"Studio Mode on.","Studiomodus an.","Студийный режим включен.","Студійний режим увімкнено.","Mode Studio activé.","Modo Estudio activado."};

enum class StreamState {Off,Starting,On,Reconnecting,Stopping};
static StreamState streamState=StreamState::Off;
static void *streamOutput{};
static bool reconnectAnnouncementActive{};
static uint64_t pauseStateGeneration{};

static void AnnounceAccessibility(const QString &message,QObject *source=nullptr){
    if(message.isEmpty())return;
    QObject *target=source?source:obsMainWindow;if(!target)return;
    QAccessibleAnnouncementEvent event(target,message);
    event.setPoliteness(QAccessible::AnnouncementPoliteness::Assertive);
    QAccessible::updateAccessibility(&event);
}

static void StreamReconnectSignal(void*,calldata*){
    if(!PluginEventTarget()||shuttingDown)return;QMetaObject::invokeMethod(PluginEventTarget(),[]{
        streamState=StreamState::Reconnecting;
        if(!reconnectAnnouncementActive){reconnectAnnouncementActive=true;AnnounceAccessibility(AText(STREAM_DISCONNECTED));}
    },Qt::QueuedConnection);
}

static void StreamReconnectSuccessSignal(void*,calldata*){
    if(!PluginEventTarget()||shuttingDown)return;QMetaObject::invokeMethod(PluginEventTarget(),[]{
        streamState=StreamState::On;
        if(reconnectAnnouncementActive)AnnounceAccessibility(AText(STREAM_RECONNECTED));
        reconnectAnnouncementActive=false;
    },Qt::QueuedConnection);
}

static void ReleaseStreamOutput(){
    if(!streamOutput)return;signal_handler *handler=api.output_signal_handler(streamOutput);
    if(handler){api.signal_disconnect(handler,"reconnect",StreamReconnectSignal,nullptr);api.signal_disconnect(handler,"reconnect_success",StreamReconnectSuccessSignal,nullptr);}
    api.output_release(streamOutput);streamOutput=nullptr;
}

static void AttachStreamOutput(){
    ReleaseStreamOutput();streamOutput=api.streaming_output();if(!streamOutput)return;signal_handler *handler=api.output_signal_handler(streamOutput);
    if(handler){api.signal_connect(handler,"reconnect",StreamReconnectSignal,nullptr);api.signal_connect(handler,"reconnect_success",StreamReconnectSuccessSignal,nullptr);}
    if(api.output_reconnecting(streamOutput)){streamState=StreamState::Reconnecting;reconnectAnnouncementActive=true;}
}

static QString CurrentStreamingStatus(){
    if(api.streaming_active&&api.streaming_active()){
        void *output=api.streaming_output();bool reconnecting=output&&api.output_reconnecting(output);if(output)api.output_release(output);
        if(reconnecting)return AText(STREAM_RECONNECTING);
    }
    switch(streamState){case StreamState::Starting:return AText(STREAM_STARTING);case StreamState::On:return AText(STREAM_ON);case StreamState::Reconnecting:return AText(STREAM_RECONNECTING);case StreamState::Stopping:return AText(STREAM_STOPPING);case StreamState::Off:return api.streaming_active&&api.streaming_active()?AText(STREAM_ON):AText(STREAM_OFF);}
    return AText(STREAM_OFF);
}

static QString CurrentRecordingStatus(){
    if(!api.recording_active())return AText(RECORDING_OFF);return api.recording_paused()?AText(RECORDING_PAUSED_STATUS):AText(RECORDING_ON);
}

static void ShowStatusInformation(){
    if(!obsMainWindow)return;QString text=QStringLiteral("%1\n%2\n%3\n%4").arg(CurrentStreamingStatus(),CurrentRecordingStatus(),api.virtualcam_active()?AText(VIRTUAL_CAMERA_ON):AText(VIRTUAL_CAMERA_OFF),api.studio_mode_active()?AText(STUDIO_MODE_ON):AText(STUDIO_MODE_OFF));
    QMessageBox box(QMessageBox::Information,AText(STATUS_INFORMATION_TITLE),text,QMessageBox::Ok,obsMainWindow);box.setDefaultButton(QMessageBox::Ok);box.setEscapeButton(QMessageBox::Ok);box.exec();
}

} // namespace

static std::string StatusInformationCommandLabel(){QByteArray text=AText(STATUS_INFORMATION_COMMAND).toUtf8();return {text.constData(),static_cast<size_t>(text.size())};}
static std::string PauseRecordingCommandLabel(){QByteArray text=AText(PAUSE_RECORDING_COMMAND).toUtf8();return {text.constData(),static_cast<size_t>(text.size())};}

static void StatusInformationHotkey(void*,hotkey_id,obs_hotkey*,bool pressed){
    if(!pressed||!PluginEventTarget())return;QMetaObject::invokeMethod(PluginEventTarget(),ShowStatusInformation,Qt::QueuedConnection);
}

static void PauseRecordingHotkey(void*,hotkey_id,obs_hotkey*,bool pressed){
    if(!pressed||!PluginEventTarget())return;QMetaObject::invokeMethod(PluginEventTarget(),[]{
        if(!api.recording_active()){AnnounceAccessibility(AText(RECORDING_NOT_RUNNING));return;}
        const bool wasPaused=api.recording_paused();const uint64_t generation=pauseStateGeneration;api.recording_pause(!wasPaused);
        QTimer::singleShot(500,PluginEventTarget(),[wasPaused,generation]{
            if(!api.recording_active()||generation!=pauseStateGeneration||api.recording_paused()!=wasPaused)return;
            AnnounceAccessibility(AText(wasPaused?RECORDING_CANNOT_RESUME:RECORDING_CANNOT_PAUSE));
        });
    },Qt::QueuedConnection);
}

static void InitializeAccessibilityAlerts(){
    streamState=api.streaming_active()?StreamState::On:StreamState::Off;if(api.streaming_active())AttachStreamOutput();
}

static void ShutdownAccessibilityAlerts(){
    ReleaseStreamOutput();
}

static void HandleAccessibilityFrontendEvent(int event){
    if(!PluginEventTarget()||shuttingDown)return;QMetaObject::invokeMethod(PluginEventTarget(),[event]{
        switch(event){
        case 0:streamState=StreamState::Starting;break;
        case 1:streamState=StreamState::On;reconnectAnnouncementActive=false;AttachStreamOutput();AnnounceAccessibility(AText(STREAM_STARTED));break;
        case 2:streamState=StreamState::Stopping;break;
        case 3:streamState=StreamState::Off;reconnectAnnouncementActive=false;ReleaseStreamOutput();AnnounceAccessibility(AText(STREAM_STOPPED));break;
        case 5:AnnounceAccessibility(AText(RECORDING_STARTED));break;
        case 7:++pauseStateGeneration;AnnounceAccessibility(AText(RECORDING_STOPPED));break;
        case 22:AnnounceAccessibility(AText(STUDIO_MODE_ON));break;
        case 23:AnnounceAccessibility(AText(STUDIO_MODE_OFF));break;
        case 28:++pauseStateGeneration;AnnounceAccessibility(AText(RECORDING_PAUSED));break;
        case 29:++pauseStateGeneration;AnnounceAccessibility(AText(RECORDING_RESUMED));break;
        case 32:AnnounceAccessibility(AText(VIRTUAL_CAMERA_STARTED));break;
        case 33:AnnounceAccessibility(AText(VIRTUAL_CAMERA_STOPPED));break;
        default:break;
        }
    },Qt::QueuedConnection);
}
