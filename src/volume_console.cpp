// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Tiflo.Info

namespace {

using VolumeText=std::array<const char*,6>;

static QString VText(const VolumeText &text){return QString::fromUtf8(text[LanguageIndex()]);}

static const VolumeText VOLUME_CONSOLE_TITLE={"Accessible Volume Console","Barrierefreie Lautstärkekonsole","Доступная консоль громкости","Доступна консоль гучності","Console de volume accessible","Consola de volumen accesible"};
static const VolumeText VOLUME_CONSOLE_COMMAND={".Open Accessible Volume Console",".Barrierefreie Lautstärkekonsole öffnen",".Открыть доступную консоль громкости",".Відкрити доступну консоль гучності",".Ouvrir la console de volume accessible",".Abrir la consola de volumen accesible"};
static const VolumeText NO_AUDIO_SOURCES={"No audio sources are currently available in the OBS Mixer.","Im OBS-Audiomixer sind derzeit keine Audioquellen verfügbar.","Сейчас в микшере OBS нет доступных аудиоисточников.","Зараз в аудіомікшері OBS немає доступних аудіоджерел.","Aucune source audio n’est actuellement disponible dans le mélangeur OBS.","Actualmente no hay fuentes de audio disponibles en el mezclador de OBS."};
static const VolumeText NO_ACTIVE_AUDIO_SOURCES={"No active audio sources are currently available.","Derzeit sind keine aktiven Audioquellen verfügbar.","Сейчас нет активных аудиоисточников.","Зараз немає активних аудіоджерел.","Aucune source audio active n’est actuellement disponible.","Actualmente no hay fuentes de audio activas disponibles."};
static const VolumeText SHOW_ALL_SOURCES={"Show All Sources","Alle Quellen anzeigen","Показать все источники","Показати всі джерела","Afficher toutes les sources","Mostrar todas las fuentes"};
static const VolumeText SHOW_ACTIVE_SOURCES={"Show Active Sources Only","Nur aktive Quellen anzeigen","Показать только активные источники","Показати лише активні джерела","Afficher uniquement les sources actives","Mostrar solo las fuentes activas"};
static const VolumeText MUTE_OUTPUT_TEXT={"Mute Output","Ausgabe stummschalten","Отключить вывод","Вимкнути вивід","Couper la sortie","Silenciar salida"};
static const VolumeText UNMUTE_OUTPUT_TEXT={"Unmute Output","Ausgabe einschalten","Включить вывод","Увімкнути вивід","Activer la sortie","Activar salida"};
static const VolumeText OUTPUT_PARAMETER_TEXT={"Output","Ausgabe","Вывод","Вивід","Sortie","Salida"};
static const VolumeText MONITORING_PARAMETER_TEXT={"Monitoring","Monitoring","Мониторинг","Моніторинг","Contrôle audio","Monitorización"};
static const VolumeText ON_TEXT={"On","Ein","Вкл.","Увімк.","Marche","Encendido"};
static const VolumeText OFF_TEXT={"Off","Aus","Выкл.","Вимк.","Arrêt","Apagado"};
static const VolumeText ENABLE_MONITORING_TEXT={"Enable Monitoring","Monitoring aktivieren","Включить мониторинг","Увімкнути моніторинг","Activer le contrôle audio","Activar monitorización"};
static const VolumeText DISABLE_MONITORING_TEXT={"Disable Monitoring","Monitoring deaktivieren","Выключить мониторинг","Вимкнути моніторинг","Désactiver le contrôle audio","Desactivar monitorización"};
static const VolumeText SILENT_TEXT={"silent","stumm","тишина","тиша","silencieux","silencio"};
static const VolumeText VOLUME_TEXT={"volume","Lautstärke","громкость","гучність","volume","volumen"};
static const VolumeText DB_TEXT={"dB","dB","дБ","дБ","dB","dB"};
static const VolumeText SLIDER_INSTRUCTIONS={"Left and Right select a source. Up and Down adjust volume. Home sets 0 dB. Space toggles monitoring and output together. Control+Space toggles monitoring only. Shift+Space toggles output only.","Links und Rechts wählen eine Quelle. Hoch und Runter ändern die Lautstärke. Pos1 setzt 0 dB. Leertaste schaltet Monitoring und Ausgabe gemeinsam um. Strg+Leertaste schaltet nur das Monitoring um. Umschalt+Leertaste schaltet nur die Ausgabe um.","Стрелки влево и вправо выбирают источник. Стрелки вверх и вниз меняют громкость. Home устанавливает 0 дБ. Пробел одновременно переключает мониторинг и вывод. Control+Пробел переключает только мониторинг. Shift+Пробел переключает только вывод.","Стрілки ліворуч і праворуч вибирають джерело. Стрілки вгору і вниз змінюють гучність. Home встановлює 0 дБ. Пробіл одночасно перемикає моніторинг і вивід. Control+Пробіл перемикає лише моніторинг. Shift+Пробіл перемикає лише вивід.","Gauche et Droite sélectionnent une source. Haut et Bas règlent le volume. Origine établit 0 dB. Espace bascule ensemble le contrôle audio et la sortie. Contrôle+Espace bascule uniquement le contrôle audio. Majuscule+Espace bascule uniquement la sortie.","Izquierda y Derecha seleccionan una fuente. Arriba y Abajo ajustan el volumen. Inicio establece 0 dB. Espacio alterna conjuntamente la monitorización y la salida. Control+Espacio alterna solo la monitorización. Mayús+Espacio alterna solo la salida."};

struct VolumeEntry{
    void *source{};
    QString uuid;
    QString name;
    bool active{};
    int value{-100};
    int maximumValue{0};
    bool outputEnabled{true};
    bool monitoringEnabled{};
    QPointer<QWidget> column;
    QPointer<QLabel> nameLabel;
    QPointer<QSlider> slider;
    QPointer<QLabel> valueLabel;
    QPointer<QPushButton> outputButton;
    QPointer<QPushButton> monitoringButton;
};

static int MixerRank(const QString &name,const std::vector<QString> &mixerNames){
    for(size_t index=0;index<mixerNames.size();++index)if(mixerNames[index].contains(name,Qt::CaseInsensitive)||name.contains(mixerNames[index],Qt::CaseInsensitive))return static_cast<int>(index);
    return static_cast<int>(mixerNames.size());
}

struct AudioSourceCollection{std::vector<VolumeEntry> *entries{};bool activeOnly{};};

static bool CollectAudioSource(void *parameter,void *source){
    auto *collection=static_cast<AudioSourceCollection*>(parameter);constexpr uint32_t AUDIO_FLAG=1u<<1;if(!source||(api.source_output_flags(source)&AUDIO_FLAG)==0)return true;
    bool active=api.source_active(source)&&api.source_audio_active(source);if(collection->activeOnly&&!active)return true;
    void *reference=api.source_get_ref(source);if(!reference)return true;const char *rawUuid=api.source_uuid(reference),*rawName=api.source_name(reference);QString name=QString::fromUtf8(rawName?rawName:"");if(name.isEmpty())name=LText(LocalText::AudioSource);VolumeEntry entry;entry.source=reference;entry.uuid=QString::fromUtf8(rawUuid?rawUuid:"");entry.name=std::move(name);entry.active=active;collection->entries->push_back(std::move(entry));return true;
}

static std::vector<VolumeEntry> CurrentMixerEntries(bool activeOnly){
    std::vector<VolumeEntry> entries;AudioSourceCollection collection{&entries,activeOnly};api.enum_sources(CollectAudioSource,&collection);std::vector<QString> mixerNames;for(QAbstractSlider *slider:MixerSliders()){QVariant original=slider->property("accessibleObsStudioOriginalName");QString name=original.isValid()?original.toString():slider->accessibleName();if(!name.isEmpty())mixerNames.push_back(name);}
    std::stable_sort(entries.begin(),entries.end(),[&](const VolumeEntry &left,const VolumeEntry &right){if(left.active!=right.active)return left.active>right.active;int leftRank=MixerRank(left.name,mixerNames),rightRank=MixerRank(right.name,mixerNames);if(leftRank!=rightRank)return leftRank<rightRank;return left.name.compare(right.name,Qt::CaseInsensitive)<0;});return entries;
}

static int VolumeToDb(float multiplier){if(multiplier<=0.00001f)return -100;double db=20.0*std::log10(static_cast<double>(multiplier));return std::max(static_cast<int>(std::lround(db)),-100);}
static float DbToVolume(int value){if(value<=-100)return 0.0f;return static_cast<float>(std::pow(10.0,static_cast<double>(value)/20.0));}
static QString DbValueText(int value){if(value<=-100)return VText(SILENT_TEXT);return QStringLiteral("%1 %2").arg(value).arg(VText(DB_TEXT));}

class VolumeConsoleDialog final:public QDialog{
public:
    explicit VolumeConsoleDialog(QWidget *parent,const QString &initialSourceUuid={}):QDialog(parent){
        setAttribute(Qt::WA_DeleteOnClose);setWindowTitle(VText(VOLUME_CONSOLE_TITLE));setAccessibleDescription(VText(SLIDER_INSTRUCTIONS));setWindowModality(Qt::ApplicationModal);setModal(true);resize(800,460);setMinimumSize(460,360);
        auto *outer=new QVBoxLayout(this);sourceViewButton_=new QPushButton(this);sourceViewButton_->setAutoDefault(false);sourceViewButton_->setDefault(false);sourceViewButton_->installEventFilter(this);UpdateSourceViewButton();outer->addWidget(sourceViewButton_);scroll_=new QScrollArea(this);scroll_->setWidgetResizable(true);scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);scroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);scroll_->setFocusPolicy(Qt::NoFocus);outer->addWidget(scroll_);
        panel_=new QWidget(scroll_);sourceLayout_=new QHBoxLayout(panel_);sourceLayout_->setAlignment(Qt::AlignLeft);emptyMessage_=new QLabel(VText(NO_AUDIO_SOURCES),panel_);emptyMessage_->setWordWrap(true);sourceLayout_->addWidget(emptyMessage_);sourceLayout_->addStretch(1);scroll_->setWidget(panel_);
        buttons_=new QDialogButtonBox(QDialogButtonBox::Close,this);if(QPushButton *close=buttons_->button(QDialogButtonBox::Close))close->setText(QString::fromWCharArray(Tr(UiText::Close)));connect(buttons_,&QDialogButtonBox::rejected,this,&QDialog::reject);connect(buttons_,&QDialogButtonBox::accepted,this,&QDialog::accept);outer->addWidget(buttons_);
        connect(sourceViewButton_,&QPushButton::clicked,this,[this]{ToggleSourceView();});
        InitializeSources(CurrentMixerEntries(true));refreshTimer_=new QTimer(this);refreshTimer_->setInterval(500);connect(refreshTimer_,&QTimer::timeout,this,[this]{RefreshFromObs();});refreshTimer_->start();if(!initialSourceUuid.isEmpty()&&FocusSource(initialSourceUuid)){}else if(!sources_.empty())FocusIndex(0);else sourceViewButton_->setFocus(Qt::OtherFocusReason);
    }
    ~VolumeConsoleDialog() override{ReleaseSources();}
    bool FocusSource(const QString &uuid){
        auto findIndex=[this,&uuid]{for(size_t index=0;index<sources_.size();++index)if(sources_[index].uuid==uuid)return static_cast<int>(index);return -1;};
        int index=findIndex();if(index<0&&activeOnlyState_){activeOnlyState_=false;UpdateSourceViewButton();RebuildSources(false);index=findIndex();}if(index<0)return false;
        if(isMinimized())showNormal();else if(!isVisible())show();raise();activateWindow();FocusIndex(index);return true;
    }
protected:
    bool eventFilter(QObject *watched,QEvent *event) override{
        if(watched==sourceViewButton_&&(event->type()==QEvent::KeyPress||event->type()==QEvent::KeyRelease)){auto *keyEvent=static_cast<QKeyEvent*>(event);if(keyEvent->key()==Qt::Key_Space)return true;if(event->type()==QEvent::KeyPress&&(keyEvent->key()==Qt::Key_Return||keyEvent->key()==Qt::Key_Enter)){ToggleSourceView();return true;}}
        if(event->type()!=QEvent::KeyPress)return QDialog::eventFilter(watched,event);auto *keyEvent=static_cast<QKeyEvent*>(event);int index=EntryIndex(watched);if(index<0)return QDialog::eventFilter(watched,event);
        Qt::KeyboardModifiers modifiers=keyEvent->modifiers()&~Qt::KeypadModifier;
        if(keyEvent->key()==Qt::Key_Space){
            if(modifiers==Qt::NoModifier){ToggleMaster(index);return true;}
            if(modifiers==Qt::ControlModifier){ToggleMonitoring(index);return true;}
            if(modifiers==Qt::ShiftModifier){ToggleOutput(index);return true;}
        }
        if(modifiers==Qt::NoModifier){
            if(keyEvent->key()==Qt::Key_Left){FocusIndex(index-1);return true;}if(keyEvent->key()==Qt::Key_Right){FocusIndex(index+1);return true;}
            if(watched==sources_[static_cast<size_t>(index)].slider&&(keyEvent->key()==Qt::Key_Up||keyEvent->key()==Qt::Key_Down)){VolumeEntry &entry=sources_[static_cast<size_t>(index)];int step=keyEvent->key()==Qt::Key_Up?1:-1;entry.slider->setValue(std::clamp(entry.value+step,-100,entry.maximumValue));return true;}
            if(keyEvent->key()==Qt::Key_Home&&watched==sources_[static_cast<size_t>(index)].slider){QSlider *slider=sources_[static_cast<size_t>(index)].slider;if(slider->value()!=0)slider->setValue(0);else Announce(index);return true;}
            if(keyEvent->key()==Qt::Key_Return||keyEvent->key()==Qt::Key_Enter){const VolumeEntry &entry=sources_[static_cast<size_t>(index)];if(watched==entry.outputButton){ToggleOutput(index);return true;}if(watched==entry.monitoringButton){ToggleMonitoring(index);return true;}}
            int direct=DirectIndex(keyEvent->key());if(direct>=0){FocusIndex(direct);return true;}
            if(keyEvent->key()==Qt::Key_Escape){reject();return true;}
        }
        return QDialog::eventFilter(watched,event);
    }
    void keyPressEvent(QKeyEvent *event) override{
        Qt::KeyboardModifiers modifiers=event->modifiers()&~Qt::KeypadModifier;if(modifiers==Qt::NoModifier){if(event->key()==Qt::Key_Escape||event->key()==Qt::Key_Return||event->key()==Qt::Key_Enter){reject();return;}int direct=DirectIndex(event->key());if(direct>=0){FocusIndex(direct);return;}}QDialog::keyPressEvent(event);
    }
private:
    static int DirectIndex(int key){if(key>=Qt::Key_1&&key<=Qt::Key_9)return key-Qt::Key_1;if(key==Qt::Key_0)return 9;return -1;}
    void UpdateSourceViewButton(){QString text=VText(activeOnlyState_?SHOW_ALL_SOURCES:SHOW_ACTIVE_SOURCES);sourceViewButton_->setText(text);sourceViewButton_->setAccessibleName(text);}
    void ToggleSourceView(){activeOnlyState_=!activeOnlyState_;UpdateSourceViewButton();RebuildSources(activeOnlyState_);}
    static bool IndependentMonitoringMute(){uint32_t version=api.get_version();uint32_t major=(version>>24)&0xFF,minor=(version>>16)&0xFF;return major>32||(major==32&&minor>=2);}
    int EntryIndex(QObject *object) const{for(size_t index=0;index<sources_.size();++index)if(object==sources_[index].slider||object==sources_[index].outputButton||object==sources_[index].monitoringButton)return static_cast<int>(index);return -1;}
    int FocusedIndex() const{QWidget *focus=QApplication::focusWidget();return EntryIndex(focus);}
    QString Announcement(int index) const{const VolumeEntry &entry=sources_[static_cast<size_t>(index)];return QStringLiteral("%1. %2, %3 %4. %5: %6. %7: %8.").arg(index+1).arg(entry.name).arg(VText(VOLUME_TEXT)).arg(DbValueText(entry.value)).arg(VText(OUTPUT_PARAMETER_TEXT)).arg(VText(entry.outputEnabled?ON_TEXT:OFF_TEXT)).arg(VText(MONITORING_PARAMETER_TEXT)).arg(VText(entry.monitoringEnabled?ON_TEXT:OFF_TEXT));}
    void Announce(int index){if(index<0||index>=static_cast<int>(sources_.size()))return;QAccessibleAnnouncementEvent announcement(this,Announcement(index));announcement.setPoliteness(QAccessible::AnnouncementPoliteness::Assertive);QAccessible::updateAccessibility(&announcement);}
    void UpdateValueVisual(VolumeEntry &entry){QString valueText=DbValueText(entry.value);if(entry.valueLabel&&entry.valueLabel->text()!=valueText)entry.valueLabel->setText(valueText);}
    void UpdateRoutingVisuals(VolumeEntry &entry){
        if(entry.outputButton){QString text=VText(entry.outputEnabled?MUTE_OUTPUT_TEXT:UNMUTE_OUTPUT_TEXT);entry.outputButton->setText(text);entry.outputButton->setAccessibleName(QStringLiteral("%1: %2").arg(entry.name,text));}
        if(entry.monitoringButton){QString text=VText(entry.monitoringEnabled?DISABLE_MONITORING_TEXT:ENABLE_MONITORING_TEXT);entry.monitoringButton->setText(text);entry.monitoringButton->setAccessibleName(QStringLiteral("%1: %2").arg(entry.name,text));}
    }
    void ApplyRouting(int index,bool outputEnabled,bool monitoringEnabled){
        if(index<0||index>=static_cast<int>(sources_.size()))return;VolumeEntry &entry=sources_[static_cast<size_t>(index)];
        if(!outputEnabled)api.source_set_muted(entry.source,true);
        const int monitorType=monitoringEnabled?(IndependentMonitoringMute()?2:(outputEnabled?2:1)):0;
        api.source_set_monitoring_type(entry.source,monitorType);
        if(outputEnabled||(!IndependentMonitoringMute()&&monitoringEnabled))api.source_set_muted(entry.source,false);
        entry.outputEnabled=outputEnabled;entry.monitoringEnabled=monitoringEnabled;UpdateRoutingVisuals(entry);Announce(index);
    }
    void ToggleMaster(int index){if(index<0||index>=static_cast<int>(sources_.size()))return;const VolumeEntry &entry=sources_[static_cast<size_t>(index)];const bool enable=!(entry.outputEnabled||entry.monitoringEnabled);ApplyRouting(index,enable,enable);}
    void ToggleOutput(int index){if(index<0||index>=static_cast<int>(sources_.size()))return;const VolumeEntry &entry=sources_[static_cast<size_t>(index)];ApplyRouting(index,!entry.outputEnabled,entry.monitoringEnabled);}
    void ToggleMonitoring(int index){if(index<0||index>=static_cast<int>(sources_.size()))return;const VolumeEntry &entry=sources_[static_cast<size_t>(index)];ApplyRouting(index,entry.outputEnabled,!entry.monitoringEnabled);}
    void FocusIndex(int index){if(index<0||index>=static_cast<int>(sources_.size())){QApplication::beep();return;}QSlider *slider=sources_[static_cast<size_t>(index)].slider;if(!slider)return;scroll_->ensureWidgetVisible(slider,30,30);if(QApplication::focusWidget()!=slider)slider->setFocus(Qt::ShortcutFocusReason);}
    void ReleaseSources(){for(VolumeEntry &entry:sources_){if(entry.source)api.source_release(entry.source);if(entry.column)delete entry.column.data();}sources_.clear();}
    void CreateControls(VolumeEntry &entry,int index){
        entry.column=new QWidget(panel_);entry.column->setMinimumWidth(160);auto *columnLayout=new QVBoxLayout(entry.column);entry.nameLabel=new QLabel(entry.column);entry.nameLabel->setAlignment(Qt::AlignHCenter);entry.nameLabel->setWordWrap(true);columnLayout->addWidget(entry.nameLabel);entry.slider=new QSlider(Qt::Vertical,entry.column);entry.slider->setSingleStep(1);entry.slider->setPageStep(5);entry.slider->setMinimumHeight(230);entry.slider->setFocusPolicy(Qt::StrongFocus);entry.slider->installEventFilter(this);columnLayout->addWidget(entry.slider,1,Qt::AlignHCenter);entry.valueLabel=new QLabel(entry.column);entry.valueLabel->setAlignment(Qt::AlignHCenter);entry.valueLabel->setAccessibleName(QStringLiteral(" "));entry.valueLabel->setAccessibleDescription(QStringLiteral(" "));columnLayout->addWidget(entry.valueLabel);entry.outputButton=new QPushButton(entry.column);entry.outputButton->setFocusPolicy(Qt::StrongFocus);entry.outputButton->installEventFilter(this);entry.outputButton->setAccessibleDescription(VText(SLIDER_INSTRUCTIONS));columnLayout->addWidget(entry.outputButton);entry.monitoringButton=new QPushButton(entry.column);entry.monitoringButton->setFocusPolicy(Qt::StrongFocus);entry.monitoringButton->installEventFilter(this);entry.monitoringButton->setAccessibleDescription(VText(SLIDER_INSTRUCTIONS));columnLayout->addWidget(entry.monitoringButton);void *source=entry.source;
        entry.value=VolumeToDb(api.source_get_volume(source));entry.maximumValue=std::max(entry.value,0);entry.slider->setRange(-100,entry.maximumValue);const int monitorType=api.source_monitoring_type(source);entry.monitoringEnabled=monitorType!=0;entry.outputEnabled=!api.source_muted(source)&&monitorType!=1;entry.slider->setValue(entry.value);entry.nameLabel->setText(QStringLiteral("%1. %2").arg(index+1).arg(entry.name));entry.slider->setAccessibleName(QStringLiteral("%1. %2. %3.").arg(index+1).arg(entry.name,VText(VOLUME_TEXT)));entry.slider->setAccessibleDescription(VText(SLIDER_INSTRUCTIONS));UpdateValueVisual(entry);UpdateRoutingVisuals(entry);
        connect(entry.slider,&QSlider::valueChanged,this,[this,source](int value){auto found=std::find_if(sources_.begin(),sources_.end(),[source](const VolumeEntry &candidate){return candidate.source==source;});if(found==sources_.end())return;found->value=value;api.source_set_volume(source,DbToVolume(value));UpdateValueVisual(*found);});
        connect(entry.outputButton,&QPushButton::clicked,this,[this,source]{auto found=std::find_if(sources_.begin(),sources_.end(),[source](const VolumeEntry &candidate){return candidate.source==source;});if(found!=sources_.end())ToggleOutput(static_cast<int>(std::distance(sources_.begin(),found)));});
        connect(entry.monitoringButton,&QPushButton::clicked,this,[this,source]{auto found=std::find_if(sources_.begin(),sources_.end(),[source](const VolumeEntry &candidate){return candidate.source==source;});if(found!=sources_.end())ToggleMonitoring(static_cast<int>(std::distance(sources_.begin(),found)));});
    }
    void InitializeSources(std::vector<VolumeEntry> entries){sources_=std::move(entries);emptyMessage_->setText(VText(activeOnlyState_?NO_ACTIVE_AUDIO_SOURCES:NO_AUDIO_SOURCES));emptyMessage_->setVisible(sources_.empty());for(size_t index=0;index<sources_.size();++index){CreateControls(sources_[index],static_cast<int>(index));sourceLayout_->insertWidget(static_cast<int>(index),sources_[index].column);}}
    void RebuildSources(bool activeOnly){
        QString focusedUuid;int focused=FocusedIndex();if(focused>=0)focusedUuid=sources_[static_cast<size_t>(focused)].uuid;
        ReleaseSources();InitializeSources(CurrentMixerEntries(activeOnly));
        int replacement=-1;for(size_t index=0;index<sources_.size();++index)if(sources_[index].uuid==focusedUuid){replacement=static_cast<int>(index);break;}
        if(replacement>=0)FocusIndex(replacement);else if(!sources_.empty())FocusIndex(0);else sourceViewButton_->setFocus(Qt::OtherFocusReason);
    }
    void RefreshFromObs(){
        std::vector<VolumeEntry> current=CurrentMixerEntries(activeOnlyState_);
        bool changed=current.size()!=sources_.size();
        if(!changed)for(size_t index=0;index<current.size();++index)if(current[index].uuid!=sources_[index].uuid){changed=true;break;}
        if(changed){
            for(VolumeEntry &entry:current)if(entry.source)api.source_release(entry.source);
            RebuildSources(activeOnlyState_);
            return;
        }
        for(VolumeEntry &entry:current)if(entry.source)api.source_release(entry.source);
        for(VolumeEntry &entry:sources_){
            int value=VolumeToDb(api.source_get_volume(entry.source));int maximum=std::max({0,entry.maximumValue,value});
            if(entry.slider&&(entry.value!=value||entry.maximumValue!=maximum)){
                QSignalBlocker blocker(entry.slider);entry.maximumValue=maximum;entry.value=value;entry.slider->setMaximum(maximum);entry.slider->setValue(value);UpdateValueVisual(entry);
            }
            const int monitorType=api.source_monitoring_type(entry.source);const bool monitoring=monitorType!=0;const bool output=!api.source_muted(entry.source)&&monitorType!=1;
            if(entry.monitoringEnabled!=monitoring||entry.outputEnabled!=output){entry.monitoringEnabled=monitoring;entry.outputEnabled=output;UpdateRoutingVisuals(entry);}
        }
    }
    bool activeOnlyState_{true};QPushButton *sourceViewButton_{};QScrollArea *scroll_{};QWidget *panel_{};QHBoxLayout *sourceLayout_{};QLabel *emptyMessage_{};QDialogButtonBox *buttons_{};QTimer *refreshTimer_{};std::vector<VolumeEntry> sources_;
};

static QPointer<VolumeConsoleDialog> volumeConsoleWindow;
static QPointer<QWidget> volumeConsoleReturnFocus;

} // namespace

static std::string VolumeConsoleCommandLabel(){QByteArray utf8=VText(VOLUME_CONSOLE_COMMAND).toUtf8();return std::string(utf8.constData(),static_cast<size_t>(utf8.size()));}

static void OpenVolumeConsoleForSource(const QString &sourceUuid,bool automatic){
    if(!obsMainWindow||shuttingDown)return;
    if(volumeConsoleWindow){if(!sourceUuid.isEmpty())volumeConsoleWindow->FocusSource(sourceUuid);else{if(volumeConsoleWindow->isMinimized())volumeConsoleWindow->showNormal();else if(!volumeConsoleWindow->isVisible())volumeConsoleWindow->show();volumeConsoleWindow->raise();volumeConsoleWindow->activateWindow();}return;}
    if(!automatic&&!MainInterfaceActive())return;
    volumeConsoleReturnFocus=QApplication::focusWidget();auto *dialog=new VolumeConsoleDialog(obsMainWindow,sourceUuid);volumeConsoleWindow=dialog;QObject::connect(dialog,&QObject::destroyed,obsMainWindow,[]{volumeConsoleWindow=nullptr;if(shuttingDown){volumeConsoleReturnFocus=nullptr;return;}QPointer<QWidget> returnFocus=volumeConsoleReturnFocus;volumeConsoleReturnFocus=nullptr;if(returnFocus&&returnFocus->isVisible()&&returnFocus->isEnabled()&&QApplication::focusWidget()!=returnFocus){if(QWidget *window=returnFocus->window();window&&QApplication::activeWindow()!=window)window->activateWindow();returnFocus->setFocus(Qt::ShortcutFocusReason);}});dialog->open();dialog->raise();dialog->activateWindow();
}

static void VolumeConsoleHotkey(void*,hotkey_id,obs_hotkey*,bool pressed){
    if(!pressed||!PluginEventTarget()||shuttingDown)return;QMetaObject::invokeMethod(PluginEventTarget(),[]{OpenVolumeConsoleForSource({},false);},Qt::QueuedConnection);
}

static void ShutdownVolumeConsole(){
    volumeConsoleReturnFocus=nullptr;if(!volumeConsoleWindow)return;VolumeConsoleDialog *dialog=volumeConsoleWindow.data();volumeConsoleWindow=nullptr;dialog->setAttribute(Qt::WA_DeleteOnClose,false);delete dialog;
}
