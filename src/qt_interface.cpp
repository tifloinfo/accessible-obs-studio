// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Tiflo.Info

static UINT QtVirtualKey(QKeyEvent *event){
    UINT key=static_cast<UINT>(event->nativeVirtualKey());if(key)return key;int qtKey=event->key();
    if(qtKey>=Qt::Key_A&&qtKey<=Qt::Key_Z)return static_cast<UINT>('A'+qtKey-Qt::Key_A);
    if(qtKey>=Qt::Key_0&&qtKey<=Qt::Key_9)return static_cast<UINT>('0'+qtKey-Qt::Key_0);
    if(qtKey>=Qt::Key_F1&&qtKey<=Qt::Key_F24)return static_cast<UINT>(VK_F1+qtKey-Qt::Key_F1);
    switch(qtKey){case Qt::Key_Space:return VK_SPACE;case Qt::Key_Backspace:return VK_BACK;case Qt::Key_Delete:return VK_DELETE;case Qt::Key_Insert:return VK_INSERT;case Qt::Key_Home:return VK_HOME;case Qt::Key_End:return VK_END;case Qt::Key_PageUp:return VK_PRIOR;case Qt::Key_PageDown:return VK_NEXT;case Qt::Key_Left:return VK_LEFT;case Qt::Key_Right:return VK_RIGHT;case Qt::Key_Up:return VK_UP;case Qt::Key_Down:return VK_DOWN;case Qt::Key_Pause:return VK_PAUSE;case Qt::Key_Print:return VK_SNAPSHOT;default:return 0;}
}

class QtShortcutCaptureEdit final:public QLineEdit{
public:
    std::function<void(key_combo)> captured;
    explicit QtShortcutCaptureEdit(QWidget *parent=nullptr):QLineEdit(parent){setReadOnly(true);setAccessibleName(QStringLiteral("Shortcut"));setAccessibleDescription(QStringLiteral("Press the desired shortcut. Tab moves to the next control."));}
protected:
    void keyPressEvent(QKeyEvent *event) override{
        if(event->key()==Qt::Key_Tab||event->key()==Qt::Key_Backtab||event->key()==Qt::Key_Return||event->key()==Qt::Key_Enter||event->key()==Qt::Key_Escape){QLineEdit::keyPressEvent(event);return;}
        UINT key=QtVirtualKey(event);Qt::KeyboardModifiers modifiers=event->modifiers();bool ctrl=modifiers.testFlag(Qt::ControlModifier),alt=modifiers.testFlag(Qt::AltModifier),shift=modifiers.testFlag(Qt::ShiftModifier),win=modifiers.testFlag(Qt::MetaModifier);if(!key||ReservedShortcut(key,ctrl,alt,shift,win)){QLineEdit::keyPressEvent(event);return;}int obsKey=ObsKeyFromVirtualKey(key);if(obsKey<=0){QApplication::beep();event->accept();return;}key_combo combo{(ctrl?OBS_MOD_CONTROL:0u)|(alt?OBS_MOD_ALT:0u)|(shift?OBS_MOD_SHIFT:0u),obsKey};if(captured)captured(combo);event->accept();
    }
};

class QtHotkeyAssignmentDialog final:public QDialog{
    size_t index{};std::vector<key_combo> bindings;QListWidget *assignments{};QtShortcutCaptureEdit *capture{};bool append{},armed{true};
    void refresh(){int selected=assignments->currentRow();assignments->clear();for(key_combo combo:bindings)assignments->addItem(QString::fromStdWString(ComboText(combo)));if(!bindings.empty()){selected=std::clamp(selected,0,static_cast<int>(bindings.size())-1);assignments->setCurrentRow(selected);if(!append)capture->setText(QString::fromStdWString(ComboText(bindings[static_cast<size_t>(selected)])));}else capture->setText(QStringLiteral("None"));}
    void receive(key_combo combo){if(!armed){QApplication::beep();return;}if(std::any_of(bindings.begin(),bindings.end(),[&](key_combo existing){return CombosEqual(existing,combo); })){QApplication::beep();return;}int selected=assignments->currentRow();if(append||selected<0)bindings.push_back(combo);else bindings[static_cast<size_t>(selected)]=combo;append=false;armed=false;refresh();}
public:
    QtHotkeyAssignmentDialog(size_t hotkeyIndex,QWidget *parent):QDialog(parent),index(hotkeyIndex),bindings(hotkeys[hotkeyIndex].bindings){
        setWindowTitle(QStringLiteral("Hot Key"));setWindowModality(Qt::WindowModal);auto *layout=new QVBoxLayout(this);auto distinct=DistinctCommandIndices();QString commandText=QString::fromStdWString(CommandDisplayLabel(index,distinct));auto *command=new QLabel(QStringLiteral("Command: %1").arg(commandText),this);command->setWordWrap(true);command->setAccessibleName(QStringLiteral(" "));command->setAccessibleDescription(QStringLiteral(" "));layout->addWidget(command);auto *assignedLabel=new QLabel(QStringLiteral("&Assigned shortcuts:"),this);assignments=new QListWidget(this);assignments->setAccessibleName(QStringLiteral("Assigned shortcuts"));assignments->setAccessibleDescription(QStringLiteral("Use the arrow keys to select an assignment. Tab moves to the Shortcut field."));assignments->setTabKeyNavigation(false);assignedLabel->setBuddy(assignments);layout->addWidget(assignedLabel);layout->addWidget(assignments);
        auto *captureLabel=new QLabel(QStringLiteral("&Shortcut:"),this);capture=new QtShortcutCaptureEdit(this);capture->setAccessibleDescription(QStringLiteral("Command: %1. Press the desired shortcut. Tab moves to the next control.").arg(commandText));captureLabel->setBuddy(capture);layout->addWidget(captureLabel);layout->addWidget(capture);auto *bindingButtons=new QHBoxLayout;auto *addAnother=new QPushButton(QStringLiteral("&Add Another Shortcut"),this);auto *remove=new QPushButton(QStringLiteral("&Remove Shortcut"),this);bindingButtons->addWidget(addAnother);bindingButtons->addWidget(remove);bindingButtons->addStretch();layout->addLayout(bindingButtons);auto *buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,this);layout->addWidget(buttons);QPushButton *ok=buttons->button(QDialogButtonBox::Ok),*cancel=buttons->button(QDialogButtonBox::Cancel);QWidget::setTabOrder(assignments,capture);QWidget::setTabOrder(capture,addAnother);QWidget::setTabOrder(addAnother,remove);QWidget::setTabOrder(remove,ok);QWidget::setTabOrder(ok,cancel);
        capture->captured=[this](key_combo combo){receive(combo);};QObject::connect(capture,&QLineEdit::returnPressed,[this]{hotkeys[index].bindings=bindings;accept();});QObject::connect(assignments,&QListWidget::currentRowChanged,[this](int row){if(row>=0&&row<static_cast<int>(bindings.size())){append=false;armed=true;capture->setText(QString::fromStdWString(ComboText(bindings[static_cast<size_t>(row)])));capture->setFocus(Qt::OtherFocusReason);}});QObject::connect(addAnother,&QPushButton::clicked,[this]{append=true;armed=true;capture->setText(QStringLiteral("None"));capture->setFocus(Qt::OtherFocusReason);});QObject::connect(remove,&QPushButton::clicked,[this]{int selected=assignments->currentRow();if(selected>=0&&selected<static_cast<int>(bindings.size()))bindings.erase(bindings.begin()+selected);append=bindings.empty();armed=true;refresh();capture->setFocus(Qt::OtherFocusReason);});QObject::connect(buttons,&QDialogButtonBox::accepted,[this]{hotkeys[index].bindings=bindings;accept();});QObject::connect(buttons,&QDialogButtonBox::rejected,this,&QDialog::reject);refresh();resize(600,420);QTimer::singleShot(0,capture,[this]{capture->setFocus(Qt::OtherFocusReason);});
    }
};

static constexpr int COMMAND_TEXT_ROLE=Qt::UserRole+1,SHORTCUT_TEXT_ROLE=Qt::UserRole+2;

class QtHotkeyListDelegate final:public QStyledItemDelegate{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *painter,const QStyleOptionViewItem &option,const QModelIndex &index) const override{
        QStyleOptionViewItem background(option);initStyleOption(&background,index);background.text.clear();QStyledItemDelegate::paint(painter,background,index);QRect content=option.rect.adjusted(8,0,-8,0);int shortcutWidth=std::clamp(content.width()/3,180,360);QRect commandRect=content;commandRect.setRight(content.right()-shortcutWidth-12);QRect shortcutRect=content;shortcutRect.setLeft(commandRect.right()+13);QColor color=(option.state&QStyle::State_Selected)?option.palette.color(QPalette::HighlightedText):option.palette.color(QPalette::Text);painter->save();painter->setPen(color);QString command=option.fontMetrics.elidedText(index.data(COMMAND_TEXT_ROLE).toString(),Qt::ElideRight,commandRect.width());QString shortcut=option.fontMetrics.elidedText(index.data(SHORTCUT_TEXT_ROLE).toString(),Qt::ElideLeft,shortcutRect.width());painter->drawText(commandRect,Qt::AlignVCenter|Qt::AlignLeft,command);painter->drawText(shortcutRect,Qt::AlignVCenter|Qt::AlignRight,shortcut);painter->restore();
    }
};

class QtCommandList final:public QListWidget{
public:
    std::function<void()> editRequested,removeRequested,closeRequested;
    using QListWidget::QListWidget;
protected:
    void keyPressEvent(QKeyEvent *event) override{if(event->key()==Qt::Key_Return||event->key()==Qt::Key_Enter){if(editRequested)editRequested();event->accept();return;}if(event->key()==Qt::Key_Delete){if(removeRequested)removeRequested();event->accept();return;}if(event->key()==Qt::Key_Escape){if(closeRequested)closeRequested();event->accept();return;}QListWidget::keyPressEvent(event);}
};

class QtHotkeyEditorDialog final:public QDialog{
    QLineEdit *search{};QtCommandList *commands{};QPushButton *editButton{},*removeButton{};bool closing{};
    int selectedIndex() const{QListWidgetItem *item=commands->currentItem();return item?item->data(Qt::UserRole).toInt():-1;}
    void refresh(int preferred=-1){QString needle=search->text();auto distinct=DistinctCommandIndices();commands->clear();QListWidgetItem *preferredItem=nullptr;for(size_t i:distinct){QString command=QString::fromStdWString(CommandDisplayLabel(i,distinct));QString context=QString::fromUtf8(hotkeys[i].context.c_str());if(!needle.isEmpty()&&!command.contains(needle,Qt::CaseInsensitive)&&!context.contains(needle,Qt::CaseInsensitive))continue;QString shortcut=QString::fromStdWString(BindingSummary(hotkeys[i]));auto *item=new QListWidgetItem(QStringLiteral("%1. Assigned shortcuts: %2").arg(command,shortcut),commands);item->setData(Qt::UserRole,static_cast<int>(i));item->setData(COMMAND_TEXT_ROLE,command);item->setData(SHORTCUT_TEXT_ROLE,shortcut);if(static_cast<int>(i)==preferred)preferredItem=item;}if(preferredItem)commands->setCurrentItem(preferredItem);else if(commands->count()>0)commands->setCurrentRow(0);bool available=selectedIndex()>=0;editButton->setEnabled(available);removeButton->setEnabled(available);}
    void editSelected(){int selected=selectedIndex();if(selected<0){QApplication::beep();return;}QtHotkeyAssignmentDialog dialog(static_cast<size_t>(selected),this);if(dialog.exec()==QDialog::Accepted)refresh(selected);}
    void removeSelected(){int selected=selectedIndex();if(selected<0){QApplication::beep();return;}hotkeys[static_cast<size_t>(selected)].bindings.clear();refresh(selected);}
    void discard(){for(Hotkey &hotkey:hotkeys)hotkey.bindings=hotkey.originalBindings;}
protected:
    void closeEvent(QCloseEvent *event) override{
        if(closing||!EditorDirty()){event->accept();return;}QMessageBox box(QMessageBox::Question,QStringLiteral("Accessible OBS Studio"),QStringLiteral("You have unsaved changes. Save changes?"),QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel,this);box.setInformativeText(QStringLiteral("Choose Save to apply the changes, Discard to close without saving, or Cancel to return to the editor."));box.setDefaultButton(QMessageBox::Save);int choice=box.exec();if(choice==QMessageBox::Save){SaveEditorChanges();closing=true;event->accept();}else if(choice==QMessageBox::Discard){discard();closing=true;event->accept();}else event->ignore();
    }
    void reject() override{close();}
public:
    explicit QtHotkeyEditorDialog(QWidget *parent):QDialog(parent){
        setWindowTitle(QStringLiteral("Accessible OBS Studio — Shortcut Editor"));setAttribute(Qt::WA_DeleteOnClose);auto *layout=new QVBoxLayout(this);auto *searchLabel=new QLabel(QStringLiteral("&Find commands:"),this);search=new QLineEdit(this);search->setAccessibleName(QStringLiteral("Find commands"));searchLabel->setBuddy(search);layout->addWidget(searchLabel);layout->addWidget(search);auto *headers=new QHBoxLayout;auto *commandHeader=new QLabel(QStringLiteral("Command"),this);auto *shortcutHeader=new QLabel(QStringLiteral("Assigned shortcuts"),this);shortcutHeader->setAlignment(Qt::AlignRight);headers->addWidget(commandHeader,2);headers->addWidget(shortcutHeader,1);layout->addLayout(headers);commands=new QtCommandList(this);commands->setItemDelegate(new QtHotkeyListDelegate(commands));commands->setAccessibleName(QStringLiteral("Commands"));commands->setAccessibleDescription(QStringLiteral("Use the arrow keys to select a command. Press Enter to add or edit shortcuts, Delete to remove assignments, or Tab to move to the buttons."));commands->setSelectionMode(QAbstractItemView::SingleSelection);commands->setEditTriggers(QAbstractItemView::NoEditTriggers);commands->setTabKeyNavigation(false);layout->addWidget(commands);auto *buttons=new QDialogButtonBox(this);editButton=buttons->addButton(QStringLiteral("&Add or Edit"),QDialogButtonBox::ActionRole);removeButton=buttons->addButton(QStringLiteral("&Remove"),QDialogButtonBox::DestructiveRole);auto *apiSettings=buttons->addButton(QStringLiteral("OpenAI &API Settings"),QDialogButtonBox::ActionRole);auto *saveClose=buttons->addButton(QStringLiteral("&Save and Close"),QDialogButtonBox::AcceptRole);auto *cancel=buttons->addButton(QDialogButtonBox::Cancel);layout->addWidget(buttons);QWidget::setTabOrder(search,commands);QWidget::setTabOrder(commands,editButton);QWidget::setTabOrder(editButton,removeButton);QWidget::setTabOrder(removeButton,apiSettings);QWidget::setTabOrder(apiSettings,saveClose);QWidget::setTabOrder(saveClose,cancel);LoadData();refresh();commands->editRequested=[this]{editSelected();};commands->removeRequested=[this]{removeSelected();};commands->closeRequested=[this]{close();};QObject::connect(search,&QLineEdit::textChanged,[this]{refresh(selectedIndex());});QObject::connect(commands,&QListWidget::currentItemChanged,[this](QListWidgetItem*,QListWidgetItem*){bool available=selectedIndex()>=0;editButton->setEnabled(available);removeButton->setEnabled(available);});QObject::connect(commands,&QListWidget::itemDoubleClicked,[this](QListWidgetItem*){editSelected();});QObject::connect(editButton,&QPushButton::clicked,[this]{editSelected();});QObject::connect(removeButton,&QPushButton::clicked,[this]{removeSelected();});QObject::connect(apiSettings,&QPushButton::clicked,[]{ShowSettings();});QObject::connect(saveClose,&QPushButton::clicked,[this]{SaveEditorChanges();closing=true;accept();});QObject::connect(cancel,&QPushButton::clicked,[this]{close();});resize(900,650);QTimer::singleShot(0,search,[this]{search->setFocus(Qt::OtherFocusReason);});
    }
    void focusSearch(){if(isMinimized())showNormal();else show();raise();activateWindow();search->setFocus(Qt::OtherFocusReason);}
};

static void ShowQtHotkeyEditor(){
    if(qtHotkeyEditor){static_cast<QtHotkeyEditorDialog*>(qtHotkeyEditor.data())->focusSearch();return;}auto *editor=new QtHotkeyEditorDialog(obsMainWindow);qtHotkeyEditor=editor;editor->show();editor->raise();editor->activateWindow();
}

static void ShowAccessibleObsMenu(){
    if(!accessibleToolsMenu||!obsMainWindow)return;if(accessibleToolsMenu->isVisible()){accessibleToolsMenu->setFocus(Qt::ShortcutFocusReason);return;}QPoint center=obsMainWindow->rect().center();QPoint global=obsMainWindow->mapToGlobal(center);accessibleToolsMenu->popup(global);QTimer::singleShot(0,accessibleToolsMenu,[menu=accessibleToolsMenu]{if(menu&&!menu->actions().empty()){menu->setActiveAction(menu->actions().front());menu->setFocus(Qt::ShortcutFocusReason);}});
}

static bool InitializeAccessibleToolsMenu(){
    if(!api.add_tools_qaction||!obsMainWindow)return false;auto *root=static_cast<QAction*>(api.add_tools_qaction("Accessible OBS Studio"));if(!root)return false;auto *menu=new QMenu(obsMainWindow);menu->setTitle(QStringLiteral("Accessible OBS Studio"));root->setMenu(menu);accessibleToolsAction=root;accessibleToolsMenu=menu;QAction *editor=menu->addAction(QStringLiteral("&Shortcut Editor"));menu->addSeparator();QAction *settings=menu->addAction(QStringLiteral("OpenAI &API Settings"));QObject::connect(editor,&QAction::triggered,[]{ShowQtHotkeyEditor();});QObject::connect(settings,&QAction::triggered,[]{ShowSettings();});return true;
}
