// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Tiflo.Info

namespace {

using VolumeText=std::array<const char*,6>;

static QString VText(const VolumeText &text){return QString::fromUtf8(text[LanguageIndex()]);}

static const VolumeText VOLUME_CONSOLE_TITLE={"Accessible Volume Console","Barrierefreie Lautstärkekonsole","Доступная консоль громкости","Доступна консоль гучності","Console de volume accessible","Consola de volumen accesible"};
static const VolumeText VOLUME_CONSOLE_COMMAND={"Accessible OBS Studio: Open Accessible Volume Console","Accessible OBS Studio: Barrierefreie Lautstärkekonsole öffnen","Accessible OBS Studio: Открыть доступную консоль громкости","Accessible OBS Studio: Відкрити доступну консоль гучності","Accessible OBS Studio : Ouvrir la console de volume accessible","Accessible OBS Studio: Abrir la consola de volumen accesible"};
static const VolumeText NO_AUDIO_SOURCES={"No audio sources are currently available in the OBS Mixer.","Im OBS-Audiomixer sind derzeit keine Audioquellen verfügbar.","Сейчас в микшере OBS нет доступных аудиоисточников.","Зараз в аудіомікшері OBS немає доступних аудіоджерел.","Aucune source audio n’est actuellement disponible dans le mélangeur OBS.","Actualmente no hay fuentes de audio disponibles en el mezclador de OBS."};
static const VolumeText MUTE_TEXT={"Mute","Stummschalten","Выключить звук","Вимкнути звук","Couper le son","Silenciar"};
static const VolumeText UNMUTE_TEXT={"Unmute","Stummschaltung aufheben","Включить звук","Увімкнути звук","Rétablir le son","Activar sonido"};
static const VolumeText MUTED_TEXT={"muted","stummgeschaltet","звук выключен","звук вимкнено","son coupé","silenciado"};
static const VolumeText UNMUTED_TEXT={"unmuted","nicht stummgeschaltet","звук включён","звук увімкнено","son actif","con sonido"};
static const VolumeText SILENT_TEXT={"silent","stumm","тишина","тиша","silencieux","silencio"};
static const VolumeText VOLUME_TEXT={"volume","Lautstärke","громкость","гучність","volume","volumen"};
static const VolumeText DB_TEXT={"dB","dB","дБ","дБ","dB","dB"};
static const VolumeText SLIDER_INSTRUCTIONS={"Left and Right select a source. Up and Down adjust volume. Home sets 0 dB. Space toggles mute.","Links und Rechts wählen eine Quelle. Hoch und Runter ändern die Lautstärke. Pos1 setzt 0 dB. Die Leertaste schaltet stumm.","Стрелки влево и вправо выбирают источник. Стрелки вверх и вниз меняют громкость. Home устанавливает 0 дБ. Пробел включает или выключает звук.","Стрілки ліворуч і праворуч вибирають джерело. Стрілки вгору і вниз змінюють гучність. Home встановлює 0 дБ. Пробіл вмикає або вимикає звук.","Gauche et Droite sélectionnent une source. Haut et Bas règlent le volume. Origine établit 0 dB. Espace active ou coupe le son.","Izquierda y Derecha seleccionan una fuente. Arriba y Abajo ajustan el volumen. Inicio establece 0 dB. Espacio alterna el silencio."};

struct VolumeEntry{
    void *source{};
    QString name;
    QPointer<QSlider> slider;
    QPointer<QLabel> valueLabel;
    QPointer<QPushButton> muteButton;
};

static int MixerRank(const QString &name,const std::vector<QString> &mixerNames){
    for(size_t index=0;index<mixerNames.size();++index)if(mixerNames[index].contains(name,Qt::CaseInsensitive)||name.contains(mixerNames[index],Qt::CaseInsensitive))return static_cast<int>(index);
    return static_cast<int>(mixerNames.size());
}

static bool CollectAudioSource(void *parameter,void *source){
    auto *entries=static_cast<std::vector<VolumeEntry>*>(parameter);constexpr uint32_t AUDIO_FLAG=1u<<1;if(!source||(api.source_output_flags(source)&AUDIO_FLAG)==0||!api.source_audio_active(source))return true;
    void *reference=api.source_get_ref(source);if(!reference)return true;const char *rawName=api.source_name(reference);QString name=QString::fromUtf8(rawName?rawName:"");if(name.isEmpty())name=QStringLiteral("Audio source");entries->push_back(VolumeEntry{reference,name,{},{},{}});return true;
}

static std::vector<VolumeEntry> CurrentMixerEntries(){
    std::vector<VolumeEntry> entries;api.enum_sources(CollectAudioSource,&entries);std::vector<QString> mixerNames;for(QAbstractSlider *slider:MixerSliders()){QVariant original=slider->property("accessibleObsStudioOriginalName");QString name=original.isValid()?original.toString():slider->accessibleName();if(!name.isEmpty())mixerNames.push_back(name);}
    std::stable_sort(entries.begin(),entries.end(),[&](const VolumeEntry &left,const VolumeEntry &right){int leftRank=MixerRank(left.name,mixerNames),rightRank=MixerRank(right.name,mixerNames);if(leftRank!=rightRank)return leftRank<rightRank;return left.name.compare(right.name,Qt::CaseInsensitive)<0;});return entries;
}

static int VolumeToDb(float multiplier){if(multiplier<=0.00001f)return -100;double db=20.0*std::log10(static_cast<double>(multiplier));return std::clamp(static_cast<int>(std::lround(db)),-100,0);}
static float DbToVolume(int value){if(value<=-100)return 0.0f;return static_cast<float>(std::pow(10.0,static_cast<double>(value)/20.0));}
static QString DbValueText(int value){if(value<=-100)return VText(SILENT_TEXT);return QStringLiteral("%1 %2").arg(value).arg(VText(DB_TEXT));}

class VolumeConsoleDialog final:public QDialog{
public:
    explicit VolumeConsoleDialog(QWidget *parent):QDialog(parent){
        setWindowTitle(VText(VOLUME_CONSOLE_TITLE));setWindowModality(Qt::ApplicationModal);setModal(true);resize(800,460);setMinimumSize(460,360);
        auto *outer=new QVBoxLayout(this);scroll_=new QScrollArea(this);scroll_->setWidgetResizable(true);scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);scroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);scroll_->setFocusPolicy(Qt::NoFocus);outer->addWidget(scroll_);
        ReplaceSources(CurrentMixerEntries(),nullptr);
        buttons_=new QDialogButtonBox(QDialogButtonBox::Close,this);connect(buttons_,&QDialogButtonBox::rejected,this,&QDialog::reject);connect(buttons_,&QDialogButtonBox::accepted,this,&QDialog::accept);outer->addWidget(buttons_);
        timer_=new QTimer(this);timer_->setInterval(500);connect(timer_,&QTimer::timeout,this,[this]{RefreshFromObs();});timer_->start();
    }
    ~VolumeConsoleDialog() override{ReleaseSources();}
protected:
    bool eventFilter(QObject *watched,QEvent *event) override{
        if(event->type()!=QEvent::KeyPress)return QDialog::eventFilter(watched,event);auto *keyEvent=static_cast<QKeyEvent*>(event);int index=EntryIndex(watched);if(index<0)return QDialog::eventFilter(watched,event);
        Qt::KeyboardModifiers modifiers=keyEvent->modifiers()&~Qt::KeypadModifier;if(modifiers==Qt::NoModifier){
            if(keyEvent->key()==Qt::Key_Left){FocusIndex(index-1);return true;}if(keyEvent->key()==Qt::Key_Right){FocusIndex(index+1);return true;}
            if(keyEvent->key()==Qt::Key_Home&&watched==sources_[static_cast<size_t>(index)].slider){QSlider *slider=sources_[static_cast<size_t>(index)].slider;if(slider->value()!=0)slider->setValue(0);else Announce(index);return true;}
            if(keyEvent->key()==Qt::Key_Space&&watched==sources_[static_cast<size_t>(index)].slider){ToggleMute(index);return true;}
            int direct=DirectIndex(keyEvent->key());if(direct>=0){FocusIndex(direct);return true;}
            if(keyEvent->key()==Qt::Key_Escape){reject();return true;}
        }
        return QDialog::eventFilter(watched,event);
    }
    void keyPressEvent(QKeyEvent *event) override{
        Qt::KeyboardModifiers modifiers=event->modifiers()&~Qt::KeypadModifier;if(modifiers==Qt::NoModifier){if(event->key()==Qt::Key_Escape){reject();return;}int direct=DirectIndex(event->key());if(direct>=0){FocusIndex(direct);return;}}QDialog::keyPressEvent(event);
    }
private:
    static int DirectIndex(int key){if(key>=Qt::Key_1&&key<=Qt::Key_9)return key-Qt::Key_1;if(key==Qt::Key_0)return 9;return -1;}
    int EntryIndex(QObject *object) const{for(size_t index=0;index<sources_.size();++index)if(object==sources_[index].slider||object==sources_[index].muteButton)return static_cast<int>(index);return -1;}
    int FocusedIndex() const{QWidget *focus=QApplication::focusWidget();return EntryIndex(focus);}
    QString Announcement(int index) const{const VolumeEntry &entry=sources_[static_cast<size_t>(index)];int value=VolumeToDb(api.source_get_volume(entry.source));bool muted=api.source_muted(entry.source);return QStringLiteral("%1. %2, %3 %4, %5.").arg(index+1).arg(entry.name,VText(VOLUME_TEXT),DbValueText(value),VText(muted?MUTED_TEXT:UNMUTED_TEXT));}
    void Announce(int index){if(index<0||index>=static_cast<int>(sources_.size()))return;QAccessibleAnnouncementEvent announcement(this,Announcement(index));announcement.setPoliteness(QAccessible::AnnouncementPoliteness::Assertive);QAccessible::updateAccessibility(&announcement);}
    void UpdateVisual(int index){
        VolumeEntry &entry=sources_[static_cast<size_t>(index)];int value=VolumeToDb(api.source_get_volume(entry.source));bool muted=api.source_muted(entry.source);syncing_=true;if(entry.slider&&entry.slider->value()!=value)entry.slider->setValue(value);syncing_=false;QString valueText=DbValueText(value);if(entry.valueLabel&&entry.valueLabel->text()!=valueText)entry.valueLabel->setText(valueText);if(entry.slider){QString accessibleName=QStringLiteral("%1. %2. %3.").arg(index+1).arg(entry.name,VText(VOLUME_TEXT));if(entry.slider->accessibleName()!=accessibleName)entry.slider->setAccessibleName(accessibleName);QString instructions=VText(SLIDER_INSTRUCTIONS);if(entry.slider->accessibleDescription()!=instructions)entry.slider->setAccessibleDescription(instructions);}if(entry.muteButton){QString buttonText=VText(muted?UNMUTE_TEXT:MUTE_TEXT);if(entry.muteButton->text()!=buttonText)entry.muteButton->setText(buttonText);QString buttonName=QStringLiteral("%1: %2").arg(entry.name,buttonText);if(entry.muteButton->accessibleName()!=buttonName)entry.muteButton->setAccessibleName(buttonName);}
    }
    void ToggleMute(int index){if(index<0||index>=static_cast<int>(sources_.size()))return;VolumeEntry &entry=sources_[static_cast<size_t>(index)];api.source_set_muted(entry.source,!api.source_muted(entry.source));UpdateVisual(index);Announce(index);}
    void FocusIndex(int index){if(index<0||index>=static_cast<int>(sources_.size())){QApplication::beep();return;}QSlider *slider=sources_[static_cast<size_t>(index)].slider;if(!slider)return;scroll_->ensureWidgetVisible(slider,30,30);if(QApplication::focusWidget()!=slider)slider->setFocus(Qt::ShortcutFocusReason);}
    void ReleaseSources(){for(VolumeEntry &entry:sources_)if(entry.source)api.source_release(entry.source);sources_.clear();}
    bool SameSources(const std::vector<VolumeEntry> &fresh) const{if(fresh.size()!=sources_.size())return false;for(size_t index=0;index<fresh.size();++index)if(fresh[index].source!=sources_[index].source)return false;return true;}
    void ReplaceSources(std::vector<VolumeEntry> fresh,void *preferredSource){
        QWidget *old=scroll_->takeWidget();delete old;ReleaseSources();sources_=std::move(fresh);auto *panel=new QWidget(scroll_);auto *layout=new QHBoxLayout(panel);layout->setAlignment(Qt::AlignLeft);
        if(sources_.empty()){auto *message=new QLabel(VText(NO_AUDIO_SOURCES),panel);message->setWordWrap(true);layout->addWidget(message);scroll_->setWidget(panel);if(isVisible()&&buttons_)QTimer::singleShot(0,buttons_,[this]{if(QPushButton *close=buttons_->button(QDialogButtonBox::Close);close&&QApplication::focusWidget()!=close)close->setFocus(Qt::OtherFocusReason);});return;}
        int preferredIndex=0;for(size_t index=0;index<sources_.size();++index){VolumeEntry &entry=sources_[index];if(entry.source==preferredSource)preferredIndex=static_cast<int>(index);auto *column=new QWidget(panel);column->setMinimumWidth(145);auto *columnLayout=new QVBoxLayout(column);auto *name=new QLabel(QStringLiteral("%1. %2").arg(index+1).arg(entry.name),column);name->setAlignment(Qt::AlignHCenter);name->setWordWrap(true);columnLayout->addWidget(name);entry.slider=new QSlider(Qt::Vertical,column);entry.slider->setRange(-100,0);entry.slider->setSingleStep(1);entry.slider->setPageStep(5);entry.slider->setMinimumHeight(230);entry.slider->setFocusPolicy(Qt::StrongFocus);entry.slider->installEventFilter(this);columnLayout->addWidget(entry.slider,1,Qt::AlignHCenter);entry.valueLabel=new QLabel(column);entry.valueLabel->setAlignment(Qt::AlignHCenter);columnLayout->addWidget(entry.valueLabel);entry.muteButton=new QPushButton(column);entry.muteButton->setFocusPolicy(Qt::StrongFocus);entry.muteButton->installEventFilter(this);columnLayout->addWidget(entry.muteButton);layout->addWidget(column);void *source=entry.source;connect(entry.slider,&QSlider::valueChanged,this,[this,source](int value){if(syncing_)return;auto found=std::find_if(sources_.begin(),sources_.end(),[source](const VolumeEntry &candidate){return candidate.source==source;});if(found==sources_.end())return;int index=static_cast<int>(std::distance(sources_.begin(),found));api.source_set_volume(source,DbToVolume(value));UpdateVisual(index);});connect(entry.muteButton,&QPushButton::clicked,this,[this,source]{auto found=std::find_if(sources_.begin(),sources_.end(),[source](const VolumeEntry &candidate){return candidate.source==source;});if(found!=sources_.end())ToggleMute(static_cast<int>(std::distance(sources_.begin(),found)));});UpdateVisual(static_cast<int>(index));}
        layout->addStretch(1);scroll_->setWidget(panel);if(isVisible())QTimer::singleShot(0,this,[this,preferredIndex]{FocusIndex(preferredIndex);});
    }
    void RefreshFromObs(){
        std::vector<VolumeEntry> fresh=CurrentMixerEntries();if(!SameSources(fresh)){int focused=FocusedIndex();void *preferred=focused>=0?sources_[static_cast<size_t>(focused)].source:nullptr;ReplaceSources(std::move(fresh),preferred);return;}for(VolumeEntry &entry:fresh)api.source_release(entry.source);for(size_t index=0;index<sources_.size();++index)UpdateVisual(static_cast<int>(index));
    }
    QScrollArea *scroll_{};QDialogButtonBox *buttons_{};QTimer *timer_{};std::vector<VolumeEntry> sources_;bool syncing_{};
};

static QPointer<VolumeConsoleDialog> volumeConsoleWindow;

} // namespace

static std::string VolumeConsoleCommandLabel(){QByteArray utf8=VText(VOLUME_CONSOLE_COMMAND).toUtf8();return std::string(utf8.constData(),static_cast<size_t>(utf8.size()));}

static void VolumeConsoleHotkey(void*,hotkey_id,obs_hotkey*,bool pressed){
    if(!pressed||!obsMainWindow)return;if(volumeConsoleWindow){volumeConsoleWindow->raise();volumeConsoleWindow->activateWindow();return;}if(!MainInterfaceActive())return;QPointer<QWidget> returnFocus=QApplication::focusWidget();VolumeConsoleDialog dialog(obsMainWindow);volumeConsoleWindow=&dialog;dialog.exec();volumeConsoleWindow=nullptr;if(returnFocus&&returnFocus->isVisible()&&returnFocus->isEnabled()){returnFocus->setFocus(Qt::ShortcutFocusReason);returnFocus->activateWindow();}
}
