// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Tiflo.Info

namespace {
constexpr uint32_t AM_AUDIO_FLAG=1u<<1;
constexpr int AM_MAX_CHANNELS=8,AM_FADER_LOG=2,AM_SAMPLE_PEAK=0,AM_TRUE_PEAK=1;
constexpr int AM_DEFAULT_WARNING_MS=1500,AM_RECOVERY_MS=1000,AM_MIN_DB=-100,AM_MAX_DB=20,AM_BINS=121;
constexpr double AM_BLOCK_MS=400.0,AM_DEFAULT_PEAK_MARGIN_DB=3.0,AM_DEFAULT_MAX_REDUCTION_DB=12.0;
using AudibleText=std::array<const char*,6>;
static QString AMText(const AudibleText &v){
    size_t language=LanguageIndex();QString text=QString::fromUtf8(v[language]);
    if(language==2)text.replace(QStringLiteral("Предупреждения Audible Meter"),QStringLiteral("Предупреждения звукового индикатора")).replace(QStringLiteral("Audible Meter не удалось"),QStringLiteral("Звуковому индикатору не удалось")).replace(QStringLiteral("Audible Meter"),QStringLiteral("Звуковой индикатор"));
    else if(language==3)text.replace(QStringLiteral("Попередження Audible Meter"),QStringLiteral("Попередження звукового індикатора")).replace(QStringLiteral("Audible Meter не вдалося"),QStringLiteral("Звуковому індикатору не вдалося")).replace(QStringLiteral("Audible Meter"),QStringLiteral("Звуковий індикатор"));
    return text;
}
static const AudibleText AM_COMMAND={".Start or stop Audible Meter",".Audible Meter starten oder stoppen",".Запустить или остановить Audible Meter",".Запустити або зупинити Audible Meter",".Démarrer ou arrêter Audible Meter",".Iniciar o detener Audible Meter"};
static const AudibleText AM_STARTED={"Audible Meter Started","Audible Meter gestartet","Audible Meter запущен","Audible Meter запущено","Audible Meter démarré","Audible Meter iniciado"};
static const AudibleText AM_STOPPED={"Audible Meter Stopped","Audible Meter beendet","Audible Meter остановлен","Audible Meter зупинено","Audible Meter arrêté","Audible Meter detenido"};
static const AudibleText AM_WARNINGS_ON={"Audible Meter Warnings On","Audible Meter-Warnungen ein","Предупреждения Audible Meter включены","Попередження Audible Meter увімкнено","Alertes Audible Meter activées","Avisos de Audible Meter activados"};
static const AudibleText AM_WARNINGS_OFF={"Audible Meter Warnings Off","Audible Meter-Warnungen aus","Предупреждения Audible Meter выключены","Попередження Audible Meter вимкнено","Alertes Audible Meter désactivées","Avisos de Audible Meter desactivados"};
static const AudibleText AM_OUTPUT_WARNING={"%1 output has remained above the warning level. Adjust its volume in the Accessible Volume Console.","Die Ausgabe von %1 lag anhaltend über dem Warnpegel. Passen Sie die Lautstärke in der barrierefreien Lautstärkekonsole an.","Выход %1 длительное время остаётся выше уровня предупреждения. Отрегулируйте громкость в доступной консоли громкости.","Вихід %1 тривалий час залишається вище рівня попередження. Відрегулюйте гучність у доступній консолі гучності.","La sortie de %1 est restée au-dessus du niveau d’avertissement. Réglez son volume dans la console de volume accessible.","La salida de %1 ha permanecido por encima del nivel de aviso. Ajuste su volumen en la consola de volumen accesible."};
static const AudibleText AM_INPUT_DIALOG_TITLE={"Pre-fader input is too loud","Der Pegel vor dem Regler ist zu hoch","Слишком высокий уровень до регулятора","Зависокий рівень до регулятора","Le niveau avant curseur est trop élevé","La entrada anterior al control es demasiado alta"};
static const AudibleText AM_INPUT_DIALOG_TEXT={"\"%1\" is too loud before its signal reaches the OBS volume control, so the sound may already be distorted. The Accessible Volume Console cannot correct this. Lower the input gain using the microphone, audio interface, Windows input level, or source application. Do you want Audible Meter to continue warning while you make this adjustment? Choosing No stops future pre-fader warnings for this unchanged source.","„%1“ ist zu laut, bevor das Signal den OBS-Lautstärkeregler erreicht; der Ton kann daher bereits verzerrt sein. Die barrierefreie Lautstärkekonsole kann dies nicht korrigieren. Senken Sie den Eingangspegel am Mikrofon, Audio-Interface, in den Windows-Eingangseinstellungen oder in der Quellanwendung. Soll Audible Meter Sie während dieser Anpassung weiter warnen? Mit Nein werden künftige Warnungen vor dem Regler für diese unveränderte Quelle beendet.","Сигнал «%1» слишком громкий ещё до регулятора громкости OBS, поэтому звук уже может быть искажён. Доступная консоль громкости не может это исправить. Уменьшите входное усиление на микрофоне, аудиоинтерфейсе, в уровне входа Windows или в приложении-источнике. Продолжать предупреждать во время настройки? Ответ «Нет» отключает будущие предупреждения до регулятора для этого неизменённого источника.","Сигнал «%1» зависокий ще до регулятора гучності OBS, тому звук уже може бути спотворений. Доступна консоль гучності не може це виправити. Зменште вхідне підсилення на мікрофоні, аудіоінтерфейсі, у рівні входу Windows або в програмі-джерелі. Продовжувати попереджати під час налаштування? Відповідь «Ні» вимикає майбутні попередження до регулятора для цього незміненого джерела.","Le signal de « %1 » est trop fort avant d’atteindre le réglage de volume d’OBS ; le son est donc peut-être déjà déformé. La console de volume accessible ne peut pas corriger ce problème. Réduisez le gain d’entrée sur le microphone, l’interface audio, le niveau d’entrée Windows ou l’application source. Audible Meter doit-il continuer à vous avertir pendant ce réglage ? Non désactive les futurs avertissements avant curseur pour cette source inchangée.","«%1» está demasiado alto antes de que la señal llegue al control de volumen de OBS, por lo que el sonido ya puede estar distorsionado. La Consola de volumen accesible no puede corregirlo. Reduzca la ganancia de entrada en el micrófono, la interfaz de audio, el nivel de entrada de Windows o la aplicación de origen. ¿Desea que Audible Meter siga avisando mientras realiza el ajuste? No detiene los futuros avisos anteriores al control para esta fuente sin cambios."};
static const AudibleText AM_INPUT_SAVE_FAILED={"The choice applies for this session, but Audible Meter could not save it for future launches.","Die Auswahl gilt für diese Sitzung, konnte aber nicht für zukünftige Starts gespeichert werden.","Выбор действует в этом сеансе, но Audible Meter не удалось сохранить его для следующих запусков.","Вибір діє в цьому сеансі, але Audible Meter не вдалося зберегти його для наступних запусків.","Le choix s’applique à cette session, mais Audible Meter n’a pas pu l’enregistrer pour les prochains démarrages.","La elección se aplica a esta sesión, pero Audible Meter no pudo guardarla para futuros inicios."};
static const AudibleText AM_INPUT_RESOLVED={"The pre-fader level for \"%1\" is now safe.","Der Pegel vor dem Regler für „%1“ ist jetzt sicher.","Уровень до регулятора для «%1» теперь безопасен.","Рівень до регулятора для «%1» тепер безпечний.","Le niveau avant curseur de « %1 » est maintenant sûr.","El nivel anterior al control de «%1» ahora es seguro."};
static const AudibleText AM_INPUT_NO_SIGNAL={"No signal from the monitored source.","Kein Signal von der überwachten Quelle.","Нет сигнала от контролируемого источника.","Немає сигналу від контрольованого джерела.","Aucun signal de la source surveillée.","No hay señal de la fuente supervisada."};
static const AudibleText AM_INPUT_SILENT_DIALOG={"The monitored source \"%1\" is still silent. Do you want to continue setting up its pre-fader level? Choosing No stops pre-fader checking for this unchanged source.","Die überwachte Quelle „%1“ ist weiterhin stumm. Möchten Sie mit der Einrichtung ihres Pegels vor dem Regler fortfahren? Mit Nein wird die Prüfung vor dem Regler für diese unveränderte Quelle beendet.","Контролируемый источник «%1» по-прежнему молчит. Продолжить настройку его уровня до регулятора? Ответ «Нет» отключает проверку до регулятора для этого неизменённого источника.","Контрольоване джерело «%1» досі мовчить. Продовжити налаштування його рівня до регулятора? Відповідь «Ні» вимикає перевірку до регулятора для цього незміненого джерела.","La source surveillée « %1 » est toujours silencieuse. Voulez-vous continuer à régler son niveau avant curseur ? Non désactive la vérification avant curseur pour cette source inchangée.","La fuente supervisada «%1» sigue en silencio. ¿Desea continuar configurando su nivel anterior al control? No detiene la comprobación anterior al control para esta fuente sin cambios."};
static const AudibleText AM_INPUT_UNAVAILABLE={"The monitored pre-fader source is no longer available.","Die überwachte Quelle vor dem Regler ist nicht mehr verfügbar.","Контролируемый источник до регулятора больше недоступен.","Контрольоване джерело до регулятора більше недоступне.","La source surveillée avant curseur n’est plus disponible.","La fuente supervisada anterior al control ya no está disponible."};
static const AudibleText AM_INPUT_CONSOLE_DECISION={"\"%1\" still has a pre-fader level problem. The Accessible Volume Console cannot correct it. Would you like to continue adjusting this source? Choosing No adds it to the pre-fader stop list and opens the console.","„%1“ hat weiterhin ein Pegelproblem vor dem Regler. Die barrierefreie Lautstärkekonsole kann es nicht beheben. Möchten Sie diese Quelle weiter anpassen? Mit Nein wird sie zur Stoppliste für Pegel vor dem Regler hinzugefügt und die Konsole geöffnet.","У источника «%1» всё ещё есть проблема с уровнем до регулятора. Доступная консоль громкости не может её исправить. Продолжить настройку этого источника? Ответ «Нет» добавит его в стоп-лист проверки до регулятора и откроет консоль.","Джерело «%1» усе ще має проблему з рівнем до регулятора. Доступна консоль гучності не може її виправити. Продовжити налаштування цього джерела? Відповідь «Ні» додасть його до стоп-листа перевірки до регулятора й відкриє консоль.","« %1 » présente toujours un problème de niveau avant curseur. La console de volume accessible ne peut pas le corriger. Voulez-vous continuer à régler cette source ? Non l’ajoute à la liste d’arrêt avant curseur et ouvre la console.","«%1» todavía tiene un problema de nivel anterior al control. La Consola de volumen accesible no puede corregirlo. ¿Desea continuar ajustando esta fuente? No la añade a la lista de exclusión anterior al control y abre la consola."};
static const AudibleText AM_STATUS_ON={"Audible Meter on. Warnings %1.","Audible Meter on. Warnings %1.","Звуковой индикатор включён. Предупреждения %1.","Звуковий індикатор увімкнено. Попередження %1.","Audible Meter on. Warnings %1.","Audible Meter on. Warnings %1."};
static const AudibleText AM_STATUS_OFF={"Audible Meter off.","Audible Meter off.","Звуковой индикатор выключен.","Звуковий індикатор вимкнено.","Audible Meter off.","Audible Meter off."};
static const AudibleText AM_STATUS_WARNINGS_ON={"on","on","включены","увімкнено","on","on"};
static const AudibleText AM_STATUS_WARNINGS_OFF={"off","off","выключены","вимкнено","off","off"};
enum class MeterZone{NoSignal,Green,Yellow,Red};
enum class AudibleTone{None,InputWarning,OutputWarning,YellowMeasurement,RedMeasurement};
enum class InputPriorityPhase{None,AwaitingDecision,Adjusting,ResolvedAcknowledgement};
struct AlertTracker{double redMs{},recoveryMs{},peakDb{-INFINITY};bool armed{true},pending{},automaticallyCorrected{};};
struct LevelAggregate{std::array<uint64_t,AM_BINS> bins{};uint64_t totalBlocks{};};
struct MeterSource{
    void *source{};obs_volmeter *meter{};QString uuid,name;int channels{2};std::mutex mutex;
    QString sourceType,signalReference,settingsFingerprint;bool checkPrefader{true},inputIgnored{},inputPromptShown{};
    double inputDb{-INFINITY},outputDb{-INFINITY};std::chrono::steady_clock::time_point updated{},lastCallback{};
    MeterZone inputZone{MeterZone::NoSignal},outputZone{MeterZone::NoSignal};AlertTracker inputAlert,outputAlert;
    LevelAggregate level;double blockEnergyMs{},blockMs{};
};
struct SessionSource{QString uuid,name;LevelAggregate level;};
struct TypicalLevel{bool available{};double db{-INFINITY},activity{};};
struct AutomaticLevelChange{QString uuid,name;float before{},after{};};
static std::atomic_bool audibleMeterActive{},audibleWarningsEnabled{true};
static std::atomic_bool prefaderPriorityActive{};
static std::atomic<int> audiblePeakType{AM_SAMPLE_PEAK};
static std::atomic<int> warningExposureMs{AM_DEFAULT_WARNING_MS};
static std::atomic<double> outputWarningDb{-9.0},peakSafetyMarginDb{AM_DEFAULT_PEAK_MARGIN_DB},maximumReductionDb{AM_DEFAULT_MAX_REDUCTION_DB};
static bool outputWarningCustomized{};
static std::vector<std::unique_ptr<MeterSource>> meterSources;
static std::vector<SessionSource> sessionSources;
static std::array<double,AM_BINS> binEnergy;
static QTimer *meterTimer{};static int refreshTicks{};
static QByteArray inputToneBytes,outputToneBytes,yellowToneBytes,redToneBytes;static AudibleTone playingTone{AudibleTone::None};static int playingAmplitude{};
static std::atomic_bool consoleOpen{};static bool collectionSwitchInProgress{};static QString consoleFocusedUuid,selectedUuid;
static QJsonArray ignoredInputSources;static QPointer<QMessageBox> inputWarningDialog;
static QPointer<QMessageBox> inputSilenceDialog,inputResolvedDialog;static InputPriorityPhase inputPriorityPhase{InputPriorityPhase::None};static QString priorityInputUuid,priorityInputName,priorityInputType,priorityInputSignal,priorityInputFingerprint;static std::chrono::steady_clock::time_point priorityHealthySince{},prioritySilentSince{};static bool silenceFirstAnnounced{},silenceSecondAnnounced{},silencePromptSuppressed{};static int priorityToneAmplitude{};
static std::vector<AutomaticLevelChange> automaticLevelUndo;
static int CurrentPeakType(){config *c=api.profile_config?api.profile_config():nullptr;return c&&api.config_get_uint(c,"Audio","PeakMeterType")?AM_TRUE_PEAK:AM_SAMPLE_PEAK;}
static double RedBoundaryDb(){return audiblePeakType.load()==AM_TRUE_PEAK?-2.0:-9.0;}
static MeterZone ZoneForDb(double db){if(!std::isfinite(db))return MeterZone::NoSignal;bool t=audiblePeakType.load()==AM_TRUE_PEAK;double yellow=t?-13.0:-20.0,red=RedBoundaryDb();return db<yellow?MeterZone::Green:(db<red?MeterZone::Yellow:MeterZone::Red);}
static bool CheckPrefaderForSourceType(const QString &type){return type!=QStringLiteral("ffmpeg_source")&&type!=QStringLiteral("vlc_source");}
static double ConfiguredDouble(config *cfg,const char *name,double fallback,double minimum,double maximum){
    if(!cfg||!api.config_has_user_value(cfg,"AccessibleStudio",name))return fallback;double value=api.config_get_double(cfg,"AccessibleStudio",name);return std::isfinite(value)&&value>=minimum&&value<=maximum?value:fallback;
}
static void LoadAudibleMeterSettings(){
    audiblePeakType=CurrentPeakType();config *cfg=api.global_config?api.global_config():nullptr;double seconds=ConfiguredDouble(cfg,"AudibleMeterWarningSeconds",AM_DEFAULT_WARNING_MS/1000.0,0.1,30.0);
    bool hasCustomizedFlag=cfg&&api.config_has_user_value(cfg,"AccessibleStudio","AudibleMeterOutputWarningCustomized");outputWarningCustomized=hasCustomizedFlag?api.config_get_double(cfg,"AccessibleStudio","AudibleMeterOutputWarningCustomized")>=0.5:cfg&&api.config_has_user_value(cfg,"AccessibleStudio","AudibleMeterOutputWarningDb");
    outputWarningDb=outputWarningCustomized?ConfiguredDouble(cfg,"AudibleMeterOutputWarningDb",RedBoundaryDb(),-30.0,0.0):RedBoundaryDb();peakSafetyMarginDb=ConfiguredDouble(cfg,"AudibleMeterPeakSafetyMarginDb",AM_DEFAULT_PEAK_MARGIN_DB,0.0,12.0);maximumReductionDb=ConfiguredDouble(cfg,"AudibleMeterMaximumReductionDb",AM_DEFAULT_MAX_REDUCTION_DB,0.5,30.0);warningExposureMs=static_cast<int>(std::lround(seconds*1000.0));
}
static QString InputDecisionPath(){
    QString roaming=QString::fromLocal8Bit(qgetenv("APPDATA"));if(roaming.isEmpty())return {};
    return QDir(roaming).filePath(QStringLiteral("obs-studio/plugin_config/accessible-studio/audible-meter-input-decisions.json"));
}
static void LoadInputDecisions(){
    ignoredInputSources=QJsonArray{};QFile file(InputDecisionPath());if(!file.open(QIODevice::ReadOnly))return;QJsonDocument document=QJsonDocument::fromJson(file.readAll());
    if(document.isObject()&&document.object().value(QStringLiteral("version")).toInt()==1&&document.object().value(QStringLiteral("ignored")).isArray())ignoredInputSources=document.object().value(QStringLiteral("ignored")).toArray();
}
static bool SaveInputDecisions(){
    QString path=InputDecisionPath();if(path.isEmpty())return false;QFileInfo info(path);if(!QDir().mkpath(info.absolutePath()))return false;QSaveFile file(path);if(!file.open(QIODevice::WriteOnly))return false;
    QJsonDocument document(QJsonObject{{QStringLiteral("version"),1},{QStringLiteral("ignored"),ignoredInputSources}});if(file.write(document.toJson(QJsonDocument::Indented))<0)return false;return file.commit();
}
static QString JsonIdentityValue(const QJsonObject &settings){
    static constexpr std::array<const char*,11> keys={"local_file","file","playlist","device_id","audio_device_id","video_device_id","device","window","capture_window","url","input"};
    for(const char *raw:keys){QJsonValue value=settings.value(QString::fromLatin1(raw));if(value.isUndefined()||value.isNull())continue;if(value.isString()&&!value.toString().isEmpty())return value.toString();if(value.isArray())return QString::fromUtf8(QJsonDocument(value.toArray()).toJson(QJsonDocument::Compact));if(value.isObject())return QString::fromUtf8(QJsonDocument(value.toObject()).toJson(QJsonDocument::Compact));}
    return {};
}
static void SourceIdentity(void *source,QString &type,QString &signal,QString &fingerprint){
    const char *rawType=source?api.source_id(source):nullptr;type=QString::fromUtf8(rawType?rawType:"");obs_data *data=source?api.source_settings(source):nullptr;QByteArray json;
    if(data){const char *raw=api.data_json(data);if(raw)json=raw;api.data_release(data);}QJsonDocument document=QJsonDocument::fromJson(json);QJsonObject settings=document.isObject()?document.object():QJsonObject{};signal=JsonIdentityValue(settings);QByteArray identity=QJsonDocument(settings).toJson(QJsonDocument::Compact);QFileInfo signalFile(signal);if(signalFile.exists()&&signalFile.isFile()){identity.append('\0');identity.append(signalFile.absoluteFilePath().toUtf8());identity.append('\0');identity.append(QByteArray::number(signalFile.size()));identity.append('\0');identity.append(QByteArray::number(signalFile.lastModified().toMSecsSinceEpoch()));}fingerprint=QString::fromLatin1(QCryptographicHash::hash(identity,QCryptographicHash::Sha256).toHex());
}
static bool InputIgnored(const QString &name,const QString &type,const QString &signal,const QString &fingerprint){
    for(const QJsonValue &value:ignoredInputSources){QJsonObject item=value.toObject();if(item.value(QStringLiteral("name")).toString()==name&&item.value(QStringLiteral("sourceType")).toString()==type&&item.value(QStringLiteral("signal")).toString()==signal&&item.value(QStringLiteral("identitySha256")).toString()==fingerprint)return true;}return false;
}
static bool RememberInputIgnored(const QString &name,const QString &type,const QString &signal,const QString &fingerprint){
    if(!InputIgnored(name,type,signal,fingerprint))ignoredInputSources.append(QJsonObject{{QStringLiteral("name"),name},{QStringLiteral("sourceType"),type},{QStringLiteral("signal"),signal},{QStringLiteral("identitySha256"),fingerprint}});
    return SaveInputDecisions();
}
static QString ZoneText(MeterZone z){switch(z){case MeterZone::Green:return QStringLiteral("green");case MeterZone::Yellow:return QStringLiteral("yellow");case MeterZone::Red:return QStringLiteral("red");default:return QStringLiteral("no signal");}}
static QString MeterDbText(double db){return std::isfinite(db)?QStringLiteral("%1 dBFS").arg(db,0,'f',1):QStringLiteral("no signal");}
static void BuildTone(QByteArray &sound,double frequency,int amplitude){
    if(!sound.isEmpty())return;constexpr int rate=22050,samples=rate/2,data=samples*2;sound.resize(44+data);auto *b=reinterpret_cast<unsigned char*>(sound.data());
    auto word=[b](int o,uint16_t v){b[o]=v&255;b[o+1]=(v>>8)&255;};auto dword=[b](int o,uint32_t v){for(int i=0;i<4;++i)b[o+i]=(v>>(i*8))&255;};
    std::memcpy(b,"RIFF",4);dword(4,36+data);std::memcpy(b+8,"WAVEfmt ",8);dword(16,16);word(20,1);word(22,1);dword(24,rate);dword(28,rate*2);word(32,2);word(34,16);std::memcpy(b+36,"data",4);dword(40,data);
    auto *pcm=reinterpret_cast<int16_t*>(b+44);for(int i=0;i<samples;++i)pcm[i]=static_cast<int16_t>(std::lround(amplitude*std::sin(2.0*3.14159265358979323846*frequency*i/rate)));
}
static int ToneAmplitude(AudibleTone tone,double environmentDb){
    bool warning=tone==AudibleTone::InputWarning||tone==AudibleTone::OutputWarning;if(!warning)return 5898;
    double fraction=std::isfinite(environmentDb)?0.28+std::clamp((environmentDb+30.0)/28.0,0.0,1.0)*0.17:0.30;fraction=std::clamp(fraction,0.30,0.45);
    return static_cast<int>(std::lround(std::round(fraction/0.05)*0.05*32767.0));
}
static void SetToneAmplitude(AudibleTone tone,int amplitude){
    if(tone==playingTone&&amplitude==playingAmplitude)return;if(playingTone!=AudibleTone::None)PlaySoundW(nullptr,nullptr,0);
    playingTone=AudibleTone::None;playingAmplitude=0;if(tone==AudibleTone::None)return;QByteArray *bytes=&yellowToneBytes;double hz=600.0;
    if(tone==AudibleTone::InputWarning){bytes=&inputToneBytes;hz=475.0;}else if(tone==AudibleTone::OutputWarning){bytes=&outputToneBytes;hz=700.0;}else if(tone==AudibleTone::RedMeasurement){bytes=&redToneBytes;hz=700.0;}bytes->clear();BuildTone(*bytes,hz,amplitude);
    if(PlaySoundW(reinterpret_cast<LPCWSTR>(bytes->constData()),nullptr,SND_MEMORY|SND_ASYNC|SND_LOOP|SND_NODEFAULT)){playingTone=tone;playingAmplitude=amplitude;}
}
static void SetTone(AudibleTone tone,double environmentDb=-INFINITY){SetToneAmplitude(tone,tone==AudibleTone::None?0:ToneAmplitude(tone,environmentDb));}
static void ResetAlert(AlertTracker &a){a=AlertTracker{};}
static void UpdateAlert(AlertTracker &a,bool aboveWarningLevel,double peakDb,double elapsed){
    if(aboveWarningLevel){a.redMs+=elapsed;a.recoveryMs=0;if(std::isfinite(peakDb))a.peakDb=std::max(a.peakDb,peakDb);if(a.armed&&a.redMs>=warningExposureMs.load()){a.pending=true;a.armed=false;}}
    else if((a.recoveryMs+=elapsed)>=AM_RECOVERY_MS){a.redMs=0;a.peakDb=-INFINITY;if(!a.armed){a.armed=true;a.pending=false;}}
}
static int LevelBin(double db){return std::clamp(static_cast<int>(std::lround(db))-AM_MIN_DB,0,AM_BINS-1);}
static void AddMagnitude(MeterSource &s,double db,double elapsed){
    s.blockMs+=elapsed;if(std::isfinite(db))s.blockEnergyMs+=binEnergy[static_cast<size_t>(LevelBin(db))]*elapsed;if(s.blockMs<AM_BLOCK_MS)return;
    double block=s.blockEnergyMs>0?10.0*std::log10(s.blockEnergyMs/s.blockMs):-INFINITY;++s.level.totalBlocks;if(std::isfinite(block))++s.level.bins[static_cast<size_t>(LevelBin(block))];s.blockMs=0;s.blockEnergyMs=0;
}
static void MeterCallback(void *p,const float *magnitude,const float *peak,const float *inputPeak){
    auto *s=static_cast<MeterSource*>(p);if(!s||!magnitude||!peak||!inputPeak||!audibleMeterActive.load())return;double output=-INFINITY,input=-INFINITY,rms=-INFINITY;
    for(int i=0;i<std::clamp(s->channels,1,AM_MAX_CHANNELS);++i){if(std::isfinite(peak[i]))output=std::max(output,double(peak[i]));if(std::isfinite(inputPeak[i]))input=std::max(input,double(inputPeak[i]));if(std::isfinite(magnitude[i]))rms=std::max(rms,double(magnitude[i]));}
    auto now=std::chrono::steady_clock::now();std::unique_lock<std::mutex> lock(s->mutex,std::try_to_lock);if(!lock.owns_lock()||!audibleMeterActive.load())return;
    double elapsed=s->lastCallback.time_since_epoch().count()?std::chrono::duration<double,std::milli>(now-s->lastCallback).count():0;s->lastCallback=now;elapsed=std::clamp(elapsed,0.0,100.0);
    s->inputDb=input;s->outputDb=output;s->updated=now;s->inputZone=ZoneForDb(input);s->outputZone=ZoneForDb(output);AddMagnitude(*s,rms,elapsed);
    if(audibleWarningsEnabled.load()&&!prefaderPriorityActive.load()){if(s->checkPrefader&&!s->inputIgnored)UpdateAlert(s->inputAlert,s->inputZone==MeterZone::Red,input,elapsed);if(!consoleOpen.load())UpdateAlert(s->outputAlert,output>=outputWarningDb.load(),output,elapsed);}
}
static void Detach(MeterSource &s){if(s.meter){api.volmeter_remove_callback(s.meter,MeterCallback,&s);api.volmeter_detach(s.meter);api.volmeter_destroy(s.meter);s.meter=nullptr;}if(s.source){api.source_release(s.source);s.source=nullptr;}}
static bool Attach(MeterSource &s,void *enumerated){
    void *ref=api.source_get_ref(enumerated);if(!ref)return false;obs_volmeter *m=api.volmeter_create(AM_FADER_LOG);if(!m){api.source_release(ref);return false;}api.volmeter_set_peak_type(m,audiblePeakType.load());api.volmeter_add_callback(m,MeterCallback,&s);
    if(!api.volmeter_attach(m,ref)){api.volmeter_remove_callback(m,MeterCallback,&s);api.volmeter_destroy(m);api.source_release(ref);return false;}s.source=ref;s.meter=m;SourceIdentity(ref,s.sourceType,s.signalReference,s.settingsFingerprint);s.checkPrefader=CheckPrefaderForSourceType(s.sourceType);s.inputIgnored=InputIgnored(s.name,s.sourceType,s.signalReference,s.settingsFingerprint);s.channels=std::clamp(api.volmeter_channels(m),1,AM_MAX_CHANNELS);return true;
}
static void Merge(LevelAggregate &to,const LevelAggregate &from){to.totalBlocks+=from.totalBlocks;for(size_t i=0;i<to.bins.size();++i)to.bins[i]+=from.bins[i];}
static void Archive(MeterSource &s){
    std::lock_guard<std::mutex> lock(s.mutex);auto it=std::find_if(sessionSources.begin(),sessionSources.end(),[&](const auto &v){return v.uuid==s.uuid;});
    if(it==sessionSources.end()){sessionSources.push_back({s.uuid,s.name});it=std::prev(sessionSources.end());}else it->name=s.name;Merge(it->level,s.level);
}
static bool Collect(void *p,void *enumerated){
    auto *ids=static_cast<std::vector<QString>*>(p);if(!enumerated||(api.source_output_flags(enumerated)&AM_AUDIO_FLAG)==0||!api.source_active(enumerated)||!api.source_audio_active(enumerated))return true;
    QString id=QString::fromUtf8(api.source_uuid(enumerated)?api.source_uuid(enumerated):"");if(id.isEmpty())return true;ids->push_back(id);QString name=QString::fromUtf8(api.source_name(enumerated)?api.source_name(enumerated):"Audio source");
    auto it=std::find_if(meterSources.begin(),meterSources.end(),[&](const auto &s){return s->uuid==id;});if(it!=meterSources.end()){QString type,signal,fingerprint;SourceIdentity(enumerated,type,signal,fingerprint);std::lock_guard<std::mutex> lock((*it)->mutex);bool changed=(*it)->name!=name||(*it)->sourceType!=type||(*it)->signalReference!=signal||(*it)->settingsFingerprint!=fingerprint;(*it)->name=name;if(changed){(*it)->sourceType=type;(*it)->signalReference=signal;(*it)->settingsFingerprint=fingerprint;(*it)->checkPrefader=CheckPrefaderForSourceType(type);(*it)->inputIgnored=InputIgnored(name,type,signal,fingerprint);(*it)->inputPromptShown=false;ResetAlert((*it)->inputAlert);}return true;}
    auto s=std::make_unique<MeterSource>();s->uuid=id;s->name=name;if(Attach(*s,enumerated))meterSources.push_back(std::move(s));return true;
}
static void RefreshSources(){
    if(!audibleMeterActive.load()||collectionSwitchInProgress)return;int type=CurrentPeakType();if(type!=audiblePeakType.load()){audiblePeakType=type;if(!outputWarningCustomized)outputWarningDb=RedBoundaryDb();for(auto &s:meterSources)if(s->meter)api.volmeter_set_peak_type(s->meter,type);}
    std::vector<QString> ids;api.enum_sources(Collect,&ids);for(auto it=meterSources.begin();it!=meterSources.end();)if(std::find(ids.begin(),ids.end(),(*it)->uuid)==ids.end()){Archive(*(*it));Detach(*(*it));it=meterSources.erase(it);}else ++it;
}
static void StartMeters(){for(auto &s:meterSources)Detach(*s);meterSources.clear();audiblePeakType=CurrentPeakType();RefreshSources();}
static void StopMeters(){for(auto &s:meterSources)Detach(*s);meterSources.clear();}
static void SuspendMeters(){for(auto &s:meterSources){Detach(*s);Archive(*s);}meterSources.clear();}
static MeterSource *ActiveSource(const QString &id){auto it=std::find_if(meterSources.begin(),meterSources.end(),[&](const auto &s){return s->uuid==id;});return it==meterSources.end()?nullptr:it->get();}
static void UpdateAudibleOutput();
static void ResetAllAlerts();
static void CloseInputWarningDialog(){
    if(!inputWarningDialog)return;QObject::disconnect(inputWarningDialog.data(),nullptr,PluginEventTarget(),nullptr);inputWarningDialog->close();inputWarningDialog->deleteLater();inputWarningDialog=nullptr;
}
static void CloseInputSilenceDialog(){
    if(!inputSilenceDialog)return;QObject::disconnect(inputSilenceDialog.data(),nullptr,PluginEventTarget(),nullptr);inputSilenceDialog->close();inputSilenceDialog->deleteLater();inputSilenceDialog=nullptr;
}
static void CloseInputResolvedDialog(){
    if(!inputResolvedDialog)return;QObject::disconnect(inputResolvedDialog.data(),nullptr,PluginEventTarget(),nullptr);inputResolvedDialog->close();inputResolvedDialog->deleteLater();inputResolvedDialog=nullptr;
}
static void ResetPrioritySilenceTracking(){
    priorityHealthySince={};prioritySilentSince={};silenceFirstAnnounced=false;silenceSecondAnnounced=false;
}
static void ClearInputPriority(){
    prefaderPriorityActive=false;inputPriorityPhase=InputPriorityPhase::None;priorityInputUuid.clear();priorityInputName.clear();priorityInputType.clear();priorityInputSignal.clear();priorityInputFingerprint.clear();ResetPrioritySilenceTracking();silencePromptSuppressed=false;priorityToneAmplitude=0;CloseInputSilenceDialog();CloseInputResolvedDialog();
}
static void ResetOutputAlerts(){
    for(auto &s:meterSources){std::lock_guard<std::mutex> lock(s->mutex);ResetAlert(s->outputAlert);}
}
static void FinishInputPriority(const AudibleText *announcement=nullptr){
    QString name=priorityInputName;CloseInputWarningDialog();ClearInputPriority();ResetOutputAlerts();SetTone(AudibleTone::None);if(announcement)AnnounceAccessibility(AMText(*announcement).arg(name));UpdateAudibleOutput();
}
static void ResolveInputPriority(){
    if(inputResolvedDialog||priorityInputUuid.isEmpty())return;SetTone(AudibleTone::None);CloseInputWarningDialog();CloseInputSilenceDialog();inputPriorityPhase=InputPriorityPhase::ResolvedAcknowledgement;QString name=priorityInputName;
    auto *dialog=new QMessageBox(QMessageBox::Information,AMText(AM_INPUT_DIALOG_TITLE),AMText(AM_INPUT_RESOLVED).arg(name),QMessageBox::Ok,obsMainWindow);dialog->setDefaultButton(QMessageBox::Ok);dialog->setEscapeButton(QMessageBox::Ok);dialog->setWindowModality(Qt::ApplicationModal);dialog->setAttribute(Qt::WA_DeleteOnClose);inputResolvedDialog=dialog;
    QObject::connect(dialog,&QMessageBox::finished,PluginEventTarget(),[](int){inputResolvedDialog=nullptr;ClearInputPriority();ResetOutputAlerts();UpdateAudibleOutput();});dialog->show();if(QAbstractButton *ok=dialog->button(QMessageBox::Ok))ok->setFocus(Qt::OtherFocusReason);
}
static bool PriorityIdentityMatches(MeterSource *source){
    if(!source)return false;std::lock_guard<std::mutex> lock(source->mutex);return source->name==priorityInputName&&source->sourceType==priorityInputType&&source->signalReference==priorityInputSignal&&source->settingsFingerprint==priorityInputFingerprint;
}
static void IgnorePrioritySource(const QString &uuid,const QString &name,const QString &type,const QString &signal,const QString &fingerprint,bool updateOutput=true){
    MeterSource *current=ActiveSource(uuid);if(current){std::lock_guard<std::mutex> lock(current->mutex);if(current->name==name&&current->sourceType==type&&current->signalReference==signal&&current->settingsFingerprint==fingerprint){current->inputIgnored=true;ResetAlert(current->inputAlert);}}ClearInputPriority();ResetOutputAlerts();SetTone(AudibleTone::None);if(!RememberInputIgnored(name,type,signal,fingerprint))QMessageBox::warning(obsMainWindow,AMText(AM_INPUT_DIALOG_TITLE),AMText(AM_INPUT_SAVE_FAILED));if(updateOutput)UpdateAudibleOutput();
}
static bool BeginInputPriority(const QString &uuid){
    MeterSource *source=ActiveSource(uuid);if(!source)return false;{std::lock_guard<std::mutex> lock(source->mutex);if(source->inputIgnored||!source->checkPrefader)return false;priorityInputUuid=source->uuid;priorityInputName=source->name;priorityInputType=source->sourceType;priorityInputSignal=source->signalReference;priorityInputFingerprint=source->settingsFingerprint;source->inputAlert.pending=false;}prefaderPriorityActive=true;inputPriorityPhase=InputPriorityPhase::AwaitingDecision;ResetPrioritySilenceTracking();silencePromptSuppressed=false;priorityToneAmplitude=0;ResetOutputAlerts();SetTone(AudibleTone::None);return true;
}
static bool ShowInputWarningDialog(const QString &uuid){
    if(inputWarningDialog)return false;MeterSource *source=ActiveSource(uuid);if(!source)return true;QString name,type,signal,fingerprint;
    {std::lock_guard<std::mutex> lock(source->mutex);if(source->inputIgnored){source->inputAlert.pending=false;return true;}source->inputAlert.pending=false;name=source->name;type=source->sourceType;signal=source->signalReference;fingerprint=source->settingsFingerprint;}
    auto *dialog=new QMessageBox(QMessageBox::Warning,AMText(AM_INPUT_DIALOG_TITLE),AMText(AM_INPUT_DIALOG_TEXT).arg(name),QMessageBox::Yes|QMessageBox::No,obsMainWindow);dialog->setDefaultButton(QMessageBox::Yes);dialog->setEscapeButton(QMessageBox::No);dialog->setWindowModality(Qt::ApplicationModal);dialog->setAttribute(Qt::WA_DeleteOnClose);inputWarningDialog=dialog;
    QObject::connect(dialog,&QMessageBox::finished,PluginEventTarget(),[uuid,name,type,signal,fingerprint](int result){inputWarningDialog=nullptr;if(result==QMessageBox::Yes){inputPriorityPhase=InputPriorityPhase::Adjusting;ResetPrioritySilenceTracking();UpdateAudibleOutput();return;}IgnorePrioritySource(uuid,name,type,signal,fingerprint);});
    dialog->show();if(QAbstractButton *yes=dialog->button(QMessageBox::Yes))yes->setFocus(Qt::OtherFocusReason);return true;
}
static void ShowInputSilenceDialog(){
    if(inputSilenceDialog||priorityInputUuid.isEmpty())return;QString uuid=priorityInputUuid,name=priorityInputName,type=priorityInputType,signal=priorityInputSignal,fingerprint=priorityInputFingerprint;auto *dialog=new QMessageBox(QMessageBox::Question,AMText(AM_INPUT_DIALOG_TITLE),AMText(AM_INPUT_SILENT_DIALOG).arg(name),QMessageBox::Yes|QMessageBox::No,obsMainWindow);dialog->setDefaultButton(QMessageBox::Yes);dialog->setEscapeButton(QMessageBox::No);dialog->setWindowModality(Qt::ApplicationModal);dialog->setAttribute(Qt::WA_DeleteOnClose);inputSilenceDialog=dialog;
    QObject::connect(dialog,&QMessageBox::finished,PluginEventTarget(),[uuid,name,type,signal,fingerprint](int result){inputSilenceDialog=nullptr;if(result==QMessageBox::Yes){silencePromptSuppressed=true;ResetPrioritySilenceTracking();UpdateAudibleOutput();return;}IgnorePrioritySource(uuid,name,type,signal,fingerprint);});dialog->show();if(QAbstractButton *yes=dialog->button(QMessageBox::Yes))yes->setFocus(Qt::OtherFocusReason);
}
static bool HandleInputPriority(){
    if(priorityInputUuid.isEmpty())return false;MeterSource *source=ActiveSource(priorityInputUuid);if(!source||!PriorityIdentityMatches(source)){FinishInputPriority(&AM_INPUT_UNAVAILABLE);return false;}auto now=std::chrono::steady_clock::now();bool fresh=false;double input=-INFINITY,environment=-INFINITY;MeterZone zone=MeterZone::NoSignal;{std::lock_guard<std::mutex> lock(source->mutex);fresh=source->updated.time_since_epoch().count()&&std::chrono::duration_cast<std::chrono::milliseconds>(now-source->updated).count()<=500;input=source->inputDb;environment=source->outputDb;zone=fresh?source->inputZone:MeterZone::NoSignal;}
    if(inputPriorityPhase==InputPriorityPhase::AwaitingDecision||inputPriorityPhase==InputPriorityPhase::ResolvedAcknowledgement||inputWarningDialog||inputResolvedDialog){SetTone(AudibleTone::None);return true;}if(inputSilenceDialog){SetTone(AudibleTone::None);return true;}
    if(fresh&&std::isfinite(input)){prioritySilentSince={};silenceFirstAnnounced=false;silenceSecondAnnounced=false;silencePromptSuppressed=false;if(zone==MeterZone::Red){priorityHealthySince={};if(consoleOpen.load())SetTone(AudibleTone::None);else{if(!priorityToneAmplitude)priorityToneAmplitude=ToneAmplitude(AudibleTone::InputWarning,environment);SetToneAmplitude(AudibleTone::InputWarning,priorityToneAmplitude);}return true;}SetTone(AudibleTone::None);if(!priorityHealthySince.time_since_epoch().count())priorityHealthySince=now;if(std::chrono::duration_cast<std::chrono::milliseconds>(now-priorityHealthySince).count()>=warningExposureMs.load()){ResolveInputPriority();return true;}return true;}
    priorityHealthySince={};SetTone(AudibleTone::None);if(silencePromptSuppressed)return true;if(!prioritySilentSince.time_since_epoch().count())prioritySilentSince=now;long long silentMs=std::chrono::duration_cast<std::chrono::milliseconds>(now-prioritySilentSince).count();if(silentMs>=2000&&!silenceFirstAnnounced){silenceFirstAnnounced=true;AnnounceAccessibility(AMText(AM_INPUT_NO_SIGNAL));}if(silentMs>=12000&&!silenceSecondAnnounced){silenceSecondAnnounced=true;AnnounceAccessibility(AMText(AM_INPUT_NO_SIGNAL));}if(silentMs>=22000)ShowInputSilenceDialog();return true;
}
static std::vector<SessionSource> Aggregates(){
    auto result=sessionSources;for(const auto &s:meterSources){std::lock_guard<std::mutex> lock(s->mutex);auto it=std::find_if(result.begin(),result.end(),[&](const auto &v){return v.uuid==s->uuid;});if(it==result.end()){result.push_back({s->uuid,s->name});it=std::prev(result.end());}else it->name=s->name;Merge(it->level,s->level);}return result;
}
static TypicalLevel Typical(const LevelAggregate &v){
    uint64_t count=0;double energy=0;for(int i=30;i<AM_BINS;++i){uint64_t n=v.bins[static_cast<size_t>(i)];count+=n;energy+=n*binEnergy[static_cast<size_t>(i)];}if(count<5)return {};
    double gate=std::max(-70.0,10.0*std::log10(energy/count)-10.0);uint64_t active=0;double activeEnergy=0;for(int i=0;i<AM_BINS;++i)if(i+AM_MIN_DB>=gate){uint64_t n=v.bins[static_cast<size_t>(i)];active+=n;activeEnergy+=n*binEnergy[static_cast<size_t>(i)];}
    if(active<5||activeEnergy<=0)return {};return {true,10.0*std::log10(activeEnergy/active),v.totalBlocks?100.0*active/v.totalBlocks:0};
}
static void SelectedInstant(){
    if(selectedUuid.isEmpty()){AnnounceAccessibility(QStringLiteral("No Source Selected"));return;}auto *s=ActiveSource(selectedUuid);if(!s){AnnounceAccessibility(QStringLiteral("The selected source has no current signal."));return;}auto now=std::chrono::steady_clock::now();QString name;double db;MeterZone zone;
    {std::lock_guard<std::mutex> lock(s->mutex);name=s->name;db=s->outputDb;zone=s->updated.time_since_epoch().count()&&std::chrono::duration_cast<std::chrono::milliseconds>(now-s->updated).count()<=500?s->outputZone:MeterZone::NoSignal;}AnnounceAccessibility(zone==MeterZone::NoSignal?QStringLiteral("%1, no current signal.").arg(name):QStringLiteral("%1, %2, %3.").arg(name,MeterDbText(db),ZoneText(zone)));
}
static void LoudestInstant(){
    auto now=std::chrono::steady_clock::now();QString name;double db=-INFINITY;MeterZone zone=MeterZone::NoSignal;for(const auto &s:meterSources){std::lock_guard<std::mutex> lock(s->mutex);if(!s->updated.time_since_epoch().count()||std::chrono::duration_cast<std::chrono::milliseconds>(now-s->updated).count()>500||!std::isfinite(s->outputDb))continue;if(!std::isfinite(db)||s->outputDb>db){name=s->name;db=s->outputDb;zone=s->outputZone;}}AnnounceAccessibility(name.isEmpty()?QStringLiteral("No active audio signal."):QStringLiteral("%1, %2, %3.").arg(name,MeterDbText(db),ZoneText(zone)));
}
static void SelectedOverall(){
    if(selectedUuid.isEmpty()){AnnounceAccessibility(QStringLiteral("No Source Selected"));return;}auto all=Aggregates();auto it=std::find_if(all.begin(),all.end(),[](const auto &v){return v.uuid==selectedUuid;});if(it==all.end()){AnnounceAccessibility(QStringLiteral("Not enough active signal for the selected source."));return;}TypicalLevel t=Typical(it->level);AnnounceAccessibility(t.available?QStringLiteral("%1, %2, %3.").arg(it->name,MeterDbText(t.db),ZoneText(ZoneForDb(t.db))):QStringLiteral("Not enough active signal for %1.").arg(it->name));
}
static void LoudestOverall(){
    auto all=Aggregates();const SessionSource *best=nullptr;TypicalLevel bestLevel;for(const auto &v:all){TypicalLevel t=Typical(v.level);if(t.available&&(!best||t.db>bestLevel.db)){best=&v;bestLevel=t;}}AnnounceAccessibility(best?QStringLiteral("%1, %2, %3.").arg(best->name,MeterDbText(bestLevel.db),ZoneText(ZoneForDb(bestLevel.db))):QStringLiteral("Not enough active signal to determine the overall loudest source."));
}
static MeterZone FocusedZone(){auto *s=ActiveSource(consoleFocusedUuid);if(!s)return MeterZone::NoSignal;auto now=std::chrono::steady_clock::now();std::lock_guard<std::mutex> lock(s->mutex);return s->updated.time_since_epoch().count()&&std::chrono::duration_cast<std::chrono::milliseconds>(now-s->updated).count()<=500?s->outputZone:MeterZone::NoSignal;}
static QString LoudestUuid(){auto now=std::chrono::steady_clock::now();QString id;double db=-INFINITY;for(const auto &s:meterSources){std::lock_guard<std::mutex> lock(s->mutex);if(s->updated.time_since_epoch().count()&&std::chrono::duration_cast<std::chrono::milliseconds>(now-s->updated).count()<=500&&(id.isEmpty()||s->outputDb>db)){id=s->uuid;db=s->outputDb;}}return id;}
static QString MostProblematicUuid(){
    auto now=std::chrono::steady_clock::now();QString outputId;double outputDb=-INFINITY;
    for(const auto &s:meterSources){std::lock_guard<std::mutex> lock(s->mutex);bool fresh=s->updated.time_since_epoch().count()&&std::chrono::duration_cast<std::chrono::milliseconds>(now-s->updated).count()<=500;if(!fresh)continue;
        if(!s->outputAlert.armed&&s->outputDb>=outputWarningDb.load()&&(outputId.isEmpty()||s->outputDb>outputDb)){outputId=s->uuid;outputDb=s->outputDb;}
    }
    return outputId;
}
static double GainDb(float gain){return gain>0.00001f?20.0*std::log10(static_cast<double>(gain)):-100.0;}
static float DbGain(double db){return db<=-100.0?0.0f:static_cast<float>(std::pow(10.0,db/20.0));}
struct SourceLookup{QString uuid;void *source{};};
static bool FindSourceReference(void *parameter,void *enumerated){
    auto *lookup=static_cast<SourceLookup*>(parameter);QString uuid=QString::fromUtf8(api.source_uuid(enumerated)?api.source_uuid(enumerated):"");if(uuid!=lookup->uuid)return true;lookup->source=api.source_get_ref(enumerated);return false;
}
static void ReduceWarningSources(){
    if(prefaderPriorityActive.load()||playingTone==AudibleTone::InputWarning){AnnounceAccessibility(QStringLiteral("Prefader warning requires manual adjustment."));return;}if(playingTone!=AudibleTone::OutputWarning){AnnounceAccessibility(QStringLiteral("No output warning sources."));return;}
    struct Candidate{MeterSource *meterSource{};void *source{};QString uuid,name;double peakDb{-INFINITY};};
    auto now=std::chrono::steady_clock::now();std::vector<Candidate> candidates;
    for(const auto &s:meterSources){std::lock_guard<std::mutex> lock(s->mutex);bool fresh=s->updated.time_since_epoch().count()&&std::chrono::duration_cast<std::chrono::milliseconds>(now-s->updated).count()<=500;if(fresh&&!s->outputAlert.armed&&!s->outputAlert.automaticallyCorrected&&s->outputDb>=outputWarningDb.load())candidates.push_back({s.get(),s->source,s->uuid,s->name,std::isfinite(s->outputAlert.peakDb)?s->outputAlert.peakDb:s->outputDb});}
    if(candidates.empty()){AnnounceAccessibility(QStringLiteral("No output warning sources."));return;}
    std::stable_sort(candidates.begin(),candidates.end(),[](const Candidate &a,const Candidate &b){return a.peakDb>b.peakDb;});
    const double peakTarget=outputWarningDb.load()-peakSafetyMarginDb.load(),maximumReduction=maximumReductionDb.load();std::vector<AutomaticLevelChange> changes;QStringList announcements;bool limited=false;
    for(const Candidate &candidate:candidates){
        if(!candidate.source)continue;float before=api.source_get_volume(candidate.source);double beforeDb=GainDb(before);
        double required=std::max(0.0,candidate.peakDb-peakTarget);if(required<=0.0)continue;
        if(required>maximumReduction)limited=true;double requested=std::min(required,maximumReduction);double reduction=std::ceil(requested*2.0)/2.0;double afterDb=std::max(-100.0,beforeDb-reduction);float after=DbGain(afterDb);api.source_set_volume(candidate.source,after);{std::lock_guard<std::mutex> lock(candidate.meterSource->mutex);candidate.meterSource->outputAlert.automaticallyCorrected=true;}changes.push_back({candidate.uuid,candidate.name,before,after});announcements.push_back(QStringLiteral("%1, reduced %2 dB, fader %3 dB.").arg(candidate.name).arg(beforeDb-afterDb,0,'f',1).arg(afterDb,0,'f',1));
    }
    if(changes.empty()){AnnounceAccessibility(QStringLiteral("No output warning sources required adjustment."));return;}automaticLevelUndo=std::move(changes);if(limited)announcements.push_back(QStringLiteral("Manual adjustment may still be needed."));UpdateAudibleOutput();AnnounceAccessibility(announcements.join(QLatin1Char(' ')));
}
static void RestoreAutomaticLevelCorrection(){
    if(automaticLevelUndo.empty()){AnnounceAccessibility(QStringLiteral("No automatic level correction to restore."));return;}QStringList restored;int skipped=0;
    for(const AutomaticLevelChange &change:automaticLevelUndo){SourceLookup lookup{change.uuid};api.enum_sources(FindSourceReference,&lookup);if(!lookup.source){++skipped;continue;}float current=api.source_get_volume(lookup.source);float tolerance=std::max(0.000001f,std::abs(change.after)*0.01f);if(std::abs(current-change.after)<=tolerance){api.source_set_volume(lookup.source,change.before);restored.push_back(change.name);}else ++skipped;api.source_release(lookup.source);}
    automaticLevelUndo.clear();UpdateAudibleOutput();if(restored.isEmpty()){AnnounceAccessibility(QStringLiteral("Automatic level correction was not restored because the source levels changed."));return;}QString message=QStringLiteral("Restored %1.").arg(restored.join(QStringLiteral(", ")));if(skipped)message+=QStringLiteral(" Some source levels had changed.");AnnounceAccessibility(message);
}
struct PendingAlert{QString uuid,name;double db{-INFINITY};};
static void UpdateAudibleOutput(){
    if(!audibleMeterActive.load()){SetTone(AudibleTone::None);return;}if(prefaderPriorityActive.load()&&HandleInputPriority())return;if(consoleOpen.load()){MeterZone z=FocusedZone();SetTone(z==MeterZone::Red?AudibleTone::RedMeasurement:(z==MeterZone::Yellow?AudibleTone::YellowMeasurement:AudibleTone::None));return;}auto now=std::chrono::steady_clock::now();bool outputActive=false;double environmentDb=-INFINITY;std::vector<PendingAlert> input,output;
    if(audibleWarningsEnabled.load())for(const auto &s:meterSources){std::lock_guard<std::mutex> lock(s->mutex);bool fresh=s->updated.time_since_epoch().count()&&std::chrono::duration_cast<std::chrono::milliseconds>(now-s->updated).count()<=500;if(fresh&&std::isfinite(s->outputDb))environmentDb=std::max(environmentDb,s->outputDb);outputActive|=fresh&&!s->outputAlert.armed&&s->outputDb>=outputWarningDb.load();if(s->inputAlert.pending){if(!s->checkPrefader||s->inputIgnored)s->inputAlert.pending=false;else input.push_back({s->uuid,s->name,s->inputDb});}if(s->outputAlert.pending){output.push_back({s->uuid,s->name,s->outputDb});s->outputAlert.pending=false;}}
    if(!input.empty()){auto it=std::max_element(input.begin(),input.end(),[](const auto &a,const auto &b){return a.db<b.db;});if(BeginInputPriority(it->uuid)){ShowInputWarningDialog(it->uuid);HandleInputPriority();return;}}
    if(outputActive)SetTone(AudibleTone::OutputWarning,environmentDb);else SetTone(AudibleTone::None);if(!output.empty()){auto it=std::max_element(output.begin(),output.end(),[](const auto &a,const auto &b){return a.db<b.db;});AnnounceAccessibility(AMText(AM_OUTPUT_WARNING).arg(it->name));}
}
static void ResetAllAlerts(){for(auto &s:meterSources){std::lock_guard<std::mutex> lock(s->mutex);ResetAlert(s->inputAlert);ResetAlert(s->outputAlert);}}
static void ToggleWarnings(){bool enabled=!audibleWarningsEnabled.load();audibleWarningsEnabled=enabled;if(!enabled){CloseInputWarningDialog();ClearInputPriority();}ResetAllAlerts();UpdateAudibleOutput();AnnounceAccessibility(AMText(enabled?AM_WARNINGS_ON:AM_WARNINGS_OFF));}
static bool EditableFocus(QWidget *w){for(QWidget *p=w;p;p=p->parentWidget())if(p->inherits("QLineEdit")||p->inherits("QTextEdit")||p->inherits("QPlainTextEdit")||p->inherits("QAbstractSpinBox")||p->inherits("QComboBox"))return true;return false;}
class AudibleMeterKeyFilter final:public QObject{public:using QObject::QObject;protected:bool eventFilter(QObject *o,QEvent *e)override{
    if(e->type()!=QEvent::KeyPress||!audibleMeterActive.load()||QApplication::applicationState()!=Qt::ApplicationActive)return QObject::eventFilter(o,e);auto *k=static_cast<QKeyEvent*>(e);if(k->isAutoRepeat()||EditableFocus(QApplication::focusWidget()))return QObject::eventFilter(o,e);Qt::KeyboardModifiers modifiers=k->modifiers()&~Qt::KeypadModifier;
    if(k->key()==Qt::Key_I&&modifiers==Qt::ShiftModifier){ReduceWarningSources();return true;}if(k->key()==Qt::Key_I&&modifiers==(Qt::ControlModifier|Qt::ShiftModifier)){RestoreAutomaticLevelCorrection();return true;}if(modifiers!=Qt::NoModifier)return QObject::eventFilter(o,e);
    switch(k->key()){case Qt::Key_I:ToggleWarnings();return true;case Qt::Key_H:SelectedInstant();return true;case Qt::Key_J:LoudestInstant();return true;case Qt::Key_K:SelectedOverall();return true;case Qt::Key_L:LoudestOverall();return true;default:return QObject::eventFilter(o,e);}
}};
static void ServiceTick(){if(!audibleMeterActive.load())return;if(++refreshTicks>=10){refreshTicks=0;RefreshSources();}UpdateAudibleOutput();}
static void StartTimer(){if(meterTimer)return;meterTimer=new QTimer(PluginEventTarget());meterTimer->setInterval(100);QObject::connect(meterTimer,&QTimer::timeout,PluginEventTarget(),ServiceTick);meterTimer->start();}
static void StopTimer(){if(meterTimer){meterTimer->stop();delete meterTimer;meterTimer=nullptr;}refreshTicks=0;}
static void StartAudibleMeter(){LoadInputDecisions();sessionSources.clear();automaticLevelUndo.clear();selectedUuid=consoleFocusedUuid;collectionSwitchInProgress=false;ClearInputPriority();audibleWarningsEnabled=true;audibleMeterActive=true;if(audibleMeterMenuAction)audibleMeterMenuAction->setChecked(true);StartMeters();StartTimer();AnnounceAccessibility(AMText(AM_STARTED));}
static void StopAudibleMeter(bool announce){if(!audibleMeterActive.exchange(false))return;collectionSwitchInProgress=false;CloseInputWarningDialog();ClearInputPriority();SetTone(AudibleTone::None);StopTimer();StopMeters();sessionSources.clear();automaticLevelUndo.clear();selectedUuid.clear();consoleFocusedUuid.clear();consoleOpen=false;if(audibleMeterMenuAction)audibleMeterMenuAction->setChecked(false);if(announce)AnnounceAccessibility(AMText(AM_STOPPED));}
}
static double AudibleMeterDefaultOutputWarningDb(){return CurrentPeakType()==AM_TRUE_PEAK?-2.0:-9.0;}
static double AudibleMeterOutputWarningDb(){return outputWarningDb.load();}
static double AudibleMeterWarningSeconds(){return warningExposureMs.load()/1000.0;}
static double AudibleMeterPeakSafetyMarginDb(){return peakSafetyMarginDb.load();}
static double AudibleMeterMaximumReductionDb(){return maximumReductionDb.load();}
static bool SaveAudibleMeterSettings(double warningDb,double seconds,double marginValue,double maximumValue){
    if(!std::isfinite(warningDb)||warningDb<-30.0||warningDb>0.0||!std::isfinite(seconds)||seconds<0.1||seconds>30.0||!std::isfinite(marginValue)||marginValue<0.0||marginValue>12.0||!std::isfinite(maximumValue)||maximumValue<0.5||maximumValue>30.0)return false;
    bool customized=std::abs(warningDb-AudibleMeterDefaultOutputWarningDb())>=0.01;config *cfg=api.global_config?api.global_config():nullptr;if(!cfg)return false;api.config_set_double(cfg,"AccessibleStudio","AudibleMeterOutputWarningCustomized",customized?1.0:0.0);api.config_set_double(cfg,"AccessibleStudio","AudibleMeterOutputWarningDb",warningDb);api.config_set_double(cfg,"AccessibleStudio","AudibleMeterWarningSeconds",seconds);api.config_set_double(cfg,"AccessibleStudio","AudibleMeterPeakSafetyMarginDb",marginValue);api.config_set_double(cfg,"AccessibleStudio","AudibleMeterMaximumReductionDb",maximumValue);if(api.config_save_safe(cfg,"tmp",nullptr)!=0)return false;
    outputWarningCustomized=customized;outputWarningDb=warningDb;warningExposureMs=static_cast<int>(std::lround(seconds*1000.0));peakSafetyMarginDb=marginValue;maximumReductionDb=maximumValue;automaticLevelUndo.clear();ResetAllAlerts();UpdateAudibleOutput();return true;
}
static QJsonArray AudibleMeterInputStopList(){return ignoredInputSources;}
static bool SaveAudibleMeterInputStopList(const QJsonArray &items){
    QJsonArray previous=ignoredInputSources;ignoredInputSources=items;if(!SaveInputDecisions()){ignoredInputSources=previous;return false;}
    for(auto &source:meterSources){std::lock_guard<std::mutex> lock(source->mutex);source->inputIgnored=InputIgnored(source->name,source->sourceType,source->signalReference,source->settingsFingerprint);ResetAlert(source->inputAlert);}
    UpdateAudibleOutput();return true;
}
static std::string AudibleMeterCommandLabel(){QByteArray v=AMText(AM_COMMAND).toUtf8();return {v.constData(),size_t(v.size())};}
static void ToggleAudibleMeter(){if(!audibleMeterActive.load()&&SoundDoctorIsRunning()){if(audibleMeterMenuAction)audibleMeterMenuAction->setChecked(false);AnnounceAccessibility(SoundDoctorBusyMessage());return;}audibleMeterActive.load()?StopAudibleMeter(true):StartAudibleMeter();}
static void AudibleMeterHotkey(void*,hotkey_id,obs_hotkey*,bool pressed){if(pressed&&PluginEventTarget())QMetaObject::invokeMethod(PluginEventTarget(),[]{ToggleAudibleMeter();},Qt::QueuedConnection);}
static void InitializeAudibleMeter(){LoadAudibleMeterSettings();LoadInputDecisions();for(int i=0;i<AM_BINS;++i)binEnergy[static_cast<size_t>(i)]=std::pow(10.0,(i+AM_MIN_DB)/10.0);qApp->installEventFilter(new AudibleMeterKeyFilter(pluginEventContext));}
static bool AudibleMeterRequestConsoleOpen(){
    if(!audibleMeterActive.load()||!prefaderPriorityActive.load())return true;if(inputPriorityPhase!=InputPriorityPhase::Adjusting)return false;
    MeterSource *source=ActiveSource(priorityInputUuid);if(!source||!PriorityIdentityMatches(source)){FinishInputPriority(&AM_INPUT_UNAVAILABLE);return true;}
    SetTone(AudibleTone::None);QMessageBox box(QMessageBox::Question,AMText(AM_INPUT_DIALOG_TITLE),AMText(AM_INPUT_CONSOLE_DECISION).arg(priorityInputName),QMessageBox::Yes|QMessageBox::No,obsMainWindow);box.setDefaultButton(QMessageBox::Yes);box.setEscapeButton(QMessageBox::No);int result=box.exec();
    if(result==QMessageBox::Yes){UpdateAudibleOutput();return false;}
    QString uuid=priorityInputUuid,name=priorityInputName,type=priorityInputType,signal=priorityInputSignal,fingerprint=priorityInputFingerprint;IgnorePrioritySource(uuid,name,type,signal,fingerprint,false);return true;
}
static void AudibleMeterConsoleOpened(){consoleOpen=true;ResetOutputAlerts();UpdateAudibleOutput();}
static void AudibleMeterConsoleClosed(){consoleOpen=false;consoleFocusedUuid.clear();ResetOutputAlerts();SetTone(AudibleTone::None);QTimer::singleShot(0,PluginEventTarget(),[]{UpdateAudibleOutput();});}
static void AudibleMeterConsoleFocusSource(const QString &uuid){consoleFocusedUuid=uuid;if(audibleMeterActive.load())selectedUuid=uuid;if(consoleOpen)UpdateAudibleOutput();}
static QString AudibleMeterPreferredConsoleSource(){if(!audibleMeterActive.load())return {};QString warningSource=MostProblematicUuid();if(!warningSource.isEmpty())return warningSource;if(!selectedUuid.isEmpty()&&ActiveSource(selectedUuid))return selectedUuid;return LoudestUuid();}
static QString AudibleMeterStatusText(){return audibleMeterActive.load()?AMText(AM_STATUS_ON).arg(AMText(audibleWarningsEnabled.load()?AM_STATUS_WARNINGS_ON:AM_STATUS_WARNINGS_OFF)):AMText(AM_STATUS_OFF);}
static void ShutdownAudibleMeter(){StopAudibleMeter(false);}
static void HandleAudibleMeterFrontendEvent(int event){
    constexpr int COLLECTION_CHANGED=13,PROFILE_CHANGED=15,EXIT=17,COLLECTION_CLEANUP=25,COLLECTION_CHANGING=35;
    if(event==EXIT){ShutdownAudibleMeter();return;}
    if(event==COLLECTION_CHANGING)ShutdownVolumeConsole();
    if((event==COLLECTION_CHANGING||event==COLLECTION_CLEANUP)&&audibleMeterActive.load()){collectionSwitchInProgress=true;automaticLevelUndo.clear();CloseInputWarningDialog();ClearInputPriority();SetTone(AudibleTone::None);SuspendMeters();return;}
    if(event==COLLECTION_CHANGED)collectionSwitchInProgress=false;
    if(event==PROFILE_CHANGED&&!audibleMeterActive.load()){audiblePeakType=CurrentPeakType();if(!outputWarningCustomized)outputWarningDb=RedBoundaryDb();}
    if((event==COLLECTION_CHANGED||event==PROFILE_CHANGED)&&audibleMeterActive.load())if(QObject *t=PluginEventTarget())QTimer::singleShot(0,t,RefreshSources);
}
