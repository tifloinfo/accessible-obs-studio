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

static void NumberMixerSources(){
    auto sliders=MixerSliders();for(size_t i=0;i<sliders.size();++i){QAbstractSlider *slider=sliders[i];QVariant original=slider->property("accessibleObsStudioOriginalName");if(!original.isValid()){original=slider->accessibleName();slider->setProperty("accessibleObsStudioOriginalName",original);}QString name=original.toString();if(name.isEmpty())name=QStringLiteral("Audio source");slider->setAccessibleName(QStringLiteral("%1. %2").arg(i+1).arg(name));}
}

static void ApplyHotkeyFocusPolicy(){
    if(!api.hotkey_enable_background_press||!api.global_config)return;config *cfg=api.global_config();if(!cfg)return;const char *value=api.config_get_string(cfg,"General","HotkeyFocusType");bool active=QApplication::applicationState()==Qt::ApplicationActive;bool enabled=true;if(value&&_stricmp(value,"DisableHotkeysOutOfFocus")==0)enabled=active;else if(value&&_stricmp(value,"DisableHotkeysInFocus")==0)enabled=!active;api.hotkey_enable_background_press(enabled);
}

static void EnsureSafeHotkeyFocusDefault(){
    if(!api.global_config)return;config *cfg=api.global_config();if(!cfg)return;if(!api.config_has_user_value(cfg,"AccessibleOBSStudio","SafeHotkeyFocusInitialized")){api.config_set_string(cfg,"General","HotkeyFocusType","DisableHotkeysOutOfFocus");api.config_set_string(cfg,"AccessibleOBSStudio","SafeHotkeyFocusInitialized","true");api.config_save_safe(cfg,"tmp",nullptr);}ApplyHotkeyFocusPolicy();
}

class AccessibilityFilter final:public QObject{
public:
    bool eventFilter(QObject *object,QEvent *event) override{
        if(event->type()==QEvent::ApplicationActivate||event->type()==QEvent::ApplicationDeactivate||event->type()==QEvent::ApplicationStateChange)ApplyHotkeyFocusPolicy();
        if(event->type()==QEvent::FocusIn){QWidget *widget=qobject_cast<QWidget*>(object);QWidget *dock=obsMainWindow?obsMainWindow->findChild<QWidget*>(QStringLiteral("mixerDock")):nullptr;if(widget&&dock&&Contains(dock,widget))NumberMixerSources();}
        if(event->type()!=QEvent::KeyPress)return QObject::eventFilter(object,event);auto *keyEvent=static_cast<QKeyEvent*>(event);QWidget *focus=QApplication::focusWidget();QWidget *dock=obsMainWindow?obsMainWindow->findChild<QWidget*>(QStringLiteral("mixerDock")):nullptr;if(!focus||!dock||!Contains(dock,focus)||keyEvent->modifiers()&~Qt::KeypadModifier)return QObject::eventFilter(object,event);
        int index=-1;if(keyEvent->key()>=Qt::Key_1&&keyEvent->key()<=Qt::Key_9)index=keyEvent->key()-Qt::Key_1;else if(keyEvent->key()==Qt::Key_0)index=9;if(index<0)return QObject::eventFilter(object,event);auto sliders=MixerSliders();if(index>=static_cast<int>(sliders.size())){QApplication::beep();return true;}NumberMixerSources();sliders[static_cast<size_t>(index)]->setFocus(Qt::ShortcutFocusReason);return true;
    }
};

static bool FocusRegion(QWidget *region){
    if(region->objectName()==QStringLiteral("previewContainer")){
        QWidget *preview=region->findChild<QWidget*>(QStringLiteral("preview"));
        if(preview){preview->setAccessibleName(QString::fromWCharArray(Tr(UiText::PreviewName)));if(preview->focusPolicy()==Qt::NoFocus)preview->setFocusPolicy(Qt::ClickFocus);preview->setFocus(Qt::ShortcutFocusReason);if(preview->hasFocus())return true;}
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

static void NavigationHotkey(void *data,hotkey_id,obs_hotkey*,bool pressed){if(pressed)CycleInterfaceArea(data!=nullptr);}

static constexpr std::array<const char*,6> DIRECT_AREA_WIDGETS={"previewContainer","scenesDock","sourcesDock","mixerDock","transitionsDock","controlsDock"};

static void DirectAreaHotkey(void *data,hotkey_id,obs_hotkey*,bool pressed){
    if(!pressed||!MainInterfaceActive()||!obsMainWindow)return;
    const intptr_t encoded=reinterpret_cast<intptr_t>(data);if(encoded<1||encoded>static_cast<intptr_t>(DIRECT_AREA_WIDGETS.size()))return;
    QWidget *region=obsMainWindow->findChild<QWidget*>(DIRECT_AREA_WIDGETS[static_cast<size_t>(encoded-1)]);if(!region||!region->isVisible())return;if(encoded==4){NumberMixerSources();auto sliders=MixerSliders();if(!sliders.empty()){sliders.front()->setFocus(Qt::ShortcutFocusReason);return;}}FocusRegion(region);
}

static void FocusMediaControlsHotkey(void*,hotkey_id,obs_hotkey*,bool pressed){
    if(!pressed||!MainInterfaceActive()||!obsMainWindow)return;QWidget *controls=obsMainWindow->findChild<QWidget*>(QStringLiteral("MediaControls"));if(!controls||!controls->isVisible()){MessageBeep(MB_ICONINFORMATION);return;}FocusRegion(controls);
}

static bool EnsureSourceSelection(){
    QAbstractItemView *sources=obsMainWindow?obsMainWindow->findChild<QAbstractItemView*>(QStringLiteral("sources")):nullptr;if(!sources||!sources->model())return false;if(sources->selectionModel()&&!sources->selectionModel()->selectedRows().empty())return true;QStringList names;for(int row=0;row<sources->model()->rowCount();++row)names<<sources->model()->index(row,0).data(Qt::DisplayRole).toString();if(names.empty())return false;bool accepted=false;QString selected=QInputDialog::getItem(obsMainWindow,QStringLiteral("Accessible OBS Studio - Select Source"),QStringLiteral("Source:"),names,0,false,&accepted);if(!accepted)return false;int row=names.indexOf(selected);if(row<0)return false;QModelIndex index=sources->model()->index(row,0);sources->setCurrentIndex(index);if(sources->selectionModel())sources->selectionModel()->select(index,QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);return true;
}

static void ShowSuggestedActions(){
    if(!obsMainWindow||!EnsureSourceSelection()){QMessageBox::information(obsMainWindow,QStringLiteral("Accessible OBS Studio"),QStringLiteral("No source is available or selected."));return;}
    struct ActionSpec{const char *objectName;const char *label;const char *risk;};static constexpr ActionSpec specs[]={{"actionCenterToScreen","Center fully","Low"},{"actionHorizontalCenter","Center horizontally","Low"},{"actionVerticalCenter","Center vertically","Low"},{"actionFitToScreen","Fit to canvas, preserving proportions","Medium"},{"actionResetTransform","Reset transform","Medium"},{"actionFlipHorizontal","Flip horizontally","Medium"},{"actionFlipVertical","Flip vertically","Medium"},{"actionRotate90CW","Rotate 90 degrees clockwise","Medium"},{"actionRotate90CCW","Rotate 90 degrees counterclockwise","Medium"},{"actionRotate180","Rotate 180 degrees","Medium"}};
    QDialog dialog(obsMainWindow);dialog.setWindowTitle(QStringLiteral("Accessible OBS Studio - Suggested Actions"));dialog.setModal(true);auto *layout=new QVBoxLayout(&dialog);bool studio=api.studio_mode_active&&api.studio_mode_active();auto *state=new QLabel(studio?QStringLiteral("Studio Mode is active. Approved actions affect the preview scene."):QStringLiteral("Studio Mode is not active. Approved actions may immediately affect the live output."),&dialog);state->setWordWrap(true);layout->addWidget(state);auto *list=new QListWidget(&dialog);list->setAccessibleName(QStringLiteral("Suggested actions"));for(const auto &spec:specs){QAction *action=obsMainWindow->findChild<QAction*>(QString::fromLatin1(spec.objectName));auto *item=new QListWidgetItem(QStringLiteral("%1. Risk: %2").arg(QString::fromLatin1(spec.label),QString::fromLatin1(spec.risk)),list);item->setData(Qt::UserRole,QString::fromLatin1(spec.objectName));item->setFlags(item->flags()|Qt::ItemIsUserCheckable);item->setCheckState(Qt::Unchecked);if(!action||!action->isEnabled()){item->setFlags(item->flags()&~Qt::ItemIsEnabled);item->setToolTip(QStringLiteral("Unavailable for the current selection, possibly because the source is locked."));}}layout->addWidget(list);auto *explanation=new QLabel(QStringLiteral("Nothing will change until you check one or more actions and activate Apply Selected. OBS records each applied transform in its Undo history."),&dialog);explanation->setWordWrap(true);layout->addWidget(explanation);auto *buttons=new QDialogButtonBox(&dialog);QPushButton *apply=buttons->addButton(QStringLiteral("Apply Selected"),QDialogButtonBox::AcceptRole);QPushButton *reject=buttons->addButton(QStringLiteral("Reject All"),QDialogButtonBox::RejectRole);QPushButton *explain=buttons->addButton(QStringLiteral("Explain"),QDialogButtonBox::HelpRole);QPushButton *cancel=buttons->addButton(QDialogButtonBox::Cancel);QObject::connect(apply,&QPushButton::clicked,&dialog,&QDialog::accept);QObject::connect(reject,&QPushButton::clicked,&dialog,&QDialog::reject);QObject::connect(cancel,&QPushButton::clicked,&dialog,&QDialog::reject);QObject::connect(explain,&QPushButton::clicked,[&]{QMessageBox::information(&dialog,QStringLiteral("Suggested Actions"),QStringLiteral("Centering changes position only. Fit to canvas changes position and scale while preserving proportions. Reset Transform restores the source's default transform. Flip and Rotate change orientation. Locked or unavailable actions cannot be selected."));});layout->addWidget(buttons);dialog.resize(620,520);list->setFocus();if(dialog.exec()!=QDialog::Accepted)return;QStringList applied,skipped;for(int i=0;i<list->count();++i){QListWidgetItem *item=list->item(i);if(item->checkState()!=Qt::Checked)continue;QAction *action=obsMainWindow->findChild<QAction*>(item->data(Qt::UserRole).toString());if(action&&action->isEnabled()){action->trigger();applied<<item->text();}else skipped<<item->text();}QString result=applied.isEmpty()?QStringLiteral("No actions were applied."):QStringLiteral("Applied:\n%1\n\nUse Ctrl+Z in OBS to undo.").arg(applied.join(QStringLiteral("\n")));if(!skipped.empty())result+=QStringLiteral("\n\nSkipped because the source state changed or the action became unavailable:\n%1").arg(skipped.join(QStringLiteral("\n")));QMessageBox::information(obsMainWindow,QStringLiteral("Accessible OBS Studio"),result);
}

static bool SaveNavigationBinding(config *cfg,hotkey_id id,const char *name){
    if(!cfg)return false;obs_data_array *array=api.hotkey_save(id);obs_data *data=api.data_create();api.data_set_array(data,"bindings",array);const char *json=api.data_json(data);bool saved=json&&*json&&!strstr(json,"\"bindings\":[]");api.config_set_string(cfg,"Hotkeys",name,json?json:"");api.data_release(data);api.array_release(array);return saved;
}

static bool LoadNavigationBinding(hotkey_id id,const char *name,const char *defaultKey,uint32_t modifiers,bool persistMissing){
    config *cfg=api.profile_config?api.profile_config():nullptr;
    if(cfg&&api.config_has_user_value(cfg,"Hotkeys",name)){
        const char *json=api.config_get_string(cfg,"Hotkeys",name);obs_data *data=json&&*json?api.data_create_json(json):nullptr;
        if(data){obs_data_array *array=api.data_get_array(data,"bindings");if(array){api.hotkey_load(id,array);api.array_release(array);}api.data_release(data);return false;}
        api.load_bindings(id,nullptr,0);return false;
    }
    key_combo combo{modifiers,api.key_from_name(defaultKey)};api.load_bindings(id,&combo,1);if(persistMissing)SaveNavigationBinding(cfg,id,name);return persistMissing&&cfg;
}

static void LoadNavigationBindings(bool persistMissing=false){
    bool changed=false;changed|=LoadNavigationBinding(nextAreaHotkey,NEXT_AREA_NAME,"OBS_KEY_F6",0,persistMissing);changed|=LoadNavigationBinding(previousAreaHotkey,PREVIOUS_AREA_NAME,"OBS_KEY_F6",OBS_MOD_SHIFT,persistMissing);changed|=LoadNavigationBinding(describeCanvasHotkey,DESCRIBE_CANVAS_NAME,"OBS_KEY_F3",0,persistMissing);changed|=LoadNavigationBinding(focusMediaHotkey,FOCUS_MEDIA_NAME,"OBS_KEY_F4",0,persistMissing);
    static constexpr std::array<const char*,6> keys={"OBS_KEY_0","OBS_KEY_1","OBS_KEY_2","OBS_KEY_3","OBS_KEY_4","OBS_KEY_5"};
    for(size_t i=0;i<directAreaHotkeys.size();++i)changed|=LoadNavigationBinding(directAreaHotkeys[i],DIRECT_AREA_NAMES[i],keys[i],OBS_MOD_CONTROL,persistMissing);if(changed){config *cfg=api.profile_config?api.profile_config():nullptr;if(cfg)api.config_save_safe(cfg,"tmp",nullptr);}
}

struct DefaultObsHotkeyContext{config *cfg{};uint32_t verified{};bool changed{};};
static bool AssignDefaultObsHotkey(void *parameter,hotkey_id id,obs_hotkey *hotkey){
    static constexpr std::array<std::pair<const char*,const char*>,6> defaults={{{"OBSBasic.StartStreaming","OBS_KEY_F5"},{"OBSBasic.StopStreaming","OBS_KEY_F5"},{"OBSBasic.StartRecording","OBS_KEY_F7"},{"OBSBasic.StopRecording","OBS_KEY_F7"},{"OBSBasic.StartVirtualCam","OBS_KEY_F8"},{"OBSBasic.StopVirtualCam","OBS_KEY_F8"}}};
    auto *context=static_cast<DefaultObsHotkeyContext*>(parameter);const char *name=api.hk_name(hotkey);if(!name)return true;for(size_t index=0;index<defaults.size();++index){const auto &[target,key]=defaults[index];if(strcmp(name,target)!=0)continue;const char *json=context->cfg&&api.config_has_user_value(context->cfg,"Hotkeys",target)?api.config_get_string(context->cfg,"Hotkeys",target):nullptr;if(json&&*json&&!strstr(json,"\"bindings\":[]")){context->verified|=1u<<index;return true;}key_combo combo{0,api.key_from_name(key)};api.load_bindings(id,&combo,1);if(SaveNavigationBinding(context->cfg,id,target))context->verified|=1u<<index;context->changed=true;return true;}return true;
}
static void EnsureDefaultObsHotkeys(){config *cfg=api.profile_config?api.profile_config():nullptr;if(!cfg||api.config_has_user_value(cfg,"AccessibleOBSStudio","DefaultHotkeysVerified"))return;DefaultObsHotkeyContext context{cfg,0,false};api.enum_hotkeys(AssignDefaultObsHotkey,&context);if(context.verified!=0x3Fu)return;api.config_set_string(cfg,"AccessibleOBSStudio","DefaultHotkeysVerified","true");if(context.changed&&api.frontend_save)api.frontend_save();api.config_save_safe(cfg,"tmp",nullptr);}

static void FrontendEvent(int event,void*){constexpr int PROFILE_CHANGED=15,FINISHED_LOADING=26;if(event==PROFILE_CHANGED||event==FINISHED_LOADING){LoadNavigationBindings(true);EnsureDefaultObsHotkeys();EnsureSafeHotkeyFocusDefault();}}
