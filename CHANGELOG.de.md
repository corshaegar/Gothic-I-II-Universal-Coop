# Änderungen

> *English version: [CHANGELOG.md](CHANGELOG.md).*

Jeder Abschnitt nennt die Projektversion und für jedes Spiel und jede Mod die Coop-Fassung (`uNN`); die neueste Version steht
oben. Ganz unten folgt die Vorgeschichte: der Othello-Coop 0.1.0–0.1.2 (github.com/corshaegar/Othello-Coop) und die
unveröffentlichten Fassungen u1–u61, aus denen 0.2.0 hervorging. So ist hier jede Änderung seit der allerersten Fassung festgehalten.

## 0.11.1 – Stillstand beim Host alle 5 Sekunden, Archolos aus Steam (Gothic I u254, Gothic II u254, Othello u254, Legend of Ahssun u254, Archolos u254, Xeres' Rückkehr u254)

Gefunden in den Logs zweier Spieler, die am 12.11.2026 Archolos zusammen gespielt haben.

**Korrekturen**

- **Das Spiel des Hosts stand alle 5 Sekunden still** (u253): Eine alte Diagnose aus den Anfängen des Projekts schrieb, solange man verbunden war, alle 5 Sekunden einen Schnappschuss aller Figuren in der Nähe nach `system\GothicCoopSnapshot.log` – und öffnete und schloss die Datei dabei für jede einzelne Zeile neu. Auf einem PC, dessen Virenscanner jede Datei beim Schließen prüft, dauerte das umso länger, je größer die Datei wurde: Im Log des Spielers stand der Host anfangs 35 ms still, nach 13 Minuten fast 0,9 Sekunden. Auch der Mitspieler bekam das mit, weil in diesen Momenten keine Bewegungen vom Host kamen. Der Schnappschuss ist jetzt abgeschaltet, ebenso zwei weitere Diagnosen, die niemand mehr braucht: eine beim Client, die nach längst behobenen Fehlern suchte, und eine Liste der NPC-Namen, die beim Laden über 1.000 Zeilen auf einmal schrieb. Auch der Zeitraffer der automatischen Testläufe ist nicht mehr im Release-Bau. Was bei Fehlermeldungen hilft, bleibt: Bildzeiten und eine Netz-Zusammenfassung alle 30 Sekunden, das Log, das an den Partner geht, die Selbstprüfung beim Start und die Berichte über Hänger und Abstürze.
- **Archolos lief beim Start aus Steam ohne sein Profil** (u254): Steam startet Archolos mit `-game:TheChroniclesOfMyrtana.ini`. An diesem Ini-Namen erkannte der Coop die Mod nicht und behandelte Archolos wie eine unbekannte Mod – ohne die Anpassungen, die es dafür gibt, etwa die Werte, die Archolos je PC führt (Abblenden, Abstands-Warnungen, die Zähler der Zwischensequenzen im Prolog). Startete nur einer der beiden Archolos auf diesem Weg, lehnte der Coop außerdem die Verbindung ab, als spielten beide verschiedene Spiele. Jetzt erkennt der Coop Archolos auch beim Start mit eigener Ini an `Data\KM_Scripts.mod` – beide Startwege führen zum selben Profil.

## 0.11.0 – Xeres' Rückkehr, Installer für Archolos, Absturz durch Blutspritzer, eingesperrte Maus (Gothic I u252, Gothic II u252, Othello u252, Legend of Ahssun u252, Archolos u252, Xeres' Rückkehr u252)

**Neu**

- **Xeres' Rückkehr** (u246–u252): eigenes Profil in der Gothic-2-DLL, Start mit `-game:Xeres.ini`. Vorher wurden alle 6.583 Skript-Funktionen der Mod gelesen und jeder schwere Befund am Code geprüft. Jeder Held behält seine eigenen Gifte, Krankheiten, Übung, Götterbonus, Verwandlungen und Perks; die Zähler der Szenen, die Lorenfahrt und die Einstellungen der Mod gelten je PC. Geprüft in automatischen Testläufen mit neuem Spiel (Host und Client auf einem PC). Die Geschichte ist noch nicht durchgespielt. Für Spieler ohne Coop gibt es die Fehlerbehebungen auch als eigenes Paket: [Xeres-Fixes](https://github.com/corshaegar/Xeres-Fixes).
- **Der Installer erkennt Archolos**: Bis 0.10.3 lehnte er den Ordner von Archolos ab, weil Archolos die Addon-Welt nur als leere Datei beilegt. Jetzt erkennt er Archolos wie der Coop selbst an `Data\KM_Scripts.mod` und installiert ganz normal.

**Korrekturen**

- **Archolos startete nach der Installation von Hand nicht mehr**: ZIP und Installer ersetzten `system\steam_api.dll` und `steam_appid.txt`. Archolos bringt eine eigene Steam-Anbindung mit, die eine Funktion braucht, die es in der neueren `steam_api.dll` des Coops nicht mehr gibt – danach startete Archolos nicht. Der Installer lässt die eigenen Steam-Dateien eines Spiels jetzt stehen; in der ZIP liegen sie in eigenen Ordnern und werden nur kopiert, wenn das Spiel keine hat. Coop über Steam bleibt in Archolos deshalb aus – dort über IP verbinden. Startet dein Archolos nicht mehr: in Steam die Spieldateien überprüfen lassen und danach mit dem neuen Installer installieren.
- **Absturz, wenn einen Pfeile treffen und Blutspritzer an sind** (u249–u252): Mit dem Union-Toolkit (`Data\Toolkit.vdf`, das mehrere Union-Plugins mitbringen) ersetzt LeGo 2.9 das ältere LeGo einer Mod. Der Blutspritzer und die Schadens-Funktion von Mods wie Xeres' Rückkehr riefen sich dann gegenseitig auf, bis das Spiel abstürzte. Jetzt bekommt die Mod ihre eigene Blutspritzer-Funktion zurück: keine Schleife mehr, und der Blutspritzer zeigt den tatsächlichen Schaden. Geprüft mit einem nachgestellten Treffer aus dem Bogen; ohne die Korrektur blieb das Spiel genau dort hängen.
- **Objekte oberhalb von 2 GB galten als nicht vorhanden** (u246): Gothic 2 darf bis zu 4 GB Speicher nutzen, und manche Mods prüfen die Adresse eines Objekts mit `> 0` – ein Objekt in der oberen Hälfte gilt dann als nicht vorhanden. In Xeres' Rückkehr betraf das 20 Stellen (Weltwechsel, der Pass ins Minental, die Türen des Spinnenbaus, die Loren, Truhen). Der Coop stellt solche Prüfungen beim Spielstart richtig, in jeder Mod.
- **Maus oben links eingesperrt** (u251, u252): Der Weltprozess – ein zweites Gothic, das der Coop im Hintergrund auf einem eigenen Windows-Desktop laufen lässt – sperrte die echte Maus manchmal in sein unsichtbares Fenster. Das unterbindet der Coop jetzt von Anfang an. Gemessen in zwei Testläufen mit bis zu fünf Gothic-Instanzen: kein einziges Mal eingesperrt (vorher etwa 40 Sekunden je Lauf).
- **Archolos: NPC blieb im Gespräch stehen, doppelte NPCs** (u244, u245): Ein NPC konnte mitten im Gespräch stehen bleiben, weil seine Befehlsliste nicht weiterlief; der Dialog-Wächter stößt sie jetzt wieder an. Ein NPC, den ein Skript gerade eingesetzt hatte, entstand beim Partner manchmal ein zweites Mal (etwa Ana und Lea); der Coop übernimmt den eigenen NPC jetzt auch auf Entfernung, wenn er der einzige seiner Art ist.
- **Anleitung zur Installation von Hand**: Union fehlt, wenn `system\Union.patch` fehlt – die alte Anleitung nannte `system\Shw32.dll`, die jedes Gothic hat.

## 0.10.3 – Szenen im Archolos-Prolog, Kämpfe in Othello (Gothic I u243, Gothic II u243, Othello u243, Legend of Ahssun u243, Archolos u243)

Gefunden in automatischen Testläufen am 09.11.2026 in Archolos und Othello (Host und Client auf einem PC).

**Korrekturen**

- **Kein Speichern mehr mitten in einer Szene** (u243): Wurde während einer Szene gespeichert – automatisch, gemeinsam mit dem Partner oder beim Beitritt des Partners –, stand das Spiel kurz still. Danach holten die Skripte der Mod die verpassten Schritte auf einen Schlag nach und übersprangen dabei Teile der Szene. In Archolos fiel so beim Host die Szene mit dem Kapitän auf dem Schiff aus, während sie beim Client lief. Wie Gothic im Einzelspielermodus wartet der Coop jetzt mit dem Speichern, solange eine Kamerafahrt oder eine Szene der Mod läuft. Im Test lief der Prolog vollständig durch.
- **Unverwundbar nach einem Gespräch des Partners** (u242): Führte der Partner ein Gespräch, machte das Dialogskript des NPCs auch den eigenen Helden unverwundbar – und das blieb so. In Othello griffen Monster einen solchen Helden nicht an, sondern liefen weg; der Client traf in 9 von 10 Kämpfen nichts. In manchen Mods redeten NPCs dann nicht mehr mit ihm. Jetzt hebt der Coop die Unverwundbarkeit 10 Sekunden nach dem Ende des Gesprächs wieder auf. Im Test in Othello traf der Client in allen 13 Kämpfen.
- **Doppelte Kamerafahrten** (u241): War der Client in einer Szene hinter dem Host zurück, konnte eine Kamerafahrt ein zweites Mal ablaufen. Der Host erkennt jetzt auch Schritte, die er selbst schon bis zu zwei Minuten vorher ausgeführt hat, und wiederholt sie nicht.

## 0.10.2 – Speichern in Mods mit LeGo ohne Absturz, Archolos-Prolog bis zum Ende (Gothic I u240, Gothic II u240, Othello u240, Legend of Ahssun u240, Archolos u240)

Gefunden in automatischen Testläufen am 09.11.2026 in Archolos (Host und Client auf einem PC; der Client trat mitten im Prolog bei).

**Korrekturen**

- **Absturz beim Speichern in Archolos und anderen Mods mit LeGo** (u239): Mit 0.10.0 und 0.10.1 stürzte das Spiel bei jedem Speichern ab, wenn sich die Skripte einer Mod per LeGo in das Speichern einhängen (z. B. Archolos). Seit 0.10.0 setzt dort auch der Coop einen Hook, damit nur gemeinsam gespeichert wird. LeGo kopiert beim Spielstart den Sprung in den Coop an eine andere Stelle, wo er ins Leere zeigt. Solche Kopien korrigiert der Coop zwar seit 0.6.0 (damals für Legend of Ahssun), die neuen Stellen kannte er aber nicht. Jetzt erkennt er beim Spielstart selbst jede Stelle, an der er oder ein anderes Plugin einen Hook gesetzt hat. Geprüft in Archolos: der Spielstand zu Spielbeginn und gemeinsame Sicherungen.
- **Archolos: Das Ende des Prologs hing** (u240): Manche Szenen spielt jeder PC für sich ab. Sie blieben stehen, wenn der Client dabei schon weiter war als der Host: Die Skripte des Clients töteten oder versetzten NPCs, auf die die Szene beim Host noch wartete – auf dem Schiff muss zum Beispiel erst Rupert ins Wasser springen. Die Kamera des Clients blieb in der letzten Einstellung hängen, beide Helden standen an Deck. Jetzt wartet der Host bis zu 30 Sekunden, ob seine eigene Szene denselben Schritt ausführt; erst wenn nicht, übernimmt er den Schritt des Clients. Im Test war der Client wieder voraus, das Ende des Prologs lief auf beiden PCs durch, und beide Helden kamen am Strand an.

## 0.10.1 – Teleport-Rune per Taste in die Hand, immer genau eine; aufgeräumte Meldungszeile (Gothic I u238, Gothic II u238, Othello u238, Legend of Ahssun u238, Archolos u238)

**Neu**

- **Teleport-Rune per Taste** (u238): Die Runen-Taste (standardmäßig Shift+T) nimmt die Teleport-Rune in die Hand. Fehlt sie, kommt sie vorher ins Inventar und ins Zauberbuch; ein zweiter Druck steckt sie wieder weg. Gewirkt wird sie wie jede andere Rune. Bisher teleportierte die Taste sofort.
- **Immer genau eine Coop-Rune** (u238): Jeder Held trägt seine Teleport-Rune fest bei sich, in Gothic 1 und Gothic 2, mit und ohne Mod – nach jedem Laden auch im Zauberbuch, sofern dort Platz ist. Fehlt sie, kommt sie zurück; hebt man eine zweite auf, bleibt es trotzdem bei einer. Eine Rune, die frei in der Welt liegt, verschwindet nach wenigen Sekunden.

**Korrekturen**

- **Meldungszeile** (u238): Der Hinweis auf das Zuschau-Fenster und seine Taste erscheint nur noch einmal pro Sitzung statt bei jedem automatischen Wechsel. Schwierigkeitsgrad und Abgleich zeigt der Client nur noch an, wenn der Host sie während des Spiels ändert; die Werte beim Verbinden stehen nur im Log.

## 0.10.0 – Nur noch gemeinsam speichern, Coop-Tasten und Hilfe, Archolos-Prolog mit Trialogen (Gothic I u237, Gothic II u237, Othello u237, Legend of Ahssun u237, Archolos u237)

Entstanden aus automatischen Testläufen mit Aufnahmen am 08. und 09.11.2026 in Archolos und Legend of Ahssun (Host und Client auf einem PC, beide Helden automatisch gesteuert) und einem Welten-Testlauf in Gothic 2 mit u237.

**Neu**

- **Nur noch gemeinsam speichern** (u227, u236, u237): Beide PCs speichern im selben Moment – laut den Dateizeiten der Spielstände innerhalb einer Zehntelsekunde, auch bei automatischen Sicherungen. Bisher speicherte der Partner erst, wenn der andere fertig war, und wenn er gerade beschäftigt war, sogar noch später – mit einem anderen Spielstand. Jetzt teilen sich beide PCs laufend mit, ob sie gerade mitspeichern können. Kann der Mitspieler gerade nicht (Dialog, offenes Inventar, Ladevorgang), lehnt das Spiel das Speichern ab und nennt den Grund (z. B. „Diego: Dialog“); die automatische Sicherung wartet so lange.
- **Coop-Tasten** (u226): Alle Coop-Tasten lassen sich unter Einstellungen › COOP › Coop-Tasten frei belegen, auch mit Shift, Strg oder Alt; ist eine Taste schon von Gothic belegt, steht das dabei. Neu belegt sind Hilfe (H, oder F1, wenn Gothic H schon verwendet), Coop-Fenster (Shift+K) und Teleport zum Mitspieler (Shift+T, wenn die Rune im Inventar ist).
- **Hilfe, Coop-Fenster, Einführung** (u226): Ein Hilfe-Menü zeigt alle Tasten und erklärt kurz das Wichtigste. Das Coop-Fenster öffnet sich per Taste oder unter Einstellungen › COOP (Hosten, Beitreten und Trennen auch während des Spiels). Vor einem neuen Spiel erscheint eine kurze Einführung (abschaltbar), und im Spiel gibt es Hinweise – beim Hosten zu Port und Firewall, und wenn der Mitspieler bewusstlos am Boden liegt, zur Taste fürs Wiederbeleben.
- **Mitspieler-Anzeige** (u226): Name, Entfernung, Darstellung hinter Wänden und der Pfeil am Bildrand sind einstellbar (Einstellungen › COOP › Mitspieler-Anzeige). Hat man den Mitspieler im Fokus, steht sein Name nicht mehr doppelt da.
- **Aussehen wie ein NPC** (u222): Die Auswahl kennt jetzt alle 654 Figuren von Gothic 2, auch in Mods; die Figuren des laufenden Spiels stehen vorn.
- **Kopfdrehung** (u235): NPCs drehen den Kopf auf beiden PCs in dieselbe Richtung; bisher blickten sie beim Client meist zum eigenen Helden.
- **Zuschau-Fenster** (u225): Bei niedriger Bildrate zeichnet es nur noch jedes zweite bis vierte Bild neu – das Spiel ruckelt weniger.

**Korrekturen**

- **Archolos, Prolog auf dem Schiff** (u218, u221, u224, u228, u229): Szenen und Zwischensequenzen laufen auf beiden PCs bis zum Ende. Ein Gespräch des Partners löscht keine Befehle einer Szene mehr; beginnt beim Partner eine Szene, wird das eigene Gespräch beendet, oder die Szene wartet, bis es vorbei ist; Zähler, die den Ablauf einer Szene steuern, starten beim Partner richtig; Todesskripte und Szenen, die auf beiden PCs laufen, lösen nichts doppelt aus. Dadurch bleibt der Kapitän nicht mehr hinter dem Gitter stehen.
- **Trialoge** (LeGo – Archolos und viele andere Mods; u220, u230–u235): In Gesprächen mit mehreren NPCs (auf dem Schiff: Kapitän, Jorn und Timo) sieht der Partner den richtigen Sprecher in der richtigen Reihenfolge. NPCs springen nicht mehr an andere Stellen, die Zeilen der anderen Sprecher erscheinen, und Überspringen trifft die richtige Zeile.
- **Gespräche mit demselben NPC** (u220): Sprechen beide Spieler gleichzeitig mit demselben NPC, stören sich die Gespräche nicht mehr.
- **Geleit** (u223): Führt ein NPC die Gruppe, misst er den Abstand zum nächsten Spieler statt nur zum Helden des eigenen PCs.
- **Einstellungen › COOP** (u227): Der Eintrag erscheint jetzt auch in Mods, deren Menü keinen Grafik-Eintrag von Gothic hat (z. B. Archolos).

**Entfernt** (u237)

- Die Liste der Coop-Sicherungen im Spiel (F9, F8) – die Sicherungen stehen im Menü unter Esc › „Spielstand wählen“.
- Die Taste für den Weltprozess (Shift+F12) – er startet weiterhin von selbst.
- Der Coop-Testlauf mit seinen Tasten (Shift+F11, F7), die Netzwerk-Statistik (O) und der Bearbeitungsmodus der Vorlage (`editmode`). Testläufe gibt es nur noch in der Entwicklungsfassung.

## 0.9.1 – Archolos: NPCs reden wieder nach hängender Abblendung (Gothic I u217, Gothic II u217, Othello u217, Legend of Ahssun u217, Archolos u217)

Aus dem siebten und achten einstündigen automatischen Testlauf am 08.11.2026 (Archolos und Legend of Ahssun gleichzeitig, Host und Client auf einem PC): kein Absturz, kein Hänger, und die abschließenden Prüfrunden fanden keinen Fehler.

**Korrekturen**

- **Abblendsperre** (u217): Archolos lässt kein Gespräch zu, solange das Bild abgeblendet ist (Skriptvariablen `SCREENFADED` und `FADESCREENSTATE`) – auch nicht die wichtigen Dialoge, mit denen NPCs die Geschichte voranbringen. Wurde mitten im Abblenden gespeichert und dieser Stand mit einer Fassung vor u211 geladen, wurde nie wieder aufgeblendet: Danach redete kein NPC mehr mit dem Helden, auch in jedem späteren Spielstand. Bleibt das Bild jetzt 60 Sekunden lang abgeblendet, ohne dass ein Dialog offen ist, blendet der Coop mit der Funktion der Mod selbst auf; das hebt auch die Bewegungssperre des Helden auf. Im Testlauf griff das nach dem Laden solcher Stände bei Host und Client; danach redeten die NPCs wieder, und Rupert führte die Helden weiter. Das gilt für alle Mods mit diesen Abblendfunktionen.

## 0.9.0 – Stabilität: Absturz beim Laden, fliegende Monster, Hänger-Diagnose (Gothic I u216, Gothic II u216, Othello u216, Legend of Ahssun u216, Archolos u216)

Grundlage sind ein langer Lauf in der Nacht (drei Abstürze beim Laden in Archolos, daraus entstand u211) und sechs einstündige automatische Testläufe am 08.11.2026 in Archolos und Legend of Ahssun (beide gleichzeitig, Host und Client auf einem PC, jeder Lauf ausgewertet). In den sechs Läufen gab es keinen Absturz und einen Hänger (im zweiten Lauf, Legend of Ahssun, beim Weltwechsel; danach kam die Hänger-Diagnose dazu, seitdem trat keiner mehr auf). Die Prüfrunden am Ende der letzten vier Läufe fanden keinen Fehler.

**Korrekturen**

- **Absturz beim Laden** (u211): In Mods, die Funktionszeiger von Ikarus im Spielstand ablegen, stürzte das Spiel nach dem Laden ab, wenn der Skriptcode im neuen Prozess an einer anderen Adresse lag – in Archolos bei Spielständen, die während einer Abblendung gespeichert wurden (im Coop kann der Partner jederzeit ein gemeinsames Speichern auslösen). Der Coop speichert jetzt die Adresse des Skriptcodes im Spielstand und rechnet solche Zeiger beim Laden um. Das gilt für alle Mods.
- **Fliegende Monster** (u212–u215): Beim Client schwebten vergrößerte Monster (Archolos: Goblin-Krieger, Lurker; Legend of Ahssun: alte Maulwurfsratten, Fleischwanzen) Hunderte Meter bis Kilometer über ihrem Platz. Ursache ist ein Fehler der Engine: Bei vergrößerten Modellen weit weg von der Kamera wächst die Bodenebene der Animation von Bild zu Bild, und die Bodenkorrektur hebt die Figur um genau diesen Betrag an. Der Coop hält den Wert jetzt fest (Gothic 2; in Gothic 1 berechnet er ihn bei NPCs des Hosts neu, sobald er zu groß geworden ist).
- **Zauber des Partners** (u211): Wurde ein Zauber des Partners nachgespielt, während das Zauberbuch leer war, stürzte das Spiel ab, und ein abgebrochenes Nachspielen blieb danach hängen. Beides ist behoben.
- **Gegenstände in NPC-Händen** (u211): Wird ein Gegenstand gelöscht, den ein NPC gerade benutzt, entfernt der Coop sofort den Verweis darauf.
- **Hänger-Diagnose** (u213): Steht das Spiel 90 Sekunden still, schreibt der Coop den Call-Stack des Haupt-Threads, den Skript-Trace und den Spielzustand nach `system\GothicCoopStapel.log`. So lassen sich auch Hänger untersuchen, bei denen das Spiel nicht abstürzt.
- **Gesprächssperre abgesichert** (u216): Archolos setzt zu Beginn eines Gesprächs beim Helden das Flag „unverwundbar im Gespräch“ und nimmt es nur beim regulären Ende wieder zurück; solange es gesetzt ist, beginnt kein NPC ein Gespräch mit ihm. Endet ein Gespräch anders (Laden mitten im Dialog, Abbruch), bliebe der Held gesperrt, auch im Spielstand. Der Coop setzt das Flag 10 Sekunden nach dem Gesprächsende und nach dem Laden zurück, außer die Mod braucht es gerade (offener Dialog, Benutzung eines interaktiven Objekts, gewollte Unsichtbarkeit). Gefunden beim Lesen der Mod-Skripte; in den Testläufen bisher nicht ausgelöst.

## 0.8.0 – The Chronicles of Myrtana: Archolos, Schlafen beim Client, Tote beim Weltwechsel, Handel (Gothic I u210, Gothic II u210, Othello u210, Legend of Ahssun u210, Archolos u210)

**Neu**

- **The Chronicles of Myrtana: Archolos** (u204–u210): Der Coop erkennt Archolos (an `Data\KM_Scripts.mod`) und bringt ein eigenes Mod-Profil mit: Werte, die pro PC oder pro Held getrennt bleiben, eigene Zähler für Zwischensequenzen, Abblendung und Abstandswarnungen der Mod für jeden PC sowie einen COOP-Eintrag im Hauptmenü der Mod. Behoben sind ein Absturz beim ersten Speichern (u205) und der Prolog auf dem Schiff, der im Coop nicht zu Ende kam (u207). Geprüft in langen automatischen Testläufen (zusammen etwa 7 Stunden ohne Absturz); gespielt wurden bisher der Prolog und Kapitel 1 bis zu Fabios Höhle.
- **Schlafen beim Client** (u204): Schläft der Client oder stellt ein Skript bei ihm die Uhr, gilt die neue Uhrzeit für beide.
- **Handel** (u201): Käufe und Verkäufe bei einem Händler gelten jetzt für beide – Ware und Gold des Händlers stimmen auf beiden PCs überein. Bisher konnte der Partner dieselbe Ware noch einmal kaufen. Gilt für Gothic 1 und 2 und alle Mods; im normalen Spiel noch nicht bestätigt.
- **Mod-Profile: Objektzustände** (u201): Neuer Eintrag `vobSync` – ändern Mod-Skripte Sichtbarkeit, Kollision, Transparenz oder Aussehen benannter Objekte in der Welt, gilt das für beide Spieler in derselben Welt. Legend of Ahssun: Die Barrikade beim Ausheben des Nests (Kapitel 5) ist jetzt auch für den Partner offen; Orkgräber, gefällte Bäume, Blutspur und Folterbänke sehen bei beiden gleich aus.

**Korrekturen**

- **Tote beim Weltwechsel** (u203): Nach dem Betreten einer Welt fehlten beim Partner manche Leichen (Legend of Ahssun: die tote Fleischwanze), und Tote, die der Host nach seinem Laden erzeugt hatte, trugen beim Partner einen anderen Namen.
- **Gespräche mit NPCs, die erst aufstehen müssen** (u206, u210): Der Client konnte NPCs, die lehnen, sitzen oder Wache stehen, nicht ansprechen. Jetzt stehen sie auf; beim Client spielen sie dabei nur das Aufstehen selbst ab und folgen ansonsten weiter dem Stand des Hosts.
- **Skript-Teleports über Ikarus** (u204): Versetzt ein Skript beim Client einen NPC des Hosts per Ikarus, geschieht das jetzt auch beim Host.
- **Gleichzeitig geänderte Skriptwerte** (u204): Ändern beide PCs denselben Wert fast gleichzeitig (Archolos würfelt beim Tageswechsel über 100 Ereignisse aus), behält der Host seinen Wert.
- **Legend of Ahssun** (u202): 15 weitere Einträge im Mod-Profil, gefunden bei der Durchsicht der Analyse.
- **Absturzprotokoll** (u205, u210): Stack-Overflows stehen jetzt in `system\GothicCoopStapel.log`; bei gelöschten Gegenständen in NPC-Händen und bei verschwundenen Leichen nennt das Protokoll mehr Aufrufer.

## 0.7.1 – Legend of Ahssun: weitere Werte pro PC und pro Held, mehr NPC-Zustände für den Spieler vor Ort (Gothic I u200, Gothic II u200, Othello u200, Legend of Ahssun u200)

Aus der weiteren Durchsicht der Analyse von Legend of Ahssun (Vorschläge zu Werten und alle NPC-Zustände, die den Helden betreffen). Am Code geprüft und im automatischen Testlauf, im normalen Spiel noch nicht bestätigt.

**Korrekturen**

- **Legend of Ahssun** (u200): Der Traum sperrte auch beim Partner das Inventar; eine Antwort beim Thronwächter sperrte die Gespräche beider Spieler; die Kälte- und die Durstleiste sprangen zwischen den Werten beider Helden hin und her; Kochtalent, Wasserwissen, eine Infektion, einmalige EP beim Lesen und Menüzustände (Beten, Farben mischen, Tabak) wurden auf den Partner übertragen; offene Anzeigen, Glücksspiel-Einsätze, Taschendiebstahl und Effekte des Magus-Kampfs vermischten sich zwischen den PCs. All das ist behoben.
- **Legend of Ahssun, NPCs** (u200): Hinterhalte, Wachen, Wartende, Führer, Begleittiere, Schlafende und ein Geist reagieren auf den Spieler, der vor Ort ist, nicht mehr immer auf den Host – der Geist traf bisher beide Helden. Begleiter und Beschwörungen folgen weiterhin ihrem eigenen Spieler.
- **Legend of Ahssun, Welten** (u200): Wer die Goldmine in Kapitel 4 als Zweiter allein betrat, fand dort noch alle Bewohner lebend vor: Das Startskript der Welt übersprang den entsprechenden Teil, weil das Flag schon vom Partner gesetzt war. Solche Flags gehören jetzt zur Weltdatei (neuer Profil-Eintrag `worldVars`), ebenso das Inventar des Untoten Drachen.
- **Legend of Ahssun, Zähler** (u200): Gute Taten, abgebautes Erz und 24 weitere Zähler aus Gesprächen (Respekt der Handwerker und Rekruten, Stimmen der Kämpfer, gewonnene Kämpfe …) gehen nicht mehr verloren, wenn beide Spieler gleichzeitig verschiedene Gespräche führen: Beide Schritte zählen.

## 0.7.0 – Legend of Ahssun vollständig analysiert, Begleiter und Beschwörungen des Clients, Skript-Ereignisse beim Client sofort wirksam (Gothic I u199, Gothic II u199, Othello u199, Legend of Ahssun u199)

Die Änderungen aus der Analyse von Legend of Ahssun sind am Code der Mod geprüft, die so gekennzeichneten auch im automatischen Testlauf; im normalen Spiel hat sie noch niemand erlebt.

**Neu**

- **Legend of Ahssun vollständig gelesen** (u198, u199): Alle 3.914 eigenen Funktionen der Mod wurden vor dem nächsten Test gelesen (per Mustersuche in den Skripten und mit einem lokalen Sprachmodell), jeder schwere Befund wurde einzeln am Code geprüft. Daraus stammen die Änderungen unten; so wird künftig jede neue Mod geprüft.
- **Begleiter und Beschwörungen des Clients** (u199): Sie folgen jetzt dem Client. Bisher liefen sie immer zum Host, weil die KI der NPCs beim Host läuft (Gothic 2: Beschwörungen; Legend of Ahssun: angeworbene Begleiter). Im Log steht dazu „Begleiter … gehoert zu FRIEND_n“.
- **Spawns aus Skripten des Clients** (u199): Was Skripte des Clients mit `Wld_SpawnNpcRange` erscheinen lassen (Beschwörungen, Gräber, Fallen, Boss-Angriffe), erscheint jetzt bei beiden; bisher entstanden diese NPCs nur beim Client.
- **Zauber des Partners** (u199): Wird ein Zauber des Partners nachgespielt, hat das keine Skriptwirkung mehr. Bisher erzeugte eine Beschwörung des Partners auf dem eigenen PC die feindliche Variante (Gothic 2), und Runen mit Dauerwirkung wirkten auch beim Partner.
- **Mod-Profile: Zähler** (u199): Neuer Eintrag `deltaVars` – Zähler, die nur Skripte der Spieler erhöhen, gleicht der Coop als Änderung ab statt als festen Wert; zwei gleichzeitige Schritte zählen beide.
- **Mod-Profile: „der Spieler vor Ort“** (u199): Neuer Eintrag `heroStates` – in den aufgeführten KI-Zuständen meint `HERO` den Spieler, gegen den der NPC kämpft, sonst den nächsten Spieler. Legend of Ahssun: Die Beschwörung im Sumpf, die Faustkampf-Arena, Kastor/Parcival und die Illusionen des Magus beziehen sich jetzt auf den Spieler vor Ort, bisher immer auf den Host (die Arena nahm sogar dem Host die Waffe ab, wenn der Client kämpfte). Der Mechanismus ist im automatischen Testlauf geprüft.
- **Beitritt nur mit demselben Spiel** (u198): Beim Verbinden werden Engine und Mod mitgeschickt; ein Client mit einem anderen Spiel trennt die Verbindung, bevor er einen Spielstand lädt (bisher stürzte etwa ein Gothic-2-Client an einem Gothic-1-Host beim Laden ab).

**Korrekturen**

- **Abstürze beim Client** (u199): Skripte, die einen gerade eingefügten NPC oder Gegenstand sofort weiterverwenden, griffen beim Client ins Leere (Legend of Ahssun: Kapitel 5 beim Gespräch mit Argos, Tor im Tempel des Erwachens, Arena Runde 9). Der Client legt das Objekt jetzt sofort selbst an, und die spätere Nachricht des Hosts wird diesem Objekt zugeordnet (im automatischen Testlauf in Gothic 2 und Legend of Ahssun geprüft).
- **Legend of Ahssun** (u199): Geben und Verkaufen von Gegenständen, der Schleifstein und das Zerlegen von Runen liefen beim Partner mit (sein ausgewählter Gegenstand war weg, die Bewegungssperre blieb hängen); der Segen am Schrein nahm dem Partner nach Ablauf Werte, die er nie bekommen hatte; Fluchpunkte, Rezepte, Lesepult-Boni, Charakterwerte und weitere Werte des eigenen Helden gehen nicht mehr auf den Partner über; löschen beide gleichzeitig Brände, zählen beide (Quest „Löscharbeit“).
- **Gothic 2** (u199): Die Übung beim Goldhacken bleibt beim eigenen Helden.
- **Doppelte Ereignisse** (u199): Fügen beide PCs binnen 3 Sekunden dasselbe ein, fasst der Host das jetzt auch dann zusammen, wenn die Skripte den Wegpunkt auf jedem PC anders auswürfeln.
- **Testlauf** (u198, u199): Zwei Läufe, die kurz nacheinander starten, belegen nicht mehr denselben Port; der Engine-Vergleich hängt nicht mehr an der Schreibweise der Modulnamen; der Diagnoseschritt mit dem Apfel funktioniert in Gothic 1 und Othello.

## 0.6.0 – Ein Coop für alle Spiele und Mods, Legend of Ahssun, Mod-Profile (Gothic I u197, Gothic II u197, Othello u197, Legend of Ahssun u197)

**Neu**

- **Ein Coop für alle Spiele und Mods** (u195, u197): Der Installer fragt nicht mehr nach dem Spiel. Er erkennt am Inhalt des Ordners Gothic 1, Othello, Gothic 2 und die darin installierten Mods und zeigt schon vor der Installation, welche unterstützt werden. Installiert wird der Coop für die jeweilige Engine samt den Skripten aller Varianten. Wechselt die Mod später (über den GothicStarter, oder Othello kommt hinzu oder wird entfernt), stellt sich der Coop beim Start selbst um (im Log: „Skript-Weiche“). Statt einer ZIP pro Spiel gibt es eine Universal-ZIP mit einem Ordner pro Engine.
- **Legend of Ahssun** (Gothic 2, u170–u197): Die Mod wird als zweite nach Othello unterstützt, mit eigenem Profil in der Gothic-2-DLL. Der vollständige automatische Testlauf läuft mit einem Spielstand der Mod durch. Nötig waren dafür unter anderem: der COOP-Eintrag im Hauptmenü von Mods, die Gothics Menüraster verwenden (u170); Hooks des Coops, die hinter denen von LeGo greifen, auch im Weltprozess (u170, u190); getrennte Werte pro PC für Skriptbibliotheken wie Ikarus, LeGo und Gothic Free Aim (u189, u195); die Behebung eines Absturzes beim Laden samt Reparatur beschädigter Spielstände (u196); eigene Übung, Talente, Rezepte und Erfolge für jeden Helden, auch beim Verbinden (u197).
- **Mod-Profile** (u187, u194, u197): Mods, die mit `-game:<ini>` starten, erkennt der Coop jetzt grundsätzlich. Was eine Mod braucht, steht in einer Profildatei `system\GothicCoopMod-<Ini>.json` statt im Code: Werte pro PC (`perPcVars`), Werte, die mit dem eigenen Helden reisen (`heroVars`), und Vorgaben für Coop-Schalter (`configDefaults`). Eine Vorlage mit Erklärungen liegt jedem Paket bei.
- **Ereignisse an NPCs des Hosts** (u195): Teleport, Tod durch ein Skript und Entfernen eines NPCs, die ein Dialog oder eine Skriptschleife beim Client auslöst, gelten jetzt auch beim Host (sofern beide in derselben Welt sind). Im automatischen Testlauf geprüft.
- **Inventare von NPCs aus Gesprächen** (u163): Was ein Gespräch einem NPC ins Inventar legt oder herausnimmt (etwa die Händlerware beim Kapitelwechsel in Gothic 1), bekommt auch der Partner. In Spielständen, in denen der Client schon ein Kapitel gewechselt hat, kommt die Ware nicht nachträglich dazu.
- **Bewegliche Teile, die selbst auf Berührung reagieren** (u167): Die Zielsteine und ein Tor im Schläfertempel (Gothic 1) und Vinos Kellertür (Gothic 2) bewegen sich jetzt auch beim Partner. Im automatischen Testlauf geprüft, im normalen Spiel noch nicht bestätigt.

**Korrekturen**

- **NPCs beim Client** (u179, u188, u193): NPCs, die der Host steuert, reagieren beim Client wieder, nachdem dort eine ihrer Reaktionen unterdrückt wurde – bisher nahmen sie danach keine Gespräche mehr an, und Torwachen übersahen den Helden. Feindlich werden sie beim Client nur noch, wenn der eigene Held angreift oder angegriffen wird, nicht schon, wenn sie etwas anderes beobachten.
- **K.o. beim Trinken** (u161): Der Held wacht wieder auf; bisher blieb er liegen (gesehen in Gothic 1).
- **Treffer** (u165): Zwei Treffer im selben Bild (etwa bei einem Sturz) spielt der Partner nicht mehr mit zu hohem Schaden nach.
- **Laden und Übernahme** (u168, u178): Der Held des Clients wird nicht mehr an eine Kante gesetzt, von der er abstürzt. Ein Fehler mitten im Laden beendet Gothic, statt es mit halb geladener Welt stehen zu lassen.
- **Haltungen pro Held** (u162): Die Haltung von NPCs zum Helden des Hosts, die aus Gilden-Quests stammt, kommt nicht mehr mit dem vollständigen Abgleich nach dem Laden zum Client.
- **NPC mit ungültiger Position** (u164): Ein NPC mit ungültiger Position (NaN, etwa nachdem ein Kampf an den Client übergeben wurde) wird an seinen Platz zurückgesetzt, auch in bestehenden Spielständen.
- **Kapitelbildschirm** (u182): Der Kapitelbildschirm des Partners erscheint sofort, auch mitten im eigenen Gespräch.
- **Gothic 2** (u195): Die Ausbaustufe der Klaue Beliars und der Menüzustand an Herstellungsplätzen bleiben beim eigenen Helden.
- **Uhr** (u186): Fristen und Wartezeiten des Coops hängen nicht mehr an der Uhrzeit des PCs; sprang die Uhr zurück, verschoben sich bisher alle um eine Stunde.
- **Abstürze:** Pfeilschuss ohne Fernkampfwaffe in der Hand (u173, u177); Aufheben eines schon gelöschten Gegenstands (u186, Gothic 1); Tod der Figur des Clients beim Host (u186, Gothic 1, abgesichert); der Schutz für Gegenstände in NPC-Slots übersah Speicher, der schon neu belegt war (u166).

## 0.5.0 – Trigger und Gegenstände der NPCs gemeinsam, eigener Held beim Laden, Gildenfortschritt pro Held (Gothic I u160, Gothic II u160, Othello u160)

**Neu**

- **Trigger** (u146, u158): Trigger, die ein Pfeil oder Zauber trifft oder die ein Held oder NPC betritt, lösen jetzt auch beim Partner aus – etwa die Brücke auf Irdorath, das Haupttor des Alten Lagers, Fallen und Türen in der Krypta und im Adanos-Tempel, in Gothic 1 die Bodenplatten im Schläfertempel. Bisher lief die Kette nur auf dem PC, auf dem sie ausgelöst wurde. Im automatischen Testlauf geprüft, im normalen Spiel noch nicht bestätigt.
- **Gegenstände der NPCs** (u156–u158): Was NPCs fallen lassen oder werfen (Waffen bei Tod oder Bewusstlosigkeit, Werkzeuge), liegt bei beiden an derselben Stelle. Bisher fiel es auf jedem PC für sich, unterschiedlich weit oder nur auf einer Seite. Pfeile und Bolzen bleiben pro PC getrennt: Sie stecken im Ziel oder verschwinden nach Sekunden.
- **Gildenfortschritt pro Held, auch beim Laden** (u150, u152, u154, u160): Werte, Gespräche, Tagebuchthemen und Haltungen von NPCs aus Gilden-Quests und Beitritten bleiben beim eigenen Helden, auch nach gemeinsamem Laden und auch dann, wenn der Partner nie dabei war. Bisher bekam der Partner beim Laden den Gildenfortschritt des Hosts und kam danach unter Umständen nicht mehr in seine Gilde. Spielstände von vor u150 enthalten diese Angaben nicht.
- **Spielende** (u147, nur Gothic 2): Nach dem Abspann landen beide im Hauptmenü, nicht nur derjenige, der das letzte Gespräch geführt hat.

**Korrekturen**

- **Eigener Held beim Laden** (u159): Wer selbst einen Spielstand lädt, in dem die Welt des Partners liegt (etwa nach einer Übernahme oder beim Neustart), bekommt jetzt seinen eigenen Helden statt einer Kopie des Partner-Helden.
- **Hebel** (u145, u147): Ziehen beide gleichzeitig am selben Hebel, löst er nur einmal aus; bisher setzten sich Reihenfolge-Rätsel zurück, und Tore gingen auf und gleich wieder zu. Ein Schloss an einem beweglichen Teil (die letzte Tür auf Irdorath) wird jetzt auch beim Partner gefunden.
- **NPC-Namen und Tote** (u150–u152): Gleichnamige NPCs, etwa mehrere Goblins einer Gruppe, wurden nach dem Laden auf den beiden PCs vertauscht; ein Tod traf dann beim Partner das falsche Tier. Falsche Tote aus älteren Spielständen und Leichen, die beim Host noch leben, stehen wieder auf.
- **Verwandlung** (u149): Während einer Verwandlung oder Beherrschung gibt es keine gemeinsame Sicherung mehr (bisher wurde das Tier zum Helden des Spielstands), und der Abgleich der Missionsgegenstände nimmt dem Partner nichts mehr weg.
- **Figur des Partners** (u144): Sie fällt nicht mehr endlos, wenn unter ihr kein Boden ist.
- **Missionsgegenstände** (u160, vor allem Gothic 1): Ein Gegenstand, dessen Infozeilen alle belegt sind, wird nicht mehr zweimal pro Sekunde neu gekennzeichnet.
- **Abstürze:** Laden mitten im Gespräch (u155); Zauberwechsel eines NPCs (u146); Weltprozess im ersten Bild nach einem Weltwechsel, jetzt allgemein abgefangen (u148); Gothic-1-Client beim Laden mit eigenen Gildenthemen (u153).

## 0.4.1 – Türen gemeinsam (Gothic I u143, Gothic II u143, Othello u143)

- **Türen** (u143): Öffnen, Schließen und Aufschließen durch einen Helden gelten jetzt für beide. 0.4.0 führte Türen schon auf, tatsächlich griff der Abgleich dort aber nie: Die Engine ruft bei Türen die Funktion, an der er hing, gar nicht auf. NPCs benutzen Türen weiterhin auf jedem PC für sich; ob eine Tür offen oder zu ist, kann also abweichen, wenn ein NPC hindurchgeht.

## 0.4.0 – Mechanismen gemeinsam, Weltwechsel ohne doppelte NPCs, Drachen auf beiden PCs (Gothic I u142, Gothic II u142, Othello u142; nur Tag `v0.4.0`, kein eigenes Release)

**Neu**

- **Mechanismen gemeinsam** (u137): Hebel, Steinplatten und Portale wirken jetzt für beide (Türen erst ab 0.4.1), samt den Skripten und Kamerafahrten dahinter. Bisher blieb zum Beispiel ein Portal beim Partner zu; nach dem Laden war es für beide wieder zu, der eingesetzte Gegenstand aber trotzdem weg. Feuerstellen bleiben pro PC getrennt (u138).

**Korrekturen**

- **Weltwechsel** (u140, u141, Gothic 2): Wer eine Welt als Zweiter betrat, bekam die Weltdatei des Partners – und trotzdem lief das Startskript der Welt noch einmal. Im Minental standen dann NPCs doppelt, und die doppelten wurden nicht abgeglichen. Die Flags kommen jetzt mit der Weltdatei, und was das Startskript setzt, bekommt auch der Partner. Spielstände, in denen schon doppelte NPCs stehen, bleiben so.
- **Drachen** (u139, u140): Das Gespräch macht einen Drachen sterblich, das galt aber nur auf dem PC des Sprechenden. Schutz, Gilde und Typ der NPCs werden jetzt abgeglichen. Spielstände, in denen ein Drache dadurch unsterblich blieb, werden beim Laden repariert.
- **Abstürze:** Speichern bei offenem Zuschau-Fenster ohne `AtmosphericScattering` (u135, Gothic 2, Renderer coop9); Kastell im Minental, Raum mit Gilde -1 (u136, Gothic 2); Laden eines Spielstands, der während einer Kamerafahrt entstanden war (u138); ein Gegenstand, den eine laufende Nachricht noch brauchte, war schon gelöscht, vor allem beim Client in Gothic 1 und Othello (u142, abgefangen); Kamera des Weltprozesses im ersten Bild nach einem Weltwechsel (u142).

## 0.3.1 – Gothic 1 und Othello automatisch geprüft, Korrekturen am Weltprozess (Gothic I u134, Gothic II u134, Othello u134)

Erstmals läuft der automatische Testlauf (Host und Client auf einem PC) auch in Gothic 1 und Othello; die Ergebnisse stehen im [README](README.de.md#teststand). Gefunden und behoben wurde dabei:

- **Weltprozess** (allgemein, u134): Stand der Partner in der anderen Welt still, bekam der Weltprozess des Hosts keinen Auftrag, und das Zuschau-Fenster blieb leer. Schon eine kurze Pause im Netzverkehr setzte die Wartezeit zurück.
- **Othello** (u134): Weltprozess und Testinstanz legen ihre Spielstände im selben Ordner ab wie das Hauptspiel (`SAVES_OTHELLOZ`, bisher fest `Saves`). Der Testlauf prüft getrennte Welten jetzt auch unter Othello.
- **Gothic 1, Othello** (u134): Das Flag „Mine schon betreten“ gehört zum Weltstand des jeweiligen PCs und wird nicht mehr abgeglichen.

## 0.3.0 – Coop-Inventar, gemeinsame Missionsgegenstände, Gilden pro Held (Gothic I u106, Gothic II u106, Othello u106; nur Tag `v0.3.0`, kein eigenes Release)

**Neu**

- **Coop-Inventar** (u97–u103): eine gemeinsame Truhe. Die Taste (standardmäßig I, Einstellungen › COOP) öffnet sie direkt, ohne dass sich der Held bewegen muss; beide können sie gleichzeitig offen haben. Der Inhalt liegt beim Host und im Spielstand, Gegenstände daraus zählen bei Quests.
- **Gemeinsame Missionsgegenstände** (u96, u104, u106): Bekommt einer einen, hat ihn der Partner auch; gibt einer ihn ab, verliert ihn auch der Partner. Im Namen steht „(gemeinsam)“, die Beschreibung erklärt es.
- **Gilden pro Held** (u85): Gilden-Dialoge teilen nur Helden derselben Gilde (samt Belohnung); das Aufnahmegespräch gilt beim Partner nie als schon geführt.
- **Auto-Sicherung** (u105): jetzt auch alle 3 Minuten.
- **Weltwechsel unter Othello** (u64, nur Othello): Die Änderungen des Othello-Coops v107–v114 sind übernommen – der Coop lädt die neue Welt selbst, wenn Union den Wechsel nicht meldet, und drei Menü-Hooks sind unter Othello abgeschaltet.

**Korrekturen** (allgemein)

- **Abstürze und Hänger:** Absturz beim Beenden über das Esc-Menü (u79); gelöschte Gegenstände in NPC-Slots und beim Ablegen (u88–u90); gelöschtes Schwert in der Hand nach der Übernahme eines Spielstands (u91, u93); ein hängender Dialog blockiert das Laden nicht mehr (u100); nach dem Tod an der Weltgrenze von Gothic 2 lädt der Coop über das Hauptmenü, statt endlos zu warten (u91).
- **Spieluhr:** Der Host schickt die Uhrzeit alle 10 statt alle 60 Sekunden (u69); die Uhr des Clients und des Weltprozesses läuft sekundengenau mit der des Hosts, und Tagesabläufe stellen sich bei großen Zeitsprüngen neu ein (u83).
- **NPCs beim Client:** Kleine Abweichungen werden weich ausgeglichen statt mit einem Sprung, große Abstände sauber neu gesetzt, damit niemand hängen bleibt (u72, u74).
- **Gegenstände und Truhen:** Verspätete Löschungen werden nachgeholt, sodass bis zum nächsten Laden keine Geistergegenstände mehr herumliegen (u70); Truhen in der Welt geben Gegenstände sauber heraus (u98).
- **Story-Videos:** Das Überspringen für beide klappt auch, wenn die interne Nummer des Videos zehnstellig ist (u87).

## 0.2.0 – Erste Veröffentlichung als Gothic-I-II-Universal-Coop (Gothic I u62, Gothic II u62, Othello u62)

- Coop für Gothic 1 (1.08k), Gothic 2 Die Nacht des Raben (2.6) und Gothic 1 mit Othello 4.x aus einem gemeinsamen Quelltext:
  Spieler und NPCs; Kämpfe, in denen der eigene PC über Treffer auf den eigenen Helden entscheidet und der PC des Clients die
  NPCs berechnet, die gegen ihn kämpfen; Dialoge, Story und Tagebuch (Quest-EP für beide); Truhen, Gegenstände und Plündern;
  gemeinsame Spielstände mit Vorschau und Sicherung beim Partner; getrennte Welten mit Weltprozess und Zuschau-Fenster;
  gemeinsames neues Spiel; Story-Videos, die sich für beide überspringen lassen; COOP-Menü mit Figur und 3D-Vorschau; Coop über
  Steam; COOP-Schwierigkeit; Teleport-Rune zum Mitspieler; Coop-Testlauf (Shift+F11).
- u62: Union 1.0m liegt als Dateien bei (im Installer und in den ZIPs für Gothic 1 und 2). Der Installer installiert Union mit,
  wenn es fehlt, verschiebt `system\dinput.dll` von SystemPack nach `CoopBackup` und stellt beim Rückgängigmachen beides wieder
  her (bis u61 startete er das Union-Setup). Neu ist eine ZIP für Othello.
- u56–u61: Der Coop selbst ist gegenüber u55 unverändert.

## Vorgeschichte

### Gothic Union Coop u1–u61 (unveröffentlicht)

Aus dem Othello-Coop entstand ein Union-Plugin, das Gothic 1, Gothic 2 Die Nacht des Raben und Othello mit einem gemeinsamen
Quelltext abdeckt. Die wichtigsten Schritte:

- **u1–u7 – ein Kern für alle Engines:** Der Kern des Coops ist unabhängig von der Engine. Alle Adressen in der Engine stehen in
  einer Tabelle, und jeder Teil des Coops wird für jede Engine einzeln freigeschaltet. Der Coop lässt sich jetzt auch für
  Gothic 2 bauen, und in Gothic 1 läuft er auch ohne Othello (mit eigenem Hauptmenü und eigenem Laden). Die Neuerungen des
  Othello-Coops bis v110 sind übernommen, dazu kommt ein automatischer Coop-Testlauf.
- **u8–u20 – Gothic 2 Die Nacht des Raben:** COOP-Menü mit Mausbedienung, gemeinsames neues Spiel mit Intro für beide,
  Coop-Renderer mit 3D-Vorschau der Figur und Zuschau-Fenster, Gespräche (einer spricht, der andere hört zu – Fortschritt,
  Tagebuch und Erfahrung gelten für beide), Abgleich der Welt, gemeinsame Spielstände mit Vorschau, das Kampfsystem für zwei PCs
  (wie in 0.2.0 beschrieben), COOP-Schwierigkeit, Teleport-Rune, Markierung des Partners und eine Selbstprüfung, ob alle
  Adressen zur Engine passen.
- **u21–u28 – Bedienung:** Esc-Menü mit „Hauptmenü“ und „Spielstand wählen“, Tagebuch mit Mausbedienung, ein Mauszeiger, das
  Kapitelbild der eigenen Gilde, Story-Videos auch beim Partner und der Partner auf Gothics Karte.
- **u29–u40 – getrennte Welten:** Jeder Spieler kann in seiner eigenen Welt sein (Khorinis, Minental, Jharkendar, Irdorath);
  ein gemeinsamer Weltwechsel lässt sich einstellen. Der Weltprozess – ein zweites Gothic im Hintergrund – zeigt die Welt des
  Partners im Zuschau-Fenster. Dazu kommen der Teleport zwischen den Welten und der Austausch der Weltstände, wenn in
  verschiedenen Welten gespeichert wird.
- **u41–u55 – stabil und installierbar:** Absturzkorrekturen aus Testläufen, ein automatischer Testlauf mit Host und Client auf
  einem PC, der Installer (erkennt das Spiel am Ordnerinhalt, sichert alles, „Letzte Installation rückgängig“), die Sicherung
  beim Partner (jeder bekommt auch den Spielstand des anderen) und Story-Videos, die sich für beide überspringen lassen.

### Othello-Coop 0.1.0–0.1.2 (nur Gothic 1 mit Othello 4.4.2.1, Alpha)

- **0.1.2:** Teleport-Rune zum Mitspieler (kostet kein Mana, eigenes Aussehen). Markierung des Mitspielers: Name über dem Kopf
  (hinter Wänden blass, mit Entfernung), Pfeil am Bildrand, beide Spieler als Pfeile auf Othellos Weltkarte. Behoben: Sprünge,
  wenn NPCs ihr Ziel zwischen den Spielern wechseln; Fehler in Kämpfen von NPCs, die dem Host feindlich gesinnt sind; Abstürze
  beim Seitenschritt im Kampf und bei einer Zwischensequenz kurz nach dem Laden. Beim Laden wird der Zustand der alten Welt
  verworfen.
- **0.1.1:** Speicherleck beim Host behoben (der Speicherverbrauch wuchs mit der Spielzeit und jedem Abgleich); die
  Absturzmeldung stürzt nicht mehr selbst ab; stabilere Warteschlange zwischen Netzwerk und Spiel; kein Absturz mehr beim
  Zauberwechsel des Mitspielers.
- **0.1.0:** erste öffentliche Fassung – Coop für Gothic 1 mit Othello: COOP-Menü mit Mitspieler-Figur, Renderer mit
  Zuschau-Fenster und Figurvorschau, Verbinden über IP oder Steam.
