# Accessible OBS Studio 1.1 Audible-Meter-Test

Accessible OBS Studio ist ein Windows-Zugänglichkeits-Plugin für die 64-Bit-Version von OBS Studio 32. Es richtet sich an blinde Tastatur- und Screenreader-Nutzer und wurde für JAWS und NVDA entwickelt. Windows 10 oder 11 wird benötigt. Ein OpenAI-API-Schlüssel und Internetzugang sind nur für OpenAI-Funktionen erforderlich.

## Installation

Installieren Sie die 64-Bit-Version von OBS Studio 32.0 oder neuer und starten Sie `AccessibleOBSStudio-1.0.7-Setup.exe`. Fehlt OBS Studio, ist die Installation beschädigt oder älter als 32.0, bietet Setup die [offizielle OBS-Download-Seite](https://obsproject.com/download) an und beendet sich ohne Änderungen. Eine ältere Version kann auch in OBS Studio über Hilfe > Nach Updates suchen aktualisiert werden. OBS Studio 32.x wird unterstützt. Bei OBS Studio 33 oder neuer warnt Setup vor möglicher Inkompatibilität und verweist auf die [Seite der neuesten Plugin-Version](https://tiflo.info/aobs), bevor eine ausdrückliche Installation trotzdem möglich ist. Läuft OBS Studio, fordert Setup zum vollständigen Schließen und anschließenden Auswählen von Wiederholen auf; es beendet OBS niemals automatisch. Das Plugin wird unter `C:\ProgramData\obs-studio\plugins\accessible-obs-studio` installiert. Fehlende Microsoft-WebView2- und Visual-C++-Komponenten werden erst nach diesen Prüfungen ergänzt; OBS- und Qt-Dateien werden nicht ersetzt. Auf der Abschlussseite öffnet das Kontrollkästchen **ReadMe im Webbrowser öffnen** die deutsche HTML-Dokumentation. Einstellungen bleiben bei der Deinstallation standardmäßig erhalten.

## Standard-Tastenkombinationen

- F3: Kurzbeschreibung des Canvas mit höchstens 80 Zeichen.
- Umschalt+F3: detaillierte Canvas-Beschreibung.
- Alt+F3: sichtbaren Text ohne Übersetzung oder Kommentar lesen.
- Strg+F3: sichtbare Personen und Hintergründe beschreiben.
- F4: Visuelle Prüfung des Streams oder der Aufnahme auf Layout-, Kamera-, Beleuchtungs-, Bildausschnitt-, Schärfe-, Körnungs-, Erscheinungs-, Kleidungs-, Hintergrund- und Objektprobleme.
- Strg+M: sichtbare Mediensteuerung fokussieren.
- F5: Streaming starten oder beenden.
- Alt+F2: Statusinformationen für Streaming, Aufnahme, virtuelle Kamera und Studiomodus anzeigen.
- F6 / Umschalt+F6: nächster / vorheriger Hauptbereich.
- F7: Aufnahme starten oder beenden.
- Alt+F7: Aufnahme pausieren oder fortsetzen.
- F8: virtuelle Kamera ein- oder ausschalten.
- Strg+0 bis Strg+5: Videovorschau, Szenen, Quellen, Audiomixer, Szenenübergänge oder Steuerung fokussieren.
- Strg+Gravis: barrierefreie Lautstärkekonsole öffnen. Gemeint ist die physische Taste direkt unter Escape.

Wenn NVDA als einziger laufender Screenreader erkannt wird, kündigt das Plugin nach einem erfolgreichen Bereichswechsel mit F6, Umschalt+F6 oder Strg+0 bis Strg+5 den lokalisierten Namen des Bereichs ausdrücklich an. Bei JAWS, Narrator, unbekannten Screenreadern oder mehreren gleichzeitig erkannten Screenreadern wird diese zusätzliche Ansage unterdrückt.

Der Befehl **.Tastenkombinations-Editor öffnen** öffnet denselben Editor direkt, besitzt aber keine Standard-Tastenkombination. Seine interne Kennung bleibt unverändert, sodass eine vorhandene Zuweisung erhalten bleibt.

Standardmäßig erzwingt Accessible OBS Studio, dass alle OBS-Tastenkombinationen nur funktionieren, solange OBS die aktive Anwendung ist. Der Wert unter **Einstellungen > Erweitert > Hotkey-Fokusverhalten** wird auf **Hotkeys deaktivieren, wenn das Hauptfenster nicht fokussiert ist** gehalten und nach Änderungen wiederhergestellt. Aktivieren und speichern Sie im Tastenkombinations-Editor **OBS Studio verwalten lassen, ob Tastenkombinationen außerhalb von OBS funktionieren**, um die Steuerung an OBS zurückzugeben. Danach greift das Plugin nicht mehr ein.

Beim ersten Start und nach einem Profilwechsel werden geplante Standards mit vorhandenen Zuweisungen verglichen. Nur bei echten Konflikten erscheint ein modaler, screenreader-zugänglicher Dialog. Sie können ausschließlich die kollidierenden Kombinationen entfernen oder die vorhandenen Zuweisungen beibehalten; dann bleiben kollidierende Accessibility-Standards unbelegt. **Für diese Version nicht erneut fragen** merkt sich die gewählte Richtlinie profilübergreifend. Eine neue Plugin-Version oder ein neuer Build prüft erneut.

## Tastenkombinations-Editor

Öffnen Sie **Werkzeuge > Accessible OBS Studio**, um den Tastenkombinations-Editor direkt zu öffnen. Die Befehlsliste wird mit den Pfeiltasten bedient; Tab wechselt zum standardmäßig deaktivierten OBS-Hotkey-Steuerungs-Kontrollkästchen und zu den Schaltflächen. Eingabe oder **Hinzufügen oder Bearbeiten** öffnet den Dialog **Tastenkombination**. Dort sind mehrere Zuweisungen möglich. Eingabe oder OK prüft sofort auf Duplikate. Bei einem Konflikt wird der andere Befehl genannt; Nein kehrt zum Eingabedialog zurück, Ja weist die Kombination neu zu. Escape verwirft die Bearbeitung. Mit **OpenAI-API-Einstellungen** in diesem Editor wird OpenAI konfiguriert.

## Audiomixer und Mediensteuerung

Strg+3 fokussiert den nativen Audiomixer. Das Plugin benennt, nummeriert und überwacht dessen Regler nicht mehr; verwenden Sie Tab und Umschalt+Tab mit der nativen OBS-Tastaturbedienung.

Strg+Gravis öffnet die modale barrierefreie Lautstärkekonsole. Links und Rechts wechseln die Quelle, Hoch und Runter ändern die Lautstärke um 1 dB und Pos1 setzt 0 dB. Die Leertaste schaltet Monitoring und Programmausgabe sicher gemeinsam um, Strg+Leertaste nur das Monitoring und Umschalt+Leertaste nur die Ausgabe. Jede Quelle besitzt außerdem getrennte Schaltflächen für Ausgabe und Monitoring. 1 bis 9 wählen die ersten neun Quellen, 0 die zehnte. Beim Öffnen übernimmt die Konsole exklusiv die dann verfügbaren Quellen, Lautstärken, Ausgabe- und Monitoringzustände. Änderungen wirken sofort in OBS; Änderungen über den nativen Mixer, externe Controller, Szenenwechsel oder andere Bedienwege werden während der Sitzung nicht überwacht. OBS 32.2 oder neuer behandelt Stummschaltung und Monitoring unabhängig; unter OBS 32.0 und 32.1 setzt die Konsole dieselben Befehle in die älteren Zustände „Nur Monitor“, „Monitor und Ausgabe“ und „stumm“ um. Bedienen Sie den Mixer währenddessen nicht an anderer Stelle; schließen und öffnen Sie die Konsole erneut, um externe Änderungen zu laden. Beim Schließen wird der vorherige OBS-Fokus nur dann wiederhergestellt, wenn dies noch angemessen ist.

Wenn sich der Fokus in der Mediensteuerung befindet, springen Links und Rechts 5 Sekunden zurück oder vor. Umschalt+Links und Umschalt+Rechts springen 1 Minute zurück oder vor, Bild auf 5 Minuten zurück und Bild ab 5 Minuten vor. Außerhalb der Mediensteuerung behalten diese Tasten ihre normale Funktion.

Die Lautstärkekonsole zeigt anfangs nur Quellen der aktuellen Programmausgabe; Vorschauquellen im Studiomodus sind ausgeschlossen. Aktivieren Sie die nicht als Standardschaltfläche festgelegte Schaltfläche **Alle Quellen anzeigen** mit Eingabe, um alle Mixerquellen anzuzeigen, aktive Quellen zuerst. Danach heißt dieselbe Schaltfläche **Nur aktive Quellen anzeigen**. Die Leertaste aktiviert diese Ansichtsschaltfläche nicht.

Statusänderungen von Streaming, Aufnahme, Aufnahme-Pause, virtueller Kamera und Studiomodus werden für Screenreader angekündigt. Alt+F2 zeigt **Statusinformationen** einschließlich der Zustände „erneute Verbindung“ und „Aufnahme pausiert“. Alt+F7 pausiert oder setzt eine Aufnahme fort.

## Canvas-Beschreiber

Alle fünf Canvas-Tastenkombinationen erfassen das gerenderte OBS-Canvas ohne Vorschaufokus. Ein kurzer Klickton bestätigt den Start. Das WebView2-Ergebnisfenster erhält danach den Fokus. Jede neu empfangene erste Antwort oder Antwort auf eine Anschlussfrage wird einmal über einen assertiven ARIA-Live-Bereich angekündigt; Fragen werden niemals wiederholt. Alle fünf Modi erlauben bildbezogene Anschlussfragen.

Die Kurzbeschreibung ist zugleich ein Ausgangspunkt. **Detaillierte Beschreibung** ist immer verfügbar, **Text lesen** nur bei erkanntem Text und **Personen und Hintergründe** nur bei erkannten Personen. Diese Voreinstellungen verwenden dasselbe Bild ohne erneuten Upload. **Vorgeschlagene Korrekturen** erscheint ausschließlich bei einer tatsächlich automatisch behebbaren OBS-Quellentransformation.

**Personen und Hintergründe** stellt sichtbare Personen in den Mittelpunkt und beschreibt anschließend deren unmittelbaren Hintergrund. Nicht zugehörige Oberflächen-, Text- und Szenendetails werden ausgelassen, sofern sie die Darstellung einer Person nicht direkt beeinflussen.

**Visuelle Prüfung** prüft ausschließlich, wie der Stream oder die Aufnahme aussieht. Die bisherigen visuellen Prüfungen auf OBS-Layout, leere Aufnahmen, Kamera, Beleuchtung, Zoom-Vollbild, Bildausschnitt, Unschärfe, mögliche Linsenverschmutzung, Körnung, Erscheinung, Kleidung, Hintergrund und unerwünschte Objekte bleiben erhalten. Sprachliche Inhalte wie Sprache, Rechtschreibung, Grammatik, Übersetzung, Formulierung, Fakten, Zahlen, Thema, Ton, Angemessenheit, Untertitel und Beschriftungen werden ignoriert. Text wird nur als visuelles Objekt gemeldet, wenn er etwa zu klein, abgeschnitten, unscharf, kontrastarm oder verdeckt ist oder wichtige Bildinhalte verdeckt. Ein Dialog- oder Fehlerfenster wird nur gemeldet, wenn es Bildinhalt verdeckt oder ein sichtbares Aufnahme- oder Layoutproblem zeigt, niemals wegen seiner Meldung. Automatische Korrekturen werden nur für die feste Liste umkehrbarer OBS-Quellentransformationen angeboten. **Erneut prüfen** erfasst ein neues Bild und meldet visuelle Verbesserungen, Verschlechterungen, Änderungen und verbleibende visuelle Probleme.

Bei der Auswahl einer automatischen Korrektur enthält die Quellenliste nur videofähige Quellen; ist nur eine vorhanden, wird sie automatisch ausgewählt. Bei nicht korrekt positionierten Inhalten hat **An Canvas anpassen** Vorrang. Nach der Zustimmung wird ein neues Bild ausschließlich auf unbrauchbare Unschärfe, Körnung, Rauschen oder Pixelbildung geprüft. Wenn die Qualität nicht akzeptabel ist oder nicht bestätigt werden kann, wird die Anpassung automatisch rückgängig gemacht und stattdessen vollständig zentriert. **Auf Bildschirm strecken** wird niemals angeboten.

Ein API-Schlüssel wird in der Windows-Anmeldeinformationsverwaltung verschlüsselt gespeichert und niemals angezeigt. Format und OpenAI-Authentifizierung werden vor dem Speichern geprüft. **Ich habe noch keinen Schlüssel** erscheint nur ohne gespeicherten Schlüssel; **Schlüssel entfernen** nur mit einem gespeicherten Schlüssel. Das Entfernen benötigt eine Bestätigung und meldet den Erfolg.

## Audible Meter

Strg+I startet oder beendet Audible Meter; automatische Warnungen sind zunächst eingeschaltet. I schaltet die automatischen Eingangs- und Ausgangswarnungen gemeinsam aus oder ein. H meldet den aktuellen Pegel der zuletzt in der Konsole fokussierten Quelle, J die aktuell lauteste Quelle und ihren Pegel, K den typischen aktiven Pegel der ausgewählten Quelle und L die Quelle mit dem höchsten typischen aktiven Pegel. I, H, J, K und L werden in Eingabefeldern nie abgefangen.

Alt+1 bis Alt+9 wechseln zu den ersten neun Szenen in der angezeigten Reihenfolge; Alt+0 wechselt zur zehnten Szene. Szenen nach den ersten zehn haben keine nummerierte Standardtastenkombination.

Die automatischen Töne sind Warnungen, keine Messungen. Ausgangswarnungen bewerten den Pegel nach dem OBS-Regler. Eine Warnung vor dem Regler erkennt bereits vorhandene Qualitätseinbußen und öffnet einmal pro Sitzung einen Dialog mit dem Quellennamen. Ja ist Standard und setzt die Prüfung fort; Nein oder Escape beendet diese Prüfung für die unveränderte Quelle und speichert die Auswahl. Tief bedeutet: Signal vor dem Regler anhaltend rot. Mittel bedeutet: fokussierte Quelle im gelben Ausgangsbereich. Hoch bedeutet: fokussierte Quelle rot oder automatische Ausgangswarnung. Stille bedeutet grün, kein Signal oder keine fokussierte Quelle. I steuert beide automatischen Warntöne, nicht die gelben und roten Konsolentöne. Es werden weder Verlauf noch Berichtsdateien gespeichert.

## Datenschutz und Lizenz

Canvas-Funktionen senden das erfasste Bild, die OBS-Sprache, feste Sicherheitsanweisungen und gültige Anschlussfragen an OpenAI. Voreinstellungen verwenden die bestehende Antwortkette; **Erneut prüfen** sendet absichtlich ein neues Vergleichsbild. Der API-Schlüssel wird nicht als Inhalt übertragen. Es gibt keine Telemetrie oder Werbung.

Copyright (C) 2026 [Tiflo.Info](https://tiflo.info). GNU GPL Version 2 oder neuer; siehe [LICENSE.txt](../LICENSE.txt). [English](../README.md).
