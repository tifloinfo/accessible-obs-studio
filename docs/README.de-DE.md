# Accessible OBS Studio 1.0

[![Tiflo.Info-Logo: blaue Wellen über dem Schriftzug Tiflo.info und dem russischen Motto „Schließe die Augen und sieh“](../assets/tiflo-info-logo.jpg)](https://tiflo.info)

Accessible OBS Studio ist ein Windows-Zugänglichkeits-Plugin für OBS Studio. Es richtet sich an blinde Personen, die OBS mit Tastatur und Screenreader, insbesondere JAWS oder NVDA, bedienen.

## Voraussetzungen und Installation

Benötigt werden 64-Bit-Windows 10 oder 11 und eine unterstützte 64-Bit-Version von OBS Studio 32. Nur die OpenAI-Funktionen benötigen einen eigenen API-Schlüssel und Internetzugang.

Schließen Sie OBS und starten Sie `AccessibleOBSStudio-1.0-Setup.exe`. Das Installationsprogramm installiert das Plugin unter `C:\ProgramData\obs-studio\plugins\accessible-obs-studio` und prüft Microsoft WebView2 sowie Visual C++. Fehlende Komponenten werden nur bei Bedarf direkt von Microsoft heruntergeladen; hierfür ist Internetzugang erforderlich. OBS- und Qt-Dateien werden nicht ersetzt. Zur Deinstallation verwenden Sie „Installierte Apps“ in Windows. Einstellungen bleiben standardmäßig erhalten.

## Standardtasten

- F3: Canvas erfassen und durch OpenAI analysieren.
- F4: sichtbare native Mediensteuerung fokussieren.
- F5: Streaming starten oder beenden.
- F6 / Umschalt+F6: nächster / vorheriger Hauptbereich.
- F7: Aufnahme starten oder beenden.
- F8: virtuelle Kamera ein- oder ausschalten.

Der Befehl **Accessible OBS Studio: Open Accessible OBS Studio** öffnet den Tastenkürzel-Editor. Er kann zugewiesen werden, hat aber standardmäßig kein Tastenkürzel.
- Strg+0 bis Strg+5: Videovorschau, Szenen, Quellen, Audiomixer, Szenenübergänge oder Steuerung fokussieren.

Beim ersten Start deaktiviert das Plugin OBS-Hotkeys außerhalb des fokussierten OBS-Hauptfensters, jedoch nur, wenn Sie diese OBS-Einstellung noch nie selbst festgelegt haben. Spätere Änderungen werden respektiert.

## Tastenkürzel-Editor

Öffnen Sie **Werkzeuge > Accessible OBS Studio**. Suchen Sie einen Befehl und drücken Sie Eingabe oder aktivieren Sie **Hinzufügen oder Bearbeiten**. Die OpenAI-Konfiguration erreichen Sie über die Schaltfläche **API Settings** in diesem Fenster. Im Dialog „Hot Key“ können mehrere Kürzel verwaltet werden. **Weiteres Tastenkürzel hinzufügen** erstellt eine zusätzliche Zuweisung. Eingabe bestätigt, Escape verwirft den Dialog. Tab, Umschalt+Tab, Eingabe, Escape, Alt+F4, Windows-Tastenkombinationen und reservierte Systembefehle werden nicht erfasst.

Änderungen werden erst mit OK im Hauptfenster in OBS gespeichert. Entf oder **Entfernen** löscht alle Kürzel des markierten Befehls. Beim Schließen mit ungespeicherten Änderungen stehen Speichern, Verwerfen und Abbrechen zur Verfügung.

## Audiomixer und Mediensteuerung

Strg+3 fokussiert den Audiomixer. Die sichtbaren Lautstärkeregler werden in Anzeigereihenfolge nummeriert. 1 bis 9 wählen die ersten neun Quellen, 0 die zehnte. Die Pfeiltasten und weitere Bedienung bleiben native OBS-Funktionen. F4 fokussiert die Mediensteuerung, sofern OBS sie anzeigt; das Plugin ersetzt keine OBS-Medientasten.

## Canvas, OpenAI und Aktionen

Ein API-Schlüssel ist optional. Speichern Sie ihn erst bei Bedarf über **API Settings** im Shortcut Editor. Der Dialog unterstützt Tab und Umschalt+Tab; Eingabe im Schlüsselfeld aktiviert **Schlüssel speichern**. Format und OpenAI-Authentifizierung werden vor der verschlüsselten Speicherung in der Windows-Anmeldeinformationsverwaltung geprüft. Ohne Schlüssel bleiben nur OpenAI-Funktionen gesperrt; alle anderen Plugin-Funktionen arbeiten weiter.

F3 funktioniert ohne Vorschaufokus. Ein kurzer Klickton bestätigt, dass die Anfrage gestartet wurde. Während der Erfassung bleibt der aktuelle Fokus erhalten. Das Ergebnisfenster erhält danach den Fokus; beim Schließen wird der vorherige OBS-Bedienpunkt wiederhergestellt, sofern er noch existiert. Ein anderes aktives Programm wird nicht übergangen.

Anschlussfragen müssen sich unmittelbar auf das erfasste Bild beziehen. Bildtext und Fragen gelten als nicht vertrauenswürdige Daten. Themenfremde Fragen werden abgelehnt. Die Sitzung endet beim Schließen oder bei einer neuen Aufnahme.

**Vorgeschlagene Aktionen** bietet eine begrenzte Liste umkehrbarer OBS-Transformationen. Falls keine Quelle ausgewählt ist, erscheint eine zugängliche Quellenauswahl. Jede Aktion besitzt ein eigenes Kontrollkästchen und eine Risikostufe. Erst **Auswahl anwenden** führt markierte Aktionen über die nativen OBS-Aktionen und deren Rückgängig-Verlauf aus. Streaming, Aufnahme, Audio, Zugangsdaten, beliebige Befehle und Ausgabeeinstellungen sind ausgeschlossen.

## Kompatibilitätsanalyse

Bei einer neueren, nicht getesteten OBS-Hauptversion bietet die Warnung **Abbrechen**, **Trotzdem ausführen** und **Kompatibilität analysieren**. Die Analyse kombiniert lokale Nur-Lese-Prüfungen mit offiziellen OBS-Quellen und meldet Niedriges, Mittleres, Hohes oder Unbekanntes Risiko. Sie ist keine Garantie.

Ein erfolgreicher Bericht wird für die genaue OBS-Version, Plugin-Version und Architektur gespeichert und ohne weiteren OpenAI-Aufruf erneut angezeigt. Er wird in einem zugänglichen WebView2-Fenster geöffnet und in die Zwischenablage kopiert.

## Datenschutz, Kosten und Lizenz

Die Canvas-Analyse sendet Canvas, OBS-Sprache, festen Sicherheitsprompt und gültige Anschlussfragen an OpenAI. Die Kompatibilitätsanalyse sendet Versions- und Abhängigkeitsdaten sowie ein festes Funktionsverzeichnis, aber keine Szenen, Medien oder Zugangsdaten. API-Kosten trägt der Schlüsselinhaber. Das Plugin enthält keine Telemetrie oder Werbung.

Copyright (C) 2026 [Tiflo.Info](https://tiflo.info). GNU GPL Version 2 oder neuer; siehe [LICENSE.txt](../LICENSE.txt). [English](../README.md).
