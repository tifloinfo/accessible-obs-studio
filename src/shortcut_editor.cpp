// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Tiflo.Info

static bool CombosEqual(key_combo left,key_combo right){return left.key==right.key&&left.modifiers==right.modifiers;}
static bool BindingListsEqual(const std::vector<key_combo> &left,const std::vector<key_combo> &right){return left.size()==right.size()&&std::equal(left.begin(),left.end(),right.begin(),CombosEqual);}
static bool EditorDirty(){return std::any_of(hotkeys.begin(),hotkeys.end(),[](const Hotkey &hotkey){return !BindingListsEqual(hotkey.bindings,hotkey.originalBindings);});}

static std::wstring BindingSummary(const Hotkey &hotkey){std::wstring text;for(key_combo combo:hotkey.bindings){if(!text.empty())text+=L", ";text+=ComboText(combo);}return text.empty()?std::wstring(Tr(UiText::Unassigned)):text;}

static std::vector<size_t> DistinctCommandIndices(){
    std::vector<size_t> result;for(size_t index=0;index<hotkeys.size();++index){const Hotkey &candidate=hotkeys[index];bool duplicate=std::any_of(result.begin(),result.end(),[&](size_t existingIndex){const Hotkey &existing=hotkeys[existingIndex];return candidate.name==existing.name&&candidate.description==existing.description&&candidate.context==existing.context&&BindingListsEqual(candidate.bindings,existing.bindings);});if(!duplicate)result.push_back(index);}return result;
}

static std::wstring CommandDisplayLabel(size_t index,const std::vector<size_t> &distinct){
    const Hotkey &hotkey=hotkeys[index];std::wstring label=Wide(hotkey.description.c_str());size_t sameDescription=0,sameContext=0,ordinal=0;for(size_t candidateIndex:distinct){const Hotkey &candidate=hotkeys[candidateIndex];if(candidate.description!=hotkey.description)continue;++sameDescription;if(candidate.context==hotkey.context){++sameContext;if(candidateIndex<=index)++ordinal;}}if(sameDescription<2)return label;std::wstring context=Wide(hotkey.context.c_str());if(context.empty())context=L"Separate OBS command";label+=L" ("+context;if(sameContext>1)label+=L"; instance "+std::to_wstring(ordinal)+L" of "+std::to_wstring(sameContext);label+=L")";return label;
}

static int ObsKeyFromVirtualKey(UINT key){
    if(key>='A'&&key<='Z'){char name[]="OBS_KEY_A";name[8]=static_cast<char>(key);return api.key_from_name(name);}if(key>='0'&&key<='9'){char name[]="OBS_KEY_0";name[8]=static_cast<char>(key);return api.key_from_name(name);}if(key>=VK_F1&&key<=VK_F24){std::string name="OBS_KEY_F"+std::to_string(key-VK_F1+1);return api.key_from_name(name.c_str());}
    struct Mapping{UINT vk;const char *name;};static constexpr Mapping mappings[]={{VK_SPACE,"OBS_KEY_SPACE"},{VK_BACK,"OBS_KEY_BACKSPACE"},{VK_DELETE,"OBS_KEY_DELETE"},{VK_INSERT,"OBS_KEY_INSERT"},{VK_HOME,"OBS_KEY_HOME"},{VK_END,"OBS_KEY_END"},{VK_PRIOR,"OBS_KEY_PAGEUP"},{VK_NEXT,"OBS_KEY_PAGEDOWN"},{VK_LEFT,"OBS_KEY_LEFT"},{VK_RIGHT,"OBS_KEY_RIGHT"},{VK_UP,"OBS_KEY_UP"},{VK_DOWN,"OBS_KEY_DOWN"},{VK_PAUSE,"OBS_KEY_PAUSE"},{VK_SNAPSHOT,"OBS_KEY_PRINT"},{VK_NUMPAD0,"OBS_KEY_NUM0"},{VK_NUMPAD1,"OBS_KEY_NUM1"},{VK_NUMPAD2,"OBS_KEY_NUM2"},{VK_NUMPAD3,"OBS_KEY_NUM3"},{VK_NUMPAD4,"OBS_KEY_NUM4"},{VK_NUMPAD5,"OBS_KEY_NUM5"},{VK_NUMPAD6,"OBS_KEY_NUM6"},{VK_NUMPAD7,"OBS_KEY_NUM7"},{VK_NUMPAD8,"OBS_KEY_NUM8"},{VK_NUMPAD9,"OBS_KEY_NUM9"},{VK_ADD,"OBS_KEY_PLUS"},{VK_SUBTRACT,"OBS_KEY_MINUS"},{VK_MULTIPLY,"OBS_KEY_ASTERISK"},{VK_DIVIDE,"OBS_KEY_SLASH"},{VK_DECIMAL,"OBS_KEY_PERIOD"}};for(const auto &mapping:mappings)if(mapping.vk==key)return api.key_from_name(mapping.name);return api.key_from_virtual_key?api.key_from_virtual_key(static_cast<int>(key)):0;
}

static bool ReservedShortcut(UINT key,bool ctrl,bool alt,bool shift,bool win){
    if(key==VK_TAB||key==VK_RETURN||key==VK_ESCAPE||key==VK_LWIN||key==VK_RWIN||key==VK_SHIFT||key==VK_CONTROL||key==VK_MENU)return true;if(win)return true;if(alt&&(key==VK_F4||key==VK_TAB||key==VK_ESCAPE||key==VK_SPACE))return true;if(ctrl&&key==VK_ESCAPE)return true;if(ctrl&&alt&&key==VK_DELETE)return true;(void)shift;return false;
}

enum class EditorSaveResult{Saved,Cancelled,Failed};

static bool SameLogicalCommand(const Hotkey &left,const Hotkey &right){return left.name==right.name&&left.description==right.description&&left.context==right.context;}

static EditorSaveResult SaveEditorChanges(QWidget *parent){
    std::vector<size_t> changed;for(size_t index=0;index<hotkeys.size();++index)if(!BindingListsEqual(hotkeys[index].bindings,hotkeys[index].originalBindings))changed.push_back(index);if(changed.empty())return EditorSaveResult::Saved;
    config *cfg=api.profile_config?api.profile_config():nullptr;auto rollback=[&]{bool rollbackConfiguration=false;for(size_t index:changed){Hotkey original=hotkeys[index];original.bindings=hotkeys[index].originalBindings;Persist(original,rollbackConfiguration);}if(rollbackConfiguration&&cfg)api.config_save_safe(cfg,"tmp",nullptr);if(api.frontend_save)api.frontend_save();};
    bool configurationChanged=false;for(size_t index:changed)if(!Persist(hotkeys[index],configurationChanged)){rollback();QMessageBox::critical(parent,QStringLiteral("Accessible OBS Studio"),LText(LocalText::PrepareSaveFailure));return EditorSaveResult::Failed;}
    if(configurationChanged&&(!cfg||api.config_save_safe(cfg,"tmp",nullptr)!=0)){rollback();QMessageBox::critical(parent,QStringLiteral("Accessible OBS Studio"),LText(LocalText::SaveFailure));return EditorSaveResult::Failed;}
    if(api.frontend_save)api.frontend_save();for(size_t index:changed)hotkeys[index].originalBindings=hotkeys[index].bindings;return EditorSaveResult::Saved;
}
static void OpenAccessibleObsHotkey(void*,hotkey_id,obs_hotkey*,bool pressed){if(pressed&&obsMainWindow)QMetaObject::invokeMethod(obsMainWindow,[]{ShowQtHotkeyEditor();},Qt::QueuedConnection);}
