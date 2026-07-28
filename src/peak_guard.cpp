// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Tiflo.Info

namespace {

constexpr uint32_t PG_AUDIO_FLAG=1u<<1;
constexpr int PG_MAX_CHANNELS=8;
constexpr int PG_FADER_LOG=2;
constexpr int PG_SAMPLE_PEAK=0;
constexpr int PG_TRUE_PEAK=1;
constexpr const char *PG_LIMITER_ID="limiter_filter";
constexpr const char *PG_LIMITER_NAME="Accessible OBS Studio Peak Guard";

enum class PeakGuardMode {Idle,SoundCheck,ProtectionSetup,Emergency};
enum class PeakCondition {OutputNear,OutputClip,InputNear,InputClip};
enum class PeakWarningTone {None,Input,Output};

struct PeakGuardConfig {
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
    QStringList previousSources;
};

struct PeakHistoryEvent {
    QDateTime time;
    QString uuid;
    QString source;
    PeakCondition condition{PeakCondition::OutputNear};
    double maximumDb{};
    int durationMilliseconds{};
    bool emergency{};
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

struct PeakSource {
    void *source{};
    obs_volmeter *meter{};
    QString uuid;
    QString name;
    int channels{2};
    PeakGuardConfig config;
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
};

struct PeakSourceSnapshot {
    QString uuid;
    QString name;
    double maximumInput{-INFINITY};
    double maximumOutput{-INFINITY};
    int inputEvents{};
    int outputEvents{};
    bool exercised{};
    bool existingLimiter{};
    bool existingLimiterEnabled{};
};

static std::atomic<PeakGuardMode> peakGuardMode{PeakGuardMode::Idle};
static PeakGuardConfig peakGuardConfig;
static bool peakGuardConfigLoaded{};
static QString peakGuardConfigError;
static std::vector<std::unique_ptr<PeakSource>> peakSources;
static std::vector<PeakHistoryEvent> peakHistory;
static QString peakHistoryBasePath;
static QDateTime peakHistoryStarted;
static QPointer<QDialog> soundCheckWindow;
static QPointer<QDialog> protectionSetupWindow;
static QTimer *peakSourceRefreshTimer{};
static QStringList temporarilyDisabledOwnedLimiterSources;
static QByteArray peakInputWarningSound;
static QByteArray peakOutputWarningSound;
static PeakWarningTone peakWarningTone{PeakWarningTone::None};
static QString peakWarningTargetUuid;
static PeakWarningTone peakWarningTargetTone{PeakWarningTone::None};

static QString PeakGuardConfigDirectory(){
    wchar_t appData[32768]{};
    DWORD count=GetEnvironmentVariableW(L"APPDATA",appData,static_cast<DWORD>(std::size(appData)));
    QString root=count>0&&static_cast<size_t>(count)<std::size(appData)?QString::fromWCharArray(appData):QDir::homePath();
    return QDir(root).filePath(QStringLiteral("obs-studio/plugin_config/accessible-obs-studio"));
}

static QString PeakGuardConfigPath(){return QDir(PeakGuardConfigDirectory()).filePath(QStringLiteral("peak-guard.json"));}
static QString PeakGuardSchemaPath(){return QDir(PeakGuardConfigDirectory()).filePath(QStringLiteral("peak-guard.schema.json"));}
static QString PeakGuardDefaultsPath(){return QDir(PeakGuardConfigDirectory()).filePath(QStringLiteral("peak-guard.defaults.json"));}
static QString PeakGuardLogsDirectory(){return QDir(PeakGuardConfigDirectory()).filePath(QStringLiteral("peak-guard-history"));}

static QJsonObject PeakGuardJson(const PeakGuardConfig &config,bool includeSources=true){
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
    QJsonObject root{{QStringLiteral("$schema"),QStringLiteral("peak-guard.schema.json")},
                     {QStringLiteral("configurationVersion"),config.version},
                     {QStringLiteral("analysis"),analysis},
                     {QStringLiteral("limiter"),limiter},
                     {QStringLiteral("history"),history}};
    if(includeSources){
        QJsonArray previous;for(const QString &uuid:config.previousSources)previous.append(uuid);
        root.insert(QStringLiteral("previousEmergencySources"),previous);
    }
    return root;
}

static const char *PeakGuardSchemaText(){
    return R"JSON({
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "title": "Accessible OBS Studio Peak Guard configuration",
  "description": "User-editable settings for Sound Check and Emergency Monitoring. Times are milliseconds and levels are decibels relative to full scale.",
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
        "thresholdDb": {"type": "number", "minimum": -20, "maximum": -0.5, "description": "Threshold used for Peak Guard-owned OBS limiter filters."},
        "releaseMilliseconds": {"type": "integer", "minimum": 1, "maximum": 1000, "description": "Release time used for Peak Guard-owned OBS limiter filters."}
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
    },
    "previousEmergencySources": {
      "type": "array",
      "description": "Source UUIDs retained from the last accepted Emergency Monitoring selection.",
      "items": {"type": "string"},
      "uniqueItems": true
    }
  }
})JSON";
}

static bool WriteBytesAtomically(const QString &path,const QByteArray &bytes){
    QSaveFile file(path);if(!file.open(QIODevice::WriteOnly))return false;
    if(file.write(bytes)!=bytes.size()){file.cancelWriting();return false;}
    return file.commit();
}

static void EnsurePeakGuardSupportFiles(){
    QDir().mkpath(PeakGuardConfigDirectory());
    if(!QFile::exists(PeakGuardSchemaPath()))WriteBytesAtomically(PeakGuardSchemaPath(),QByteArray(PeakGuardSchemaText()));
    if(!QFile::exists(PeakGuardDefaultsPath()))WriteBytesAtomically(PeakGuardDefaultsPath(),QJsonDocument(PeakGuardJson(PeakGuardConfig{},false)).toJson(QJsonDocument::Indented));
}

static bool SavePeakGuardConfig(const PeakGuardConfig &config,QString *error=nullptr){
    EnsurePeakGuardSupportFiles();
    if(!WriteBytesAtomically(PeakGuardConfigPath(),QJsonDocument(PeakGuardJson(config)).toJson(QJsonDocument::Indented))){
        if(error)*error=QStringLiteral("The Peak Guard configuration file could not be saved.");
        return false;
    }
    peakGuardConfig=config;peakGuardConfigLoaded=true;peakGuardConfigError.clear();return true;
}

static bool ReadNumber(const QJsonObject &object,const char *name,double minimum,double maximum,double &value,QString &error){
    QJsonValue raw=object.value(QString::fromLatin1(name));if(!raw.isDouble()){error=QStringLiteral("%1 must be a number.").arg(QString::fromLatin1(name));return false;}
    double candidate=raw.toDouble();if(candidate<minimum||candidate>maximum){error=QStringLiteral("%1 must be between %2 and %3.").arg(QString::fromLatin1(name)).arg(minimum).arg(maximum);return false;}value=candidate;return true;
}

static bool ReadInteger(const QJsonObject &object,const char *name,int minimum,int maximum,int &value,QString &error){
    double number{};if(!ReadNumber(object,name,minimum,maximum,number,error))return false;
    if(std::floor(number)!=number){error=QStringLiteral("%1 must be a whole number.").arg(QString::fromLatin1(name));return false;}value=static_cast<int>(number);return true;
}

static bool LoadPeakGuardConfig(QString *reportedError=nullptr){
    EnsurePeakGuardSupportFiles();PeakGuardConfig loaded;
    QFile file(PeakGuardConfigPath());
    if(!file.exists()){QString error;if(!SavePeakGuardConfig(loaded,&error)){peakGuardConfigError=error;if(reportedError)*reportedError=error;return false;}return true;}
    if(!file.open(QIODevice::ReadOnly)){peakGuardConfigError=QStringLiteral("The Peak Guard configuration file could not be opened.");if(reportedError)*reportedError=peakGuardConfigError;return false;}
    QJsonParseError parse;QJsonDocument document=QJsonDocument::fromJson(file.readAll(),&parse);
    if(parse.error!=QJsonParseError::NoError||!document.isObject()){peakGuardConfigError=QStringLiteral("Invalid Peak Guard JSON at offset %1: %2").arg(parse.offset).arg(parse.errorString());if(reportedError)*reportedError=peakGuardConfigError;return false;}
    QJsonObject root=document.object();QString error;
    if(root.value(QStringLiteral("configurationVersion")).toInt()!=1)error=QStringLiteral("configurationVersion must be 1.");
    QJsonObject analysis=root.value(QStringLiteral("analysis")).toObject(),limiter=root.value(QStringLiteral("limiter")).toObject(),history=root.value(QStringLiteral("history")).toObject();
    if(error.isEmpty()&&analysis.isEmpty())error=QStringLiteral("analysis must be an object.");
    if(error.isEmpty()&&!ReadNumber(analysis,"nearClippingDb",-20.0,-0.6,loaded.nearClippingDb,error)){}
    else if(error.isEmpty()&&!ReadNumber(analysis,"clippingDb",-6.0,0.0,loaded.clippingDb,error)){}
    else if(error.isEmpty()&&!ReadInteger(analysis,"sustainedMilliseconds",100,10000,loaded.sustainedMilliseconds,error)){}
    else if(error.isEmpty()&&!ReadInteger(analysis,"clippingExposureMilliseconds",20,2000,loaded.clippingExposureMilliseconds,error)){}
    else if(error.isEmpty()&&!ReadInteger(analysis,"recoveryMilliseconds",250,30000,loaded.recoveryMilliseconds,error)){}
    else if(error.isEmpty()&&!ReadInteger(analysis,"eventCooldownMilliseconds",1000,120000,loaded.eventCooldownMilliseconds,error)){}
    if(error.isEmpty()&&loaded.nearClippingDb>=loaded.clippingDb)error=QStringLiteral("nearClippingDb must be lower than clippingDb.");
    if(error.isEmpty()&&limiter.isEmpty())error=QStringLiteral("limiter must be an object.");
    if(error.isEmpty()&&!ReadNumber(limiter,"thresholdDb",-20.0,-0.5,loaded.limiterThresholdDb,error)){}
    else if(error.isEmpty()&&!ReadInteger(limiter,"releaseMilliseconds",1,1000,loaded.limiterReleaseMilliseconds,error)){}
    if(error.isEmpty()&&history.isEmpty())error=QStringLiteral("history must be an object.");
    if(error.isEmpty()){
        QJsonValue autoSave=history.value(QStringLiteral("autoSave"));if(!autoSave.isBool())error=QStringLiteral("autoSave must be true or false.");else loaded.autoSaveHistory=autoSave.toBool();
    }
    if(error.isEmpty()&&!ReadInteger(history,"retentionDays",1,3650,loaded.historyRetentionDays,error)){}
    QJsonArray previous=root.value(QStringLiteral("previousEmergencySources")).toArray();for(const QJsonValue &value:previous)if(value.isString()&&!loaded.previousSources.contains(value.toString()))loaded.previousSources.push_back(value.toString());
    if(!error.isEmpty()){peakGuardConfigError=error;if(reportedError)*reportedError=error;return false;}
    peakGuardConfig=loaded;peakGuardConfigLoaded=true;peakGuardConfigError.clear();return true;
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
            {QStringLiteral("mode"),event.emergency?QStringLiteral("emergency"):QStringLiteral("soundCheck")}};
}

static QString PeakHistoryText(){
    QString result=QStringLiteral("Accessible OBS Studio Peak Guard\nStarted: %1\n\n").arg(peakHistoryStarted.toString(Qt::ISODateWithMs));
    for(const PeakHistoryEvent &event:peakHistory)result+=PeakEventLine(event)+QLatin1Char('\n');
    return result;
}

static void SavePeakHistory(){
    if(peakHistoryBasePath.isEmpty()||!peakGuardConfig.autoSaveHistory)return;
    QJsonArray events;for(const PeakHistoryEvent &event:peakHistory)events.append(PeakEventJson(event));
    QJsonObject root{{QStringLiteral("formatVersion"),1},{QStringLiteral("started"),peakHistoryStarted.toString(Qt::ISODateWithMs)},{QStringLiteral("events"),events}};
    WriteBytesAtomically(peakHistoryBasePath+QStringLiteral(".json"),QJsonDocument(root).toJson(QJsonDocument::Indented));
    WriteBytesAtomically(peakHistoryBasePath+QStringLiteral(".txt"),PeakHistoryText().toUtf8());
}

static void RemoveExpiredPeakHistory(){
    QDir directory(PeakGuardLogsDirectory());if(!directory.exists())return;
    QDateTime cutoff=QDateTime::currentDateTime().addDays(-peakGuardConfig.historyRetentionDays);
    for(const QFileInfo &file:directory.entryInfoList({QStringLiteral("*.txt"),QStringLiteral("*.json")},QDir::Files))if(file.lastModified()<cutoff)QFile::remove(file.absoluteFilePath());
}

static void BeginPeakHistory(){
    peakHistory.clear();peakHistoryStarted=QDateTime::currentDateTime();QDir().mkpath(PeakGuardLogsDirectory());RemoveExpiredPeakHistory();
    peakHistoryBasePath=QDir(PeakGuardLogsDirectory()).filePath(QStringLiteral("peak-guard-%1").arg(peakHistoryStarted.toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"))));
    SavePeakHistory();
}

static void DiscardPeakHistory(){
    if(!peakHistoryBasePath.isEmpty()){QFile::remove(peakHistoryBasePath+QStringLiteral(".json"));QFile::remove(peakHistoryBasePath+QStringLiteral(".txt"));}
    peakHistory.clear();peakHistoryBasePath.clear();peakHistoryStarted={};
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
    if(peakGuardMode.load()!=PeakGuardMode::SoundCheck){SetPeakWarningSound(PeakWarningTone::None);peakWarningTargetUuid.clear();peakWarningTargetTone=PeakWarningTone::None;return;}
    std::vector<ActivePeakWarning> warnings=ActivePeakWarnings();PeakWarningTone tone=std::any_of(warnings.begin(),warnings.end(),[](const ActivePeakWarning &warning){return warning.tone==PeakWarningTone::Input;})?PeakWarningTone::Input:(warnings.empty()?PeakWarningTone::None:PeakWarningTone::Output);SetPeakWarningSound(tone);
    if(warnings.empty()){peakWarningTargetUuid.clear();peakWarningTargetTone=PeakWarningTone::None;return;}
    auto current=std::find_if(warnings.begin(),warnings.end(),[&](const ActivePeakWarning &warning){return warning.uuid==peakWarningTargetUuid&&warning.tone==peakWarningTargetTone&&warning.tone==tone;});if(current!=warnings.end())return;
    const ActivePeakWarning *mostSevere=nullptr;for(const ActivePeakWarning &warning:warnings){if(warning.tone!=tone)continue;if(!mostSevere||(warning.clipping&&!mostSevere->clipping)||(warning.clipping==mostSevere->clipping&&warning.db>mostSevere->db))mostSevere=&warning;}
    if(!mostSevere)return;peakWarningTargetUuid=mostSevere->uuid;peakWarningTargetTone=mostSevere->tone;OpenVolumeConsoleForSource(peakWarningTargetUuid,true);
}

static void StopPeakWarning(){
    SetPeakWarningSound(PeakWarningTone::None);peakWarningTargetUuid.clear();peakWarningTargetTone=PeakWarningTone::None;
}

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
<title>Peak Guard Sound Check history</title>
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
  <h1>Peak Guard Sound Check history</h1>
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

class SoundCheckDialog;
static SoundCheckDialog *SoundCheckDialogPointer();

static void RecordPeakEvent(const PeakHistoryEvent &event);

static void QueuePeakEvent(PeakSource *source,bool input,bool clipping,double maximum,int duration){
    if(!obsMainWindow||shuttingDown)return;QString uuid=source->uuid,name=source->name;PeakGuardMode mode=peakGuardMode.load();
    PeakHistoryEvent event{QDateTime::currentDateTime(),uuid,name,input?(clipping?PeakCondition::InputClip:PeakCondition::InputNear):(clipping?PeakCondition::OutputClip:PeakCondition::OutputNear),maximum,duration,mode==PeakGuardMode::Emergency};
    QMetaObject::invokeMethod(obsMainWindow,[event]{RecordPeakEvent(event);},Qt::QueuedConnection);
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
    QueuePeakEvent(source,input,clipped,tracker.maximumDb,duration);
}

static void PeakMeterCallback(void *parameter,const float*,const float *peak,const float *inputPeak){
    auto *source=static_cast<PeakSource*>(parameter);if(!source||!peak||!inputPeak)return;PeakGuardMode mode=peakGuardMode.load();if(mode!=PeakGuardMode::SoundCheck&&mode!=PeakGuardMode::Emergency)return;
    double output=-INFINITY,input=-INFINITY;for(int channel=0;channel<std::clamp(source->channels,1,PG_MAX_CHANNELS);++channel){if(std::isfinite(peak[channel]))output=std::max(output,static_cast<double>(peak[channel]));if(std::isfinite(inputPeak[channel]))input=std::max(input,static_cast<double>(inputPeak[channel]));}
    if(!std::isfinite(output)&&!std::isfinite(input))return;auto now=std::chrono::steady_clock::now();std::lock_guard<std::mutex> lock(source->mutex);source->exercised=true;source->maximumOutput=std::max(source->maximumOutput,output);source->maximumInput=std::max(source->maximumInput,input);source->warningInputDb=input;source->warningOutputDb=output;source->warningUpdated=now;
    ProcessPeakLevel(source,source->input,input,true,now);ProcessPeakLevel(source,source->output,output,false,now);
}

static int CurrentPeakMeterType(){
    config *profile=api.profile_config?api.profile_config():nullptr;return profile&&api.config_get_uint(profile,"Audio","PeakMeterType")!=0?PG_TRUE_PEAK:PG_SAMPLE_PEAK;
}

static bool AddPeakSource(void *enumerated){
    if(!enumerated||(api.source_output_flags(enumerated)&PG_AUDIO_FLAG)==0)return true;const char *uuidRaw=api.source_uuid(enumerated);QString uuid=QString::fromUtf8(uuidRaw?uuidRaw:"");if(uuid.isEmpty())return true;
    if(std::any_of(peakSources.begin(),peakSources.end(),[&](const auto &item){return item->uuid==uuid;}))return true;
    void *reference=api.source_get_ref(enumerated);if(!reference)return true;auto source=std::make_unique<PeakSource>();source->source=reference;source->uuid=uuid;source->name=QString::fromUtf8(api.source_name(reference)?api.source_name(reference):"Audio source");source->config=peakGuardConfig;source->meter=api.volmeter_create(PG_FADER_LOG);
    if(!source->meter){api.source_release(reference);return true;}api.volmeter_set_peak_type(source->meter,CurrentPeakMeterType());api.volmeter_add_callback(source->meter,PeakMeterCallback,source.get());if(!api.volmeter_attach(source->meter,reference)){api.volmeter_remove_callback(source->meter,PeakMeterCallback,source.get());api.volmeter_destroy(source->meter);api.source_release(reference);return true;}source->channels=std::clamp(api.volmeter_channels(source->meter),1,PG_MAX_CHANNELS);peakSources.push_back(std::move(source));return true;
}

static bool CollectPeakSource(void*,void *source){return AddPeakSource(source);}

static void RefreshPeakSources(){
    if(peakGuardMode.load()!=PeakGuardMode::SoundCheck)return;api.enum_sources(CollectPeakSource,nullptr);
}

static void StopPeakMeters(){
    if(peakSourceRefreshTimer){peakSourceRefreshTimer->stop();delete peakSourceRefreshTimer;peakSourceRefreshTimer=nullptr;}
    for(auto &source:peakSources){if(source->meter){api.volmeter_remove_callback(source->meter,PeakMeterCallback,source.get());api.volmeter_detach(source->meter);api.volmeter_destroy(source->meter);source->meter=nullptr;}if(source->source){api.source_release(source->source);source->source=nullptr;}}
    peakSources.clear();
}

static void StartAllPeakSources(){
    StopPeakMeters();api.enum_sources(CollectPeakSource,nullptr);peakSourceRefreshTimer=new QTimer(obsMainWindow);peakSourceRefreshTimer->setInterval(1000);QObject::connect(peakSourceRefreshTimer,&QTimer::timeout,[]{RefreshPeakSources();});peakSourceRefreshTimer->start();
}

struct FilterScan {
    bool limiter{};
    bool enabled{};
    bool enabledUserLimiter{};
    bool owned{};
    void *ownedFilter{};
};

static void ScanFilter(void*,void *filter,void *parameter){
    auto *scan=static_cast<FilterScan*>(parameter);const char *id=api.source_id(filter);if(!id||strcmp(id,PG_LIMITER_ID)!=0)return;scan->limiter=true;bool enabled=api.source_enabled(filter);scan->enabled|=enabled;const char *name=api.source_name(filter);obs_data *settings=api.source_settings(filter);bool owned=(name&&strcmp(name,PG_LIMITER_NAME)==0)||(settings&&api.data_get_bool(settings,"peak_guard_managed"));if(settings)api.data_release(settings);if(owned){scan->owned=true;scan->ownedFilter=filter;}else scan->enabledUserLimiter|=enabled;
}

static FilterScan SourceLimiterStatus(void *source){FilterScan scan;api.enum_filters(source,ScanFilter,&scan);return scan;}

static std::vector<PeakSourceSnapshot> PeakSnapshots(){
    std::vector<PeakSourceSnapshot> snapshots;snapshots.reserve(peakSources.size());
    for(const auto &source:peakSources){std::lock_guard<std::mutex> lock(source->mutex);FilterScan limiter=SourceLimiterStatus(source->source);bool enabled=limiter.enabled||temporarilyDisabledOwnedLimiterSources.contains(source->uuid);snapshots.push_back({source->uuid,source->name,source->maximumInput,source->maximumOutput,source->inputEvents,source->outputEvents,source->exercised,limiter.limiter,enabled});}
    std::sort(snapshots.begin(),snapshots.end(),[](const auto &left,const auto &right){return left.name.compare(right.name,Qt::CaseInsensitive)<0;});return snapshots;
}

struct FindSourceContext {QString uuid;void *source{};};
static bool FindSourceCallback(void *parameter,void *source){
    auto *context=static_cast<FindSourceContext*>(parameter);const char *uuid=api.source_uuid(source);if(uuid&&context->uuid==QString::fromUtf8(uuid)){context->source=api.source_get_ref(source);return false;}return true;
}
static void *FindSource(const QString &uuid){FindSourceContext context{uuid};api.enum_sources(FindSourceCallback,&context);return context.source;}

static bool DisableOwnedLimiterCallback(void*,void *source){
    if(!source)return true;FilterScan scan=SourceLimiterStatus(source);
    if(!scan.ownedFilter||!api.source_enabled(scan.ownedFilter))return true;
    api.source_set_enabled(scan.ownedFilter,false);const char *uuid=api.source_uuid(source);
    if(uuid){QString value=QString::fromUtf8(uuid);if(!value.isEmpty()&&!temporarilyDisabledOwnedLimiterSources.contains(value))temporarilyDisabledOwnedLimiterSources.push_back(value);}
    return true;
}

static void TemporarilyDisableOwnedLimiters(){
    temporarilyDisabledOwnedLimiterSources.clear();api.enum_sources(DisableOwnedLimiterCallback,nullptr);
}

static void RestoreTemporarilyDisabledOwnedLimiters(){
    QStringList sources=temporarilyDisabledOwnedLimiterSources;temporarilyDisabledOwnedLimiterSources.clear();
    for(const QString &uuid:sources){void *source=FindSource(uuid);if(!source)continue;FilterScan scan=SourceLimiterStatus(source);if(scan.ownedFilter)api.source_set_enabled(scan.ownedFilter,true);api.source_release(source);}
}

static bool EnsurePeakGuardLimiter(const QString &uuid,QString &error,bool forceOwned=false){
    void *source=FindSource(uuid);if(!source){error=QStringLiteral("An audio source selected for protection is no longer available.");return false;}FilterScan scan=SourceLimiterStatus(source);
    if(scan.enabledUserLimiter&&!forceOwned){api.source_release(source);return true;}
    obs_data *settings=api.data_create();api.data_set_double(settings,"threshold",peakGuardConfig.limiterThresholdDb);api.data_set_int(settings,"release_time",peakGuardConfig.limiterReleaseMilliseconds);api.data_set_bool(settings,"peak_guard_managed",true);api.data_set_int(settings,"peak_guard_version",1);
    if(scan.ownedFilter){api.source_update(scan.ownedFilter,settings);api.source_set_enabled(scan.ownedFilter,true);api.data_release(settings);api.source_release(source);return true;}
    void *filter=api.source_create(PG_LIMITER_ID,PG_LIMITER_NAME,settings,nullptr);api.data_release(settings);if(!filter){api.source_release(source);error=QStringLiteral("OBS could not create the Peak Guard limiter filter.");return false;}api.source_filter_add(source,filter);api.source_release(filter);api.source_release(source);return true;
}

class PeakGuardSettingsDialog final:public QDialog {
public:
    explicit PeakGuardSettingsDialog(QWidget *parent):QDialog(parent){
        setWindowTitle(QStringLiteral("Peak Guard Settings"));setWindowModality(Qt::ApplicationModal);auto *outer=new QVBoxLayout(this);auto *form=new QFormLayout();
        near_=DoubleBox(-20.0,-0.6,peakGuardConfig.nearClippingDb,QStringLiteral("Near-clipping threshold in decibels."));clip_=DoubleBox(-6.0,0.0,peakGuardConfig.clippingDb,QStringLiteral("Clipping threshold in decibels."));
        sustained_=IntegerBox(100,10000,peakGuardConfig.sustainedMilliseconds,QStringLiteral("Near-clipping exposure required before Sound Check records an event."));
        clipExposure_=IntegerBox(20,2000,peakGuardConfig.clippingExposureMilliseconds,QStringLiteral("Clipping exposure required before Sound Check records an event."));
        recovery_=IntegerBox(250,30000,peakGuardConfig.recoveryMilliseconds,QStringLiteral("Safe time required before another event from the same source."));
        cooldown_=IntegerBox(1000,120000,peakGuardConfig.eventCooldownMilliseconds,QStringLiteral("Minimum time between repeated source events."));
        limiter_=DoubleBox(-20.0,-0.5,peakGuardConfig.limiterThresholdDb,QStringLiteral("Threshold for limiters created by Peak Guard."));
        release_=IntegerBox(1,1000,peakGuardConfig.limiterReleaseMilliseconds,QStringLiteral("Release time for limiters created by Peak Guard."));
        retention_=IntegerBox(1,3650,peakGuardConfig.historyRetentionDays,QStringLiteral("Days automatic history files are retained."));
        autoSave_=new QCheckBox(QStringLiteral("Automatically save Sound Check and Emergency Monitoring history"),this);autoSave_->setChecked(peakGuardConfig.autoSaveHistory);
        form->addRow(QStringLiteral("Near-clipping threshold, dB:"),near_);form->addRow(QStringLiteral("Clipping threshold, dB:"),clip_);form->addRow(QStringLiteral("Sustained exposure, milliseconds:"),sustained_);form->addRow(QStringLiteral("Clipping exposure, milliseconds:"),clipExposure_);form->addRow(QStringLiteral("Recovery time, milliseconds:"),recovery_);form->addRow(QStringLiteral("Event cooldown, milliseconds:"),cooldown_);form->addRow(QStringLiteral("Limiter threshold, dB:"),limiter_);form->addRow(QStringLiteral("Limiter release, milliseconds:"),release_);form->addRow(QStringLiteral("History retention, days:"),retention_);outer->addLayout(form);outer->addWidget(autoSave_);
        auto *buttons=new QDialogButtonBox(this);auto *defaults=buttons->addButton(QStringLiteral("Restore Defaults"),QDialogButtonBox::ResetRole);auto *save=buttons->addButton(QStringLiteral("Save"),QDialogButtonBox::AcceptRole);buttons->addButton(QStringLiteral("Cancel"),QDialogButtonBox::RejectRole);outer->addWidget(buttons);
        connect(defaults,&QPushButton::clicked,this,[this]{SetValues(PeakGuardConfig{});});connect(save,&QPushButton::clicked,this,[this]{Save();});connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);resize(650,540);
    }
private:
    QDoubleSpinBox *DoubleBox(double minimum,double maximum,double value,const QString &description){auto *box=new QDoubleSpinBox(this);box->setRange(minimum,maximum);box->setDecimals(1);box->setSingleStep(0.1);box->setSuffix(QStringLiteral(" dB"));box->setValue(value);box->setAccessibleDescription(description);return box;}
    QSpinBox *IntegerBox(int minimum,int maximum,int value,const QString &description){auto *box=new QSpinBox(this);box->setRange(minimum,maximum);box->setValue(value);box->setAccessibleDescription(description);return box;}
    void SetValues(const PeakGuardConfig &config){near_->setValue(config.nearClippingDb);clip_->setValue(config.clippingDb);sustained_->setValue(config.sustainedMilliseconds);clipExposure_->setValue(config.clippingExposureMilliseconds);recovery_->setValue(config.recoveryMilliseconds);cooldown_->setValue(config.eventCooldownMilliseconds);limiter_->setValue(config.limiterThresholdDb);release_->setValue(config.limiterReleaseMilliseconds);retention_->setValue(config.historyRetentionDays);autoSave_->setChecked(config.autoSaveHistory);}
    void Save(){
        if(near_->value()>=clip_->value()){QMessageBox::warning(this,QStringLiteral("Peak Guard Settings"),QStringLiteral("The near-clipping threshold must be lower than the clipping threshold."));near_->setFocus();return;}
        PeakGuardConfig updated=peakGuardConfig;updated.nearClippingDb=near_->value();updated.clippingDb=clip_->value();updated.sustainedMilliseconds=sustained_->value();updated.clippingExposureMilliseconds=clipExposure_->value();updated.recoveryMilliseconds=recovery_->value();updated.eventCooldownMilliseconds=cooldown_->value();updated.limiterThresholdDb=limiter_->value();updated.limiterReleaseMilliseconds=release_->value();updated.historyRetentionDays=retention_->value();updated.autoSaveHistory=autoSave_->isChecked();QString error;if(!SavePeakGuardConfig(updated,&error)){QMessageBox::critical(this,QStringLiteral("Peak Guard Settings"),error);return;}accept();
    }
    QDoubleSpinBox *near_{},*clip_{},*limiter_{};QSpinBox *sustained_{},*clipExposure_{},*recovery_{},*cooldown_{},*release_{},*retention_{};QCheckBox *autoSave_{};
};

static bool EditPeakGuardSettings(QWidget *parent){
    PeakGuardConfig before=peakGuardConfig;PeakGuardSettingsDialog dialog(parent);if(dialog.exec()!=QDialog::Accepted)return false;
    if(peakGuardMode.load()==PeakGuardMode::SoundCheck){StopPeakWarning();StartAllPeakSources();}
    return PeakGuardJson(before)!=PeakGuardJson(peakGuardConfig);
}

class ProtectionSetupDialog final:public QDialog {
public:
    ProtectionSetupDialog(std::vector<PeakSourceSnapshot> snapshots,QWidget *parent):QDialog(parent),snapshots_(std::move(snapshots)){
        setWindowTitle(QStringLiteral("Peak Guard Select Sources"));setWindowModality(Qt::ApplicationModal);auto *outer=new QVBoxLayout(this);auto *instructions=new QLabel(QStringLiteral("Select the audio sources to monitor silently. Peak Guard will use a verified enabled OBS limiter or create its own limiter before Emergency Monitoring starts."),this);instructions->setWordWrap(true);outer->addWidget(instructions);
        sources_=new QListWidget(this);sources_->setAccessibleName(QStringLiteral("Sources for Emergency Monitoring"));sources_->setSelectionMode(QAbstractItemView::SingleSelection);outer->addWidget(sources_);
        for(const auto &source:snapshots_){QString status=!source.exercised?QStringLiteral("not exercised"):QStringLiteral("maximum output %1 dB").arg(source.maximumOutput,0,'f',1);if(source.existingLimiter)status+=source.existingLimiterEnabled?QStringLiteral(", existing limiter enabled"):QStringLiteral(", existing limiter disabled");auto *item=new QListWidgetItem(QStringLiteral("%1; %2").arg(source.name,status),sources_);item->setData(Qt::UserRole,source.uuid);item->setFlags(item->flags()|Qt::ItemIsUserCheckable);item->setCheckState((source.inputEvents+source.outputEvents)>0?Qt::Checked:Qt::Unchecked);}
        auto *buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,this);ok_=buttons->button(QDialogButtonBox::Ok);ok_->setDefault(true);ok_->setAutoDefault(true);outer->addWidget(buttons);
        connect(buttons,&QDialogButtonBox::accepted,this,[this]{Start();});connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);sources_->installEventFilter(this);setTabOrder(sources_,ok_);setTabOrder(ok_,buttons->button(QDialogButtonBox::Cancel));resize(760,520);if(sources_->count())sources_->setCurrentRow(0);UpdateOk();connect(sources_,&QListWidget::itemChanged,this,[this](QListWidgetItem*){UpdateOk();});sources_->setFocus(Qt::OtherFocusReason);
    }
    QStringList selected() const{return selected_;}
protected:
    bool eventFilter(QObject *watched,QEvent *event) override{
        if(watched==sources_&&event->type()==QEvent::KeyPress){auto *key=static_cast<QKeyEvent*>(event);if(key->key()==Qt::Key_Return||key->key()==Qt::Key_Enter){if(ok_->isEnabled())ok_->click();return true;}}
        return QDialog::eventFilter(watched,event);
    }
private:
    void UpdateOk(){bool selected=false;for(int i=0;i<sources_->count();++i)selected|=sources_->item(i)->checkState()==Qt::Checked;ok_->setEnabled(selected);}
    void Start(){selected_.clear();for(int i=0;i<sources_->count();++i){auto *item=sources_->item(i);if(item->checkState()==Qt::Checked)selected_.push_back(item->data(Qt::UserRole).toString());}if(selected_.isEmpty()){sources_->setFocus();return;}accept();}
    std::vector<PeakSourceSnapshot> snapshots_;QListWidget *sources_{};QPushButton *ok_{};QStringList selected_;
};

class SoundCheckDialog final:public QDialog {
public:
    SoundCheckDialog():QDialog(nullptr){
        setAttribute(Qt::WA_ShowWithoutActivating);setWindowTitle(QStringLiteral("Peak Guard Sound Check"));setWindowModality(Qt::NonModal);setAttribute(Qt::WA_DeleteOnClose);auto *outer=new QVBoxLayout(this);status_=new QLabel(this);status_->setWordWrap(true);outer->addWidget(status_);history_=new PeakHistoryWebView(this);history_->escapeRequested=[this]{CancelSoundCheck();};outer->addWidget(history_);
        auto *buttons=new QDialogButtonBox(this);auto *settings=buttons->addButton(QStringLiteral("Settings"),QDialogButtonBox::ActionRole);auto *save=buttons->addButton(QStringLiteral("Save History"),QDialogButtonBox::ActionRole);auto *finish=buttons->addButton(QStringLiteral("Finish Sound Check"),QDialogButtonBox::AcceptRole);auto *cancel=buttons->addButton(QStringLiteral("Cancel"),QDialogButtonBox::RejectRole);outer->addWidget(buttons);
        connect(settings,&QPushButton::clicked,this,[this]{EditPeakGuardSettings(this);});connect(save,&QPushButton::clicked,this,[this]{SaveHistoryAs();});connect(finish,&QPushButton::clicked,this,[this]{Finish();});connect(cancel,&QPushButton::clicked,this,[this]{CancelSoundCheck();});statusTimer_=new QTimer(this);statusTimer_->setInterval(100);connect(statusTimer_,&QTimer::timeout,this,[this]{UpdatePeakWarning();if(++statusTicks_>=10){statusTicks_=0;UpdateStatus();}});statusTimer_->start();UpdateStatus();resize(850,600);history_->setFocus(Qt::OtherFocusReason);
    }
    void AppendEvent(const PeakHistoryEvent &event){
        history_->AppendEvent(event);
    }
    void BringForward(){if(isMinimized())showNormal();show();raise();activateWindow();}
    void FinishFromHotkey(){Finish();}
protected:
    void closeEvent(QCloseEvent *event) override{if(allowClose_){event->accept();return;}event->ignore();CancelSoundCheck();}
private:
    void UpdateStatus(){int exercised=0;for(const auto &source:peakSources){std::lock_guard<std::mutex> lock(source->mutex);if(source->exercised)++exercised;}status_->setText(QStringLiteral("Sound Check active. Monitoring %1 audio sources; %2 exercised; %3 events recorded. You may switch back to OBS while this window continues updating.").arg(peakSources.size()).arg(exercised).arg(peakHistory.size()));}
    void SaveHistoryAs(){QString path=QFileDialog::getSaveFileName(this,QStringLiteral("Save Peak Guard History"),QDir::home().filePath(QStringLiteral("Peak Guard Sound Check.txt")),QStringLiteral("Text files (*.txt);;All files (*.*)"));if(path.isEmpty())return;if(!WriteBytesAtomically(path,PeakHistoryText().toUtf8()))QMessageBox::critical(this,QStringLiteral("Peak Guard"),QStringLiteral("The history file could not be saved."));}
    void CancelSoundCheck(){if(transitioning_)return;transitioning_=true;StopPeakWarning();StopPeakMeters();RestoreTemporarilyDisabledOwnedLimiters();DiscardPeakHistory();peakGuardMode=PeakGuardMode::Idle;allowClose_=true;close();}
    void Finish(){
        if(transitioning_)return;transitioning_=true;StopPeakWarning();std::vector<PeakSourceSnapshot> snapshots=PeakSnapshots();StopPeakMeters();RestoreTemporarilyDisabledOwnedLimiters();peakGuardMode=PeakGuardMode::ProtectionSetup;hide();ProtectionSetupDialog setup(std::move(snapshots),nullptr);protectionSetupWindow=&setup;int result=setup.exec();protectionSetupWindow=nullptr;
        if(result!=QDialog::Accepted){DiscardPeakHistory();peakGuardMode=PeakGuardMode::Idle;allowClose_=true;close();return;}
        SavePeakHistory();QStringList selected=setup.selected();QString error;for(const QString &uuid:selected)if(!EnsurePeakGuardLimiter(uuid,error)){QMessageBox::critical(obsMainWindow,QStringLiteral("Peak Guard"),error);peakGuardMode=PeakGuardMode::Idle;allowClose_=true;close();return;}
        peakGuardConfig.previousSources=selected;if(!SavePeakGuardConfig(peakGuardConfig,&error))QMessageBox::warning(this,QStringLiteral("Peak Guard"),error);if(api.frontend_save)api.frontend_save();BeginPeakHistory();peakGuardMode=PeakGuardMode::Emergency;for(const QString &uuid:selected){void *source=FindSource(uuid);if(source){AddPeakSource(source);api.source_release(source);}}
        allowClose_=true;close();
    }
    QLabel *status_{};PeakHistoryWebView *history_{};QTimer *statusTimer_{};int statusTicks_{};bool allowClose_{},transitioning_{};
};

static SoundCheckDialog *SoundCheckDialogPointer(){return static_cast<SoundCheckDialog*>(soundCheckWindow.data());}

static void RecordPeakEvent(const PeakHistoryEvent &event){
    PeakGuardMode mode=peakGuardMode.load();if(mode!=PeakGuardMode::SoundCheck&&mode!=PeakGuardMode::Emergency)return;peakHistory.push_back(event);SavePeakHistory();
    if(mode==PeakGuardMode::SoundCheck){if(auto *window=SoundCheckDialogPointer())window->AppendEvent(event);}
    else if(event.condition==PeakCondition::OutputNear||event.condition==PeakCondition::OutputClip){QString error;if(EnsurePeakGuardLimiter(event.uuid,error,true)&&api.frontend_save)api.frontend_save();}
}

static void StartSoundCheck(){
    if(api.streaming_active&&api.streaming_active()){QMessageBox::information(obsMainWindow,QStringLiteral("Peak Guard"),QStringLiteral("Peak Guard Sound Check is unavailable while streaming."));return;}
    QString error;if(!LoadPeakGuardConfig(&error)){QMessageBox::critical(obsMainWindow,QStringLiteral("Peak Guard Configuration"),QStringLiteral("%1\n\nPeak Guard will use safe defaults until the file is corrected in Settings.").arg(error));peakGuardConfig=PeakGuardConfig{};peakGuardConfigLoaded=true;}
    BeginPeakHistory();peakGuardMode=PeakGuardMode::SoundCheck;TemporarilyDisableOwnedLimiters();StartAllPeakSources();auto *window=new SoundCheckDialog();soundCheckWindow=window;QObject::connect(window,&QObject::destroyed,[]{soundCheckWindow=nullptr;});window->setAttribute(Qt::WA_ShowWithoutActivating);window->show();
}

static void StopEmergencyMonitoring(){
    StopPeakMeters();SavePeakHistory();peakGuardMode=PeakGuardMode::Idle;
}

} // namespace

static std::string PeakGuardCommandLabel(){return ".Peak Guard Sound Check and Emergency Monitoring";}

static void PeakGuardHotkey(void*,hotkey_id,obs_hotkey*,bool pressed){
    if(!pressed||!obsMainWindow)return;QMetaObject::invokeMethod(obsMainWindow,[]{
        PeakGuardMode mode=peakGuardMode.load();
        if(mode==PeakGuardMode::Idle){StartSoundCheck();return;}
        if(mode==PeakGuardMode::Emergency){StopEmergencyMonitoring();return;}
        if(mode==PeakGuardMode::SoundCheck){if(auto *window=SoundCheckDialogPointer())window->FinishFromHotkey();return;}
        if(mode==PeakGuardMode::ProtectionSetup&&protectionSetupWindow){protectionSetupWindow->raise();protectionSetupWindow->activateWindow();}
    },Qt::QueuedConnection);
}

static void ShutdownPeakGuard(){
    PeakGuardMode mode=peakGuardMode.load();StopPeakWarning();StopPeakMeters();RestoreTemporarilyDisabledOwnedLimiters();if(mode==PeakGuardMode::SoundCheck||mode==PeakGuardMode::ProtectionSetup)DiscardPeakHistory();else SavePeakHistory();peakGuardMode=PeakGuardMode::Idle;if(soundCheckWindow){delete soundCheckWindow.data();soundCheckWindow=nullptr;}
}

static void HandlePeakGuardFrontendEvent(int event){
    constexpr int STREAMING_STARTING=0;if(event!=STREAMING_STARTING||peakGuardMode.load()!=PeakGuardMode::SoundCheck||!obsMainWindow)return;
    QMetaObject::invokeMethod(obsMainWindow,[]{if(soundCheckWindow)soundCheckWindow->close();},Qt::QueuedConnection);
}
