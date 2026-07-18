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
    config *cfg=api.global_config?api.global_config():nullptr;if(!cfg||!api.config_has_user_value(cfg,"AccessibleOBSStudio",ALLOW_OBS_HOTKEY_CONTROL))return false;const char *value=api.config_get_string(cfg,"AccessibleOBSStudio",ALLOW_OBS_HOTKEY_CONTROL);return value&&(_stricmp(value,"true")==0||strcmp(value,"1")==0);
}

static void ApplyHotkeyFocusPolicy(){
    if(AllowObsToManageHotkeysOutsideObs()||!api.hotkey_enable_background_press)return;api.hotkey_enable_background_press(QApplication::applicationState()==Qt::ApplicationActive);
}

static void EnsureSafeHotkeyFocusDefault(){
    if(AllowObsToManageHotkeysOutsideObs())return;config *cfg=api.global_config?api.global_config():nullptr;if(!cfg)return;const char *value=api.config_get_string(cfg,"General","HotkeyFocusType");if(!value||_stricmp(value,SAFE_HOTKEY_FOCUS_TYPE)!=0){api.config_set_string(cfg,"General","HotkeyFocusType",SAFE_HOTKEY_FOCUS_TYPE);api.config_save_safe(cfg,"tmp",nullptr);}ApplyHotkeyFocusPolicy();
}

static bool SaveObsHotkeyManagementPreference(bool allow){
    config *cfg=api.global_config?api.global_config():nullptr;if(!cfg)return false;api.config_set_string(cfg,"AccessibleOBSStudio",ALLOW_OBS_HOTKEY_CONTROL,allow?"true":"false");if(!allow)api.config_set_string(cfg,"General","HotkeyFocusType",SAFE_HOTKEY_FOCUS_TYPE);if(api.config_save_safe(cfg,"tmp",nullptr)!=0)return false;if(!allow)ApplyHotkeyFocusPolicy();return true;
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

static bool MainInterfaceActive(){
    if(!obsMainWindow||QApplication::activeModalWidget()||QApplication::activePopupWidget())return false;
    HWND foreground=GetForegroundWindow(),obsHandle=api.main_hwnd?api.main_hwnd():nullptr;if(!obsHandle||GetAncestor(foreground,GA_ROOTOWNER)!=GetAncestor(obsHandle,GA_ROOTOWNER))return false;
    if(QApplication::activeWindow()==obsMainWindow)return true;
    QWidget *focus=QApplication::focusWidget();for(QWidget *region:InterfaceRegions())if(Contains(region,focus))return true;
    return false;
}

static void CycleInterfaceArea(bool backwards){
    if(!MainInterfaceActive())return;auto regions=InterfaceRegions();if(regions.empty())return;
    QWidget *focus=QApplication::focusWidget();int current=-1;for(size_t i=0;i<regions.size();++i)if(Contains(regions[i],focus)){current=static_cast<int>(i);break;}
    const int count=static_cast<int>(regions.size());for(int step=1;step<=count;++step){int index=backwards?(current-step+count*2)%count:(current+step)%count;if(FocusRegion(regions[static_cast<size_t>(index)]))return;}
}

static void NavigationHotkey(void *data,hotkey_id,obs_hotkey*,bool pressed){if(pressed&&obsMainWindow){bool backwards=data!=nullptr;QMetaObject::invokeMethod(obsMainWindow,[backwards]{CycleInterfaceArea(backwards);},Qt::QueuedConnection);}}

static constexpr std::array<const char*,6> DIRECT_AREA_WIDGETS={"previewContainer","scenesDock","sourcesDock","mixerDock","transitionsDock","controlsDock"};

static void DirectAreaHotkey(void *data,hotkey_id,obs_hotkey*,bool pressed){
    if(!pressed||!obsMainWindow)return;const intptr_t encoded=reinterpret_cast<intptr_t>(data);QMetaObject::invokeMethod(obsMainWindow,[encoded]{if(!MainInterfaceActive()||encoded<1||encoded>static_cast<intptr_t>(DIRECT_AREA_WIDGETS.size()))return;QWidget *region=obsMainWindow->findChild<QWidget*>(DIRECT_AREA_WIDGETS[static_cast<size_t>(encoded-1)]);if(region&&region->isVisible())FocusRegion(region);},Qt::QueuedConnection);
}

static void FocusMediaControlsHotkey(void*,hotkey_id,obs_hotkey*,bool pressed){
    if(!pressed||!obsMainWindow)return;QMetaObject::invokeMethod(obsMainWindow,[]{
        if(!MainInterfaceActive())return;QWidget *controls=obsMainWindow->findChild<QWidget*>(QStringLiteral("MediaControls"));if(!controls||!controls->isVisible()){MessageBeep(MB_ICONINFORMATION);return;}
        struct MediaControlSpec{const char *objectName;LocalText name;};
        static constexpr MediaControlSpec specs[]={{"playPauseButton",LocalText::MediaPlayPause},{"stopButton",LocalText::MediaStop},{"previousButton",LocalText::MediaPrevious},{"nextButton",LocalText::MediaNext},{"slider",LocalText::MediaPosition}};
        QWidget *playPause=nullptr;for(const MediaControlSpec &spec:specs)if(QWidget *widget=controls->findChild<QWidget*>(QString::fromLatin1(spec.objectName))){widget->setAccessibleName(LText(spec.name));if(strcmp(spec.objectName,"playPauseButton")==0)playPause=widget;}
        if(playPause&&playPause->isVisible()&&playPause->isEnabled()){if(playPause->focusPolicy()==Qt::NoFocus)playPause->setFocusPolicy(Qt::StrongFocus);playPause->setFocus(Qt::ShortcutFocusReason);if(playPause->hasFocus())return;}
        if(!FocusRegion(controls))MessageBeep(MB_ICONINFORMATION);
    },Qt::QueuedConnection);
}

enum class SourceSelectionResult{Ready,Cancelled,Unavailable};

static SourceSelectionResult EnsureSourceSelection(){
    QAbstractItemView *sources=obsMainWindow?obsMainWindow->findChild<QAbstractItemView*>(QStringLiteral("sources")):nullptr;if(!sources||!sources->model())return SourceSelectionResult::Unavailable;
    if(sources->selectionModel()&&!sources->selectionModel()->selectedRows().empty())return SourceSelectionResult::Ready;
    QStringList names;for(int row=0;row<sources->model()->rowCount();++row)names<<sources->model()->index(row,0).data(Qt::DisplayRole).toString();if(names.empty())return SourceSelectionResult::Unavailable;
    bool accepted=false;QString selected=QInputDialog::getItem(obsMainWindow,LText(LocalText::SelectSource),LText(LocalText::Source),names,0,false,&accepted);if(!accepted)return SourceSelectionResult::Cancelled;
    int row=names.indexOf(selected);if(row<0)return SourceSelectionResult::Unavailable;QModelIndex index=sources->model()->index(row,0);sources->setCurrentIndex(index);if(sources->selectionModel())sources->selectionModel()->select(index,QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);return SourceSelectionResult::Ready;
}

static void ShowSuggestedFixes(const std::vector<std::string> &allowed){
    if(!obsMainWindow)return;HWND foreground=GetForegroundWindow();bool restoreDescription=descriptionWindow&&foreground&&GetAncestor(foreground,GA_ROOT)==descriptionWindow;auto restoreFocus=[&]{if(restoreDescription&&descriptionWindow&&IsWindowVisible(descriptionWindow)&&descriptionController&&ActivateKeyboardWindow(descriptionWindow))descriptionController->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);};SourceSelectionResult sourceResult=EnsureSourceSelection();if(sourceResult==SourceSelectionResult::Cancelled){restoreFocus();return;}if(sourceResult==SourceSelectionResult::Unavailable){QMessageBox::information(obsMainWindow,QStringLiteral("Accessible OBS Studio"),LText(LocalText::NoSource));restoreFocus();return;}
    struct ActionSpec{const char *objectName;LocalText label;LocalText risk;};static constexpr ActionSpec specs[]={{"actionCenterToScreen",LocalText::CenterFully,LocalText::RiskLow},{"actionHorizontalCenter",LocalText::CenterHorizontally,LocalText::RiskLow},{"actionVerticalCenter",LocalText::CenterVertically,LocalText::RiskLow},{"actionFitToScreen",LocalText::FitCanvas,LocalText::RiskMedium},{"actionResetTransform",LocalText::ResetTransform,LocalText::RiskMedium},{"actionFlipHorizontal",LocalText::FlipHorizontally,LocalText::RiskMedium},{"actionFlipVertical",LocalText::FlipVertically,LocalText::RiskMedium},{"actionRotate90CW",LocalText::Rotate90Clockwise,LocalText::RiskMedium},{"actionRotate90CCW",LocalText::Rotate90Counterclockwise,LocalText::RiskMedium},{"actionRotate180",LocalText::Rotate180,LocalText::RiskMedium}};
    QDialog dialog(obsMainWindow);dialog.setWindowTitle(QStringLiteral("Accessible OBS Studio - ")+CText(CanvasText::SuggestedFixes));dialog.setModal(true);auto *layout=new QVBoxLayout(&dialog);bool studio=api.studio_mode_active&&api.studio_mode_active();auto *state=new QLabel(LText(studio?LocalText::StudioActive:LocalText::StudioInactive),&dialog);state->setWordWrap(true);layout->addWidget(state);auto *list=new QListWidget(&dialog);list->setAccessibleName(CText(CanvasText::SuggestedFixes));
    for(const auto &spec:specs){if(!allowed.empty()&&std::find(allowed.begin(),allowed.end(),spec.objectName)==allowed.end())continue;QAction *action=obsMainWindow->findChild<QAction*>(QString::fromLatin1(spec.objectName));auto *item=new QListWidgetItem(LText(LocalText::RiskFormat).arg(LText(spec.label),LText(spec.risk)),list);item->setData(Qt::UserRole,QString::fromLatin1(spec.objectName));item->setFlags(item->flags()|Qt::ItemIsUserCheckable);item->setCheckState(Qt::Unchecked);if(!action||!action->isEnabled()){QString unavailable=LText(LocalText::UnavailableAction);item->setFlags(item->flags()&~Qt::ItemIsEnabled);item->setToolTip(unavailable);item->setData(Qt::AccessibleDescriptionRole,unavailable);}}
    layout->addWidget(list);auto *explanation=new QLabel(LText(LocalText::NothingChanges),&dialog);explanation->setWordWrap(true);layout->addWidget(explanation);auto *buttons=new QDialogButtonBox(&dialog);QPushButton *apply=buttons->addButton(LText(LocalText::ApplySelected),QDialogButtonBox::AcceptRole);QPushButton *explain=buttons->addButton(LText(LocalText::Explain),QDialogButtonBox::HelpRole);QPushButton *cancel=buttons->addButton(LText(LocalText::Cancel),QDialogButtonBox::RejectRole);apply->setEnabled(false);auto updateApply=[&]{bool checked=false;for(int i=0;i<list->count();++i)if(list->item(i)->flags().testFlag(Qt::ItemIsEnabled)&&list->item(i)->checkState()==Qt::Checked){checked=true;break;}apply->setEnabled(checked);};QObject::connect(list,&QListWidget::itemChanged,[&](QListWidgetItem*){updateApply();});QObject::connect(apply,&QPushButton::clicked,&dialog,&QDialog::accept);QObject::connect(cancel,&QPushButton::clicked,&dialog,&QDialog::reject);QObject::connect(explain,&QPushButton::clicked,[&]{QMessageBox::information(&dialog,CText(CanvasText::SuggestedFixes),LText(LocalText::ExplainActions));});layout->addWidget(buttons);dialog.resize(620,520);if(restoreDescription)SetWindowLongPtr(reinterpret_cast<HWND>(dialog.winId()),GWLP_HWNDPARENT,reinterpret_cast<LONG_PTR>(descriptionWindow));if(list->count()>0)list->setCurrentRow(0);list->setFocus(Qt::OtherFocusReason);if(dialog.exec()!=QDialog::Accepted){restoreFocus();return;}
    QStringList applied,skipped;for(int i=0;i<list->count();++i){QListWidgetItem *item=list->item(i);if(item->checkState()!=Qt::Checked)continue;QAction *action=obsMainWindow->findChild<QAction*>(item->data(Qt::UserRole).toString());if(action&&action->isEnabled()){action->trigger();applied<<item->text();}else skipped<<item->text();}QString result=applied.isEmpty()?LText(LocalText::NoActionsApplied):LText(LocalText::Applied).arg(applied.join(QStringLiteral("\n")))+QStringLiteral("\n\n")+LText(LocalText::Undo);if(!skipped.empty())result+=QStringLiteral("\n\n")+LText(LocalText::Skipped).arg(skipped.join(QStringLiteral("\n")));QMessageBox::information(obsMainWindow,QStringLiteral("Accessible OBS Studio"),result);restoreFocus();
}

static constexpr const char *ACCESSIBLE_OBS_BUILD_ID="1.0.1-build-20260718-1";

static void LoadSavedBinding(hotkey_id id,const char *name){
    config *cfg=api.profile_config?api.profile_config():nullptr;if(!cfg||!api.config_has_user_value(cfg,"Hotkeys",name)){api.load_bindings(id,nullptr,0);return;}const char *json=api.config_get_string(cfg,"Hotkeys",name);obs_data *data=json&&*json?api.data_create_json(json):nullptr;if(!data){api.load_bindings(id,nullptr,0);return;}obs_data_array *array=api.data_get_array(data,"bindings");if(array){api.hotkey_load(id,array);api.array_release(array);}else api.load_bindings(id,nullptr,0);api.data_release(data);
}

static void LoadAccessibilityBindings(){
    LoadSavedBinding(nextAreaHotkey,NEXT_AREA_NAME);LoadSavedBinding(previousAreaHotkey,PREVIOUS_AREA_NAME);for(size_t i=0;i<canvasHotkeys.size();++i)LoadSavedBinding(canvasHotkeys[i],CANVAS_HOTKEY_NAMES[i]);LoadSavedBinding(focusMediaHotkey,FOCUS_MEDIA_NAME);LoadSavedBinding(volumeConsoleHotkey,VOLUME_CONSOLE_NAME);LoadSavedBinding(openAccessibleObsHotkey,OPEN_ACCESSIBLE_OBS_NAME);for(size_t i=0;i<directAreaHotkeys.size();++i)LoadSavedBinding(directAreaHotkeys[i],DIRECT_AREA_NAMES[i]);
}

struct HotkeyCollector{std::vector<Hotkey> values;};
static bool CollectHotkey(void *parameter,hotkey_id id,obs_hotkey *hotkey){auto *collector=static_cast<HotkeyCollector*>(parameter);Hotkey value;value.id=id;value.name=api.hk_name(hotkey)?api.hk_name(hotkey):"";value.description=api.hk_desc(hotkey)?api.hk_desc(hotkey):value.name;value.type=api.hk_type(hotkey);value.registerer=api.hk_registerer(hotkey);value.context=Narrow(TypeText(value.type));collector->values.push_back(std::move(value));return true;}
static bool CollectBinding(void *parameter,size_t,obs_hotkey_binding *binding){auto *collector=static_cast<HotkeyCollector*>(parameter);hotkey_id id=api.binding_id(binding);auto found=std::find_if(collector->values.begin(),collector->values.end(),[&](const Hotkey &hotkey){return hotkey.id==id;});if(found!=collector->values.end())found->bindings.push_back(api.binding_combo(binding));return true;}

struct DefaultShortcut{hotkey_id id;const char *name,*key;uint32_t modifiers;int virtualKey{};};
static std::vector<DefaultShortcut> AccessibilityDefaults(){
    std::vector<DefaultShortcut> defaults={{nextAreaHotkey,NEXT_AREA_NAME,"OBS_KEY_F6",0},{previousAreaHotkey,PREVIOUS_AREA_NAME,"OBS_KEY_F6",OBS_MOD_SHIFT},{canvasHotkeys[0],CANVAS_HOTKEY_NAMES[0],"OBS_KEY_F3",0},{canvasHotkeys[1],CANVAS_HOTKEY_NAMES[1],"OBS_KEY_F3",OBS_MOD_SHIFT},{canvasHotkeys[2],CANVAS_HOTKEY_NAMES[2],"OBS_KEY_F3",OBS_MOD_ALT},{canvasHotkeys[3],CANVAS_HOTKEY_NAMES[3],"OBS_KEY_F3",OBS_MOD_CONTROL},{canvasHotkeys[4],CANVAS_HOTKEY_NAMES[4],"OBS_KEY_F4",0},{focusMediaHotkey,FOCUS_MEDIA_NAME,"OBS_KEY_M",OBS_MOD_CONTROL},{volumeConsoleHotkey,VOLUME_CONSOLE_NAME,"OBS_KEY_QUOTELEFT",OBS_MOD_CONTROL,VK_OEM_3}};static constexpr std::array<const char*,6> directKeys={"OBS_KEY_0","OBS_KEY_1","OBS_KEY_2","OBS_KEY_3","OBS_KEY_4","OBS_KEY_5"};for(size_t i=0;i<directAreaHotkeys.size();++i)defaults.push_back({directAreaHotkeys[i],DIRECT_AREA_NAMES[i],directKeys[i],OBS_MOD_CONTROL});static constexpr std::array<std::pair<const char*,const char*>,6> obsDefaults={{{"OBSBasic.StartStreaming","OBS_KEY_F5"},{"OBSBasic.StopStreaming","OBS_KEY_F5"},{"OBSBasic.StartRecording","OBS_KEY_F7"},{"OBSBasic.StopRecording","OBS_KEY_F7"},{"OBSBasic.StartVirtualCam","OBS_KEY_F8"},{"OBSBasic.StopVirtualCam","OBS_KEY_F8"}}};for(const auto &[name,key]:obsDefaults)defaults.push_back({static_cast<hotkey_id>(-1),name,key,0});return defaults;
}

enum class ConflictPolicy{Keep,Replace};
struct ConflictChoice{ConflictPolicy policy{ConflictPolicy::Keep};bool suppress{};};
static ConflictChoice ShowConflictChoice(){
    QDialog dialog(obsMainWindow);dialog.setWindowTitle(CText(CanvasText::ConflictTitle));dialog.setWindowModality(Qt::ApplicationModal);auto *layout=new QVBoxLayout(&dialog);auto *message=new QLabel(CText(CanvasText::ConflictMessage),&dialog);message->setWordWrap(true);message->setAccessibleName(QStringLiteral(" "));message->setAccessibleDescription(QStringLiteral(" "));layout->addWidget(message);auto *replace=new QRadioButton(CText(CanvasText::ReplaceConflicts),&dialog);auto *keep=new QRadioButton(CText(CanvasText::KeepExisting),&dialog);keep->setChecked(true);keep->setAccessibleDescription(CText(CanvasText::ConflictMessage));layout->addWidget(replace);layout->addWidget(keep);auto *suppress=new QCheckBox(CText(CanvasText::DoNotAskBuild),&dialog);layout->addWidget(suppress);auto *buttons=new QDialogButtonBox(QDialogButtonBox::Apply|QDialogButtonBox::Cancel,&dialog);buttons->button(QDialogButtonBox::Apply)->setText(QString::fromWCharArray(Tr(UiText::Apply)));buttons->button(QDialogButtonBox::Cancel)->setText(LText(LocalText::Cancel));QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);layout->addWidget(buttons);QWidget::setTabOrder(keep,replace);QWidget::setTabOrder(replace,suppress);QWidget::setTabOrder(suppress,buttons->button(QDialogButtonBox::Apply));keep->setFocus(Qt::OtherFocusReason);dialog.resize(720,300);if(dialog.exec()!=QDialog::Accepted)return {};return {replace->isChecked()?ConflictPolicy::Replace:ConflictPolicy::Keep,suppress->isChecked()};
}

struct PlannedShortcut{size_t target{};key_combo combo{};std::vector<size_t> conflicts;};
static void ReviewKeyboardShortcutConflicts(){
    config *profile=api.profile_config?api.profile_config():nullptr;if(!profile)return;LoadAccessibilityBindings();const char *reviewed=api.config_get_string(profile,"AccessibleOBSStudio","ShortcutReviewBuild");if(reviewed&&strcmp(reviewed,ACCESSIBLE_OBS_BUILD_ID)==0)return;HotkeyCollector collector;api.enum_hotkeys(CollectHotkey,&collector);api.enum_bindings(CollectBinding,&collector);auto defaults=AccessibilityDefaults();std::vector<PlannedShortcut> plans;
    for(DefaultShortcut &spec:defaults){if(api.config_has_user_value(profile,"Hotkeys",spec.name))continue;size_t target=collector.values.size();for(size_t i=0;i<collector.values.size();++i)if((spec.id!=static_cast<hotkey_id>(-1)&&collector.values[i].id==spec.id)||collector.values[i].name==spec.name){target=i;spec.id=collector.values[i].id;break;}if(target==collector.values.size())continue;int key=spec.virtualKey&&api.key_from_virtual_key?api.key_from_virtual_key(spec.virtualKey):api.key_from_name(spec.key);if(key<=0)continue;plans.push_back({target,{spec.modifiers,key},{}});}
    for(PlannedShortcut &plan:plans)for(size_t index=0;index<collector.values.size();++index){if(index==plan.target)continue;bool intentionalTarget=std::any_of(plans.begin(),plans.end(),[&](const PlannedShortcut &other){return other.target==index&&CombosEqual(other.combo,plan.combo);});if(intentionalTarget)continue;if(std::any_of(collector.values[index].bindings.begin(),collector.values[index].bindings.end(),[&](key_combo binding){return CombosEqual(binding,plan.combo);}))plan.conflicts.push_back(index);}
    bool hasConflicts=std::any_of(plans.begin(),plans.end(),[](const PlannedShortcut &plan){return !plan.conflicts.empty();});ConflictChoice choice;config *global=api.global_config?api.global_config():nullptr;const char *suppressed=global?api.config_get_string(global,"AccessibleOBSStudio","ShortcutPromptSuppressedBuild"):nullptr;if(hasConflicts&&suppressed&&strcmp(suppressed,ACCESSIBLE_OBS_BUILD_ID)==0){const char *policy=api.config_get_string(global,"AccessibleOBSStudio","ShortcutConflictPolicy");choice.policy=policy&&strcmp(policy,"replace")==0?ConflictPolicy::Replace:ConflictPolicy::Keep;choice.suppress=true;}else if(hasConflicts)choice=ShowConflictChoice();
    std::vector<Hotkey> originals=collector.values;std::vector<size_t> touched;auto touch=[&](size_t index){if(std::find(touched.begin(),touched.end(),index)==touched.end())touched.push_back(index);};if(choice.policy==ConflictPolicy::Replace)for(const PlannedShortcut &plan:plans)for(size_t index:plan.conflicts){touch(index);auto &bindings=collector.values[index].bindings;bindings.erase(std::remove_if(bindings.begin(),bindings.end(),[&](key_combo binding){return CombosEqual(binding,plan.combo);}),bindings.end());}
    for(const PlannedShortcut &plan:plans){if(!plan.conflicts.empty()&&choice.policy==ConflictPolicy::Keep)continue;touch(plan.target);collector.values[plan.target].bindings={plan.combo};}
    bool configurationChanged=false,ok=true;for(size_t index:touched)if(!Persist(collector.values[index],configurationChanged)){ok=false;break;}if(ok&&configurationChanged)ok=api.config_save_safe(profile,"tmp",nullptr)==0;if(ok&&api.frontend_save)api.frontend_save();if(!ok){bool rollbackConfiguration=false;for(size_t index:touched)Persist(originals[index],rollbackConfiguration);if(rollbackConfiguration)api.config_save_safe(profile,"tmp",nullptr);if(api.frontend_save)api.frontend_save();QMessageBox::critical(obsMainWindow,QStringLiteral("Accessible OBS Studio"),CText(CanvasText::ConflictSaveFailed));return;}
    api.config_set_string(profile,"AccessibleOBSStudio","ShortcutReviewBuild",ACCESSIBLE_OBS_BUILD_ID);api.config_save_safe(profile,"tmp",nullptr);if(choice.suppress&&global){api.config_set_string(global,"AccessibleOBSStudio","ShortcutPromptSuppressedBuild",ACCESSIBLE_OBS_BUILD_ID);api.config_set_string(global,"AccessibleOBSStudio","ShortcutConflictPolicy",choice.policy==ConflictPolicy::Replace?"replace":"keep");api.config_save_safe(global,"tmp",nullptr);}
}

static bool profileReviewQueued{};
static void QueueProfileReview(){if(profileReviewQueued||!obsMainWindow)return;profileReviewQueued=true;QMetaObject::invokeMethod(obsMainWindow,[]{profileReviewQueued=false;EnsureSafeHotkeyFocusDefault();ReviewKeyboardShortcutConflicts();},Qt::QueuedConnection);}
static void FrontendEvent(int event,void*){constexpr int PROFILE_CHANGED=15,FINISHED_LOADING=26;if(event==PROFILE_CHANGED||event==FINISHED_LOADING)QueueProfileReview();}
