// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Tiflo.Info

#include <windows.h>
#include <wincred.h>
#include <winhttp.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <wrl.h>
#include <WebView2.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <cmath>
#include <cwctype>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <QApplication>
#include <QAbstractButton>
#include <QAbstractSlider>
#include <QAbstractItemView>
#include <QAction>
#include <QAccessible>
#include <QBuffer>
#include <QByteArray>
#include <QDockWidget>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMainWindow>
#include <QMetaObject>
#include <QKeyEvent>
#include <QPointer>
#include <QObject>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QCloseEvent>
#include <QEventLoop>
#include <QFormLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QPainter>
#include <QScrollArea>
#include <QSlider>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>

using hotkey_id = size_t;
struct obs_hotkey;
struct obs_hotkey_binding;
struct obs_data;
struct obs_data_array;
struct config;
struct gs_texture;
struct gs_stagesurf;
struct key_combo { uint32_t modifiers; int key; };
enum { REG_FRONTEND, REG_SOURCE, REG_OUTPUT, REG_ENCODER, REG_SERVICE };
enum { OBS_MOD_SHIFT=2, OBS_MOD_CONTROL=4, OBS_MOD_ALT=8, OBS_MOD_COMMAND=128 };

template<class T> static T Proc(HMODULE m, const char *name) { return reinterpret_cast<T>(GetProcAddress(m, name)); }

struct Api {
    HMODULE obs{}, frontend{};
    void (*enum_hotkeys)(bool(*)(void*,hotkey_id,obs_hotkey*),void*){};
    void (*enum_bindings)(bool(*)(void*,size_t,obs_hotkey_binding*),void*){};
    const char *(*hk_name)(const obs_hotkey*){};
    const char *(*hk_desc)(const obs_hotkey*){};
    int (*hk_type)(const obs_hotkey*){};
    void *(*hk_registerer)(const obs_hotkey*){};
    key_combo (*binding_combo)(obs_hotkey_binding*){};
    hotkey_id (*binding_id)(obs_hotkey_binding*){};
    void (*load_bindings)(hotkey_id,key_combo*,size_t){};
    const char *(*key_name)(int){};
    const char *(*source_name)(const void*){};
    void (*enum_sources)(bool(*)(void*,void*),void*){};
    void *(*source_get_ref)(void*){};
    uint32_t (*source_output_flags)(const void*){};
    bool (*source_audio_active)(const void*){};
    float (*source_get_volume)(const void*){};
    void (*source_set_volume)(void*,float){};
    bool (*source_muted)(const void*){};
    void (*source_set_muted)(void*,bool){};
    void *(*weak_source_get)(void*){};
    void (*source_release)(void*){};
    void *(*weak_output_get)(void*){};
    void (*output_release)(void*){};
    obs_data_array *(*hotkey_save)(hotkey_id){};
    obs_data *(*data_create)(){};
    obs_data *(*data_create_json)(const char*){};
    obs_data_array *(*data_get_array)(obs_data*,const char*){};
    void (*data_set_array)(obs_data*,const char*,obs_data_array*){};
    const char *(*data_json)(obs_data*){};
    void (*data_release)(obs_data*){};
    void (*array_release)(obs_data_array*){};
    obs_data *(*save_output)(void*){};
    hotkey_id (*register_frontend)(const char*,const char*,void(*)(void*,hotkey_id,obs_hotkey*,bool),void*){};
    void (*unregister_hotkey)(hotkey_id){};
    void (*hotkey_load)(hotkey_id,obs_data_array*){};
    int (*key_from_name)(const char*){};
    int (*key_from_virtual_key)(int){};
    const char *(*get_locale)(){};
    uint32_t (*get_version)(){};
    gs_texture *(*main_texture)(){};
    void (*add_tick)(void(*)(void*,float),void*){};
    void (*remove_tick)(void(*)(void*,float),void*){};
    void (*enter_graphics)(){};
    void (*leave_graphics)(){};
    uint32_t (*texture_width)(const gs_texture*){};
    uint32_t (*texture_height)(const gs_texture*){};
    int (*texture_format)(const gs_texture*){};
    gs_stagesurf *(*stage_create)(uint32_t,uint32_t,int){};
    void (*stage_destroy)(gs_stagesurf*){};
    void (*stage_texture)(gs_stagesurf*,gs_texture*){};
    bool (*stage_map)(gs_stagesurf*,uint8_t**,uint32_t*){};
    void (*stage_unmap)(gs_stagesurf*){};
    void *(*add_tools)(const char*,void(*)(void*),void*){};
    void *(*add_tools_qaction)(const char*){};
    void *(*main_window)(){};
    HWND (*main_hwnd)(){};
    config *(*profile_config)(){};
    config *(*global_config)(){};
    void (*add_event_callback)(void(*)(int,void*),void*){};
    void (*remove_event_callback)(void(*)(int,void*),void*){};
    void (*frontend_save)(){};
    bool (*studio_mode_active)(){};
    void (*config_set_string)(config*,const char*,const char*,const char*){};
    const char *(*config_get_string)(config*,const char*,const char*){};
    bool (*config_has_user_value)(config*,const char*,const char*){};
    int (*config_save_safe)(config*,const char*,const char*){};
    void (*hotkey_enable_background_press)(bool){};
} api;

struct Hotkey { hotkey_id id{}; std::string name, description, context; int type{}; void *registerer{}; std::vector<key_combo> bindings,originalBindings; };
static std::vector<Hotkey> hotkeys;
static HWND settingsWindow{},apiKeyEdit{},settingsStatus{},descriptionWindow{};
static bool apiKeyPromptDeclined{};
static bool settingsValidationRunning{};
static HINSTANCE instance{};
static QMainWindow *obsMainWindow{};
static QPointer<QDialog> qtHotkeyEditor;
static QPointer<QAction> accessibleToolsAction;
struct CanvasCapture;
enum class CanvasMode {Basic,Detailed,ReadText,PeopleBackgrounds,AnalyzeIssues};
struct CanvasTurn {std::wstring label,text;bool assistant{true};};
struct CanvasWebAction {std::string id;std::wstring label,accessibleLabel;std::vector<std::string> fixes;CanvasMode mode{CanvasMode::Basic};bool capturesNewFrame{false};};
static hotkey_id nextAreaHotkey=static_cast<hotkey_id>(-1),previousAreaHotkey=static_cast<hotkey_id>(-1),focusMediaHotkey=static_cast<hotkey_id>(-1),openAccessibleObsHotkey=static_cast<hotkey_id>(-1),volumeConsoleHotkey=static_cast<hotkey_id>(-1);
static std::array<hotkey_id,5> canvasHotkeys={static_cast<hotkey_id>(-1),static_cast<hotkey_id>(-1),static_cast<hotkey_id>(-1),static_cast<hotkey_id>(-1),static_cast<hotkey_id>(-1)};
static std::array<hotkey_id,6> directAreaHotkeys={static_cast<hotkey_id>(-1),static_cast<hotkey_id>(-1),static_cast<hotkey_id>(-1),static_cast<hotkey_id>(-1),static_cast<hotkey_id>(-1),static_cast<hotkey_id>(-1)};
static std::thread openAIThread;
static std::atomic_bool requestRunning{false},shuttingDown{false};
static Microsoft::WRL::ComPtr<ICoreWebView2Controller> descriptionController;
static Microsoft::WRL::ComPtr<ICoreWebView2> descriptionWebView;
static std::vector<CanvasTurn> descriptionTurns;
static std::vector<CanvasWebAction> canvasWebActions;
static CanvasMode activeCanvasMode{CanvasMode::Basic};
static std::string pendingCanvasActionId;
static std::string currentResponseId;
static bool compatibilityReportMode{};
static std::wstring compatibilityReportText;
static bool activateDescriptionAfterNavigation{};
static bool canvasDocumentReady{},canvasNavigationPending{},pendingDescriptionActivate{},pendingDescriptionAnnouncement{};
static std::atomic<uint64_t> canvasConversationGeneration{};
static std::atomic_bool descriptionUserClosed{};
static bool comInitialized{};
static EventRegistrationToken descriptionMessageToken{},descriptionNavigationToken{},descriptionAcceleratorToken{};
static CanvasCapture *activeCapture{};
static std::mutex captureMutex;
static QPointer<QWidget> canvasReturnFocus;
static QMetaObject::Connection hotkeyFocusConnection;
static QTimer *hotkeyFocusGuardTimer{};
static constexpr const char *NEXT_AREA_NAME="accessible_obs_studio.next_interface_area";
static constexpr const char *PREVIOUS_AREA_NAME="accessible_obs_studio.previous_interface_area";
static constexpr std::array<const char*,5> CANVAS_HOTKEY_NAMES={"accessible_obs_studio.describe_canvas","accessible_obs_studio.describe_canvas_detailed","accessible_obs_studio.read_canvas_text","accessible_obs_studio.describe_people_backgrounds","accessible_obs_studio.analyze_canvas_issues"};
static constexpr const char *FOCUS_MEDIA_NAME="accessible_obs_studio.focus_media_controls";
static constexpr const char *OPEN_ACCESSIBLE_OBS_NAME="accessible_obs_studio.open_accessible_obs_studio";
static constexpr const char *VOLUME_CONSOLE_NAME="accessible_obs_studio.open_volume_console";
static constexpr std::array<const char*,6> DIRECT_AREA_NAMES={"accessible_obs_studio.focus_video_preview","accessible_obs_studio.focus_scenes","accessible_obs_studio.focus_sources","accessible_obs_studio.focus_audio_mixer","accessible_obs_studio.focus_scene_transitions","accessible_obs_studio.focus_controls"};
static constexpr const wchar_t *CREDENTIAL_NAME=L"AccessibleOBSStudio/OpenAI";

enum class UiText {
    Numpad,Control,Alt,Shift,Windows,SourceScene,Output,Encoder,Service,Other,Unassigned,CommandsShown,Selected,
    ChooseKey,SelectCommand,SelectAssignment,RemovedApply,KeyStored,NoKeyStored,ApiKeyLabel,SaveKey,RemoveKey,Close,
    EnterApiKey,KeySaved,KeySaveFailed,ConfirmRemoveKey,KeyRemoved,KeyRemoveFailed,InvalidApiKeyFormat,InvalidApiKey,ApiKeyValidationFailed,ValidatingApiKey,SettingsTitle,CanvasTitle,CanvasHeading,AskFollowup,Send,FollowupHint,
    ConflictMessage,ConflictTitle,AppliedSaved,FindCommands,Commands,Command,Context,Assignments,CurrentAssignments,
    Key,Add,Replace,Remove,Apply,Reload,OpenAISettings,Ready,UnreadableResponse,NoResponse,NetworkInitFailed,
    RequestFailed,EncodeFailed,Status,OpenAIResponse,Error,YourQuestion,RequestingFollowup,NoCanvas,UnsupportedFormat,
    InvalidCanvas,CreateSurfaceFailed,ReadCanvasFailed,FocusPreview,RequestInProgress,NeedSavedKey,Capturing,
    NextArea,PreviousArea,DescribeCanvas,FocusVideoPreview,FocusScenes,FocusSources,FocusAudioMixer,FocusSceneTransitions,FocusControls,PreviewName,HttpStatus,UntestedObsVersion,Count
};

static size_t uiLanguageIndex{};
static size_t DetectLanguage(){const char *raw=api.get_locale?api.get_locale():"en-US";std::string locale=raw?raw:"en-US";std::transform(locale.begin(),locale.end(),locale.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});if(locale.rfind("de",0)==0)return 1;if(locale.rfind("ru",0)==0)return 2;if(locale.rfind("uk",0)==0)return 3;if(locale.rfind("fr",0)==0)return 4;if(locale.rfind("es",0)==0)return 5;return 0;}
static size_t LanguageIndex(){return uiLanguageIndex;}

static const wchar_t *Tr(UiText id){
    using A=std::array<const wchar_t*,6>;static const A t[]={
        A{L"Numpad",L"Nummernblock",L"Цифровая клавиатура",L"Цифрова клавіатура",L"Pavé numérique",L"Teclado numérico"},
        A{L"Control",L"Strg",L"Ctrl",L"Ctrl",L"Contrôle",L"Control"},A{L"Alt",L"Alt",L"Alt",L"Alt",L"Alt",L"Alt"},A{L"Shift",L"Umschalt",L"Shift",L"Shift",L"Maj",L"Mayús"},A{L"Windows",L"Windows",L"Windows",L"Windows",L"Windows",L"Windows"},
        A{L"Source or scene",L"Quelle oder Szene",L"Источник или сцена",L"Джерело або сцена",L"Source ou scène",L"Fuente o escena"},A{L"Output",L"Ausgabe",L"Вывод",L"Виведення",L"Sortie",L"Salida"},A{L"Encoder",L"Encoder",L"Кодировщик",L"Кодувальник",L"Encodeur",L"Codificador"},A{L"Service",L"Dienst",L"Сервис",L"Служба",L"Service",L"Servicio"},A{L"Other",L"Andere",L"Другое",L"Інше",L"Autre",L"Otro"},
        A{L"Unassigned",L"Nicht zugewiesen",L"Не назначено",L"Не призначено",L"Non attribué",L"Sin asignar"},A{L"%1 commands shown.",L"%1 Befehle angezeigt.",L"Показано команд: %1.",L"Показано команд: %1.",L"%1 commandes affichées.",L"Se muestran %1 comandos."},A{L"Selected %1.",L"%1 ausgewählt.",L"Выбрано: %1.",L"Вибрано: %1.",L"%1 sélectionné.",L"Seleccionado: %1."},
        A{L"Choose a key first.",L"Wählen Sie zuerst eine Taste.",L"Сначала выберите клавишу.",L"Спочатку виберіть клавішу.",L"Choisissez d’abord une touche.",L"Elija primero una tecla."},A{L"Select a command first.",L"Wählen Sie zuerst einen Befehl.",L"Сначала выберите команду.",L"Спочатку виберіть команду.",L"Sélectionnez d’abord une commande.",L"Seleccione primero un comando."},A{L"Select an assignment to remove.",L"Wählen Sie eine zu entfernende Zuweisung.",L"Выберите назначение для удаления.",L"Виберіть призначення для видалення.",L"Sélectionnez une attribution à supprimer.",L"Seleccione una asignación para eliminar."},A{L"Removed %1. Choose Apply to save this change.",L"%1 entfernt. Wählen Sie Anwenden, um die Änderung zu speichern.",L"Удалено: %1. Нажмите «Применить», чтобы сохранить изменение.",L"Видалено: %1. Натисніть «Застосувати», щоб зберегти зміну.",L"%1 supprimé. Choisissez Appliquer pour enregistrer.",L"Se eliminó %1. Elija Aplicar para guardar el cambio."},
        A{L"An encrypted API key is stored for this Windows account.",L"Für dieses Windows-Konto ist ein verschlüsselter API-Schlüssel gespeichert.",L"Для этой учётной записи Windows сохранён зашифрованный API-ключ.",L"Для цього облікового запису Windows збережено зашифрований ключ API.",L"Une clé API chiffrée est enregistrée pour ce compte Windows.",L"Hay una clave de API cifrada guardada para esta cuenta de Windows."},A{L"No OpenAI API key is stored.",L"Es ist kein OpenAI-API-Schlüssel gespeichert.",L"Ключ API OpenAI не сохранён.",L"Ключ API OpenAI не збережено.",L"Aucune clé API OpenAI n’est enregistrée.",L"No hay ninguna clave de API de OpenAI guardada."},A{L"OpenAI &API key:",L"OpenAI-&API-Schlüssel:",L"&Ключ API OpenAI:",L"&Ключ API OpenAI:",L"Clé &API OpenAI :",L"Clave de &API de OpenAI:"},A{L"&Save key",L"Schlüssel &speichern",L"&Сохранить ключ",L"&Зберегти ключ",L"&Enregistrer la clé",L"&Guardar clave"},A{L"&Remove key",L"Schlüssel &entfernen",L"&Удалить ключ",L"&Видалити ключ",L"&Supprimer la clé",L"&Eliminar clave"},A{L"&Close",L"S&chließen",L"&Закрыть",L"&Закрити",L"&Fermer",L"&Cerrar"},
        A{L"Enter an API key first.",L"Geben Sie zuerst einen API-Schlüssel ein.",L"Сначала введите API-ключ.",L"Спочатку введіть ключ API.",L"Saisissez d’abord une clé API.",L"Introduzca primero una clave de API."},A{L"The API key was encrypted and saved.",L"Der API-Schlüssel wurde verschlüsselt und gespeichert.",L"API-ключ зашифрован и сохранён.",L"Ключ API зашифровано та збережено.",L"La clé API a été chiffrée et enregistrée.",L"La clave de API se cifró y se guardó."},A{L"Windows could not save the API key.",L"Windows konnte den API-Schlüssel nicht speichern.",L"Windows не удалось сохранить API-ключ.",L"Windows не вдалося зберегти ключ API.",L"Windows n’a pas pu enregistrer la clé API.",L"Windows no pudo guardar la clave de API."},
        A{L"Remove the stored OpenAI API key?",L"Möchten Sie den gespeicherten OpenAI-API-Schlüssel entfernen?",L"Удалить сохранённый API-ключ OpenAI?",L"Видалити збережений ключ API OpenAI?",L"Supprimer la clé API OpenAI enregistrée ?",L"¿Eliminar la clave de API de OpenAI guardada?"},A{L"Your OpenAI API key has been successfully removed.",L"Ihr OpenAI-API-Schlüssel wurde erfolgreich entfernt.",L"Ваш API-ключ OpenAI успешно удалён.",L"Ваш ключ API OpenAI успішно видалено.",L"Votre clé API OpenAI a bien été supprimée.",L"Su clave de API de OpenAI se ha eliminado correctamente."},A{L"Windows could not remove the OpenAI API key.",L"Windows konnte den OpenAI-API-Schlüssel nicht entfernen.",L"Windows не удалось удалить API-ключ OpenAI.",L"Windows не вдалося видалити ключ API OpenAI.",L"Windows n’a pas pu supprimer la clé API OpenAI.",L"Windows no pudo eliminar la clave de API de OpenAI."},
        A{L"The API key format is invalid. Enter a key beginning with sk- and containing no spaces.",L"Das Format des API-Schlüssels ist ungültig. Geben Sie einen Schlüssel ein, der mit sk- beginnt und keine Leerzeichen enthält.",L"Неверный формат API-ключа. Введите ключ, который начинается с sk- и не содержит пробелов.",L"Неправильний формат ключа API. Введіть ключ, який починається з sk- і не містить пробілів.",L"Le format de la clé API n’est pas valide. Saisissez une clé commençant par sk- et ne contenant aucun espace.",L"El formato de la clave de API no es válido. Introduzca una clave que comience por sk- y no contenga espacios."},
        A{L"OpenAI rejected this API key. Check the key and try again.",L"OpenAI hat diesen API-Schlüssel abgelehnt. Prüfen Sie den Schlüssel und versuchen Sie es erneut.",L"OpenAI отклонил этот API-ключ. Проверьте ключ и повторите попытку.",L"OpenAI відхилив цей ключ API. Перевірте ключ і повторіть спробу.",L"OpenAI a refusé cette clé API. Vérifiez-la, puis réessayez.",L"OpenAI rechazó esta clave de API. Compruebe la clave e inténtelo de nuevo."},
        A{L"Accessible OBS Studio could not validate the API key. Check your internet connection and try again.",L"Accessible OBS Studio konnte den API-Schlüssel nicht prüfen. Prüfen Sie Ihre Internetverbindung und versuchen Sie es erneut.",L"Accessible OBS Studio не удалось проверить API-ключ. Проверьте подключение к Интернету и повторите попытку.",L"Accessible OBS Studio не вдалося перевірити ключ API. Перевірте підключення до Інтернету та повторіть спробу.",L"Accessible OBS Studio n’a pas pu vérifier la clé API. Vérifiez votre connexion Internet, puis réessayez.",L"Accessible OBS Studio no pudo validar la clave de API. Compruebe la conexión a Internet e inténtelo de nuevo."},
        A{L"Validating API key...",L"API-Schlüssel wird geprüft...",L"Проверка API-ключа...",L"Перевірка ключа API...",L"Vérification de la clé API...",L"Validando la clave de API..."},A{L"OpenAI Settings",L"OpenAI-Einstellungen",L"Настройки OpenAI",L"Налаштування OpenAI",L"Paramètres OpenAI",L"Configuración de OpenAI"},
        A{L"Canvas Description",L"Canvas-Beschreibung",L"Описание холста",L"Опис полотна",L"Description du canevas",L"Descripción del lienzo"},A{L"Canvas Description",L"Canvas-Beschreibung",L"Описание холста",L"Опис полотна",L"Description du canevas",L"Descripción del lienzo"},A{L"Ask a follow-up question",L"Eine Anschlussfrage stellen",L"Задать уточняющий вопрос",L"Поставити уточнювальне запитання",L"Poser une question complémentaire",L"Hacer una pregunta de seguimiento"},A{L"Send",L"Senden",L"Отправить",L"Надіслати",L"Envoyer",L"Enviar"},A{L"Press Enter in the edit field or activate Send.",L"Drücken Sie im Eingabefeld die Eingabetaste oder aktivieren Sie Senden.",L"Нажмите Enter в поле ввода или активируйте кнопку «Отправить».",L"Натисніть Enter у полі введення або активуйте кнопку «Надіслати».",L"Appuyez sur Entrée dans le champ ou activez Envoyer.",L"Pulse Intro en el campo o active Enviar."},
        A{L"%1 is also assigned to \"%2\". Apply anyway?",L"%1 ist auch \"%2\" zugewiesen. Trotzdem anwenden?",L"%1 также назначено команде «%2». Всё равно применить?",L"%1 також призначено команді «%2». Усе одно застосувати?",L"%1 est aussi attribué à « %2 ». Appliquer quand même ?",L"%1 también está asignado a «%2». ¿Aplicar de todos modos?"},A{L"Keyboard shortcut conflict",L"Konflikt bei Tastenkombinationen",L"Конфликт сочетаний клавиш",L"Конфлікт сполучень клавіш",L"Conflit de raccourcis clavier",L"Conflicto de métodos abreviados de teclado"},A{L"Keyboard shortcut assignments applied and saved.",L"Tastenkombinationen angewendet und gespeichert.",L"Назначения сочетаний клавиш применены и сохранены.",L"Призначення сполучень клавіш застосовано та збережено.",L"Attributions de raccourcis clavier appliquées et enregistrées.",L"Asignaciones de métodos abreviados de teclado aplicadas y guardadas."},
        A{L"&Find commands:",L"Befehle &suchen:",L"&Найти команды:",L"&Знайти команди:",L"&Rechercher des commandes :",L"&Buscar comandos:"},A{L"Commands",L"Befehle",L"Команды",L"Команди",L"Commandes",L"Comandos"},A{L"Command",L"Befehl",L"Команда",L"Команда",L"Commande",L"Comando"},A{L"Context",L"Kontext",L"Контекст",L"Контекст",L"Contexte",L"Contexto"},A{L"Assignments",L"Zuweisungen",L"Назначения",L"Призначення",L"Attributions",L"Asignaciones"},A{L"Current assignments",L"Aktuelle Zuweisungen",L"Текущие назначения",L"Поточні призначення",L"Attributions actuelles",L"Asignaciones actuales"},A{L"Key",L"Taste",L"Клавиша",L"Клавіша",L"Touche",L"Tecla"},A{L"&Add",L"&Hinzufügen",L"&Добавить",L"&Додати",L"&Ajouter",L"&Añadir"},A{L"&Replace",L"&Ersetzen",L"&Заменить",L"&Замінити",L"&Remplacer",L"&Reemplazar"},A{L"&Remove",L"&Entfernen",L"&Удалить",L"&Видалити",L"&Supprimer",L"&Eliminar"},A{L"A&pply",L"An&wenden",L"&Применить",L"&Застосувати",L"A&ppliquer",L"A&plicar"},A{L"&Reload",L"&Neu laden",L"&Обновить",L"&Оновити",L"&Recharger",L"&Recargar"},A{L"OpenAI &Settings",L"OpenAI-&Einstellungen",L"&Настройки OpenAI",L"&Налаштування OpenAI",L"&Paramètres OpenAI",L"&Configuración de OpenAI"},A{L"Ready",L"Bereit",L"Готово",L"Готово",L"Prêt",L"Listo"},
        A{L"OpenAI returned a response that Accessible OBS Studio could not read.",L"OpenAI hat eine Antwort zurückgegeben, die Accessible OBS Studio nicht lesen konnte.",L"OpenAI вернул ответ, который Accessible OBS Studio не удалось прочитать.",L"OpenAI повернув відповідь, яку Accessible OBS Studio не вдалося прочитати.",L"OpenAI a renvoyé une réponse illisible par Accessible OBS Studio.",L"OpenAI devolvió una respuesta que Accessible OBS Studio no pudo leer."},A{L"OpenAI returned no response text.",L"OpenAI hat keinen Antworttext zurückgegeben.",L"OpenAI не вернул текст ответа.",L"OpenAI не повернув текст відповіді.",L"OpenAI n’a renvoyé aucun texte de réponse.",L"OpenAI no devolvió texto de respuesta."},A{L"Accessible OBS Studio could not initialize the network connection.",L"Accessible OBS Studio konnte die Netzwerkverbindung nicht initialisieren.",L"Accessible OBS Studio не удалось инициализировать сетевое подключение.",L"Accessible OBS Studio не вдалося ініціалізувати мережеве з’єднання.",L"Accessible OBS Studio n’a pas pu initialiser la connexion réseau.",L"Accessible OBS Studio no pudo inicializar la conexión de red."},A{L"The OpenAI request failed. Check the internet connection and the stored API key.",L"Die OpenAI-Anfrage ist fehlgeschlagen. Prüfen Sie Internetverbindung und gespeicherten API-Schlüssel.",L"Запрос OpenAI не выполнен. Проверьте подключение к Интернету и сохранённый API-ключ.",L"Запит OpenAI не виконано. Перевірте підключення до Інтернету та збережений ключ API.",L"La requête OpenAI a échoué. Vérifiez la connexion Internet et la clé API enregistrée.",L"La solicitud a OpenAI falló. Compruebe Internet y la clave de API guardada."},A{L"Accessible OBS Studio could not encode the canvas image.",L"Accessible OBS Studio konnte das Canvas-Bild nicht codieren.",L"Accessible OBS Studio не удалось закодировать изображение холста.",L"Accessible OBS Studio не вдалося закодувати зображення полотна.",L"Accessible OBS Studio n’a pas pu encoder l’image du canevas.",L"Accessible OBS Studio no pudo codificar la imagen del lienzo."},
        A{L"Status",L"Status",L"Состояние",L"Стан",L"État",L"Estado"},A{L"OpenAI response",L"OpenAI-Antwort",L"Ответ OpenAI",L"Відповідь OpenAI",L"Réponse OpenAI",L"Respuesta de OpenAI"},A{L"Error",L"Fehler",L"Ошибка",L"Помилка",L"Erreur",L"Error"},A{L"Your question",L"Ihre Frage",L"Ваш вопрос",L"Ваше запитання",L"Votre question",L"Su pregunta"},A{L"Requesting a follow-up response...",L"Anschlussantwort wird angefordert...",L"Запрашивается уточняющий ответ...",L"Запитується уточнювальна відповідь...",L"Demande d’une réponse complémentaire...",L"Solicitando una respuesta de seguimiento..."},
        A{L"OBS has no rendered canvas image available.",L"OBS hat kein gerendertes Canvas-Bild verfügbar.",L"В OBS нет доступного отрисованного изображения холста.",L"В OBS немає доступного відтвореного зображення полотна.",L"OBS ne dispose d’aucune image de canevas rendue.",L"OBS no dispone de una imagen renderizada del lienzo."},A{L"This canvas color format is not supported for description in this build.",L"Dieses Canvas-Farbformat wird in diesem Build nicht unterstützt.",L"Этот цветовой формат холста не поддерживается в данной сборке.",L"Цей колірний формат полотна не підтримується в цій збірці.",L"Ce format de couleur du canevas n’est pas pris en charge.",L"Este formato de color del lienzo no es compatible con esta compilación."},A{L"The OBS canvas has an invalid size.",L"Das OBS-Canvas hat eine ungültige Größe.",L"Холст OBS имеет недопустимый размер.",L"Полотно OBS має неприпустимий розмір.",L"Le canevas OBS a une taille non valide.",L"El lienzo de OBS tiene un tamaño no válido."},A{L"Accessible OBS Studio could not create a canvas capture surface.",L"Accessible OBS Studio konnte keine Canvas-Aufnahmefläche erstellen.",L"Accessible OBS Studio не удалось создать поверхность захвата холста.",L"Accessible OBS Studio не вдалося створити поверхню захоплення полотна.",L"Accessible OBS Studio n’a pas pu créer la surface de capture.",L"Accessible OBS Studio no pudo crear una superficie de captura del lienzo."},A{L"Accessible OBS Studio could not read the rendered canvas.",L"Accessible OBS Studio konnte das gerenderte Canvas nicht lesen.",L"Accessible OBS Studio не удалось прочитать отрисованный холст.",L"Accessible OBS Studio не вдалося прочитати відтворене полотно.",L"Accessible OBS Studio n’a pas pu lire le canevas rendu.",L"Accessible OBS Studio no pudo leer el lienzo renderizado."},
        A{L"Move focus to Video Preview before using Describe current canvas.",L"Setzen Sie den Fokus auf die Videovorschau, bevor Sie das aktuelle Canvas beschreiben.",L"Перед описанием холста переместите фокус на предпросмотр видео.",L"Перед описом полотна перемістіть фокус на попередній перегляд відео.",L"Placez le focus sur l’aperçu vidéo avant de décrire le canevas.",L"Mueva el foco a la vista previa de vídeo antes de describir el lienzo."},A{L"A canvas description request is already in progress.",L"Eine Anfrage zur Canvas-Beschreibung läuft bereits.",L"Запрос описания холста уже выполняется.",L"Запит опису полотна вже виконується.",L"Une demande de description est déjà en cours.",L"Ya hay una solicitud de descripción en curso."},A{L"Enter and save an OpenAI API key before requesting a canvas description.",L"Geben Sie einen OpenAI-API-Schlüssel ein und speichern Sie ihn, bevor Sie eine Beschreibung anfordern.",L"Введите и сохраните API-ключ OpenAI перед запросом описания холста.",L"Введіть і збережіть ключ API OpenAI перед запитом опису полотна.",L"Saisissez et enregistrez une clé API OpenAI avant de demander une description.",L"Introduzca y guarde una clave de API de OpenAI antes de solicitar una descripción."},A{L"Capturing the OBS canvas and requesting a description...",L"OBS-Canvas wird erfasst und Beschreibung angefordert...",L"Захват холста OBS и запрос описания...",L"Захоплення полотна OBS і запит опису...",L"Capture du canevas OBS et demande de description...",L"Capturando el lienzo de OBS y solicitando una descripción..."},
        A{L"Accessible OBS Studio: Next interface area",L"Accessible OBS Studio: Nächster Oberflächenbereich",L"Accessible OBS Studio: Следующая область интерфейса",L"Accessible OBS Studio: Наступна область інтерфейсу",L"Accessible OBS Studio : Zone d’interface suivante",L"Accessible OBS Studio: Siguiente área de la interfaz"},A{L"Accessible OBS Studio: Previous interface area",L"Accessible OBS Studio: Vorheriger Oberflächenbereich",L"Accessible OBS Studio: Предыдущая область интерфейса",L"Accessible OBS Studio: Попередня область інтерфейсу",L"Accessible OBS Studio : Zone d’interface précédente",L"Accessible OBS Studio: Área anterior de la interfaz"},A{L"Accessible OBS Studio: Describe current canvas",L"Accessible OBS Studio: Aktuelles Canvas beschreiben",L"Accessible OBS Studio: Описать текущий холст",L"Accessible OBS Studio: Описати поточне полотно",L"Accessible OBS Studio : Décrire le canevas actuel",L"Accessible OBS Studio: Describir el lienzo actual"},
        A{L"Accessible OBS Studio: Focus Video Preview",L"Accessible OBS Studio: Videovorschau fokussieren",L"Accessible OBS Studio: Перейти к предпросмотру видео",L"Accessible OBS Studio: Перейти до попереднього перегляду відео",L"Accessible OBS Studio : Placer le focus sur l’aperçu vidéo",L"Accessible OBS Studio: Enfocar la vista previa de vídeo"},A{L"Accessible OBS Studio: Focus Scenes",L"Accessible OBS Studio: Szenen fokussieren",L"Accessible OBS Studio: Перейти к сценам",L"Accessible OBS Studio: Перейти до сцен",L"Accessible OBS Studio : Placer le focus sur les scènes",L"Accessible OBS Studio: Enfocar las escenas"},A{L"Accessible OBS Studio: Focus Sources",L"Accessible OBS Studio: Quellen fokussieren",L"Accessible OBS Studio: Перейти к источникам",L"Accessible OBS Studio: Перейти до джерел",L"Accessible OBS Studio : Placer le focus sur les sources",L"Accessible OBS Studio: Enfocar las fuentes"},A{L"Accessible OBS Studio: Focus Audio Mixer",L"Accessible OBS Studio: Audiomixer fokussieren",L"Accessible OBS Studio: Перейти к микшеру аудио",L"Accessible OBS Studio: Перейти до аудіомікшера",L"Accessible OBS Studio : Placer le focus sur le mélangeur audio",L"Accessible OBS Studio: Enfocar el mezclador de audio"},A{L"Accessible OBS Studio: Focus Scene Transitions",L"Accessible OBS Studio: Szenenübergänge fokussieren",L"Accessible OBS Studio: Перейти к переходам между сценами",L"Accessible OBS Studio: Перейти до переходів між сценами",L"Accessible OBS Studio : Placer le focus sur les transitions de scènes",L"Accessible OBS Studio: Enfocar las transiciones de escena"},A{L"Accessible OBS Studio: Focus Controls",L"Accessible OBS Studio: Steuerung fokussieren",L"Accessible OBS Studio: Перейти к управлению",L"Accessible OBS Studio: Перейти до елементів керування",L"Accessible OBS Studio : Placer le focus sur les commandes",L"Accessible OBS Studio: Enfocar los controles"},
        A{L"Video Preview",L"Videovorschau",L"Предпросмотр видео",L"Попередній перегляд відео",L"Aperçu vidéo",L"Vista previa de vídeo"},A{L"OpenAI returned HTTP status %1.",L"OpenAI hat den HTTP-Status %1 zurückgegeben.",L"OpenAI вернул состояние HTTP %1.",L"OpenAI повернув стан HTTP %1.",L"OpenAI a renvoyé l’état HTTP %1.",L"OpenAI devolvió el estado HTTP %1."},A{L"Accessible OBS Studio has not been tested with OBS Studio %1. Continuing may cause missing features, instability, or crashes. Do you want to load the plugin anyway?",L"Accessible OBS Studio wurde nicht mit OBS Studio %1 getestet. Das Fortfahren kann zu fehlenden Funktionen, Instabilität oder Abstürzen führen. Möchten Sie das Plugin trotzdem laden?",L"Accessible OBS Studio не тестировался с OBS Studio %1. Продолжение может привести к отсутствию функций, нестабильности или сбоям. Всё равно загрузить модуль?",L"Accessible OBS Studio не тестувався з OBS Studio %1. Продовження може спричинити відсутність функцій, нестабільність або збої. Усе одно завантажити модуль?",L"Accessible OBS Studio n’a pas été testé avec OBS Studio %1. Continuer peut entraîner des fonctions manquantes, une instabilité ou des plantages. Voulez-vous tout de même charger le module ?",L"Accessible OBS Studio no se ha probado con OBS Studio %1. Continuar puede provocar funciones ausentes, inestabilidad o bloqueos. ¿Desea cargar el complemento de todos modos?"}
    };static_assert(std::size(t)==static_cast<size_t>(UiText::Count));return t[static_cast<size_t>(id)][LanguageIndex()];
}

static std::wstring TrFormat(UiText id,const std::wstring &first,const std::wstring &second=L""){std::wstring value=Tr(id);size_t p=value.find(L"%1");if(p!=std::wstring::npos)value.replace(p,2,first);p=value.find(L"%2");if(p!=std::wstring::npos)value.replace(p,2,second);return value;}

#include "src/localized_ui.cpp"
#include "src/canvas_localization.cpp"

enum { ID_SEARCH=100, ID_COMMANDS, ID_BINDINGS, ID_SHIFT, ID_CONTROL, ID_ALT, ID_WINDOWS, ID_KEY,
       ID_ADD, ID_REPLACE, ID_REMOVE, ID_APPLY, ID_RELOAD, ID_OPENAI_SETTINGS, ID_CLOSE,
       ID_API_KEY=200, ID_SAVE_KEY, ID_REMOVE_KEY, ID_NO_API_KEY, ID_SETTINGS_CLOSE };

static std::wstring Wide(const char *s) {
    if (!s) return L""; int n=MultiByteToWideChar(CP_UTF8,0,s,-1,nullptr,0); if(n<=1)return L"";
    std::wstring w(static_cast<size_t>(n),L'\0'); MultiByteToWideChar(CP_UTF8,0,s,-1,w.data(),n); w.pop_back(); return w;
}
static std::string Narrow(const std::wstring &w) {
    int n=WideCharToMultiByte(CP_UTF8,0,w.c_str(),-1,nullptr,0,nullptr,nullptr); if(n<=1)return "";
    std::string s(static_cast<size_t>(n),'\0'); WideCharToMultiByte(CP_UTF8,0,w.c_str(),-1,s.data(),n,nullptr,nullptr); s.pop_back(); return s;
}
static std::wstring FriendlyKey(const char *raw) {
    std::wstring s=Wide(raw); if (s.rfind(L"OBS_KEY_",0)==0) s.erase(0,8);
    std::replace(s.begin(),s.end(),L'_',L' ');
    if(s.rfind(L"NUM",0)==0) s=std::wstring(Tr(UiText::Numpad))+L" "+s.substr(3);
    return s;
}
static std::wstring ComboText(key_combo c) {
    std::wstring s; if(c.modifiers&OBS_MOD_CONTROL)s+=std::wstring(Tr(UiText::Control))+L"+"; if(c.modifiers&OBS_MOD_ALT)s+=std::wstring(Tr(UiText::Alt))+L"+";
    if(c.modifiers&OBS_MOD_SHIFT)s+=std::wstring(Tr(UiText::Shift))+L"+"; if(c.modifiers&OBS_MOD_COMMAND)s+=std::wstring(Tr(UiText::Windows))+L"+";
    s+=FriendlyKey(api.key_name(c.key)); return s;
}
static const wchar_t *TypeText(int t) {if(t==0)return L"OBS";if(t==1)return Tr(UiText::SourceScene);if(t==2)return Tr(UiText::Output);if(t==3)return Tr(UiText::Encoder);if(t==4)return Tr(UiText::Service);return Tr(UiText::Other);}

static bool EnumHotkey(void*,hotkey_id id,obs_hotkey *h) {
    Hotkey k; k.id=id; k.name=api.hk_name(h)?api.hk_name(h):""; k.description=api.hk_desc(h)?api.hk_desc(h):k.name;
    k.type=api.hk_type(h); k.registerer=api.hk_registerer(h); k.context=Narrow(TypeText(k.type));
    if(k.type==REG_SOURCE&&k.registerer&&api.weak_source_get&&api.source_name&&api.source_release){
        void *source=api.weak_source_get(k.registerer);
        if(source){const char*n=api.source_name(source);if(n&&*n)k.context=n;api.source_release(source);}
    }
    hotkeys.push_back(std::move(k)); return true;
}
static bool EnumBinding(void*,size_t,obs_hotkey_binding *b) {
    hotkey_id id=api.binding_id(b); auto it=std::find_if(hotkeys.begin(),hotkeys.end(),[&](auto&h){return h.id==id;});
    if(it!=hotkeys.end()) it->bindings.push_back(api.binding_combo(b)); return true;
}
static void LoadData() {
    hotkeys.clear(); api.enum_hotkeys(EnumHotkey,nullptr); api.enum_bindings(EnumBinding,nullptr);
    std::sort(hotkeys.begin(),hotkeys.end(),[](const Hotkey&a,const Hotkey&b){return _stricmp(a.description.c_str(),b.description.c_str())<0;});for(auto &hotkey:hotkeys)hotkey.originalBindings=hotkey.bindings;
}
static void ShowWindowPreservingState(HWND window){if(!window)return;if(IsIconic(window))ShowWindow(window,SW_RESTORE);else if(!IsWindowVisible(window))ShowWindow(window,IsZoomed(window)?SW_SHOWMAXIMIZED:SW_SHOW);}
static bool ActivateKeyboardWindow(HWND window,HWND initialFocus=nullptr){
    if(!window)return false;ShowWindowPreservingState(window);HWND windowRoot=GetAncestor(window,GA_ROOT),foregroundRoot=GetAncestor(GetForegroundWindow(),GA_ROOT);if(windowRoot&&windowRoot==foregroundRoot){if(initialFocus&&IsWindowEnabled(initialFocus)&&GetFocus()!=initialFocus)SetFocus(initialFocus);return true;}BringWindowToTop(window);BOOL foreground=SetForegroundWindow(window);SetActiveWindow(window);
    bool active=foreground!=FALSE||GetAncestor(GetForegroundWindow(),GA_ROOT)==GetAncestor(window,GA_ROOT);if(active){if(initialFocus&&IsWindowEnabled(initialFocus))SetFocus(initialFocus);return true;}
    FLASHWINFO flash{sizeof(flash),window,FLASHW_TRAY|FLASHW_TIMERNOFG,3,0};FlashWindowEx(&flash);return false;
}
static HWND FocusedControlWithin(HWND owner){HWND focus=GetFocus();return owner&&focus&&(focus==owner||IsChild(owner,focus))?focus:nullptr;}
static void RestoreOwnerFocus(HWND owner,HWND previousFocus=nullptr){if(!owner||!IsWindow(owner))return;EnableWindow(owner,TRUE);HWND target=previousFocus&&IsWindow(previousFocus)&&IsWindowEnabled(previousFocus)&&(previousFocus==owner||IsChild(owner,previousFocus))?previousFocus:nullptr;HWND ownerRoot=GetAncestor(owner,GA_ROOT),foregroundRoot=GetAncestor(GetForegroundWindow(),GA_ROOT);if(ownerRoot&&ownerRoot==foregroundRoot){if(!target||GetFocus()==target)return;SetActiveWindow(owner);SetFocus(target);return;}ActivateKeyboardWindow(owner,target);}
static void CloseOwnedWindow(HWND window){HWND owner=GetWindow(window,GW_OWNER);if(owner&&IsWindow(owner))EnableWindow(owner,TRUE);DestroyWindow(window);}

static bool ValidApiKeyFormat(const std::string &key);
static void EraseString(std::string &value){if(!value.empty())SecureZeroMemory(value.data(),value.size());value.clear();}
struct SecureWideText{
    std::wstring value;
    SecureWideText()=default;SecureWideText(const SecureWideText&)=delete;SecureWideText& operator=(const SecureWideText&)=delete;
    SecureWideText(SecureWideText &&other) noexcept:value(std::move(other.value)){}SecureWideText& operator=(SecureWideText &&other) noexcept{if(this!=&other){if(!value.empty())SecureZeroMemory(value.data(),value.size()*sizeof(wchar_t));value=std::move(other.value);}return *this;}
    ~SecureWideText(){if(!value.empty())SecureZeroMemory(value.data(),value.size()*sizeof(wchar_t));}
};
static SecureWideText AuthorizationHeader(const std::string &key,bool json){SecureWideText header;header.value.reserve(key.size()+64);header.value=L"Authorization: Bearer ";for(unsigned char c:key)header.value.push_back(static_cast<wchar_t>(c));if(json)header.value+=L"\r\nContent-Type: application/json";else header.value+=L"\r\n";return header;}
static void DisableRequestRedirects(HINTERNET request){if(!request)return;DWORD policy=WINHTTP_OPTION_REDIRECT_POLICY_NEVER;WinHttpSetOption(request,WINHTTP_OPTION_REDIRECT_POLICY,&policy,sizeof(policy));}

static bool ReadApiKey(std::string &key){
    PCREDENTIALW credential=nullptr;if(!CredReadW(CREDENTIAL_NAME,CRED_TYPE_GENERIC,0,&credential))return false;
    key.assign(reinterpret_cast<const char*>(credential->CredentialBlob),credential->CredentialBlobSize);if(credential->CredentialBlob&&credential->CredentialBlobSize)SecureZeroMemory(credential->CredentialBlob,credential->CredentialBlobSize);CredFree(credential);if(!ValidApiKeyFormat(key)){EraseString(key);return false;}return true;
}

static bool StoredApiKeyExists(){std::string key;bool stored=ReadApiKey(key);if(!key.empty())SecureZeroMemory(key.data(),key.size());return stored;}

static bool ApiKeyFieldHasValue(HWND edit){if(!edit)return false;wchar_t value[512]{};GetWindowTextW(edit,value,static_cast<int>(std::size(value)));bool hasValue=std::any_of(std::begin(value),std::end(value),[](wchar_t c){return c&&!iswspace(c);});SecureZeroMemory(value,sizeof(value));return hasValue;}

static bool SaveApiKey(const std::string &key){
    CREDENTIALW credential{};credential.Type=CRED_TYPE_GENERIC;credential.TargetName=const_cast<wchar_t*>(CREDENTIAL_NAME);credential.CredentialBlobSize=static_cast<DWORD>(key.size());credential.CredentialBlob=reinterpret_cast<LPBYTE>(const_cast<char*>(key.data()));credential.Persist=CRED_PERSIST_LOCAL_MACHINE;credential.UserName=const_cast<wchar_t*>(L"OpenAI API key");
    return CredWriteW(&credential,0)!=FALSE;
}

static LRESULT CALLBACK ControlProc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);

static bool ValidApiKeyFormat(const std::string &key){
    if(key.size()<20||key.rfind("sk-",0)!=0)return false;return std::all_of(key.begin(),key.end(),[](unsigned char c){return c>=33&&c<=126;});
}

enum class ApiKeyValidation{Valid,Invalid,ConnectionFailed};
static ApiKeyValidation ValidateApiKeyOnline(const std::string &key){
    HINTERNET session=WinHttpOpen(L"Accessible OBS Studio/1.0.1",WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0);if(!session)return ApiKeyValidation::ConnectionFailed;WinHttpSetTimeouts(session,10000,10000,10000,15000);
    HINTERNET connection=WinHttpConnect(session,L"api.openai.com",INTERNET_DEFAULT_HTTPS_PORT,0);HINTERNET request=connection?WinHttpOpenRequest(connection,L"GET",L"/v1/models",nullptr,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,WINHTTP_FLAG_SECURE):nullptr;DisableRequestRedirects(request);SecureWideText authorization=AuthorizationHeader(key,false);BOOL sent=request?WinHttpSendRequest(request,authorization.value.c_str(),static_cast<DWORD>(authorization.value.size()),WINHTTP_NO_REQUEST_DATA,0,0,0):FALSE;if(!authorization.value.empty())SecureZeroMemory(authorization.value.data(),authorization.value.size()*sizeof(wchar_t));BOOL received=sent?WinHttpReceiveResponse(request,nullptr):FALSE;DWORD status=0,size=sizeof(status);if(received)WinHttpQueryHeaders(request,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,WINHTTP_HEADER_NAME_BY_INDEX,&status,&size,WINHTTP_NO_HEADER_INDEX);if(request)WinHttpCloseHandle(request);if(connection)WinHttpCloseHandle(connection);WinHttpCloseHandle(session);if(!received)return ApiKeyValidation::ConnectionFailed;if((status>=200&&status<300)||status==403||status==429)return ApiKeyValidation::Valid;if(status==401)return ApiKeyValidation::Invalid;return ApiKeyValidation::ConnectionFailed;
}

static void SetWindowTextIfChanged(HWND control,const wchar_t *text){if(!control)return;int length=GetWindowTextLengthW(control);std::wstring current(static_cast<size_t>(length)+1,L'\0');GetWindowTextW(control,current.data(),length+1);current.resize(static_cast<size_t>(length));if(current!=text)SetWindowTextW(control,text);}
static void UpdateSettingsStatus(){bool stored=StoredApiKeyExists();SetWindowTextIfChanged(settingsStatus,Tr(stored?UiText::KeyStored:UiText::NoKeyStored));HWND window=settingsWindow?settingsWindow:(settingsStatus?GetParent(settingsStatus):nullptr),save=window?GetDlgItem(window,ID_SAVE_KEY):nullptr,remove=window?GetDlgItem(window,ID_REMOVE_KEY):nullptr,noKey=window?GetDlgItem(window,ID_NO_API_KEY):nullptr;if(save)EnableWindow(save,!settingsValidationRunning&&ApiKeyFieldHasValue(apiKeyEdit));if(remove)ShowWindow(remove,stored?SW_SHOW:SW_HIDE);if(noKey)ShowWindow(noKey,stored?SW_HIDE:SW_SHOW);}

static ApiKeyValidation ValidateApiKeyWithProgress(HWND window,const std::string &key){
    settingsValidationRunning=true;SetWindowText(settingsStatus,Tr(UiText::ValidatingApiKey));for(int id:{ID_API_KEY,ID_SAVE_KEY,ID_REMOVE_KEY,ID_NO_API_KEY,ID_SETTINGS_CLOSE})EnableWindow(GetDlgItem(window,id),FALSE);SetCursor(LoadCursor(nullptr,IDC_WAIT));HANDLE completed=CreateEvent(nullptr,TRUE,FALSE,nullptr);ApiKeyValidation result=ApiKeyValidation::ConnectionFailed;std::thread worker([&]{result=ValidateApiKeyOnline(key);SetEvent(completed);});for(;;){DWORD wait=MsgWaitForMultipleObjects(1,&completed,FALSE,250,QS_ALLINPUT);if(wait==WAIT_OBJECT_0)break;MSG message{};while(PeekMessage(&message,nullptr,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessage(&message);}}worker.join();CloseHandle(completed);SetCursor(LoadCursor(nullptr,IDC_ARROW));for(int id:{ID_API_KEY,ID_SAVE_KEY,ID_REMOVE_KEY,ID_NO_API_KEY,ID_SETTINGS_CLOSE})EnableWindow(GetDlgItem(window,id),TRUE);settingsValidationRunning=false;UpdateSettingsStatus();return result;
}

static LRESULT CALLBACK SettingsProc(HWND w,UINT m,WPARAM wp,LPARAM lp){
    if(m==WM_CREATE){HFONT font=(HFONT)GetStockObject(DEFAULT_GUI_FONT);auto add=[&](HWND c){SendMessage(c,WM_SETFONT,(WPARAM)font,TRUE);if(GetWindowLongPtr(c,GWL_STYLE)&WS_TABSTOP)SetWindowSubclass(c,ControlProc,2,0);};
        add(CreateWindow(L"STATIC",Tr(UiText::ApiKeyLabel),WS_CHILD|WS_VISIBLE,12,16,130,22,w,nullptr,instance,nullptr));apiKeyEdit=CreateWindowEx(WS_EX_CLIENTEDGE,L"EDIT",L"",WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL|ES_PASSWORD,145,12,520,26,w,(HMENU)ID_API_KEY,instance,nullptr);add(apiKeyEdit);
        add(CreateWindow(L"BUTTON",Tr(UiText::SaveKey),WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_DEFPUSHBUTTON,12,52,115,30,w,(HMENU)ID_SAVE_KEY,instance,nullptr));add(CreateWindow(L"BUTTON",Tr(UiText::RemoveKey),WS_CHILD|WS_VISIBLE|WS_TABSTOP,137,52,125,30,w,(HMENU)ID_REMOVE_KEY,instance,nullptr));std::wstring noKeyText=LWide(LocalText::NoKeyYet);add(CreateWindow(L"BUTTON",noKeyText.c_str(),WS_CHILD|WS_VISIBLE|WS_TABSTOP,272,52,160,30,w,(HMENU)ID_NO_API_KEY,instance,nullptr));add(CreateWindow(L"BUTTON",Tr(UiText::Close),WS_CHILD|WS_VISIBLE|WS_TABSTOP,550,52,115,30,w,(HMENU)ID_SETTINGS_CLOSE,instance,nullptr));
        settingsStatus=CreateWindow(L"STATIC",L"",WS_CHILD|WS_VISIBLE,12,96,653,42,w,nullptr,instance,nullptr);add(settingsStatus);UpdateSettingsStatus();return 0;}
    if(m==WM_COMMAND&&LOWORD(wp)==ID_SAVE_KEY){wchar_t value[512]{};GetWindowText(apiKeyEdit,value,512);std::wstring entered=value;while(!entered.empty()&&iswspace(entered.front()))entered.erase(entered.begin());while(!entered.empty()&&iswspace(entered.back()))entered.pop_back();std::string key=Narrow(entered);if(key.empty()){SecureZeroMemory(value,sizeof(value));MessageBox(w,Tr(UiText::EnterApiKey),L"Accessible OBS Studio",MB_OK|MB_ICONERROR);SetFocus(apiKeyEdit);return 0;}if(!ValidApiKeyFormat(key)){SecureZeroMemory(key.data(),key.size());SecureZeroMemory(entered.data(),entered.size()*sizeof(wchar_t));SecureZeroMemory(value,sizeof(value));MessageBox(w,Tr(UiText::InvalidApiKeyFormat),L"Accessible OBS Studio",MB_OK|MB_ICONERROR);SetFocus(apiKeyEdit);SendMessage(apiKeyEdit,EM_SETSEL,0,-1);return 0;}ApiKeyValidation validation=ValidateApiKeyWithProgress(w,key);if(validation!=ApiKeyValidation::Valid){SecureZeroMemory(key.data(),key.size());SecureZeroMemory(entered.data(),entered.size()*sizeof(wchar_t));SecureZeroMemory(value,sizeof(value));MessageBox(w,Tr(validation==ApiKeyValidation::Invalid?UiText::InvalidApiKey:UiText::ApiKeyValidationFailed),L"Accessible OBS Studio",MB_OK|MB_ICONERROR);SetFocus(apiKeyEdit);SendMessage(apiKeyEdit,EM_SETSEL,0,-1);return 0;}bool ok=SaveApiKey(key);if(ok)apiKeyPromptDeclined=false;SecureZeroMemory(key.data(),key.size());SecureZeroMemory(entered.data(),entered.size()*sizeof(wchar_t));SecureZeroMemory(value,sizeof(value));if(ok)SetWindowText(apiKeyEdit,L"");UpdateSettingsStatus();MessageBox(w,Tr(ok?UiText::KeySaved:UiText::KeySaveFailed),L"Accessible OBS Studio",MB_OK|(ok?MB_ICONINFORMATION:MB_ICONERROR));if(!ok){SetFocus(apiKeyEdit);SendMessage(apiKeyEdit,EM_SETSEL,0,-1);}return 0;}
    if(m==WM_COMMAND&&LOWORD(wp)==ID_API_KEY&&HIWORD(wp)==EN_CHANGE){HWND save=GetDlgItem(w,ID_SAVE_KEY);if(save)EnableWindow(save,!settingsValidationRunning&&ApiKeyFieldHasValue(apiKeyEdit));return 0;}
    if(m==WM_COMMAND&&LOWORD(wp)==ID_REMOVE_KEY){if(MessageBox(w,Tr(UiText::ConfirmRemoveKey),L"Accessible OBS Studio",MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2)!=IDYES)return 0;BOOL removed=CredDeleteW(CREDENTIAL_NAME,CRED_TYPE_GENERIC,0);DWORD error=removed?ERROR_SUCCESS:GetLastError();if(removed||error==ERROR_NOT_FOUND){apiKeyPromptDeclined=false;SetWindowText(apiKeyEdit,L"");MessageBox(w,Tr(UiText::KeyRemoved),L"Accessible OBS Studio",MB_OK|MB_ICONINFORMATION);UpdateSettingsStatus();SetFocus(apiKeyEdit);}else MessageBox(w,Tr(UiText::KeyRemoveFailed),L"Accessible OBS Studio",MB_OK|MB_ICONERROR);return 0;}
    if(m==WM_COMMAND&&LOWORD(wp)==ID_NO_API_KEY){apiKeyPromptDeclined=true;CloseOwnedWindow(w);std::wstring message=LWide(LocalText::NoKeyInformation);MessageBox(api.main_hwnd?api.main_hwnd():nullptr,message.c_str(),L"Accessible OBS Studio",MB_OK|MB_ICONINFORMATION);return 0;}
    if(m==WM_COMMAND&&(LOWORD(wp)==ID_SETTINGS_CLOSE||LOWORD(wp)==IDCANCEL)){if(settingsValidationRunning)MessageBeep(MB_ICONWARNING);else CloseOwnedWindow(w);return 0;}if(m==WM_KEYDOWN&&wp==VK_ESCAPE){if(settingsValidationRunning)MessageBeep(MB_ICONWARNING);else CloseOwnedWindow(w);return 0;}if(m==WM_CLOSE){if(settingsValidationRunning){MessageBeep(MB_ICONWARNING);return 0;}CloseOwnedWindow(w);return 0;}if(m==WM_DESTROY){settingsWindow=nullptr;apiKeyEdit=nullptr;settingsStatus=nullptr;return 0;}return DefWindowProc(w,m,wp,lp);
}

static void ShowClassicSettings(const wchar_t *status=nullptr){
    if(settingsWindow){if(status)SetWindowText(settingsStatus,status);ActivateKeyboardWindow(settingsWindow,apiKeyEdit);return;}
    WNDCLASSEX wc{sizeof(wc)};wc.lpfnWndProc=SettingsProc;wc.hInstance=instance;wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);wc.lpszClassName=L"AccessibleOBSStudioSettings";RegisterClassEx(&wc);
    HWND owner=api.main_hwnd?api.main_hwnd():nullptr,returnFocus=FocusedControlWithin(owner);std::wstring title=L"Accessible OBS Studio - "+std::wstring(Tr(UiText::SettingsTitle));settingsWindow=CreateWindowEx(WS_EX_DLGMODALFRAME|WS_EX_CONTROLPARENT,wc.lpszClassName,title.c_str(),WS_POPUP|WS_CAPTION|WS_SYSMENU,CW_USEDEFAULT,CW_USEDEFAULT,700,190,owner,nullptr,instance,nullptr);if(!settingsWindow)return;if(status)SetWindowText(settingsStatus,status);if(owner)EnableWindow(owner,FALSE);HWND dialog=settingsWindow;ActivateKeyboardWindow(dialog,apiKeyEdit);MSG message{};bool quit=false;while(IsWindow(dialog)){BOOL result=GetMessage(&message,nullptr,0,0);if(result<=0){quit=result==0;break;}if(!IsDialogMessage(dialog,&message)){TranslateMessage(&message);DispatchMessage(&message);}}RestoreOwnerFocus(owner,returnFocus);if(quit)PostQuitMessage(static_cast<int>(message.wParam));
}

class ApiSettingsDialog final:public QDialog{
protected:
    void reject() override{if(settingsValidationRunning){QApplication::beep();return;}QDialog::reject();}
    void closeEvent(QCloseEvent *event) override{if(settingsValidationRunning){QApplication::beep();event->ignore();return;}QDialog::closeEvent(event);}
public:
    using QDialog::QDialog;
};

static void ShowSettings(const wchar_t *initialStatus=nullptr){
    if(!QApplication::instance()||!obsMainWindow){ShowClassicSettings(initialStatus);return;}
    QWidget *parent=QApplication::activeWindow();if(!parent)parent=obsMainWindow;
    ApiSettingsDialog dialog(parent);dialog.setWindowTitle(QStringLiteral("Accessible OBS Studio - ")+QString::fromWCharArray(Tr(UiText::SettingsTitle)));dialog.setWindowModality(Qt::WindowModal);settingsValidationRunning=false;
    auto *layout=new QVBoxLayout(&dialog);auto *form=new QFormLayout;auto *keyEdit=new QLineEdit(&dialog);keyEdit->setEchoMode(QLineEdit::Password);keyEdit->setMaxLength(511);keyEdit->setAccessibleName(QString::fromWCharArray(Tr(UiText::ApiKeyLabel)).remove(u'&'));
    auto *keyLabel=new QLabel(QString::fromWCharArray(Tr(UiText::ApiKeyLabel)),&dialog);keyLabel->setBuddy(keyEdit);form->addRow(keyLabel,keyEdit);layout->addLayout(form);
    auto *statusLabel=new QLabel(&dialog);statusLabel->setWordWrap(true);statusLabel->setAccessibleName(LText(LocalText::ApiKeyStatus));layout->addWidget(statusLabel);auto *progress=new QProgressBar(&dialog);progress->setRange(0,0);progress->setAccessibleName(QString::fromWCharArray(Tr(UiText::ValidatingApiKey)));progress->hide();layout->addWidget(progress);
    auto *buttons=new QDialogButtonBox(&dialog);auto *save=buttons->addButton(QString::fromWCharArray(Tr(UiText::SaveKey)),QDialogButtonBox::AcceptRole);auto *remove=buttons->addButton(QString::fromWCharArray(Tr(UiText::RemoveKey)),QDialogButtonBox::DestructiveRole);auto *noKey=buttons->addButton(LText(LocalText::NoKeyYet),QDialogButtonBox::ActionRole);auto *close=buttons->addButton(QString::fromWCharArray(Tr(UiText::Close)),QDialogButtonBox::RejectRole);layout->addWidget(buttons);QWidget::setTabOrder(keyEdit,save);QWidget::setTabOrder(save,remove);QWidget::setTabOrder(remove,noKey);QWidget::setTabOrder(noKey,close);
    auto updateState=[&](const wchar_t *status=nullptr){bool stored=StoredApiKeyExists(),idle=!settingsValidationRunning;QString statusText=QString::fromWCharArray(status?status:Tr(stored?UiText::KeyStored:UiText::NoKeyStored));if(statusLabel->text()!=statusText)statusLabel->setText(statusText);remove->setVisible(stored);noKey->setVisible(!stored);keyEdit->setEnabled(idle);save->setEnabled(idle&&!keyEdit->text().trimmed().isEmpty());remove->setEnabled(idle&&stored);noKey->setEnabled(idle&&!stored);close->setEnabled(idle);QString description=LText(stored?LocalText::StoredKeyDescription:LocalText::EmptyKeyDescription);if(keyEdit->accessibleDescription()!=description)keyEdit->setAccessibleDescription(description);return stored;};updateState(initialStatus);
    QObject::connect(keyEdit,&QLineEdit::textChanged,[&]{save->setEnabled(!settingsValidationRunning&&!keyEdit->text().trimmed().isEmpty());});
    QObject::connect(close,&QPushButton::clicked,&dialog,&QDialog::reject);
    QObject::connect(remove,&QPushButton::clicked,[&]{if(QMessageBox::question(&dialog,QStringLiteral("Accessible OBS Studio"),QString::fromWCharArray(Tr(UiText::ConfirmRemoveKey)),QMessageBox::Yes|QMessageBox::No,QMessageBox::No)!=QMessageBox::Yes)return;BOOL removed=CredDeleteW(CREDENTIAL_NAME,CRED_TYPE_GENERIC,0);DWORD error=removed?ERROR_SUCCESS:GetLastError();if(removed||error==ERROR_NOT_FOUND){apiKeyPromptDeclined=false;keyEdit->clear();QMessageBox::information(&dialog,QStringLiteral("Accessible OBS Studio"),QString::fromWCharArray(Tr(UiText::KeyRemoved)));updateState();keyEdit->setFocus(Qt::OtherFocusReason);}else{QMessageBox::critical(&dialog,QStringLiteral("Accessible OBS Studio"),QString::fromWCharArray(Tr(UiText::KeyRemoveFailed)));updateState();remove->setFocus(Qt::OtherFocusReason);}});
    QObject::connect(noKey,&QPushButton::clicked,[&]{apiKeyPromptDeclined=true;dialog.accept();QMessageBox::information(parent,QStringLiteral("Accessible OBS Studio"),LText(LocalText::NoKeyInformation));});
    QObject::connect(save,&QPushButton::clicked,[&]{
        QString entered=keyEdit->text().trimmed();QByteArray utf8=entered.toUtf8();std::string key(utf8.constData(),static_cast<size_t>(utf8.size()));
        auto clearSecrets=[&]{if(!key.empty())SecureZeroMemory(key.data(),key.size());if(!utf8.isEmpty())SecureZeroMemory(utf8.data(),static_cast<size_t>(utf8.size()));entered.fill(u'\0');};
        if(key.empty()){clearSecrets();QMessageBox::critical(&dialog,QStringLiteral("Accessible OBS Studio"),QString::fromWCharArray(Tr(UiText::EnterApiKey)));keyEdit->setFocus(Qt::OtherFocusReason);return;}
        if(!ValidApiKeyFormat(key)){clearSecrets();QMessageBox::critical(&dialog,QStringLiteral("Accessible OBS Studio"),QString::fromWCharArray(Tr(UiText::InvalidApiKeyFormat)));keyEdit->setFocus(Qt::OtherFocusReason);keyEdit->selectAll();return;}
        settingsValidationRunning=true;progress->show();updateState(Tr(UiText::ValidatingApiKey));QApplication::setOverrideCursor(Qt::WaitCursor);
        std::atomic_bool completed{false};ApiKeyValidation validation=ApiKeyValidation::ConnectionFailed;std::thread worker([&]{validation=ValidateApiKeyOnline(key);completed.store(true,std::memory_order_release);});QEventLoop waitLoop;QTimer poll;QObject::connect(&poll,&QTimer::timeout,[&]{if(completed.load(std::memory_order_acquire))waitLoop.quit();});poll.start(50);waitLoop.exec();worker.join();poll.stop();QApplication::restoreOverrideCursor();progress->hide();settingsValidationRunning=false;updateState();
        if(validation!=ApiKeyValidation::Valid){clearSecrets();QMessageBox::critical(&dialog,QStringLiteral("Accessible OBS Studio"),QString::fromWCharArray(Tr(validation==ApiKeyValidation::Invalid?UiText::InvalidApiKey:UiText::ApiKeyValidationFailed)));keyEdit->setFocus(Qt::OtherFocusReason);keyEdit->selectAll();return;}
        bool ok=SaveApiKey(key);if(ok)apiKeyPromptDeclined=false;clearSecrets();if(ok)keyEdit->clear();updateState();if(ok)QMessageBox::information(&dialog,QStringLiteral("Accessible OBS Studio"),QString::fromWCharArray(Tr(UiText::KeySaved)));else{QMessageBox::critical(&dialog,QStringLiteral("Accessible OBS Studio"),QString::fromWCharArray(Tr(UiText::KeySaveFailed)));keyEdit->setFocus(Qt::OtherFocusReason);keyEdit->selectAll();}
    });
    QObject::connect(&dialog,&QDialog::finished,[keyEdit]{QString overwrite(keyEdit->text().size(),QChar(u'X'));keyEdit->setText(overwrite);overwrite.fill(u'\0');keyEdit->clear();});
    dialog.resize(660,230);keyEdit->setFocus(Qt::OtherFocusReason);dialog.exec();
}

static bool RequireApiKey(std::string &key){
    if(ReadApiKey(key))return true;
    if(apiKeyPromptDeclined){QMessageBox::information(obsMainWindow,QStringLiteral("Accessible OBS Studio"),LText(LocalText::ApiKeyRequired));return false;}
    ShowSettings(Tr(UiText::NeedSavedKey));return false;
}

static void SendFollowup(const std::wstring &question);
static void HandleCanvasAction(const std::string &id);
static void ShowSuggestedFixes(const std::vector<std::string> &allowed={});

static CanvasText CanvasModeText(CanvasMode mode){switch(mode){case CanvasMode::Basic:return CanvasText::BasicDescription;case CanvasMode::Detailed:return CanvasText::DetailedDescription;case CanvasMode::ReadText:return CanvasText::ReadText;case CanvasMode::PeopleBackgrounds:return CanvasText::PeopleBackgrounds;case CanvasMode::AnalyzeIssues:return CanvasText::AnalyzeIssues;}return CanvasText::BasicDescription;}

static std::wstring HtmlEscape(const std::wstring &text){
    std::wstring escaped;escaped.reserve(text.size());
    for(wchar_t c:text){if(c==L'&')escaped+=L"&amp;";else if(c==L'<')escaped+=L"&lt;";else if(c==L'>')escaped+=L"&gt;";else if(c==L'\"')escaped+=L"&quot;";else escaped+=c;}return escaped;
}

static std::wstring DescriptionHtml(){
    std::wstring locale=HtmlEscape(Wide(api.get_locale?api.get_locale():"en-US"));
    if(compatibilityReportMode){std::wstring compatibilityTitle=LWide(LocalText::CompatibilityAnalysis);std::wstring html=L"<!doctype html><html lang=\""+locale+L"\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width\"><title>Accessible OBS Studio - "+HtmlEscape(compatibilityTitle)+L"</title><style>body{font-family:Segoe UI,sans-serif;font-size:1rem;line-height:1.55;margin:1.25rem;max-width:75rem}.report{white-space:pre-wrap}h1{font-size:1.6rem}</style></head><body><main><h1 id=\"page-title\" tabindex=\"-1\">"+HtmlEscape(compatibilityTitle)+L"</h1><div class=\"report\">"+HtmlEscape(compatibilityReportText)+L"</div></main></body></html>";return html;}
    return L"<!doctype html><html lang=\""+locale+L"\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width\"><title>Accessible OBS Studio - "+HtmlEscape(CWText(CanvasModeText(activeCanvasMode)))+L"</title><style>:root{color-scheme:light dark}body{font-family:Segoe UI,sans-serif;font-size:1rem;line-height:1.5;margin:1.25rem;max-width:70rem;background:Canvas;color:CanvasText}main{display:block}section{border-bottom:1px solid GrayText;padding-bottom:.75rem;margin-bottom:1rem}.text{white-space:pre-wrap}.actions{display:flex;flex-wrap:wrap;gap:.5rem;margin:.75rem 0 1rem}form{position:sticky;bottom:0;background:Canvas;padding:1rem 0;border-top:2px solid CanvasText}label{display:block;font-weight:600;margin-bottom:.35rem}input{box-sizing:border-box;font:inherit;width:calc(100% - 7rem);padding:.45rem;background:Field;color:FieldText;border:1px solid GrayText}button{font:inherit;padding:.45rem 1rem;background:ButtonFace;color:ButtonText;border:1px solid ButtonBorder;margin:0}.hint{font-size:.9rem}</style></head><body><main id=\"canvas-results\" tabindex=\"-1\"><h1 id=\"page-title\">"+HtmlEscape(CWText(CanvasModeText(activeCanvasMode)))+L"</h1><div id=\"conversation\"></div><div id=\"actions\" class=\"actions\"></div><form id=\"follow-up-form\"><label for=\"follow-up\">"+HtmlEscape(Tr(UiText::AskFollowup))+L"</label><input id=\"follow-up\" name=\"follow-up\" type=\"text\" maxlength=\"500\" autocomplete=\"off\" disabled><button id=\"send-follow-up\" type=\"submit\" disabled>"+HtmlEscape(Tr(UiText::Send))+L"</button><p class=\"hint\">"+HtmlEscape(Tr(UiText::FollowupHint))+L"</p></form></main><script>const conversation=document.getElementById('conversation'),actions=document.getElementById('actions'),input=document.getElementById('follow-up'),send=document.getElementById('send-follow-up');document.getElementById('follow-up-form').addEventListener('submit',function(e){e.preventDefault();const q=input.value.trim();if(q){input.value='';window.chrome.webview.postMessage('followup:'+q);}});window.chrome.webview.addEventListener('message',function(e){const data=e.data;if(!data||data.type!=='render')return;while(conversation.children.length>data.turns.length)conversation.lastElementChild.remove();data.turns.forEach(function(turn,index){let section=conversation.children[index],heading,body;if(!section){section=document.createElement('section');heading=document.createElement('h2');body=document.createElement('div');body.className='text';section.append(heading,body);conversation.appendChild(section);}else{heading=section.children[0];body=section.children[1];}heading.hidden=!turn.label;if(turn.label&&heading.textContent!==turn.label)heading.textContent=turn.label;body.removeAttribute('role');body.removeAttribute('aria-atomic');if(body.textContent!==turn.text)body.textContent=turn.text;if(index===data.announceIndex&&turn.kind==='assistant'){body.setAttribute('role','alert');body.setAttribute('aria-atomic','true');}section.id=index===data.turns.length-1?'latest-response':'';});actions.replaceChildren();data.actions.forEach(function(action){const button=document.createElement('button');button.type='button';button.textContent=action.label;if(action.accessibleLabel)button.setAttribute('aria-label',action.accessibleLabel);button.disabled=!!data.disabled;button.addEventListener('click',function(){window.chrome.webview.postMessage('action:'+action.id);});actions.appendChild(button);});input.disabled=!!data.disabled;send.disabled=!!data.disabled;const latest=document.getElementById('latest-response');if(latest)latest.scrollIntoView({block:'start'});});</script></body></html>";
}

static void PushDescriptionState(bool announceLatest){
    if(!descriptionWebView||!canvasDocumentReady)return;QJsonArray turns;int announceIndex=-1;
    for(size_t i=0;i<descriptionTurns.size();++i){const auto &turn=descriptionTurns[i];QString kind=turn.assistant?QStringLiteral("assistant"):QStringLiteral("status");turns.append(QJsonObject{{QStringLiteral("kind"),kind},{QStringLiteral("label"),QString::fromStdWString(turn.label)},{QStringLiteral("text"),QString::fromStdWString(turn.text)}});if(announceLatest&&i+1==descriptionTurns.size()&&turn.assistant)announceIndex=static_cast<int>(i);}
    QJsonArray actions;for(const auto &action:canvasWebActions)actions.append(QJsonObject{{QStringLiteral("id"),QString::fromStdString(action.id)},{QStringLiteral("label"),QString::fromStdWString(action.label)},{QStringLiteral("accessibleLabel"),QString::fromStdWString(action.accessibleLabel)}});
    QJsonObject payload{{QStringLiteral("type"),QStringLiteral("render")},{QStringLiteral("turns"),turns},{QStringLiteral("actions"),actions},{QStringLiteral("announceIndex"),announceIndex},{QStringLiteral("disabled"),requestRunning||currentResponseId.empty()}};std::wstring json=QString::fromUtf8(QJsonDocument(payload).toJson(QJsonDocument::Compact)).toStdWString();descriptionWebView->PostWebMessageAsJson(json.c_str());
}

static void RenderDescription(bool activate,bool announceLatest=false){
    if(!descriptionWebView)return;if(compatibilityReportMode){canvasDocumentReady=false;canvasNavigationPending=false;activateDescriptionAfterNavigation=activate;std::wstring html=DescriptionHtml();descriptionWebView->NavigateToString(html.c_str());return;}
    pendingDescriptionActivate|=activate;pendingDescriptionAnnouncement|=announceLatest;if(canvasDocumentReady){bool shouldActivate=pendingDescriptionActivate,shouldAnnounce=pendingDescriptionAnnouncement;pendingDescriptionActivate=pendingDescriptionAnnouncement=false;if(shouldActivate&&descriptionController&&ActivateKeyboardWindow(descriptionWindow))descriptionController->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);PushDescriptionState(shouldAnnounce);return;}if(!canvasNavigationPending){canvasNavigationPending=true;std::wstring html=DescriptionHtml();descriptionWebView->NavigateToString(html.c_str());}
}

static void ResizeDescriptionWebView(){if(descriptionController&&descriptionWindow){RECT bounds{};GetClientRect(descriptionWindow,&bounds);descriptionController->put_Bounds(bounds);}}

static constexpr UINT WM_DESCRIPTION_WEBVIEW_FAILED=WM_APP+41;
static void QueueDescriptionWebViewFailure(HWND parent){if(parent&&IsWindow(parent))PostMessage(parent,WM_DESCRIPTION_WEBVIEW_FAILED,0,0);}

static void InitializeDescriptionWebView(HWND parent){
    wchar_t localAppData[MAX_PATH]{};GetEnvironmentVariable(L"LOCALAPPDATA",localAppData,MAX_PATH);std::wstring userData=std::wstring(localAppData)+L"\\AccessibleOBSStudio\\WebView2";
    using Microsoft::WRL::Callback;
    HRESULT createResult=CreateCoreWebView2EnvironmentWithOptions(nullptr,userData.c_str(),nullptr,Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([parent](HRESULT result,ICoreWebView2Environment *environment)->HRESULT{
        if(FAILED(result)||!environment){QueueDescriptionWebViewFailure(parent);return result;}
        HRESULT controllerCreateResult=environment->CreateCoreWebView2Controller(parent,Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([parent](HRESULT controllerResult,ICoreWebView2Controller *controller)->HRESULT{
            if(FAILED(controllerResult)||!controller){QueueDescriptionWebViewFailure(parent);return controllerResult;}descriptionController=controller;HRESULT webViewResult=controller->get_CoreWebView2(&descriptionWebView);if(FAILED(webViewResult)||!descriptionWebView){QueueDescriptionWebViewFailure(parent);return FAILED(webViewResult)?webViewResult:E_FAIL;}ResizeDescriptionWebView();
            Microsoft::WRL::ComPtr<ICoreWebView2Settings> settings;if(SUCCEEDED(descriptionWebView->get_Settings(&settings))){settings->put_AreDefaultContextMenusEnabled(FALSE);settings->put_AreDevToolsEnabled(FALSE);settings->put_IsStatusBarEnabled(FALSE);settings->put_IsWebMessageEnabled(TRUE);}
            descriptionWebView->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>([](ICoreWebView2*,ICoreWebView2WebMessageReceivedEventArgs *args)->HRESULT{LPWSTR message=nullptr;if(SUCCEEDED(args->TryGetWebMessageAsString(&message))&&message){std::wstring value=message;CoTaskMemFree(message);constexpr wchar_t followupPrefix[]=L"followup:",actionPrefix[]=L"action:";if(value.rfind(followupPrefix,0)==0)SendFollowup(value.substr(std::size(followupPrefix)-1));else if(value.rfind(actionPrefix,0)==0)HandleCanvasAction(Narrow(value.substr(std::size(actionPrefix)-1)));}return S_OK;}).Get(),&descriptionMessageToken);
            descriptionWebView->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>([](ICoreWebView2*,ICoreWebView2NavigationCompletedEventArgs *args)->HRESULT{BOOL success=FALSE;if(!args||FAILED(args->get_IsSuccess(&success))||!success){QueueDescriptionWebViewFailure(descriptionWindow);return S_OK;}if(compatibilityReportMode){if(activateDescriptionAfterNavigation&&descriptionController){activateDescriptionAfterNavigation=false;if(ActivateKeyboardWindow(descriptionWindow))descriptionController->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);}return S_OK;}canvasDocumentReady=true;canvasNavigationPending=false;bool shouldActivate=pendingDescriptionActivate,shouldAnnounce=pendingDescriptionAnnouncement;pendingDescriptionActivate=pendingDescriptionAnnouncement=false;if(shouldActivate&&descriptionController&&ActivateKeyboardWindow(descriptionWindow))descriptionController->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);PushDescriptionState(shouldAnnounce);return S_OK;}).Get(),&descriptionNavigationToken);
            descriptionController->add_AcceleratorKeyPressed(Callback<ICoreWebView2AcceleratorKeyPressedEventHandler>([](ICoreWebView2Controller*,ICoreWebView2AcceleratorKeyPressedEventArgs *args)->HRESULT{UINT key=0;COREWEBVIEW2_KEY_EVENT_KIND kind{};if(SUCCEEDED(args->get_VirtualKey(&key))&&SUCCEEDED(args->get_KeyEventKind(&kind))&&key==VK_ESCAPE&&(kind==COREWEBVIEW2_KEY_EVENT_KIND_KEY_DOWN||kind==COREWEBVIEW2_KEY_EVENT_KIND_SYSTEM_KEY_DOWN)){args->put_Handled(TRUE);if(descriptionWindow)PostMessage(descriptionWindow,WM_CLOSE,0,0);}return S_OK;}).Get(),&descriptionAcceleratorToken);
            bool activate=IsWindowVisible(parent)&&(compatibilityReportMode||!descriptionUserClosed);RenderDescription(activate,activate&&!compatibilityReportMode&&!descriptionTurns.empty());return S_OK;
        }).Get());if(FAILED(controllerCreateResult))QueueDescriptionWebViewFailure(parent);return controllerCreateResult;
    }).Get());if(FAILED(createResult))QueueDescriptionWebViewFailure(parent);
}

static LRESULT CALLBACK DescriptionProc(HWND w,UINT m,WPARAM wp,LPARAM lp){
    if(m==WM_CREATE){InitializeDescriptionWebView(w);return 0;}if(m==WM_SIZE){ResizeDescriptionWebView();return 0;}if(m==WM_KEYDOWN&&wp==VK_ESCAPE){PostMessage(w,WM_CLOSE,0,0);return 0;}
    if(m==WM_DESCRIPTION_WEBVIEW_FAILED){if(IsWindowVisible(w)){std::wstring message=LWide(LocalText::WebViewFailure);MessageBox(w,message.c_str(),L"Accessible OBS Studio",MB_OK|MB_ICONERROR);}DestroyWindow(w);return 0;}
    if(m==WM_CLOSE){HWND foreground=GetForegroundWindow(),owner=api.main_hwnd?api.main_hwnd():nullptr;bool restore=foreground&&(GetAncestor(foreground,GA_ROOT)==w);if(compatibilityReportMode&&owner)EnableWindow(owner,TRUE);if(!compatibilityReportMode){descriptionUserClosed=true;++canvasConversationGeneration;}ShowWindow(w,SW_HIDE);currentResponseId.clear();descriptionTurns.clear();canvasWebActions.clear();pendingCanvasActionId.clear();pendingDescriptionActivate=pendingDescriptionAnnouncement=false;activateDescriptionAfterNavigation=false;if(restore&&obsMainWindow&&!compatibilityReportMode){if(ActivateKeyboardWindow(owner)&&canvasReturnFocus&&QApplication::focusWidget()!=canvasReturnFocus)canvasReturnFocus->setFocus(Qt::OtherFocusReason);}if(!compatibilityReportMode)canvasReturnFocus.clear();return 0;}if(m==WM_DESTROY){if(descriptionWebView){descriptionWebView->remove_WebMessageReceived(descriptionMessageToken);descriptionWebView->remove_NavigationCompleted(descriptionNavigationToken);}descriptionWebView.Reset();if(descriptionController){descriptionController->remove_AcceleratorKeyPressed(descriptionAcceleratorToken);descriptionController->Close();descriptionController.Reset();}descriptionWindow=nullptr;canvasDocumentReady=canvasNavigationPending=pendingDescriptionActivate=pendingDescriptionAnnouncement=false;return 0;}return DefWindowProc(w,m,wp,lp);
}

static void ShowDescription(const std::wstring &text,bool activate){
    compatibilityReportMode=false;
    std::wstring title=L"Accessible OBS Studio - "+CWText(CanvasModeText(activeCanvasMode));if(!descriptionWindow){WNDCLASSEX wc{sizeof(wc)};wc.lpfnWndProc=DescriptionProc;wc.hInstance=instance;wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);wc.lpszClassName=L"AccessibleOBSStudioDescription";RegisterClassEx(&wc);descriptionWindow=CreateWindowEx(WS_EX_APPWINDOW|WS_EX_CONTROLPARENT,wc.lpszClassName,title.c_str(),WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,750,480,nullptr,nullptr,instance,nullptr);}if(!descriptionWindow){std::wstring message=LWide(LocalText::CanvasWindowFailure);MessageBox(api.main_hwnd?api.main_hwnd():nullptr,message.c_str(),L"Accessible OBS Studio",MB_OK|MB_ICONERROR);return;}SetWindowText(descriptionWindow,title.c_str());
    descriptionTurns.clear();descriptionTurns.push_back({L"",text,true});canvasDocumentReady=false;canvasNavigationPending=false;ShowWindowPreservingState(descriptionWindow);RenderDescription(activate,true);
}

static bool CopyTextToClipboard(const std::wstring &text){if(!OpenClipboard(descriptionWindow))return false;EmptyClipboard();SIZE_T bytes=(text.size()+1)*sizeof(wchar_t);HGLOBAL memory=GlobalAlloc(GMEM_MOVEABLE,bytes);if(!memory){CloseClipboard();return false;}void *target=GlobalLock(memory);if(!target){GlobalFree(memory);CloseClipboard();return false;}memcpy(target,text.c_str(),bytes);GlobalUnlock(memory);if(!SetClipboardData(CF_UNICODETEXT,memory)){GlobalFree(memory);CloseClipboard();return false;}CloseClipboard();return true;}
static void ShowCompatibilityReport(const std::wstring &text){
    compatibilityReportMode=true;compatibilityReportText=text;canvasReturnFocus=QApplication::focusWidget();std::wstring title=L"Accessible OBS Studio - "+LWide(LocalText::CompatibilityAnalysis);if(!descriptionWindow){WNDCLASSEX wc{sizeof(wc)};wc.lpfnWndProc=DescriptionProc;wc.hInstance=instance;wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);wc.lpszClassName=L"AccessibleOBSStudioDescription";RegisterClassEx(&wc);descriptionWindow=CreateWindowEx(WS_EX_APPWINDOW|WS_EX_CONTROLPARENT,wc.lpszClassName,title.c_str(),WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,800,600,nullptr,nullptr,instance,nullptr);}if(!descriptionWindow){std::wstring failure=LWide(LocalText::CompatibilityWindowFailure);MessageBox(api.main_hwnd?api.main_hwnd():nullptr,failure.c_str(),L"Accessible OBS Studio",MB_OK|MB_ICONERROR);compatibilityReportMode=false;compatibilityReportText.clear();canvasReturnFocus.clear();return;}SetWindowText(descriptionWindow,title.c_str());ShowWindowPreservingState(descriptionWindow);RenderDescription(true);bool copied=CopyTextToClipboard(text);if(!copied){std::wstring failure=LWide(LocalText::ClipboardFailure);MessageBox(descriptionWindow,failure.c_str(),L"Accessible OBS Studio",MB_OK|MB_ICONWARNING);}HWND owner=api.main_hwnd?api.main_hwnd():nullptr;if(owner)EnableWindow(owner,FALSE);MSG message{};while(IsWindowVisible(descriptionWindow)&&GetMessage(&message,nullptr,0,0)>0){TranslateMessage(&message);DispatchMessage(&message);}RestoreOwnerFocus(owner);if(canvasReturnFocus)canvasReturnFocus->setFocus(Qt::OtherFocusReason);canvasReturnFocus.clear();compatibilityReportMode=false;compatibilityReportText.clear();
}

static bool Persist(Hotkey &h,bool &configurationChanged){
    api.load_bindings(h.id,h.bindings.empty()?nullptr:h.bindings.data(),h.bindings.size());
    config *cfg=api.profile_config?api.profile_config():nullptr;
    if(cfg&&h.type==REG_FRONTEND){obs_data_array*a=api.hotkey_save(h.id);obs_data*d=api.data_create();if(!a||!d){if(d)api.data_release(d);if(a)api.array_release(a);return false;}api.data_set_array(d,"bindings",a);const char *json=api.data_json(d);if(!json){api.data_release(d);api.array_release(a);return false;}api.config_set_string(cfg,"Hotkeys",h.name.c_str(),json);api.data_release(d);api.array_release(a);configurationChanged=true;}
    else if(cfg&&h.type==REG_OUTPUT&&api.save_output&&api.weak_output_get&&api.output_release&&h.registerer){
        void *output=api.weak_output_get(h.registerer);
        if(!output)return false;obs_data*d=api.save_output(output);if(!d){api.output_release(output);return false;}const char *json=api.data_json(d);if(!json){api.data_release(d);api.output_release(output);return false;}api.config_set_string(cfg,"Hotkeys","ReplayBuffer",json);api.data_release(d);api.output_release(output);configurationChanged=true;
    }
    return true;
}
static LRESULT CALLBACK ControlProc(HWND w,UINT m,WPARAM wp,LPARAM lp,UINT_PTR,DWORD_PTR){
    if((m==WM_KEYDOWN||m==WM_SYSKEYDOWN)&&wp==VK_ESCAPE){HWND root=GetAncestor(w,GA_ROOT);if(root)SendMessage(root,WM_CLOSE,0,0);return 0;}
    if((m==WM_CHAR&&(wp==VK_ESCAPE||wp==VK_RETURN))||(m==WM_SYSCHAR&&wp==VK_ESCAPE))return 0;
    if(m==WM_KEYDOWN&&wp==VK_RETURN){HWND root=GetAncestor(w,GA_ROOT);if(root==settingsWindow&&w==apiKeyEdit){SendMessage(root,WM_COMMAND,ID_SAVE_KEY,0);return 0;}wchar_t className[16]{};GetClassName(w,className,static_cast<int>(std::size(className)));if(_wcsicmp(className,L"Button")==0){SendMessage(w,BM_CLICK,0,0);return 0;}}
    if(m==WM_KEYDOWN&&wp==VK_TAB){BOOL previous=(GetKeyState(VK_SHIFT)&0x8000)!=0;HWND root=GetAncestor(w,GA_ROOT);HWND next=GetNextDlgTabItem(root,w,previous);if(next){SetFocus(next);return 0;}}
    return DefSubclassProc(w,m,wp,lp);
}
struct ChoiceDialogState{const wchar_t *instruction{};const wchar_t *content{};std::array<const wchar_t*,3> labels{};int defaultChoice{3};int result{3};};
static LRESULT CALLBACK ChoiceDialogProc(HWND window,UINT message,WPARAM wParam,LPARAM lParam){
    ChoiceDialogState *state=reinterpret_cast<ChoiceDialogState*>(GetWindowLongPtr(window,GWLP_USERDATA));
    if(message==WM_NCCREATE){state=reinterpret_cast<ChoiceDialogState*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);SetWindowLongPtr(window,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(state));}
    if(message==WM_CREATE&&state){HFONT font=reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));HWND instruction=CreateWindow(L"STATIC",state->instruction,WS_CHILD|WS_VISIBLE,20,18,520,46,window,nullptr,instance,nullptr);HWND content=CreateWindow(L"STATIC",state->content,WS_CHILD|WS_VISIBLE,20,70,520,70,window,nullptr,instance,nullptr);SendMessage(instruction,WM_SETFONT,reinterpret_cast<WPARAM>(font),TRUE);SendMessage(content,WM_SETFONT,reinterpret_cast<WPARAM>(font),TRUE);for(int i=0;i<3;++i){DWORD style=WS_CHILD|WS_VISIBLE|WS_TABSTOP|(i+1==state->defaultChoice?BS_DEFPUSHBUTTON:BS_PUSHBUTTON);HWND button=CreateWindow(L"BUTTON",state->labels[static_cast<size_t>(i)],style,80+i*145,155,130,32,window,reinterpret_cast<HMENU>(static_cast<INT_PTR>(1101+i)),instance,nullptr);SendMessage(button,WM_SETFONT,reinterpret_cast<WPARAM>(font),TRUE);}SetFocus(GetDlgItem(window,1100+state->defaultChoice));return 0;}
    if(message==WM_COMMAND&&state){int id=LOWORD(wParam);if(id>=1101&&id<=1103){state->result=id-1100;CloseOwnedWindow(window);return 0;}if(id==IDCANCEL){state->result=3;CloseOwnedWindow(window);return 0;}}
    if(message==WM_KEYDOWN&&wParam==VK_ESCAPE){if(state)state->result=3;CloseOwnedWindow(window);return 0;}
    if(message==WM_CLOSE){if(state)state->result=3;CloseOwnedWindow(window);return 0;}
    return DefWindowProc(window,message,wParam,lParam);
}
static int ShowClassicChoiceDialog(HWND owner,const wchar_t *title,const wchar_t *instruction,const wchar_t *content,const std::array<const wchar_t*,3> &labels,int defaultChoice){
    static bool registered=false;if(!registered){WNDCLASSEX wc{sizeof(wc)};wc.lpfnWndProc=ChoiceDialogProc;wc.hInstance=instance;wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);wc.lpszClassName=L"AccessibleOBSStudioChoiceDialog";registered=RegisterClassEx(&wc)!=0||GetLastError()==ERROR_CLASS_ALREADY_EXISTS;}
    ChoiceDialogState state{instruction,content,labels,defaultChoice,3};HWND returnFocus=FocusedControlWithin(owner);RECT area{};if(owner)GetWindowRect(owner,&area);else SystemParametersInfo(SPI_GETWORKAREA,0,&area,0);int width=580,height=245,x=area.left+((area.right-area.left)-width)/2,y=area.top+((area.bottom-area.top)-height)/2;if(owner)EnableWindow(owner,FALSE);HWND window=CreateWindowEx(WS_EX_DLGMODALFRAME|WS_EX_CONTROLPARENT,wcscmp(title,L"")==0?L"AccessibleOBSStudioChoiceDialog":L"AccessibleOBSStudioChoiceDialog",title,WS_POPUP|WS_CAPTION|WS_SYSMENU,x,y,width,height,owner,nullptr,instance,&state);if(window){ShowWindow(window,SW_SHOW);SetForegroundWindow(window);MSG msg{};while(IsWindow(window)&&GetMessage(&msg,nullptr,0,0)>0){if(!IsDialogMessage(window,&msg)){TranslateMessage(&msg);DispatchMessage(&msg);}}}RestoreOwnerFocus(owner,returnFocus);return state.result;
}

static int ShowChoiceDialog(HWND owner,const wchar_t *title,const wchar_t *instruction,const wchar_t *content,const std::array<const wchar_t*,3> &labels,int defaultChoice){
    if(!QApplication::instance()||!obsMainWindow)return ShowClassicChoiceDialog(owner,title,instruction,content,labels,defaultChoice);
    QWidget *parent=QApplication::activeWindow();if(!parent)parent=obsMainWindow;QMessageBox box(QMessageBox::Warning,QString::fromWCharArray(title),QString::fromWCharArray(instruction),QMessageBox::NoButton,parent);box.setInformativeText(QString::fromWCharArray(content));std::array<QAbstractButton*,3> buttons{};for(size_t i=0;i<buttons.size();++i)buttons[i]=box.addButton(QString::fromWCharArray(labels[i]),i==2?QMessageBox::RejectRole:QMessageBox::ActionRole);if(defaultChoice>=1&&defaultChoice<=3)box.setDefaultButton(qobject_cast<QPushButton*>(buttons[static_cast<size_t>(defaultChoice-1)]));box.exec();for(size_t i=0;i<buttons.size();++i)if(box.clickedButton()==buttons[i])return static_cast<int>(i+1);return 3;
}

static void ShowQtHotkeyEditor();
static bool InitializeAccessibleToolsAction();
static bool AllowObsToManageHotkeysOutsideObs();
static bool SaveObsHotkeyManagementPreference(bool allow);

#include "src/shortcut_editor.cpp"

#include "src/qt_interface.cpp"

#include "src/focus_navigation.cpp"

#include "src/volume_console.cpp"

#include "src/canvas_openai.cpp"

#include "src/compatibility.cpp"

static bool LoadApi(){api.obs=GetModuleHandle(L"obs.dll");api.frontend=GetModuleHandle(L"obs-frontend-api.dll");if(!api.obs||!api.frontend)return false;
#define O(field,type,name) api.field=Proc<type>(api.obs,name)
#define F(field,type,name) api.field=Proc<type>(api.frontend,name)
    O(enum_hotkeys,decltype(api.enum_hotkeys),"obs_enum_hotkeys");O(enum_bindings,decltype(api.enum_bindings),"obs_enum_hotkey_bindings");
    O(hk_name,decltype(api.hk_name),"obs_hotkey_get_name");O(hk_desc,decltype(api.hk_desc),"obs_hotkey_get_description");O(hk_type,decltype(api.hk_type),"obs_hotkey_get_registerer_type");O(hk_registerer,decltype(api.hk_registerer),"obs_hotkey_get_registerer");
    O(binding_combo,decltype(api.binding_combo),"obs_hotkey_binding_get_key_combination");O(binding_id,decltype(api.binding_id),"obs_hotkey_binding_get_hotkey_id");O(load_bindings,decltype(api.load_bindings),"obs_hotkey_load_bindings");O(hotkey_load,decltype(api.hotkey_load),"obs_hotkey_load");
    O(register_frontend,decltype(api.register_frontend),"obs_hotkey_register_frontend");O(unregister_hotkey,decltype(api.unregister_hotkey),"obs_hotkey_unregister");O(key_name,decltype(api.key_name),"obs_key_to_name");O(key_from_name,decltype(api.key_from_name),"obs_key_from_name");O(key_from_virtual_key,decltype(api.key_from_virtual_key),"obs_key_from_virtual_key");O(get_locale,decltype(api.get_locale),"obs_get_locale");O(get_version,decltype(api.get_version),"obs_get_version");O(hotkey_enable_background_press,decltype(api.hotkey_enable_background_press),"obs_hotkey_enable_background_press");
    O(main_texture,decltype(api.main_texture),"obs_get_main_texture");O(add_tick,decltype(api.add_tick),"obs_add_tick_callback");O(remove_tick,decltype(api.remove_tick),"obs_remove_tick_callback");O(enter_graphics,decltype(api.enter_graphics),"obs_enter_graphics");O(leave_graphics,decltype(api.leave_graphics),"obs_leave_graphics");
    O(texture_width,decltype(api.texture_width),"gs_texture_get_width");O(texture_height,decltype(api.texture_height),"gs_texture_get_height");O(texture_format,decltype(api.texture_format),"gs_texture_get_color_format");O(stage_create,decltype(api.stage_create),"gs_stagesurface_create");O(stage_destroy,decltype(api.stage_destroy),"gs_stagesurface_destroy");O(stage_texture,decltype(api.stage_texture),"gs_stage_texture");O(stage_map,decltype(api.stage_map),"gs_stagesurface_map");O(stage_unmap,decltype(api.stage_unmap),"gs_stagesurface_unmap");
    O(source_name,decltype(api.source_name),"obs_source_get_name");O(enum_sources,decltype(api.enum_sources),"obs_enum_sources");O(source_get_ref,decltype(api.source_get_ref),"obs_source_get_ref");O(source_output_flags,decltype(api.source_output_flags),"obs_source_get_output_flags");O(source_audio_active,decltype(api.source_audio_active),"obs_source_audio_active");O(source_get_volume,decltype(api.source_get_volume),"obs_source_get_volume");O(source_set_volume,decltype(api.source_set_volume),"obs_source_set_volume");O(source_muted,decltype(api.source_muted),"obs_source_muted");O(source_set_muted,decltype(api.source_set_muted),"obs_source_set_muted");O(weak_source_get,decltype(api.weak_source_get),"obs_weak_source_get_source");O(source_release,decltype(api.source_release),"obs_source_release");O(weak_output_get,decltype(api.weak_output_get),"obs_weak_output_get_output");O(output_release,decltype(api.output_release),"obs_output_release");
    O(hotkey_save,decltype(api.hotkey_save),"obs_hotkey_save");O(data_create,decltype(api.data_create),"obs_data_create");O(data_create_json,decltype(api.data_create_json),"obs_data_create_from_json");O(data_get_array,decltype(api.data_get_array),"obs_data_get_array");O(data_set_array,decltype(api.data_set_array),"obs_data_set_array");O(data_json,decltype(api.data_json),"obs_data_get_json");O(data_release,decltype(api.data_release),"obs_data_release");O(array_release,decltype(api.array_release),"obs_data_array_release");O(save_output,decltype(api.save_output),"obs_hotkeys_save_output");
    O(config_set_string,decltype(api.config_set_string),"config_set_string");O(config_get_string,decltype(api.config_get_string),"config_get_string");O(config_has_user_value,decltype(api.config_has_user_value),"config_has_user_value");O(config_save_safe,decltype(api.config_save_safe),"config_save_safe");
    F(add_tools,decltype(api.add_tools),"obs_frontend_add_tools_menu_item");F(add_tools_qaction,decltype(api.add_tools_qaction),"obs_frontend_add_tools_menu_qaction");F(main_window,decltype(api.main_window),"obs_frontend_get_main_window");F(main_hwnd,decltype(api.main_hwnd),"obs_frontend_get_main_window_handle");F(profile_config,decltype(api.profile_config),"obs_frontend_get_profile_config");F(global_config,decltype(api.global_config),"obs_frontend_get_global_config");F(studio_mode_active,decltype(api.studio_mode_active),"obs_frontend_preview_program_mode_active");F(add_event_callback,decltype(api.add_event_callback),"obs_frontend_add_event_callback");F(remove_event_callback,decltype(api.remove_event_callback),"obs_frontend_remove_event_callback");F(frontend_save,decltype(api.frontend_save),"obs_frontend_save");
#undef O
#undef F
    return api.enum_hotkeys&&api.enum_bindings&&api.hk_name&&api.hk_desc&&api.hk_type&&api.hk_registerer&&api.binding_combo&&api.binding_id&&api.load_bindings&&api.hotkey_load&&api.register_frontend&&api.unregister_hotkey&&api.key_name&&api.key_from_name&&api.key_from_virtual_key&&api.get_locale&&api.get_version&&api.main_texture&&api.add_tick&&api.remove_tick&&api.enter_graphics&&api.leave_graphics&&api.texture_width&&api.texture_height&&api.texture_format&&api.stage_create&&api.stage_destroy&&api.stage_texture&&api.stage_map&&api.stage_unmap&&api.source_name&&api.enum_sources&&api.source_get_ref&&api.source_output_flags&&api.source_audio_active&&api.source_get_volume&&api.source_set_volume&&api.source_muted&&api.source_set_muted&&api.weak_source_get&&api.source_release&&api.weak_output_get&&api.output_release&&api.hotkey_save&&api.data_create&&api.data_create_json&&api.data_get_array&&api.data_set_array&&api.data_json&&api.data_release&&api.array_release&&api.config_set_string&&api.config_get_string&&api.config_has_user_value&&api.config_save_safe&&api.add_tools_qaction&&api.main_window&&api.main_hwnd&&api.profile_config&&api.global_config&&api.add_event_callback&&api.remove_event_callback;
}

extern "C" __declspec(dllexport) void obs_module_set_pointer(void*){}
extern "C" __declspec(dllexport) uint32_t obs_module_ver(){return 0x20010002;}
extern "C" __declspec(dllexport) bool obs_module_get_string(const char*,const char**){return false;}
extern "C" __declspec(dllexport) void obs_module_set_locale(const char*){}
extern "C" __declspec(dllexport) void obs_module_free_locale(){}
extern "C" __declspec(dllexport) bool obs_module_load(){
    HRESULT comResult=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);comInitialized=SUCCEEDED(comResult);
    if(!LoadApi()){if(comInitialized){CoUninitialize();comInitialized=false;}return false;}uiLanguageIndex=DetectLanguage();obsMainWindow=static_cast<QMainWindow*>(api.main_window());if(!obsMainWindow){if(comInitialized){CoUninitialize();comInitialized=false;}return false;}uint32_t obsVersion=api.get_version();uint32_t major=(obsVersion>>24)&0xFF,minor=(obsVersion>>16)&0xFF,patch=obsVersion&0xFFFF;
    if(major!=32){std::wstring version=std::to_wstring(major)+L"."+std::to_wstring(minor)+L"."+std::to_wstring(patch);if(!ConfirmUntestedObs(version)){if(comInitialized){CoUninitialize();comInitialized=false;}return false;}}
    std::string next=Narrow(Tr(UiText::NextArea)),previous=Narrow(Tr(UiText::PreviousArea));
    nextAreaHotkey=api.register_frontend(NEXT_AREA_NAME,next.c_str(),NavigationHotkey,nullptr);
    previousAreaHotkey=api.register_frontend(PREVIOUS_AREA_NAME,previous.c_str(),NavigationHotkey,reinterpret_cast<void*>(1));
    for(size_t i=0;i<canvasHotkeys.size();++i){QByteArray label=(QStringLiteral("Accessible OBS Studio: ")+CText(CanvasModeText(static_cast<CanvasMode>(i)))).toUtf8();canvasHotkeys[i]=api.register_frontend(CANVAS_HOTKEY_NAMES[i],label.constData(),CanvasHotkey,reinterpret_cast<void*>(static_cast<intptr_t>(i)));}
    QByteArray focusMediaLabel=LText(LocalText::FocusMediaCommand).toUtf8();focusMediaHotkey=api.register_frontend(FOCUS_MEDIA_NAME,focusMediaLabel.constData(),FocusMediaControlsHotkey,nullptr);
    QByteArray openAccessibleLabel=LText(LocalText::OpenAccessibleCommand).toUtf8();openAccessibleObsHotkey=api.register_frontend(OPEN_ACCESSIBLE_OBS_NAME,openAccessibleLabel.constData(),OpenAccessibleObsHotkey,nullptr);
    std::string volumeConsoleLabel=VolumeConsoleCommandLabel();volumeConsoleHotkey=api.register_frontend(VOLUME_CONSOLE_NAME,volumeConsoleLabel.c_str(),VolumeConsoleHotkey,nullptr);
    static constexpr std::array<UiText,6> directAreaText={UiText::FocusVideoPreview,UiText::FocusScenes,UiText::FocusSources,UiText::FocusAudioMixer,UiText::FocusSceneTransitions,UiText::FocusControls};
    for(size_t i=0;i<directAreaHotkeys.size();++i){std::string label=Narrow(Tr(directAreaText[i]));directAreaHotkeys[i]=api.register_frontend(DIRECT_AREA_NAMES[i],label.c_str(),DirectAreaHotkey,reinterpret_cast<void*>(static_cast<intptr_t>(i+1)));}
    LoadAccessibilityBindings();EnsureSafeHotkeyFocusDefault();hotkeyFocusConnection=QObject::connect(qApp,&QGuiApplication::applicationStateChanged,obsMainWindow,[](Qt::ApplicationState){EnsureSafeHotkeyFocusDefault();},Qt::DirectConnection);hotkeyFocusGuardTimer=new QTimer(obsMainWindow);hotkeyFocusGuardTimer->setInterval(250);QObject::connect(hotkeyFocusGuardTimer,&QTimer::timeout,obsMainWindow,[]{EnsureSafeHotkeyFocusDefault();});hotkeyFocusGuardTimer->start();api.add_event_callback(FrontendEvent,nullptr);if(!InitializeAccessibleToolsAction())return false;return true;
}
extern "C" __declspec(dllexport) void obs_module_unload(){
    shuttingDown=true;CanvasCapture *capture=nullptr;{std::lock_guard<std::mutex> lock(captureMutex);capture=activeCapture;}if(capture){api.remove_tick(CanvasTick,capture);std::lock_guard<std::mutex> lock(captureMutex);if(activeCapture==capture){if(capture->surface){api.enter_graphics();api.stage_destroy(capture->surface);api.leave_graphics();}if(!capture->apiKey.empty())SecureZeroMemory(capture->apiKey.data(),capture->apiKey.size());delete capture;activeCapture=nullptr;}}
    if(openAIThread.joinable())openAIThread.join();if(qtHotkeyEditor){delete qtHotkeyEditor.data();qtHotkeyEditor=nullptr;}if(accessibleToolsAction){delete accessibleToolsAction.data();accessibleToolsAction=nullptr;}if(settingsWindow)DestroyWindow(settingsWindow);if(descriptionWindow)DestroyWindow(descriptionWindow);if(api.remove_event_callback)api.remove_event_callback(FrontendEvent,nullptr);
    if(api.unregister_hotkey&&nextAreaHotkey!=static_cast<hotkey_id>(-1))api.unregister_hotkey(nextAreaHotkey);
    if(api.unregister_hotkey&&previousAreaHotkey!=static_cast<hotkey_id>(-1))api.unregister_hotkey(previousAreaHotkey);
    if(api.unregister_hotkey)for(hotkey_id id:canvasHotkeys)if(id!=static_cast<hotkey_id>(-1))api.unregister_hotkey(id);
    if(api.unregister_hotkey&&focusMediaHotkey!=static_cast<hotkey_id>(-1))api.unregister_hotkey(focusMediaHotkey);
    if(api.unregister_hotkey&&openAccessibleObsHotkey!=static_cast<hotkey_id>(-1))api.unregister_hotkey(openAccessibleObsHotkey);
    if(api.unregister_hotkey&&volumeConsoleHotkey!=static_cast<hotkey_id>(-1))api.unregister_hotkey(volumeConsoleHotkey);
    if(api.unregister_hotkey)for(hotkey_id id:directAreaHotkeys)if(id!=static_cast<hotkey_id>(-1))api.unregister_hotkey(id);QObject::disconnect(hotkeyFocusConnection);if(hotkeyFocusGuardTimer){hotkeyFocusGuardTimer->stop();delete hotkeyFocusGuardTimer;hotkeyFocusGuardTimer=nullptr;}
    if(comInitialized){CoUninitialize();comInitialized=false;}
}
BOOL WINAPI DllMain(HINSTANCE h,DWORD reason,LPVOID){if(reason==DLL_PROCESS_ATTACH){instance=h;DisableThreadLibraryCalls(h);}return TRUE;}
