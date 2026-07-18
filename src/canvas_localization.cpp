// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2026 Tiflo.Info

enum class CanvasText {
    BasicDescription,DetailedDescription,ReadText,PeopleBackgrounds,AnalyzeIssues,
    SuggestedFixes,FixAutomatically,AnalyzeAgain,Recommendation,HighSeverity,MediumSeverity,LowSeverity,
    NoTextFound,NoIssuesFound,NoPeopleFound,
    ConflictTitle,ConflictMessage,ReplaceConflicts,KeepExisting,DoNotAskBuild,
    ConflictSaveFailed,Count
};

static QString CText(CanvasText id){
    using A=std::array<const char*,6>;
    static const A text[]={
        A{"Basic Description","Kurzbeschreibung","Краткое описание","Короткий опис","Description de base","Descripción básica"},
        A{"Detailed Description","Detaillierte Beschreibung","Подробное описание","Докладний опис","Description détaillée","Descripción detallada"},
        A{"Read Text","Text lesen","Прочитать текст","Прочитати текст","Lire le texte","Leer texto"},
        A{"People and Backgrounds","Personen und Hintergründe","Люди и фон","Люди й тло","Personnes et arrière-plans","Personas y fondos"},
        A{"Visual Checker","Visuelle Prüfung","Визуальная проверка","Візуальна перевірка","Vérification visuelle","Verificación visual"},
        A{"Suggested Fixes","Vorgeschlagene Korrekturen","Предлагаемые исправления","Запропоновані виправлення","Corrections suggérées","Correcciones sugeridas"},
        A{"Fix Automatically","Automatisch korrigieren","Исправить автоматически","Виправити автоматично","Corriger automatiquement","Corregir automáticamente"},
        A{"Check Again","Erneut prüfen","Проверить снова","Перевірити знову","Vérifier à nouveau","Comprobar de nuevo"},
        A{"Recommendation","Empfehlung","Рекомендация","Рекомендація","Recommandation","Recomendación"},
        A{"High severity","Hoher Schweregrad","Высокая важность","Висока важливість","Gravité élevée","Gravedad alta"},
        A{"Medium severity","Mittlerer Schweregrad","Средняя важность","Середня важливість","Gravité moyenne","Gravedad media"},
        A{"Low severity","Niedriger Schweregrad","Низкая важность","Низька важливість","Gravité faible","Gravedad baja"},
        A{"No text found","Kein Text gefunden","Текст не найден","Текст не знайдено","Aucun texte trouvé","No se encontró texto"},
        A{"No visual issues found","Keine visuellen Probleme gefunden","Визуальные проблемы не найдены","Візуальних проблем не знайдено","Aucun problème visuel détecté","No se encontraron problemas visuales"},
        A{"No people found","Keine Personen gefunden","Люди не найдены","Людей не знайдено","Aucune personne détectée","No se encontraron personas"},
        A{"Keyboard Shortcut Conflicts","Konflikte bei Tastenkombinationen","Конфликты сочетаний клавиш","Конфлікти сполучень клавіш","Conflits de raccourcis clavier","Conflictos de métodos abreviados de teclado"},
        A{"OBS Studio Accessibility found existing keyboard shortcuts that conflict with its defaults. What would you like to do?","OBS Studio Accessibility hat vorhandene Tastenkombinationen gefunden, die mit den Standardbelegungen kollidieren. Was möchten Sie tun?","OBS Studio Accessibility обнаружила существующие сочетания клавиш, конфликтующие с назначениями по умолчанию. Что вы хотите сделать?","OBS Studio Accessibility виявила наявні сполучення клавіш, що конфліктують зі стандартними призначеннями. Що ви хочете зробити?","OBS Studio Accessibility a trouvé des raccourcis clavier existants qui entrent en conflit avec ses valeurs par défaut. Que souhaitez-vous faire ?","OBS Studio Accessibility encontró métodos abreviados de teclado existentes que entran en conflicto con sus valores predeterminados. ¿Qué desea hacer?"},
        A{"Remove only the conflicting assignments and use OBS Studio Accessibility defaults","Nur die kollidierenden Zuweisungen entfernen und die Standardbelegungen von OBS Studio Accessibility verwenden","Удалить только конфликтующие назначения и использовать сочетания OBS Studio Accessibility по умолчанию","Видалити лише конфліктні призначення та використовувати стандартні сполучення OBS Studio Accessibility","Supprimer uniquement les attributions en conflit et utiliser les raccourcis par défaut d’OBS Studio Accessibility","Eliminar solo las asignaciones en conflicto y usar los valores predeterminados de OBS Studio Accessibility"},
        A{"Keep the existing assignments; conflicting Accessibility defaults will remain unassigned","Vorhandene Zuweisungen beibehalten; kollidierende Accessibility-Standardbelegungen bleiben nicht zugewiesen","Сохранить существующие назначения; конфликтующие сочетания Accessibility останутся неназначенными","Зберегти наявні призначення; конфліктні сполучення Accessibility залишаться непризначеними","Conserver les attributions existantes ; les raccourcis Accessibility en conflit resteront non attribués","Conservar las asignaciones existentes; los accesos de Accessibility en conflicto quedarán sin asignar"},
        A{"Do not ask me again for this version of OBS Studio Accessibility","Für diese Version von OBS Studio Accessibility nicht erneut fragen","Больше не спрашивать для этой версии OBS Studio Accessibility","Більше не запитувати для цієї версії OBS Studio Accessibility","Ne plus me demander pour cette version d’OBS Studio Accessibility","No volver a preguntar para esta versión de OBS Studio Accessibility"},
        A{"OBS could not save the keyboard shortcut changes. The previous assignments were restored.","OBS konnte die Änderungen an den Tastenkombinationen nicht speichern. Die vorherigen Zuweisungen wurden wiederhergestellt.","OBS не удалось сохранить изменения сочетаний клавиш. Предыдущие назначения восстановлены.","OBS не вдалося зберегти зміни сполучень клавіш. Попередні призначення відновлено.","OBS n’a pas pu enregistrer les modifications des raccourcis clavier. Les attributions précédentes ont été restaurées.","OBS no pudo guardar los cambios de métodos abreviados de teclado. Se restauraron las asignaciones anteriores."}
    };
    static_assert(std::size(text)==static_cast<size_t>(CanvasText::Count));
    return QString::fromUtf8(text[static_cast<size_t>(id)][LanguageIndex()]);
}

static std::wstring CWText(CanvasText id){return CText(id).toStdWString();}
