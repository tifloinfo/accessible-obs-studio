// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Tiflo.Info

static std::vector<QWidget*> InterfaceRegions(){
    std::vector<QWidget*> regions;if(!obsMainWindow)return regions;
    const char *preferred[]={"previewContainer","scenesDock","sourcesDock","mixerDock","transitionsDock","controlsDock"};
    for(const char *name:preferred){QWidget *widget=obsMainWindow->findChild<QWidget*>(name);if(widget&&widget->isVisible())regions.push_back(widget);}
    for(QDockWidget *dock:obsMainWindow->findChildren<QDockWidget*>())if(dock->isVisible()&&std::find(regions.begin(),regions.end(),dock)==regions.end())regions.push_back(dock);
    return regions;
}

static bool Contains(QWidget *region,QWidget *widget){return widget&&(widget==region||region->isAncestorOf(widget));}

static std::vector<QAbstractSlider*> MixerSliders(){
    std::vector<QAbstractSlider*> result;QWidget *dock=obsMainWindow?obsMainWindow->findChild<QWidget*>(QStringLiteral("mixerDock")):nullptr;if(!dock)return result;
    for(QAbstractSlider *slider:dock->findChildren<QAbstractSlider*>()){
        if(!slider->isVisible()||!slider->isEnabled())continue;
        const QString className=QString::fromLatin1(slider->metaObject()->className());if(!className.contains(QStringLiteral("VolumeSlider")))continue;
        result.push_back(slider);
    }
    std::sort(result.begin(),result.end(),[](QAbstractSlider *a,QAbstractSlider *b){QPoint ap=a->mapToGlobal(QPoint()),bp=b->mapToGlobal(QPoint());return ap.y()==bp.y()?ap.x()<bp.x():ap.y()<bp.y();});
    return result;
}

static constexpr const char *ALLOW_OBS_HOTKEY_CONTROL="AllowObsToManageHotkeysOutsideObs";
static constexpr const char *SAFE_HOTKEY_FOCUS_TYPE="DisableHotkeysOutOfFocus";

static bool AllowObsToManageHotkeysOutsideObs(){
    config *cfg=api.global_config?api.global_config():nullptr;if(!cfg||!api.config_has_user_value(cfg,"AccessibleStudio",ALLOW_OBS_HOTKEY_CONTROL))return false;const char *value=api.config_get_string(cfg,"AccessibleStudio",ALLOW_OBS_HOTKEY_CONTROL);return value&&(_stricmp(value,"true")==0||strcmp(value,"1")==0);
}

static void ApplyHotkeyFocusPolicy(){
    if(AllowObsToManageHotkeysOutsideObs()||!api.hotkey_enable_background_press)return;api.hotkey_enable_background_press(QApplication::applicationState()==Qt::ApplicationActive);
}

static void EnsureSafeHotkeyFocusDefault(){
    if(AllowObsToManageHotkeysOutsideObs())return;config *cfg=api.global_config?api.global_config():nullptr;if(!cfg)return;const char *value=api.config_get_string(cfg,"General","HotkeyFocusType");if(!value||_stricmp(value,SAFE_HOTKEY_FOCUS_TYPE)!=0){api.config_set_string(cfg,"General","HotkeyFocusType",SAFE_HOTKEY_FOCUS_TYPE);api.config_save_safe(cfg,"tmp",nullptr);}ApplyHotkeyFocusPolicy();
}

static bool SaveObsHotkeyManagementPreference(bool allow){
    config *cfg=api.global_config?api.global_config():nullptr;if(!cfg)return false;api.config_set_string(cfg,"AccessibleStudio",ALLOW_OBS_HOTKEY_CONTROL,allow?"true":"false");if(!allow)api.config_set_string(cfg,"General","HotkeyFocusType",SAFE_HOTKEY_FOCUS_TYPE);if(api.config_save_safe(cfg,"tmp",nullptr)!=0)return false;if(!allow)ApplyHotkeyFocusPolicy();return true;
}

static bool FocusRegion(QWidget *region){
    if(region->objectName()==QStringLiteral("previewContainer")){
        QWidget *preview=region->findChild<QWidget*>(QStringLiteral("preview"));
        if(preview){QString previewName=QString::fromWCharArray(Tr(UiText::PreviewName));if(preview->accessibleName()!=previewName)preview->setAccessibleName(previewName);if(preview->focusPolicy()==Qt::NoFocus)preview->setFocusPolicy(Qt::ClickFocus);if(!preview->hasFocus())preview->setFocus(Qt::ShortcutFocusReason);if(preview->hasFocus())return true;}
    }
    if(region->focusPolicy()!=Qt::NoFocus&&region->isEnabled()){region->setFocus(Qt::ShortcutFocusReason);if(region->hasFocus())return true;}
    for(QWidget *candidate:region->findChildren<QWidget*>())if(candidate->isVisible()&&candidate->isEnabled()&&candidate->focusPolicy()!=Qt::NoFocus){candidate->setFocus(Qt::ShortcutFocusReason);if(candidate->hasFocus())return true;}
    return false;
}

enum class ScreenReaderKind {None,Nvda,Jaws,Narrator,Multiple,Unknown};

static ScreenReaderKind DetectRunningScreenReader(){
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);if(snapshot==INVALID_HANDLE_VALUE)return ScreenReaderKind::Unknown;
    bool nvda=false,jaws=false,narrator=false;PROCESSENTRY32W process{};process.dwSize=sizeof(process);
    if(Process32FirstW(snapshot,&process))do{const wchar_t *name=process.szExeFile;if(_wcsicmp(name,L"nvda.exe")==0)nvda=true;else if(_wcsicmp(name,L"jfw.exe")==0||_wcsicmp(name,L"jfwUI.exe")==0)jaws=true;else if(_wcsicmp(name,L"Narrator.exe")==0)narrator=true;}while(Process32NextW(snapshot,&process));
    else{CloseHandle(snapshot);return ScreenReaderKind::Unknown;}CloseHandle(snapshot);
    const int count=static_cast<int>(nvda)+static_cast<int>(jaws)+static_cast<int>(narrator);if(count>1)return ScreenReaderKind::Multiple;if(nvda)return ScreenReaderKind::Nvda;if(jaws)return ScreenReaderKind::Jaws;if(narrator)return ScreenReaderKind::Narrator;return ScreenReaderKind::None;
}

static QString RegionAnnouncementName(QWidget *region){
    if(!region)return {};
    if(region->objectName()==QStringLiteral("previewContainer"))return QString::fromWCharArray(Tr(UiText::PreviewName));
    QString name;if(auto *dock=qobject_cast<QDockWidget*>(region))name=dock->windowTitle();if(name.isEmpty())name=region->accessibleName();if(name.isEmpty())name=region->windowTitle();return name.remove(u'&').trimmed();
}

static bool FocusRegionAndAnnounce(QWidget *region){
    if(!FocusRegion(region))return false;if(DetectRunningScreenReader()!=ScreenReaderKind::Nvda)return true;QString name=RegionAnnouncementName(region);if(!name.isEmpty()){QAccessibleAnnouncementEvent event(obsMainWindow,name);event.setPoliteness(QAccessible::AnnouncementPoliteness::Assertive);QAccessible::updateAccessibility(&event);}return true;
}

static bool MainInterfaceActive(){
    if(!obsMainWindow||QApplication::activeModalWidget()||QApplication::activePopupWidget())return false;
    HWND foreground=GetForegroundWindow(),obsHandle=api.main_hwnd?api.main_hwnd():nullptr;if(!obsHandle||GetAncestor(foreground,GA_ROOTOWNER)!=GetAncestor(obsHandle,GA_ROOTOWNER))return false;
    if(QApplication::activeWindow()==obsMainWindow)return true;
    QWidget *focus=QApplication::focusWidget();for(QWidget *region:InterfaceRegions())if(Contains(region,focus))return true;
    return false;
}

static QWidget *VisibleMediaControls(){
    if(!obsMainWindow)return nullptr;
    for(QWidget *controls:obsMainWindow->findChildren<QWidget*>(QStringLiteral("MediaControls")))if(controls->isVisible()&&controls->isEnabled())return controls;
    return nullptr;
}

static bool FocusVisibleMediaControls(){
    QWidget *controls=VisibleMediaControls();if(!controls)return false;
    struct MediaControlSpec{const char *objectName;LocalText name;};
    static constexpr MediaControlSpec specs[]={{"playPauseButton",LocalText::MediaPlayPause},{"stopButton",LocalText::MediaStop},{"previousButton",LocalText::MediaPrevious},{"nextButton",LocalText::MediaNext},{"slider",LocalText::MediaPosition}};
    QWidget *playPause=nullptr;for(const MediaControlSpec &spec:specs)if(QWidget *widget=controls->findChild<QWidget*>(QString::fromLatin1(spec.objectName))){widget->setAccessibleName(LText(spec.name));if(strcmp(spec.objectName,"playPauseButton")==0)playPause=widget;}
    if(playPause&&playPause->isVisible()&&playPause->isEnabled()){if(playPause->focusPolicy()==Qt::NoFocus)playPause->setFocusPolicy(Qt::StrongFocus);playPause->setFocus(Qt::ShortcutFocusReason);if(playPause->hasFocus())return true;}
    return FocusRegion(controls);
}

static bool CollectMediaSourceName(void *parameter,void *source){
    constexpr uint32_t CONTROLLABLE_MEDIA_FLAG=1u<<13;if(!source||(api.source_output_flags(source)&CONTROLLABLE_MEDIA_FLAG)==0)return true;
    const char *rawName=api.source_name(source);QString name=QString::fromUtf8(rawName?rawName:"");auto *names=static_cast<QStringList*>(parameter);if(!name.isEmpty()&&!names->contains(name))names->push_back(name);return true;
}

static QString SourceIndexName(const QModelIndex &index){
    QString name=index.data(Qt::AccessibleTextRole).toString();if(name.isEmpty())name=index.data(Qt::DisplayRole).toString();return name;
}

static bool SelectMediaSourceForControls(){
    QAbstractItemView *sources=obsMainWindow?obsMainWindow->findChild<QAbstractItemView*>(QStringLiteral("sources")):nullptr;if(!sources||!sources->model()||!sources->selectionModel())return false;
    QStringList mediaNames;api.enum_sources(CollectMediaSourceName,&mediaNames);if(mediaNames.empty())return false;
    std::vector<QModelIndex> candidates;QStringList candidateNames;
    for(int row=0;row<sources->model()->rowCount();++row){QModelIndex index=sources->model()->index(row,0);QString name=SourceIndexName(index);if(mediaNames.contains(name)){candidates.push_back(index);candidateNames.push_back(name);}}
    if(candidates.empty())return false;
    QModelIndex selected;
    QModelIndex current=sources->currentIndex();for(const QModelIndex &candidate:candidates)if(candidate==current){selected=candidate;break;}
    if(!selected.isValid()&&candidates.size()==1)selected=candidates.front();
    if(!selected.isValid()){bool accepted=false;QString name=QInputDialog::getItem(obsMainWindow,LText(LocalText::FocusMediaCommand),LText(LocalText::Source),candidateNames,0,false,&accepted);if(!accepted)return false;int choice=candidateNames.indexOf(name);if(choice<0)return false;selected=candidates[static_cast<size_t>(choice)];}
    sources->setCurrentIndex(selected);sources->selectionModel()->select(selected,QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);return true;
}

class MediaSeekEventFilter final:public QObject{
public:
    explicit MediaSeekEventFilter(QObject *parent):QObject(parent){seekClock_.start();}
protected:
    bool eventFilter(QObject *watched,QEvent *event) override{
        if(event->type()!=QEvent::ShortcutOverride&&event->type()!=QEvent::KeyPress)return QObject::eventFilter(watched,event);
        QWidget *target=qobject_cast<QWidget*>(watched);if(!target)target=QApplication::focusWidget();
        QWidget *controls=nullptr;for(QWidget *widget=target;widget;widget=widget->parentWidget())if(widget->objectName()==QStringLiteral("MediaControls")){controls=widget;break;}
        if(!controls||!controls->isVisible()||!controls->isEnabled())return QObject::eventFilter(watched,event);
        auto *keyEvent=static_cast<QKeyEvent*>(event);const Qt::KeyboardModifiers modifiers=keyEvent->modifiers()&~Qt::KeypadModifier;
        int seconds=0;
        if(modifiers==Qt::NoModifier&&keyEvent->key()==Qt::Key_Right)seconds=5;
        else if(modifiers==Qt::NoModifier&&keyEvent->key()==Qt::Key_Left)seconds=-5;
        else if(modifiers==Qt::ShiftModifier&&keyEvent->key()==Qt::Key_Right)seconds=60;
        else if(modifiers==Qt::ShiftModifier&&keyEvent->key()==Qt::Key_Left)seconds=-60;
        else if(modifiers==Qt::NoModifier&&keyEvent->key()==Qt::Key_PageUp)seconds=-300;
        else if(modifiers==Qt::NoModifier&&keyEvent->key()==Qt::Key_PageDown)seconds=300;
        if(seconds==0)return QObject::eventFilter(watched,event);
        event->accept();if(event->type()==QEvent::ShortcutOverride)return true;
        if(Seek(seconds))return true;
        const char *fallback=seconds>0?"MoveSliderFoward":"MoveSliderBackwards";
        return QMetaObject::invokeMethod(controls,fallback,Qt::DirectConnection,Q_ARG(int,std::abs(seconds)));
    }
private:
    struct MediaSourceLookup{QString name;void *source{};};
    static bool FindMediaSource(void *parameter,void *source){
        auto *lookup=static_cast<MediaSourceLookup*>(parameter);if(!source||lookup->source)return true;
        const char *rawName=api.source_name(source);if(QString::fromUtf8(rawName?rawName:"")!=lookup->name)return true;
        constexpr uint32_t CONTROLLABLE_MEDIA_FLAG=1u<<13;if((api.source_output_flags(source)&CONTROLLABLE_MEDIA_FLAG)==0)return true;
        lookup->source=api.source_get_ref(source);return lookup->source==nullptr;
    }
    bool Seek(int seconds){
        QAbstractItemView *sources=obsMainWindow?obsMainWindow->findChild<QAbstractItemView*>(QStringLiteral("sources")):nullptr;if(!sources||!sources->model())return false;
        QModelIndex currentIndex=sources->currentIndex();if(!currentIndex.isValid())return false;
        MediaSourceLookup lookup{SourceIndexName(currentIndex)};if(lookup.name.isEmpty())return false;
        api.enum_sources(FindMediaSource,&lookup);if(!lookup.source)return false;
        const int64_t current=std::max<int64_t>(0,api.source_media_time(lookup.source));const int64_t duration=api.source_media_duration(lookup.source);const qint64 now=seekClock_.elapsed();
        int64_t base=current;if(pending_&&lookup.name==pendingSource_&&now-lastSeekMs_<=750)base=seconds>0?std::max(current,pendingTargetMs_):std::min(current,pendingTargetMs_);
        int64_t target=base+static_cast<int64_t>(seconds)*1000;target=std::max<int64_t>(0,target);if(duration>0)target=std::min(target,duration);
        api.source_media_set_time(lookup.source,target);api.source_release(lookup.source);
        pending_=true;pendingSource_=lookup.name;pendingTargetMs_=target;lastSeekMs_=now;return true;
    }
    QElapsedTimer seekClock_;
    QString pendingSource_;
    int64_t pendingTargetMs_{};
    qint64 lastSeekMs_{};
    bool pending_{};
};
static QPointer<MediaSeekEventFilter> mediaSeekEventFilter;

static void CycleInterfaceArea(bool backwards){
    if(!MainInterfaceActive())return;auto regions=InterfaceRegions();if(regions.empty())return;
    QWidget *focus=QApplication::focusWidget();int current=-1;for(size_t i=0;i<regions.size();++i)if(Contains(regions[i],focus)){current=static_cast<int>(i);break;}
    const int count=static_cast<int>(regions.size());for(int step=1;step<=count;++step){int index=backwards?(current-step+count*2)%count:(current+step)%count;if(FocusRegionAndAnnounce(regions[static_cast<size_t>(index)]))return;}
}

static void NavigationHotkey(void *data,hotkey_id,obs_hotkey*,bool pressed){if(pressed&&PluginEventTarget()){bool backwards=data!=nullptr;QMetaObject::invokeMethod(PluginEventTarget(),[backwards]{CycleInterfaceArea(backwards);},Qt::QueuedConnection);}}

static constexpr std::array<const char*,6> DIRECT_AREA_WIDGETS={"previewContainer","scenesDock","sourcesDock","mixerDock","transitionsDock","controlsDock"};

static void DirectAreaHotkey(void *data,hotkey_id,obs_hotkey*,bool pressed){
    if(!pressed||!PluginEventTarget())return;const intptr_t encoded=reinterpret_cast<intptr_t>(data);QMetaObject::invokeMethod(PluginEventTarget(),[encoded]{if(!MainInterfaceActive()||encoded<1||encoded>static_cast<intptr_t>(DIRECT_AREA_WIDGETS.size()))return;QWidget *region=obsMainWindow->findChild<QWidget*>(DIRECT_AREA_WIDGETS[static_cast<size_t>(encoded-1)]);if(region&&region->isVisible())FocusRegionAndAnnounce(region);},Qt::QueuedConnection);
}

static QString NumberedSceneLabel(size_t index){
    static constexpr std::array<const char*,6> formats={".Switch to Scene %1",".Zu Szene %1 wechseln",".Переключиться на сцену %1",".Перемкнутися на сцену %1",".Passer à la scène %1",".Cambiar a la escena %1"};
    return QString::fromUtf8(formats[LanguageIndex()]).arg(index+1);
}

static void SwitchToNumberedScene(size_t index){
    if(!MainInterfaceActive())return;FrontendSourceList scenes{};api.frontend_scenes(&scenes);QString name;void *target=index<scenes.sources.num?scenes.sources.array[index]:nullptr;
    if(target){const char *raw=api.source_name(target);name=QString::fromUtf8(raw?raw:"");api.frontend_set_scene(target);}
    for(size_t i=0;i<scenes.sources.num;++i)if(scenes.sources.array[i])api.source_release(scenes.sources.array[i]);
    api.memory_free(scenes.sources.array);
    if(!target){MessageBeep(MB_ICONINFORMATION);return;}if(!name.isEmpty())AnnounceAccessibility(name,nullptr);
}

static void NumberedSceneHotkey(void *data,hotkey_id,obs_hotkey*,bool pressed){
    if(!pressed||!PluginEventTarget())return;intptr_t encoded=reinterpret_cast<intptr_t>(data);if(encoded<1||encoded>10)return;size_t index=static_cast<size_t>(encoded-1);QMetaObject::invokeMethod(PluginEventTarget(),[index]{SwitchToNumberedScene(index);},Qt::QueuedConnection);
}

static void FocusMediaControlsHotkey(void*,hotkey_id,obs_hotkey*,bool pressed){
    if(!pressed||!PluginEventTarget())return;QMetaObject::invokeMethod(PluginEventTarget(),[]{
        if(!MainInterfaceActive())return;if(FocusVisibleMediaControls())return;if(!SelectMediaSourceForControls()){MessageBeep(MB_ICONINFORMATION);return;}
        QTimer::singleShot(0,PluginEventTarget(),[]{if(!FocusVisibleMediaControls())MessageBeep(MB_ICONINFORMATION);});
    },Qt::QueuedConnection);
}

enum class SourceSelectionResult{Ready,Cancelled,Unavailable};
static QPersistentModelIndex pendingFitSource;

static bool CollectVideoSourceName(void *parameter,void *source){
    constexpr uint32_t VIDEO_FLAG=1u<<0;if(!source||(api.source_output_flags(source)&VIDEO_FLAG)==0)return true;
    const char *rawName=api.source_name(source);QString name=QString::fromUtf8(rawName?rawName:"");auto *names=static_cast<QStringList*>(parameter);if(!name.isEmpty()&&!names->contains(name))names->push_back(name);return true;
}

static SourceSelectionResult SelectSourceForAutomaticFix(){
    QAbstractItemView *sources=obsMainWindow?obsMainWindow->findChild<QAbstractItemView*>(QStringLiteral("sources")):nullptr;if(!sources||!sources->model())return SourceSelectionResult::Unavailable;
    QStringList videoNames;api.enum_sources(CollectVideoSourceName,&videoNames);if(videoNames.empty())return SourceSelectionResult::Unavailable;
    std::vector<QModelIndex> indexes;QStringList names;QModelIndex current=sources->currentIndex();int selectedRow=0;
    for(int row=0;row<sources->model()->rowCount();++row){QModelIndex index=sources->model()->index(row,0);QString name=SourceIndexName(index);if(name.isEmpty()||!videoNames.contains(name))continue;if(index==current)selectedRow=static_cast<int>(indexes.size());indexes.push_back(index);names.push_back(name);}
    if(indexes.empty())return SourceSelectionResult::Unavailable;
    QModelIndex selected;if(indexes.size()==1)selected=indexes.front();
    else{
        QDialog dialog(obsMainWindow);dialog.setWindowTitle(LText(LocalText::SelectSource));dialog.setModal(true);auto *layout=new QVBoxLayout(&dialog);auto *list=new QListWidget(&dialog);list->setAccessibleName(LText(LocalText::Source));list->setSelectionMode(QAbstractItemView::SingleSelection);list->addItems(names);layout->addWidget(list);auto *buttons=new QDialogButtonBox(&dialog);QPushButton *next=buttons->addButton(LText(LocalText::Ok),QDialogButtonBox::AcceptRole);buttons->addButton(LText(LocalText::Cancel),QDialogButtonBox::RejectRole);QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);QObject::connect(list,&QListWidget::itemActivated,[&](QListWidgetItem*){dialog.accept();});QObject::connect(list,&QListWidget::currentRowChanged,[next](int row){next->setEnabled(row>=0);});layout->addWidget(buttons);list->setCurrentRow(selectedRow);list->setFocus(Qt::OtherFocusReason);dialog.resize(520,420);if(dialog.exec()!=QDialog::Accepted)return SourceSelectionResult::Cancelled;int row=list->currentRow();if(row<0||row>=static_cast<int>(indexes.size()))return SourceSelectionResult::Unavailable;selected=indexes[static_cast<size_t>(row)];
    }
    sources->setCurrentIndex(selected);if(sources->selectionModel())sources->selectionModel()->select(selected,QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);return SourceSelectionResult::Ready;
}

static void RestorePendingFitSelection(){
    if(!pendingFitSource.isValid())return;QAbstractItemView *sources=obsMainWindow?obsMainWindow->findChild<QAbstractItemView*>(QStringLiteral("sources")):nullptr;if(!sources||!sources->selectionModel())return;sources->setCurrentIndex(pendingFitSource);sources->selectionModel()->select(pendingFitSource,QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
}

static void FinishFitQualityValidation(bool acceptable){
    if(acceptable){pendingFitSource=QPersistentModelIndex();QMessageBox::information(obsMainWindow,QStringLiteral("Accessible Studio"),LText(LocalText::FitQualityAccepted)+QStringLiteral("\n\n")+LText(LocalText::Undo));return;}
    QAction *undo=obsMainWindow?obsMainWindow->findChild<QAction*>(QStringLiteral("actionMainUndo")):nullptr;if(!undo||!undo->isEnabled()){pendingFitSource=QPersistentModelIndex();QMessageBox::warning(obsMainWindow,QStringLiteral("Accessible Studio"),LText(LocalText::FitUndoFailed));return;}undo->trigger();RestorePendingFitSelection();
    QAction *center=obsMainWindow->findChild<QAction*>(QStringLiteral("actionCenterToScreen"));if(!center||!center->isEnabled()){pendingFitSource=QPersistentModelIndex();QMessageBox::warning(obsMainWindow,QStringLiteral("Accessible Studio"),LText(LocalText::FitCenterFailed));return;}center->trigger();pendingFitSource=QPersistentModelIndex();QMessageBox::information(obsMainWindow,QStringLiteral("Accessible Studio"),LText(LocalText::FitQualityFallback)+QStringLiteral("\n\n")+LText(LocalText::Undo));
}

static void ShowSuggestedFixes(const std::vector<std::string> &allowed){
    if(!obsMainWindow)return;SourceSelectionResult sourceResult=SelectSourceForAutomaticFix();if(sourceResult==SourceSelectionResult::Cancelled)return;if(sourceResult==SourceSelectionResult::Unavailable){QMessageBox::information(obsMainWindow,QStringLiteral("Accessible Studio"),LText(LocalText::NoSource));return;}
    struct ActionSpec{const char *objectName;LocalText label;LocalText risk;};static constexpr ActionSpec specs[]={{"actionFitToScreen",LocalText::FitCanvas,LocalText::RiskMedium},{"actionCenterToScreen",LocalText::CenterFully,LocalText::RiskLow},{"actionHorizontalCenter",LocalText::CenterHorizontally,LocalText::RiskLow},{"actionVerticalCenter",LocalText::CenterVertically,LocalText::RiskLow},{"actionResetTransform",LocalText::ResetTransform,LocalText::RiskMedium},{"actionFlipHorizontal",LocalText::FlipHorizontally,LocalText::RiskMedium},{"actionFlipVertical",LocalText::FlipVertically,LocalText::RiskMedium},{"actionRotate90CW",LocalText::Rotate90Clockwise,LocalText::RiskMedium},{"actionRotate90CCW",LocalText::Rotate90Counterclockwise,LocalText::RiskMedium},{"actionRotate180",LocalText::Rotate180,LocalText::RiskMedium}};
    QDialog dialog(obsMainWindow);dialog.setWindowTitle(QStringLiteral("Accessible Studio - ")+CText(CanvasText::SuggestedFixes));dialog.setModal(true);auto *layout=new QVBoxLayout(&dialog);auto *list=new QListWidget(&dialog);list->setAccessibleName(LText(LocalText::SuggestedActions));list->setSelectionMode(QAbstractItemView::SingleSelection);int firstEnabled=-1;
    for(const auto &spec:specs){if(!allowed.empty()&&std::find(allowed.begin(),allowed.end(),spec.objectName)==allowed.end())continue;QAction *action=obsMainWindow->findChild<QAction*>(QString::fromLatin1(spec.objectName));auto *item=new QListWidgetItem(LText(LocalText::RiskFormat).arg(LText(spec.label),LText(spec.risk)),list);item->setData(Qt::UserRole,QString::fromLatin1(spec.objectName));if(action&&action->isEnabled()){if(firstEnabled<0)firstEnabled=list->row(item);}else{QString unavailable=LText(LocalText::UnavailableAction);item->setFlags(item->flags()&~Qt::ItemIsEnabled&~Qt::ItemIsSelectable);item->setToolTip(unavailable);item->setData(Qt::AccessibleDescriptionRole,unavailable);}}
    layout->addWidget(list);auto *explanation=new QLabel(LText(LocalText::NothingChanges),&dialog);explanation->setWordWrap(true);layout->addWidget(explanation);auto *buttons=new QDialogButtonBox(&dialog);QPushButton *apply=buttons->addButton(LText(LocalText::ApplySelected),QDialogButtonBox::AcceptRole);QPushButton *explain=buttons->addButton(LText(LocalText::Explain),QDialogButtonBox::HelpRole);buttons->addButton(LText(LocalText::Cancel),QDialogButtonBox::RejectRole);auto updateApply=[&]{QListWidgetItem *item=list->currentItem();apply->setEnabled(item&&item->flags().testFlag(Qt::ItemIsEnabled));};QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);QObject::connect(explain,&QPushButton::clicked,[&]{QMessageBox::information(&dialog,CText(CanvasText::SuggestedFixes),LText(LocalText::ExplainActions));});QObject::connect(list,&QListWidget::currentItemChanged,[&](QListWidgetItem*,QListWidgetItem*){updateApply();});QObject::connect(list,&QListWidget::itemActivated,[&](QListWidgetItem *item){if(item&&item->flags().testFlag(Qt::ItemIsEnabled))dialog.accept();});layout->addWidget(buttons);dialog.resize(620,520);if(firstEnabled>=0)list->setCurrentRow(firstEnabled);updateApply();list->setFocus(Qt::OtherFocusReason);if(dialog.exec()!=QDialog::Accepted)return;
    QListWidgetItem *item=list->currentItem();QString actionId=item?item->data(Qt::UserRole).toString():QString();QAction *action=item?obsMainWindow->findChild<QAction*>(actionId):nullptr;QString result;if(action&&action->isEnabled()){if(actionId==QStringLiteral("actionFitToScreen")){QAbstractItemView *sources=obsMainWindow->findChild<QAbstractItemView*>(QStringLiteral("sources"));pendingFitSource=sources?QPersistentModelIndex(sources->currentIndex()):QPersistentModelIndex();action->trigger();if(!StartFitQualityValidation())FinishFitQualityValidation(false);return;}action->trigger();result=LText(LocalText::Applied).arg(item->text())+QStringLiteral("\n\n")+LText(LocalText::Undo);}else result=item?LText(LocalText::Skipped).arg(item->text()):LText(LocalText::NoActionsApplied);QMessageBox::information(obsMainWindow,QStringLiteral("Accessible Studio"),result);
}

static constexpr const char *ACCESSIBLE_OBS_BUILD_ID="1.1.0-sound-doctor-test-20260730-5";
static constexpr const char *SHORTCUT_DEFAULTS_SCHEMA="4";

static void LoadSavedBinding(hotkey_id id,const char *name){
    config *cfg=api.profile_config?api.profile_config():nullptr;if(!cfg||!api.config_has_user_value(cfg,"Hotkeys",name)){api.load_bindings(id,nullptr,0);return;}const char *json=api.config_get_string(cfg,"Hotkeys",name);obs_data *data=json&&*json?api.data_create_json(json):nullptr;if(!data){api.load_bindings(id,nullptr,0);return;}obs_data_array *array=api.data_get_array(data,"bindings");if(array){api.hotkey_load(id,array);api.array_release(array);}else api.load_bindings(id,nullptr,0);api.data_release(data);
}

static void LoadAccessibilityBindings(){
    LoadSavedBinding(nextAreaHotkey,NEXT_AREA_NAME);LoadSavedBinding(previousAreaHotkey,PREVIOUS_AREA_NAME);for(size_t i=0;i<canvasHotkeys.size();++i)LoadSavedBinding(canvasHotkeys[i],CANVAS_HOTKEY_NAMES[i]);LoadSavedBinding(focusMediaHotkey,FOCUS_MEDIA_NAME);LoadSavedBinding(volumeConsoleHotkey,VOLUME_CONSOLE_NAME);LoadSavedBinding(openAccessibleObsHotkey,OPEN_ACCESSIBLE_OBS_NAME);LoadSavedBinding(audibleMeterHotkey,AUDIBLE_METER_NAME);LoadSavedBinding(soundDoctorHotkey,SOUND_DOCTOR_NAME);for(size_t i=0;i<directAreaHotkeys.size();++i)LoadSavedBinding(directAreaHotkeys[i],DIRECT_AREA_NAMES[i]);for(size_t i=0;i<numberedSceneHotkeys.size();++i)LoadSavedBinding(numberedSceneHotkeys[i],NUMBERED_SCENE_NAMES[i]);
}

struct HotkeyCollector{std::vector<Hotkey> values;};
static bool CollectHotkey(void *parameter,hotkey_id id,obs_hotkey *hotkey){auto *collector=static_cast<HotkeyCollector*>(parameter);Hotkey value;value.id=id;value.name=api.hk_name(hotkey)?api.hk_name(hotkey):"";value.description=api.hk_desc(hotkey)?api.hk_desc(hotkey):value.name;value.type=api.hk_type(hotkey);value.registerer=api.hk_registerer(hotkey);value.context=Narrow(TypeText(value.type));collector->values.push_back(std::move(value));return true;}
static bool CollectBinding(void *parameter,size_t,obs_hotkey_binding *binding){auto *collector=static_cast<HotkeyCollector*>(parameter);hotkey_id id=api.binding_id(binding);auto found=std::find_if(collector->values.begin(),collector->values.end(),[&](const Hotkey &hotkey){return hotkey.id==id;});if(found!=collector->values.end())found->bindings.push_back(api.binding_combo(binding));return true;}

struct DefaultShortcut{hotkey_id id;const char *name,*key;uint32_t modifiers;int virtualKey{};};
static std::vector<DefaultShortcut> AccessibilityDefaults(){
    std::vector<DefaultShortcut> defaults={{nextAreaHotkey,NEXT_AREA_NAME,"OBS_KEY_F6",0},{previousAreaHotkey,PREVIOUS_AREA_NAME,"OBS_KEY_F6",OBS_MOD_SHIFT},{canvasHotkeys[0],CANVAS_HOTKEY_NAMES[0],"OBS_KEY_F3",0},{canvasHotkeys[1],CANVAS_HOTKEY_NAMES[1],"OBS_KEY_F3",OBS_MOD_SHIFT},{canvasHotkeys[2],CANVAS_HOTKEY_NAMES[2],"OBS_KEY_F3",OBS_MOD_ALT},{canvasHotkeys[3],CANVAS_HOTKEY_NAMES[3],"OBS_KEY_F3",OBS_MOD_CONTROL},{canvasHotkeys[4],CANVAS_HOTKEY_NAMES[4],"OBS_KEY_F4",0},{focusMediaHotkey,FOCUS_MEDIA_NAME,"OBS_KEY_M",OBS_MOD_CONTROL},{volumeConsoleHotkey,VOLUME_CONSOLE_NAME,"OBS_KEY_QUOTELEFT",OBS_MOD_CONTROL,VK_OEM_3},{statusInformationHotkey,"accessible_studio.status_information","OBS_KEY_F2",OBS_MOD_ALT},{pauseRecordingHotkey,"accessible_studio.pause_resume_recording","OBS_KEY_F7",OBS_MOD_ALT},{audibleMeterHotkey,AUDIBLE_METER_NAME,"OBS_KEY_I",OBS_MOD_CONTROL},{soundDoctorHotkey,SOUND_DOCTOR_NAME,"OBS_KEY_D",OBS_MOD_CONTROL|OBS_MOD_SHIFT}};static constexpr std::array<const char*,6> directKeys={"OBS_KEY_0","OBS_KEY_1","OBS_KEY_2","OBS_KEY_3","OBS_KEY_4","OBS_KEY_5"};for(size_t i=0;i<directAreaHotkeys.size();++i)defaults.push_back({directAreaHotkeys[i],DIRECT_AREA_NAMES[i],directKeys[i],OBS_MOD_CONTROL});static constexpr std::array<const char*,10> sceneKeys={"OBS_KEY_1","OBS_KEY_2","OBS_KEY_3","OBS_KEY_4","OBS_KEY_5","OBS_KEY_6","OBS_KEY_7","OBS_KEY_8","OBS_KEY_9","OBS_KEY_0"};for(size_t i=0;i<numberedSceneHotkeys.size();++i)defaults.push_back({numberedSceneHotkeys[i],NUMBERED_SCENE_NAMES[i],sceneKeys[i],OBS_MOD_ALT});static constexpr std::array<std::pair<const char*,const char*>,6> obsDefaults={{{"OBSBasic.StartStreaming","OBS_KEY_F5"},{"OBSBasic.StopStreaming","OBS_KEY_F5"},{"OBSBasic.StartRecording","OBS_KEY_F7"},{"OBSBasic.StopRecording","OBS_KEY_F7"},{"OBSBasic.StartVirtualCam","OBS_KEY_F8"},{"OBSBasic.StopVirtualCam","OBS_KEY_F8"}}};for(const auto &[name,key]:obsDefaults)defaults.push_back({static_cast<hotkey_id>(-1),name,key,0});return defaults;
}

enum class ConflictPolicy{Keep,Replace};
struct ConflictChoice{ConflictPolicy policy{ConflictPolicy::Keep};bool suppress{};};
static ConflictChoice ShowConflictChoice(){
    QDialog dialog(obsMainWindow);dialog.setWindowTitle(CText(CanvasText::ConflictTitle));dialog.setWindowModality(Qt::ApplicationModal);auto *layout=new QVBoxLayout(&dialog);auto *message=new QLabel(CText(CanvasText::ConflictMessage),&dialog);message->setWordWrap(true);message->setAccessibleName(QStringLiteral(" "));message->setAccessibleDescription(QStringLiteral(" "));layout->addWidget(message);auto *replace=new QRadioButton(CText(CanvasText::ReplaceConflicts),&dialog);auto *keep=new QRadioButton(CText(CanvasText::KeepExisting),&dialog);keep->setChecked(true);keep->setAccessibleDescription(CText(CanvasText::ConflictMessage));layout->addWidget(replace);layout->addWidget(keep);auto *suppress=new QCheckBox(CText(CanvasText::DoNotAskBuild),&dialog);layout->addWidget(suppress);auto *buttons=new QDialogButtonBox(QDialogButtonBox::Apply|QDialogButtonBox::Cancel,&dialog);buttons->button(QDialogButtonBox::Apply)->setText(QString::fromWCharArray(Tr(UiText::Apply)));buttons->button(QDialogButtonBox::Cancel)->setText(LText(LocalText::Cancel));QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);layout->addWidget(buttons);QWidget::setTabOrder(keep,replace);QWidget::setTabOrder(replace,suppress);QWidget::setTabOrder(suppress,buttons->button(QDialogButtonBox::Apply));keep->setFocus(Qt::OtherFocusReason);dialog.resize(720,300);if(dialog.exec()!=QDialog::Accepted)return {};return {replace->isChecked()?ConflictPolicy::Replace:ConflictPolicy::Keep,suppress->isChecked()};
}

struct PlannedShortcut{size_t target{};key_combo combo{};std::vector<size_t> conflicts;};
static void ReviewKeyboardShortcutConflicts(){
    config *profile=api.profile_config?api.profile_config():nullptr;if(!profile)return;LoadAccessibilityBindings();const char *reviewed=api.config_get_string(profile,"AccessibleStudio","ShortcutReviewBuild"),*schema=api.config_get_string(profile,"AccessibleStudio","ShortcutDefaultsSchema");bool initializeDefaults=!schema||strcmp(schema,SHORTCUT_DEFAULTS_SCHEMA)!=0;if(reviewed&&strcmp(reviewed,ACCESSIBLE_OBS_BUILD_ID)==0&&!initializeDefaults)return;HotkeyCollector collector;api.enum_hotkeys(CollectHotkey,&collector);api.enum_bindings(CollectBinding,&collector);auto defaults=AccessibilityDefaults();std::vector<PlannedShortcut> plans;
    for(DefaultShortcut &spec:defaults){size_t target=collector.values.size();for(size_t i=0;i<collector.values.size();++i)if((spec.id!=static_cast<hotkey_id>(-1)&&collector.values[i].id==spec.id)||collector.values[i].name==spec.name){target=i;spec.id=collector.values[i].id;break;}if(target==collector.values.size())continue;bool repairEmpty=initializeDefaults&&collector.values[target].bindings.empty();if(api.config_has_user_value(profile,"Hotkeys",spec.name)&&!repairEmpty)continue;int key=spec.virtualKey&&api.key_from_virtual_key?api.key_from_virtual_key(spec.virtualKey):api.key_from_name(spec.key);if(key<=0)continue;plans.push_back({target,{spec.modifiers,key},{}});}
    for(PlannedShortcut &plan:plans)for(size_t index=0;index<collector.values.size();++index){if(index==plan.target)continue;bool intentionalTarget=std::any_of(plans.begin(),plans.end(),[&](const PlannedShortcut &other){return other.target==index&&CombosEqual(other.combo,plan.combo);});if(intentionalTarget)continue;if(std::any_of(collector.values[index].bindings.begin(),collector.values[index].bindings.end(),[&](key_combo binding){return CombosEqual(binding,plan.combo);}))plan.conflicts.push_back(index);}
    bool hasConflicts=std::any_of(plans.begin(),plans.end(),[](const PlannedShortcut &plan){return !plan.conflicts.empty();});ConflictChoice choice;config *global=api.global_config?api.global_config():nullptr;const char *suppressed=global?api.config_get_string(global,"AccessibleStudio","ShortcutPromptSuppressedBuild"):nullptr;if(hasConflicts&&suppressed&&strcmp(suppressed,ACCESSIBLE_OBS_BUILD_ID)==0){const char *policy=api.config_get_string(global,"AccessibleStudio","ShortcutConflictPolicy");choice.policy=policy&&strcmp(policy,"replace")==0?ConflictPolicy::Replace:ConflictPolicy::Keep;choice.suppress=true;}else if(hasConflicts)choice=ShowConflictChoice();
    std::vector<Hotkey> originals=collector.values;std::vector<size_t> touched;auto touch=[&](size_t index){if(std::find(touched.begin(),touched.end(),index)==touched.end())touched.push_back(index);};if(choice.policy==ConflictPolicy::Replace)for(const PlannedShortcut &plan:plans)for(size_t index:plan.conflicts){touch(index);auto &bindings=collector.values[index].bindings;bindings.erase(std::remove_if(bindings.begin(),bindings.end(),[&](key_combo binding){return CombosEqual(binding,plan.combo);}),bindings.end());}
    for(const PlannedShortcut &plan:plans){if(!plan.conflicts.empty()&&choice.policy==ConflictPolicy::Keep)continue;touch(plan.target);collector.values[plan.target].bindings={plan.combo};}
    bool configurationChanged=false,ok=true;for(size_t index:touched)if(!Persist(collector.values[index],configurationChanged)){ok=false;break;}if(ok&&configurationChanged)ok=api.config_save_safe(profile,"tmp",nullptr)==0;if(ok&&api.frontend_save)api.frontend_save();if(!ok){bool rollbackConfiguration=false;for(size_t index:touched)Persist(originals[index],rollbackConfiguration);if(rollbackConfiguration)api.config_save_safe(profile,"tmp",nullptr);if(api.frontend_save)api.frontend_save();QMessageBox::critical(obsMainWindow,QStringLiteral("Accessible Studio"),CText(CanvasText::ConflictSaveFailed));return;}
    api.config_set_string(profile,"AccessibleStudio","ShortcutDefaultsSchema",SHORTCUT_DEFAULTS_SCHEMA);api.config_set_string(profile,"AccessibleStudio","ShortcutReviewBuild",ACCESSIBLE_OBS_BUILD_ID);api.config_save_safe(profile,"tmp",nullptr);if(choice.suppress&&global){api.config_set_string(global,"AccessibleStudio","ShortcutPromptSuppressedBuild",ACCESSIBLE_OBS_BUILD_ID);api.config_set_string(global,"AccessibleStudio","ShortcutConflictPolicy",choice.policy==ConflictPolicy::Replace?"replace":"keep");api.config_save_safe(global,"tmp",nullptr);}
}

static bool profileReviewQueued{};
static void QueueProfileReview(){if(profileReviewQueued||!PluginEventTarget())return;profileReviewQueued=true;QMetaObject::invokeMethod(PluginEventTarget(),[]{profileReviewQueued=false;EnsureSafeHotkeyFocusDefault();ReviewKeyboardShortcutConflicts();},Qt::QueuedConnection);}
static void FrontendEvent(int event,void*){constexpr int PROFILE_CHANGED=15,FINISHED_LOADING=26;if(event==PROFILE_CHANGED||event==FINISHED_LOADING)QueueProfileReview();HandleAccessibilityFrontendEvent(event);HandleAudibleMeterFrontendEvent(event);HandleSoundDoctorFrontendEvent(event);}
