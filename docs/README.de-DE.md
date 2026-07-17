# Accessible OBS Studio 1.0

Accessible OBS Studio ist ein Windows-Zugänglichkeits-Plugin für die 64-Bit-Version von OBS Studio 32. Es richtet sich an blinde Tastatur- und Screenreader-Nutzer und wurde für JAWS und NVDA entwickelt. Windows 10 oder 11 wird benötigt. Ein OpenAI-API-Schlüssel und Internetzugang sind nur für OpenAI-Funktionen erforderlich.

## Installation

Installieren Sie die 64-Bit-Version von OBS Studio 32.0 oder neuer und starten Sie `AccessibleOBSStudio-1.0-Setup.exe`. Fehlt OBS Studio, ist die Installation beschädigt oder älter als 32.0, bietet Setup die [offizielle OBS-Download-Seite](https://obsproject.com/download) an und beendet sich ohne Änderungen. Eine ältere Version kann auch in OBS Studio über Hilfe > Nach Updates suchen aktualisiert werden. OBS Studio 32.x wird unterstützt. Bei OBS Studio 33 oder neuer warnt Setup vor möglicher Inkompatibilität und verweist auf die [Seite der neuesten Plugin-Version](https://tiflo.info/aobs), bevor eine ausdrückliche Installation trotzdem möglich ist. Läuft OBS Studio, fordert Setup zum vollständigen Schließen und anschließenden Auswählen von Wiederholen auf; es beendet OBS niemals automatisch. Das Plugin wird unter `C:\ProgramData\obs-studio\plugins\accessible-obs-studio` installiert. Fehlende Microsoft-WebView2- und Visual-C++-Komponenten werden erst nach diesen Prüfungen ergänzt; OBS- und Qt-Dateien werden nicht ersetzt. Auf der Abschlussseite öffnet das Kontrollkästchen **ReadMe im Webbrowser öffnen** die deutsche HTML-Dokumentation. Einstellungen bleiben bei der Deinstallation standardmäßig erhalten.

## Standard-Tastenkombinationen

- F3: Kurzbeschreibung des Canvas mit höchstens 80 Zeichen.
- Umschalt+F3: detaillierte Canvas-Beschreibung.
- Alt+F3: sichtbaren Text ohne Übersetzung oder Kommentar lesen.
- Strg+F3: sichtbare Personen und Hintergründe beschreiben.
- F4: Canvas auf Aufnahme-, Streaming-, Kamera-, Beleuchtungs-, Bildausschnitt-, Schärfe-, Körnungs-, Erscheinungs- und Hintergrundprobleme prüfen.
- Strg+M: sichtbare Mediensteuerung fokussieren.
- F5: Streaming starten oder beenden.
- F6 / Umschalt+F6: nächster / vorheriger Hauptbereich.
- F7: Aufnahme starten oder beenden.
- F8: virtuelle Kamera ein- oder ausschalten.
- Strg+0 bis Strg+5: Videovorschau, Szenen, Quellen, Audiomixer, Szenenübergänge oder Steuerung fokussieren.
- Strg+Gravis: barrierefreie Lautstärkekonsole öffnen. Gemeint ist die physische Taste direkt unter Escape.

Der Befehl **Accessible OBS Studio: Tastenkombinations-Editor öffnen** öffnet denselben Editor direkt, besitzt aber keine Standard-Tastenkombination. Seine interne Kennung bleibt unverändert, sodass eine vorhandene Zuweisung erhalten bleibt.

Standardmäßig erzwingt Accessible OBS Studio, dass alle OBS-Tastenkombinationen nur funktionieren, solange OBS die aktive Anwendung ist. Der Wert unter **Einstellungen > Erweitert > Hotkey-Fokusverhalten** wird auf **Hotkeys deaktivieren, wenn das Hauptfenster nicht fokussiert ist** gehalten und nach Änderungen wiederhergestellt. Aktivieren und speichern Sie im Tastenkombinations-Editor **OBS Studio verwalten lassen, ob Tastenkombinationen außerhalb von OBS funktionieren**, um die Steuerung an OBS zurückzugeben. Danach greift das Plugin nicht mehr ein.

Beim ersten Start und nach einem Profilwechsel werden geplante Standards mit vorhandenen Zuweisungen verglichen. Nur bei echten Konflikten erscheint ein modaler, screenreader-zugänglicher Dialog. Sie können ausschließlich die kollidierenden Kombinationen entfernen oder die vorhandenen Zuweisungen beibehalten; dann bleiben kollidierende Accessibility-Standards unbelegt. **Für diese Version nicht erneut fragen** merkt sich die gewählte Richtlinie profilübergreifend. Eine neue Plugin-Version oder ein neuer Build prüft erneut.

## Tastenkombinations-Editor

Öffnen Sie **Werkzeuge > Accessible OBS Studio**, um den Tastenkombinations-Editor direkt zu öffnen. Die Befehlsliste wird mit den Pfeiltasten bedient; Tab wechselt zum standardmäßig deaktivierten OBS-Hotkey-Steuerungs-Kontrollkästchen und zu den Schaltflächen. Eingabe oder **Hinzufügen oder Bearbeiten** öffnet den Dialog **Tastenkombination**. Dort sind mehrere Zuweisungen möglich. Eingabe oder OK prüft sofort auf Duplikate. Bei einem Konflikt wird der andere Befehl genannt; Nein kehrt zum Eingabedialog zurück, Ja weist die Kombination neu zu. Escape verwirft die Bearbeitung. Mit **OpenAI-API-Einstellungen** in diesem Editor wird OpenAI konfiguriert.

## Audiomixer und Mediensteuerung

Strg+3 fokussiert den nativen Audiomixer. Das Plugin benennt, nummeriert und überwacht dessen Regler nicht mehr; verwenden Sie Tab und Umschalt+Tab mit der nativen OBS-Tastaturbedienung.

Strg+Gravis öffnet die modale barrierefreie Lautstärkekonsole. Links und Rechts wechseln die Quelle, Hoch und Runter ändern die Lautstärke um 1 dB, Pos1 setzt 0 dB und die Leertaste schaltet stumm. 1 bis 9 wählen die ersten neun Quellen, 0 die zehnte. Beim Öffnen übernimmt die Konsole exklusiv die dann verfügbaren Quellen, Lautstärken und Stummschaltungen. Änderungen wirken sofort in OBS; Änderungen über den nativen Mixer, externe Controller, Szenenwechsel oder andere Bedienwege werden während der Sitzung nicht überwacht. Bedienen Sie den Mixer währenddessen nicht an anderer Stelle; schließen und öffnen Sie die Konsole erneut, um externe Änderungen zu laden. Beim Schließen wird der vorherige OBS-Fokus nur dann wiederhergestellt, wenn dies noch angemessen ist.

## Canvas-Beschreiber

Alle fünf Canvas-Tastenkombinationen erfassen das gerenderte OBS-Canvas ohne Vorschaufokus. Ein kurzer Klickton bestätigt den Start. Das WebView2-Ergebnisfenster erhält danach den Fokus. Nur die neueste Antwort besitzt die aggressive ARIA-Alert-Rolle; Fragen werden niemals wiederholt. Alle fünf Modi erlauben bildbezogene Anschlussfragen.

Die Kurzbeschreibung ist zugleich ein Ausgangspunkt. **Detaillierte Beschreibung** ist immer verfügbar, **Text lesen** nur bei erkanntem Text und **Personen und Hintergründe** nur bei erkannten Personen. Diese Voreinstellungen verwenden dasselbe Bild ohne erneuten Upload. **Vorgeschlagene Korrekturen** erscheint ausschließlich bei einer tatsächlich automatisch behebbaren OBS-Quellentransformation.

**Personen und Hintergründe** stellt sichtbare Personen in den Mittelpunkt und beschreibt anschließend deren unmittelbaren Hintergrund. Nicht zugehörige Oberflächen-, Text- und Szenendetails werden ausgelassen, sofern sie die Darstellung einer Person nicht direkt beeinflussen.

**Auf Probleme analysieren** meldet als schwerwiegend: automatisch korrigierbare OBS-Layoutprobleme, leere schwarze oder weiße Aufnahmen, unbrauchbare Kamera- oder Beleuchtungszustände und sichtbar nicht im Vollbild dargestellte Zoom-Meetings. Bei einer leeren Aufnahme wird keine Ursache vermutet. Bei einem sichtbar nicht im Vollbild dargestellten Zoom-Meeting lautet die Anleitung: zu Zoom wechseln, Alt+F drücken, zu OBS zurückkehren und Zoom beziehungsweise nicht alle Fenster während des Streams minimieren. Browser-, Präsentations- und andere Videokonferenzoberflächen gelten als möglicher Aufnahmeinhalt. Bildausschnittprobleme sind mittel, weniger schwere Beleuchtungs-, Unschärfe-, mögliche Linsenverschmutzungs-, Körnungs-, Erscheinungs-, Kleidungs- und Hintergrundprobleme niedrig eingestuft. Automatische Korrekturen werden nur für eine feste Liste umkehrbarer OBS-Quellentransformationen angeboten. **Erneut analysieren** erfasst bewusst ein neues Bild und meldet Verbesserungen, Verschlechterungen, Änderungen und verbleibende Probleme.

Ein API-Schlüssel wird in der Windows-Anmeldeinformationsverwaltung verschlüsselt gespeichert und niemals angezeigt. Format und OpenAI-Authentifizierung werden vor dem Speichern geprüft. **Ich habe noch keinen Schlüssel** erscheint nur ohne gespeicherten Schlüssel; **Schlüssel entfernen** nur mit einem gespeicherten Schlüssel. Das Entfernen benötigt eine Bestätigung und meldet den Erfolg.

## Datenschutz und Lizenz

Canvas-Funktionen senden das erfasste Bild, die OBS-Sprache, feste Sicherheitsanweisungen und gültige Anschlussfragen an OpenAI. Voreinstellungen verwenden die bestehende Antwortkette; **Erneut analysieren** sendet absichtlich ein neues Vergleichsbild. Der API-Schlüssel wird nicht als Inhalt übertragen. Es gibt keine Telemetrie oder Werbung.

Copyright (C) 2026 [Tiflo.Info](https://tiflo.info). GNU GPL Version 2 oder neuer; siehe [LICENSE.txt](../LICENSE.txt). [English](../README.md).
