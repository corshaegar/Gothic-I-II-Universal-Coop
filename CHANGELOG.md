# Änderungen

Projektversion (Veröffentlichung) und je Spiel/Mod die Coop-Fassung `uNN`. Vorgänger: das Othello-Coop
(github.com/corshaegar/Othello-Coop, 0.1.0–0.1.2, nur Othello). Die Fassungen dazwischen (u1–u61) stehen im Entwicklungsverlauf;
hier beginnt der Verlauf mit u62.

## 0.7.0 – Legend of Ahssun vollständig analysiert, Begleiter und Beschwörungen des Clients, Skript-Ereignisse beim Client sofort (Gothic I u199, Gothic II u199, Othello u199, Legend of Ahssun u199)

Die Änderungen aus der Analyse von Legend of Ahssun sind am Code der Mod geprüft und, wo es dabei steht, im automatischen Testlauf; im echten Spielablauf hat sie noch niemand gesehen.

**Neu**

- **Legend of Ahssun vollständig gelesen** (u198, u199): Alle 3.914 eigenen Funktionen der Mod wurden vor dem nächsten Test gelesen (Muster im Skript und ein lokales Sprachmodell) und jeder schwere Befund einzeln am Code geprüft. Daraus kommen die Änderungen unten; der Ablauf gilt jetzt für jede neue Mod.
- **Begleiter und Beschwörungen des Clients** (u199): folgen jetzt dem Client. Bisher liefen sie immer zum Host, weil die KI der NPCs beim Host läuft (Gothic 2: Beschwörungen; Legend of Ahssun: angeworbene Begleiter). Log „Begleiter … gehoert zu FRIEND_n“.
- **Spawns aus Skripten des Clients** (u199): `Wld_SpawnNpcRange` (Beschwörungen, Gräber, Fallen, Boss-Angriffe) gilt jetzt bei beiden; bisher entstanden diese NPCs nur beim Client.
- **Zauber des Partners** (u199): Das Nachspielen eines Zaubers beim Partner hat keine Skript-Wirkung mehr. Bisher erzeugte eine Beschwörung des Partners bei einem selbst die feindliche Variante (Gothic 2), und Runen mit Dauerwirkung wirkten auch beim Partner.
- **Mod-Profile: Zähler** (u199): neuer Eintrag `deltaVars` – Zähler, die nur Spieler-Skripte hochzählen, gleicht der Coop als Änderung ab; zwei gleichzeitige Schritte zählen beide.
- **Mod-Profile: „der Spieler dort“** (u199): neuer Eintrag `heroStates` – in den gelisteten KI-Zuständen meint `HERO` den Spieler, gegen den der NPC kämpft, sonst den nächsten Spieler. Legend of Ahssun: Die Beschwörung im Sumpf, die Faustkampf-Arena, Kastor/Parcival und die Magus-Illusionen wirken jetzt um den Spieler, der dort ist; bisher immer um den Host (die Arena nahm sogar dem Host die Waffe ab, wenn der Client kämpfte). Der Mechanismus ist im automatischen Testlauf geprüft.
- **Beitritt nur mit demselben Spiel** (u198): Engine und Mod gehen beim Verbinden mit; ein Client mit einem anderen Spiel trennt, bevor er einen Stand lädt (bisher stürzte etwa ein Gothic-2-Client an einem Gothic-1-Host beim Laden ab).

**Korrekturen**

- **Abstürze beim Client** (u199): Skripte, die einen eben eingefügten NPC oder Gegenstand sofort weiterbenutzen, griffen beim Client ins Leere (Legend of Ahssun: Kapitel 5 beim Gespräch mit Argos, Tor im Tempel des Erwachens, Arena Runde 9). Der Client legt das Objekt jetzt sofort selbst an, die Meldung des Hosts übernimmt es (im automatischen Testlauf in Gothic 2 und Legend of Ahssun geprüft).
- **Legend of Ahssun** (u199): Geben und Verkaufen von Gegenständen, Schleifstein und Runen zerlegen starteten beim Partner mit (sein ausgewählter Gegenstand war weg, die Bewegungssperre blieb hängen); der Schrein-Segen nahm dem Partner beim Ablauf Werte, die er nie bekam; Fluch-Punkte, Rezepte, Lesepult-Boni, Charakterwerte und weitere Werte des eigenen Helden gehen nicht mehr zum Partner; gleichzeitig gelöschte Brände zählen beide (Quest „Löscharbeit“).
- **Gothic 2** (u199): Die Übung beim Goldhacken bleibt beim eigenen Helden.
- **Doppelte Ereignisse** (u199): Gleiche Einfügungen beider PCs binnen 3 s fasst der Host jetzt auch zusammen, wenn die Skripte den Wegpunkt je PC würfeln.
- **Testlauf** (u198, u199): Zwei Läufe, die kurz nacheinander starten, nehmen nicht mehr denselben Port; der Engine-Vergleich hängt nicht mehr an der Schreibweise der Modulnamen; der Diagnose-Schritt mit dem Apfel klappt in Gothic 1 und Othello.

## 0.6.0 – Ein Coop für alle Spiele und Mods, Legend of Ahssun, Mod-Profile (Gothic I u197, Gothic II u197, Othello u197, Legend of Ahssun u197)

**Neu**

- **Ein Coop für alle Spiele und Mods** (u195, u197): Der Installer fragt nicht mehr nach dem Spiel. Er erkennt am Inhalt des Ordners Gothic 1, Othello, Gothic 2 und die Mods darin und zeigt schon vor dem Installieren, welche unterstützt sind. Installiert wird der Coop der Engine mit den Skripten aller Varianten; wechselt die Mod später (GothicStarter, Othello dazu oder weg), stellt sich der Coop beim Start selbst um (Log „Skript-Weiche“). Statt einer ZIP je Spiel gibt es eine Universal-ZIP mit einem Ordner je Engine.
- **Legend of Ahssun** (Gothic 2, u170–u197): als zweite Mod nach Othello unterstützt, mit eigenem Profil in der Gothic-2-DLL. Der volle automatische Testlauf läuft mit einem Spielstand der Mod durch. Dafür unter anderem: COOP im Hauptmenü von Mods mit Gothics Menüraster (u170); Haken des Coops hinter LeGo-Haken, auch im Weltprozess (u170, u190); Werte von Skript-Bibliotheken (Ikarus, LeGo, Gothic Free Aim) bleiben je PC (u189, u195); Absturz beim Laden behoben, beschädigte Spielstände werden beim Laden repariert (u196); Übung, Talente, Rezepte und Erfolge bleiben beim Verbinden die des eigenen Helden (u197).
- **Mod-Profile** (u187, u194, u197): Mods, die mit `-game:<ini>` starten, erkennt der Coop allgemein. Was eine Mod braucht, steht in einer Profil-Datei `system\GothicCoopMod-<Ini>.json` statt im Code: Werte je PC (`perPcVars`), Werte, die mit dem eigenen Helden reisen (`heroVars`), Vorgaben für Coop-Schalter (`configDefaults`). Eine Vorlage mit Erklärung liegt jedem Paket bei.
- **Ereignisse an NPCs des Hosts** (u195): Teleport, Skript-Tod und Entfernen eines NPCs, die ein Dialog oder eine Schleife beim Client auslöst, gelten jetzt auch beim Host (nur mit dem Host in derselben Welt). Im automatischen Testlauf geprüft.
- **Inventare von NPCs aus Gesprächen** (u163): Was ein Gespräch einem NPC ins Inventar legt oder herausnimmt (etwa die Händlerware beim Kapitelwechsel in Gothic 1), bekommt auch der Partner. Spielstände, in denen der Client schon ein Kapitel wechselte, bekommen die Ware nicht nachträglich.
- **Bewegliche Teile, die selbst auf Berührung reagieren** (u167): die Zielsteine und ein Tor im Schläfertempel (Gothic 1) und Vinos Kellertür (Gothic 2) bewegen sich jetzt auch beim Partner. Im automatischen Testlauf geprüft, im echten Spielablauf noch nicht gesehen.

**Korrekturen**

- **NPCs beim Client** (u179, u188, u193): werden nach einer gesperrten Reaktion nicht mehr taub (Gespräche, Torwachen) und werden nur noch feindlich, wenn der eigene Held angreift oder angegriffen wird, nicht schon, wenn sie etwas anderes beobachten.
- **K.o. beim Trinken** (u161): Der Held wacht wieder auf; bisher blieb er liegen (gesehen in Gothic 1).
- **Treffer** (u165): Zwei Treffer im selben Bild (etwa ein Sturz) spielt der Partner nicht mehr mit zu viel Schaden nach.
- **Laden und Übernahme** (u168, u178): Der Held des Clients wird nicht mehr an eine Kante gesetzt, von der er abstürzt. Ein Fehler mitten im Laden beendet Gothic, statt es mit halb geladener Welt stehen zu lassen.
- **Haltungen je Held** (u162): Haltungen aus Gilden-Quests des Host-Helden kommen nicht mehr über den Gesamtstand zum Client.
- **NPC mit ungültiger Lage** (u164): Ein NPC, dessen Lage ungültig wurde (NaN, nach einer Kampf-Übergabe), wird an seinen Ort gesetzt, auch in bestehenden Spielständen.
- **Kapitel-Bildschirm** (u182): Der Kapitel-Bildschirm des Partners erscheint sofort, auch mitten im eigenen Gespräch.
- **Gothic 2** (u195): Ausbaustufe der Klaue Beliars und der Menüstand der Herstell-Stellen bleiben beim eigenen Helden.
- **Uhr** (u186): Fristen hängen nicht mehr an der Uhrzeit des PCs; ein Zurückstellen der Uhr verschob bisher alle um eine Stunde.
- **Abstürze:** Pfeilschuss ohne Fernkampfwaffe in der Hand (u173, u177); Aufheben eines schon freigegebenen Gegenstands (u186, Gothic 1); Tod der Figur des Clients beim Host (u186, Gothic 1, Schutz); neu belegter Speicher im Slot-Schutz (u166).

## 0.5.0 – Auslöser und Gegenstände der NPCs gemeinsam, eigener Held beim Laden, Gildenfortschritt je Held (Gothic I u160, Gothic II u160, Othello u160)

**Neu**

- **Auslöser** (u146, u158): Auslöser, die ein Pfeil oder Zauber trifft oder die ein Held oder ein NPC betritt, lösen jetzt auch beim Partner aus – etwa die Brücke auf Irdorath, das Haupttor des Alten Lagers, Fallen und Türen in der Krypta und im Adanos-Tempel, in Gothic 1 die Bodenplatten im Schläfertempel. Bisher lief die Kette nur auf dem PC, auf dem sie ausgelöst wurde. Im automatischen Testlauf geprüft, im echten Spielablauf noch nicht gesehen.
- **Gegenstände der NPCs** (u156–u158): Was NPCs fallen lassen oder werfen (Waffen bei Tod oder Bewusstlosigkeit, Werkzeuge), liegt bei beiden an derselben Stelle. Bisher fiel es auf jedem PC für sich, verschieden weit oder nur auf einer Seite. Pfeile und Bolzen bleiben je PC: Sie stecken im Ziel oder verschwinden nach Sekunden.
- **Gildenfortschritt je Held, auch beim Laden** (u150, u152, u154, u160): Werte, Gespräche, Tagebuch-Themen und Haltungen von NPCs aus Gilden-Quests und Beitritten bleiben beim eigenen Helden, auch nach gemeinsamem Laden und auch, wenn der Partner nie dabei war. Bisher bekam der Partner beim Laden den Gildenfortschritt des Hosts und kam danach unter Umständen nicht mehr in seine Gilde. Spielstände von vor u150 haben diese Angaben nicht.
- **Spielende** (u147, nur Gothic 2): Nach dem Abspann stehen beide im Hauptmenü, nicht nur der, der das letzte Gespräch geführt hat.

**Korrekturen**

- **Eigener Held beim Laden** (u159): Wer selbst einen Spielstand lädt, in dem die Welt des Partners liegt (etwa nach einer Übernahme, beim Neustart), bekommt jetzt seinen eigenen Helden statt einer Kopie des Partner-Helden.
- **Hebel** (u145, u147): Ziehen beide zugleich am selben Hebel, löst er nur einmal aus; bisher setzten sich Reihenfolge-Rätsel zurück, und Tore gingen auf und gleich wieder zu. Ein Schloss auf einem beweglichen Teil (die letzte Tür auf Irdorath) wird beim Partner gefunden.
- **NPC-Namen und Tote** (u150–u152): Gleichnamige NPCs, etwa mehrere Goblins einer Gruppe, wurden nach dem Laden auf den beiden PCs vertauscht; ein Tod traf dann beim Partner das falsche Tier. Falsche Tote aus älteren Spielständen und Leichen, die beim Host leben, stehen wieder auf.
- **Verwandlung** (u149): Während einer Verwandlung oder Beherrschung gibt es keine gemeinsame Sicherung mehr (bisher wurde das Tier zum Helden des Spielstands), und der Abgleich der Missions-Gegenstände nimmt dem Partner nichts weg.
- **Figur des Partners** (u144): fällt nicht mehr endlos, wenn unter ihr kein Boden ist.
- **Missions-Gegenstände** (u160, vor allem Gothic 1): Ein Gegenstand, dessen Infozeilen alle belegt sind, wurde zweimal je Sekunde neu gekennzeichnet.
- **Abstürze:** Laden mitten im Gespräch (u155); Zauberwechsel eines NPCs (u146); Weltprozess im ersten Bild nach einem Weltwechsel, jetzt allgemein abgefangen (u148); Gothic-1-Client beim Laden mit eigenen Gilden-Themen (u153).

## 0.4.1 – Türen gemeinsam (Gothic I u143, Gothic II u143, Othello u143)

- **Türen** (u143): Öffnen, Schließen und Aufschließen durch einen Helden gelten jetzt für beide. 0.4.0 nannte Türen schon, tatsächlich kam der Abgleich dort nie an: Die Engine ruft bei Türen die Funktion nicht auf, an der er hing. NPCs benutzen Türen weiter je PC, offen oder zu kann also abweichen, wenn ein NPC durchgeht.

## 0.4.0 – Mechanismen gemeinsam, Weltwechsel ohne doppelte NPCs, Drachen auf beiden PCs (Gothic I u142, Gothic II u142, Othello u142; nur Tag `v0.4.0`, kein eigenes Release)

**Neu**

- **Mechanismen gemeinsam** (u137): Hebel, Steinplatten und Portale gelten jetzt für beide (Türen erst ab 0.4.1), samt den Skripten und Kamerafahrten dahinter. Bisher blieb zum Beispiel ein Portal beim Partner zu; nach dem Laden war es für beide wieder zu und der eingesetzte Gegenstand trotzdem weg. Feuerstellen bleiben je PC (u138).

**Korrekturen**

- **Weltwechsel** (u140, u141, Gothic 2): Wer eine Welt als Zweiter betrat, holte die Weltdatei des Partners, und das Einzugs-Skript lief trotzdem noch einmal. Im Minental standen dann NPCs doppelt, und die Doppelten liefen nicht im Abgleich. Die Merker kommen jetzt mit der Weltdatei, und was der Einzug setzt, bekommt auch der Partner. Spielstände, in denen schon doppelte NPCs stehen, bleiben so.
- **Drachen** (u139, u140): Das Gespräch macht einen Drachen sterblich, das galt aber nur auf dem PC des Sprechenden. Schutz, Gilde und Typ der NPCs laufen jetzt mit. Spielstände, in denen ein Drache dadurch unsterblich blieb, heilen beim Laden.
- **Abstürze:** Speichern bei offenem Zuschau-Fenster ohne `AtmosphericScattering` (u135, Gothic 2, Renderer coop9); Kastell im Minental, Raum mit Gilde -1 (u136, Gothic 2); Laden eines Spielstands, der während einer Kamerafahrt entstand (u138); Gegenstand einer laufenden Nachricht schon freigegeben, vor allem beim Client in Gothic 1 und Othello (u142, abgefangen); Kamera des Weltprozesses im ersten Bild nach einem Weltwechsel (u142).

## 0.3.1 – Gothic 1 und Othello automatisch geprüft, Korrekturen am Weltprozess (Gothic I u134, Gothic II u134, Othello u134)

Erstmals läuft der automatische Testlauf (Host und Client auf einem PC) auch in Gothic 1 und Othello: Ergebnisse im [README](README.de.md#teststand). Was er dabei gefunden hat:

- **Weltprozess** (allgemein, u134): Stand der Partner in der anderen Welt still, bekam der Weltprozess des Hosts keinen Auftrag, und das Zuschau-Fenster blieb leer. Schon eine kurze Paketpause setzte die Wartezeit zurück.
- **Othello** (u134): Weltprozess und Testinstanz legen den Spielstand im selben Ordner ab wie das Hauptspiel (`SAVES_OTHELLOZ`, bisher fest `Saves`). Der Testlauf prüft getrennte Welten jetzt auch unter Othello.
- **Gothic 1, Othello** (u134): Der Merker „Mine schon betreten“ gehört zum Weltstand des PCs und wird nicht mehr abgeglichen.

## 0.3.0 – Coop-Inventar, gemeinsame Missions-Gegenstände, Gilden je Held (Gothic I u106, Gothic II u106, Othello u106; nur Tag `v0.3.0`, kein eigenes Release)

**Neu**

- **Coop-Inventar** (u97–u103): eine gemeinsame Truhe. Die Taste (Vorgabe I, Einstellungen › COOP) öffnet sie direkt, ohne Bewegung;
  beide können sie gleichzeitig offen haben. Der Inhalt liegt beim Host und im Spielstand, Gegenstände daraus zählen bei Quests.
- **Gemeinsame Missions-Gegenstände** (u96, u104, u106): bekommt einer einen, hat ihn der Partner auch; gibt einer ihn ab, verliert ihn
  der Partner. Im Namen steht „(gemeinsam)“, die Beschreibung erklärt es.
- **Gilden je Held** (u85): Gilden-Dialoge teilen nur Helden derselben Gilde (samt Belohnung); die Aufnahme gilt beim Partner nie als erzählt.
- **Auto-Sicherung** auch alle 3 Minuten (u105).
- **Weltwechsel unter Othello** (u64, nur Othello): Othello-Coop v107–v114 nachgezogen – GT lädt selbst, wenn Union den Wechsel nicht
  meldet; drei Menü-Haken unter Othello aus.

**Korrekturen** (allgemein)

- **Abstürze und Hänger:** Absturz beim Beenden über das Esc-Menü (u79); gelöschte Gegenstände in NPC-Slots und mitten beim Ablegen
  (u88–u90); gelöschtes Schwert in der Hand nach der Übernahme eines Spielstands (u91, u93); ein hängender Dialog blockiert das Laden nicht
  mehr (u100); nach dem Tod an der Weltgrenze von Gothic 2 lädt der Coop über das Hauptmenü, statt ewig zu warten (u91).
- **Spieluhr:** der Host schickt die Uhrzeit alle 10 statt 60 s (u69); die Uhr des Clients und des Weltprozesses geht jede Sekunde genau wie
  beim Host, Tagesabläufe stellen sich bei großen Sprüngen neu ein (u83).
- **NPCs beim Client:** kleine Abweichungen weich nachziehen statt springen, weite Sprünge sauber setzen, damit niemand hängen bleibt (u72, u74).
- **Gegenstände und Truhen:** verspätete Entfernungen werden nachgeholt, keine Geistergegenstände bis zum nächsten Laden (u70); Welt-Truhen
  nehmen Gegenstände sauber heraus (u98).
- **Story-Videos:** Überspringen geht auch bei 10-stelligen Nummern (u87).

## 0.2.0 – erste Veröffentlichung als Gothic-I-II-Universal-Coop (Gothic I u62, Gothic II u62, Othello u62)

- Coop für Gothic 1 (1.08k), Gothic 2 Die Nacht des Raben (2.6) und Gothic 1 mit Othello 4.x aus einem gemeinsamen Quelltext:
  Spieler und NPCs, Kampf mit Treffer-Hoheit und Kampf-Übergabe, Dialoge, Story und Tagebuch (Quest-EP für beide),
  Truhen, Gegenstände und Plündern, gemeinsame Spielstände mit Vorschau und Sicherung beim Partner, getrennte Welten mit
  Weltprozess und Zuschau-Fenster, gemeinsames neues Spiel, Story-Videos für beide überspringen, COOP-Menü mit Figur und
  3D-Vorschau, Coop über Steam, COOP-Schwierigkeit, Teleport-Rune zum Mitspieler, Coop-Testlauf (Shift+F11).
- u62: Union 1.0m liegt als Dateien bei (Installer und ZIPs für Gothic 1/2); der Installer installiert Union mit, wenn es
  fehlt, legt `system\dinput.dll` von SystemPack nach `CoopBackup` und nimmt beides beim Rückgängig wieder heraus (bis u61
  startete er das Union-Setup). Neu: ZIP für Othello.
- u56–u61: am Coop unverändert gegenüber u55.
