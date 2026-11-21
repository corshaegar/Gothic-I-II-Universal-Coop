// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {

    // --- u97 (P119): gemeinsame Coop-Truhe im Inventar; u99 (P121): fuer beide gleichzeitig offen ---------------------------
    //
    // Nutzer (28.10.2026): "Koennen wir ein item erstellen, das wie eine Truhe funktioniert", "die Truhe soll auch oeffenbar im
    // Inventar sein und dass man sachen rueberziehen kann", "also ohne ablegen, einfach im inventar oeffnen"; Inhalt "Gemeinsam fuer
    // beide". Jeder Held traegt den Gegenstand ITMI_COOP_TRUHE (COOP_ITEMS.D; fehlt er, legt GT ihn ins Inventar). Benutzen (Schema
    // MAPSEALED wie Gothics Beutel - verbraucht die Engine ihn dabei, gibt GT gleich einen neuen) setzt nur den Merker
    // COOP_TRUHE_OEFFNEN (nie abgeglichen, DlgNeverSync); GT oeffnet dann das Truhenfenster einer eigenen oCMobContainer ausserhalb
    // der Welt, das Inventar daneben - Gegenstaende hin und her wie bei jeder Truhe.
    // Engine (Disassembly G2A/G1): Open (G2A 0x00726500, G1 0x006843D0) legt das Fenster an, das direkt auf containList arbeitet,
    // und oeffnet das Inventar; Close (G2A 0x00726640, G1 0x00684490) schliesst das Inventar und loescht das Fenster. Esc bzw. Tab
    // im Truhenfenster schliessen Fenster und Inventar (oCItemContainer::HandleEvent, G2A ab 0x0070AB21); die Truhen-Bedienung, die
    // danach Close riefe, gibt es hier nicht - das tut GT im naechsten Bild, auch wenn nur das Inventar zuging. Insert zaehlt den
    // Gegenstand mit (zCListSort::Insert, ohne Sortierung). Remove dagegen legt ihn ueber oCWorld::InsertVobInWorld in die Listen
    // der Welt - GT nimmt Gegenstaende darum selbst aus der Liste (u98: wie CoopChestDropItem). Den Welt-Truhenabgleich
    // (CoopWorldSync.cpp) laesst diese Truhe aus (CoopTruheObj); ihre Gegenstaende haben keine homeWorld, das Melden aufgehobener
    // Gegenstaende greift also nicht (CoopItemLiegtFrei).
    //
    // u99 (P121, Nutzer 29.10.: "denkst du wir schaffen es dass beide die truhe gleichzeitig offen haben", dann "Gleichzeitig
    // oeffnen"): keine Sperre mehr (bis u98 war sie beim Oeffnen fuer den anderen zu). Jede Aenderung hier - Fenster, Testlauf -
    // erkennt GT im naechsten Bild am Vergleich mit dem zuletzt bekannten Stand und meldet sie sofort: der Client als Zug (Nummer
    // je Epoche, "chg") an den Host, bis der ihn bestaetigt (alle 3 s neu gesendet); der Host wendet die Zuege der Reihe nach auf
    // seinen Inhalt an und verteilt den neuen Stand mit den Bestaetigungen ("state"). Der Client gleicht an: Stand des Hosts plus
    // seine noch unbestaetigten Zuege. Nahm ein Client etwas, das beim Host schon weg war (beide griffen zu, der andere kam zuerst
    // an), schickt der Host "undo": der Client nimmt es aus seinem Inventar wieder heraus und bekommt eine Meldung. Fremde
    // Aenderungen am offenen Fenster macht GT wie die Engine in oCItemContainer::Insert/Remove: in die Liste bzw. heraus, danach
    // CheckSelectedItem (G2A 0x00709660, G1 0x00669560), damit die Auswahl gueltig bleibt - ohne den Umweg ueber die Weltlisten.
    // Pakete ueber den Fortschritts-Kanal (SYNC_DIALOG_STATE, auch zwischen Welten). Gespeichert wird der Inhalt im Spielstand-
    // Ordner (COOPTRUHE.TXT - die Uebertragung eines Slots nimmt alle Dateien mit), geladen nur beim Laden eines Spielstands; ein
    // neues Spiel beginnt leer. Pakete waehrend des Ladens verwirft der Empfaenger - der Client fragt darum nach dem Laden, bis der
    // Stand des Hosts da ist. Allgemein (G1/G2).
    //
    // u101 (P123, Nutzer 29.10.: "vielleicht sollten wir es umbennen zu Coop inventar oder so. Außerdem darf man das Item nicht in
    // sich selbst reinlegen. Und zusätzlich möchte ich dass die "Truhe" sich direkt öffnet ohne Animation", dann "es soll auch einen
    // hotkey geben mit dem man es direkt öffnen kann einstellbar in coop einstellungen"): fuer den Spieler heisst sie jetzt
    // "Coop-Inventar" (Gegenstand, Fenster, Meldungen; Instanz und Dateien bleiben). Benutzen aus dem Inventar reiht fuer
    // Gegenstaende mit Schema EV_USEITEMTOSTATE ein (oCNpcInventory::HandleEvent, G2A 0x0070EFBF, G1 0x0066F31D) - ein Haken auf
    // oCNpc::EV_UseItemToState erledigt das fuer die Coop-Truhe sofort: keine Bewegung, nichts verbraucht, das Fenster geht auf.
    // Hinueberschieben laeuft ueber oCItemContainer::TransferItem (ausgewaehlten Gegenstand heraus, beim Nachbarfenster Insert) -
    // bei offenem Coop-Inventar nimmt GT die Coop-Truhe selbst dort nicht heraus. Taste coopInvKey (COOP-Einstellungen, Vorgabe I):
    // oeffnet bzw. schliesst es direkt, auch ohne den Gegenstand - nicht in Menues, im Gespraech, bei einem anderen offenen Fenster.
    //
    // u103 (P125, Nutzer 29.10.: "ergründe außerdem die möglichkeit dass Items aus dem Coop Inventar auch bei Quests erkannt werden",
    // dann "Ja, u103"): Dialoge und Skripte fragen den Besitz mit Npc_HasItems(npc, inst) ab (G2A 0x006E72E0, G1 0x00654E20:
    // inventory2.GetAmount, Summe aller Stapel) und nehmen ihn mit Npc_RemoveInvItems(npc, inst, n) (G2A 0x006E8200: hoechstens, was
    // da ist; G1 0x00655160: nur, wenn alle n da sind) bzw. Npc_RemoveInvItem(npc, inst) (G2A 0x006E7D90, G1 0x006550F0) -
    // B_GiveInvItems benutzt beide. Alle drei geben int zurueck (Entfernen: 1, wenn der Gegenstand da war). Fuer den eigenen Helden
    // leitet GT sie um: Npc_HasItems zaehlt das Coop-Inventar mit; Entfernen nimmt zuerst aus dem eigenen Inventar, den Rest aus dem
    // Coop-Inventar (gemeldet wie jede Aenderung hier, Meldung im Chat). Andere NPCs, andere Parser und das Laden gehen unveraendert
    // ans Original. Nicht betroffen: was die Engine selbst prueft (Schluessel an Tueren, Handel). Allgemein (G1/G2).

    // u275 (P346): Coop-Test - bis zu dieser Zeit gilt "Coop-Inventar nimmt neue Missions-Gegenstaende" (P324) als aus, damit die
    // Schritte "Missions-Gegenstand ..." die Spiegelung pruefen (CoopSelfTest.cpp, PMi setzt den Wert je Schritt neu)
    long long CoopTestMiOhneTruheBis = 0;

#if COOP_BUILD(Dialog) && COOP_BUILD(Save)

    typedef std::map<std::string, int> CoopTruheBestand;   // Instanz -> Menge (ohne die Coop-Truhe selbst)

    struct CoopTruheZug {      // Client: eigene Aenderung, vom Host noch nicht bestaetigt
        int seq;
        CoopTruheBestand d;
    };

    struct CoopTruheGast {     // Host: Stand je Client
        long long epoche = 0;
        int seq = 0;           // zuletzt angewandter Zug
        bool offen = false;
    };

    static const char* const TruheInst = "ITMI_COOP_TRUHE";
    static const char* const TruheMerker = "COOP_TRUHE_OEFFNEN";
    static CoopTruheBestand TruheBekannt;      // zuletzt bekannter Stand hier - was davon abweicht, hat der Spieler hier getan
    static bool TruheOffenHier = false;
    static bool TruheInvGesehen = false;       // das Inventar neben dem Fenster war offen
    static bool TruheWill = false;             // Merker gesehen - oeffnen, sobald der Held frei ist
    static long long TruheWillMs = 0;
    static long long TruheGebenMs = 0;
    static bool TruheGegebenGemeldet = false;
    static std::set<std::string> TruheAndereOffen;   // wer ausser mir sie offen hat (zuletzt gemeldet)
    // Client
    static std::vector<CoopTruheZug> TruheZuege;
    static int TruheSeq = 0;
    static long long TruheEpoche = 0;          // neu je Laden bzw. neuem Spiel - der Host zaehlt die Zuege je Epoche
    static long long TruheSendMs = 0;
    static int TruheSendungen = 0;             // Sendungen seit der letzten Bestaetigung
    static int TruheLadenFragen = 0;           // nach dem Laden: so oft noch nach dem Stand fragen
    static long long TruheLadenFrageMs = 0;
    static std::string TruheOffenBei;          // wer sie offen hat (Stand des Hosts)
    static int TruheRueckgaben = 0;            // so oft zurueckgegeben (der andere war schneller)
    // Host
    static std::map<std::string, CoopTruheGast> TruheGaeste;
    // Testlauf
    static int TruheTestZuNach = 0;            // das naechste Fenster nach so vielen ms selbst schliessen
    static long long TruheTestZuMs = 0;
    static int TruheTestOeffnungen = 0;        // so oft hier geoeffnet und vom Test wieder geschlossen
    static bool TruheTestHalt = false;         // Client: Staende des Hosts zurueckhalten (veraltete Kopie fuer den Gleichstand)
    static long long TruheTestHaltMs = 0;
    static json TruheTestHaltStand;
    static std::string TruheTestTitel = "-";   // u102: Titel des zuletzt geoeffneten Fensters (G2; G1 "-")
    static int TruheDirektZahl = 0;            // u101: so oft ueber den Haken (Benutzen) direkt geoeffnet
    static int TruheGegebenZahl = 0;           // u101: so oft neu ins Inventar gelegt
    static long long TruheSelbstMeldungMs = 0;
    // u101: Taste (Gothic.ini [GOTHICCOOP] coopInvKey, Reihenfolge wie im COOP-Menue)
    struct CoopTruheTaste { const char* name; int key; };
    static const CoopTruheTaste TruheTasten[] = { { "aus", 0 }, { "I", KEY_I }, { "J", KEY_J }, { "K", KEY_K }, { "U", KEY_U },
        { "G", KEY_G }, { "T", KEY_T }, { "V", KEY_V } };
    static const int TruheTastenZahl = sizeof(TruheTasten) / sizeof(TruheTasten[0]);
    static int TruheTasteIdx = -1;
    static long long TruheTasteLesenMs = 0;
    static bool TruheTasteWar = false;
    // u265 (P321): Coop-Inventar mit dem Inventar (Gothic.ini [GOTHICCOOP] coopInvMitInventar, Vorgabe 1)
    static bool TruheMitInv = false;          // das offene Fenster gehoert zum Inventar (passiv daneben)
    static bool TruheMitInvWill = false;      // Inventar ging auf - Inventar zu, im naechsten Bild beide oeffnen
    static bool TruheInvWarOffen = false;     // Inventar im letzten Takt offen (Flanke)
    static bool TruheSchliesstSelbst = false; // u270 (P335): GT schliesst die Coop-Truhe selbst (TruheSchliessenHier) - nie verhindern
    static int TruheMitInvOpt = 1;
    static long long TruheMitInvLesenMs = 0;
    static long long TruheEinloesenMs = 0;    // u265 (P322): Munition/Schluessel/Dietrich aus dem Coop-Inventar, Takt
    // u266 (P324): selbst aus dem Coop-Inventar Genommenes (Instanz -> Menge, Zeit) - solche Quest-Gegenstaende bleiben im eigenen Inventar
    struct CoopTruheGenommen { int menge = 0; long long ms = 0; };
    static std::map<std::string, CoopTruheGenommen> TruheSelbstGenommen;

    static void TruheLog(const char* fmt, ...) {
        char buf[900];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        buf[sizeof(buf) - 1] = 0;
        OthelloCompatLog("[%lld s, %s] Coop-Truhe: %s", DiagSeconds(), DiagRole(), buf);
    }

    static bool TruheCoop() {
        return (ServerThread || ClientThread) && !(MyselfId == "_player_");
    }

    // die Truhe selbst - ausserhalb der Welt, von GT gehalten (Verweis aus der Erzeugung, nie freigegeben)
    static oCMobContainer* TruheObj() {
        if (CoopTruheObj)
            return CoopTruheObj;
        // u102: kein oCMOB::SetName mehr - das nimmt den Namen eines Skript-Symbols (MOBNAME_..., G2A 0x0071BC10: parser->GetIndex
        // nach +0x17C), "Coop-Inventar" gibt es nicht; den Titel bekommt das Fenster beim Oeffnen (TruheOeffnenHier)
        zCObject* o = oCMobContainer::_CreateNewInstance();
        CoopTruheObj = o ? (oCMobContainer*)o : NULL;
        return CoopTruheObj;
    }

    // Inhalt je Instanz; die Coop-Truhe selbst zaehlt nicht
    static CoopTruheBestand TruheLesen() {
        CoopTruheBestand b;
        oCMobContainer* c = TruheObj();
        if (!c)
            return b;
        for (zCListSort<oCItem>* n = c->containList.next; n; n = n->next)
            if (n->data) {
                const std::string inst = CoopItemInst(n->data);
                if (inst != TruheInst)
                    b[inst] += CoopItemAmount(n->data);
            }
        return b;
    }

    static int TruheAnzahl(const CoopTruheBestand& b) {
        int n = 0;
        for (const auto& e : b)
            n += e.second;
        return n;
    }

    static CoopTruheBestand TruheDiff(const CoopTruheBestand& von, const CoopTruheBestand& nach) {
        CoopTruheBestand d;
        for (const auto& e : nach) {
            auto f = von.find(e.first);
            const int alt = f == von.end() ? 0 : f->second;
            if (e.second != alt)
                d[e.first] = e.second - alt;
        }
        for (const auto& e : von)
            if (!nach.count(e.first) && e.second)
                d[e.first] = -e.second;
        return d;
    }

    static std::string TruheText(const CoopTruheBestand& d) {
        std::string s;
        for (const auto& e : d) {
            if (s.size() > 300) {
                s += ", ...";
                break;
            }
            s += (s.empty() ? "" : ", ") + e.first + (e.second > 0 ? " +" : " ") + std::to_string(e.second);
        }
        return s.empty() ? std::string("-") : s;
    }

    // wie oCItemContainer::Remove, ohne die Weltlisten: aus der Liste, Auswahl eines offenen Fensters pruefen, Verweis frei
    static void TruheNimmAus(oCMobContainer* c, oCItem* it) {
        c->containList.Remove(it);
        if (c->items)
            c->items->CheckSelectedItem();
        it->Release();
    }

    // in die Liste (Insert zaehlt mit, der Verweis des Aufrufers bleibt), Auswahl eines offenen Fensters pruefen
    static void TruheLegeEin(oCMobContainer* c, oCItem* it) {
        c->Insert(it);
        if (c->items)
            c->items->CheckSelectedItem();
    }

    // hineingelegte Coop-Truhe heraus (TruheGeben legt gleich eine neue ins Inventar)
    static int TruheEntfernenSelbst(oCMobContainer* c) {
        std::vector<oCItem*> weg;
        for (zCListSort<oCItem>* n = c->containList.next; n; n = n->next)
            if (n->data && CoopItemInst(n->data) == TruheInst)
                weg.push_back(n->data);
        for (oCItem* it : weg)
            TruheNimmAus(c, it);
        return (int)weg.size();
    }

    // Menge einer Instanz hier aendern, auch bei offenem Fenster. Rueckgabe: tatsaechliche Aenderung (heraus: negativ)
    static int TruheAendernHier(const std::string& inst, int delta) {
        oCMobContainer* c = TruheObj();
        if (!c || !parser || !delta || inst == TruheInst)
            return 0;
        WorldSyncApplying++;
        int done = 0;
        if (delta > 0) {
            for (zCListSort<oCItem>* n = c->containList.next; n && !done; n = n->next)
                if (n->data && (n->data->flags & ITM_FLAG_MULTI) && CoopItemInst(n->data) == inst) {
                    n->data->amount = CoopItemAmount(n->data) + delta;
                    done = delta;
                }
            const int idx = done ? -1 : parser->GetIndex(zSTRING(inst.c_str()));
            while (idx >= 0 && done < delta && done < 1000) {
                oCItem* it = CreateCoopItem(idx);
                if (!it)
                    break;
                it->amount = (it->flags & ITM_FLAG_MULTI) ? delta - done : 1;
                done += it->amount;
                TruheLegeEin(c, it);
                it->Release();
            }
        }
        else {
            int need = -delta;
            std::vector<oCItem*> its;
            for (zCListSort<oCItem>* n = c->containList.next; n; n = n->next)
                if (n->data && CoopItemInst(n->data) == inst)
                    its.push_back(n->data);
            for (oCItem* it : its) {
                if (need <= 0)
                    break;
                const int a = CoopItemAmount(it);
                if (a > need) {
                    it->amount = a - need;
                    done -= need;
                    need = 0;
                }
                else {
                    need -= a;
                    done -= a;
                    TruheNimmAus(c, it);
                }
            }
        }
        WorldSyncApplying--;
        return done;
    }

    // ganzen Inhalt ersetzen (Laden, neues Spiel) - danach ist er der bekannte Stand
    static void TruheSetzen(const CoopTruheBestand& b) {
        oCMobContainer* c = TruheObj();
        if (!c)
            return;
        std::vector<oCItem*> alle;
        for (zCListSort<oCItem>* n = c->containList.next; n; n = n->next)
            if (n->data)
                alle.push_back(n->data);
        for (oCItem* it : alle)
            TruheNimmAus(c, it);
        for (const auto& e : b)
            TruheAendernHier(e.first, e.second);
        TruheBekannt = TruheLesen();
    }

    static json TruheJson(const CoopTruheBestand& b) {
        json a = json::array();
        for (const auto& e : b)
            if (e.second)
                a.push_back(json::array({ e.first, e.second }));
        return a;
    }

    // Bestand (nur Mengen > 0) bzw. mit negativ = Aenderung
    static CoopTruheBestand TruheAusJson(const json& a, bool negativ = false) {
        CoopTruheBestand b;
        if (!a.is_array())
            return b;
        for (const auto& x : a)
            if (x.is_array() && x.size() >= 2 && x.at(0).is_string() && x.at(1).is_number_integer()) {
                const int n = x.at(1).get<int>();
                if (n != 0 && n <= 100000 && n >= -100000 && (negativ || n > 0))
                    b[x.at(0).get<std::string>()] += n;
            }
        return b;
    }

    static void TruheSenden(json t) {
        json j;
        j["truhe"] = t;
        DialogSend(j, SYNC_DIALOG_STATE);
    }

    // Host: wer sie offen hat (Host und Clients)
    static std::string TruheOffenListe() {
        std::string s = TruheOffenHier ? DialogMe() : std::string();
        for (const auto& g : TruheGaeste)
            if (g.second.offen)
                s += (s.empty() ? "" : ", ") + g.first;
        return s;
    }

    // hat ein anderer sie neu geoeffnet, waehrend sie hier offen ist: Meldung
    static void TruheAndereMelden(const std::string& liste) {
        std::set<std::string> jetzt;
        size_t a = 0;
        while (a < liste.size()) {
            size_t e = liste.find(", ", a);
            if (e == std::string::npos)
                e = liste.size();
            const std::string n = liste.substr(a, e - a);
            if (!n.empty() && n != DialogMe())
                jetzt.insert(n);
            a = e + 2;
        }
        for (const auto& n : jetzt)
            if (!TruheAndereOffen.count(n) && TruheOffenHier)
                ChatLog(string::Combine(CoopTr("%s hat das Coop-Inventar auch offen.", "%s has the coop inventory open too."), string(n.c_str())),
                    zCOLOR(200, 220, 255, 255));
        TruheAndereOffen = jetzt;
    }

    // Host: neuen Stand an alle (Inhalt, Bestaetigungen je Client, wer sie offen hat)
    static void TruheVerteilen() {
        if (!ServerThread)
            return;
        json t;
        t["op"] = "state";
        t["inhalt"] = TruheJson(TruheLesen());
        json ack = json::object();
        for (const auto& g : TruheGaeste)
            ack[g.first] = json::array({ g.second.epoche, g.second.seq });
        t["ack"] = ack;
        t["offen"] = TruheOffenListe();
        TruheSenden(t);
    }

    // Client: alle unbestaetigten Zuege an den Host (er wendet nur die neuen an)
    static void TruheZuegeSenden() {
        if (TruheZuege.empty() || !TruheCoop() || ServerThread)
            return;
        json z = json::array();
        for (const auto& e : TruheZuege)
            z.push_back(json::array({ e.seq, TruheJson(e.d) }));
        json t;
        t["op"] = "chg";
        t["ep"] = TruheEpoche;
        t["z"] = z;
        TruheSenden(t);
        TruheSendMs = GetCurrentMs();
        TruheSendungen++;
    }

    // eigene Aenderung hier (Fenster, Testlauf): mit dem bekannten Stand vergleichen und melden
    static void TruhePruefeHier() {
        oCMobContainer* c = TruheObj();
        if (!c)
            return;
        if (TruheEntfernenSelbst(c))
            TruheGebenMs = 0;
        const CoopTruheBestand jetzt = TruheLesen();
        if (jetzt == TruheBekannt)
            return;
        const CoopTruheBestand d = TruheDiff(TruheBekannt, jetzt);
        TruheBekannt = jetzt;
        // u266 (P324): was hier herausging (Klick, Einloesen, Skript), merken - so genommene Quest-Gegenstaende bleiben im eigenen Inventar
        for (const auto& e : d)
            if (e.second < 0) {
                CoopTruheGenommen& g = TruheSelbstGenommen[e.first];
                if (GetCurrentMs() > g.ms + 10000)
                    g.menge = 0;
                g.menge -= e.second;
                g.ms = GetCurrentMs();
            }
        if (!TruheCoop())
            return;
        if (ServerThread) {
            TruheVerteilen();
            return;
        }
        if (!TruheEpoche)
            TruheEpoche = GetCurrentMs();
        TruheZuege.push_back({ ++TruheSeq, d });
        TruheSendungen = 0;
        TruheZuegeSenden();
    }

    // Client: Stand des Hosts - plus eigene unbestaetigte Zuege - hierher (bei offenem Fenster mitten hinein)
    static void TruheStandAnwenden(const json& t) {
        TruhePruefeHier();   // Aenderungen dieses Bilds zuerst melden - sonst gingen sie beim Angleichen verloren
        TruheLadenFragen = 0;
        TruheOffenBei = t.value("offen", std::string());
        const std::string me = DialogMe();
        if (t.contains("ack") && t["ack"].is_object() && t["ack"].contains(me)) {
            const json& a = t["ack"][me];
            if (a.is_array() && a.size() >= 2 && a[0].is_number_integer() && a[1].is_number_integer() && a[0].get<long long>() == TruheEpoche) {
                const int s = a[1].get<int>();
                const size_t vorher = TruheZuege.size();
                TruheZuege.erase(std::remove_if(TruheZuege.begin(), TruheZuege.end(), [s](const CoopTruheZug& z) { return z.seq <= s; }),
                    TruheZuege.end());
                if (TruheZuege.size() != vorher)
                    TruheSendungen = 0;
            }
        }
        CoopTruheBestand ziel = TruheAusJson(t["inhalt"]);
        for (const auto& z : TruheZuege)
            for (const auto& e : z.d)
                ziel[e.first] += e.second;
        for (auto i = ziel.begin(); i != ziel.end();)
            i = i->second <= 0 ? ziel.erase(i) : std::next(i);
        const CoopTruheBestand d = TruheDiff(TruheLesen(), ziel);
        for (const auto& e : d)
            TruheAendernHier(e.first, e.second);
        TruheBekannt = TruheLesen();
        if (!d.empty())
            TruheLog("Stand des Hosts uebernommen: %s%s", TruheText(d).c_str(), TruheOffenHier ? " (im offenen Fenster)" : "");
        TruheAndereMelden(TruheOffenBei);
    }

    // Host: Zuege eines Clients der Reihe nach anwenden; was er nahm, aber nicht mehr da war, bekommt er als "undo" zurueck
    static void TruheZuegeAnwenden(const json& t, const std::string& by) {
        TruhePruefeHier();   // eigene Aenderungen des Hosts zuerst
        CoopTruheGast& g = TruheGaeste[by];
        const long long ep = t.contains("ep") && t["ep"].is_number_integer() ? t["ep"].get<long long>() : 0;
        if (ep != g.epoche) {
            g.epoche = ep;
            g.seq = 0;
        }
        CoopTruheBestand fehlt, angewandt;
        if (t.contains("z") && t["z"].is_array())
            for (const auto& z : t["z"]) {
                if (!z.is_array() || z.size() < 2 || !z[0].is_number_integer())
                    continue;
                const int seq = z[0].get<int>();
                if (seq <= g.seq)
                    continue;   // schon angewandt (neu gesendet)
                g.seq = seq;
                for (const auto& e : TruheAusJson(z[1], true)) {
                    const int r = TruheAendernHier(e.first, e.second);
                    if (r)
                        angewandt[e.first] += r;
                    if (e.second < 0 && r > e.second)
                        fehlt[e.first] += r - e.second;
                }
            }
        TruheBekannt = TruheLesen();
        if (!fehlt.empty()) {
            json u;
            u["op"] = "undo";
            u["to"] = by;
            u["d"] = TruheJson(fehlt);
            TruheSenden(u);
            TruheLog("%s nahm, was schon weg war: %s - gibt es zurueck", by.c_str(), TruheText(fehlt).c_str());
        }
        if (!angewandt.empty())
            TruheLog("von %s: %s%s", by.c_str(), TruheText(angewandt).c_str(), TruheOffenHier ? " (im offenen Fenster)" : "");
        TruheVerteilen();
    }

    // Client: der andere war schneller - das Genommene aus dem eigenen Inventar wieder heraus (es ist weg)
    static void TruheRueckgabe(const json& t) {
        const CoopTruheBestand d = TruheAusJson(t["d"]);
        if (!player || !parser || d.empty())
            return;
        std::string shown;
        WorldSyncApplying++;
        for (const auto& e : d) {
            const int idx = parser->GetIndex(zSTRING(e.first.c_str()));
            int left = e.second, weg = 0;
            std::string name = e.first;
            while (idx >= 0 && left > 0) {
                oCItem* it = player->IsInInv(idx, 1);
                if (!it)
                    break;
                if (weg == 0)
                    name = it->name.ToChar();
                const int take = (std::min)(left, CoopItemAmount(it));
                oCItem* r = player->RemoveFromInv(it, take);
                if (!r)
                    break;
                ogame->GetGameWorld()->RemoveVob(r);
                left -= take;
                weg += take;
            }
            if (weg)
                ChatLog(string::Combine(CoopTr("Coop-Inventar: %i x %s war schon weg - der andere war schneller.",
                    "Coop inventory: %i x %s was already gone - the other player was faster."), weg, string(name.c_str())), zCOLOR(255, 200, 80, 255));
            shown += (shown.empty() ? "" : ", ") + e.first + " " + std::to_string(weg) + "/" + std::to_string(e.second);
        }
        WorldSyncApplying--;
        CoopMissionReset(500);   // GTs eigene Aenderung - nicht spiegeln (u96)
        TruheRueckgaben++;
        TruheLog("der Host hatte es nicht mehr - aus dem Inventar zurueckgegeben: %s", shown.c_str());
    }

    // u265 (P321): Einstellung (je 2 s gelesen)
    static bool TruheMitInvAn() {
        const long long now = GetCurrentMs();
        if (zoptions && now > TruheMitInvLesenMs + 2000) {
            TruheMitInvLesenMs = now;
            TruheMitInvOpt = zoptions->ReadInt(zSTRING("GOTHICCOOP"), "coopInvMitInventar", 1);
        }
        return TruheMitInvOpt != 0;
    }

    // u265 (P321): im Gespraech (auch Handel) nicht mit dem Inventar oeffnen
    static bool TruheImGespraech() {
        oCInformationManager& im = oCInformationManager::GetInformationManager();
        return im.Npc && !im.IsDone;
    }

    // u265 (P321), u266 (P323): Fokus. Das Coop-Fenster bleibt nicht passiv - wie bei einer Truhe wechseln Tab und die Coop-Taste zwischen
    // den Fenstern, Esc schliesst. Dass Aktion, Linksklick und Alt im eigenen Inventar anlegen, benutzen bzw. wegwerfen, regelt der Haken
    // auf oCNpcInventory::HandleEvent (Coop-Fenster nur waehrend dieses Aufrufs passiv - die Engine sieht dann kein Nachbarfenster)
    static void TruheFokusInventar(oCMobContainer* c) {
        if (!c || !c->items || !player)
            return;
        if (c->items->IsActive())
            c->items->Deactivate();
        c->items->passive = TruheMitInvAn() ? TRUE : FALSE;   // u272 (P337): mit der Einstellung kein Fenster daneben fuer die Engine
        if (!player->inventory2.IsActive())
            player->inventory2.Activate();
    }

    static void TruheFokusTruhe(oCMobContainer* c) {
        if (!c || !c->items || !player)
            return;
        c->items->passive = FALSE;
        if (player->inventory2.IsActive())
            player->inventory2.Deactivate();
        c->items->Activate();
    }

    static void TruheSchliessenHier(const char* why) {
        oCMobContainer* c = TruheObj();
        if (!c || !TruheOffenHier)
            return;
        TruhePruefeHier();   // letzte Aenderungen im Fenster
        TruheOffenHier = false;
        TruheMitInv = false;   // u265 (P321)
        if (c->items && player) {
            TruheSchliesstSelbst = true;   // u270 (P335)
            c->Close(player);
            TruheSchliesstSelbst = false;
        }
        TruheLog("geschlossen (%s) - %d Gegenstaende", why, TruheAnzahl(TruheBekannt));
        if (TruheTestZuMs) {
            TruheTestZuMs = 0;
            TruheTestOeffnungen++;
        }
        if (!TruheCoop())
            return;
        if (ServerThread) {
            TruheVerteilen();
            return;
        }
        json t;
        t["op"] = "zu";
        TruheSenden(t);
    }

    // oeffnen - keine Sperre: hat der andere sie offen, sehen beide jede Aenderung (u99)
    static void TruheOeffnenHier() {
        oCMobContainer* c = TruheObj();
        if (!c || !player || TruheOffenHier)
            return;
        TruhePruefeHier();
        c->Open(player);
        TruheOffenHier = true;
        TruheInvGesehen = false;
        if (!c->items) {
            TruheMitInvWill = false;
            TruheSchliessenHier("Fenster kam nicht");
            return;
        }
        // u265 (P321): mit dem Inventar geoeffnet - Coop-Fenster passiv daneben, das eigene Inventar bleibt bedienbar wie immer
        TruheMitInv = TruheMitInvWill;
        TruheMitInvWill = false;
        if (TruheMitInv)
            TruheFokusInventar(c);
        if (TruheMitInv && !player->inventory2.IsOpen())   // u293 (P392): nur Diagnose
            TruheLog("eigenes Inventar ging nicht mit auf (Waffe %d, Koerperzustand %d) (P392)", (int)player->GetWeaponMode(), (int)player->GetBodyState());
#if ENGINE >= Engine_G2
        // u102 (P124, Nutzer: "man kann noch einen Namen hinzufügen wie beim Handeln mit NPCs, zb "Brian""): Titel ueber dem Fenster.
        // G2A oCMobContainer::Open gibt dem Fenster oCMOB::GetName (Skript-Symbol, hier leer); oCItemContainer::SetName (0x00704F60)
        // setzt titleText (+0x0C), DrawCategory (0x00706B60) zeichnet ihn in viewTitle (+0x60). G1 hat kein Titelfeld (dort steht die
        // Kategorie) - dort bleibt es ohne.
        zSTRING titel(CoopTr("Coop-Inventar", "Coop inventory"));
        c->items->SetName(titel);
        TruheTestTitel = c->items->titleText.ToChar();
#endif
        if (TruheTestZuNach > 0) {
            TruheTestZuMs = GetCurrentMs() + TruheTestZuNach;
            TruheTestZuNach = 0;
        }
        TruheLog("geoeffnet (%d Gegenstaende)", TruheAnzahl(TruheBekannt));
        if (!TruheCoop())
            return;
        TruheAndereOffen.clear();   // wer sie schon offen hat, gilt beim Oeffnen als neu (Meldung)
        if (ServerThread) {
            TruheVerteilen();
            TruheAndereMelden(TruheOffenListe());
            return;
        }
        json t;
        t["op"] = "auf";   // der Host antwortet mit seinem Stand (Angleichen im offenen Fenster)
        TruheSenden(t);
    }

    // CoopDialogSync.cpp, DialogApplyState: Paket "truhe"
    void CoopTruheReceive(const json& t, const std::string& by) {
        const std::string op = t.value("op", std::string());
        if (ServerThread) {   // der Host fuehrt - Zuege und Meldungen der Clients
            if (op == "chg")
                TruheZuegeAnwenden(t, by);
            else if (op == "auf" || op == "zu") {
                TruheGaeste[by].offen = op == "auf";
                TruheLog("%s hat sie %s", by.c_str(), op == "auf" ? "geoeffnet" : "geschlossen");
                TruheVerteilen();
                TruheAndereMelden(TruheOffenListe());
            }
            else if (op == "frage")
                TruheVerteilen();
            return;
        }
        // Client: was der Host schickt
        if (op == "state") {
            if (TruheTestHalt) {   // Testlauf: veraltete Kopie behalten (Gleichstand nachstellen)
                TruheTestHaltStand = t;
                return;
            }
            TruheStandAnwenden(t);
        }
        else if (op == "undo" && t.value("to", std::string()) == DialogMe())
            TruheRueckgabe(t);
    }

    // den Gegenstand ins Inventar, falls er fehlt - nicht, solange der Held etwas benutzt (dann ist er gerade in der Hand)
    static void TruheGeben() {
        if (!player || player->IsDead() || player->IsUnconscious() || !parser || player->interactItem || CoopHeroSwapped())   // u149 (P152): nicht dem Tier
            return;
        if (player->GetBodyState() > BS_SPRINT && !player->inventory2.IsOpen())
            return;
        const int idx = parser->GetIndex(zSTRING(TruheInst));
        if (idx < 0 || player->IsInInv(idx, 1))
            return;
        WorldSyncApplying++;
        oCItem* it = CreateCoopItem(idx);
        oCItem* in = it ? player->PutInInv(it) : NULL;
        WorldSyncApplying--;
        if (in)
            TruheGegebenZahl++;
        if (!TruheGegebenGemeldet || !in) {
            TruheGegebenGemeldet = true;
            TruheLog("Gegenstand %s ins Inventar %s", TruheInst, in ? "gelegt" : "- FEHLGESCHLAGEN");
        }
    }

    static bool TruheIstGegenstand(zCVob* v) {
        oCItem* it = v ? zDYNAMIC_CAST<oCItem>(v) : NULL;
        return it && CoopItemInst(it) == TruheInst;
    }

    // u101: ein anderes Gegenstands-Fenster offen (Truhe der Welt, Pluendern, Handel) bzw. eine Truhe in Benutzung - dann nicht oeffnen
    static bool TruheAnderesFenster() {
        if (!player)
            return true;
        for (zCList<oCItemContainer>* l = oCItemContainer::contList.GetNextInList(); l; l = l->GetNextInList()) {
            oCItemContainer* f = l->GetData();
            if (f && f != &player->inventory2 && !(CoopTruheObj && f == CoopTruheObj->items) && f->IsOpen())
                return true;
        }
        return player->GetInteractMob() != NULL;
    }

    // u101: Benutzen bzw. Taste - direkt oeffnen (ohne Bewegung), sobald der Held frei ist (das Inventar geht dafuer erst zu)
    static void TruheDirekt(const char* wie) {
        if (TruheOffenHier)
            return;
        TruheWill = true;
        TruheWillMs = GetCurrentMs();
        TruheLog("%s - oeffne direkt", wie);
    }

    // u101 (P123): Benutzen aus dem Inventar - fuer die Coop-Truhe gleich erledigt (TRUE: Nachricht fertig), ohne Bewegung, nichts
    // verbraucht; alle anderen Gegenstaende und Helden unveraendert ans Original
    int __fastcall oCNpc_EV_UseItemToState_Truhe(oCNpc* _this, void* vtable, oCMsgManipulate* msg);
    CInvoke<int(__thiscall*)(oCNpc*, oCMsgManipulate*)> Ivk_oCNpc_EV_UseItemToState_Truhe(CoopAddr::oCNpc_EV_UseItemToState,
        &oCNpc_EV_UseItemToState_Truhe, COOP_IVK(Save));
    int __fastcall oCNpc_EV_UseItemToState_Truhe(oCNpc* _this, void* vtable, oCMsgManipulate* msg) {
        CoopMsgVobCheck(_this, msg, "EV_UseItemToState");   // u142 (P142, Helpers.cpp)
        CoopMsgItemNote(_this, msg);                         // u169 (P176): Diagnose, wer den Gegenstand freigibt
        if (_this && _this == player && msg && TruheIstGegenstand(msg->targetVob)) {
            TruheDirektZahl++;
            TruheDirekt("benutzt");
            return TRUE;
        }
        const void* keep = CoopMsgItemInUse;
        CoopMsgItemInUse = msg ? msg->targetVob : NULL;
        const int r = Ivk_oCNpc_EV_UseItemToState_Truhe(_this, msg);
        CoopMsgItemInUse = keep;
        return r;
    }

    // u169 (P176, Liste 1): Diagnose - gibt die Engine einen Gegenstand frei, den eine laufende Nachricht haelt (Helpers.cpp)
    void __fastcall oCItem_Dtor_Coop(oCItem* _this, void* vtable);
    CInvoke<void(__thiscall*)(oCItem*)> Ivk_oCItem_Dtor_Coop(CoopAddr::oCItem_Dtor, &oCItem_Dtor_Coop, COOP_IVK(Save));
    void __fastcall oCItem_Dtor_Coop(oCItem* _this, void* vtable) {
        if (!CoopMsgItems.empty())
            CoopMsgItemFreeing(_this);
        CoopZauberbuchItemFreeing(_this);   // u292 (P389): Diagnose - Gegenstand aus dem Zauberbuch des eigenen Helden wird zerstoert (Helpers.cpp)
        Ivk_oCItem_Dtor_Coop(_this);
    }

    bool CoopIstTeleportRune(oCItem* it);   // u272 (P338): CoopTeleport.cpp

    // u101 (P123): Hinueberschieben aus dem eigenen Inventar bei offenem Coop-Inventar - die Coop-Truhe selbst bleibt, wo sie ist
    int __fastcall oCItemContainer_TransferItem_Truhe(oCItemContainer* _this, void* vtable, int a, int b);
    CInvoke<int(__thiscall*)(oCItemContainer*, int, int)> Ivk_oCItemContainer_TransferItem_Truhe(CoopAddr::oCItemContainer_TransferItem,
        &oCItemContainer_TransferItem_Truhe, COOP_IVK(Save));
    int __fastcall oCItemContainer_TransferItem_Truhe(oCItemContainer* _this, void* vtable, int a, int b) {
        // u272 (P338, Nutzer 15.11.: "es sollte nicht moeglich sein die Teleport rune aus dem Inventar zu entfernen weder boden schmeissen,
        // noch in truhe legen"): die Teleport-Rune geht in kein anderes Fenster (Truhe, Coop-Inventar, Pluendern)
        if (_this && player && _this == &player->inventory2 && CoopIstTeleportRune(_this->GetSelectedItem())) {
            const long long now = GetCurrentMs();
            if (now > TruheSelbstMeldungMs + 2000) {
                TruheSelbstMeldungMs = now;
                ChatLog(CoopTr("Die Teleport-Rune bleibt im Inventar.", "The teleport rune stays in your inventory."), zCOLOR(255, 200, 80, 255));
                TruheLog("Teleport-Rune sollte in ein anderes Fenster - nicht verschoben (P338)");
            }
            return FALSE;
        }
        // u266 (P323, Nutzer 15.11.: "man soll das Coop inventar nicht wegschmeissen koennen"): die Coop-Truhe geht in kein anderes Fenster
        // (Coop-Inventar, Truhe der Welt, Pluendern) - vorher nur bei offenem Coop-Inventar gesperrt
        if (_this && player && _this == &player->inventory2 && TruheIstGegenstand(_this->GetSelectedItem())) {
            const long long now = GetCurrentMs();
            if (now > TruheSelbstMeldungMs + 2000) {
                TruheSelbstMeldungMs = now;
                ChatLog(CoopTr("Das Coop-Inventar bleibt bei dir.", "The coop inventory stays with you."), zCOLOR(255, 200, 80, 255));
                TruheLog("Coop-Truhe sollte in ein anderes Fenster - nicht verschoben");
            }
            return FALSE;
        }
        // u281 (P359): angelegte Munition ins offene Coop-Inventar - die Engine verschiebt Angelegtes nicht. Merker kurz weg, verschieben, am
        // Rest im eigenen Inventar wieder setzen (die Fernkampfwaffe bleibt angelegt)
        oCItem* mun = _this && player && _this == &player->inventory2 && TruheOffenHier ? _this->GetSelectedItem() : NULL;
        if (mun && (mun->flags & ITM_FLAG_ACTIVE) && (mun->mainflag & ITM_CAT_MUN)) {
            const int inst = mun->GetInstance();
            const std::string nm = mun->GetInstanceName().ToChar();
            mun->flags &= ~ITM_FLAG_ACTIVE;
            const int r = Ivk_oCItemContainer_TransferItem_Truhe(_this, a, b);
            oCItem* rest = player->IsInInv(inst, 1);
            if (rest)
                rest->flags |= ITM_FLAG_ACTIVE;
            TruheLog("angelegte Munition %s hinueber: %s, Rest im Inventar %s (P359)", nm.c_str(), r ? "verschoben" : "nicht verschoben",
                rest ? "wieder angelegt" : "keiner");
            return r;
        }
        return Ivk_oCItemContainer_TransferItem_Truhe(_this, a, b);
    }

    // u266 (P323): Meldung (Chat hoechstens alle 2 s)
    static void TruheMeldungBleibt() {
        const long long now = GetCurrentMs();
        if (now > TruheSelbstMeldungMs + 2000) {
            TruheSelbstMeldungMs = now;
            ChatLog(CoopTr("Das Coop-Inventar bleibt bei dir.", "The coop inventory stays with you."), zCOLOR(255, 200, 80, 255));
            TruheLog("Coop-Truhe sollte weg (wegwerfen bzw. in ein anderes Fenster) - bleibt");
        }
    }

    // u266 (P323, Nutzer 15.11.: "wieso denn den ganzen stapel"): Rechtsklick wie der Linksklick bei einer Truhe - ein Stueck je Klick,
    // gedrueckt halten legt nach und nach mehr hinueber (Zaehler der Engine; G2: bei einem neuen Druck von vorn). Rueckgabe: Taste fuers
    // Original
    static int TruheWieLinksklick(oCItemContainer* f) {
#if ENGINE >= Engine_G2
        if (f && zinput->KeyToggled(MOUSE_BUTTONRIGHT))
            f->TransferCount = 0;
#endif
        return MOUSE_BUTTONLEFT;
    }

    // u268 (P332, Nutzer 15.11. zu u267 in Othello: "rechtsclick schliesst nicht mehr das inventar aber das coop inventar und reingelegt
    // wird auch nichts"): der Rechtsklick legt selbst hinueber bzw. nimmt selbst heraus - ohne die Engine, deren Truhen-Klick unter
    // Gothic 1 anders laeuft (sie schloss das Coop-Fenster). Welches Fenster gemeint ist, sagt der Fokus. Ein Stueck je Klick, gehalten
    // nach 0,45 s fortlaufend (CoopTruhePulse)
    static long long TruheRechtsWeiterMs = 0;
    static bool TruheRechtsFrei = true;
    static int TruheRechtsZahl = 0;

    // u269 (P333, Nutzer 15.11. zu u268 in Othello: "immernoch das selbe ausserdem laggt das spiel jetzt sobald man rechtsclick drueckt",
    // "es koennte an GRawInput liegen"): Othello liefert der Engine keine Maustasten-Ereignisse - der Puls erkennt den Rechtsklick
    // selbst. Dazu Diagnose: wer will das Coop-Fenster bzw. das Inventar schliessen, welche Tasten kommen an
    static bool TruheRechtsWar = false;          // rechte Maustaste im letzten Takt gedrueckt
    static long long TruheRechtsDruckMs = 0;     // Zeit des letzten Drucks
    static int TruheRechtsWiederAuf = 0;         // Coop-Fenster in diesem Druck wieder geoeffnet
    static int TruheZuVerhindert = 0;            // CloseInventory in diesem Druck verhindert
    static bool TruheRechtsAusCoop = false;      // u271 (P336): Coop-Fenster hatte beim Druck den Fokus (Richtung fuer den ganzen Druck)
    static bool TruheTabZuCoop = false;          // u272 (P337): Tab im Inventar - der Puls wechselt ins Coop-Fenster
    static long long TruheDiagMs = 0;            // Diagnosezeilen hoechstens alle 300 ms

    static bool TruheRechtsGedrueckt() {
        return zinput && (zinput->GetMouseButtonPressedRight() || zinput->KeyPressed(MOUSE_BUTTONRIGHT));
    }

    static std::string TruheAufrufer(const void* ret) {
        char m[160];
        CoopModuleOf(ret, m, sizeof(m));
        return m[0] ? std::string(m) : std::string("?");
    }

    static bool TruheDiagJetzt() {
        const long long now = GetCurrentMs();
        if (now - TruheDiagMs < 300)
            return false;
        TruheDiagMs = now;
        return true;
    }

    static bool TruheRechtsEinStueck(bool ausCoop) {
        oCMobContainer* c = TruheOffenHier ? TruheObj() : NULL;
        if (!c || !c->items || !player || !parser)
            return false;
        if (ausCoop) {   // Coop-Fenster hatte beim Druck den Fokus: ein Stueck heraus (u271: Richtung vom Druck, P336)
            oCItem* sel = c->items->GetSelectedItem();
            if (!sel)
                return false;
            const int idx = sel->GetInstance();
            const std::string inst = CoopItemInst(sel);
            TruhePruefeHier();
            const int r = -TruheAendernHier(inst, -1);
            TruhePruefeHier();
            if (r <= 0)
                return false;
            if (oCItem* it = CreateCoopItem(idx)) {
                it->amount = 1;
                WorldSyncApplying++;
                player->PutInInv(it);
                WorldSyncApplying--;
            }
            return true;
        }
        oCItem* sel = player->inventory2.GetSelectedItem();   // eigenes Inventar hat den Fokus: ein Stueck hinein
        if (!sel)
            return false;
        // u281 (P359, Test Othello u278 17.11.: "Pfeile sind immer ausgeruestet, man kann sie nicht ablegen, um sie z. B. ins Coop-Inventar zu
        // legen"): Munition gilt bei angelegter Fernkampfwaffe als angelegt - sie darf trotzdem hinueber (die Waffe bleibt angelegt)
        if (sel->HasFlag(ITM_FLAG_ACTIVE) && !(sel->mainflag & ITM_CAT_MUN)) {
            ChatLog(CoopTr("Angelegte Gegenst" "\xE4" "nde erst ablegen.", "Unequip items first."), zCOLOR(255, 200, 80, 255));
            return false;
        }
        if (TruheIstGegenstand(sel)) {
            TruheMeldungBleibt();
            return false;
        }
        if (CoopIstTeleportRune(sel)) {   // u272 (P338): die Teleport-Rune bleibt im Inventar
            ChatLog(CoopTr("Die Teleport-Rune bleibt im Inventar.", "The teleport rune stays in your inventory."), zCOLOR(255, 200, 80, 255));
            return false;
        }
        const std::string inst = CoopItemInst(sel);
        TruhePruefeHier();
        WorldSyncApplying++;
        oCItem* r = player->RemoveFromInv(sel, 1);
        if (r)
            ogame->GetGameWorld()->RemoveVob(r);
        WorldSyncApplying--;
        if (r)
            TruheAendernHier(inst, 1);
        TruhePruefeHier();
        return r != NULL;
    }

    // ein Druck = ein Stueck; weitere Ereignisse desselben Drucks zaehlen nicht (CoopTruhePulse gibt nach dem Loslassen frei). Rueckgabe
    // TRUE fuer die Haken: der Klick ist verbraucht, nichts schliesst
    static int TruheRechtsKlick(const char* wo) {
        if (!TruheRechtsFrei)
            return TRUE;
        TruheRechtsFrei = false;
        TruheRechtsZahl = 0;
        TruheRechtsWeiterMs = GetCurrentMs() + 450;
        oCMobContainer* c = TruheObj();
        const int coopAktiv = c && c->items && c->items->IsActive() ? 1 : 0;
        TruheRechtsAusCoop = coopAktiv != 0;   // u271 (P336)
        const bool ok = TruheRechtsEinStueck(TruheRechtsAusCoop);
        TruheLog("Rechtsklick (%s): Coop-Fenster aktiv %d - %s (P332)", wo, coopAktiv, ok ? "ein Stueck hinueber" : "nichts");
        return TRUE;
    }

    // u266 (P323, Nutzer 15.11.: "man soll das Coop inventar nicht wegschmeissen koennen", "Alt ist die taste fuer wegschmeissen,
    // vielleicht solltest du irgendwie rechtsclick fuer legen ins coop inventar nutzen", "das inventar sollte sich nicht schliessen
    // koennen mit rechtsclick"): eigenes Inventar. Alt auf die Coop-Truhe wirft sie nie weg (immer). Bei offenem Coop-Inventar legt der
    // Rechtsklick hinueber wie der Linksklick bei einer Truhe und schliesst nichts; angelegte Gegenstaende bleiben (Meldung). Mit dem
    // Coop-Inventar daneben (Einstellung) wirken Aktion, Linksklick und Alt wie im Inventar allein (anlegen bzw. benutzen, Alt wirft weg):
    // das Coop-Fenster ist dafuer waehrend des Aufrufs passiv. Tab wechselt wie bei jeder Truhe (Original)
    int __fastcall oCNpcInventory_HandleEvent_Truhe(oCNpcInventory* _this, void* vtable, int key);
    CInvoke<int(__thiscall*)(oCNpcInventory*, int)> Ivk_oCNpcInventory_HandleEvent_Truhe(CoopAddr::oCNpcInventory_HandleEvent,
        &oCNpcInventory_HandleEvent_Truhe, COOP_IVK(Save));
    int __fastcall oCNpcInventory_HandleEvent_Truhe(oCNpcInventory* _this, void* vtable, int key) {
        // u269 (P333): Diagnose - welche Taste kommt bei gedrueckter Maustaste an (Othello/GRawInput)?
        if (TruheOffenHier && zinput && player && _this == &player->inventory2
            && (zinput->GetMouseButtonPressedRight() || zinput->GetMouseButtonPressedLeft()) && TruheDiagJetzt())
            TruheLog("Ereignis im Inventar bei gedrueckter Maustaste (links %d, rechts %d): Taste %d, aktiv %d (P333)",
                zinput->GetMouseButtonPressedLeft(), zinput->GetMouseButtonPressedRight(), key, (int)_this->IsActive());
        // u267 (P331): Diagnose - kommt der Rechtsklick hier an, und in welchem Zustand?
        if (key == MOUSE_BUTTONRIGHT && player && _this == &player->inventory2 && zinput && zinput->KeyToggled(MOUSE_BUTTONRIGHT))
            TruheLog("Rechtsklick im Inventar: aktiv %d, Coop-Fenster %d, mit Inventar %d (P331)", (int)_this->IsActive(), (int)TruheOffenHier, (int)TruheMitInv);
        // u268 (P332): bei offenem Coop-Fenster legt der Rechtsklick selbst hinueber bzw. nimmt heraus - auch wenn gerade das Coop-Fenster
        // den Fokus hat und die Engine das Ereignis hier zuerst abliefert
        if (key == MOUSE_BUTTONRIGHT && player && _this == &player->inventory2 && TruheOffenHier && TruheObj() && TruheObj()->items)
            return TruheRechtsKlick("Inventar");
        if (_this && player && _this == &player->inventory2 && zinput && _this->IsActive()) {
            oCItem* sel = _this->GetSelectedItem();
            const bool alt = zinput->IsBinded(GAME_SMOVE, (unsigned short)key) != 0;
            if (alt && TruheIstGegenstand(sel)) {
                if (zinput->GetToggled(GAME_SMOVE))
                    TruheMeldungBleibt();
                return TRUE;
            }
            oCMobContainer* c = TruheOffenHier ? TruheObj() : NULL;
            if (c && c->items) {
                if (TruheMitInvAn() && (alt || key == MOUSE_BUTTONLEFT || zinput->IsBinded(GAME_ACTION, (unsigned short)key))) {
                    const int warPassiv = c->items->passive;   // u272 (P337): mit Inventar-Fokus haelt der Puls es ohnehin passiv
                    c->items->passive = TRUE;
                    // u269 (P334, Nutzer 15.11.: "der linksclick legt jetzt das item ins inventar sodass man es nicht verwenden kann"):
                    // Gothic 1 legt trotz passiv hinueber - fuer diesen Aufruf steht das Coop-Fenster auch nicht in der Liste der offenen
                    // Fenster (GetNextContainerLeft findet es dann nicht), danach wieder hinein
                    oCItemContainer* ci = c->items;
                    const bool drin = oCItemContainer::contList.IsInList(ci) != 0;
                    if (drin)
                        oCItemContainer::contList.Remove(ci);
                    const int r = Ivk_oCNpcInventory_HandleEvent_Truhe(_this, key);
                    if (drin && CoopTruheObj && CoopTruheObj->items == ci && ci->IsOpen() && !oCItemContainer::contList.IsInList(ci))
                        oCItemContainer::contList.Insert(ci);
                    if (c->items)
                        c->items->passive = warPassiv;
                    return r;
                }
            }
            // u267 (P331, Nutzer 15.11.: "rechtsclick schliesst immernoch das inventar"): mit der Einstellung schliesst der Rechtsklick
            // das Inventar nie - auch ohne Coop-Fenster daneben (allein gespielt, noch nicht verbunden)
            if (key == MOUSE_BUTTONRIGHT && TruheMitInvAn())
                return TRUE;
        }
        return Ivk_oCNpcInventory_HandleEvent_Truhe(_this, key);
    }

    // u266 (P323, Nutzer 15.11.: "im coop inventar ist es dann egal ob man links click oder rechtsclick drueckt"): im Coop-Fenster nimmt
    // auch der Rechtsklick heraus - wie der Linksklick (ein Stueck, gehalten mehr) - und schliesst nichts (die Engine schloss mit
    // Rechtsklick Fenster und Inventar)
    int __fastcall oCItemContainer_HandleEvent_Truhe(oCItemContainer* _this, void* vtable, int key);
    CInvoke<int(__thiscall*)(oCItemContainer*, int)> Ivk_oCItemContainer_HandleEvent_Truhe(CoopAddr::oCItemContainer_HandleEvent,
        &oCItemContainer_HandleEvent_Truhe, COOP_IVK(Save));
    int __fastcall oCItemContainer_HandleEvent_Truhe(oCItemContainer* _this, void* vtable, int key) {
        // u269 (P333): Diagnose wie im Inventar
        if (TruheOffenHier && zinput && CoopTruheObj && _this == CoopTruheObj->items
            && (zinput->GetMouseButtonPressedRight() || zinput->GetMouseButtonPressedLeft()) && TruheDiagJetzt())
            TruheLog("Ereignis im Coop-Fenster bei gedrueckter Maustaste (links %d, rechts %d): Taste %d, aktiv %d (P333)",
                zinput->GetMouseButtonPressedLeft(), zinput->GetMouseButtonPressedRight(), key, (int)_this->IsActive());
        // u268 (P332): Rechtsklick im bzw. am Coop-Fenster - ob es den Fokus hat oder nicht - legt selbst hinueber bzw. nimmt heraus und
        // schliesst nichts (unter Gothic 1 schloss die Engine das Fenster)
        if (_this && player && key == MOUSE_BUTTONRIGHT && CoopTruheObj && _this == CoopTruheObj->items && TruheOffenHier)
            return TruheRechtsKlick("Coop-Fenster");
        return Ivk_oCItemContainer_HandleEvent_Truhe(_this, key);
    }

    // u267 (P331, Nutzer 15.11.: "rechtsclick schliesst immernoch das inventar"): schliesst ein anderer Weg das Inventar am Coop-Haken
    // vorbei (Maussteuerung einer Mod, Engine ausserhalb von HandleEvent), bleibt es mit der Einstellung offen, solange die rechte
    // Maustaste gedrueckt ist. Gespraech und Tod schliessen weiter
    void __fastcall oCNpc_CloseInventory_Truhe(oCNpc* _this, void* vtable);
    CInvoke<void(__thiscall*)(oCNpc*)> Ivk_oCNpc_CloseInventory_Truhe(CoopAddr::oCNpc_CloseInventory, &oCNpc_CloseInventory_Truhe,
        COOP_IVK(Save));
    void __fastcall oCNpc_CloseInventory_Truhe(oCNpc* _this, void* vtable) {
        // u272 (P337): Tab (GAME_INVENTORY) im eigenen Inventar bei offenem, passivem Coop-Fenster - die Engine sieht kein Fenster daneben
        // und will das Inventar schliessen; statt dessen wechselt der Fokus ins Coop-Fenster (im Puls, nicht mitten in den Ereignissen)
        if (_this && _this == player && zinput && TruheOffenHier && TruheMitInvAn() && zinput->GetState(GAME_INVENTORY) > 0.0f
            && !TruheImGespraech() && !_this->IsDead()) {
            oCMobContainer* tc = TruheObj();
            if (tc && tc->items && tc->items->IsOpen() && !tc->items->IsActive()) {
                TruheTabZuCoop = true;
                return;
            }
        }
        if (_this && _this == player && zinput && TruheMitInvAn() && TruheRechtsGedrueckt() && !TruheImGespraech()
            && !_this->IsDead()) {
            if (++TruheZuVerhindert == 1)   // u269 (P333): einmal je Druck, mit Aufrufer
                TruheLog("Inventar schliessen bei Rechtsklick verhindert, Aufrufer %s (P331)", TruheAufrufer(_ReturnAddress()).c_str());
            return;
        }
        Ivk_oCNpc_CloseInventory_Truhe(_this);
    }

    // u268 (P332): schliesst ein anderer Weg das Coop-Fenster beim Rechtsklick (Engine bzw. Mod ausserhalb von HandleEvent), bleibt es
    // offen, und der Klick legt hinueber bzw. nimmt heraus wie oben. Laden, Gespraech und Tod schliessen weiter
    void __fastcall oCItemContainer_Close_Truhe(oCItemContainer* _this, void* vtable);
    CInvoke<void(__thiscall*)(oCItemContainer*)> Ivk_oCItemContainer_Close_Truhe(CoopAddr::oCItemContainer_Close, &oCItemContainer_Close_Truhe,
        COOP_IVK(Save));
    void __fastcall oCItemContainer_Close_Truhe(oCItemContainer* _this, void* vtable) {
        // u269 (P333): Diagnose - wer schliesst das Coop-Fenster?
        if (_this && CoopTruheObj && _this == CoopTruheObj->items && TruheOffenHier && TruheDiagJetzt())
            TruheLog("Coop-Fenster soll schliessen, Aufrufer %s, rechte Maustaste %d (P333)", TruheAufrufer(_ReturnAddress()).c_str(),
                (int)TruheRechtsGedrueckt());
        if (_this && player && zinput && CoopTruheObj && _this == CoopTruheObj->items && TruheOffenHier && !IsLoadingLevel
            && TruheRechtsGedrueckt() && !TruheImGespraech() && !player->IsDead()) {
            TruheRechtsKlick("Schliessen");
            return;
        }
        Ivk_oCItemContainer_Close_Truhe(_this);
    }

    // --- u103 (P125): Gegenstaende im Coop-Inventar zaehlen bei Quests ---
    typedef int(__cdecl* CoopTruheExternal)();
    static CoopTruheExternal TruheOrigHasItems = NULL, TruheOrigRemoveItems = NULL, TruheOrigRemoveItem = NULL;
    static bool TruheExterneVersucht = false;
    static int TruheQuestGenommen = 0;   // so viele Stueck haben Skripte aus dem Coop-Inventar genommen

    // Parameter ansehen, ohne sie vom Stapel zu nehmen (wie CheckInfoPeek, CoopDialogSync.cpp): oben die n int-Parameter, letzter
    // zuerst, je [Wert, 0x40] bzw. [Adresse, 0x41]; darunter die Symbolnummer der Instanz (GetInstance nimmt einen Wert)
    static bool TruheStapel(zCParser* p, int n, oCNpc*& npc, int* w) {
        const int* st = p->datastack.stack;
        const int sp = p->datastack.sptr;
        const int spMax = (int)(sizeof(p->datastack.stack) / sizeof(p->datastack.stack[0]));
        if (sp < 2 * n + 1 || sp > spMax)
            return false;
        for (int i = 0; i < n; i++) {
            const int tok = st[sp - 1 - 2 * i], v = st[sp - 2 - 2 * i];
            if (tok == 0x40)
                w[n - 1 - i] = v;
            else if (tok == 0x41 && v)
                w[n - 1 - i] = *(const int*)v;
            else
                return false;
        }
        const int idx = st[sp - 1 - 2 * n];
        zCPar_Symbol* s = idx > 0 ? p->GetSymbol(idx) : NULL;
        npc = s ? (oCNpc*)s->offset : NULL;
        return true;
    }

    // Aufruf fuer den eigenen Helden aus dem Spiel-Parser, nicht beim Laden (der Inhalt kommt dann erst aus dem Spielstand)
    static bool TruheQuestHier(zCParser* p, int n, int* w) {
        oCNpc* npc = NULL;
        return p && p == parser && player && !IsLoadingLevel && CoopTruheObj && TruheStapel(p, n, npc, w) && npc == player;
    }

    // Menge einer Instanz (Symbolnummer) im Coop-Inventar
    static int TruheMengeIdx(int idx) {
        int n = 0;
        if (CoopTruheObj && idx > 0)
            for (zCListSort<oCItem>* l = CoopTruheObj->containList.next; l; l = l->next)
                if (l->data && l->data->GetInstance() == idx)
                    n += CoopItemAmount(l->data);
        return n;
    }

    // n Stueck aus dem Coop-Inventar nehmen (verbraucht wie im eigenen Inventar) - gemeldet wie jede Aenderung hier. Rueckgabe: genommen
    static int TruheQuestNehmen(int idx, int n, const char* wie) {
        zCPar_Symbol* s = parser && idx > 0 ? parser->GetSymbol(idx) : NULL;
        if (!s || n <= 0)
            return 0;
        const std::string inst = s->name.ToChar();
        std::string name = inst;
        for (zCListSort<oCItem>* l = CoopTruheObj->containList.next; l; l = l->next)
            if (l->data && l->data->GetInstance() == idx) {
                name = l->data->name.ToChar();
                break;
            }
        TruhePruefeHier();   // Aenderungen dieses Bilds zuerst
        const int r = -TruheAendernHier(inst, -n);
        TruhePruefeHier();
        TruheQuestGenommen += r;
        if (r > 0)
            ChatLog(string::Combine(CoopTr("Aus dem Coop-Inventar genommen: %i x %s.", "Taken from the coop inventory: %i x %s."), r,
                string(name.c_str())), zCOLOR(200, 220, 255, 255));
        TruheLog("%s(Held, %s): %d von %d aus dem Coop-Inventar genommen", wie, inst.c_str(), r, n);
        return r;
    }

    // u265 (P322, Nutzer 15.11.: "die Items im Coop-Inventar auch fuer andere Sachen einloesbar ... fuer den Bogen"): was die Engine
    // selbst im eigenen Inventar sucht - Munition der gezogenen Fernkampfwaffe, Schluessel bzw. Dietrich fuer die verschlossene Tuer oder
    // Truhe im Fokus -, legt GT aus dem Coop-Inventar ins eigene, wenn es dort fehlt (Meldung wie TruheQuestNehmen). Skripte (Erz,
    // Quest-Gegenstaende) zaehlen das Coop-Inventar seit u103 (P125) ohnehin mit
    static void TruheInsInventar(int idx, int n, const char* wofuer) {
        if (idx <= 0 || n <= 0 || !player)
            return;
        const int got = TruheQuestNehmen(idx, n, wofuer);
        if (got <= 0)
            return;
        WorldSyncApplying++;
        int rest = got;
        while (rest > 0) {
            oCItem* it = CreateCoopItem(idx);
            if (!it)
                break;
            const bool multi = (it->flags & ITM_FLAG_MULTI) != 0;
            it->amount = multi ? rest : 1;
            rest -= it->amount;
            player->PutInInv(it);
        }
        WorldSyncApplying--;
    }

    static void TruheEinloesen() {
        if (!player || !parser || !CoopTruheObj || player->IsDead() || player->IsUnconscious() || CoopHeroSwapped())
            return;
        // Munition: Bogen links, Armbrust rechts in der Hand (u177), sonst die angelegte Fernkampfwaffe
        const int fm = (int)player->fmode;
        if (fm == 5 || fm == 6) {
            oCItem* w = player->GetSlotItem(zSTRING(fm == 5 ? "ZS_LEFTHAND" : "ZS_RIGHTHAND"));
            if (!w)
                w = player->GetEquippedRangedWeapon();
            const int mun = w ? w->munition : 0;
            if (mun > 0 && !player->IsInInv(mun, 1)) {
                const int n = TruheMengeIdx(mun);
                if (n > 0)
                    TruheInsInventar(mun, n, "Munition");
            }
        }
        // u267 (P327, Nutzer 15.11.: "die Items im Coop inventar immer zugaenglich halten fuer alle moeglichen aktionen"): Mob im Fokus, das
        // einen Gegenstand braucht (Amboss, Kochstelle, Alchemietisch ... - oCMobInter::GetUseWithItem), der nur im Coop-Inventar liegt -
        // ein Stueck ins eigene Inventar
        if (oCMobInter* mi = zDYNAMIC_CAST<oCMobInter>(player->GetFocusVob())) {
            const int uw = mi->GetUseWithItem();
            if (uw > 0 && !player->IsInInv(uw, 1) && TruheMengeIdx(uw) > 0)
                TruheInsInventar(uw, 1, "Werkbank");
        }
        // Schluessel und Dietrich: verschlossene Tuer bzw. Truhe im Fokus
        oCMobLockable* m = zDYNAMIC_CAST<oCMobLockable>(player->GetFocusVob());
        if (!m || !m->locked)
            return;
        if (!m->keyInstance.IsEmpty()) {
            const int k = parser->GetIndex(m->keyInstance);
            if (k > 0 && !player->IsInInv(k, 1) && TruheMengeIdx(k) > 0)
                TruheInsInventar(k, 1, "Schluessel");
        }
        if (!m->pickLockStr.IsEmpty()) {
            static const char* const dietriche[] = { "ITKE_LOCKPICK", "ITKELOCKPICK" };
            for (const char* nm : dietriche) {
                const int d = parser->GetIndex(zSTRING(nm));
                if (d <= 0)
                    continue;
                if (!player->IsInInv(d, 1)) {
                    const int n = TruheMengeIdx(d);
                    if (n > 0)
                        TruheInsInventar(d, n, "Dietrich");
                }
                break;
            }
        }
    }

    // --- u267 (P328, Nutzer 15.11.: "Dann koennen wir ja diese Funktion fuer den Handel hinzufuegen"): beim Handeln liegt der Inhalt des
    // Coop-Inventars voruebergehend im eigenen Inventar (verkaufen, mit Erz bzw. Gold daraus bezahlen); endet der Handel, geht zurueck, was
    // davon noch da ist (hoechstens die geliehene Menge je Instanz, angelegte nie). Beginn: oCViewDialogTrade::SetNpcRight mit dem eigenen
    // Helden; Ende: AllDialogsStop, sicherheitshalber auch das Gespraechsende (CoopTruhePulse); beim Laden verworfen (CoopTruheLoadBegin)
    static CoopTruheBestand TruheHandelGeliehen;
    static bool TruheHandelAktiv = false;

    static void TruheHandelBeginn() {
        if (TruheHandelAktiv || !TruheCoop() || !player || !parser || !CoopTruheObj)
            return;
        TruhePruefeHier();
        const CoopTruheBestand b = TruheBekannt;
        int n = 0;
        for (const auto& e : b) {
            const int idx = parser->GetIndex(zSTRING(e.first.c_str()));
            if (idx <= 0 || e.second <= 0)
                continue;
            const int got = -TruheAendernHier(e.first, -e.second);
            if (got <= 0)
                continue;
            WorldSyncApplying++;
            int rest = got;
            while (rest > 0) {
                oCItem* it = CreateCoopItem(idx);
                if (!it)
                    break;
                it->amount = (it->flags & ITM_FLAG_MULTI) ? rest : 1;
                rest -= it->amount;
                player->PutInInv(it);
            }
            WorldSyncApplying--;
            TruheHandelGeliehen[e.first] += got;
            n += got;
        }
        TruhePruefeHier();
        TruheHandelAktiv = true;
        if (n > 0) {
            ChatLog(string::Combine(CoopTr("Handel: %i Gegenst" "\xE4" "nde aus dem Coop-Inventar liegen bei dir.", "Trade: %i items from the coop inventory are with you."), n),
                zCOLOR(200, 220, 255, 255));
            TruheLog("Handel beginnt: %d Gegenstaende aus dem Coop-Inventar ins eigene (%s) (P328)", n, TruheText(TruheHandelGeliehen).c_str());
        }
    }

    static void TruheHandelEnde(const char* why) {
        if (!TruheHandelAktiv)
            return;
        TruheHandelAktiv = false;
        if (!player || !parser) {
            TruheHandelGeliehen.clear();
            return;
        }
        int n = 0;
        for (const auto& e : TruheHandelGeliehen) {
            const int idx = parser->GetIndex(zSTRING(e.first.c_str()));
            if (idx <= 0)
                continue;
            int moved = 0;
            WorldSyncApplying++;
            while (moved < e.second) {
                oCItem* have = player->IsInInv(idx, 1);
                if (!have || have->HasFlag(ITM_FLAG_ACTIVE))
                    break;
                const int k = (std::min)(e.second - moved, CoopItemAmount(have));
                oCItem* r = player->RemoveFromInv(have, k);
                if (r)
                    ogame->GetGameWorld()->RemoveVob(r);
                moved += k;
            }
            WorldSyncApplying--;
            if (moved > 0) {
                TruheAendernHier(e.first, moved);
                n += moved;
            }
        }
        TruhePruefeHier();
        TruheLog("Handel endet (%s): %d Gegenstaende zurueck ins Coop-Inventar (P328)", why, n);
        if (n > 0)
            ChatLog(string::Combine(CoopTr("Handel beendet: %i Gegenst" "\xE4" "nde zur" "\xFC" "ck ins Coop-Inventar.", "Trade over: %i items back to the coop inventory."), n),
                zCOLOR(200, 220, 255, 255));
        TruheHandelGeliehen.clear();
    }

    void __fastcall oCViewDialogTrade_SetNpcRight_Truhe(oCViewDialogTrade* _this, oCNpc* npc);
    CInvoke<void(__fastcall*)(oCViewDialogTrade*, oCNpc*)> Ivk_oCViewDialogTrade_SetNpcRight_Truhe(CoopAddr::oCViewDialogTrade_SetNpcRight,
        &oCViewDialogTrade_SetNpcRight_Truhe, COOP_IVK(Save));
    void __fastcall oCViewDialogTrade_SetNpcRight_Truhe(oCViewDialogTrade* _this, oCNpc* npc) {
        if (npc && npc == player && !IsLoadingLevel)
            TruheHandelBeginn();   // vor dem Original - das Handelsfenster zeigt das Inventar mit den geliehenen Gegenstaenden
        Ivk_oCViewDialogTrade_SetNpcRight_Truhe(_this, npc);
    }

    void __fastcall oCViewDialogTrade_AllDialogsStop_Truhe(oCViewDialogTrade* _this);
    CInvoke<void(__fastcall*)(oCViewDialogTrade*)> Ivk_oCViewDialogTrade_AllDialogsStop_Truhe(CoopAddr::oCViewDialogTrade_AllDialogsStop,
        &oCViewDialogTrade_AllDialogsStop_Truhe, COOP_IVK(Save));
    void __fastcall oCViewDialogTrade_AllDialogsStop_Truhe(oCViewDialogTrade* _this) {
        Ivk_oCViewDialogTrade_AllDialogsStop_Truhe(_this);
        TruheHandelEnde("Handelsfenster zu");
    }

    // Npc_HasItems(npc, inst): eigener Held - das Coop-Inventar zaehlt mit (Rueckgabe des Originals ersetzt)
    static int __cdecl TruheExtHasItems() {
        zCParser* p = zCParser::GetParser();
        int w[1] = { 0 };
        const bool hier = TruheQuestHier(p, 1, w);
        const int r = TruheOrigHasItems ? TruheOrigHasItems() : 0;
        const int extra = hier ? TruheMengeIdx(w[0]) : 0;
        if (extra > 0) {
            int eigen = 0;
            p->GetParameter(eigen);   // [Wert, 0x40] von SetReturn
            p->SetReturn(eigen + extra);
            static unsigned long lines = 0;
            if (lines++ < 40) {
                zCPar_Symbol* s = p->GetSymbol(w[0]);
                TruheLog("Npc_HasItems(Held, %s): %d eigene + %d im Coop-Inventar", s ? s->name.ToChar() : "?", eigen, extra);
            }
        }
        return r;
    }

    // Npc_RemoveInvItems(npc, inst, n): eigener Held - was im eigenen Inventar fehlt, aus dem Coop-Inventar; wie die Engine nimmt G2
    // hoechstens, was da ist, G1 nur, wenn alles da ist. Das Original nimmt danach nur noch den eigenen Bestand (Menge auf dem Stapel
    // ersetzt) bzw. faellt ohne eigenen Bestand aus (Rueckgabe 1 wie beim Original, wenn etwas da war)
    static int __cdecl TruheExtRemoveItems() {
        zCParser* p = zCParser::GetParser();
        int w[2] = { 0, 0 };
        if (!TruheQuestHier(p, 2, w) || w[1] <= 0)
            return TruheOrigRemoveItems ? TruheOrigRemoveItems() : 0;
        const int inst = w[0], menge = w[1];
        const int eigen = player->inventory2.GetAmount(inst);
        const int coop = eigen < menge ? TruheMengeIdx(inst) : 0;
        int aus = (std::min)(menge - eigen, coop);
#if ENGINE < Engine_G2
        if (eigen + coop < menge)
            aus = 0;
#endif
        if (aus <= 0)
            return TruheOrigRemoveItems ? TruheOrigRemoveItems() : 0;
        TruheQuestNehmen(inst, aus, "Npc_RemoveInvItems");
        if (eigen > 0) {
            int* st = p->datastack.stack;
            const int sp = p->datastack.sptr;
            st[sp - 2] = eigen;   // Menge: nur noch der eigene Bestand
            st[sp - 1] = 0x40;
            return TruheOrigRemoveItems ? TruheOrigRemoveItems() : 0;
        }
        int x = 0;
        p->GetParameter(x);   // wie das Original vom Stapel nehmen (letzter zuerst)
        p->GetParameter(x);
        p->GetInstance();
        p->SetReturn(1);
        return 0;
    }

    // Npc_RemoveInvItem(npc, inst): eigener Held ohne den Gegenstand - einer aus dem Coop-Inventar
    static int __cdecl TruheExtRemoveItem() {
        zCParser* p = zCParser::GetParser();
        int w[1] = { 0 };
        if (!TruheQuestHier(p, 1, w) || player->inventory2.GetAmount(w[0]) > 0 || TruheMengeIdx(w[0]) <= 0)
            return TruheOrigRemoveItem ? TruheOrigRemoveItem() : 0;
        TruheQuestNehmen(w[0], 1, "Npc_RemoveInvItem");
        int x = 0;
        p->GetParameter(x);
        p->GetInstance();
        p->SetReturn(1);
        return 0;
    }

    static void TruheExternUmleiten(const char* name, CoopTruheExternal& orig, CoopTruheExternal mine) {
        zCPar_Symbol* s = parser->GetSymbol(zSTRING(name));
        if (!s || s->type != zPAR_TYPE_FUNC || !(s->flags & zPAR_FLAG_EXTERNAL) || !s->single_intdata) {
            TruheLog("External %s nicht gefunden - dort zaehlt das Coop-Inventar nicht mit", name);
            return;
        }
        if ((CoopTruheExternal)s->single_intdata == mine)
            return;
        orig = (CoopTruheExternal)s->single_intdata;
        s->single_intdata = (int)mine;
        TruheLog("External %s umgeleitet (Original %p) - das Coop-Inventar zaehlt fuer den eigenen Helden mit", name, orig);
    }

    static void TruheExterneUmleiten() {
        if (TruheExterneVersucht || !parser)
            return;
        TruheExterneVersucht = true;
        TruheExternUmleiten("NPC_HASITEMS", TruheOrigHasItems, &TruheExtHasItems);
        TruheExternUmleiten("NPC_REMOVEINVITEMS", TruheOrigRemoveItems, &TruheExtRemoveItems);
        TruheExternUmleiten("NPC_REMOVEINVITEM", TruheOrigRemoveItem, &TruheExtRemoveItem);
    }

    // CoopLook.cpp, CoopSettingsDefaults: Taste vorbelegen (I), damit das COOP-Menue sie schon im Hauptmenue zeigt
    void CoopTruheDefaults() {
        const zSTRING sec("GOTHICCOOP");
        if (zoptions && !zoptions->EntryExists(sec, "coopInvKey"))
            zoptions->WriteInt(sec, "coopInvKey", 1, 0);
        if (zoptions && !zoptions->EntryExists(sec, "coopInvMitInventar"))   // u265 (P321): Vorgabe an
            zoptions->WriteInt(sec, "coopInvMitInventar", 1, 0);
    }

    // Plugin.cpp, Game_Loop (Tasten, nicht im Gespraech): Taste aus den COOP-Einstellungen (je 2 s gelesen) oeffnet bzw. schliesst
    // das Coop-Inventar - einmal je Druck (Flanke von KeyPressed wie die Kamera-Taste, CoopCamera.cpp), nicht mit Shift, nicht in Menues
    // u226 (P264): Nutzer "Coop-Inventar beliebige Tasten" - die Taste kommt aus Einstellungen > COOP > Coop-Tasten (coopKeyInv, jede
    // Taste mit Shift/Strg/Alt; Vorgabe aus coopInvKey). Flanke, Zusatztasten und "nicht in Menues" prueft CoopKeyHit.
    void CoopTruheKeyPulse(bool shift) {
        (void)shift;
        (void)TruheTasteIdx;
        (void)TruheTasteLesenMs;
        (void)TruheTasteWar;
        if (!zinput || !player || IsLoadingLevel)
            return;
        if (!CoopKeyHit(CK_INV))
            return;
        if (TruheOffenHier) {
            // u265 (P321): mit dem Inventar geoeffnet - die Coop-Taste wechselt zwischen eigenem Inventar und Coop-Inventar
            oCMobContainer* fc = TruheObj();
            if (TruheMitInv && fc && fc->items && player->inventory2.IsOpen()) {
                if (fc->items->IsActive())
                    TruheFokusInventar(fc);
                else
                    TruheFokusTruhe(fc);
                TruheLog("Taste %s - jetzt %s", CoopKeyText(CK_INV).c_str(), fc->items->IsActive() ? "im Coop-Inventar" : "im eigenen Inventar");
                return;
            }
            TruheSchliessenHier("Taste");
            return;
        }
        if (TruheAnderesFenster()) {
            TruheLog("Taste %s - ein anderes Fenster ist offen (Truhe, Pluendern, Handel) - nicht geoeffnet", CoopKeyText(CK_INV).c_str());
            return;
        }
        // u293 (P392, Nutzer 19.11.: "Wenn Waffe ausgeruestet und man I drueckt, dann oeffnet das Inventar nicht, nur das Coop-Inventar"):
        // Gothic steckt beim Oeffnen des Inventars erst die Waffe weg (oCNpc::OpenInventory, EV_REMOVEWEAPON); oCMobContainer::Open tut
        // das nicht, mit gezogener Waffe ging nur das Coop-Fenster auf. Also hier vorher wegstecken.
        if (player->GetWeaponMode() != NPC_WEAPON_NONE) {
            const int modus = (int)player->GetWeaponMode();
            player->SetWeaponMode2(NPC_WEAPON_NONE);
            TruheLog("Taste %s: Waffe weggesteckt (Modus %d), damit das eigene Inventar mit aufgeht (P392)", CoopKeyText(CK_INV).c_str(), modus);
        }
        // u266 (P323, Nutzer 15.11.: "dass in dem Modus I genau die selbe Funktion hat"): mit der Einstellung oeffnet die Coop-Taste beides
        // wie Tab (eigenes Inventar vorn, Coop-Inventar daneben); im offenen Inventar wechselt sie wie Tab
        if (TruheCoop() && TruheMitInvAn())
            TruheMitInvWill = true;
        TruheDirekt("Taste");
    }

    void CoopTruheTestHalten(bool an);

    // Plugin.cpp, Game_Loop
    void CoopTruhePulse() {
        if (!player || IsLoadingLevel || !parser)
            return;
        TruheExterneUmleiten();   // u103: einmal, sobald das Spiel laeuft
        const long long now = GetCurrentMs();
        // u268 (P332): Rechtsklick gehalten - nach 0,45 s weiter ein Stueck nach dem anderen (erst alle 120 ms, ab dem zehnten alle 40 ms);
        // losgelassen: der naechste Druck zaehlt wieder
        // u269 (P333): den Druck selbst erkennen - kam kein Ereignis an (Othello), legt der Puls das Stueck hinueber
        const bool rechts = TruheRechtsGedrueckt();
        if (rechts && !TruheRechtsWar) {
            TruheRechtsDruckMs = now;
            TruheRechtsWiederAuf = 0;
            TruheZuVerhindert = 0;
            if (TruheRechtsFrei && TruheOffenHier && player->inventory2.IsOpen())
                TruheRechtsKlick("Maus");
        }
        if (rechts) {
            // u271 (P336): nur bei offenem Coop-Fenster - geht es kurz zu (Othello), oeffnet der Puls es unten wieder
            oCMobContainer* rc = TruheOffenHier ? TruheObj() : NULL;
            if (!TruheRechtsFrei && TruheRechtsWeiterMs && now >= TruheRechtsWeiterMs && rc && rc->items && rc->items->IsOpen()) {
                TruheRechtsZahl++;
                TruheRechtsWeiterMs = TruheRechtsEinStueck(TruheRechtsAusCoop) ? now + (TruheRechtsZahl < 10 ? 120 : 40) : 0;
            }
        }
        else {
            if (TruheRechtsWar && TruheZuVerhindert > 1)
                TruheLog("Inventar schliessen waehrend des Rechtsklicks %d-mal verhindert (P333)", TruheZuVerhindert);
            if (TruheRechtsWar && TruheRechtsWiederAuf > 1)
                TruheLog("Coop-Fenster waehrend des Rechtsklicks %d-mal wieder geoeffnet (P336)", TruheRechtsWiederAuf);
            TruheRechtsFrei = true;
            TruheRechtsWeiterMs = 0;
        }
        TruheRechtsWar = rechts;
        if (now > TruheGebenMs + 2000) {
            TruheGebenMs = now;
            TruheGeben();
        }
        if (zCPar_Symbol* s = parser->GetSymbol(zSTRING(TruheMerker))) {
            int v = 0;
            s->GetValue(v, 0);
            if (v) {
                s->SetValue(0, 0);
                TruheGebenMs = now - 1000;   // verbraucht? in 1 s nachsehen (sobald der Held frei ist)
                if (!TruheOffenHier) {
                    TruheWill = true;
                    TruheWillMs = now;
                }
                TruheLog("benutzt%s", TruheOffenHier ? " - schon offen" : "");
            }
        }
        TruhePruefeHier();
        // u265 (P321): Flanke - Inventar eben aufgegangen
        const bool invJetzt = player->inventory2.IsOpen() != 0;
        const bool invNeu = invJetzt && !TruheInvWarOffen;
        TruheInvWarOffen = invJetzt;
        if (now > TruheEinloesenMs + 300) {   // u265 (P322)
            TruheEinloesenMs = now;
            TruheEinloesen();
            if (TruheHandelAktiv && !TruheImGespraech())   // u267 (P328): Gespraech zu, ohne dass das Handelsfenster es meldete
                TruheHandelEnde("Gespraech zu");
        }
        if (TruheTestHalt && now > TruheTestHaltMs + 15000)
            CoopTruheTestHalten(false);   // Testlauf: hoechstens 15 s
        if (!TruheZuege.empty() && now > TruheSendMs + 3000) {
            TruheZuegeSenden();
            if (TruheSendungen % 10 == 0)
                TruheLog("%u Zuege vom Host noch nicht bestaetigt (%d Sendungen)", (unsigned)TruheZuege.size(), TruheSendungen);
        }
        if (TruheLadenFragen > 0 && now >= TruheLadenFrageMs && TruheCoop() && !ServerThread) {
            TruheLadenFragen--;
            TruheLadenFrageMs = now + 5000;
            json t;
            t["op"] = "frage";
            TruheSenden(t);
        }
        if (TruheOffenHier) {
            oCMobContainer* c = TruheObj();
            const bool open = c && c->items && c->items->IsOpen();
            const bool inv = player->inventory2.IsOpen() != 0;
            if (inv)
                TruheInvGesehen = true;
            // u272 (P337, Nutzer 15.11. zu u271 in Othello: "die Items werden mit linksclick ins inventar getan, dabei soll es ja so sein, dass
            // wir die Items ausruesten koennen", "mit Alt werden die Sachen ins coop inventar getan statt auf den Boden"): Othello (OTHELLO_ABI)
            // umgeht die Haken auf HandleEvent - darum haelt der Puls den Zustand selbst: hat das eigene Inventar den Fokus, ist das
            // Coop-Fenster passiv. Gothics Suche nach dem Fenster daneben (GetNextContainerLeft/Right, auch eingebaut in
            // oCNpcInventory::HandleEvent) ueberspringt passive Fenster - Linksklick und Aktion ruesten aus bzw. benutzen, Alt wirft auf den
            // Boden, Rechtsklick schliesst nichts. Hat das Coop-Fenster den Fokus, ist es normal (Linksklick nimmt heraus)
            if (open && inv && TruheMitInvAn()) {
                if (TruheTabZuCoop) {   // Tab im Inventar (oCNpc_CloseInventory_Truhe)
                    TruheTabZuCoop = false;
                    TruheFokusTruhe(c);
                    TruheLog("Tab - jetzt im Coop-Inventar (P337)");
                }
                const int soll = c->items->IsActive() ? FALSE : TRUE;
                if (c->items->passive != soll)
                    c->items->passive = soll;
            }
            else
                TruheTabZuCoop = false;
            if (!open) {
                // u271 (P336, Nutzer 15.11. zu u270 in Othello: "Sobald man rechtsclick drueckt schliesst sich das Coop Fenster, das Item
                // landet trotzdem dort"): in Othello schliessen die Inventar-Ereignisse der Engine nur das Coop-Fenster, an den Haken vorbei
                // (OTHELLO_ABI.DLL vorn auf HandleEvent und oCItemContainer::Close, ZUTILITIES.DLL in den vtables von Close); das Inventar
                // bleibt offen. Waehrend bzw. kurz nach dem Rechtsklick oeffnet der Puls nur das Fenster wieder, mit dem Fokus wie beim Druck
                if (c && c->items && inv && TruheMitInvAn() && !IsLoadingLevel && !TruheImGespraech() && !player->IsDead()
                    && (TruheRechtsGedrueckt() || (TruheRechtsDruckMs && now - TruheRechtsDruckMs < 500))) {
#if ENGINE >= Engine_G2
                    c->items->Open(0, 0, INV_MODE_CONTAINER);
#else
                    c->items->Open(0, 0, oCItemContainer::FULLSCREEN);   // wie oCMobContainer::Open unter Gothic 1
#endif
                    if (TruheRechtsAusCoop)
                        TruheFokusTruhe(c);
                    else
                        TruheFokusInventar(c);
                    if (++TruheRechtsWiederAuf == 1)
                        TruheLog("Coop-Fenster nach dem Rechtsklick wieder geoeffnet (P336)");
                }
                else {
                    if (TruheDiagJetzt())
                        TruheLog("Coop-Fenster zu: rechte Maustaste %d, seit dem letzten Rechtsklick %lld ms, Inventar offen %d (P333)",
                            (int)TruheRechtsGedrueckt(), TruheRechtsDruckMs ? now - TruheRechtsDruckMs : -1LL, (int)inv);
                    TruheSchliessenHier("Fenster zu");
                }
            }
            else if (TruheInvGesehen && !inv)
                TruheSchliessenHier("Inventar zu");
            else if (TruheTestZuMs && now >= TruheTestZuMs)
                TruheSchliessenHier("Testlauf");
            else if (player->IsDead() || player->IsUnconscious())
                TruheSchliessenHier("Held tot bzw. bewusstlos");
            return;
        }
        // u265 (P321, Nutzer 15.11.: "das Coop-Inventar sich immer mitoeffnet wenn man das normale Inventar oeffnet"): geht das Inventar
        // auf (nicht beim Pluendern, Handeln, Stehlen, an Truhen der Welt, im Gespraech), schliesst GT es und oeffnet im naechsten Bild
        // beide wie mit der Coop-Taste - das Coop-Fenster dann passiv daneben (TruheOeffnenHier)
        if (invNeu && !TruheWill && TruheCoop() && TruheMitInvAn() && !TruheAnderesFenster() && !TruheImGespraech()) {
            TruheMitInvWill = true;
            TruheWill = true;
            TruheWillMs = now;
        }
        if (TruheWill) {   // erst, wenn der Held frei ist (Koerperzustand normal, Inventar zu), hoechstens 5 s warten
            if (now > TruheWillMs + 5000 || player->IsDead() || player->IsUnconscious()) {
                TruheWill = false;
                TruheMitInvWill = false;
                TruheTestZuNach = 0;
                TruheLog("Held 5 s beschaeftigt bzw. ausser Gefecht - nicht geoeffnet (Koerperzustand %d)", player->GetBodyState());
            }
            else if (TruheAnderesFenster()) {   // u101: nicht mitten in eine Truhe, ein Pluendern oder einen Handel
                TruheWill = false;
                TruheMitInvWill = false;
                TruheTestZuNach = 0;
                TruheLog("ein anderes Fenster ist offen (Truhe, Pluendern, Handel) - nicht geoeffnet");
            }
            else if (player->inventory2.IsOpen())
                player->CloseInventory();   // aus dem Inventar benutzt und es blieb offen - erst zu, im naechsten Bild oeffnen
            else if (player->GetBodyState() <= BS_SPRINT && !player->interactItem) {
                TruheWill = false;
                TruheOeffnenHier();
            }
        }
    }

    // Plugin.cpp, LoadBegin: offen beim Laden bzw. Weltwechsel - schliessen
    void CoopTruheLoadBegin() {
        TruheHandelAktiv = false;   // u267 (P328): der Stand kommt aus dem Spielstand - Geliehenes nicht zurueckbuchen
        TruheHandelGeliehen.clear();
        if (TruheOffenHier)
            TruheSchliessenHier("Laden");
        TruheWill = false;
        TruheLadenFragen = 0;
        TruheTestZuNach = 0;
        TruheTestHalt = false;
        TruheTestHaltStand = json();
    }

    // neuer Stand fuer den Abgleich (Laden, neues Spiel): offene Zuege, Zaehler und Offen-Meldungen gelten nicht mehr
    static void TruheAbgleichNeu() {
        TruheZuege.clear();
        TruheSeq = 0;
        TruheEpoche = GetCurrentMs();
        TruheSendungen = 0;
        TruheGaeste.clear();
        TruheOffenBei.clear();
        TruheAndereOffen.clear();
    }

    // UnionHooks.cpp, Game_LoadBegin_NewGame: ein neues Spiel beginnt mit leerer Truhe
    void CoopTruheNeuesSpiel() {
        TruheSetzen(CoopTruheBestand());
        TruheAbgleichNeu();
        TruheLog("neues Spiel - leer");
    }

    // Plugin.cpp, LoadEnd (spielstand = false) bzw. Game_LoadEnd_SaveGame (true): Inhalt aus dem geladenen Spielstand; beim
    // Weltwechsel bleibt er, wie er ist
    void CoopTruheLoadEnd(bool spielstand) {
        TruheGebenMs = GetCurrentMs() + 1000;   // erst nach dem Wiederherstellen des eigenen Helden pruefen (CoopSaveSync.cpp)
        if (!spielstand)
            return;
        const int slot = UnionCore::SaveLoadGameInfo.slotID;
        CoopTruheBestand b;
        bool datei = false;
        if (slot >= 0)
            if (FILE* f = fopen((CoopSlotDir(slot) + "COOPTRUHE.TXT").c_str(), "r")) {
                datei = true;
                char line[512];
                while (fgets(line, sizeof(line), f)) {
                    char inst[128];
                    int n = 0;
                    if (sscanf(line, "item %127s %d", inst, &n) == 2 && n > 0 && n <= 100000)
                        b[inst] += n;
                }
                fclose(f);
            }
        TruheSetzen(b);
        TruheAbgleichNeu();
        if (!TruheCoop() || ServerThread) {
            TruheVerteilen();
            TruheLog("aus Slot %d geladen: %d Gegenstaende%s", slot, TruheAnzahl(b), datei ? "" : " (keine Datei - leer)");
            return;
        }
        TruheLadenFragen = 4;   // der Host fuehrt - seinen Stand holen; bis dahin die eigene Kopie
        TruheLadenFrageMs = GetCurrentMs() + 2000;
        TruheLog("aus Slot %d: %d Gegenstaende (eigene Kopie%s) - frage den Host", slot, TruheAnzahl(b), datei ? "" : ", keine Datei");
    }

    // Plugin.cpp, Game_SaveEnd: Inhalt in den Spielstand-Ordner (beide PCs - der Client mit seiner Kopie des Host-Stands)
    void CoopTruheSaveEnd() {
        const int slot = UnionCore::SaveLoadGameInfo.slotID;
        if (slot < 0 || IsLoadingLevel)
            return;   // Weltwechsel (current)
        std::string s = "// Coop-Truhe (GothicCoop " COOP_FASSUNG "): Inhalt beim Speichern\n";
        for (const auto& e : TruheLesen())
            s += "item " + e.first + " " + std::to_string(e.second) + "\n";
        s += "ende\n";
        if (!CoopWriteTextAtomic(CoopSlotDir(slot) + "COOPTRUHE.TXT", s))
            TruheLog("COOPTRUHE.TXT in Slot %d nicht schreibbar", slot);
    }

    // PacketProcessor.cpp: Verbindung weg - Host vergisst seine Zuege und dass er sie offen hatte
    void CoopTruhePeerGone(const std::string& who) {
        if (!ServerThread || !TruheGaeste.count(who))
            return;
        const bool offen = TruheGaeste[who].offen;
        TruheGaeste.erase(who);
        TruheVerteilen();
        TruheAndereMelden(TruheOffenListe());
        if (offen)
            TruheLog("%s getrennt, waehrend die Truhe bei ihm offen war", who.c_str());
    }

    // --- Testlauf (CoopSelfTest.cpp, P119/P121) ---
    void CoopTruheTestStand(int& anzahl, std::string& offen, const char* inst) {
        const CoopTruheBestand b = TruheLesen();
        auto f = b.find(inst ? inst : "");
        anzahl = f == b.end() ? 0 : f->second;
        offen = (!TruheCoop() || ServerThread) ? TruheOffenListe() : TruheOffenBei;
    }

    bool CoopTruheTestOffenHier() {
        return TruheOffenHier;
    }

    int CoopTruheTestRueckgaben() {
        return TruheRueckgaben;
    }

    // inst um delta aendern wie beim Rueberziehen (auch bei offenem Fenster) - gemeldet wie jede Aenderung hier
    bool CoopTruheTestAendern(const char* inst, int delta) {
        if (!TruheObj() || !inst || !delta)
            return false;
        TruhePruefeHier();
        const int r = TruheAendernHier(inst, delta);
        TruhePruefeHier();
        return r == delta;
    }

    // n Stueck herausnehmen wie beim Rueberziehen - ins Inventar oder (insInventar = false) verbraucht. Rueckgabe: genommen
    int CoopTruheTestNimm(const char* inst, int n, bool insInventar) {
        if (!TruheObj() || !inst || n <= 0 || !parser)
            return 0;
        TruhePruefeHier();
        const int r = -TruheAendernHier(inst, -n);
        if (r > 0 && insInventar && player) {
            const int idx = parser->GetIndex(zSTRING(inst));
            int left = r;
            while (idx >= 0 && left > 0) {
                oCItem* it = CreateCoopItem(idx);
                if (!it)
                    break;
                it->amount = (it->flags & ITM_FLAG_MULTI) ? left : 1;
                left -= it->amount;
                player->PutInInv(it);   // wie ein aus der Truhe genommener Gegenstand
            }
        }
        TruhePruefeHier();
        return r;
    }

    // das Fenster hier oeffnen (wie Benutzen, ohne den Gegenstand) bzw. offen lassen; GT schliesst es nach ms selbst
    bool CoopTruheTestFenster(int ms) {
        if (!player)
            return false;
        if (TruheOffenHier) {
            TruheTestZuMs = GetCurrentMs() + ms;
            return true;
        }
        TruheTestZuNach = ms;
        TruheOeffnenHier();
        return TruheOffenHier;
    }

    void CoopTruheTestZu() {
        if (TruheOffenHier)
            TruheSchliessenHier("Testlauf");
    }

    // Client: Staende des Hosts zurueckhalten bzw. den zurueckgehaltenen jetzt anwenden
    void CoopTruheTestHalten(bool an) {
        if (an) {
            TruheTestHalt = !ServerThread && TruheCoop();
            TruheTestHaltMs = GetCurrentMs();
            TruheTestHaltStand = json();
            return;
        }
        if (!TruheTestHalt)
            return;
        TruheTestHalt = false;
        if (TruheTestHaltStand.is_object()) {
            const json t = TruheTestHaltStand;
            TruheTestHaltStand = json();
            TruheStandAnwenden(t);
        }
    }

    // den Gegenstand benutzen wie aus dem Inventar: u101 dieselbe Nachricht wie oCNpcInventory::HandleEvent (EV_USEITEMTOSTATE mit
    // dem Gegenstand, Zustand + 1; bis u100 EV_USEITEM mit Namen wie AI_UseItem); das Fenster, das darauf aufgeht, schliesst GT nach
    // zuNachMs selbst
    bool CoopTruheTestBenutzen(int zuNachMs) {
        if (!player || !parser || TruheOffenHier || TruheWill)
            return false;
        const int idx = parser->GetIndex(zSTRING(TruheInst));
        oCItem* it = idx >= 0 ? player->IsInInv(idx, 1) : NULL;
        if (!it)
            return false;
        TruheTestZuNach = zuNachMs;
        player->GetEM(false)->OnMessage(new oCMsgManipulate(oCMsgManipulate::EV_USEITEMTOSTATE, it, player->interactItemCurrentState + 1), player);
        return true;
    }

    int CoopTruheTestOeffnungen() {
        return TruheTestOeffnungen;
    }

    int CoopTruheTestDirekt() {
        return TruheDirektZahl;
    }

    int CoopTruheTestGegeben() {
        return TruheGegebenZahl;
    }

    std::string CoopTruheTestTitel() {
        return TruheTestTitel;
    }

    // u103 (P125): alle drei Skript-Funktionen umgeleitet
    bool CoopTruheTestQuestBereit() {
        return TruheOrigHasItems && TruheOrigRemoveItems && TruheOrigRemoveItem;
    }

    int CoopTruheTestQuestGenommen() {
        return TruheQuestGenommen;
    }

    // u270 (P335, Nutzer 15.11. zu u269 in Othello: "jetzt landet das Item im Coop inventar aber beide inventare schliessen sich"):
    // Gothic schliesst eine Truhe ueber oCMobContainer::Close (Inventar zu, Fenster geloescht) - darum griff der Haken auf
    // oCItemContainer::Close nie. Die Coop-Truhe bleibt beim Rechtsklick offen; Laden, Gespraech, Tod und GTs eigenes Schliessen
    // gehen durch. Aufruf aus oCMobContainer_Close_Coop (CoopWorldSync.cpp); true = nicht schliessen
    bool CoopTruheCloseVerhindern(oCMobContainer* m, oCNpc* npc, const void* ret) {
        if (!m || m != CoopTruheObj || TruheSchliesstSelbst || !player || npc != player || !TruheOffenHier || IsLoadingLevel)
            return false;
        if (!TruheMitInvAn() || !TruheRechtsGedrueckt() || TruheImGespraech() || player->IsDead())
            return false;
        if (++TruheZuVerhindert == 1)
            TruheLog("Coop-Truhe schliessen bei Rechtsklick verhindert, Aufrufer %s (P335)", TruheAufrufer(ret).c_str());
        return true;
    }

    // --- u266 (P324): Quest-Gegenstaende im Coop-Inventar (Nutzer 15.11.: "vielleicht waere es sinnvoll wenn QuestItems immer im
    // Coop-Inventar liegen und im normalen Inventar unsichtbar sind dann braucht man dieses gemeinsam auch nicht mehr", dann "der Brief ist
    // noch nicht im Coop-Inventar gelandet"). Mit der Einstellung "Coop-Inventar mit dem Inventar" legt GT jeden Missions-Gegenstand, den
    // der eigene Held neu bekommt, ins Coop-Inventar (CoopDialogSync.cpp, CoopMissionPulse) statt ihn dem Partner zu spiegeln (u96). Was
    // man selbst herausnimmt (Lesen, Anlegen, Einloesen), bleibt im eigenen Inventar
    bool CoopTruheQuestModus() {
        return TruheCoop() && TruheMitInvAn() && TruheObj() != NULL && GetCurrentMs() >= CoopTestMiOhneTruheBis;   // u275 (P346)
    }

    void CoopTruheJetztPruefen() {
        TruhePruefeHier();
    }

    // so viel von inst wurde in den letzten 10 s hier selbst herausgenommen - bis zu n davon verbrauchen (Rueckgabe: verbraucht)
    int CoopTruheSelbstGenommen(const char* inst, int n) {
        if (!inst || n <= 0)
            return 0;
        auto f = TruheSelbstGenommen.find(inst);
        if (f == TruheSelbstGenommen.end() || GetCurrentMs() > f->second.ms + 10000)
            return 0;
        const int k = (std::min)(n, f->second.menge);
        f->second.menge -= k;
        return k;
    }

    // bis zu n Stueck von inst aus dem eigenen Inventar ins Coop-Inventar (angelegte und die Coop-Truhe nie). Rueckgabe: verschoben
    int CoopTruheQuestHinein(const char* inst, int n) {
        oCMobContainer* c = TruheObj();
        if (!c || !player || !parser || !inst || n <= 0 || !strcmp(inst, TruheInst))
            return 0;
        const int idx = parser->GetIndex(zSTRING(inst));
        if (idx <= 0)
            return 0;
        TruhePruefeHier();
        std::string name;
        int moved = 0;
        WorldSyncApplying++;
        while (moved < n) {
            oCItem* have = player->IsInInv(idx, 1);
            if (!have || have->HasFlag(ITM_FLAG_ACTIVE))
                break;
            if (name.empty())
                name = have->name.ToChar();
            const int k = (std::min)(n - moved, CoopItemAmount(have));
            oCItem* r = player->RemoveFromInv(have, k);
            if (r)
                ogame->GetGameWorld()->RemoveVob(r);
            moved += k;
        }
        WorldSyncApplying--;
        if (moved <= 0)
            return 0;
        TruheAendernHier(inst, moved);
        TruhePruefeHier();   // meldet es wie jede Aenderung hier
        ChatLog(string::Combine(CoopTr("Quest-Gegenstand ins Coop-Inventar: %i x %s.", "Quest item moved to the coop inventory: %i x %s."),
            moved, string(name.c_str())), zCOLOR(200, 220, 255, 255));
        TruheLog("Quest-Gegenstand %s: %d ins Coop-Inventar (P324)", inst, moved);
        return moved;
    }

#else   // Modul in dieser Engine nicht freigegeben
    void CoopTruheReceive(const json&, const std::string&) {}
    bool CoopTruheQuestModus() { return false; }   // u266 (P324)
    bool CoopTruheCloseVerhindern(oCMobContainer*, oCNpc*, const void*) { return false; }   // u270 (P335)
    void CoopTruheJetztPruefen() {}
    int CoopTruheSelbstGenommen(const char*, int) { return 0; }
    int CoopTruheQuestHinein(const char*, int) { return 0; }
    void CoopTruhePulse() {}
    void CoopTruheLoadBegin() {}
    void CoopTruheNeuesSpiel() {}
    void CoopTruheLoadEnd(bool) {}
    void CoopTruheSaveEnd() {}
    void CoopTruhePeerGone(const std::string&) {}
    void CoopTruheTestStand(int& anzahl, std::string& offen, const char*) { anzahl = -1; offen.clear(); }
    bool CoopTruheTestOffenHier() { return false; }
    int CoopTruheTestRueckgaben() { return 0; }
    bool CoopTruheTestAendern(const char*, int) { return false; }
    int CoopTruheTestNimm(const char*, int, bool) { return 0; }
    bool CoopTruheTestFenster(int) { return false; }
    void CoopTruheTestZu() {}
    void CoopTruheTestHalten(bool) {}
    bool CoopTruheTestBenutzen(int) { return false; }
    int CoopTruheTestOeffnungen() { return 0; }
    int CoopTruheTestDirekt() { return 0; }
    int CoopTruheTestGegeben() { return 0; }
    std::string CoopTruheTestTitel() { return "-"; }
    bool CoopTruheTestQuestBereit() { return false; }
    int CoopTruheTestQuestGenommen() { return 0; }
    void CoopTruheDefaults() {}
    void CoopTruheKeyPulse(bool) {}
#endif
}
