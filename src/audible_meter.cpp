// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Tiflo.Info

namespace {
constexpr uint32_t AM_AUDIO_FLAG=1u<<1;
constexpr int AM_MAX_CHANNELS=8,AM_FADER_LOG=2,AM_SAMPLE_PEAK=0,AM_TRUE_PEAK=1;
constexpr int AM_WARNING_MS=1000,AM_RECOVERY_MS=1000,AM_MIN_DB=-100,AM_MAX_DB=20,AM_BINS=121;
constexpr double AM_BLOCK_MS=400.0;
using AudibleText=std::array<const char*,6>;
static QString AMText(const AudibleText &v){return QString::fromUtf8(v[LanguageIndex()]);}
static const AudibleText AM_COMMAND={".Start or stop Audible Meter",".Audible Meter starten oder stoppen",".Запустить или остановить Audible Meter",".Запустити або зупинити Audible Meter",".Démarrer ou arrêter Audible Meter",".Iniciar o detener Audible Meter"};
static const AudibleText AM_STARTED={"Audible Meter Started","Audible Meter gestartet","Audible Meter запущен","Audible Meter запущено","Audible Meter démarré","Audible Meter iniciado"};
static const AudibleText AM_STOPPED={"Audible Meter Stopped","Audible Meter beendet","Audible Meter остановлен","Audible Meter зупинено","Audible Meter arrêté","Audible Meter detenido"};
static const AudibleText AM_WARNINGS_ON={"Audible Meter Warnings On","Audible Meter-Warnungen ein","Предупреждения Audible Meter включены","Попередження Audible Meter увімкнено","Alertes Audible Meter activées","Avisos de Audible Meter activados"};
static const AudibleText AM_WARNINGS_OFF={"Audible Meter Warnings Off","Audible Meter-Warnungen aus","Предупреждения Audible Meter выключены","Попередження Audible Meter вимкнено","Alertes Audible Meter désactivées","Avisos de Audible Meter desactivados"};
static const AudibleText AM_OUTPUT_WARNING={"%1 output level has remained in the red. Adjust its volume in the Accessible Volume Console.","Der Ausgangspegel von %1 war anhaltend im roten Bereich. Passen Sie die Lautstärke in der barrierefreien Lautstärkekonsole an.","Выходной уровень %1 длительное время находится в красной зоне. Отрегулируйте громкость в доступной консоли громкости.","Вихідний рівень %1 тривалий час перебуває в червоній зоні. Відрегулюйте гучність у доступній консолі гучності.","Le niveau de sortie de %1 est resté dans le rouge. Réglez son volume dans la console de volume accessible.","El nivel de salida de %1 ha permanecido en rojo. Ajuste su volumen en la consola de volumen accesible."};
static const AudibleText AM_INPUT_DIALOG_TITLE={"Sound quality is reduced","Die Tonqualität ist beeinträchtigt","Качество звука снижено","Якість звуку знижено","La qualité sonore est réduite","La calidad del sonido se ha reducido"};
static const AudibleText AM_INPUT_DIALOG_TEXT={"Sound quality is reduced for \"%1\" because its pre-fader signal is in the red. Do you wish to continue adjusting this source?","Die Tonqualität von „%1“ ist beeinträchtigt, weil das Signal vor dem Regler im roten Bereich liegt. Möchten Sie diese Quelle weiter anpassen?","Качество звука «%1» снижено, поскольку сигнал до регулятора находится в красной зоне. Продолжить настройку этого источника?","Якість звуку «%1» знижено, оскільки сигнал до регулятора перебуває в червоній зоні. Продовжити налаштування цього джерела?","La qualité sonore de « %1 » est réduite, car son signal avant curseur est dans le rouge. Voulez-vous continuer à régler cette source ?","La calidad del sonido de «%1» se ha reducido porque su señal antes del control está en rojo. ¿Desea continuar ajustando esta fuente?"};
static const AudibleText AM_INPUT_SAVE_FAILED={"The choice applies for this session, but Audible Meter could not save it for future launches.","Die Auswahl gilt für diese Sitzung, konnte aber nicht für zukünftige Starts gespeichert werden.","Выбор действует в этом сеансе, но Audible Meter не удалось сохранить его для следующих запусков.","Вибір діє в цьому сеансі, але Audible Meter не вдалося зберегти його для наступних запусків.","Le choix s’applique à cette session, mais Audible Meter n’a pas pu l’enregistrer pour les prochains démarrages.","La elección se aplica a esta sesión, pero Audible Meter no pudo guardarla para futuros inicios."};
enum class MeterZone{NoSignal,Green,Yellow,Red};
enum class AudibleTone{None,InputWarning,OutputWarning,YellowMeasurement,RedMeasurement};
struct AlertTracker{double redMs{},recoveryMs{};bool armed{true},pending{};};
struct LevelAggregate{std::array<uint64_t,AM_BINS> bins{};uint64_t totalBlocks{};};
struct MeterSource{
    void *source{};obs_volmeter *meter{};QString uuid,name;int channels{2};std::mutex mutex;
    QString sourceType,signalReference,settingsFingerprint;bool inputIgnored{},inputPromptShown{};
    double inputDb{-INFINITY},outputDb{-INFINITY};std::chrono::steady_clock::time_point updated{},lastCallback{};
    MeterZone inputZone{MeterZone::NoSignal},outputZone{MeterZone::NoSignal};AlertTracker inputAlert,outputAlert;
    LevelAggregate level;double blockEnergyMs{},blockMs{};
};
struct SessionSource{QString uuid,name;LevelAggregate level;};
struct TypicalLevel{bool available{};double db{-INFINITY},activity{};};
static std::atomic_bool audibleMeterActive{},audibleWarningsEnabled{true};
static std::atomic<int> audiblePeakType{AM_SAMPLE_PEAK};
static std::vector<std::unique_ptr<MeterSource>> meterSources;
static std::vector<SessionSource> sessionSources;
static std::array<double,AM_BINS> binEnergy;
static QTimer *meterTimer{};static int refreshTicks{};
static QByteArray inputToneBytes,outputToneBytes,yellowToneBytes,redToneBytes;static AudibleTone playingTone{AudibleTone::None};static int playingAmplitude{};
static bool consoleOpen{},collectionSwitchInProgress{};static QString consoleFocusedUuid,selectedUuid;
static QJsonArray ignoredInputSources;static QPointer<QMessageBox> inputWarningDialog;
static int CurrentPeakType(){config *c=api.profile_config?api.profile_config():nullptr;return c&&api.config_get_uint(c,"Audio","PeakMeterType")?AM_TRUE_PEAK:AM_SAMPLE_PEAK;}
static MeterZone ZoneForDb(double db){if(!std::isfinite(db))return MeterZone::NoSignal;bool t=audiblePeakType.load()==AM_TRUE_PEAK;double yellow=t?-13.0:-20.0,red=t?-2.0:-9.0;return db<yellow?MeterZone::Green:(db<red?MeterZone::Yellow:MeterZone::Red);}
static QString InputDecisionPath(){
    QString roaming=QString::fromLocal8Bit(qgetenv("APPDATA"));if(roaming.isEmpty())return {};
    return QDir(roaming).filePath(QStringLiteral("obs-studio/plugin_config/accessible-obs-studio/audible-meter-input-decisions.json"));
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
static void SetTone(AudibleTone tone,double environmentDb=-INFINITY){
    int amplitude=tone==AudibleTone::None?0:ToneAmplitude(tone,environmentDb);if(tone==playingTone&&amplitude==playingAmplitude)return;if(playingTone!=AudibleTone::None)PlaySoundW(nullptr,nullptr,0);
    playingTone=AudibleTone::None;playingAmplitude=0;if(tone==AudibleTone::None)return;QByteArray *bytes=&yellowToneBytes;double hz=600.0;
    if(tone==AudibleTone::InputWarning){bytes=&inputToneBytes;hz=475.0;}else if(tone==AudibleTone::OutputWarning){bytes=&outputToneBytes;hz=700.0;}else if(tone==AudibleTone::RedMeasurement){bytes=&redToneBytes;hz=700.0;}bytes->clear();BuildTone(*bytes,hz,amplitude);
    if(PlaySoundW(reinterpret_cast<LPCWSTR>(bytes->constData()),nullptr,SND_MEMORY|SND_ASYNC|SND_LOOP|SND_NODEFAULT)){playingTone=tone;playingAmplitude=amplitude;}
}
static void ResetAlert(AlertTracker &a){a=AlertTracker{};}
static void UpdateAlert(AlertTracker &a,MeterZone zone,double elapsed){
    if(zone==MeterZone::Red){a.redMs+=elapsed;a.recoveryMs=0;if(a.armed&&a.redMs>=AM_WARNING_MS){a.pending=true;a.armed=false;}}
    else if((a.recoveryMs+=elapsed)>=AM_RECOVERY_MS){a.redMs=0;if(!a.armed){a.armed=true;a.pending=false;}}
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
    if(audibleWarningsEnabled.load()){if(!s->inputIgnored)UpdateAlert(s->inputAlert,s->inputZone,elapsed);UpdateAlert(s->outputAlert,s->outputZone,elapsed);}
}
static void Detach(MeterSource &s){if(s.meter){api.volmeter_remove_callback(s.meter,MeterCallback,&s);api.volmeter_detach(s.meter);api.volmeter_destroy(s.meter);s.meter=nullptr;}if(s.source){api.source_release(s.source);s.source=nullptr;}}
static bool Attach(MeterSource &s,void *enumerated){
    void *ref=api.source_get_ref(enumerated);if(!ref)return false;obs_volmeter *m=api.volmeter_create(AM_FADER_LOG);if(!m){api.source_release(ref);return false;}api.volmeter_set_peak_type(m,audiblePeakType.load());api.volmeter_add_callback(m,MeterCallback,&s);
    if(!api.volmeter_attach(m,ref)){api.volmeter_remove_callback(m,MeterCallback,&s);api.volmeter_destroy(m);api.source_release(ref);return false;}s.source=ref;s.meter=m;SourceIdentity(ref,s.sourceType,s.signalReference,s.settingsFingerprint);s.inputIgnored=InputIgnored(s.name,s.sourceType,s.signalReference,s.settingsFingerprint);s.channels=std::clamp(api.volmeter_channels(m),1,AM_MAX_CHANNELS);return true;
}
static void Merge(LevelAggregate &to,const LevelAggregate &from){to.totalBlocks+=from.totalBlocks;for(size_t i=0;i<to.bins.size();++i)to.bins[i]+=from.bins[i];}
static void Archive(MeterSource &s){
    std::lock_guard<std::mutex> lock(s.mutex);auto it=std::find_if(sessionSources.begin(),sessionSources.end(),[&](const auto &v){return v.uuid==s.uuid;});
    if(it==sessionSources.end()){sessionSources.push_back({s.uuid,s.name});it=std::prev(sessionSources.end());}else it->name=s.name;Merge(it->level,s.level);
}
static bool Collect(void *p,void *enumerated){
    auto *ids=static_cast<std::vector<QString>*>(p);if(!enumerated||(api.source_output_flags(enumerated)&AM_AUDIO_FLAG)==0||!api.source_active(enumerated)||!api.source_audio_active(enumerated))return true;
    QString id=QString::fromUtf8(api.source_uuid(enumerated)?api.source_uuid(enumerated):"");if(id.isEmpty())return true;ids->push_back(id);QString name=QString::fromUtf8(api.source_name(enumerated)?api.source_name(enumerated):"Audio source");
    auto it=std::find_if(meterSources.begin(),meterSources.end(),[&](const auto &s){return s->uuid==id;});if(it!=meterSources.end()){QString type,signal,fingerprint;SourceIdentity(enumerated,type,signal,fingerprint);std::lock_guard<std::mutex> lock((*it)->mutex);bool changed=(*it)->name!=name||(*it)->sourceType!=type||(*it)->signalReference!=signal||(*it)->settingsFingerprint!=fingerprint;(*it)->name=name;if(changed){(*it)->sourceType=type;(*it)->signalReference=signal;(*it)->settingsFingerprint=fingerprint;(*it)->inputIgnored=InputIgnored(name,type,signal,fingerprint);(*it)->inputPromptShown=false;ResetAlert((*it)->inputAlert);}return true;}
    auto s=std::make_unique<MeterSource>();s->uuid=id;s->name=name;if(Attach(*s,enumerated))meterSources.push_back(std::move(s));return true;
}
static void RefreshSources(){
    if(!audibleMeterActive.load()||collectionSwitchInProgress)return;int type=CurrentPeakType();if(type!=audiblePeakType.load()){audiblePeakType=type;for(auto &s:meterSources)if(s->meter)api.volmeter_set_peak_type(s->meter,type);}
    std::vector<QString> ids;api.enum_sources(Collect,&ids);for(auto it=meterSources.begin();it!=meterSources.end();)if(std::find(ids.begin(),ids.end(),(*it)->uuid)==ids.end()){Archive(*(*it));Detach(*(*it));it=meterSources.erase(it);}else ++it;
}
static void StartMeters(){for(auto &s:meterSources)Detach(*s);meterSources.clear();audiblePeakType=CurrentPeakType();RefreshSources();}
static void StopMeters(){for(auto &s:meterSources)Detach(*s);meterSources.clear();}
static void SuspendMeters(){for(auto &s:meterSources){Detach(*s);Archive(*s);}meterSources.clear();}
static MeterSource *ActiveSource(const QString &id){auto it=std::find_if(meterSources.begin(),meterSources.end(),[&](const auto &s){return s->uuid==id;});return it==meterSources.end()?nullptr:it->get();}
static void UpdateAudibleOutput();
static void CloseInputWarningDialog(){
    if(!inputWarningDialog)return;QObject::disconnect(inputWarningDialog.data(),nullptr,PluginEventTarget(),nullptr);inputWarningDialog->close();inputWarningDialog->deleteLater();inputWarningDialog=nullptr;
}
static bool ShowInputWarningDialog(const QString &uuid){
    if(inputWarningDialog)return false;MeterSource *source=ActiveSource(uuid);if(!source)return true;QString name,type,signal,fingerprint;
    {std::lock_guard<std::mutex> lock(source->mutex);if(source->inputIgnored||source->inputPromptShown){source->inputAlert.pending=false;return true;}source->inputPromptShown=true;source->inputAlert.pending=false;name=source->name;type=source->sourceType;signal=source->signalReference;fingerprint=source->settingsFingerprint;}
    auto *dialog=new QMessageBox(QMessageBox::Warning,AMText(AM_INPUT_DIALOG_TITLE),AMText(AM_INPUT_DIALOG_TEXT).arg(name),QMessageBox::Yes|QMessageBox::No,obsMainWindow);dialog->setDefaultButton(QMessageBox::Yes);dialog->setEscapeButton(QMessageBox::No);dialog->setWindowModality(Qt::ApplicationModal);dialog->setAttribute(Qt::WA_DeleteOnClose);inputWarningDialog=dialog;
    QObject::connect(dialog,&QMessageBox::finished,PluginEventTarget(),[uuid,name,type,signal,fingerprint](int result){inputWarningDialog=nullptr;if(result==QMessageBox::Yes)return;MeterSource *current=ActiveSource(uuid);if(current){std::lock_guard<std::mutex> lock(current->mutex);if(current->name==name&&current->sourceType==type&&current->signalReference==signal&&current->settingsFingerprint==fingerprint){current->inputIgnored=true;ResetAlert(current->inputAlert);}}if(!RememberInputIgnored(name,type,signal,fingerprint))QMessageBox::warning(obsMainWindow,AMText(AM_INPUT_DIALOG_TITLE),AMText(AM_INPUT_SAVE_FAILED));UpdateAudibleOutput();});
    dialog->show();if(QAbstractButton *yes=dialog->button(QMessageBox::Yes))yes->setFocus(Qt::OtherFocusReason);return true;
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
    auto now=std::chrono::steady_clock::now();QString name;double db=-INFINITY;MeterZone zone=MeterZone::NoSignal;for(const auto &s:meterSources){std::lock_guard<std::mutex> lock(s->mutex);if(!s->updated.time_since_epoch().count()||std::chrono::duration_cast<std::chrono::milliseconds>(now-s->updated).count()>500||!std::isfinite(s->outputDb))continue;if(!std::isfinite(db)||s->outputDb>db){name=s->name;db=s->outputDb;zone=s->outputZone;}}AnnounceAccessibility(name.isEmpty()?QStringLiteral("No active audio signal."):QStringLiteral("%1 is loudest, %2, %3.").arg(name,MeterDbText(db),ZoneText(zone)));
}
static void SelectedOverall(){
    if(selectedUuid.isEmpty()){AnnounceAccessibility(QStringLiteral("No Source Selected"));return;}auto all=Aggregates();auto it=std::find_if(all.begin(),all.end(),[](const auto &v){return v.uuid==selectedUuid;});if(it==all.end()){AnnounceAccessibility(QStringLiteral("Not enough active signal for the selected source."));return;}TypicalLevel t=Typical(it->level);AnnounceAccessibility(t.available?QStringLiteral("%1. Typical active level %2. Active %3 percent of monitored time.").arg(it->name,MeterDbText(t.db)).arg(t.activity,0,'f',0):QStringLiteral("Not enough active signal for %1.").arg(it->name));
}
static void LoudestOverall(){
    auto all=Aggregates();const SessionSource *best=nullptr;TypicalLevel bestLevel;for(const auto &v:all){TypicalLevel t=Typical(v.level);if(t.available&&(!best||t.db>bestLevel.db)){best=&v;bestLevel=t;}}AnnounceAccessibility(best?QStringLiteral("%1 is loudest overall. Typical active level %2. Active %3 percent of monitored time.").arg(best->name,MeterDbText(bestLevel.db)).arg(bestLevel.activity,0,'f',0):QStringLiteral("Not enough active signal to determine the overall loudest source."));
}
static MeterZone FocusedZone(){auto *s=ActiveSource(consoleFocusedUuid);if(!s)return MeterZone::NoSignal;auto now=std::chrono::steady_clock::now();std::lock_guard<std::mutex> lock(s->mutex);return s->updated.time_since_epoch().count()&&std::chrono::duration_cast<std::chrono::milliseconds>(now-s->updated).count()<=500?s->outputZone:MeterZone::NoSignal;}
static QString LoudestUuid(){auto now=std::chrono::steady_clock::now();QString id;double db=-INFINITY;for(const auto &s:meterSources){std::lock_guard<std::mutex> lock(s->mutex);if(s->updated.time_since_epoch().count()&&std::chrono::duration_cast<std::chrono::milliseconds>(now-s->updated).count()<=500&&(id.isEmpty()||s->outputDb>db)){id=s->uuid;db=s->outputDb;}}return id;}
struct PendingAlert{QString uuid,name;double db{-INFINITY};};
static void UpdateAudibleOutput(){
    if(!audibleMeterActive.load()){SetTone(AudibleTone::None);return;}auto now=std::chrono::steady_clock::now();bool inputActive=false,outputActive=false;double environmentDb=-INFINITY;std::vector<PendingAlert> input,output;
    if(audibleWarningsEnabled.load())for(const auto &s:meterSources){std::lock_guard<std::mutex> lock(s->mutex);bool fresh=s->updated.time_since_epoch().count()&&std::chrono::duration_cast<std::chrono::milliseconds>(now-s->updated).count()<=500;if(fresh&&std::isfinite(s->outputDb))environmentDb=std::max(environmentDb,s->outputDb);inputActive|=fresh&&!s->inputIgnored&&!s->inputAlert.armed&&s->inputZone==MeterZone::Red;outputActive|=fresh&&!s->outputAlert.armed&&s->outputZone==MeterZone::Red;if(s->inputAlert.pending){if(s->inputPromptShown)s->inputAlert.pending=false;else input.push_back({s->uuid,s->name,s->inputDb});}if(s->outputAlert.pending){output.push_back({s->uuid,s->name,s->outputDb});s->outputAlert.pending=false;}}
    if(inputActive)SetTone(AudibleTone::InputWarning,environmentDb);else if(outputActive)SetTone(AudibleTone::OutputWarning,environmentDb);else if(consoleOpen){MeterZone z=FocusedZone();SetTone(z==MeterZone::Red?AudibleTone::RedMeasurement:(z==MeterZone::Yellow?AudibleTone::YellowMeasurement:AudibleTone::None));}else SetTone(AudibleTone::None);
    if(!input.empty()){auto it=std::max_element(input.begin(),input.end(),[](const auto &a,const auto &b){return a.db<b.db;});ShowInputWarningDialog(it->uuid);}else if(!output.empty()){auto it=std::max_element(output.begin(),output.end(),[](const auto &a,const auto &b){return a.db<b.db;});AnnounceAccessibility(AMText(AM_OUTPUT_WARNING).arg(it->name));}
}
static void ResetAllAlerts(){for(auto &s:meterSources){std::lock_guard<std::mutex> lock(s->mutex);ResetAlert(s->inputAlert);ResetAlert(s->outputAlert);}}
static void ToggleWarnings(){bool enabled=!audibleWarningsEnabled.load();audibleWarningsEnabled=enabled;ResetAllAlerts();UpdateAudibleOutput();AnnounceAccessibility(AMText(enabled?AM_WARNINGS_ON:AM_WARNINGS_OFF));}
static bool EditableFocus(QWidget *w){for(QWidget *p=w;p;p=p->parentWidget())if(p->inherits("QLineEdit")||p->inherits("QTextEdit")||p->inherits("QPlainTextEdit")||p->inherits("QAbstractSpinBox")||p->inherits("QComboBox"))return true;return false;}
class AudibleMeterKeyFilter final:public QObject{public:using QObject::QObject;protected:bool eventFilter(QObject *o,QEvent *e)override{
    if(e->type()!=QEvent::KeyPress||!audibleMeterActive.load()||QApplication::applicationState()!=Qt::ApplicationActive)return QObject::eventFilter(o,e);auto *k=static_cast<QKeyEvent*>(e);if(k->isAutoRepeat()||k->modifiers()!=Qt::NoModifier||EditableFocus(QApplication::focusWidget()))return QObject::eventFilter(o,e);
    switch(k->key()){case Qt::Key_I:ToggleWarnings();return true;case Qt::Key_H:SelectedInstant();return true;case Qt::Key_J:LoudestInstant();return true;case Qt::Key_K:SelectedOverall();return true;case Qt::Key_L:LoudestOverall();return true;default:return QObject::eventFilter(o,e);}
}};
static void ServiceTick(){if(!audibleMeterActive.load())return;if(++refreshTicks>=10){refreshTicks=0;RefreshSources();}UpdateAudibleOutput();}
static void StartTimer(){if(meterTimer)return;meterTimer=new QTimer(PluginEventTarget());meterTimer->setInterval(100);QObject::connect(meterTimer,&QTimer::timeout,PluginEventTarget(),ServiceTick);meterTimer->start();}
static void StopTimer(){if(meterTimer){meterTimer->stop();delete meterTimer;meterTimer=nullptr;}refreshTicks=0;}
static void StartAudibleMeter(){LoadInputDecisions();sessionSources.clear();selectedUuid=consoleFocusedUuid;collectionSwitchInProgress=false;audibleWarningsEnabled=true;audibleMeterActive=true;StartMeters();StartTimer();AnnounceAccessibility(AMText(AM_STARTED));}
static void StopAudibleMeter(bool announce){if(!audibleMeterActive.exchange(false))return;collectionSwitchInProgress=false;CloseInputWarningDialog();SetTone(AudibleTone::None);StopTimer();StopMeters();sessionSources.clear();selectedUuid.clear();consoleFocusedUuid.clear();consoleOpen=false;if(announce)AnnounceAccessibility(AMText(AM_STOPPED));}
}
static std::string AudibleMeterCommandLabel(){QByteArray v=AMText(AM_COMMAND).toUtf8();return {v.constData(),size_t(v.size())};}
static void AudibleMeterHotkey(void*,hotkey_id,obs_hotkey*,bool pressed){if(pressed&&PluginEventTarget())QMetaObject::invokeMethod(PluginEventTarget(),[]{audibleMeterActive.load()?StopAudibleMeter(true):StartAudibleMeter();},Qt::QueuedConnection);}
static void InitializeAudibleMeter(){for(int i=0;i<AM_BINS;++i)binEnergy[static_cast<size_t>(i)]=std::pow(10.0,(i+AM_MIN_DB)/10.0);qApp->installEventFilter(new AudibleMeterKeyFilter(pluginEventContext));}
static void AudibleMeterConsoleOpened(){consoleOpen=true;UpdateAudibleOutput();}
static void AudibleMeterConsoleClosed(){consoleOpen=false;consoleFocusedUuid.clear();UpdateAudibleOutput();}
static void AudibleMeterConsoleFocusSource(const QString &uuid){consoleFocusedUuid=uuid;if(audibleMeterActive.load())selectedUuid=uuid;if(consoleOpen)UpdateAudibleOutput();}
static QString AudibleMeterPreferredConsoleSource(){if(!audibleMeterActive.load())return {};return selectedUuid.isEmpty()?LoudestUuid():selectedUuid;}
static QString AudibleMeterStatusText(){return audibleMeterActive.load()?QStringLiteral("Audible Meter on. Warnings %1.").arg(audibleWarningsEnabled.load()?QStringLiteral("on"):QStringLiteral("off")):QStringLiteral("Audible Meter off.");}
static void ShutdownAudibleMeter(){StopAudibleMeter(false);}
static void HandleAudibleMeterFrontendEvent(int event){
    constexpr int COLLECTION_CHANGED=13,PROFILE_CHANGED=15,EXIT=17,COLLECTION_CLEANUP=25,COLLECTION_CHANGING=35;
    if(event==EXIT){ShutdownAudibleMeter();return;}
    if(event==COLLECTION_CHANGING)ShutdownVolumeConsole();
    if((event==COLLECTION_CHANGING||event==COLLECTION_CLEANUP)&&audibleMeterActive.load()){collectionSwitchInProgress=true;CloseInputWarningDialog();SetTone(AudibleTone::None);SuspendMeters();return;}
    if(event==COLLECTION_CHANGED)collectionSwitchInProgress=false;
    if((event==COLLECTION_CHANGED||event==PROFILE_CHANGED)&&audibleMeterActive.load())if(QObject *t=PluginEventTarget())QTimer::singleShot(0,t,RefreshSources);
}
