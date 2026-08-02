# FAQ zu den Audiofunktionen

Dieses Dokument erläutert Sound Doctor, Audible Meter und die barrierefreie Lautstärkekonsole in Accessible Studio 1.1.1.

## Überblick

### Worin unterscheiden sich diese Funktionen?

**Audible Meter** überwacht fortlaufend aktive Quellen und meldet Pegelprobleme durch Töne und Screenreader-Ansagen. Die **barrierefreie Lautstärkekonsole** dient zur Bedienung von OBS-Lautstärke, Ausgabe und Monitoring per Tastatur. **Sound Doctor** führt eine getrennte Diagnose durch, wertet Pegelstatistiken aus und schlägt optionale Kompressor- und Limiterfilter vor. Audible Meter und Sound Doctor können nicht gleichzeitig laufen.

### Werden meine Audiodaten aufgenommen, übertragen oder gespeichert?

Nein. Die Funktionen lesen ausschließlich numerische Pegelwerte von OBS. Sie erfassen keine Audiowellenform, erstellen keine Aufnahme, senden kein Audio über das Netzwerk und schreiben weder Messverlauf noch Berichte auf die Festplatte. Sitzungsstatistiken belegen nur festen Arbeitsspeicher und werden beim Beenden verworfen. Gespeichert werden lediglich normale Einstellungen und Quellidentitäten in der Stoppliste für Pegel vor dem Regler.

## Sound Doctor

### Was macht Sound Doctor genau?

Starten Sie ihn mit Strg+Umschalt+D oder über **Werkzeuge > Accessible Studio > Audiowerkzeuge > Sound Doctor**. Nach Ihrer Bestätigung beobachtet er aktive Audioquellen 120 Sekunden lang. Eine Quelle benötigt mindestens 45 Sekunden aktives Signal, um für eine Verarbeitungsempfehlung infrage zu kommen. Sind danach Streaming oder Aufnahme aktiv, läuft die Messung weiter und der Bericht wartet, bis beides beendet ist.

Pro Quelle führt Sound Doctor feste Verteilungen für RMS-Pegel, Spitzenbeobachtungen und aktive Zeit sowie Quellidentität, anfänglichen Reglerpegel und einen Fingerabdruck der Filterkette. Vorhandene Kompressor- und Limitereinstellungen werden ebenfalls geprüft. Wörter oder Inhalte werden nicht erkannt; Messungen werden an keinen KI-Dienst gesendet.

### Wie wird eine Kompressorempfehlung ermittelt?

Kompression wird nur für Quellentypen erwogen, die wahrscheinlich Live-Eingänge sind, etwa Mikrofone und direkte Audioeingänge. Sound Doctor vergleicht den Median des aktiven RMS-Pegels, also typische Passagen, mit dem 95. Perzentil, also lauteren Passagen. Die Differenz ist die gemessene Dynamikvariation.

Eine Empfehlung erscheint nur, wenn die Quelle mindestens 45 Sekunden aktiv war, Regler und Filterkette unverändert blieben, es sich wahrscheinlich um einen Live-Eingang handelt, kein Kompressor vorhanden ist und die Variation den eingestellten Mindestwert erreicht. Auch ein deaktivierter vorhandener Kompressor verhindert ein Duplikat.

Der vorgeschlagene Schwellenwert ist Median plus 5 dB, gerundet und auf −24 bis −10 dB begrenzt. Attack beträgt 6 ms, Release 100 ms und Ausgangsverstärkung 0 dB. Das Verhältnis ist adaptiv: 2:1 unter 10 dB Variation, 2,5:1 von 10 bis unter 14 dB und ab 14 dB der eingestellte Höchstwert. Standardmäßig sind Mindestvariation 8 dB und Höchstwert 3:1.

### Kann Sound Doctor Sprache zuverlässig von Musik unterscheiden?

Nein. Pegelwerte beschreiben Lautstärke und Dynamik, nicht den Inhalt. Der Quellentyp ist nur ein Hinweis auf einen wahrscheinlichen Live-Eingang. Ein Mikrofon kann Musik und eine Anwendungsaufnahme Sprache übertragen. Deshalb empfiehlt der Bericht, Kompression für hauptsächlich musikalische Quellen nicht auszuwählen.

### Wann wird ein Limiter empfohlen?

Standardmäßig für jeden wahrscheinlichen Live-Eingang mit mindestens 45 Sekunden aktivem Audio, unverändertem Regler und Filterweg und ohne vorhandenen Limiter. Clipping oder Spitzen nahe dem Grenzwert sind nicht erforderlich: Der Limiter ist vorbeugender Brickwall-Schutz. Er wird zuletzt eingefügt, standardmäßig mit −3 dBFS Grenzwert und 60 ms Release. Audio unter dem Grenzwert bleibt unverändert.

Die erweiterte Einstellung kann dies auf alle ausreichend aktiven Audioquellen ausweiten. Das bietet strengen Schutz, kann aber bei bereits gemasterter Musik oder Medien unnötig sein.

### Sind Kompressor- und Limiterempfehlung gekoppelt?

Nein. Sie werden unabhängig geprüft. Eine Quelle kann keine, eine oder beide Empfehlungen erhalten. Der Kompressor begrenzt Dynamikschwankungen; der abschließende Limiter setzt eine harte Spitzengrenze.

### Was meldet der Bericht außerdem?

Er zeigt vorhandene Kompressor- und Limitereinstellungen, warnt vor einem Limiter, der nicht zuletzt steht, meldet wiederholte Spitzen vor dem Regler nahe dem Clipping und erklärt unzureichende Messungen oder Änderungen während des Tests. Bereits vor OBS übersteuertes Audio muss am Gerät oder in der Quellanwendung abgesenkt werden.

### Ändert Sound Doctor automatisch etwas?

Nicht durch die Messung. Jede Empfehlung besitzt ein anfangs nicht markiertes Kontrollkästchen **Diese Änderung automatisch anwenden**. **Fertigstellen** wendet nur markierte Einträge an. Zuvor werden Quelle, Lautstärke und Filterkette erneut geprüft; veraltete Vorschläge werden übersprungen. Neue Filter tragen sichtbare Sound-Doctor-Namen, stehen in vorgesehener Reihenfolge und können mit OBS Rückgängig und Wiederholen behandelt werden. Escape schließt ohne Änderung.

### Kann ich die Empfehlungsregeln ändern?

Öffnen Sie **Werkzeuge > Accessible Studio > Audiowerkzeuge > Erweiterte Audioeinstellungen** und die Seite **Sound Doctor**:

- **Minimale Dynamikvariation für Kompression:** 6–15 dB, Standard 8 dB. Kleinere Werte erzeugen mehr Empfehlungen.
- **Maximales empfohlenes Kompressorverhältnis:** 2,5:1, 3:1 oder 3,5:1, Standard 3:1.
- **Quellen für Limiterempfehlungen:** standardmäßig nur wahrscheinliche Live-Eingänge oder alle aktiven Audioquellen.
- **Empfohlener Limiter-Grenzwert:** −12 bis −1 dBFS, Standard −3 dBFS.

Die Sitzung von 120 Sekunden und mindestens 45 Sekunden aktives Signal sind absichtlich fest. Anwenden speichert ohne Schließen, OK speichert und schließt, Abbrechen verwirft Änderungen seit dem letzten Anwenden.

## Audible Meter und automatische Pegelkorrektur

### Was überwacht Audible Meter?

Strg+I startet oder beendet einen fensterlosen Modus. Er verfolgt alle aktiven Audioquellen, verwendet Spitzen nach dem Regler für Ausgangswarnungen, Spitzen vor dem Regler für geeignete Live-Aufnahmen und RMS-Zusammenfassungen für typische Pegel. Er arbeitet unabhängig von Streaming und Aufnahme.

### Was bedeutet der automatische Warnton?

Der hohe automatische Ton bedeutet, dass mindestens eine Quelle die eingestellte tatsächliche Zeit am oder über dem Ausgangswarnpegel verbracht hat. Standard sind 1,5 Sekunden. Der Standardpegel folgt dem OBS-Messmodus: −9 dBFS bei Sample Peak oder −2 dBFS bei True Peak. Kurze Absenkungen unter einer Sekunde setzen die angesammelte Zeit nicht vollständig zurück; eine Sekunde unter der Grenze aktiviert die Warnung neu. Der Ton ist eine Warnung, keine genaue Messung; der Screenreader nennt die problematischste Quelle.

Der tiefere Warnton bedeutet: Nachdem Sie einer Anpassung zugestimmt haben, bleibt das Signal eines geeigneten Eingangs vor dem Regler im roten OBS-Bereich. Senken Sie den Pegel am Mikrofon, Interface, Windows-Eingang oder in der Quellanwendung. Der OBS-Regler kann diese Verzerrung nicht beheben.

### Was bedeuten die beiden Töne in der Lautstärkekonsole?

Wenn Audible Meter aktiv und die Konsole geöffnet ist, erzeugt die fokussierte Quelle im gelben OBS-Ausgangsbereich einen mittleren und im roten Bereich einen höheren Ton. Bei Grün, Stille, nicht verfügbarer Quelle oder Fokus außerhalb eines Quellsteuerelements ertönt nichts. Dies sind sofortige Messtöne, keine verzögerten Warnungen; I deaktiviert sie nicht.

Bei Sample Peak beginnt Gelb bei −20 dBFS und Rot bei −9 dBFS; bei True Peak beginnt Gelb bei −13 dBFS und Rot bei −2 dBFS.

### Senkt Audible Meter meinen Ton selbstständig?

Nein. Erst Umschalt+I während des hohen Ausgangswarntons fordert ausdrücklich eine Korrektur an. Für jede verantwortliche Quelle ist das Ziel Warnpegel minus Sicherheitsabstand. Audible Meter vergleicht damit die höchste beobachtete Spitze, rundet die nötige Absenkung auf die nächsten 0,5 dB auf und senkt den OBS-Regler höchstens um den eingestellten Maximalwert. Es erhöht nie einen Pegel und ändert bei einer reinen Vorreglerwarnung nichts.

Standard sind 3 dB Sicherheitsabstand und höchstens 12 dB Absenkung. Dies ist konservativer Spitzenschutz, keine künstlerische Mischung.

### Kann ich eine automatische Korrektur rückgängig machen?

Ja. Strg+Umschalt+I stellt die letzte Umschalt+I-Korrektur wieder her. Eine Quelle wird nur wiederhergestellt, wenn ihr Regler noch genau dem automatisch gesetzten Wert entspricht; später geänderte Quellen werden übersprungen. Start oder Ende von Audible Meter, ein Szenensammlungswechsel oder neue Einstellungen löschen diesen einstufigen Wiederherstellungseintrag.

### Wie ändere ich das Verhalten von Audible Meter?

Auf der Seite **Audible Meter** der erweiterten Audioeinstellungen:

- **Warnpegel für die Ausgabe:** −30 bis 0 dBFS; Standard −9 dBFS bei Sample Peak oder −2 dBFS bei True Peak.
- **Zeit über dem Warnpegel:** 0,1–30 Sekunden; Standard 1,5 Sekunden.
- **Spitzensicherheitsabstand:** 0–12 dB; Standard 3 dB.
- **Maximale automatische Absenkung:** 0,5–30 dB; Standard 12 dB.
- **Stoppliste vor dem Regler:** gespeicherte Ausnahmen entfernen, damit unveränderte Eingänge wieder geprüft werden.

Diese Werte verändern weder die festen OBS-Farbbereiche noch Konsolen-Messtöne, H/J/K/L-Messungen oder die rote Vorreglergrenze.

### Kann ich Warnungen vorübergehend abschalten?

Drücken Sie bei aktivem Audible Meter I. Beim Wiedereinschalten beginnt die Belichtungszeit neu. Konsolen-Messtöne bleiben aktiv. Buchstabenbefehle werden in bearbeitbaren Feldern niemals abgefangen.

### Wofür stehen H, J, K und L?

- H nennt aktuellen Nachreglerpegel und Bereich der zuletzt in der Konsole fokussierten Quelle.
- J nennt die momentan lauteste aktive Quelle.
- K nennt den typischen aktiven RMS-Pegel der ausgewählten Quelle in dieser Sitzung.
- L nennt die Quelle mit dem lautesten typischen aktiven RMS-Pegel der Sitzung.

### Warum hat eine Vorreglerwarnung Vorrang?

Sie weist auf möglichen Schaden vor dem OBS-Regler hin. Der Entscheidungsdialog ist still. Ja startet die Begleitung; der tiefe Ton bleibt, solange der Eingang rot ist. Ein länger gesundes aktives Signal beendet sie. Stille gilt nicht als Erfolg: Ansagen erfolgen nach 2 und 12 Sekunden, nach 22 Sekunden folgt eine Frage. Nein oder Escape speichert die unveränderte Quelle in der Stoppliste. Eine geänderte Identität oder Einstellung wird neu geprüft.

### Was kann die barrierefreie Lautstärkekonsole?

Strg+Gravis öffnet sie. Links/Rechts wählen Quellen, Hoch/Runter ändern den fokussierten Regler um 1 dB, Pos1 setzt 0 dB, 1–9 wählen die ersten neun und 0 die zehnte Quelle. Leertaste schaltet Monitoring und Programmausgabe zusammen, Strg+Leertaste nur Monitoring, Umschalt+Leertaste nur Ausgabe. Normalerweise erscheinen aktive Programmquellen; **Alle Quellen anzeigen** schließt inaktive Quellen ein. Änderungen wirken sofort. Eine normal konfigurierte Quelle wird nicht selbstständig über 0 dB angehoben.

### Welcher Arbeitsablauf ist am sichersten?

Führen Sie Sound Doctor mit repräsentativer Sprache und Audiowiedergabe aus und prüfen Sie Vorschläge einzeln. Lassen Sie Musikkompression abgewählt, sofern dieser Effekt nicht gewünscht ist. Nutzen Sie Audible Meter im Alltag, die Konsolentöne zur Sofortkontrolle, beheben Sie Vorreglerprobleme am ursprünglichen Gerät oder Programm und verwenden Sie Umschalt+I nur für eine ausdrücklich gewünschte konservative Absenkung.
