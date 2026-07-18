#define AppName "Accessible OBS Studio"
#define AppVersion "1.0.1"
#define Publisher "Tiflo.Info"
#define Website "https://tiflo.info"
#define ObsWebsite "https://obsproject.com/download"
#define PluginWebsite "https://tiflo.info/aobs"
#define PluginId "accessible-obs-studio"

[Setup]
AppId={{BDA542EA-4E63-4F03-9F5B-B7A8CD8E470B}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#Publisher}
AppPublisherURL={#Website}
AppSupportURL={#Website}
AppUpdatesURL={#Website}
DefaultDirName={commonappdata}\obs-studio\plugins\{#PluginId}
DisableDirPage=yes
DisableProgramGroupPage=yes
LicenseFile=..\LICENSE.txt
OutputDir=..\..\outputs
OutputBaseFilename=AccessibleOBSStudio-1.0.1-Setup
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
WizardImageFile=..\assets\tiflo-info-logo.png
WizardSmallImageFile=..\assets\tiflo-info-logo.png
SetupIconFile=
CloseApplications=no
RestartApplications=no
UninstallDisplayName={#AppName} {#AppVersion}
VersionInfoVersion=1.0.1.0
VersionInfoCompany={#Publisher}
VersionInfoDescription={#AppName} installer
VersionInfoCopyright=Copyright (C) 2026 {#Publisher}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "ukrainian"; MessagesFile: "compiler:Languages\Ukrainian.isl"

[CustomMessages]
english.InstallProgressTitle=Installing Accessible OBS Studio
english.InstallProgressDescription=Preparing the plugin and checking required Microsoft components.
english.CheckingComponents=Checking required components...
english.DownloadingVCRuntime=Downloading Microsoft Visual C++ Runtime...
english.InstallingVCRuntime=Installing Microsoft Visual C++ Runtime...
english.DownloadingWebView2=Downloading Microsoft Edge WebView2 Runtime...
english.InstallingWebView2=Installing Microsoft Edge WebView2 Runtime...
english.InstallingWebView2Detail=This can take several seconds.
english.StartingPluginInstall=Starting plugin installation...
english.VCRuntimeDownloadError=The Microsoft Visual C++ Runtime is required, but it could not be downloaded. Check the Internet connection and try again.
english.VCRuntimeInstallError=The Microsoft Visual C++ Runtime could not be installed. Error code:
english.WebView2DownloadError=Microsoft Edge WebView2 Runtime is required, but it could not be downloaded. Check the Internet connection and try again.
english.WebView2InstallError=Microsoft Edge WebView2 Runtime could not be installed. Error code:
english.ObsNotInstalled=Setup could not find an installed 64-bit copy of OBS Studio. OBS Studio 32.0 or later is required. Setup will close without changing your system.%n%nWould you like to open the official OBS Studio download page now?%n
english.ObsRegistrationBroken=Setup found an OBS Studio registration, but could not locate or validate the OBS Studio program file. The installation may be incomplete or damaged. Reinstall or repair OBS Studio, then run this setup again. Setup will close without making changes.%n%nWould you like to open the official OBS Studio download page now?%n
english.ObsTooOld=This version of Accessible OBS Studio requires OBS Studio 32.0 or later. Setup found: %1.%n%nUpdate OBS Studio by using Help > Check for Updates in OBS Studio, or download it from the official website. Setup will close without making changes.%n%nWould you like to open the official OBS Studio download page now?%n
english.ObsTooNew=Setup found %1. This release of Accessible OBS Studio was designed and tested for OBS Studio 32.x. It may not work correctly with OBS Studio 33 or later.%n%nFor the latest compatible plugin, visit:%n%2%n%nChoose Yes to install this release anyway. Choose No to open the latest-plugin page and close Setup.
english.ObsRunning=OBS Studio is currently running. Close OBS Studio completely before continuing. Setup will not close it automatically because OBS may be streaming or recording.%n%nAfter closing OBS Studio, choose Retry. Choose Cancel to close Setup without making changes.
english.ObsPreflightError=OBS Studio could not be validated. No files have been installed.
english.ObsStillRunningError=OBS Studio is still running. Close it completely, then try the installation again. No files have been installed.
english.ObsProcessCheckError=Setup could not determine whether OBS Studio is running. Restart Windows and try the installation again. No files have been installed.
english.OpenLocalizedReadme=Open the ReadMe in your web browser

german.InstallProgressTitle=Accessible OBS Studio wird installiert
german.InstallProgressDescription=Das Plugin wird vorbereitet und erforderliche Microsoft-Komponenten werden geprüft.
german.CheckingComponents=Erforderliche Komponenten werden geprüft...
german.DownloadingVCRuntime=Microsoft Visual C++ Runtime wird heruntergeladen...
german.InstallingVCRuntime=Microsoft Visual C++ Runtime wird installiert...
german.DownloadingWebView2=Microsoft Edge WebView2 Runtime wird heruntergeladen...
german.InstallingWebView2=Microsoft Edge WebView2 Runtime wird installiert...
german.InstallingWebView2Detail=Dies kann einige Sekunden dauern.
german.StartingPluginInstall=Plugin-Installation wird gestartet...
german.VCRuntimeDownloadError=Microsoft Visual C++ Runtime ist erforderlich, konnte aber nicht heruntergeladen werden. Prüfen Sie die Internetverbindung und versuchen Sie es erneut.
german.VCRuntimeInstallError=Microsoft Visual C++ Runtime konnte nicht installiert werden. Fehlercode:
german.WebView2DownloadError=Microsoft Edge WebView2 Runtime ist erforderlich, konnte aber nicht heruntergeladen werden. Prüfen Sie die Internetverbindung und versuchen Sie es erneut.
german.WebView2InstallError=Microsoft Edge WebView2 Runtime konnte nicht installiert werden. Fehlercode:
german.ObsNotInstalled=Setup konnte keine installierte 64-Bit-Version von OBS Studio finden. OBS Studio 32.0 oder neuer ist erforderlich. Setup wird beendet, ohne Ihr System zu ändern.%n%nMöchten Sie jetzt die offizielle Download-Seite von OBS Studio öffnen?%n
german.ObsRegistrationBroken=Setup hat eine OBS-Studio-Registrierung gefunden, konnte die Programmdatei jedoch nicht finden oder prüfen. Die Installation ist möglicherweise unvollständig oder beschädigt. Reparieren oder installieren Sie OBS Studio neu und starten Sie dieses Setup erneut. Setup wird ohne Änderungen beendet.%n%nMöchten Sie jetzt die offizielle Download-Seite öffnen?%n
german.ObsTooOld=Diese Version von Accessible OBS Studio benötigt OBS Studio 32.0 oder neuer. Gefunden wurde: %1.%n%nAktualisieren Sie OBS Studio über Hilfe > Nach Updates suchen oder laden Sie es von der offiziellen Website herunter. Setup wird ohne Änderungen beendet.%n%nMöchten Sie jetzt die offizielle Download-Seite öffnen?%n
german.ObsTooNew=Setup hat %1 gefunden. Diese Version von Accessible OBS Studio wurde für OBS Studio 32.x entwickelt und getestet. Mit OBS Studio 33 oder neuer funktioniert sie möglicherweise nicht korrekt.%n%nDas neueste kompatible Plugin finden Sie unter:%n%2%n%nWählen Sie Ja, um diese Version trotzdem zu installieren. Wählen Sie Nein, um die Plugin-Seite zu öffnen und Setup zu beenden.
german.ObsRunning=OBS Studio wird derzeit ausgeführt. Schließen Sie OBS Studio vollständig, bevor Sie fortfahren. Setup schließt es nicht automatisch, da möglicherweise eine Übertragung oder Aufnahme läuft.%n%nWählen Sie nach dem Schließen Wiederholen. Mit Abbrechen wird Setup ohne Änderungen beendet.
german.ObsPreflightError=OBS Studio konnte nicht geprüft werden. Es wurden keine Dateien installiert.
german.ObsStillRunningError=OBS Studio wird noch ausgeführt. Schließen Sie es vollständig und versuchen Sie die Installation erneut. Es wurden keine Dateien installiert.
german.ObsProcessCheckError=Setup konnte nicht feststellen, ob OBS Studio ausgeführt wird. Starten Sie Windows neu und versuchen Sie die Installation erneut. Es wurden keine Dateien installiert.
german.OpenLocalizedReadme=ReadMe im Webbrowser öffnen

spanish.InstallProgressTitle=Instalando Accessible OBS Studio
spanish.InstallProgressDescription=Preparando el complemento y comprobando los componentes de Microsoft necesarios.
spanish.CheckingComponents=Comprobando los componentes necesarios...
spanish.DownloadingVCRuntime=Descargando Microsoft Visual C++ Runtime...
spanish.InstallingVCRuntime=Instalando Microsoft Visual C++ Runtime...
spanish.DownloadingWebView2=Descargando Microsoft Edge WebView2 Runtime...
spanish.InstallingWebView2=Instalando Microsoft Edge WebView2 Runtime...
spanish.InstallingWebView2Detail=Esto puede tardar varios segundos.
spanish.StartingPluginInstall=Iniciando la instalación del complemento...
spanish.VCRuntimeDownloadError=Microsoft Visual C++ Runtime es necesario, pero no se pudo descargar. Compruebe la conexión a Internet e inténtelo de nuevo.
spanish.VCRuntimeInstallError=No se pudo instalar Microsoft Visual C++ Runtime. Código de error:
spanish.WebView2DownloadError=Microsoft Edge WebView2 Runtime es necesario, pero no se pudo descargar. Compruebe la conexión a Internet e inténtelo de nuevo.
spanish.WebView2InstallError=No se pudo instalar Microsoft Edge WebView2 Runtime. Código de error:
spanish.ObsNotInstalled=El instalador no encontró una copia de 64 bits instalada de OBS Studio. Se requiere OBS Studio 32.0 o posterior. El instalador se cerrará sin modificar el sistema.%n%n¿Desea abrir ahora la página oficial de descarga de OBS Studio?%n
spanish.ObsRegistrationBroken=El instalador encontró un registro de OBS Studio, pero no pudo localizar o validar el archivo del programa. La instalación puede estar incompleta o dañada. Repare o reinstale OBS Studio y vuelva a ejecutar este instalador. Se cerrará sin realizar cambios.%n%n¿Desea abrir ahora la página oficial de descarga?%n
spanish.ObsTooOld=Esta versión de Accessible OBS Studio requiere OBS Studio 32.0 o posterior. Se encontró: %1.%n%nActualice OBS Studio mediante Ayuda > Buscar actualizaciones, o descárguelo del sitio web oficial. El instalador se cerrará sin realizar cambios.%n%n¿Desea abrir ahora la página oficial de descarga?%n
spanish.ObsTooNew=El instalador encontró %1. Esta versión de Accessible OBS Studio fue diseñada y probada para OBS Studio 32.x. Es posible que no funcione correctamente con OBS Studio 33 o posterior.%n%nPara obtener el complemento compatible más reciente, visite:%n%2%n%nElija Sí para instalar esta versión de todos modos. Elija No para abrir la página del complemento más reciente y cerrar el instalador.
spanish.ObsRunning=OBS Studio se está ejecutando. Ciérrelo por completo antes de continuar. El instalador no lo cerrará automáticamente porque OBS podría estar transmitiendo o grabando.%n%nDespués de cerrar OBS Studio, elija Reintentar. Elija Cancelar para cerrar el instalador sin realizar cambios.
spanish.ObsPreflightError=No se pudo validar OBS Studio. No se ha instalado ningún archivo.
spanish.ObsStillRunningError=OBS Studio sigue ejecutándose. Ciérrelo por completo e intente de nuevo la instalación. No se ha instalado ningún archivo.
spanish.ObsProcessCheckError=El instalador no pudo determinar si OBS Studio se está ejecutando. Reinicie Windows e intente de nuevo la instalación. No se ha instalado ningún archivo.
spanish.OpenLocalizedReadme=Abrir el archivo Léame en el navegador web

french.InstallProgressTitle=Installation d’Accessible OBS Studio
french.InstallProgressDescription=Préparation du module et vérification des composants Microsoft requis.
french.CheckingComponents=Vérification des composants requis...
french.DownloadingVCRuntime=Téléchargement de Microsoft Visual C++ Runtime...
french.InstallingVCRuntime=Installation de Microsoft Visual C++ Runtime...
french.DownloadingWebView2=Téléchargement de Microsoft Edge WebView2 Runtime...
french.InstallingWebView2=Installation de Microsoft Edge WebView2 Runtime...
french.InstallingWebView2Detail=Cela peut prendre plusieurs secondes.
french.StartingPluginInstall=Démarrage de l’installation du module...
french.VCRuntimeDownloadError=Microsoft Visual C++ Runtime est requis, mais n’a pas pu être téléchargé. Vérifiez la connexion Internet et réessayez.
french.VCRuntimeInstallError=Microsoft Visual C++ Runtime n’a pas pu être installé. Code d’erreur :
french.WebView2DownloadError=Microsoft Edge WebView2 Runtime est requis, mais n’a pas pu être téléchargé. Vérifiez la connexion Internet et réessayez.
french.WebView2InstallError=Microsoft Edge WebView2 Runtime n’a pas pu être installé. Code d’erreur :
french.ObsNotInstalled=Le programme d’installation n’a trouvé aucune copie 64 bits installée d’OBS Studio. OBS Studio 32.0 ou version ultérieure est requis. Le programme va se fermer sans modifier le système.%n%nVoulez-vous ouvrir maintenant la page officielle de téléchargement d’OBS Studio ?%n
french.ObsRegistrationBroken=Le programme d’installation a trouvé une inscription d’OBS Studio, mais n’a pas pu localiser ou valider le fichier du programme. L’installation est peut-être incomplète ou endommagée. Réparez ou réinstallez OBS Studio, puis relancez ce programme. Il va se fermer sans apporter de modifications.%n%nVoulez-vous ouvrir maintenant la page officielle de téléchargement ?%n
french.ObsTooOld=Cette version d’Accessible OBS Studio nécessite OBS Studio 32.0 ou version ultérieure. Version trouvée : %1.%n%nMettez OBS Studio à jour avec Aide > Rechercher des mises à jour, ou téléchargez-le depuis le site officiel. Le programme va se fermer sans apporter de modifications.%n%nVoulez-vous ouvrir maintenant la page officielle de téléchargement ?%n
french.ObsTooNew=Le programme d’installation a trouvé %1. Cette version d’Accessible OBS Studio a été conçue et testée pour OBS Studio 32.x. Elle risque de ne pas fonctionner correctement avec OBS Studio 33 ou version ultérieure.%n%nPour obtenir le dernier module compatible, consultez :%n%2%n%nChoisissez Oui pour installer quand même cette version. Choisissez Non pour ouvrir la page du dernier module et fermer le programme d’installation.
french.ObsRunning=OBS Studio est en cours d’exécution. Fermez-le complètement avant de continuer. Le programme d’installation ne le fermera pas automatiquement, car OBS peut être en train de diffuser ou d’enregistrer.%n%nAprès avoir fermé OBS Studio, choisissez Réessayer. Choisissez Annuler pour fermer le programme sans apporter de modifications.
french.ObsPreflightError=OBS Studio n’a pas pu être validé. Aucun fichier n’a été installé.
french.ObsStillRunningError=OBS Studio est toujours en cours d’exécution. Fermez-le complètement, puis recommencez l’installation. Aucun fichier n’a été installé.
french.ObsProcessCheckError=Le programme d’installation n’a pas pu déterminer si OBS Studio est en cours d’exécution. Redémarrez Windows et recommencez l’installation. Aucun fichier n’a été installé.
french.OpenLocalizedReadme=Ouvrir le fichier Lisez-moi dans le navigateur web

russian.InstallProgressTitle=Установка Accessible OBS Studio
russian.InstallProgressDescription=Подготовка плагина и проверка необходимых компонентов Microsoft.
russian.CheckingComponents=Проверка необходимых компонентов...
russian.DownloadingVCRuntime=Загрузка Microsoft Visual C++ Runtime...
russian.InstallingVCRuntime=Установка Microsoft Visual C++ Runtime...
russian.DownloadingWebView2=Загрузка Microsoft Edge WebView2 Runtime...
russian.InstallingWebView2=Установка Microsoft Edge WebView2 Runtime...
russian.InstallingWebView2Detail=Это может занять несколько секунд.
russian.StartingPluginInstall=Начало установки плагина...
russian.VCRuntimeDownloadError=Требуется Microsoft Visual C++ Runtime, но его не удалось загрузить. Проверьте подключение к Интернету и повторите попытку.
russian.VCRuntimeInstallError=Не удалось установить Microsoft Visual C++ Runtime. Код ошибки:
russian.WebView2DownloadError=Требуется Microsoft Edge WebView2 Runtime, но его не удалось загрузить. Проверьте подключение к Интернету и повторите попытку.
russian.WebView2InstallError=Не удалось установить Microsoft Edge WebView2 Runtime. Код ошибки:
russian.ObsNotInstalled=Программа установки не нашла установленную 64-разрядную версию OBS Studio. Требуется OBS Studio 32.0 или новее. Программа завершит работу, не изменяя систему.%n%nОткрыть официальную страницу загрузки OBS Studio?%n
russian.ObsRegistrationBroken=Программа установки нашла регистрацию OBS Studio, но не смогла найти или проверить файл программы. Установка может быть неполной или поврежденной. Восстановите или переустановите OBS Studio, затем снова запустите эту программу установки. Она завершит работу без изменений.%n%nОткрыть официальную страницу загрузки?%n
russian.ObsTooOld=Для этой версии Accessible OBS Studio требуется OBS Studio 32.0 или новее. Обнаружено: %1.%n%nОбновите OBS Studio с помощью команды «Справка > Проверить обновления» или загрузите его с официального сайта. Программа установки завершит работу без изменений.%n%nОткрыть официальную страницу загрузки?%n
russian.ObsTooNew=Программа установки обнаружила %1. Эта версия Accessible OBS Studio разработана и проверена для OBS Studio 32.x. Она может работать неправильно с OBS Studio 33 или новее.%n%nПоследнюю совместимую версию плагина можно найти по адресу:%n%2%n%nНажмите «Да», чтобы все равно установить эту версию. Нажмите «Нет», чтобы открыть страницу последней версии плагина и закрыть программу установки.
russian.ObsRunning=OBS Studio сейчас запущена. Полностью закройте OBS Studio, прежде чем продолжить. Программа установки не закроет ее автоматически, поскольку может выполняться трансляция или запись.%n%nПосле закрытия OBS Studio нажмите «Повторить». Нажмите «Отмена», чтобы закрыть программу установки без изменений.
russian.ObsPreflightError=Не удалось проверить OBS Studio. Файлы не устанавливались.
russian.ObsStillRunningError=OBS Studio все еще запущена. Полностью закройте ее и повторите установку. Файлы не устанавливались.
russian.ObsProcessCheckError=Программе установки не удалось определить, запущена ли OBS Studio. Перезапустите Windows и повторите установку. Файлы не устанавливались.
russian.OpenLocalizedReadme=Открыть файл ReadMe в веб-браузере

ukrainian.InstallProgressTitle=Встановлення Accessible OBS Studio
ukrainian.InstallProgressDescription=Підготовка плагіна й перевірка необхідних компонентів Microsoft.
ukrainian.CheckingComponents=Перевірка необхідних компонентів...
ukrainian.DownloadingVCRuntime=Завантаження Microsoft Visual C++ Runtime...
ukrainian.InstallingVCRuntime=Встановлення Microsoft Visual C++ Runtime...
ukrainian.DownloadingWebView2=Завантаження Microsoft Edge WebView2 Runtime...
ukrainian.InstallingWebView2=Встановлення Microsoft Edge WebView2 Runtime...
ukrainian.InstallingWebView2Detail=Це може тривати кілька секунд.
ukrainian.StartingPluginInstall=Початок встановлення плагіна...
ukrainian.VCRuntimeDownloadError=Потрібен Microsoft Visual C++ Runtime, але його не вдалося завантажити. Перевірте підключення до Інтернету й повторіть спробу.
ukrainian.VCRuntimeInstallError=Не вдалося встановити Microsoft Visual C++ Runtime. Код помилки:
ukrainian.WebView2DownloadError=Потрібен Microsoft Edge WebView2 Runtime, але його не вдалося завантажити. Перевірте підключення до Інтернету й повторіть спробу.
ukrainian.WebView2InstallError=Не вдалося встановити Microsoft Edge WebView2 Runtime. Код помилки:
ukrainian.ObsNotInstalled=Програма встановлення не знайшла встановлену 64-розрядну версію OBS Studio. Потрібна OBS Studio 32.0 або новіша. Програма завершить роботу, не змінюючи систему.%n%nВідкрити офіційну сторінку завантаження OBS Studio?%n
ukrainian.ObsRegistrationBroken=Програма встановлення знайшла реєстрацію OBS Studio, але не змогла знайти або перевірити файл програми. Встановлення може бути неповним або пошкодженим. Відновіть або перевстановіть OBS Studio, а потім знову запустіть цю програму встановлення. Вона завершить роботу без змін.%n%nВідкрити офіційну сторінку завантаження?%n
ukrainian.ObsTooOld=Для цієї версії Accessible OBS Studio потрібна OBS Studio 32.0 або новіша. Виявлено: %1.%n%nОновіть OBS Studio командою «Довідка > Перевірити оновлення» або завантажте її з офіційного сайту. Програма встановлення завершить роботу без змін.%n%nВідкрити офіційну сторінку завантаження?%n
ukrainian.ObsTooNew=Програма встановлення виявила %1. Цю версію Accessible OBS Studio розроблено й перевірено для OBS Studio 32.x. Вона може працювати неправильно з OBS Studio 33 або новішою.%n%nОстанню сумісну версію плагіна можна знайти за адресою:%n%2%n%nНатисніть «Так», щоб усе одно встановити цю версію. Натисніть «Ні», щоб відкрити сторінку останньої версії плагіна й закрити програму встановлення.
ukrainian.ObsRunning=OBS Studio зараз запущено. Повністю закрийте OBS Studio, перш ніж продовжити. Програма встановлення не закриє її автоматично, оскільки може тривати трансляція або запис.%n%nПісля закриття OBS Studio натисніть «Повторити». Натисніть «Скасувати», щоб закрити програму встановлення без змін.
ukrainian.ObsPreflightError=Не вдалося перевірити OBS Studio. Файли не встановлювалися.
ukrainian.ObsStillRunningError=OBS Studio ще запущено. Повністю закрийте її та повторіть встановлення. Файли не встановлювалися.
ukrainian.ObsProcessCheckError=Програмі встановлення не вдалося визначити, чи запущено OBS Studio. Перезапустіть Windows і повторіть встановлення. Файли не встановлювалися.
ukrainian.OpenLocalizedReadme=Відкрити файл ReadMe у веббраузері

[Files]
Source: "..\package\bin\64bit\accessible-obs-studio.dll"; DestDir: "{app}\bin\64bit"; Flags: ignoreversion
Source: "..\package\data\*"; DestDir: "{app}\data"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\PROJECT_README.md"; DestDir: "{app}"; DestName: "README.md"; Flags: ignoreversion
Source: "..\docs\*"; DestDir: "{app}\docs"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\LICENSE-GPL-2.0.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\NOTICE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\assets\tiflo-info-logo.jpg"; DestDir: "{app}\assets"; Flags: ignoreversion
Source: "..\assets\tiflo-info-logo.png"; DestDir: "{app}\assets"; Flags: ignoreversion

[Run]
Filename: "{app}\docs\README.en-US.html"; Description: "{cm:OpenLocalizedReadme}"; WorkingDir: "{app}\docs"; Flags: postinstall shellexec skipifsilent nowait runasoriginaluser; Languages: english
Filename: "{app}\docs\README.de-DE.html"; Description: "{cm:OpenLocalizedReadme}"; WorkingDir: "{app}\docs"; Flags: postinstall shellexec skipifsilent nowait runasoriginaluser; Languages: german
Filename: "{app}\docs\README.es-ES.html"; Description: "{cm:OpenLocalizedReadme}"; WorkingDir: "{app}\docs"; Flags: postinstall shellexec skipifsilent nowait runasoriginaluser; Languages: spanish
Filename: "{app}\docs\README.fr-FR.html"; Description: "{cm:OpenLocalizedReadme}"; WorkingDir: "{app}\docs"; Flags: postinstall shellexec skipifsilent nowait runasoriginaluser; Languages: french
Filename: "{app}\docs\README.ru-RU.html"; Description: "{cm:OpenLocalizedReadme}"; WorkingDir: "{app}\docs"; Flags: postinstall shellexec skipifsilent nowait runasoriginaluser; Languages: russian
Filename: "{app}\docs\README.uk-UA.html"; Description: "{cm:OpenLocalizedReadme}"; WorkingDir: "{app}\docs"; Flags: postinstall shellexec skipifsilent nowait runasoriginaluser; Languages: ukrainian

[Code]
const
  TH32CS_SNAPPROCESS = $00000002;
  INVALID_HANDLE_VALUE = -1;
  ObsStatusMissing = 0;
  ObsStatusBroken = 1;
  ObsStatusTooOld = 2;
  ObsStatusSupported = 3;
  ObsStatusTooNew = 4;

type
  TProcessEntry32 = record
    dwSize: Cardinal;
    cntUsage: Cardinal;
    th32ProcessID: Cardinal;
    th32DefaultHeapID: Cardinal;
    th32ModuleID: Cardinal;
    cntThreads: Cardinal;
    th32ParentProcessID: Cardinal;
    pcPriClassBase: Integer;
    dwFlags: Cardinal;
    szExeFile: array[0..259] of Char;
  end;

function CreateToolhelp32Snapshot(dwFlags, th32ProcessID: Cardinal): THandle;
  external 'CreateToolhelp32Snapshot@kernel32.dll stdcall';
function Process32First(hSnapshot: THandle;
  var lppe: TProcessEntry32): Boolean;
  external 'Process32FirstW@kernel32.dll stdcall';
function Process32Next(hSnapshot: THandle;
  var lppe: TProcessEntry32): Boolean;
  external 'Process32NextW@kernel32.dll stdcall';
function CloseSnapshotHandle(hObject: THandle): Boolean;
  external 'CloseHandle@kernel32.dll stdcall';

var
  InstallProgressPage: TOutputProgressWizardPage;
  DownloadProgressBase: Integer;
  DownloadProgressSpan: Integer;
  ObsExecutablePaths: TArrayOfString;
  ObsRegistrationFound: Boolean;
  ObsVersionPreflightPassed: Boolean;
  NewerObsWarningAccepted: Boolean;
  ObsProcessCheckSucceeded: Boolean;
  ClosingAfterPreflightFailure: Boolean;

function PathFromCommandLine(Value: String): String;
var
  EndQuote, ExePosition: Integer;
begin
  Result := '';
  Value := Trim(Value);
  if Value = '' then Exit;

  if Value[1] = '"' then
  begin
    Delete(Value, 1, 1);
    EndQuote := Pos('"', Value);
    if EndQuote > 0 then
      Result := Copy(Value, 1, EndQuote - 1)
    else
      Result := Value;
  end
  else
    Result := Value;

  ExePosition := Pos('.exe', Lowercase(Result));
  if ExePosition > 0 then
    Result := Copy(Result, 1, ExePosition + 3);
  EndQuote := Pos(',', Result);
  if EndQuote > 0 then
    Result := Copy(Result, 1, EndQuote - 1);
  Result := Trim(Result);
end;

procedure AddObsExecutable(Path: String);
var
  I: Integer;
begin
  Path := PathFromCommandLine(Path);
  if (Path = '') or not FileExists(Path) then Exit;

  for I := 0 to GetArrayLength(ObsExecutablePaths) - 1 do
    if CompareText(ObsExecutablePaths[I], Path) = 0 then Exit;

  I := GetArrayLength(ObsExecutablePaths);
  SetArrayLength(ObsExecutablePaths, I + 1);
  ObsExecutablePaths[I] := Path;
end;

procedure AddObsFromInstallRoot(InstallRoot: String);
begin
  InstallRoot := PathFromCommandLine(InstallRoot);
  if InstallRoot = '' then Exit;
  AddObsExecutable(AddBackslash(InstallRoot) + 'bin\64bit\obs64.exe');
  AddObsExecutable(AddBackslash(InstallRoot) + 'bin\32bit\obs32.exe');
end;

procedure InspectObsRegistryView(RootKey: Integer);
var
  BaseKey, Key, DisplayName, Value: String;
  Subkeys: TArrayOfString;
  I: Integer;
begin
  BaseKey := 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall';
  if not RegGetSubkeyNames(RootKey, BaseKey, Subkeys) then Exit;

  for I := 0 to GetArrayLength(Subkeys) - 1 do
  begin
    Key := BaseKey + '\' + Subkeys[I];
    if RegQueryStringValue(RootKey, Key, 'DisplayName', DisplayName) and
       (CompareText(Trim(DisplayName), 'OBS Studio') = 0) then
    begin
      ObsRegistrationFound := True;
      if RegQueryStringValue(RootKey, Key, 'DisplayIcon', Value) then
        AddObsExecutable(Value);
      if RegQueryStringValue(RootKey, Key, 'InstallLocation', Value) then
        AddObsFromInstallRoot(Value);
      if RegQueryStringValue(RootKey, Key, 'UninstallString', Value) then
        AddObsFromInstallRoot(ExtractFileDir(PathFromCommandLine(Value)));
    end;
  end;
end;

procedure FindObsInstallations;
begin
  SetArrayLength(ObsExecutablePaths, 0);
  ObsRegistrationFound := False;
  InspectObsRegistryView(HKLM32);
  InspectObsRegistryView(HKLM64);
  InspectObsRegistryView(HKCU);

  { Support the normal official location even if its uninstall entry is absent. }
  AddObsExecutable(ExpandConstant('{pf}\obs-studio\bin\64bit\obs64.exe'));
end;

function MajorVersionOf(const Version: String): Integer;
var
  DotPosition: Integer;
  MajorText: String;
begin
  DotPosition := Pos('.', Version);
  if DotPosition > 0 then
    MajorText := Copy(Version, 1, DotPosition - 1)
  else
    MajorText := Version;
  Result := StrToIntDef(Trim(MajorText), -1);
end;

function InspectObsVersion(var VersionSummary: String): Integer;
var
  I, Major: Integer;
  Version: String;
  HasOld, HasNew: Boolean;
begin
  FindObsInstallations;
  VersionSummary := '';
  if GetArrayLength(ObsExecutablePaths) = 0 then
  begin
    if ObsRegistrationFound then
      Result := ObsStatusBroken
    else
      Result := ObsStatusMissing;
    Exit;
  end;

  HasOld := False;
  HasNew := False;
  for I := 0 to GetArrayLength(ObsExecutablePaths) - 1 do
  begin
    if not GetVersionNumbersString(ObsExecutablePaths[I], Version) then
    begin
      Result := ObsStatusBroken;
      Exit;
    end;
    Major := MajorVersionOf(Version);
    if Major < 0 then
    begin
      Result := ObsStatusBroken;
      Exit;
    end;

    if VersionSummary <> '' then
      VersionSummary := VersionSummary + ', ';
    VersionSummary := VersionSummary + 'OBS Studio ' + Version;
    if Major < 32 then HasOld := True;
    if Major >= 33 then HasNew := True;
  end;

  { The plugin is installed globally, so any detected older copy is unsafe. }
  if HasOld then
    Result := ObsStatusTooOld
  else if HasNew then
    Result := ObsStatusTooNew
  else
    Result := ObsStatusSupported;
end;

procedure OpenWebsite(const Url: String);
var
  ResultCode: Integer;
begin
  ShellExecAsOriginalUser('', Url, '', '', SW_SHOWNORMAL, ewNoWait,
    ResultCode);
end;

function RunObsVersionPreflight: Boolean;
var
  Status, Choice: Integer;
  Summary, MessageText: String;
begin
  Result := False;
  Status := InspectObsVersion(Summary);
  if Status <> ObsStatusTooNew then NewerObsWarningAccepted := False;
  case Status of
    ObsStatusMissing:
      begin
        Choice := MsgBox(CustomMessage('ObsNotInstalled') +
          '{#ObsWebsite}', mbInformation, MB_YESNO);
        if Choice = IDYES then OpenWebsite('{#ObsWebsite}');
      end;
    ObsStatusBroken:
      begin
        Choice := MsgBox(CustomMessage('ObsRegistrationBroken') +
          '{#ObsWebsite}', mbError, MB_YESNO);
        if Choice = IDYES then OpenWebsite('{#ObsWebsite}');
      end;
    ObsStatusTooOld:
      begin
        MessageText := FmtMessage(CustomMessage('ObsTooOld'), [Summary]);
        Choice := MsgBox(MessageText + '{#ObsWebsite}', mbError, MB_YESNO);
        if Choice = IDYES then OpenWebsite('{#ObsWebsite}');
      end;
    ObsStatusTooNew:
      begin
        MessageText := FmtMessage(CustomMessage('ObsTooNew'), [Summary, '{#PluginWebsite}']);
        Choice := MsgBox(MessageText, mbConfirmation, MB_YESNO);
        if Choice = IDYES then
        begin
          NewerObsWarningAccepted := True;
          Result := True;
        end
        else
          OpenWebsite('{#PluginWebsite}');
      end;
    ObsStatusSupported:
      Result := True;
  end;

  ObsVersionPreflightPassed := Result;
end;

function RevalidateObsVersion: Boolean;
var
  Status: Integer;
  Summary: String;
begin
  Status := InspectObsVersion(Summary);
  Result := (Status = ObsStatusSupported) or
    ((Status = ObsStatusTooNew) and NewerObsWarningAccepted);
  if not Result then
  begin
    ObsVersionPreflightPassed := False;
    Result := RunObsVersionPreflight;
  end;
  ObsVersionPreflightPassed := Result;
end;

function ProcessEntryName(const Entry: TProcessEntry32): String;
var
  I: Integer;
begin
  Result := '';
  for I := 0 to 259 do
  begin
    if Entry.szExeFile[I] = #0 then Break;
    Result := Result + Entry.szExeFile[I];
  end;
end;

function ObsIsRunning: Boolean;
var
  Snapshot: THandle;
  Entry: TProcessEntry32;
  ProcessName: String;
begin
  Result := False;
  ObsProcessCheckSucceeded := False;
  Snapshot := CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if Snapshot = INVALID_HANDLE_VALUE then Exit;
  ObsProcessCheckSucceeded := True;
  try
    Entry.dwSize := SizeOf(Entry);
    if Process32First(Snapshot, Entry) then
    begin
      repeat
        ProcessName := Lowercase(ProcessEntryName(Entry));
        if (ProcessName = 'obs64.exe') or (ProcessName = 'obs32.exe') then
        begin
          Result := True;
          Exit;
        end;
        Entry.dwSize := SizeOf(Entry);
      until not Process32Next(Snapshot, Entry);
    end;
  finally
    CloseSnapshotHandle(Snapshot);
  end;
end;

function WaitForObsToClose: Boolean;
var
  Running: Boolean;
begin
  Result := True;
  Running := ObsIsRunning;
  if not ObsProcessCheckSucceeded then
  begin
    MsgBox(CustomMessage('ObsProcessCheckError'), mbError, MB_OK);
    Result := False;
    Exit;
  end;
  while Running do
  begin
    if MsgBox(CustomMessage('ObsRunning'), mbError, MB_RETRYCANCEL) <>
       IDRETRY then
    begin
      Result := False;
      Exit;
    end;
    Running := ObsIsRunning;
    if not ObsProcessCheckSucceeded then
    begin
      MsgBox(CustomMessage('ObsProcessCheckError'), mbError, MB_OK);
      Result := False;
      Exit;
    end;
  end;
end;

procedure InitializeWizard();
begin
  InstallProgressPage := CreateOutputProgressPage(
    CustomMessage('InstallProgressTitle'),
    CustomMessage('InstallProgressDescription'));
end;

procedure CancelButtonClick(CurPageID: Integer; var Cancel, Confirm: Boolean);
begin
  if ClosingAfterPreflightFailure then Confirm := False;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Result := True;
  if not ObsVersionPreflightPassed then
  begin
    Result := RunObsVersionPreflight;
    if not Result then
    begin
      ClosingAfterPreflightFailure := True;
      WizardForm.Close;
      Exit;
    end;
  end;

  if (CurPageID = wpReady) and not RevalidateObsVersion then
  begin
    Result := False;
    ClosingAfterPreflightFailure := True;
    WizardForm.Close;
    Exit;
  end;

  if (CurPageID = wpReady) and not WaitForObsToClose then
  begin
    Result := False;
    ClosingAfterPreflightFailure := True;
    WizardForm.Close;
  end;
end;

function WebView2Installed: Boolean;
var
  Version: String;
  Key: String;
begin
  Key := 'SOFTWARE\Microsoft\EdgeUpdate\Clients\{F1E7E3A2-46B8-4A6C-8A53-EDC92C0D0399}';
  Result := RegQueryStringValue(HKLM32, Key, 'pv', Version) or
            RegQueryStringValue(HKLM64, Key, 'pv', Version) or
            RegQueryStringValue(HKCU, Key, 'pv', Version);
end;

function VCRuntimeInstalled: Boolean;
var
  Installed: Cardinal;
begin
  Result := RegQueryDWordValue(HKLM64,
    'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64',
    'Installed', Installed) and (Installed = 1);
end;

function OnDownloadProgress(const Url, FileName: String;
  const Progress, ProgressMax: Int64): Boolean;
begin
  if ProgressMax > 0 then
    InstallProgressPage.SetProgress(DownloadProgressBase +
      ((Progress * DownloadProgressSpan) div ProgressMax), 100)
  else
    InstallProgressPage.SetProgress(DownloadProgressBase, 100);
  Result := True;
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
var
  ResultCode, ObsStatus: Integer;
  ObsSummary: String;
  Running: Boolean;
begin
  Result := '';
  ObsStatus := InspectObsVersion(ObsSummary);
  if (ObsStatus = ObsStatusSupported) or
     ((ObsStatus = ObsStatusTooNew) and NewerObsWarningAccepted) then
    ObsVersionPreflightPassed := True
  else
    ObsVersionPreflightPassed := False;
  if not ObsVersionPreflightPassed then
  begin
    Result := CustomMessage('ObsPreflightError');
    Exit;
  end;
  Running := ObsIsRunning;
  if not ObsProcessCheckSucceeded then
  begin
    Result := CustomMessage('ObsProcessCheckError');
    Exit;
  end;
  if Running then
  begin
    Result := CustomMessage('ObsStillRunningError');
    Exit;
  end;

  InstallProgressPage.SetText(CustomMessage('CheckingComponents'), '');
  InstallProgressPage.SetProgress(0, 100);
  InstallProgressPage.Show;
  try
    InstallProgressPage.SetProgress(5, 100);
    if not VCRuntimeInstalled then
    begin
      InstallProgressPage.SetText(CustomMessage('DownloadingVCRuntime'), '');
      DownloadProgressBase := 5;
      DownloadProgressSpan := 35;
      try
        DownloadTemporaryFile('https://aka.ms/vc14/vc_redist.x64.exe',
          'vc_redist.x64.exe', '', @OnDownloadProgress);
      except
        Result := CustomMessage('VCRuntimeDownloadError') + ' ' +
          GetExceptionMessage;
        Exit;
      end;
      InstallProgressPage.SetText(CustomMessage('InstallingVCRuntime'), '');
      InstallProgressPage.SetProgress(45, 100);
      if not Exec(ExpandConstant('{tmp}\vc_redist.x64.exe'),
        '/install /quiet /norestart', '', SW_HIDE, ewWaitUntilTerminated,
        ResultCode) or ((ResultCode <> 0) and (ResultCode <> 1638) and
        (ResultCode <> 3010)) then
      begin
        Result := CustomMessage('VCRuntimeInstallError') + ' ' +
          IntToStr(ResultCode);
        Exit;
      end;
      if ResultCode = 3010 then NeedsRestart := True;
    end;

    InstallProgressPage.SetProgress(50, 100);
    if not WebView2Installed then
    begin
      InstallProgressPage.SetText(CustomMessage('DownloadingWebView2'), '');
      DownloadProgressBase := 50;
      DownloadProgressSpan := 25;
      try
        DownloadTemporaryFile('https://go.microsoft.com/fwlink/p/?LinkId=2124703',
          'MicrosoftEdgeWebview2Setup.exe', '', @OnDownloadProgress);
      except
        Result := CustomMessage('WebView2DownloadError') + ' ' +
          GetExceptionMessage;
        Exit;
      end;
      InstallProgressPage.SetText(CustomMessage('InstallingWebView2'),
        CustomMessage('InstallingWebView2Detail'));
      InstallProgressPage.SetProgress(80, 100);
      if not Exec(ExpandConstant('{tmp}\MicrosoftEdgeWebview2Setup.exe'),
        '/silent /install', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) or
        ((ResultCode <> 0) and (ResultCode <> 3010)) then
      begin
        Result := CustomMessage('WebView2InstallError') + ' ' +
          IntToStr(ResultCode);
        Exit;
      end;
      if ResultCode = 3010 then NeedsRestart := True;
    end;
    InstallProgressPage.SetText(CustomMessage('StartingPluginInstall'), '');
    InstallProgressPage.SetProgress(100, 100);
  finally
    InstallProgressPage.Hide;
  end;
end;
