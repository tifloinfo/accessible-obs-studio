// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Tiflo.Info

namespace {

constexpr uint32_t CG_AUDIO_FLAG=1u<<1;
constexpr int CG_MAX_CHANNELS=8;
constexpr int CG_FADER_LOG=2;
constexpr int CG_SAMPLE_PEAK=0;
constexpr int CG_TRUE_PEAK=1;
constexpr size_t CG_MAX_HISTORY_EVENTS=10000;
constexpr const char *CG_LIMITER_ID="limiter_filter";
constexpr const char *CG_LIMITER_NAME="Accessible OBS Studio ClipGuard";

using ClipGuardText=std::array<const char*,6>;
static QString CGText(const ClipGuardText &text){return QString::fromUtf8(text[LanguageIndex()]);}
static const ClipGuardText CG_COMMAND={".ClipGuard Sound Check",".ClipGuard-Soundcheck",".Проверка звука ClipGuard",".Перевірка звуку ClipGuard",".Vérification sonore ClipGuard",".Comprobación de sonido ClipGuard"};
static const ClipGuardText CG_CANCEL_COMMAND={".Cancel ClipGuard Sound Check",".ClipGuard-Soundcheck abbrechen",".Отменить проверку звука ClipGuard",".Скасувати перевірку звуку ClipGuard",".Annuler la vérification sonore ClipGuard",".Cancelar la comprobación de sonido ClipGuard"};
static const ClipGuardText CG_WINDOW_TITLE={"ClipGuard Window","ClipGuard-Fenster","Окно ClipGuard","Вікно ClipGuard","Fenêtre ClipGuard","Ventana ClipGuard"};
static const ClipGuardText CG_HISTORY_NAME={"Sound Check history","Soundcheck-Verlauf","Журнал проверки звука","Журнал перевірки звуку","Historique de la vérification sonore","Historial de comprobación de sonido"};
static const ClipGuardText CG_SETTINGS_BUTTON={"Settings","Einstellungen","Настройки","Налаштування","Paramètres","Configuración"};
static const ClipGuardText CG_SAVE_HISTORY_BUTTON={"Save History","Verlauf speichern","Сохранить журнал","Зберегти журнал","Enregistrer l’historique","Guardar historial"};
static const ClipGuardText CG_COMPLETE_BUTTON={"Complete Sound Check","Soundcheck abschließen","Завершить проверку звука","Завершити перевірку звуку","Terminer la vérification sonore","Completar comprobación de sonido"};
static const ClipGuardText CG_CANCEL_BUTTON={"Cancel Sound Check","Soundcheck abbrechen","Отменить проверку звука","Скасувати перевірку звуку","Annuler la vérification sonore","Cancelar comprobación de sonido"};

enum class ClipGuardMode {Idle,SoundCheck};
enum class PeakCondition {OutputNear,OutputClip,InputNear,InputClip};
enum class PeakWarningTone {None,Input,Output};

struct ClipGuardConfig {
    int version{1};
    double nearClippingDb{-3.0};
    double clippingDb{-0.5};
    int sustainedMilliseconds{800};
    int clippingExposureMilliseconds{100};
    int recoveryMilliseconds{3000};
    int eventCooldownMilliseconds{10000};
    double limiterThresholdDb{-3.0};
    int limiterReleaseMilliseconds{60};
    bool autoSaveHistory{true};
    int historyRetentionDays{30};
};

struct PeakHistoryEvent {
    QDateTime time;
    QString uuid;
    QString source;
    PeakCondition condition{PeakCondition::OutputNear};
    double maximumDb{};
    int durationMilliseconds{};
};

struct LevelTracker {
    double nearExposureMs{};
    double clipExposureMs{};
    double recoveryMs{};
    double maximumDb{-INFINITY};
    bool latched{};
    std::chrono::steady_clock::time_point last{};
    std::chrono::steady_clock::time_point lastEvent{};
};

struct PendingPeakEvent {
    qint64 epochMilliseconds{};
    double maximumDb{};
    int durationMilliseconds{};
    bool input{};
    bool clipping{};
};

struct PeakSource {
    void *source{};
    obs_volmeter *meter{};
    QString uuid;
    QString name;
    int channels{2};
    ClipGuardConfig config;
    std::mutex mutex;
    LevelTracker input;
    LevelTracker output;
    double maximumInput{-INFINITY};
    double maximumOutput{-INFINITY};
    int inputEvents{};
    int outputEvents{};
    bool exercised{};
    double warningInputDb{-INFINITY};
    double warningOutputDb{-INFINITY};
    std::chrono::steady_clock::time_point warningUpdated{};
    std::array<PendingPeakEvent,16> pendingEvents{};
    size_t pendingEventCount{};
};

struct PeakSourceSnapshot {
    QString uuid;
    QString name;
    int outputEvents{};
};

static std::atomic<ClipGuardMode> clipGuardMode{ClipGuardMode::Idle};
static ClipGuardConfig clipGuardConfig;
static QString clipGuardConfigError;
static std::vector<std::unique_ptr<PeakSource>> peakSources;
static std::vector<PeakHistoryEvent> peakHistory;
static std::vector<PeakSourceSnapshot> sessionSafeguards;
static QString peakHistoryBasePath;
static QDateTime peakHistoryStarted;
static bool peakHistoryDirty{};
static size_t peakHistoryDropped{};
static std::chrono::steady_clock::time_point peakHistoryLastSave{};
static QPointer<QDialog> clipGuardWindow;
static QTimer *peakSourceRefreshTimer{};
static QStringList temporarilyDisabledOwnedLimiterSources;
static QByteArray peakInputWarningSound;
static QByteArray peakOutputWarningSound;
static PeakWarningTone peakWarningTone{PeakWarningTone::None};
static QString peakWarningTargetUuid;
static PeakWarningTone peakWarningTargetTone{PeakWarningTone::None};

static QString ClipGuardConfigDirectory(){
    QString root=qEnvironmentVariable("APPDATA");
    if(root.isEmpty())root=QDir::homePath();
    return QDir(root).filePath(QStringLiteral("obs-studio/plugin_config/accessible-obs-studio"));
}

static QString ClipGuardConfigPath(){return QDir(ClipGuardConfigDirectory()).filePath(QStringLiteral("clip-guard.json"));}
static QString ClipGuardSchemaPath(){return QDir(ClipGuardConfigDirectory()).filePath(QStringLiteral("clip-guard.schema.json"));}
static QString ClipGuardDefaultsPath(){return QDir(ClipGuardConfigDirectory()).filePath(QStringLiteral("clip-guard.defaults.json"));}
static QString ClipGuardLogsDirectory(){return QDir(ClipGuardConfigDirectory()).filePath(QStringLiteral("clip-guard-history"));}

static QJsonObject ClipGuardJson(const ClipGuardConfig &config){
    QJsonObject analysis{{QStringLiteral("nearClippingDb"),config.nearClippingDb},
                         {QStringLiteral("clippingDb"),config.clippingDb},
                         {QStringLiteral("sustainedMilliseconds"),config.sustainedMilliseconds},
                         {QStringLiteral("clippingExposureMilliseconds"),config.clippingExposureMilliseconds},
                         {QStringLiteral("recoveryMilliseconds"),config.recoveryMilliseconds},
                         {QStringLiteral("eventCooldownMilliseconds"),config.eventCooldownMilliseconds}};
    QJsonObject limiter{{QStringLiteral("thresholdDb"),config.limiterThresholdDb},
                        {QStringLiteral("releaseMilliseconds"),config.limiterReleaseMilliseconds}};
    QJsonObject history{{QStringLiteral("autoSave"),config.autoSaveHistory},
                        {QStringLiteral("retentionDays"),config.historyRetentionDays}};
    return {{QStringLiteral("$schema"),QStringLiteral("clip-guard.schema.json")},
            {QStringLiteral("configurationVersion"),config.version},
            {QStringLiteral("analysis"),analysis},
            {QStringLiteral("limiter"),limiter},
            {QStringLiteral("history"),history}};
}

static const char *ClipGuardSchemaText(){
    return R"JSON({
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "title": "Accessible OBS Studio ClipGuard configuration",
  "description": "User-editable settings for ClipGuard Sound Check. Times are milliseconds and levels are decibels relative to full scale.",
  "type": "object",
  "additionalProperties": false,
  "required": ["configurationVersion", "analysis", "limiter", "history"],
  "properties": {
    "$schema": {"type": "string", "description": "Local schema used to document and validate this file."},
    "configurationVersion": {"const": 1, "description": "Configuration format version."},
    "analysis": {
      "type": "object",
      "additionalProperties": false,
      "required": ["nearClippingDb", "clippingDb", "sustainedMilliseconds", "clippingExposureMilliseconds", "recoveryMilliseconds", "eventCooldownMilliseconds"],
      "properties": {
        "nearClippingDb": {"type": "number", "minimum": -20, "maximum": -0.6, "description": "Level at which sustained near-clipping analysis begins."},
        "clippingDb": {"type": "number", "minimum": -6, "maximum": 0, "description": "Level treated as clipping or immediate danger."},
        "sustainedMilliseconds": {"type": "integer", "minimum": 100, "maximum": 10000, "description": "Weighted near-clipping exposure required before an event is reported."},
        "clippingExposureMilliseconds": {"type": "integer", "minimum": 20, "maximum": 2000, "description": "Clipping exposure required before an event is reported."},
        "recoveryMilliseconds": {"type": "integer", "minimum": 250, "maximum": 30000, "description": "Safe time required before the same source may produce another event."},
        "eventCooldownMilliseconds": {"type": "integer", "minimum": 1000, "maximum": 120000, "description": "Minimum separation between repeated events from one source."}
      }
    },
    "limiter": {
      "type": "object",
      "additionalProperties": false,
      "required": ["thresholdDb", "releaseMilliseconds"],
      "properties": {
        "thresholdDb": {"type": "number", "minimum": -20, "maximum": -0.5, "description": "Threshold used for ClipGuard-owned OBS limiter filters."},
        "releaseMilliseconds": {"type": "integer", "minimum": 1, "maximum": 1000, "description": "Release time used for ClipGuard-owned OBS limiter filters."}
      }
    },
    "history": {
      "type": "object",
      "additionalProperties": false,
      "required": ["autoSave", "retentionDays"],
      "properties": {
        "autoSave": {"type": "boolean", "description": "Write text and JSON history files throughout each session."},
        "retentionDays": {"type": "integer", "minimum": 1, "maximum": 3650, "description": "Number of days automatic history files are retained."}
      }
    }
  }
})JSON";
}

static bool WriteBytesAtomically(const QString &path,const QByteArray &bytes){
    QSaveFile file(path);if(!file.open(QIODevice::WriteOnly))return false;
    if(file.write(bytes)!=bytes.size()){file.cancelWriting();return false;}
    return file.commit();
}

static bool EnsureClipGuardSupportFiles(QString *error=nullptr){
    if(!QDir().mkpath(ClipGuardConfigDirectory())){
        if(error)*error=QStringLiteral("The ClipGuard configuration directory could not be created.");
        return false;
    }
    if(!QFile::exists(ClipGuardSchemaPath())&&!WriteBytesAtomically(ClipGuardSchemaPath(),QByteArray(ClipGuardSchemaText()))){
        if(error)*error=QStringLiteral("The ClipGuard configuration schema could not be saved.");
        return false;
    }
    if(!QFile::exists(ClipGuardDefaultsPath())&&!WriteBytesAtomically(ClipGuardDefaultsPath(),QJsonDocument(ClipGuardJson(ClipGuardConfig{})).toJson(QJsonDocument::Indented))){
        if(error)*error=QStringLiteral("The ClipGuard default configuration could not be saved.");
        return false;
    }
    return true;
}

static bool SaveClipGuardConfig(const ClipGuardConfig &config,QString *error=nullptr){
    if(!EnsureClipGuardSupportFiles(error))return false;
    if(!WriteBytesAtomically(ClipGuardConfigPath(),QJsonDocument(ClipGuardJson(config)).toJson(QJsonDocument::Indented))){
        if(error)*error=QStringLiteral("The ClipGuard configuration file could not be saved.");
        return false;
    }
    clipGuardConfig=config;clipGuardConfigError.clear();return true;
}

static bool ReadNumber(const QJsonObject &object,const char *name,double minimum,double maximum,double &value,QString &error){
    QJsonValue raw=object.value(QString::fromLatin1(name));if(!raw.isDouble()){error=QStringLiteral("%1 must be a number.").arg(QString::fromLatin1(name));return false;}
    double candidate=raw.toDouble();if(candidate<minimum||candidate>maximum){error=QStringLiteral("%1 must be between %2 and %3.").arg(QString::fromLatin1(name)).arg(minimum).arg(maximum);return false;}value=candidate;return true;
}

static bool ReadInteger(const QJsonObject &object,const char *name,int minimum,int maximum,int &value,QString &error){
    double number{};if(!ReadNumber(object,name,minimum,maximum,number,error))return false;
    if(std::floor(number)!=number){error=QStringLiteral("%1 must be a whole number.").arg(QString::fromLatin1(name));return false;}value=static_cast<int>(number);return true;
}

static bool HasOnlyKeys(const QJsonObject &object,const QStringList &allowed,QString &error){
    for(auto iterator=object.constBegin();iterator!=object.constEnd();++iterator){
        if(!allowed.contains(iterator.key())){error=QStringLiteral("Unknown ClipGuard setting: %1.").arg(iterator.key());return false;}
    }
    return true;
}

static bool LoadClipGuardConfig(QString *reportedError=nullptr){
    QString supportError;
    if(!EnsureClipGuardSupportFiles(&supportError)){clipGuardConfigError=supportError;if(reportedError)*reportedError=supportError;return false;}
    ClipGuardConfig loaded;
    QFile file(ClipGuardConfigPath());
    if(!file.exists()){QString error;if(!SaveClipGuardConfig(loaded,&error)){clipGuardConfigError=error;if(reportedError)*reportedError=error;return false;}return true;}
    if(!file.open(QIODevice::ReadOnly)){clipGuardConfigError=QStringLiteral("The ClipGuard configuration file could not be opened.");if(reportedError)*reportedError=clipGuardConfigError;return false;}
    QJsonParseError parse;QJsonDocument document=QJsonDocument::fromJson(file.readAll(),&parse);
    if(parse.error!=QJsonParseError::NoError||!document.isObject()){clipGuardConfigError=QStringLiteral("Invalid ClipGuard JSON at offset %1: %2").arg(parse.offset).arg(parse.errorString());if(reportedError)*reportedError=clipGuardConfigError;return false;}
    QJsonObject root=document.object();QString error;
    HasOnlyKeys(root,{QStringLiteral("$schema"),QStringLiteral("configurationVersion"),QStringLiteral("analysis"),QStringLiteral("limiter"),QStringLiteral("history")},error);
    if(error.isEmpty()&&root.value(QStringLiteral("configurationVersion")).toInt()!=1)error=QStringLiteral("configurationVersion must be 1.");
    QJsonObject analysis=root.value(QStringLiteral("analysis")).toObject(),limiter=root.value(QStringLiteral("limiter")).toObject(),history=root.value(QStringLiteral("history")).toObject();
    if(error.isEmpty()&&analysis.isEmpty())error=QStringLiteral("analysis must be an object.");
    if(error.isEmpty())HasOnlyKeys(analysis,{QStringLiteral("nearClippingDb"),QStringLiteral("clippingDb"),QStringLiteral("sustainedMilliseconds"),QStringLiteral("clippingExposureMilliseconds"),QStringLiteral("recoveryMilliseconds"),QStringLiteral("eventCooldownMilliseconds")},error);
    if(error.isEmpty()&&!ReadNumber(analysis,"nearClippingDb",-20.0,-0.6,loaded.nearClippingDb,error)){}
    else if(error.isEmpty()&&!ReadNumber(analysis,"clippingDb",-6.0,0.0,loaded.clippingDb,error)){}
    else if(error.isEmpty()&&!ReadInteger(analysis,"sustainedMilliseconds",100,10000,loaded.sustainedMilliseconds,error)){}
    else if(error.isEmpty()&&!ReadInteger(analysis,"clippingExposureMilliseconds",20,2000,loaded.clippingExposureMilliseconds,error)){}
    else if(error.isEmpty()&&!ReadInteger(analysis,"recoveryMilliseconds",250,30000,loaded.recoveryMilliseconds,error)){}
    else if(error.isEmpty()&&!ReadInteger(analysis,"eventCooldownMilliseconds",1000,120000,loaded.eventCooldownMilliseconds,error)){}
    if(error.isEmpty()&&loaded.nearClippingDb>=loaded.clippingDb)error=QStringLiteral("nearClippingDb must be lower than clippingDb.");
    if(error.isEmpty()&&limiter.isEmpty())error=QStringLiteral("limiter must be an object.");
    if(error.isEmpty())HasOnlyKeys(limiter,{QStringLiteral("thresholdDb"),QStringLiteral("releaseMilliseconds")},error);
    if(error.isEmpty()&&!ReadNumber(limiter,"thresholdDb",-20.0,-0.5,loaded.limiterThresholdDb,error)){}
    else if(error.isEmpty()&&!ReadInteger(limiter,"releaseMilliseconds",1,1000,loaded.limiterReleaseMilliseconds,error)){}
    if(error.isEmpty()&&history.isEmpty())error=QStringLiteral("history must be an object.");
    if(error.isEmpty())HasOnlyKeys(history,{QStringLiteral("autoSave"),QStringLiteral("retentionDays")},error);
    if(error.isEmpty()){
        QJsonValue autoSave=history.value(QStringLiteral("autoSave"));if(!autoSave.isBool())error=QStringLiteral("autoSave must be true or false.");else loaded.autoSaveHistory=autoSave.toBool();
    }
    if(error.isEmpty()&&!ReadInteger(history,"retentionDays",1,3650,loaded.historyRetentionDays,error)){}
    if(!error.isEmpty()){clipGuardConfigError=error;if(reportedError)*reportedError=error;return false;}
    clipGuardConfig=loaded;clipGuardConfigError.clear();return true;
}

static QString PeakConditionText(PeakCondition condition){
    switch(condition){
    case PeakCondition::OutputNear:return QStringLiteral("Output near clipping");
    case PeakCondition::OutputClip:return QStringLiteral("Output clipping");
    case PeakCondition::InputNear:return QStringLiteral("Input near clipping before OBS volume");
    case PeakCondition::InputClip:return QStringLiteral("Possible capture-device clipping");
    }return {};
}

static QString PeakEventLine(const PeakHistoryEvent &event){
    return QStringLiteral("%1. %2. %3. Peak %4 dB. Duration %5 milliseconds.")
        .arg(event.time.toString(QStringLiteral("HH:mm:ss")),event.source,PeakConditionText(event.condition))
        .arg(event.maximumDb,0,'f',1).arg(event.durationMilliseconds);
}

static QJsonObject PeakEventJson(const PeakHistoryEvent &event){
    return {{QStringLiteral("time"),event.time.toString(Qt::ISODateWithMs)},
            {QStringLiteral("sourceUuid"),event.uuid},{QStringLiteral("sourceName"),event.source},
            {QStringLiteral("condition"),PeakConditionText(event.condition)},
            {QStringLiteral("maximumDb"),event.maximumDb},
            {QStringLiteral("durationMilliseconds"),event.durationMilliseconds},
            {QStringLiteral("mode"),QStringLiteral("soundCheck")}};
}

static QString PeakHistoryText(){
    QString result=QStringLiteral("Accessible OBS Studio ClipGuard\nStarted: %1\n\n").arg(peakHistoryStarted.toString(Qt::ISODateWithMs));
    for(const PeakHistoryEvent &event:peakHistory)result+=PeakEventLine(event)+QLatin1Char('\n');
    if(peakHistoryDropped>0)result+=QStringLiteral("\n%1 additional events were omitted after the safety limit was reached.\n").arg(peakHistoryDropped);
    return result;
}

static bool SavePeakHistory(QString *error=nullptr){
    if(peakHistoryBasePath.isEmpty()||!clipGuardConfig.autoSaveHistory)return true;
    QJsonArray events;for(const PeakHistoryEvent &event:peakHistory)events.append(PeakEventJson(event));
    QJsonObject root{{QStringLiteral("formatVersion"),1},{QStringLiteral("started"),peakHistoryStarted.toString(Qt::ISODateWithMs)},{QStringLiteral("events"),events},{QStringLiteral("omittedEvents"),static_cast<qint64>(peakHistoryDropped)}};
    const bool jsonSaved=WriteBytesAtomically(peakHistoryBasePath+QStringLiteral(".json"),QJsonDocument(root).toJson(QJsonDocument::Indented));
    const bool textSaved=WriteBytesAtomically(peakHistoryBasePath+QStringLiteral(".txt"),PeakHistoryText().toUtf8());
    if(!jsonSaved||!textSaved){
        if(error)*error=QStringLiteral("ClipGuard could not save %1 history file%2. Check access to %3.")
            .arg(!jsonSaved&&!textSaved?QStringLiteral("its text and JSON"):(!jsonSaved?QStringLiteral("the JSON"):QStringLiteral("the text")))
            .arg(!jsonSaved&&!textSaved?QStringLiteral("s"):QString())
            .arg(ClipGuardLogsDirectory());
        return false;
    }
    peakHistoryDirty=false;
    peakHistoryLastSave=std::chrono::steady_clock::now();
    return true;
}

static void MaybeSavePeakHistory(){
    if(!peakHistoryDirty||!clipGuardConfig.autoSaveHistory)return;
    const auto now=std::chrono::steady_clock::now();
    if(peakHistoryLastSave.time_since_epoch().count()!=0&&std::chrono::duration_cast<std::chrono::seconds>(now-peakHistoryLastSave).count()<5)return;
    SavePeakHistory();
}

static void RemoveExpiredPeakHistory(){
    QDir directory(ClipGuardLogsDirectory());if(!directory.exists())return;
    QDateTime cutoff=QDateTime::currentDateTime().addDays(-clipGuardConfig.historyRetentionDays);
    for(const QFileInfo &file:directory.entryInfoList({QStringLiteral("*.txt"),QStringLiteral("*.json")},QDir::Files))if(file.lastModified()<cutoff)QFile::remove(file.absoluteFilePath());
}

static bool BeginPeakHistory(QString *error=nullptr){
    peakHistory.clear();sessionSafeguards.clear();peakHistoryDropped=0;peakHistoryDirty=true;peakHistoryLastSave={};peakHistoryStarted=QDateTime::currentDateTime();
    if(!QDir().mkpath(ClipGuardLogsDirectory())){if(error)*error=QStringLiteral("The ClipGuard history directory could not be created. Sound Check will continue without automatic history files.");peakHistoryBasePath.clear();return false;}
    RemoveExpiredPeakHistory();
    peakHistoryBasePath=QDir(ClipGuardLogsDirectory()).filePath(QStringLiteral("clip-guard-%1").arg(peakHistoryStarted.toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"))));
    return SavePeakHistory(error);
}

static void DiscardPeakHistory(){
    if(!peakHistoryBasePath.isEmpty()){QFile::remove(peakHistoryBasePath+QStringLiteral(".json"));QFile::remove(peakHistoryBasePath+QStringLiteral(".txt"));}
    peakHistory.clear();sessionSafeguards.clear();peakHistoryBasePath.clear();peakHistoryStarted={};peakHistoryDirty=false;peakHistoryDropped=0;peakHistoryLastSave={};
}

static void BuildPeakWarningSound(QByteArray &sound,double frequency){
    if(!sound.isEmpty())return;constexpr int sampleRate=22050;constexpr int samples=sampleRate/2;constexpr int dataBytes=samples*2;sound.resize(44+dataBytes);auto *bytes=reinterpret_cast<unsigned char*>(sound.data());
    auto word=[bytes](int offset,uint16_t value){bytes[offset]=static_cast<unsigned char>(value&0xff);bytes[offset+1]=static_cast<unsigned char>((value>>8)&0xff);};
    auto dword=[bytes](int offset,uint32_t value){for(int byte=0;byte<4;++byte)bytes[offset+byte]=static_cast<unsigned char>((value>>(byte*8))&0xff);};
    std::memcpy(bytes,"RIFF",4);dword(4,36+dataBytes);std::memcpy(bytes+8,"WAVEfmt ",8);dword(16,16);word(20,1);word(22,1);dword(24,sampleRate);dword(28,sampleRate*2);word(32,2);word(34,16);std::memcpy(bytes+36,"data",4);dword(40,dataBytes);
    auto *pcm=reinterpret_cast<int16_t*>(bytes+44);constexpr double amplitude=2100.0;for(int sample=0;sample<samples;++sample)pcm[sample]=static_cast<int16_t>(std::lround(amplitude*std::sin(2.0*3.14159265358979323846*frequency*sample/sampleRate)));
}

static void SetPeakWarningSound(PeakWarningTone tone){
    if(tone==peakWarningTone)return;
    if(tone==PeakWarningTone::None){PlaySoundW(nullptr,nullptr,0);peakWarningTone=PeakWarningTone::None;return;}
    QByteArray &sound=tone==PeakWarningTone::Input?peakInputWarningSound:peakOutputWarningSound;BuildPeakWarningSound(sound,tone==PeakWarningTone::Input?475.0:700.0);
    peakWarningTone=PlaySoundW(reinterpret_cast<LPCWSTR>(sound.constData()),nullptr,SND_MEMORY|SND_ASYNC|SND_LOOP|SND_NODEFAULT)!=FALSE?tone:PeakWarningTone::None;
}

struct ActivePeakWarning{QString uuid;double db{-INFINITY};bool clipping{};PeakWarningTone tone{PeakWarningTone::None};};

static std::vector<ActivePeakWarning> ActivePeakWarnings(){
    std::vector<ActivePeakWarning> warnings;auto now=std::chrono::steady_clock::now();
    for(const auto &source:peakSources){
        std::lock_guard<std::mutex> lock(source->mutex);if(source->warningUpdated.time_since_epoch().count()==0||std::chrono::duration_cast<std::chrono::milliseconds>(now-source->warningUpdated).count()>500)continue;
        if(source->input.latched&&source->warningInputDb>=source->config.nearClippingDb)warnings.push_back({source->uuid,source->warningInputDb,source->warningInputDb>=source->config.clippingDb,PeakWarningTone::Input});
        if(source->output.latched&&source->warningOutputDb>=source->config.nearClippingDb)warnings.push_back({source->uuid,source->warningOutputDb,source->warningOutputDb>=source->config.clippingDb,PeakWarningTone::Output});
    }
    return warnings;
}

static void UpdatePeakWarning(){
    if(clipGuardMode.load()!=ClipGuardMode::SoundCheck){SetPeakWarningSound(PeakWarningTone::None);peakWarningTargetUuid.clear();peakWarningTargetTone=PeakWarningTone::None;return;}
    std::vector<ActivePeakWarning> warnings=ActivePeakWarnings();PeakWarningTone tone=std::any_of(warnings.begin(),warnings.end(),[](const ActivePeakWarning &warning){return warning.tone==PeakWarningTone::Input;})?PeakWarningTone::Input:(warnings.empty()?PeakWarningTone::None:PeakWarningTone::Output);SetPeakWarningSound(tone);
    if(warnings.empty()){peakWarningTargetUuid.clear();peakWarningTargetTone=PeakWarningTone::None;return;}
    auto current=std::find_if(warnings.begin(),warnings.end(),[&](const ActivePeakWarning &warning){return warning.uuid==peakWarningTargetUuid&&warning.tone==peakWarningTargetTone&&warning.tone==tone;});if(current!=warnings.end())return;
    const ActivePeakWarning *mostSevere=nullptr;for(const ActivePeakWarning &warning:warnings){if(warning.tone!=tone)continue;if(!mostSevere||(warning.clipping&&!mostSevere->clipping)||(warning.clipping==mostSevere->clipping&&warning.db>mostSevere->db))mostSevere=&warning;}
    if(!mostSevere)return;peakWarningTargetUuid=mostSevere->uuid;peakWarningTargetTone=mostSevere->tone;OpenVolumeConsoleForSource(peakWarningTargetUuid,true);
}

static void StopPeakWarning(){
    SetPeakWarningSound(PeakWarningTone::None);peakWarningTargetUuid.clear();peakWarningTargetTone=PeakWarningTone::None;
}

#if 0
class PeakHistoryWebView final:public QWidget {
public:
    explicit PeakHistoryWebView(QWidget *parent):QWidget(parent){
        using Microsoft::WRL::Callback;
        setAttribute(Qt::WA_NativeWindow);setFocusPolicy(Qt::StrongFocus);setAccessibleName(QStringLiteral("Sound Check history"));
        fallback_=new QPlainTextEdit(this);fallback_->setReadOnly(true);fallback_->setUndoRedoEnabled(false);fallback_->setAccessibleName(QStringLiteral("Sound Check history"));fallback_->setPlainText(PeakHistoryText());fallback_->hide();
        pending_.push_back({{QStringLiteral("type"),QStringLiteral("started")},{QStringLiteral("time"),peakHistoryStarted.toString(Qt::ISODateWithMs)}});
        QPointer<PeakHistoryWebView> self(this);HWND host=reinterpret_cast<HWND>(winId());wchar_t localAppData[MAX_PATH]{};GetEnvironmentVariableW(L"LOCALAPPDATA",localAppData,MAX_PATH);std::wstring userData=std::wstring(localAppData)+L"\\AccessibleOBSStudio\\WebView2";
        HRESULT result=CreateCoreWebView2EnvironmentWithOptions(nullptr,userData.c_str(),nullptr,Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([self,host](HRESULT environmentResult,ICoreWebView2Environment *environment)->HRESULT{
            if(!self)return S_OK;if(FAILED(environmentResult)||!environment){self->ShowFallback();return S_OK;}
            HRESULT controllerResult=environment->CreateCoreWebView2Controller(host,Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([self](HRESULT creationResult,ICoreWebView2Controller *controller)->HRESULT{
                if(!self)return S_OK;if(FAILED(creationResult)||!controller){self->ShowFallback();return S_OK;}self->InitializeController(controller);return S_OK;
            }).Get());
            if(FAILED(controllerResult))self->ShowFallback();return S_OK;
        }).Get());
        if(FAILED(result))ShowFallback();
    }
    ~PeakHistoryWebView() override{
        if(webView_&&navigationHandler_)webView_->remove_NavigationCompleted(navigationToken_);
        if(controller_&&moveFocusHandler_)controller_->remove_MoveFocusRequested(moveFocusToken_);
        if(controller_&&acceleratorHandler_)controller_->remove_AcceleratorKeyPressed(acceleratorToken_);
        webView_.Reset();if(controller_){controller_->Close();controller_.Reset();}
    }
    void AppendEvent(const PeakHistoryEvent &event){
        fallback_->appendPlainText(PeakEventLine(event));QJsonObject message{{QStringLiteral("type"),QStringLiteral("append")},
            {QStringLiteral("time"),event.time.toString(QStringLiteral("HH:mm:ss"))},{QStringLiteral("source"),event.source},
            {QStringLiteral("condition"),PeakConditionText(event.condition)},{QStringLiteral("peak"),QStringLiteral("%1 dB").arg(event.maximumDb,0,'f',1)},
            {QStringLiteral("duration"),QStringLiteral("%1 milliseconds").arg(event.durationMilliseconds)}};
        pending_.push_back(std::move(message));if(ready_)FlushPending();
    }
    std::function<void()> escapeRequested;
protected:
    void resizeEvent(QResizeEvent *event) override{QWidget::resizeEvent(event);if(fallback_)fallback_->setGeometry(rect());ResizeController();}
    void focusInEvent(QFocusEvent *event) override{QWidget::focusInEvent(event);if(fallback_->isVisible())fallback_->setFocus(event->reason());else if(controller_)controller_->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);}
private:
    static std::wstring Html(){
        return LR"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="color-scheme" content="light dark">
<title>ClipGuard Sound Check history</title>
<style>
  :root { font: 17px/1.5 system-ui, sans-serif; color: CanvasText; background: Canvas; }
  body { max-width: 70rem; margin: 0 auto; padding: 1rem 1.25rem 3rem; }
  h1 { font-size: 1.55rem; }
  #events { padding-left: 2rem; }
  #events > li { margin: 0 0 1.25rem; padding: .75rem 1rem; border: 1px solid GrayText; border-radius: .4rem; }
  h2 { margin: 0 0 .4rem; font-size: 1.15rem; }
  dl { display: grid; grid-template-columns: max-content 1fr; gap: .2rem .8rem; margin: 0; }
  dt { font-weight: 700; }
  dd { margin: 0; }
</style>
</head>
<body>
<main>
  <h1>ClipGuard Sound Check history</h1>
  <p>New peak events are added below. Use screen-reader heading or list navigation to review them.</p>
  <p id="started"></p>
  <p id="summary">No peak events recorded.</p>
  <ol id="events" aria-label="Peak events"></ol>
</main>
<script>
(() => {
  const events = document.getElementById('events');
  const started = document.getElementById('started');
  const summary = document.getElementById('summary');
  let count = 0;
  function detail(list, term, value) {
    const dt = document.createElement('dt');
    const dd = document.createElement('dd');
    dt.textContent = term;
    dd.textContent = value;
    list.append(dt, dd);
  }
  window.chrome.webview.addEventListener('message', event => {
    const item = event.data;
    if (!item) return;
    if (item.type === 'started') {
      started.textContent = `Started: ${item.time}`;
      return;
    }
    if (item.type !== 'append') return;
    const entry = document.createElement('li');
    const article = document.createElement('article');
    const heading = document.createElement('h2');
    const details = document.createElement('dl');
    heading.textContent = `${item.source}: ${item.condition}`;
    detail(details, 'Time', item.time);
    detail(details, 'Peak', item.peak);
    detail(details, 'Duration', item.duration);
    article.append(heading, details);
    entry.append(article);
    events.append(entry);
    count += 1;
    summary.textContent = `${count} peak ${count === 1 ? 'event' : 'events'} recorded.`;
  });
})();
</script>
</body>
</html>)HTML";
    }
    void InitializeController(ICoreWebView2Controller *controller){
        using Microsoft::WRL::Callback;
        controller_=controller;if(FAILED(controller_->get_CoreWebView2(&webView_))||!webView_){ShowFallback();return;}
        Microsoft::WRL::ComPtr<ICoreWebView2Settings> settings;if(SUCCEEDED(webView_->get_Settings(&settings))){settings->put_AreDefaultContextMenusEnabled(FALSE);settings->put_AreDevToolsEnabled(FALSE);settings->put_IsStatusBarEnabled(FALSE);settings->put_IsWebMessageEnabled(TRUE);}
        QPointer<PeakHistoryWebView> self(this);
        HRESULT navigationResult=webView_->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>([self](ICoreWebView2*,ICoreWebView2NavigationCompletedEventArgs *args)->HRESULT{
            if(!self)return S_OK;BOOL success=FALSE;if(!args||FAILED(args->get_IsSuccess(&success))||!success){self->ShowFallback();return S_OK;}self->ready_=true;self->FlushPending();if(self->hasFocus()&&self->controller_)self->controller_->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);return S_OK;
        }).Get(),&navigationToken_);navigationHandler_=SUCCEEDED(navigationResult);
        HRESULT moveResult=controller_->add_MoveFocusRequested(Callback<ICoreWebView2MoveFocusRequestedEventHandler>([self](ICoreWebView2Controller*,ICoreWebView2MoveFocusRequestedEventArgs *args)->HRESULT{
            if(!self||!args)return S_OK;COREWEBVIEW2_MOVE_FOCUS_REASON reason{};if(FAILED(args->get_Reason(&reason))||(reason!=COREWEBVIEW2_MOVE_FOCUS_REASON_NEXT&&reason!=COREWEBVIEW2_MOVE_FOCUS_REASON_PREVIOUS))return S_OK;args->put_Handled(TRUE);bool next=reason==COREWEBVIEW2_MOVE_FOCUS_REASON_NEXT;QTimer::singleShot(0,self,[self,next]{if(self)self->focusNextPrevChild(next);});return S_OK;
        }).Get(),&moveFocusToken_);moveFocusHandler_=SUCCEEDED(moveResult);
        HRESULT acceleratorResult=controller_->add_AcceleratorKeyPressed(Callback<ICoreWebView2AcceleratorKeyPressedEventHandler>([self](ICoreWebView2Controller*,ICoreWebView2AcceleratorKeyPressedEventArgs *args)->HRESULT{
            if(!self||!args)return S_OK;UINT key=0;COREWEBVIEW2_KEY_EVENT_KIND kind{};if(SUCCEEDED(args->get_VirtualKey(&key))&&SUCCEEDED(args->get_KeyEventKind(&kind))&&key==VK_ESCAPE&&(kind==COREWEBVIEW2_KEY_EVENT_KIND_KEY_DOWN||kind==COREWEBVIEW2_KEY_EVENT_KIND_SYSTEM_KEY_DOWN)){args->put_Handled(TRUE);QMetaObject::invokeMethod(self,[self]{if(self&&self->escapeRequested)self->escapeRequested();},Qt::QueuedConnection);}return S_OK;
        }).Get(),&acceleratorToken_);acceleratorHandler_=SUCCEEDED(acceleratorResult);
        ResizeController();HRESULT navigation=webView_->NavigateToString(Html().c_str());if(FAILED(navigation)||!navigationHandler_)ShowFallback();
    }
    void ResizeController(){if(!controller_)return;RECT bounds{};GetClientRect(reinterpret_cast<HWND>(winId()),&bounds);controller_->put_Bounds(bounds);}
    bool Post(const QJsonObject &message){if(!webView_)return false;std::wstring json=QString::fromUtf8(QJsonDocument(message).toJson(QJsonDocument::Compact)).toStdWString();return SUCCEEDED(webView_->PostWebMessageAsJson(json.c_str()));}
    void FlushPending(){while(!pending_.empty()&&Post(pending_.front()))pending_.erase(pending_.begin());}
    void ShowFallback(){ready_=false;if(controller_)controller_->put_IsVisible(FALSE);fallback_->show();fallback_->raise();fallback_->setGeometry(rect());if(hasFocus())fallback_->setFocus(Qt::OtherFocusReason);}
    QPlainTextEdit *fallback_{};
    Microsoft::WRL::ComPtr<ICoreWebView2Controller> controller_;
    Microsoft::WRL::ComPtr<ICoreWebView2> webView_;
    EventRegistrationToken navigationToken_{},moveFocusToken_{},acceleratorToken_{};
    bool navigationHandler_{},moveFocusHandler_{},acceleratorHandler_{},ready_{};
    std::vector<QJsonObject> pending_;
};
#endif

class PeakHistoryView final:public QTreeWidget {
public:
    explicit PeakHistoryView(QWidget *parent):QTreeWidget(parent){
        setColumnCount(5);
        setHeaderLabels({QStringLiteral("Time"),QStringLiteral("Source"),QStringLiteral("Condition"),QStringLiteral("Peak"),QStringLiteral("Duration")});
        setAccessibleName(CGText(CG_HISTORY_NAME));
        setAccessibleDescription(QStringLiteral("Recorded ClipGuard peak events. Use the arrow keys to review events and columns."));
        setRootIsDecorated(false);
        setAlternatingRowColors(true);
        setUniformRowHeights(true);
        for(const PeakHistoryEvent &event:peakHistory)AppendEvent(event);
    }
    void AppendEvent(const PeakHistoryEvent &event){
        auto *item=new QTreeWidgetItem(this,{event.time.toString(QStringLiteral("HH:mm:ss")),event.source,PeakConditionText(event.condition),QStringLiteral("%1 dB").arg(event.maximumDb,0,'f',1),QStringLiteral("%1 ms").arg(event.durationMilliseconds)});
        item->setData(0,Qt::AccessibleTextRole,PeakEventLine(event));
        if(topLevelItemCount()==1)setCurrentItem(item);
    }
    std::function<void()> escapeRequested;
protected:
    void keyPressEvent(QKeyEvent *event) override{
        if(event->key()==Qt::Key_Escape&&event->modifiers()==Qt::NoModifier){if(escapeRequested)escapeRequested();event->accept();return;}
        QTreeWidget::keyPressEvent(event);
    }
};

class ClipGuardWindow;
static ClipGuardWindow *ClipGuardWindowPointer();

static void RecordPeakEvent(const PeakHistoryEvent &event);

static void StorePendingPeakEvent(PeakSource *source,bool input,bool clipping,double maximum,int duration){
    if(source->pendingEventCount>=source->pendingEvents.size())return;
    qint64 epoch=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    source->pendingEvents[source->pendingEventCount++]={epoch,maximum,duration,input,clipping};
}

static void ProcessPeakLevel(PeakSource *source,LevelTracker &tracker,double value,bool input,const std::chrono::steady_clock::time_point now){
    if(!std::isfinite(value))return;
    double elapsed=tracker.last.time_since_epoch().count()==0?0.0:std::chrono::duration<double,std::milli>(now-tracker.last).count();tracker.last=now;elapsed=std::clamp(elapsed,0.0,100.0);
    tracker.maximumDb=std::max(tracker.maximumDb,value);
    if(value>=source->config.nearClippingDb){tracker.nearExposureMs+=elapsed;tracker.recoveryMs=0.0;}else{tracker.nearExposureMs=std::max(0.0,tracker.nearExposureMs-elapsed*2.0);tracker.recoveryMs+=elapsed;}
    if(value>=source->config.clippingDb)tracker.clipExposureMs+=elapsed;else tracker.clipExposureMs=std::max(0.0,tracker.clipExposureMs-elapsed*2.0);
    if(tracker.latched){
        if(tracker.recoveryMs>=source->config.recoveryMilliseconds){auto lastEvent=tracker.lastEvent;tracker=LevelTracker{};tracker.last=now;tracker.lastEvent=lastEvent;}
        return;
    }
    bool clipped=tracker.clipExposureMs>=source->config.clippingExposureMilliseconds;
    bool sustained=tracker.nearExposureMs>=source->config.sustainedMilliseconds;
    if(!clipped&&!sustained)return;
    if(tracker.lastEvent.time_since_epoch().count()!=0&&std::chrono::duration_cast<std::chrono::milliseconds>(now-tracker.lastEvent).count()<source->config.eventCooldownMilliseconds)return;
    tracker.latched=true;tracker.lastEvent=now;int duration=static_cast<int>(std::lround(clipped?tracker.clipExposureMs:tracker.nearExposureMs));
    if(input)++source->inputEvents;else ++source->outputEvents;
    StorePendingPeakEvent(source,input,clipped,tracker.maximumDb,duration);
}

static void PeakMeterCallback(void *parameter,const float*,const float *peak,const float *inputPeak){
    auto *source=static_cast<PeakSource*>(parameter);if(!source||!peak||!inputPeak||clipGuardMode.load()!=ClipGuardMode::SoundCheck)return;
    double output=-INFINITY,input=-INFINITY;for(int channel=0;channel<std::clamp(source->channels,1,CG_MAX_CHANNELS);++channel){if(std::isfinite(peak[channel]))output=std::max(output,static_cast<double>(peak[channel]));if(std::isfinite(inputPeak[channel]))input=std::max(input,static_cast<double>(inputPeak[channel]));}
    if(!std::isfinite(output)&&!std::isfinite(input))return;auto now=std::chrono::steady_clock::now();std::lock_guard<std::mutex> lock(source->mutex);source->exercised=true;source->maximumOutput=std::max(source->maximumOutput,output);source->maximumInput=std::max(source->maximumInput,input);source->warningInputDb=input;source->warningOutputDb=output;source->warningUpdated=now;
    ProcessPeakLevel(source,source->input,input,true,now);ProcessPeakLevel(source,source->output,output,false,now);
}

static void DrainPeakEvents(){
    for(const auto &source:peakSources){
        std::array<PendingPeakEvent,16> events{};size_t count=0;QString uuid,name;
        {
            std::lock_guard<std::mutex> lock(source->mutex);count=source->pendingEventCount;std::copy_n(source->pendingEvents.begin(),count,events.begin());source->pendingEventCount=0;uuid=source->uuid;name=source->name;
        }
        for(size_t index=0;index<count;++index){const PendingPeakEvent &pending=events[index];PeakHistoryEvent event{QDateTime::fromMSecsSinceEpoch(pending.epochMilliseconds),uuid,name,pending.input?(pending.clipping?PeakCondition::InputClip:PeakCondition::InputNear):(pending.clipping?PeakCondition::OutputClip:PeakCondition::OutputNear),pending.maximumDb,pending.durationMilliseconds};RecordPeakEvent(event);}
    }
}

static int CurrentPeakMeterType(){
    config *profile=api.profile_config?api.profile_config():nullptr;return profile&&api.config_get_uint(profile,"Audio","PeakMeterType")!=0?CG_TRUE_PEAK:CG_SAMPLE_PEAK;
}

static void TemporarilyDisableOwnedLimiterForSource(void *source);

static bool AddPeakSource(void *enumerated){
    if(!enumerated||(api.source_output_flags(enumerated)&CG_AUDIO_FLAG)==0)return true;const char *uuidRaw=api.source_uuid(enumerated);QString uuid=QString::fromUtf8(uuidRaw?uuidRaw:"");if(uuid.isEmpty())return true;
    if(std::any_of(peakSources.begin(),peakSources.end(),[&](const auto &item){return item->uuid==uuid;}))return true;
    void *reference=api.source_get_ref(enumerated);if(!reference)return true;if(clipGuardMode.load()==ClipGuardMode::SoundCheck)TemporarilyDisableOwnedLimiterForSource(reference);auto source=std::make_unique<PeakSource>();source->source=reference;source->uuid=uuid;source->name=QString::fromUtf8(api.source_name(reference)?api.source_name(reference):"Audio source");source->config=clipGuardConfig;source->meter=api.volmeter_create(CG_FADER_LOG);
    if(!source->meter){api.source_release(reference);return true;}api.volmeter_set_peak_type(source->meter,CurrentPeakMeterType());api.volmeter_add_callback(source->meter,PeakMeterCallback,source.get());if(!api.volmeter_attach(source->meter,reference)){api.volmeter_remove_callback(source->meter,PeakMeterCallback,source.get());api.volmeter_destroy(source->meter);api.source_release(reference);return true;}source->channels=std::clamp(api.volmeter_channels(source->meter),1,CG_MAX_CHANNELS);peakSources.push_back(std::move(source));return true;
}

static bool CollectPeakSource(void*,void *source){return AddPeakSource(source);}

static void RefreshPeakSources(){
    if(clipGuardMode.load()!=ClipGuardMode::SoundCheck)return;api.enum_sources(CollectPeakSource,nullptr);
}

static void StopPeakMeters(){
    if(peakSourceRefreshTimer){peakSourceRefreshTimer->stop();delete peakSourceRefreshTimer;peakSourceRefreshTimer=nullptr;}
    for(auto &source:peakSources){if(source->meter){api.volmeter_remove_callback(source->meter,PeakMeterCallback,source.get());api.volmeter_detach(source->meter);api.volmeter_destroy(source->meter);source->meter=nullptr;}if(source->source){api.source_release(source->source);source->source=nullptr;}}
    peakSources.clear();
}

static void StartAllPeakSources(){
    StopPeakMeters();api.enum_sources(CollectPeakSource,nullptr);peakSourceRefreshTimer=new QTimer(PluginEventTarget());peakSourceRefreshTimer->setInterval(1000);QObject::connect(peakSourceRefreshTimer,&QTimer::timeout,PluginEventTarget(),[]{RefreshPeakSources();});peakSourceRefreshTimer->start();
}

struct FilterScan {
    bool owned{};
    void *ownedFilter{};
    void *lastFilter{};
    void *enabledUserLimiter{};
    double enabledUserLimiterThreshold{};
};

static void ScanFilter(void*,void *filter,void *parameter){
    auto *scan=static_cast<FilterScan*>(parameter);scan->lastFilter=filter;const char *id=api.source_id(filter);if(!id||strcmp(id,CG_LIMITER_ID)!=0)return;const char *name=api.source_name(filter);obs_data *settings=api.source_settings(filter);bool owned=(name&&strcmp(name,CG_LIMITER_NAME)==0)||(settings&&api.data_get_bool(settings,"clip_guard_managed"));
    if(owned){scan->owned=true;scan->ownedFilter=filter;}else if(api.source_enabled(filter)){scan->enabledUserLimiter=filter;scan->enabledUserLimiterThreshold=settings?api.data_get_double(settings,"threshold"):0.0;}
    if(settings)api.data_release(settings);
}

static FilterScan SourceLimiterStatus(void *source){FilterScan scan;api.enum_filters(source,ScanFilter,&scan);return scan;}

static std::vector<PeakSourceSnapshot> PeakSnapshots(){
    std::vector<PeakSourceSnapshot> snapshots=sessionSafeguards;
    for(const auto &source:peakSources){
        std::lock_guard<std::mutex> lock(source->mutex);
        auto found=std::find_if(snapshots.begin(),snapshots.end(),[&](const PeakSourceSnapshot &snapshot){return snapshot.uuid==source->uuid;});
        if(found==snapshots.end())snapshots.push_back({source->uuid,source->name,source->outputEvents});
        else{found->name=source->name;found->outputEvents=std::max(found->outputEvents,source->outputEvents);}
    }
    std::sort(snapshots.begin(),snapshots.end(),[](const auto &left,const auto &right){return left.name.compare(right.name,Qt::CaseInsensitive)<0;});return snapshots;
}

struct FindSourceContext {QString uuid;void *source{};};
static bool FindSourceCallback(void *parameter,void *source){
    auto *context=static_cast<FindSourceContext*>(parameter);const char *uuid=api.source_uuid(source);if(uuid&&context->uuid==QString::fromUtf8(uuid)){context->source=api.source_get_ref(source);return false;}return true;
}
static void *FindSource(const QString &uuid){FindSourceContext context{uuid};api.enum_sources(FindSourceCallback,&context);return context.source;}

static void TemporarilyDisableOwnedLimiterForSource(void *source){
    if(!source)return;FilterScan scan=SourceLimiterStatus(source);
    if(!scan.ownedFilter||!api.source_enabled(scan.ownedFilter))return;
    api.source_set_enabled(scan.ownedFilter,false);const char *uuid=api.source_uuid(source);
    if(uuid){QString value=QString::fromUtf8(uuid);if(!value.isEmpty()&&!temporarilyDisabledOwnedLimiterSources.contains(value))temporarilyDisabledOwnedLimiterSources.push_back(value);}
}

static bool DisableOwnedLimiterCallback(void*,void *source){
    TemporarilyDisableOwnedLimiterForSource(source);
    return true;
}

static void TemporarilyDisableOwnedLimiters(){
    temporarilyDisabledOwnedLimiterSources.clear();api.enum_sources(DisableOwnedLimiterCallback,nullptr);
}

static void RestoreTemporarilyDisabledOwnedLimiters(){
    QStringList sources=temporarilyDisabledOwnedLimiterSources;temporarilyDisabledOwnedLimiterSources.clear();
    for(const QString &uuid:sources){void *source=FindSource(uuid);if(!source)continue;FilterScan scan=SourceLimiterStatus(source);if(scan.ownedFilter)api.source_set_enabled(scan.ownedFilter,true);api.source_release(source);}
}

static double RequiredLimiterThreshold(void *source){
    double multiplier=std::max(0.000001,static_cast<double>(api.source_get_volume(source)));double gainDb=20.0*std::log10(multiplier);
    return std::clamp(clipGuardConfig.limiterThresholdDb-std::max(0.0,gainDb),-60.0,-0.5);
}

struct LimiterPlan {
    void *source{};
    void *filter{};
    void *createdFilter{};
    double threshold{};
    bool needsOwned{};
};

static void ReleaseLimiterPlans(std::vector<LimiterPlan> &plans){
    for(LimiterPlan &plan:plans){if(plan.createdFilter){api.source_release(plan.createdFilter);plan.createdFilter=nullptr;}if(plan.source){api.source_release(plan.source);plan.source=nullptr;}}
}

static bool ApplyClipGuardSafeguards(const std::vector<PeakSourceSnapshot> &snapshots,QString &error,QStringList &warnings){
    std::vector<LimiterPlan> plans;
    for(const PeakSourceSnapshot &snapshot:snapshots){
        if(snapshot.outputEvents<=0)continue;void *source=FindSource(snapshot.uuid);if(!source){warnings.push_back(QStringLiteral("The audio source \"%1\" is no longer available, so its safeguard was skipped.").arg(snapshot.name));continue;}
        FilterScan scan=SourceLimiterStatus(source);double threshold=RequiredLimiterThreshold(source);bool effectiveUser=scan.enabledUserLimiter&&scan.lastFilter==scan.enabledUserLimiter&&scan.enabledUserLimiterThreshold<=threshold;
        plans.push_back({source,scan.ownedFilter,nullptr,threshold,scan.ownedFilter||!effectiveUser});
    }
    for(LimiterPlan &plan:plans){
        if(!plan.needsOwned||plan.filter)continue;obs_data *settings=api.data_create();api.data_set_double(settings,"threshold",plan.threshold);api.data_set_int(settings,"release_time",clipGuardConfig.limiterReleaseMilliseconds);api.data_set_bool(settings,"clip_guard_managed",true);api.data_set_int(settings,"clip_guard_version",1);
        plan.createdFilter=api.source_create(CG_LIMITER_ID,CG_LIMITER_NAME,settings,nullptr);api.data_release(settings);if(!plan.createdFilter){error=QStringLiteral("OBS could not create a ClipGuard limiter. No ClipGuard limiter changes were accepted.");for(LimiterPlan &rollback:plans)if(rollback.createdFilter)api.source_filter_remove(rollback.source,rollback.createdFilter);ReleaseLimiterPlans(plans);return false;}api.source_filter_add(plan.source,plan.createdFilter);plan.filter=plan.createdFilter;
    }
    constexpr int MOVE_BOTTOM=3;
    for(LimiterPlan &plan:plans){
        if(!plan.needsOwned||!plan.filter)continue;obs_data *settings=api.data_create();api.data_set_double(settings,"threshold",plan.threshold);api.data_set_int(settings,"release_time",clipGuardConfig.limiterReleaseMilliseconds);api.data_set_bool(settings,"clip_guard_managed",true);api.data_set_int(settings,"clip_guard_version",1);api.source_update(plan.filter,settings);api.data_release(settings);api.source_set_enabled(plan.filter,true);api.source_filter_set_order(plan.source,plan.filter,MOVE_BOTTOM);
    }
    ReleaseLimiterPlans(plans);return true;
}

class ClipGuardSettingsDialog final:public QDialog {
public:
    explicit ClipGuardSettingsDialog(QWidget *parent):QDialog(parent){
        setWindowTitle(QStringLiteral("ClipGuard Settings"));setWindowModality(Qt::ApplicationModal);auto *outer=new QVBoxLayout(this);auto *form=new QFormLayout();
        near_=DoubleBox(-20.0,-0.6,clipGuardConfig.nearClippingDb,QStringLiteral("Near-clipping threshold in decibels."));clip_=DoubleBox(-6.0,0.0,clipGuardConfig.clippingDb,QStringLiteral("Clipping threshold in decibels."));
        sustained_=IntegerBox(100,10000,clipGuardConfig.sustainedMilliseconds,QStringLiteral("Near-clipping exposure required before Sound Check records an event."));
        clipExposure_=IntegerBox(20,2000,clipGuardConfig.clippingExposureMilliseconds,QStringLiteral("Clipping exposure required before Sound Check records an event."));
        recovery_=IntegerBox(250,30000,clipGuardConfig.recoveryMilliseconds,QStringLiteral("Safe time required before another event from the same source."));
        cooldown_=IntegerBox(1000,120000,clipGuardConfig.eventCooldownMilliseconds,QStringLiteral("Minimum time between repeated source events."));
        limiter_=DoubleBox(-20.0,-0.5,clipGuardConfig.limiterThresholdDb,QStringLiteral("Threshold for limiters created by ClipGuard."));
        release_=IntegerBox(1,1000,clipGuardConfig.limiterReleaseMilliseconds,QStringLiteral("Release time for limiters created by ClipGuard."));
        retention_=IntegerBox(1,3650,clipGuardConfig.historyRetentionDays,QStringLiteral("Days automatic history files are retained."));
        autoSave_=new QCheckBox(QStringLiteral("Automatically save completed ClipGuard Sound Check history"),this);autoSave_->setChecked(clipGuardConfig.autoSaveHistory);
        form->addRow(QStringLiteral("Near-clipping threshold, dB:"),near_);form->addRow(QStringLiteral("Clipping threshold, dB:"),clip_);form->addRow(QStringLiteral("Sustained exposure, milliseconds:"),sustained_);form->addRow(QStringLiteral("Clipping exposure, milliseconds:"),clipExposure_);form->addRow(QStringLiteral("Recovery time, milliseconds:"),recovery_);form->addRow(QStringLiteral("Event cooldown, milliseconds:"),cooldown_);form->addRow(QStringLiteral("Limiter threshold, dB:"),limiter_);form->addRow(QStringLiteral("Limiter release, milliseconds:"),release_);form->addRow(QStringLiteral("History retention, days:"),retention_);outer->addLayout(form);outer->addWidget(autoSave_);
        auto *buttons=new QDialogButtonBox(this);auto *defaults=buttons->addButton(QStringLiteral("Restore Defaults"),QDialogButtonBox::ResetRole);auto *save=buttons->addButton(QStringLiteral("Save"),QDialogButtonBox::AcceptRole);buttons->addButton(QStringLiteral("Cancel"),QDialogButtonBox::RejectRole);outer->addWidget(buttons);
        connect(defaults,&QPushButton::clicked,this,[this]{SetValues(ClipGuardConfig{});});connect(save,&QPushButton::clicked,this,[this]{Save();});connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);resize(650,540);
    }
private:
    QDoubleSpinBox *DoubleBox(double minimum,double maximum,double value,const QString &description){auto *box=new QDoubleSpinBox(this);box->setRange(minimum,maximum);box->setDecimals(1);box->setSingleStep(0.1);box->setSuffix(QStringLiteral(" dB"));box->setValue(value);box->setAccessibleDescription(description);return box;}
    QSpinBox *IntegerBox(int minimum,int maximum,int value,const QString &description){auto *box=new QSpinBox(this);box->setRange(minimum,maximum);box->setValue(value);box->setAccessibleDescription(description);return box;}
    void SetValues(const ClipGuardConfig &config){near_->setValue(config.nearClippingDb);clip_->setValue(config.clippingDb);sustained_->setValue(config.sustainedMilliseconds);clipExposure_->setValue(config.clippingExposureMilliseconds);recovery_->setValue(config.recoveryMilliseconds);cooldown_->setValue(config.eventCooldownMilliseconds);limiter_->setValue(config.limiterThresholdDb);release_->setValue(config.limiterReleaseMilliseconds);retention_->setValue(config.historyRetentionDays);autoSave_->setChecked(config.autoSaveHistory);}
    void Save(){
        if(near_->value()>=clip_->value()){QMessageBox::warning(this,QStringLiteral("ClipGuard Settings"),QStringLiteral("The near-clipping threshold must be lower than the clipping threshold."));near_->setFocus();return;}
        ClipGuardConfig updated=clipGuardConfig;updated.nearClippingDb=near_->value();updated.clippingDb=clip_->value();updated.sustainedMilliseconds=sustained_->value();updated.clippingExposureMilliseconds=clipExposure_->value();updated.recoveryMilliseconds=recovery_->value();updated.eventCooldownMilliseconds=cooldown_->value();updated.limiterThresholdDb=limiter_->value();updated.limiterReleaseMilliseconds=release_->value();updated.historyRetentionDays=retention_->value();updated.autoSaveHistory=autoSave_->isChecked();QString error;if(!SaveClipGuardConfig(updated,&error)){QMessageBox::critical(this,QStringLiteral("ClipGuard Settings"),error);return;}accept();
    }
    QDoubleSpinBox *near_{},*clip_{},*limiter_{};QSpinBox *sustained_{},*clipExposure_{},*recovery_{},*cooldown_{},*release_{},*retention_{};QCheckBox *autoSave_{};
};

static bool EditClipGuardSettings(QWidget *parent){
    ClipGuardConfig before=clipGuardConfig;ClipGuardSettingsDialog dialog(parent);if(dialog.exec()!=QDialog::Accepted)return false;
    if(clipGuardMode.load()==ClipGuardMode::SoundCheck){DrainPeakEvents();StopPeakWarning();StartAllPeakSources();}
    return ClipGuardJson(before)!=ClipGuardJson(clipGuardConfig);
}

class ClipGuardWindow final:public QDialog {
public:
    ClipGuardWindow():QDialog(nullptr){
        setAttribute(Qt::WA_ShowWithoutActivating);setWindowTitle(CGText(CG_WINDOW_TITLE));setWindowModality(Qt::NonModal);setAttribute(Qt::WA_DeleteOnClose);auto *outer=new QVBoxLayout(this);status_=new QLabel(this);status_->setWordWrap(true);outer->addWidget(status_);history_=new PeakHistoryView(this);history_->escapeRequested=[this]{CancelSoundCheck();};outer->addWidget(history_);
        auto *buttons=new QDialogButtonBox(this);auto *settings=buttons->addButton(CGText(CG_SETTINGS_BUTTON),QDialogButtonBox::ActionRole);auto *save=buttons->addButton(CGText(CG_SAVE_HISTORY_BUTTON),QDialogButtonBox::ActionRole);auto *complete=buttons->addButton(CGText(CG_COMPLETE_BUTTON),QDialogButtonBox::AcceptRole);auto *cancel=buttons->addButton(CGText(CG_CANCEL_BUTTON),QDialogButtonBox::RejectRole);outer->addWidget(buttons);
        connect(settings,&QPushButton::clicked,this,[this]{EditClipGuardSettings(this);});connect(save,&QPushButton::clicked,this,[this]{SaveHistoryAs();});connect(complete,&QPushButton::clicked,this,[this]{CompleteSoundCheck(false);});connect(cancel,&QPushButton::clicked,this,[this]{CancelSoundCheck();});statusTimer_=new QTimer(this);statusTimer_->setInterval(100);connect(statusTimer_,&QTimer::timeout,this,[this]{DrainPeakEvents();UpdatePeakWarning();MaybeSavePeakHistory();if(++statusTicks_>=10){statusTicks_=0;UpdateStatus();}});statusTimer_->start();UpdateStatus();resize(850,600);history_->setFocus(Qt::OtherFocusReason);
    }
    void AppendEvent(const PeakHistoryEvent &event){history_->AppendEvent(event);}
    void CancelSoundCheck(){
        if(transitioning_||NestedDialogOpen())return;transitioning_=true;StopPeakWarning();StopPeakMeters();RestoreTemporarilyDisabledOwnedLimiters();DiscardPeakHistory();clipGuardMode=ClipGuardMode::Idle;allowClose_=true;close();
    }
    void CompleteSoundCheck(bool automatic){
        if(transitioning_)return;
        if(NestedDialogOpen()){
            if(!automatic)return;
            QApplication::activeModalWidget()->close();
        }
        transitioning_=true;DrainPeakEvents();StopPeakWarning();std::vector<PeakSourceSnapshot> snapshots=PeakSnapshots();StopPeakMeters();RestoreTemporarilyDisabledOwnedLimiters();QString error;QStringList warnings;
        if(!ApplyClipGuardSafeguards(snapshots,error,warnings)){
            if(automatic){QString historyError;SavePeakHistory(&historyError);QString warning=historyError.isEmpty()?error:QStringLiteral("%1\n\n%2").arg(error,historyError);clipGuardMode=ClipGuardMode::Idle;allowClose_=true;close();if(QObject *target=PluginEventTarget())QTimer::singleShot(0,target,[warning]{QMessageBox::warning(obsMainWindow,QStringLiteral("ClipGuard"),warning);});return;}
            TemporarilyDisableOwnedLimiters();StartAllPeakSources();transitioning_=false;QMessageBox::critical(this,QStringLiteral("ClipGuard"),error);return;
        }
        QString historyError;SavePeakHistory(&historyError);if(api.frontend_save)api.frontend_save();clipGuardMode=ClipGuardMode::Idle;allowClose_=true;close();
        if(!historyError.isEmpty())warnings.push_back(historyError);
        if(!warnings.isEmpty()){QString warning=QStringLiteral("Sound Check completed with the following warnings:\n\n%1").arg(warnings.join(QLatin1Char('\n')));if(automatic){if(QObject *target=PluginEventTarget())QTimer::singleShot(0,target,[warning]{QMessageBox::warning(obsMainWindow,QStringLiteral("ClipGuard"),warning);});}else QMessageBox::warning(obsMainWindow,QStringLiteral("ClipGuard"),warning);}
    }
    void reject() override{CancelSoundCheck();}
protected:
    void closeEvent(QCloseEvent *event) override{if(allowClose_){event->accept();return;}event->ignore();CancelSoundCheck();}
private:
    bool NestedDialogOpen() const{QWidget *modal=QApplication::activeModalWidget();return modal&&modal!=this;}
    void UpdateStatus(){int exercised=0;for(const auto &source:peakSources){std::lock_guard<std::mutex> lock(source->mutex);if(source->exercised)++exercised;}QString text=QStringLiteral("ClipGuard Sound Check active. Monitoring %1 audio sources; %2 exercised; %3 events recorded.").arg(peakSources.size()).arg(exercised).arg(peakHistory.size());if(peakHistoryDropped>0)text+=QStringLiteral(" %1 additional events omitted after the history safety limit.").arg(peakHistoryDropped);text+=QStringLiteral(" Press Ctrl+Shift+P or activate Complete Sound Check to accept the results and safeguards.");status_->setText(text);}
    void SaveHistoryAs(){QString path=QFileDialog::getSaveFileName(this,QStringLiteral("Save ClipGuard History"),QDir::home().filePath(QStringLiteral("ClipGuard Sound Check.txt")),QStringLiteral("Text files (*.txt);;All files (*.*)"));if(path.isEmpty())return;if(!WriteBytesAtomically(path,PeakHistoryText().toUtf8()))QMessageBox::critical(this,QStringLiteral("ClipGuard"),QStringLiteral("The history file could not be saved."));}
    QLabel *status_{};PeakHistoryView *history_{};QTimer *statusTimer_{};int statusTicks_{};bool allowClose_{},transitioning_{};
};

static ClipGuardWindow *ClipGuardWindowPointer(){return static_cast<ClipGuardWindow*>(clipGuardWindow.data());}

static void RecordPeakEvent(const PeakHistoryEvent &event){
    if(clipGuardMode.load()!=ClipGuardMode::SoundCheck)return;
    if(event.condition==PeakCondition::OutputNear||event.condition==PeakCondition::OutputClip){
        auto found=std::find_if(sessionSafeguards.begin(),sessionSafeguards.end(),[&](const PeakSourceSnapshot &snapshot){return snapshot.uuid==event.uuid;});
        if(found==sessionSafeguards.end())sessionSafeguards.push_back({event.uuid,event.source,1});
        else{found->name=event.source;++found->outputEvents;}
    }
    peakHistoryDirty=true;
    if(peakHistory.size()>=CG_MAX_HISTORY_EVENTS){++peakHistoryDropped;return;}
    peakHistory.push_back(event);if(auto *window=ClipGuardWindowPointer())window->AppendEvent(event);
}

static void StartSoundCheck(){
    if((api.streaming_active&&api.streaming_active())||(api.recording_active&&api.recording_active())){QMessageBox::information(obsMainWindow,QStringLiteral("ClipGuard"),QStringLiteral("ClipGuard Sound Check is unavailable while streaming or recording."));return;}
    QString error;if(!LoadClipGuardConfig(&error)){QMessageBox::critical(obsMainWindow,QStringLiteral("ClipGuard Configuration"),QStringLiteral("%1\n\nClipGuard will use safe defaults until the file is corrected in Settings.").arg(error));clipGuardConfig=ClipGuardConfig{};}
    QString historyError;BeginPeakHistory(&historyError);clipGuardMode=ClipGuardMode::SoundCheck;TemporarilyDisableOwnedLimiters();StartAllPeakSources();auto *window=new ClipGuardWindow();clipGuardWindow=window;QObject::connect(window,&QObject::destroyed,[]{clipGuardWindow=nullptr;});window->showMinimized();if(!historyError.isEmpty())QMessageBox::warning(obsMainWindow,QStringLiteral("ClipGuard"),historyError);
}

} // namespace

static std::string ClipGuardCommandLabel(){QByteArray text=CGText(CG_COMMAND).toUtf8();return {text.constData(),static_cast<size_t>(text.size())};}
static std::string CancelClipGuardCommandLabel(){QByteArray text=CGText(CG_CANCEL_COMMAND).toUtf8();return {text.constData(),static_cast<size_t>(text.size())};}

static void ClipGuardHotkey(void*,hotkey_id,obs_hotkey*,bool pressed){
    if(!pressed||!PluginEventTarget())return;QMetaObject::invokeMethod(PluginEventTarget(),[]{
        if(clipGuardMode.load()==ClipGuardMode::Idle){StartSoundCheck();return;}
        if(auto *window=ClipGuardWindowPointer())window->CompleteSoundCheck(false);
    },Qt::QueuedConnection);
}

static void CancelClipGuardHotkey(void*,hotkey_id,obs_hotkey*,bool pressed){
    if(!pressed||!PluginEventTarget())return;QMetaObject::invokeMethod(PluginEventTarget(),[]{if(clipGuardMode.load()==ClipGuardMode::SoundCheck)if(auto *window=ClipGuardWindowPointer())window->CancelSoundCheck();},Qt::QueuedConnection);
}

static void ShutdownClipGuard(){
    StopPeakWarning();StopPeakMeters();RestoreTemporarilyDisabledOwnedLimiters();if(clipGuardMode.load()==ClipGuardMode::SoundCheck)DiscardPeakHistory();clipGuardMode=ClipGuardMode::Idle;if(clipGuardWindow){delete clipGuardWindow.data();clipGuardWindow=nullptr;}
}

static void HandleClipGuardFrontendEvent(int event){
    constexpr int STREAMING_STARTING=0,RECORDING_STARTING=4,EXIT=17;if(clipGuardMode.load()!=ClipGuardMode::SoundCheck)return;
    if(event==STREAMING_STARTING||event==RECORDING_STARTING){if(auto *window=ClipGuardWindowPointer())window->CompleteSoundCheck(true);}
    else if(event==EXIT)ShutdownClipGuard();
}
