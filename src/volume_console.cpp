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
    int value{-100};
    bool muted{};
    QPointer<QWidget> column;
    QPointer<QLabel> nameLabel;
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
    void *reference=api.source_get_ref(source);if(!reference)return true;const char *rawName=api.source_name(reference);QString name=QString::fromUtf8(rawName?rawName:"");if(name.isEmpty())name=LText(LocalText::AudioSource);entries->push_back(VolumeEntry{reference,name});return true;
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
        setWindowTitle(VText(VOLUME_CONSOLE_TITLE));setAccessibleDescription(VText(SLIDER_INSTRUCTIONS));setWindowModality(Qt::ApplicationModal);setModal(true);resize(800,460);setMinimumSize(460,360);
        auto *outer=new QVBoxLayout(this);scroll_=new QScrollArea(this);scroll_->setWidgetResizable(true);scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);scroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);scroll_->setFocusPolicy(Qt::NoFocus);outer->addWidget(scroll_);
        panel_=new QWidget(scroll_);sourceLayout_=new QHBoxLayout(panel_);sourceLayout_->setAlignment(Qt::AlignLeft);emptyMessage_=new QLabel(VText(NO_AUDIO_SOURCES),panel_);emptyMessage_->setWordWrap(true);sourceLayout_->addWidget(emptyMessage_);sourceLayout_->addStretch(1);scroll_->setWidget(panel_);
        buttons_=new QDialogButtonBox(QDialogButtonBox::Close,this);if(QPushButton *close=buttons_->button(QDialogButtonBox::Close))close->setText(QString::fromWCharArray(Tr(UiText::Close)));connect(buttons_,&QDialogButtonBox::rejected,this,&QDialog::reject);connect(buttons_,&QDialogButtonBox::accepted,this,&QDialog::accept);outer->addWidget(buttons_);
        InitializeSources(CurrentMixerEntries());if(!sources_.empty())FocusIndex(0);else if(QPushButton *close=buttons_->button(QDialogButtonBox::Close))close->setFocus(Qt::OtherFocusReason);
    }
    ~VolumeConsoleDialog() override{ReleaseSources();}
protected:
    bool eventFilter(QObject *watched,QEvent *event) override{
        if(event->type()!=QEvent::KeyPress)return QDialog::eventFilter(watched,event);auto *keyEvent=static_cast<QKeyEvent*>(event);int index=EntryIndex(watched);if(index<0)return QDialog::eventFilter(watched,event);
        Qt::KeyboardModifiers modifiers=keyEvent->modifiers()&~Qt::KeypadModifier;if(modifiers==Qt::NoModifier){
            if(keyEvent->key()==Qt::Key_Left){FocusIndex(index-1);return true;}if(keyEvent->key()==Qt::Key_Right){FocusIndex(index+1);return true;}
            if(keyEvent->key()==Qt::Key_Home&&watched==sources_[static_cast<size_t>(index)].slider){QSlider *slider=sources_[static_cast<size_t>(index)].slider;if(slider->value()!=0)slider->setValue(0);else Announce(index);return true;}
            if(keyEvent->key()==Qt::Key_Space&&watched==sources_[static_cast<size_t>(index)].slider){ToggleMute(index,true);return true;}
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
    QString Announcement(int index) const{const VolumeEntry &entry=sources_[static_cast<size_t>(index)];return QStringLiteral("%1. %2, %3 %4, %5.").arg(index+1).arg(entry.name,VText(VOLUME_TEXT),DbValueText(entry.value),VText(entry.muted?MUTED_TEXT:UNMUTED_TEXT));}
    void Announce(int index){if(index<0||index>=static_cast<int>(sources_.size()))return;QAccessibleAnnouncementEvent announcement(this,Announcement(index));announcement.setPoliteness(QAccessible::AnnouncementPoliteness::Assertive);QAccessible::updateAccessibility(&announcement);}
    void UpdateValueVisual(VolumeEntry &entry){QString valueText=DbValueText(entry.value);if(entry.valueLabel&&entry.valueLabel->text()!=valueText)entry.valueLabel->setText(valueText);}
    void UpdateMuteVisual(VolumeEntry &entry){if(!entry.muteButton)return;QString buttonText=VText(entry.muted?UNMUTE_TEXT:MUTE_TEXT);if(entry.muteButton->text()!=buttonText)entry.muteButton->setText(buttonText);QString buttonName=QStringLiteral("%1: %2").arg(entry.name,buttonText);if(entry.muteButton->accessibleName()!=buttonName)entry.muteButton->setAccessibleName(buttonName);}
    void ToggleMute(int index,bool announce){if(index<0||index>=static_cast<int>(sources_.size()))return;VolumeEntry &entry=sources_[static_cast<size_t>(index)];entry.muted=!entry.muted;api.source_set_muted(entry.source,entry.muted);UpdateMuteVisual(entry);if(announce)Announce(index);}
    void FocusIndex(int index){if(index<0||index>=static_cast<int>(sources_.size())){QApplication::beep();return;}QSlider *slider=sources_[static_cast<size_t>(index)].slider;if(!slider)return;scroll_->ensureWidgetVisible(slider,30,30);if(QApplication::focusWidget()!=slider)slider->setFocus(Qt::ShortcutFocusReason);}
    void ReleaseSources(){for(VolumeEntry &entry:sources_)if(entry.source)api.source_release(entry.source);sources_.clear();}
    void CreateControls(VolumeEntry &entry,int index){
        entry.column=new QWidget(panel_);entry.column->setMinimumWidth(145);auto *columnLayout=new QVBoxLayout(entry.column);entry.nameLabel=new QLabel(entry.column);entry.nameLabel->setAlignment(Qt::AlignHCenter);entry.nameLabel->setWordWrap(true);columnLayout->addWidget(entry.nameLabel);entry.slider=new QSlider(Qt::Vertical,entry.column);entry.slider->setRange(-100,0);entry.slider->setSingleStep(1);entry.slider->setPageStep(5);entry.slider->setMinimumHeight(230);entry.slider->setFocusPolicy(Qt::StrongFocus);entry.slider->installEventFilter(this);columnLayout->addWidget(entry.slider,1,Qt::AlignHCenter);entry.valueLabel=new QLabel(entry.column);entry.valueLabel->setAlignment(Qt::AlignHCenter);entry.valueLabel->setAccessibleName(QStringLiteral(" "));entry.valueLabel->setAccessibleDescription(QStringLiteral(" "));columnLayout->addWidget(entry.valueLabel);entry.muteButton=new QPushButton(entry.column);entry.muteButton->setFocusPolicy(Qt::StrongFocus);entry.muteButton->installEventFilter(this);columnLayout->addWidget(entry.muteButton);void *source=entry.source;
        entry.value=VolumeToDb(api.source_get_volume(source));entry.muted=api.source_muted(source);entry.slider->setValue(entry.value);entry.nameLabel->setText(QStringLiteral("%1. %2").arg(index+1).arg(entry.name));entry.slider->setAccessibleName(QStringLiteral("%1. %2. %3.").arg(index+1).arg(entry.name,VText(VOLUME_TEXT)));entry.slider->setAccessibleDescription(QString());UpdateValueVisual(entry);UpdateMuteVisual(entry);
        connect(entry.slider,&QSlider::valueChanged,this,[this,source](int value){auto found=std::find_if(sources_.begin(),sources_.end(),[source](const VolumeEntry &candidate){return candidate.source==source;});if(found==sources_.end())return;found->value=value;api.source_set_volume(source,DbToVolume(value));UpdateValueVisual(*found);});
        connect(entry.muteButton,&QPushButton::clicked,this,[this,source]{auto found=std::find_if(sources_.begin(),sources_.end(),[source](const VolumeEntry &candidate){return candidate.source==source;});if(found!=sources_.end())ToggleMute(static_cast<int>(std::distance(sources_.begin(),found)),false);});
    }
    void InitializeSources(std::vector<VolumeEntry> entries){sources_=std::move(entries);emptyMessage_->setVisible(sources_.empty());for(size_t index=0;index<sources_.size();++index){CreateControls(sources_[index],static_cast<int>(index));sourceLayout_->insertWidget(static_cast<int>(index),sources_[index].column);}}
    QScrollArea *scroll_{};QWidget *panel_{};QHBoxLayout *sourceLayout_{};QLabel *emptyMessage_{};QDialogButtonBox *buttons_{};std::vector<VolumeEntry> sources_;
};

static QPointer<VolumeConsoleDialog> volumeConsoleWindow;

} // namespace

static std::string VolumeConsoleCommandLabel(){QByteArray utf8=VText(VOLUME_CONSOLE_COMMAND).toUtf8();return std::string(utf8.constData(),static_cast<size_t>(utf8.size()));}

static void VolumeConsoleHotkey(void*,hotkey_id,obs_hotkey*,bool pressed){
    if(!pressed||!obsMainWindow)return;QMetaObject::invokeMethod(obsMainWindow,[]{if(volumeConsoleWindow){if(volumeConsoleWindow->isMinimized())volumeConsoleWindow->showNormal();else if(!volumeConsoleWindow->isVisible())volumeConsoleWindow->show();if(QApplication::activeWindow()!=volumeConsoleWindow){volumeConsoleWindow->raise();volumeConsoleWindow->activateWindow();}return;}if(!MainInterfaceActive())return;QPointer<QWidget> returnFocus=QApplication::focusWidget();VolumeConsoleDialog dialog(obsMainWindow);volumeConsoleWindow=&dialog;dialog.exec();volumeConsoleWindow=nullptr;if(returnFocus&&returnFocus->isVisible()&&returnFocus->isEnabled()&&QApplication::focusWidget()!=returnFocus){if(QWidget *window=returnFocus->window();window&&QApplication::activeWindow()!=window)window->activateWindow();returnFocus->setFocus(Qt::ShortcutFocusReason);}},Qt::QueuedConnection);
}
