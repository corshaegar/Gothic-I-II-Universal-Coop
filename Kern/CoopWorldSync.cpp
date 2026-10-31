// Supported with union (c) 2020 Union team
// Union SOURCE file

#include <intrin.h>   // _ReturnAddress
#include <set>

namespace GOTHIC_ENGINE {

    // --- Welt-Abgleich: Truhen und Gegenstaende (v13, 18.10.2026) -------------------------------
    //
    // Bis v12 gleicht GT nur NPCs ab; Truhen und Gegenstaende am Boden laufen auf jedem PC fuer sich.
    // Regel jetzt: der Host fuehrt den Bestand. Wer etwas tut, meldet es dem Host; der Host aendert
    // seine Welt und verteilt das Ergebnis an alle.
    //
    // Truhen (oCMobContainer):
    //   Oeffnen (Open 0x006843D0): Inhalt merken, SYNC_CHEST_OPEN an alle = Sperre; bei den anderen
    //   verweigert oCMobLockable::CanInteractWith (0x00681FC0) dann das Benutzen.
    //   Schliessen (Close 0x00684490): Client -> SYNC_CHEST_CLOSE mit der Differenz an den Host; der
    //   wendet sie an und schickt den vollen Inhalt als SYNC_CHEST_STATE an alle. Schliesst der Host
    //   selbst, schickt er gleich STATE. Das Bedienfeld arbeitet direkt auf containList (Open ruft
    //   items->vfunc+28(&containList)), daher genuegt der Vergleich Oeffnen/Schliessen - das deckt
    //   auch "alles nehmen" u. ae. von zUtilities ab.
    // Gegenstaende am Boden (v14):
    //   Test v13: weder normales Aufheben noch Quickloot kam an. Quickloot ist ein eigenes Plugin
    //   (UNION_QUICKLOOT.DLL in ScriptsOthello4.0_de.vdf) und ruft kein DoTakeVob, sondern legt direkt
    //   ins Inventar (oCNpcInventory::Insert). Darum jetzt unabhaengig vom Weg: ein freier Gegenstand
    //   verlaesst die Welt (oCWorld::RemoveVob 0x006D6EF0, GT-Hook in Hooks.cpp) und landet kurz darauf
    //   im Inventar eines NPCs (oCNpcInventory::Insert 0x0066C7D0 - der vtable-Platz gehoert zUnionUtils,
    //   das das Original weiter aufruft; dazu oCNpc::PutInInv 0x006A4FF0 als zweite Stelle).
    //   Spieler -> Client: SYNC_ITEM_TAKE an den Host, der seinen Gegenstand entfernt und SYNC_ITEM_GONE
    //   an alle schickt; Host: gleich SYNC_ITEM_GONE. Aufnahmen durch NPCs auf dem Host ebenso (die
    //   NPCs des Clients sind Abbilder). Von Spielern fallen gelassene Gegenstaende (RX_DROPPED_ITEM_*):
    //   das Fallenlassen bleibt beim alten GT-Weg (SYNC_DROPITEM), das Aufheben laeuft seit v18 hier mit,
    //   zugeordnet ueber den eindeutigen Objektnamen (Feld "o"), weil die Kopie beim anderen PC woanders
    //   liegt. Der alte GT-Weg (SYNC_TAKEITEM, nur ueber DoTakeVob und die Coop-Figur) griff bei Quickloot nie.
    //   Truhen ohne Oeffnen (Quickloot): Insert/Remove der Truhe (0x00683E80/EB0/F40) merken den
    //   Inhalt vorher; im naechsten Frame wird die Differenz wie beim Schliessen gemeldet.
    // Abgleich beim Verbinden und nach Weltwechsel:
    //   Client -> SYNC_WORLD_REQUEST; der Host schickt alle Truhen (SYNC_CHEST_BULK) und alle
    //   Gegenstaende am Boden (SYNC_ITEM_BULK). Der Client entfernt, was der Host nicht mehr hat
    //   (nur entfernen; weicht mehr als ein Viertel ab, Abbruch - vermutlich anderer Spielstand).
    //
    // Zuordnung ueber Weltdatei + Position: Truhen stehen fest, Gegenstaende aus der .zen liegen auf
    // beiden PCs an derselben Stelle. Alle benutzten Engine-Funktionen sind unter Othello unveraendert
    // (Speicherabbild gegen GothicMod.exe; auch vtable-Plaetze und virtuelle Aufrufstellen von
    // Open/Close/CanInteractWith/Insert/Remove geprueft). Ausgabe: system\GothicCoopOthello.log.

    // u12: Zustaende und Hilfsfunktionen bis "Senden" ohne Haken - in allen Engines gebaut, der Testlauf (CoopSelfTest.cpp)
    // braucht sie auch, wenn das Modul (G2A) noch aus ist. Ab "Senden" gilt die Freigabe (COOP_BUILD(World)).

    typedef std::map<std::string, int> CoopBestand;   // Instanzname -> Gesamtmenge

    struct CoopChestLock {
        std::string owner;
        long long since;
    };

    struct CoopItemBulk {
        int expected = -1;
        int received = 0;
        std::map<std::string, std::vector<zVEC3>> host;
    };

    static std::map<std::string, CoopChestLock> ChestLocks;              // Positionsschluessel -> Sperre
    static std::map<oCMobContainer*, CoopBestand> ChestOpenSnapshot;     // lokal geoeffnete Truhen
    static std::map<oCMobContainer*, std::vector<std::string>> ChestPendingDiffs;   // Host: waehrend er selbst offen hat
    static CoopItemBulk ItemBulk;
    static long long WorldLoadedMs = 0;
    static bool WorldRequestSent = false;
    static long long WorldBulkAtMs = 0;
    static long long ChestBusyMsgMs = 0;
    static unsigned long WorldSyncLines = 0;

    // v14: Gegenstand verlaesst die Welt -> landet er gleich in einem Inventar, ist er aufgehoben.
    struct CoopWorldGone {
        std::string inst;
        zVEC3 pos;
        long long ms;
        int amount = 1;
        std::string obj;   // Objektname, nur bei fallen gelassenen (RX_DROPPED_ITEM_*)
    };
    static std::map<oCItem*, CoopWorldGone> RecentWorldRemovals;   // je mit AddRef gehalten
    static std::map<oCItem*, long long> ReportedStillInWorld;      // schon gemeldet, Entfernen folgt noch (AddRef)
    // u70 (Dauertest u67: "HOST nahm ITPL_MUSHROOM_01 bei 31335_3889_-20391 - hier nicht gefunden", 30 min spaeter nahm der Client
    // genau dort einen Pilz, den es beim Host nicht mehr gab - doppelt): Entfernungen, die hier nicht gleich gingen (inst, Lage,
    // bis wann) - der Gegenstand kann spaeter auftauchen (eben geladen, noch nicht "frei"). Alle 2 s neu suchen, 2 min lang;
    // Laden leert die Liste (danach gleicht der Weltbestand ab)
    struct CoopPendingGone {
        std::string inst, by;
        zVEC3 pos;
        long long sinceMs = 0;
    };
    static std::vector<CoopPendingGone> PendingGone;
    static long long PendingGoneCheckMs = 0;
    static std::map<oCMobContainer*, CoopBestand> ChestDirty;      // Truhe ohne Oeffnen veraendert: Stand vorher
    static int ChestHookDepth = 0;
    static int WorldSyncApplying = 0;                              // > 0: eigene Aenderung, nicht melden
    static oCMobContainer* CoopTruheObj = NULL;                    // u97: gemeinsame Coop-Truhe (CoopTruhe.cpp) - keine Welt-Truhe
    static unsigned long WorldSyncDiagLines = 0;

    // v15: Pluendern toter/bewusstloser NPCs. Gezaehlt wird, was in diesem Frame aus ihrem Inventar
    // genommen wurde UND im Inventar des Spielers ankam (so zaehlen Skript-Aenderungen nicht mit).
    static std::map<std::string, CoopBestand> LootRemovals;        // GT-Name des NPCs -> Instanz -> Menge
    static CoopBestand PlayerReceived;                             // Instanz -> Menge (nicht aus der Welt)
    static int NpcInvHookDepth = 0;

    static void WorldSyncLog(const char* fmt, ...) {
        if (WorldSyncLines >= 800)
            return;
        WorldSyncLines++;
        char buf[1024];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        buf[sizeof(buf) - 1] = 0;
        OthelloCompatLog("[%lld s, %s] Welt-Sync: %s", DiagSeconds(), DiagRole(), buf);
    }

    // Diagnose fuer den naechsten Test: welcher Weg nimmt Gegenstaende (begrenzt auf 120 Zeilen).
    static void WorldSyncDiag(const char* fmt, ...) {
        if (WorldSyncDiagLines >= 120)
            return;
        WorldSyncDiagLines++;
        char buf[512];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        buf[sizeof(buf) - 1] = 0;
        OthelloCompatLog("[%lld s, %s] Welt-Sync Diagnose: %s", DiagSeconds(), DiagRole(), buf);
    }

    // CoopModuleOf: u8 nach CoopCore.cpp (auch CoopMenuInject.cpp braucht es, wenn das Welt-Modul in einer Engine aus ist)

    bool CoopRuleWorld();   // v75: CoopDifficulty.cpp - Abgleich "Gegenstaende/Truhen" (Regel des Hosts)

    static bool WorldSyncBase() {
        return (ServerThread || ClientThread) && !IsCoopPaused && !IsLoadingLevel && !WorldEditMode
            && ogame && ogame->GetGameWorld() && player && !(MyselfId == "_player_");
    }

    // v75: "Gegenstaende/Truhen aus" (Einstellungen -> COOP -> Abgleich, Host gibt vor) schaltet alles hier ab - jeder hat seine
    // eigenen Truhen und Gegenstaende am Boden.
    static bool WorldSyncAktiv() {
        return WorldSyncBase() && CoopRuleWorld();
    }

    // CoopWorldName: u11 in CoopCore.cpp

    static long CoopRound(float v) {
        return (long)floor(v + 0.5f);
    }

    static std::string CoopPosKey(const zVEC3& p) {
        char b[64];
        _snprintf(b, sizeof(b), "%ld_%ld_%ld", CoopRound(p.n[0]), CoopRound(p.n[1]), CoopRound(p.n[2]));
        b[sizeof(b) - 1] = 0;
        return b;
    }

    static json CoopPosJson(const zVEC3& p) {
        return json::array({ (int)CoopRound(p.n[0]), (int)CoopRound(p.n[1]), (int)CoopRound(p.n[2]) });
    }

    static bool CoopPosFromJson(const json& j, zVEC3& p) {
        if (!j.is_array() || j.size() != 3 || !j[0].is_number() || !j[1].is_number() || !j[2].is_number())
            return false;
        p = zVEC3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
        return true;
    }

    static float CoopDist(const zVEC3& a, const zVEC3& b) {
        const float dx = a.n[0] - b.n[0], dy = a.n[1] - b.n[1], dz = a.n[2] - b.n[2];
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }

    static std::string CoopItemInst(oCItem* it) {
        return std::string(it->GetInstanceName().ToChar());
    }

    static int CoopItemAmount(oCItem* it) {
        return it->amount > 0 ? it->amount : 1;
    }

    // Haelt ein NPC den Gegenstand in einem Slot (Hand, Mund, Fackel ...)? Solche Gegenstaende haengen
    // im Vob-Baum NICHT unter dem NPC (Test v14: Essen/Bier der NPCs galt als "frei"). Massgeblich ist
    // TNpcSlot::vob (+30h) in oCNpc::invSlot (+9E8h); RemoveFromSlot (0x006A5F50) ruft DisableVob,
    // solange der Slot noch auf den Gegenstand zeigt (0x006A6133).
    static void CoopCollectSlotVobs(std::set<zCVob*>& out) {
        for (zCListSort<oCNpc>* l = CoopWorldNpcListFirst(ogame->GetGameWorld()); l; l = l->next) {
            oCNpc* npc = l->data;
            if (!npc)
                continue;
            for (int i = 0; i < npc->invSlot.GetNumInList(); i++) {
                TNpcSlot* s = npc->invSlot.GetSafe(i);
                if (s && s->vob)
                    out.insert(s->vob);
            }
        }
    }

    static bool CoopItemInNpcSlot(oCItem* it) {
        for (zCListSort<oCNpc>* l = CoopWorldNpcListFirst(ogame->GetGameWorld()); l; l = l->next) {
            oCNpc* npc = l->data;
            if (!npc)
                continue;
            for (int i = 0; i < npc->invSlot.GetNumInList(); i++) {
                TNpcSlot* s = npc->invSlot.GetSafe(i);
                if (s && s->vob == it)
                    return true;
            }
        }
        return false;
    }

    static bool CoopItemIsDropped(oCItem* it) {
        return it && it->GetObjectName().StartWith("RX_DROPPED_ITEM_");
    }

    // Liegt der Gegenstand frei in der Welt? Nicht: ausserhalb der Welt, in einem NPC-Slot oder als Kind
    // eines NPCs im Vob-Baum. slots: vorab gesammelt (Bestand/Abgleich), sonst wird die NPC-Liste durchsucht.
    // allowDropped: von Spielern fallen gelassene (RX_DROPPED_ITEM_<Zufall>, GT benennt sie beim Fallenlassen,
    // der andere PC legt eine Kopie gleichen Namens bei der Coop-Figur ab) zaehlen mit - nur beim Aufheben;
    // im Bestand/Abgleich nicht, denn ihre Lage ist auf beiden PCs verschieden (v18).
    static bool CoopItemLiegtFrei(oCItem* it, const std::set<zCVob*>* slots = NULL, bool allowDropped = false) {
        if (!it || !it->GetHomeWorld())
            return false;
        if (!allowDropped && CoopItemIsDropped(it))
            return false;
        zCTree<zCVob>* node = it->globalVobTreeNode;
        zCTree<zCVob>* parent = node ? node->GetParent() : NULL;
        zCVob* parentVob = parent ? parent->GetData() : NULL;
        if (parentVob && zDYNAMIC_CAST<oCNpc>(parentVob))
            return false;
        if (slots ? slots->count(it) != 0 : CoopItemInNpcSlot(it))
            return false;
        return true;
    }

    // --- Bestand einer Truhe ------------------------------------------------------------------

    static CoopBestand ChestBestand(oCMobContainer* c) {
        CoopBestand b;
        for (zCListSort<oCItem>* n = c->containList.next; n; n = n->next) {
            if (n->data)
                b[CoopItemInst(n->data)] += CoopItemAmount(n->data);
        }
        return b;
    }

    static std::string BestandText(const CoopBestand& b) {
        std::string s;
        char num[16];
        for (auto const& kv : b) {
            if (kv.second == 0)
                continue;
            _snprintf(num, sizeof(num), "%d", kv.second);
            num[sizeof(num) - 1] = 0;
            s += kv.first;
            s += ':';
            s += num;
            s += ';';
        }
        return s;
    }

    static CoopBestand BestandAusText(const std::string& s) {
        CoopBestand b;
        size_t start = 0;
        while (start < s.size()) {
            size_t end = s.find(';', start);
            if (end == std::string::npos)
                end = s.size();
            const std::string part = s.substr(start, end - start);
            const size_t colon = part.rfind(':');
            if (colon != std::string::npos && colon > 0)
                b[part.substr(0, colon)] += atoi(part.c_str() + colon + 1);
            start = end + 1;
        }
        return b;
    }

    static std::string DiffText(const CoopBestand& vorher, const CoopBestand& nachher) {
        CoopBestand d;
        for (auto const& kv : nachher) {
            auto f = vorher.find(kv.first);
            const int alt = f == vorher.end() ? 0 : f->second;
            if (kv.second != alt)
                d[kv.first] = kv.second - alt;
        }
        for (auto const& kv : vorher) {
            if (!nachher.count(kv.first))
                d[kv.first] = -kv.second;
        }
        return BestandText(d);
    }

    // u98 (P120): Gegenstand aus einer Truhe nehmen und freigeben. Nicht oCMobContainer::Remove (G2A 0x00725FF0, G1 0x00683EB0):
    // das gibt ihn zwar frei, legt ihn aber vorher ueber oCWorld::InsertVobInWorld in die Listen der Welt (voblist und
    // voblist_items, je mit eigenem Verweis) - wer ihn nicht gleich in ein Inventar oder die Welt legt, laesst ihn dort liegen, bis
    // die Welt verworfen wird (Befund u97, Disassembly beider Engines). Hier nur aus containList (der Knoten wie in Remove ueber
    // Gothics Speicher-Pool) und den Verweis der Truhe frei - die Haken auf Remove sehen das nicht. Allgemein (G1/G2).
    static void CoopChestDropItem(oCMobContainer* c, oCItem* it) {
        c->containList.Remove(it);
        it->Release();
    }

    // Menge einer Instanz in der Truhe um delta aendern. Rueckgabe: tatsaechlich geaenderte Menge.
    // Insert nimmt eine Referenz, CoopChestDropItem gibt sie frei (u98: bis u97 Remove - siehe dort).
    static int ChestAdjustRaw(oCMobContainer* c, const std::string& inst, int delta);
    static int ChestAdjust(oCMobContainer* c, const std::string& inst, int delta) {
        WorldSyncApplying++;   // die eigenen Insert/Remove nicht als Quickloot melden
        const int r = ChestAdjustRaw(c, inst, delta);
        WorldSyncApplying--;
        return r;
    }

    static int ChestAdjustRaw(oCMobContainer* c, const std::string& inst, int delta) {
        if (delta < 0) {
            int need = -delta;
            std::vector<oCItem*> its;
            for (zCListSort<oCItem>* n = c->containList.next; n; n = n->next) {
                if (n->data && CoopItemInst(n->data) == inst)
                    its.push_back(n->data);
            }
            for (oCItem* it : its) {
                if (need <= 0)
                    break;
                const int a = CoopItemAmount(it);
                if (a > need) {
                    it->amount = a - need;
                    need = 0;
                }
                else {
                    need -= a;
                    CoopChestDropItem(c, it);   // u98 (P120): nicht Remove - kein Rest in den Listen der Welt
                }
            }
            return -delta - need;
        }
        if (delta > 0) {
            for (zCListSort<oCItem>* n = c->containList.next; n; n = n->next) {
                oCItem* it = n->data;
                if (it && (it->flags & ITM_FLAG_MULTI) && CoopItemInst(it) == inst) {
                    it->amount = CoopItemAmount(it) + delta;
                    return delta;
                }
            }
            const int idx = parser->GetIndex(zSTRING(inst.c_str()));
            if (idx < 0)
                return 0;
            int made = 0;
            while (made < delta && made < 500) {
                oCItem* it = CreateCoopItem(idx);
                if (!it)
                    break;
                const bool multi = (it->flags & ITM_FLAG_MULTI) != 0;
                it->amount = multi ? (delta - made) : 1;
                made += it->amount;
                c->Insert(it);
                it->Release();
            }
            return made;
        }
        return 0;
    }

    // Diff "INST:n;INST:-n" anwenden. Rueckgabe: Fehlmenge (angefordert, aber nicht vorhanden).
    static int ChestApplyDiff(oCMobContainer* c, const std::string& diff) {
        int fehlt = 0;
        for (auto const& kv : BestandAusText(diff)) {
            const int done = ChestAdjust(c, kv.first, kv.second);
            if (kv.second < 0 && done < -kv.second)
                fehlt += -kv.second - done;
        }
        return fehlt;
    }

    static int ChestSetBestand(oCMobContainer* c, const CoopBestand& ziel) {
        const CoopBestand ist = ChestBestand(c);
        int changes = 0;
        for (auto const& kv : ist) {
            auto f = ziel.find(kv.first);
            const int t = f == ziel.end() ? 0 : f->second;
            if (t != kv.second) {
                ChestAdjust(c, kv.first, t - kv.second);
                changes++;
            }
        }
        for (auto const& kv : ziel) {
            if (kv.second > 0 && !ist.count(kv.first)) {
                ChestAdjust(c, kv.first, kv.second);
                changes++;
            }
        }
        return changes;
    }

    static oCMobContainer* FindChest(const zVEC3& p) {
        zCArray<zCVob*> vobs = CollectVobsInRadius(p, 150);
        oCMobContainer* best = NULL;
        float bestD = 100.f;
        for (int i = 0; i < vobs.GetNumInList(); i++) {
            oCMobContainer* c = zDYNAMIC_CAST<oCMobContainer>(vobs.GetSafe(i));
            if (!c)
                continue;
            const float d = CoopDist(c->GetPositionWorld(), p);
            if (d < bestD) {
                bestD = d;
                best = c;
            }
        }
        return best;
    }

    // --- u137 (P134): Mechanismen - Tueren, Schalter, Winden und alles mit Ausloeser ---------------------------------------
    //
    // Testlauf b54 (DLL u136): Der Client setzte den Portalring am Schloss ein (Mob OBERMEGAMASTER_IMGAME_STARTER, Zustand 1), beim Host blieb
    // der Mob auf Zustand 0, sein Ring verschwand als Missions-Gegenstand trotzdem. Beim Laden gilt der Stand des Hosts: Portal zu, Ring weg,
    // Sackgasse. Dasselbe bei Tempeltuer, Vinos Keller, Fokus-Sockeln, Steinplatten (zCCodeMaster zaehlte je PC) und jedem Hebel.
    // Engine (Disassembly G2A 0x00720C80, G1 0x0067F300): oCMobInter::OnEndStateChange(npc, von, nach) schickt bei nach 1 OnTrigger, bei nach 0
    // OnUntrigger an sich selbst (von dort an die Vobs namens triggerTarget), danach das Zustandsskript.
    // Jetzt: benutzt der eigene Held so einen Mob, gehen Lage, von, nach und Schloss an die anderen (Client ueber den Host); dort Schloss und
    // Zustand setzen, dann derselbe Ausloeser an dieselben Ziele. Der Zustand steht nicht im Spielstand, Mover und Trigger schon - darum
    // bleibt das Portal nach dem Laden offen. Weltdaten G2A und G1 (tools: Vob-Baum der .ZEN): Ketten aus zCTrigger, zCTriggerList,
    // zCMessageFilter, zCCodeMaster, zCMover, oCTriggerScript, zCCSCamera, Ton- und Effekt-Vobs.
    //
    // Als Ausloeser (vobInstigator) geht beim Partner NULL durch die Kette: zCTrigger::CanBeActivatedNow (G2A 0x00610220) laesst NULL immer zu,
    // waehrend der eigene Held dort als PC, eine Figur als NPC, der Mob als Objekt gefiltert wuerde (NEWWORLD: viele Trigger ohne NPC oder Objekt).
    // Wld_SendTrigger gibt den Vob selbst an (G2A 0x006DDEE0), ein Mover sich selbst (0x00611896) - NULL kommt also nur von hier.
    // Daran erkennen die Haken unten die Kette: oCTriggerScript laeuft nur beim Benutzer (ADW_PORTALTEMPEL_FOCUS_FUNC nimmt dem Helden den
    // Fokus und zaehlt hoch, EVT_ORNAMENT_* legen NPCs an - Variablen, Tagebuch, Spawns und Trigger daraus kommen schon ueber Story-Takt und
    // Host-Ereignisse), die Kamerafahrt auch (riss den Partner sonst aus dem Kampf). Grenze: ein Skript hinter einem Mover bekommt den Mover
    // als Ausloeser und laeuft auf beiden Seiten (nur G1 OLDMINE: EVT_TRIGGERSCRIPT_CRAWLERCAVE).
    // Truhen laufen weiter ueber SYNC_CHEST_*; Baenke, Betten, Leitern und Herde aendern an der Welt nichts.
    // u138 (P136): Feuerstellen auch nicht, obwohl die Fackelhalter in G1 ein Ziel tragen (WORLD.ZEN: "TORCHSCRIPT", kein solcher Vob): Die Flamme setzt
    // oCMobFire::OnTrigger, hier stuende der Mob auf 1 ohne Flamme und liesse sich nicht mehr anzuenden.
    bool CoopRuleQuests();   // CoopDifficulty.cpp

    static long long MobChainUntilMs = 0;    // bis dahin ist ein NULL-Ausloeser die Kette eines abgeglichenen Mechanismus
    static int CoopMobSkipScripts = 0, CoopMobSkipCams = 0;   // Testlauf (CoopSelfTest.cpp)
    static int CoopCamStopped = 0;           // u138 (P136): Kamerafahrten, die laut Spielstand noch liefen
    static std::map<std::string, std::vector<json>> MobPending;   // Benutzt in einer anderen Welt: Welt -> Ereignisse, bis ich dorthin komme

    static bool MobSyncBase() {
        return WorldSyncBase() && (CoopRuleWorld() || CoopRuleQuests());   // der Ring faellt unter "Quests", die Tuer unter "Gegenstaende/Truhen"
    }

    static bool MobSyncRelevant(oCMobInter* m) {
        if (!m || zDYNAMIC_CAST<oCMobContainer>(m) || zDYNAMIC_CAST<oCMobFire>(m))
            return false;
        return !m->triggerTarget.IsEmpty() || zDYNAMIC_CAST<oCMobDoor>(m) || zDYNAMIC_CAST<oCMobSwitch>(m) || zDYNAMIC_CAST<oCMobWheel>(m);
    }

    static oCMobInter* FindMob(const zVEC3& p) {
        zCArray<zCVob*> vobs = CollectVobsInRadius(p, 150);
        oCMobInter* best = NULL;
        float bestD = 100.f;
        for (int i = 0; i < vobs.GetNumInList(); i++) {
            oCMobInter* m = zDYNAMIC_CAST<oCMobInter>(vobs.GetSafe(i));
            if (!MobSyncRelevant(m))
                continue;
            const float d = CoopDist(m->GetPositionWorld(), p);
            if (d < bestD) {
                bestD = d;
                best = m;
            }
        }
        return best;
    }

    static oCItem* FindLooseItem(const std::string& inst, const zVEC3& p, float maxDist) {
        const int idx = parser->GetIndex(zSTRING(inst.c_str()));
        if (idx < 0)
            return NULL;
        zCArray<zCVob*> vobs = CollectVobsInRadius(p, (int)maxDist + 50);
        oCItem* best = NULL;
        float bestD = maxDist;
        for (int i = 0; i < vobs.GetNumInList(); i++) {
            oCItem* it = zDYNAMIC_CAST<oCItem>(vobs.GetSafe(i));
            if (!it || it->GetInstance() != idx || !CoopItemLiegtFrei(it))
                continue;
            const float d = CoopDist(it->GetPositionWorld(), p);
            if (d < bestD) {
                bestD = d;
                best = it;
            }
        }
        return best;
    }

    // v18: fallen gelassener Gegenstand - ueber den eindeutigen Namen, erst in der Naehe, dann in der ganzen
    // Welt (die Kopie beim anderen PC liegt dort, wo dessen Coop-Figur stand, also nicht an derselben Stelle).
    static oCItem* FindDroppedItem(const std::string& obj, const zVEC3& p) {
        const zSTRING name(obj.c_str());
        zCArray<zCVob*> vobs = CollectVobsInRadius(p, 3000);
        for (int i = 0; i < vobs.GetNumInList(); i++) {
            oCItem* it = zDYNAMIC_CAST<oCItem>(vobs.GetSafe(i));
            if (it && it->GetObjectName() == name && CoopItemLiegtFrei(it, NULL, true))
                return it;
        }
        zCArray<zCVob*> all;
        ogame->GetGameWorld()->SearchVobListByClass(oCItem::classDef, all, NULL);
        for (int i = 0; i < all.GetNumInList(); i++) {
            oCItem* it = zDYNAMIC_CAST<oCItem>(all.GetSafe(i));
            if (it && it->GetObjectName() == name && CoopItemLiegtFrei(it, NULL, true))
                return it;
        }
        return NULL;
    }

    static void RemoveWorldItem(oCItem* it) {
        if (player && player->GetFocusVob() == it)
            player->ClearFocusVob();
        WorldSyncApplying++;   // nicht als eigenes Aufheben werten
        it->RemoveVobFromWorld();
        WorldSyncApplying--;
    }

#if COOP_BUILD(World)   // u3: bis u2 nur G1 - Freigabe je Engine in CoopCore.cpp; u12: erst ab hier (Hilfsfunktionen oben immer)

    // --- Senden ---------------------------------------------------------------------------------

    static void WorldSyncSend(json& j, int type) {
        j["type"] = type;
        j["id"] = MyselfId.ToChar();
        j["w"] = CoopWorldName();
        CoopTraceCount(TRACE_SENT, type);
        ReadyToSendJsons.enqueue(j);
    }

    static void SendChestState(oCMobContainer* c, const std::string& by) {
        json j;
        j["k"] = CoopPosJson(c->GetPositionWorld());
        j["lk"] = c->locked ? 1 : 0;
        j["c"] = BestandText(ChestBestand(c));
        j["by"] = by;
        WorldSyncSend(j, SYNC_CHEST_STATE);
    }

    // u137 (P134): Stand aller Mechanismen im Weltbestand - fuer einen Client, der neu dazukommt oder sich neu abgleicht. Nur Zustand und Schloss,
    // keine Ausloeser: Mover und Trigger bringt der Spielstand mit
    static void SendMobBulk() {
        zCArray<zCVob*> mobs;
        ogame->GetGameWorld()->SearchVobListByBaseClass(oCMobInter::classDef, mobs, NULL);
        json batch = json::array();
        int n = 0;
        for (int i = 0; i < mobs.GetNumInList(); i++) {
            oCMobInter* m = zDYNAMIC_CAST<oCMobInter>(mobs.GetSafe(i));
            if (!MobSyncRelevant(m))
                continue;
            json s;
            s["k"] = CoopPosJson(m->GetPositionWorld());
            s["s"] = (int)m->state;
            if (oCMobLockable* l = zDYNAMIC_CAST<oCMobLockable>(m))
                s["lk"] = l->locked ? 1 : 0;
            batch.push_back(s);
            n++;
            if (batch.size() >= 40) {
                json j;
                j["s"] = batch;
                WorldSyncSend(j, SYNC_MOB_BULK);
                batch = json::array();
            }
        }
        if (!batch.empty()) {
            json j;
            j["s"] = batch;
            WorldSyncSend(j, SYNC_MOB_BULK);
        }
        WorldSyncLog("Mechanismen gesendet (%s): %d", CoopWorldName().c_str(), n);
    }

    static void SendWorldBulk() {
        oCWorld* world = ogame->GetGameWorld();

        zCArray<zCVob*> chests;
        world->SearchVobListByClass(oCMobContainer::classDef, chests, NULL);
        json batch = json::array();
        for (int i = 0; i < chests.GetNumInList(); i++) {
            oCMobContainer* c = zDYNAMIC_CAST<oCMobContainer>(chests.GetSafe(i));
            if (!c)
                continue;
            json s;
            s["k"] = CoopPosJson(c->GetPositionWorld());
            s["lk"] = c->locked ? 1 : 0;
            s["c"] = BestandText(ChestBestand(c));
            batch.push_back(s);
            if (batch.size() >= 40) {
                json j;
                j["s"] = batch;
                WorldSyncSend(j, SYNC_CHEST_BULK);
                batch = json::array();
            }
        }
        if (!batch.empty()) {
            json j;
            j["s"] = batch;
            WorldSyncSend(j, SYNC_CHEST_BULK);
        }

        zCArray<zCVob*> items;
        world->SearchVobListByClass(oCItem::classDef, items, NULL);
        std::vector<std::string> parts(1);
        int inPart = 0, loose = 0;
        char buf[160];
        std::set<zCVob*> slots;
        CoopCollectSlotVobs(slots);
        for (int i = 0; i < items.GetNumInList(); i++) {
            oCItem* it = zDYNAMIC_CAST<oCItem>(items.GetSafe(i));
            if (!CoopItemLiegtFrei(it, &slots))
                continue;
            const zVEC3 p = it->GetPositionWorld();
            _snprintf(buf, sizeof(buf), "%s,%ld,%ld,%ld;", it->GetInstanceName().ToChar(), CoopRound(p.n[0]), CoopRound(p.n[1]), CoopRound(p.n[2]));
            buf[sizeof(buf) - 1] = 0;
            parts.back() += buf;
            loose++;
            if (++inPart >= 400) {
                parts.push_back(std::string());
                inPart = 0;
            }
        }
        if (parts.size() > 1 && parts.back().empty())
            parts.pop_back();
        for (size_t k = 0; k < parts.size(); k++) {
            json j;
            j["part"] = (int)k;
            j["of"] = (int)parts.size();
            j["s"] = parts[k];
            WorldSyncSend(j, SYNC_ITEM_BULK);
        }
        WorldSyncLog("Bestand gesendet (%s): %d Truhen, %d Gegenstaende am Boden in %d Teilen",
            CoopWorldName().c_str(), chests.GetNumInList(), loose, (int)parts.size());
        SendMobBulk();   // u137
    }

    // u137 (P134): eigener Held hat einen Mechanismus benutzt (Haken unten)
    static void MobSendUse(oCMobInter* m, int from, int to) {
        json j;
        j["k"] = CoopPosJson(m->GetPositionWorld());
        j["f"] = from;
        j["t"] = to;
        if (oCMobLockable* l = zDYNAMIC_CAST<oCMobLockable>(m))
            j["lk"] = l->locked ? 1 : 0;
        if (ServerThread)
            j["by"] = MyselfId.ToChar();
        WorldSyncSend(j, SYNC_MOB_STATE);
        WorldSyncLog("Mechanismus %s bei %s: Zustand %d -> %d, Ziel \"%s\" - an den Partner", m->GetObjectName().ToChar(),
            CoopPosKey(m->GetPositionWorld()).c_str(), from, to, m->triggerTarget.ToChar());
    }

    // --- Eigene Handlungen (aus den Hooks) ------------------------------------------------------

    static void ChestOpenedLocal(oCMobContainer* c) {
        ChestOpenSnapshot[c] = ChestBestand(c);
        json j;
        j["k"] = CoopPosJson(c->GetPositionWorld());
        WorldSyncSend(j, SYNC_CHEST_OPEN);
        WorldSyncLog("Truhe %s geoeffnet: %s", CoopPosKey(c->GetPositionWorld()).c_str(), BestandText(ChestOpenSnapshot[c]).c_str());
    }

    static void ChestClosedLocal(oCMobContainer* c) {
        auto f = ChestOpenSnapshot.find(c);
        if (f == ChestOpenSnapshot.end())
            return;
        const CoopBestand vorher = f->second;
        ChestOpenSnapshot.erase(f);
        const std::string key = CoopPosKey(c->GetPositionWorld());

        if (!WorldSyncAktiv()) {
            ChestPendingDiffs.erase(c);
            return;
        }

        const std::string diff = DiffText(vorher, ChestBestand(c));
        if (ServerThread) {
            // Waehrend der Host offen hatte, eingegangene Client-Aenderungen jetzt nachholen.
            auto p = ChestPendingDiffs.find(c);
            if (p != ChestPendingDiffs.end()) {
                for (auto const& d : p->second) {
                    const int fehlt = ChestApplyDiff(c, d);
                    WorldSyncLog("Truhe %s: zurueckgestellte Aenderung %s angewendet%s", key.c_str(), d.c_str(), fehlt ? " (KONFLIKT: fehlte)" : "");
                }
                ChestPendingDiffs.erase(p);
            }
            SendChestState(c, MyselfId.ToChar());
        }
        else {
            json j;
            j["k"] = CoopPosJson(c->GetPositionWorld());
            j["lk"] = c->locked ? 1 : 0;
            j["d"] = diff;
            WorldSyncSend(j, SYNC_CHEST_CLOSE);
        }
        WorldSyncLog("Truhe %s geschlossen, Aenderung: %s", key.c_str(), diff.empty() ? "-" : diff.c_str());
    }

    // Ein NPC hat einen Gegenstand aus der Welt genommen: Spieler -> melden (Client an den Host, Host an
    // alle), NPC auf dem Host -> an alle. NPCs auf dem Client und Coop-Figuren: nichts (Abbilder).
    static void ReportPickup(oCNpc* owner, const CoopWorldGone& g, const char* via) {
        if (!owner || !WorldSyncAktiv())
            return;
        const bool selbst = owner == player;
        const bool hostNpc = ServerThread && !selbst && !owner->IsAPlayer() && !IsCoopPlayer(owner->GetObjectName());
        if (!selbst && !hostNpc)
            return;
        json j;
        j["i"] = g.inst;
        j["p"] = CoopPosJson(g.pos);
        j["n"] = g.amount;
        if (!g.obj.empty())
            j["o"] = g.obj;   // fallen gelassen: Zuordnung ueber den eindeutigen Namen (v18)
        if (ServerThread) {
            j["by"] = selbst ? std::string(MyselfId.ToChar()) : std::string("HOST_NPC");
            WorldSyncSend(j, SYNC_ITEM_GONE);
        }
        else {
            WorldSyncSend(j, SYNC_ITEM_TAKE);
        }
        if (selbst)
            WorldSyncLog("aufgehoben (%s): %s bei %s", via, g.inst.c_str(), CoopPosKey(g.pos).c_str());
    }

    // Aus dem RemoveVob-Hook (Hooks.cpp) bzw. dem DisableVob-Platz, vor dem Original: freien Gegenstand
    // vormerken (AddRef).
    void CoopWorldSyncItemLeavesWorld(zCVob* vob, const void* ret, const char* how) {
        if (WorldSyncApplying || !vob || !WorldSyncAktiv())
            return;
        oCItem* it = zDYNAMIC_CAST<oCItem>(vob);
        if (!it)
            return;
        auto rep = ReportedStillInWorld.find(it);
        if (rep != ReportedStillInWorld.end()) {   // schon beim Einlegen ins Inventar gemeldet
            ReportedStillInWorld.erase(rep);
            it->Release();
            return;
        }
        if (!CoopItemLiegtFrei(it, NULL, true) || RecentWorldRemovals.count(it))
            return;
        CoopWorldGone g;
        g.inst = CoopItemInst(it);
        g.pos = it->GetPositionWorld();
        g.ms = CurrentMs;
        g.amount = CoopItemAmount(it);
        if (CoopItemIsDropped(it))
            g.obj = it->GetObjectName().ToChar();
        it->AddRef();
        RecentWorldRemovals[it] = g;
        char mod[96];
        CoopModuleOf(ret, mod, sizeof(mod));
        WorldSyncDiag("%s verlaesst die Welt bei %s (%s, Aufrufer %s)", g.inst.c_str(), CoopPosKey(g.pos).c_str(), how, mod);
    }

    // Gegenstand kommt in das Inventar von owner. checkWorld: liegt er noch in der Welt, gilt er schon
    // jetzt als aufgehoben (Reihenfolge Einlegen -> Entfernen).
    static void ItemIntoInventory(oCNpc* owner, oCItem* item, const char* via, const void* ret, bool checkWorld) {
        if (!item || !owner || WorldSyncApplying)
            return;
        auto f = RecentWorldRemovals.find(item);
        if (f != RecentWorldRemovals.end()) {
            const CoopWorldGone g = f->second;
            RecentWorldRemovals.erase(f);
            ReportPickup(owner, g, via);
            item->Release();
            return;
        }
        if (!checkWorld || !WorldSyncAktiv())
            return;
        char mod[96];
        if (CoopItemLiegtFrei(item, NULL, true)) {
            CoopWorldGone g;
            g.inst = CoopItemInst(item);
            g.pos = item->GetPositionWorld();
            g.ms = CurrentMs;
            g.amount = CoopItemAmount(item);
            if (CoopItemIsDropped(item))
                g.obj = item->GetObjectName().ToChar();
            ReportPickup(owner, g, via);
            if (!ReportedStillInWorld.count(item)) {
                item->AddRef();
                ReportedStillInWorld[item] = CurrentMs;
            }
            CoopModuleOf(ret, mod, sizeof(mod));
            WorldSyncDiag("%s ins Inventar, lag noch in der Welt (%s, Aufrufer %s)", g.inst.c_str(), via, mod);
        }
        else if (owner == player) {
            PlayerReceived[CoopItemInst(item)] += CoopItemAmount(item);   // fuer das Pluendern (FlushNpcLoot)
            CoopModuleOf(ret, mod, sizeof(mod));
            WorldSyncDiag("%s ins Inventar des Spielers, nicht aus der Welt (%s, Aufrufer %s)", CoopItemInst(item).c_str(), via, mod);
        }
    }

    // --- v15: Leichen / Bewusstlose pluendern ---------------------------------------------------

    static bool NpcLootRelevant(oCNpcInventory* inv, oCNpc*& owner) {
        owner = inv ? inv->owner : NULL;
        if (!owner || owner == player || WorldSyncApplying || !WorldSyncAktiv())
            return false;
        if (!owner->IsDead() && !owner->IsUnconscious())
            return false;
        return !IsCoopPlayer(owner->GetObjectName());
    }

    static void NpcLootRecord(oCNpc* owner, const std::string& inst, int n) {
        if (n <= 0 || inst.empty())
            return;
        auto f = NpcToUniqueNameList.find(owner);
        if (f == NpcToUniqueNameList.end()) {
            WorldSyncDiag("Pluendern: %s aus %s - NPC hat keinen GT-Namen, nicht abgleichbar", inst.c_str(), owner->GetInstanceName().ToChar());
            return;
        }
        LootRemovals[std::string(f->second.ToChar())][inst] += n;
    }

    static void FlushNpcLoot() {
        if (!LootRemovals.empty()) {
            for (auto const& npcKv : LootRemovals) {
                CoopBestand taken;
                for (auto const& kv : npcKv.second) {
                    auto pr = PlayerReceived.find(kv.first);
                    const int got = pr == PlayerReceived.end() ? 0 : pr->second;
                    const int n = kv.second < got ? kv.second : got;
                    if (n > 0) {
                        taken[kv.first] = n;
                        pr->second -= n;
                    }
                }
                if (taken.empty())
                    continue;
                json j;
                j["n"] = npcKv.first;
                j["d"] = BestandText(taken);
                if (ServerThread)
                    j["by"] = MyselfId.ToChar();
                WorldSyncSend(j, SYNC_NPC_LOOT);
                WorldSyncLog("gepluendert: %s -> %s", npcKv.first.c_str(), BestandText(taken).c_str());
            }
            LootRemovals.clear();
        }
        PlayerReceived.clear();
    }

    // Beim anderen PC: Gegenstaende aus dem Inventar des NPCs entfernen - wie die Engine bei
    // Npc_RemoveInvItem: RemoveFromInv (legt ab, Ausgeruestetes wird abgelegt), dann RemoveVob.
    static int NpcRemoveItems(oCNpc* npc, const std::string& inst, int n) {
        int done = 0;
        WorldSyncApplying++;
        const zSTRING zinst(inst.c_str());
        for (int guard = 0; done < n && guard < 64; guard++) {
            oCItem* it = npc->IsInInv(zinst, 1);
            if (!it)
                break;
            const int a = CoopItemAmount(it);
            const int take = (n - done) < a ? (n - done) : a;
            oCItem* r = npc->RemoveFromInv(it, take);
            if (!r)
                break;
            done += take;
            ogame->GetGameWorld()->RemoveVob(r);
        }
        WorldSyncApplying--;
        return done;
    }

    // v16: Die Waffe einer Leiche liegt auf dem einen PC am Boden (beim Sterben fallen gelassen), auf dem
    // anderen steckt sie noch im Inventar (Test v15: CS_ESKORTE2-1 - der Client hob das Schwert vom Boden
    // auf, beim Host war es in der Leiche, der Host bekam es beim Pluendern ein zweites Mal). Darum beim
    // Fehlen am jeweils anderen Ort suchen: Boden -> Inventar toter/bewusstloser NPCs in der Naehe und
    // umgekehrt. Nur gleiche Instanz, nur innerhalb von 500, nur tote/bewusstlose NPCs.
    static oCNpc* FindDeadNpcWithItem(const std::string& inst, const zVEC3& p, float maxDist) {
        oCNpc* best = NULL;
        float bestD = maxDist;
        const zSTRING zinst(inst.c_str());
        for (zCListSort<oCNpc>* l = CoopWorldNpcListFirst(ogame->GetGameWorld()); l; l = l->next) {
            oCNpc* npc = l->data;
            if (!npc || npc == player || IsCoopPlayer(npc->GetObjectName()))
                continue;
            if (!npc->IsDead() && !npc->IsUnconscious())
                continue;
            const float d = CoopDist(npc->GetPositionWorld(), p);
            if (d >= bestD || !npc->IsInInv(zinst, 1))
                continue;
            bestD = d;
            best = npc;
        }
        return best;
    }

    static int RemoveFromDeadNpcNear(const std::string& inst, int n, const zVEC3& p, std::string& who) {
        oCNpc* npc = FindDeadNpcWithItem(inst, p, 500.f);
        if (!npc)
            return 0;
        auto f = NpcToUniqueNameList.find(npc);
        who = f != NpcToUniqueNameList.end() ? std::string(f->second.ToChar()) : std::string(npc->GetInstanceName().ToChar());
        return NpcRemoveItems(npc, inst, n);
    }

    static int RemoveLooseNear(const std::string& inst, int n, const zVEC3& p) {
        int done = 0;
        for (int guard = 0; done < n && guard < 16; guard++) {
            oCItem* it = FindLooseItem(inst, p, 500.f);
            if (!it)
                break;
            const int a = CoopItemAmount(it);
            if (a > n - done) {
                it->amount = a - (n - done);   // Stapel am Boden nur verkleinern
                done = n;
            }
            else {
                done += a;
                RemoveWorldItem(it);
            }
        }
        return done;
    }

    // Truhe ohne geoeffnetes Fenster veraendert (Quickloot, Skripte): Stand vorher merken.
    static void ChestChangeBegin(oCMobContainer* c) {
        if (ChestHookDepth++ > 0)
            return;   // Remove(item, n) ruft Remove(item) - nur der aeussere Aufruf zaehlt
        if (WorldSyncApplying || !c || ChestOpenSnapshot.count(c) || ChestDirty.count(c) || !WorldSyncAktiv())
            return;
        ChestDirty[c] = ChestBestand(c);
    }

    static void ChestChangeEnd() {
        ChestHookDepth--;
    }

    static void FlushChestDirty() {
        if (ChestDirty.empty())
            return;
        std::map<oCMobContainer*, CoopBestand> dirty;
        dirty.swap(ChestDirty);
        for (auto const& kv : dirty) {
            oCMobContainer* c = kv.first;
            const std::string diff = DiffText(kv.second, ChestBestand(c));
            if (diff.empty())
                continue;
            if (ServerThread) {
                SendChestState(c, MyselfId.ToChar());
            }
            else {
                json j;
                j["k"] = CoopPosJson(c->GetPositionWorld());
                j["lk"] = c->locked ? 1 : 0;
                j["d"] = diff;
                WorldSyncSend(j, SYNC_CHEST_CLOSE);
            }
            WorldSyncLog("Truhe %s ohne Oeffnen geaendert (Quickloot?): %s", CoopPosKey(c->GetPositionWorld()).c_str(), diff.c_str());
        }
    }

    static void ExpireWorldRemovals() {
        for (auto it = RecentWorldRemovals.begin(); it != RecentWorldRemovals.end();) {
            if (CurrentMs > it->second.ms + 2000) {
                WorldSyncDiag("%s verliess die Welt, kam in kein Inventar", it->second.inst.c_str());
                it->first->Release();
                it = RecentWorldRemovals.erase(it);
            }
            else
                ++it;
        }
        for (auto it = ReportedStillInWorld.begin(); it != ReportedStillInWorld.end();) {
            if (CurrentMs > it->second + 2000) {
                it->first->Release();
                it = ReportedStillInWorld.erase(it);
            }
            else
                ++it;
        }
    }

    // --- Empfang (aus ProcessCoopPacket, Hauptfaden) --------------------------------------------

    bool CoopWorldSyncHandles(int type) {
        return (type >= SYNC_CHEST_OPEN && type <= SYNC_NPC_LOOT) || type == SYNC_MOB_STATE || type == SYNC_MOB_BULK;   // u137
    }

    // u137 (P134): Benutzung beim Partner nachvollziehen. Benutzt der eigene Held den Mob gerade selbst, bleibt sein Zustand ihm; der Ausloeser
    // gilt trotzdem - sonst liefen die Welten auseinander
    static void MobApplyUse(oCMobInter* m, int from, int to, int lk, const std::string& by, const char* why) {
        oCMobLockable* l = zDYNAMIC_CAST<oCMobLockable>(m);
        const bool lockChanged = l && lk >= 0 && (lk != 0) != (l->locked != 0);
        if (lockChanged)
            l->SetLocked(lk);
        const bool busy = m->npcsCurrent > 0;
        const int was = m->state;
        if (!busy && was != to) {
            m->StartStateChange(NULL, from, to);   // nur die Animation des Mobs (ohne Benutzer wie beim Zurueckdrehen in oCMobInter::OnTick)
            m->state = m->state_target = to;
            if (m->rewind)
                m->SetSleeping(FALSE);
        }
        int targets = 0;
        if ((to == 0 || to == 1) && !m->triggerTarget.IsEmpty() && m->GetHomeWorld()) {
            MobChainUntilMs = GetCurrentMs() + 300000;   // Verzoegerungen in der Kette (fireDelaySec, zCTriggerList)
            zCArray<zCVob*> ziele;
            m->GetHomeWorld()->SearchVobListByName(m->triggerTarget, ziele);
            for (int i = 0; i < ziele.GetNumInList(); i++) {
                zCVob* v = ziele.GetSafe(i);
                if (!v)
                    continue;
                if (to == 1)
                    v->GetEM()->OnTrigger(m, NULL);
                else
                    v->GetEM()->OnUntrigger(m, NULL);
                targets++;
            }
        }
        WorldSyncLog("Mechanismus %s bei %s von %s (%s): Zustand %d -> %d%s%s, %d Ziele (\"%s\")", m->GetObjectName().ToChar(),
            CoopPosKey(m->GetPositionWorld()).c_str(), by.c_str(), why, was, to, busy ? " (hier gerade benutzt - Zustand bleibt)" : "",
            lockChanged ? (lk ? " [verschlossen]" : " [aufgeschlossen]") : "", targets, m->triggerTarget.ToChar());
    }

    static void MobApplyEvent(const json& e, const std::string& by, const char* why) {
        zVEC3 p;
        if (!CoopPosFromJson(e.at("k"), p))
            return;
        oCMobInter* m = FindMob(p);
        if (!m) {
            WorldSyncLog("Mechanismus %s von %s: hier nicht gefunden", CoopPosKey(p).c_str(), by.c_str());
            return;
        }
        MobApplyUse(m, e.at("f").get<int>(), e.at("t").get<int>(), e.contains("lk") && e.at("lk").is_number() ? e.at("lk").get<int>() : -1, by, why);
    }

    static void MobReceive(json& e, const std::string& id, int type) {
        if (!MobSyncBase())
            return;
        const std::string w = e.contains("w") && e["w"].is_string() ? e["w"].get<std::string>() : std::string();
        if (type == SYNC_MOB_BULK) {
            if (ServerThread || w != CoopWorldName() || !e["s"].is_array())
                return;
            int found = 0, missing = 0, changed = 0;
            zVEC3 p;
            for (auto const& s : e["s"]) {
                if (!s.is_object() || !s.contains("k") || !CoopPosFromJson(s.at("k"), p))
                    continue;
                oCMobInter* m = FindMob(p);
                if (!m) {
                    missing++;
                    continue;
                }
                found++;
                const int lk = s.contains("lk") && s.at("lk").is_number() ? s.at("lk").get<int>() : -1;
                oCMobLockable* l = zDYNAMIC_CAST<oCMobLockable>(m);
                if (l && lk >= 0 && (lk != 0) != (l->locked != 0)) {
                    l->SetLocked(lk);
                    changed++;
                }
                const int st = s.contains("s") && s.at("s").is_number() ? s.at("s").get<int>() : (int)m->state;
                if (st != m->state && m->npcsCurrent <= 0) {
                    m->state = m->state_target = st;
                    if (zCModel* model = m->GetModel()) {
                        const int ani = model->GetAniIDFromAniName(zSTRING(("S_S" + std::to_string(st)).c_str()));
                        if (ani >= 0)
                            model->StartAni(ani, 0);
                    }
                    if (m->rewind)
                        m->SetSleeping(FALSE);
                    changed++;
                }
            }
            if (changed || missing)
                WorldSyncLog("Mechanismen-Abgleich: %d zugeordnet, %d geaendert, %d hier nicht gefunden", found, changed, missing);
            return;
        }
        // Client -> Host ohne "by"; der Host wendet an und verteilt mit "by". Die automatisch weitergeleitete Client-Fassung ignorieren die anderen Clients.
        const bool relay = e.contains("by") && e["by"].is_string();
        const std::string by = relay ? e["by"].get<std::string>() : id;
        if ((!ServerThread && !relay) || by == MyselfId.ToChar())
            return;
        if (w != CoopWorldName()) {
            auto& q = MobPending[w];
            if (q.size() < 200) {
                json g = e;
                g["by"] = by;
                q.push_back(g);
            }
            WorldSyncLog("Mechanismus von %s in %s, ich in %s - gilt, sobald ich dorthin komme (%d warten)", by.c_str(), w.c_str(), CoopWorldName().c_str(), (int)q.size());
        }
        else
            MobApplyEvent(e, by, "Paket");
        if (ServerThread && !relay) {
            json j = e;
            j["by"] = id;
            j["type"] = type;
            j["id"] = MyselfId.ToChar();
            CoopTraceCount(TRACE_SENT, type);
            ReadyToSendJsons.enqueue(j);   // "w" bleibt die Welt des Benutzers
        }
    }

    static void MobPendingFlush() {
        if (MobPending.empty() || IsLoadingLevel || CurrentMs < WorldLoadedMs + 3000)
            return;
        auto f = MobPending.find(CoopWorldName());
        if (f == MobPending.end())
            return;
        const std::vector<json> q = f->second;
        MobPending.erase(f);
        for (const json& e : q)
            MobApplyEvent(e, e.at("by").get<std::string>(), "nachgeholt");
    }

    // Laden eines Spielstands: was der gespeicherte Stand kennt, gilt; Warteliste verwerfen (Plugin.cpp)
    void CoopWorldSyncSaveLoaded() {
        if (!MobPending.empty())
            WorldSyncLog("Mechanismen: %d Welten mit wartenden Ereignissen verworfen (Spielstand geladen)", (int)MobPending.size());
        MobPending.clear();
    }

    static void ReleaseLock(const std::string& key, const std::string& owner) {
        auto f = ChestLocks.find(key);
        if (f != ChestLocks.end() && f->second.owner == owner)
            ChestLocks.erase(f);
    }

    static void ApplyChestState(oCMobContainer* c, const json& s, const char* why) {
        const std::string key = CoopPosKey(c->GetPositionWorld());
        if (ChestOpenSnapshot.count(c)) {
            WorldSyncLog("Truhe %s (%s): selbst geoeffnet, Stand folgt nach dem Schliessen", key.c_str(), why);
            return;
        }
        const CoopBestand ziel = BestandAusText(s.at("c").get<std::string>());
        const std::string vorher = BestandText(ChestBestand(c));
        const int changes = ChestSetBestand(c, ziel);
        const int lk = s.contains("lk") && s.at("lk").is_number() ? s.at("lk").get<int>() : (c->locked ? 1 : 0);
        const bool lockChanged = (lk != 0) != (c->locked != 0);
        if (lockChanged)
            c->SetLocked(lk);
        if (changes || lockChanged)
            WorldSyncLog("Truhe %s (%s): %s -> %s%s", key.c_str(), why, vorher.empty() ? "leer" : vorher.c_str(),
                BestandText(ChestBestand(c)).c_str(), lockChanged ? (lk ? " [verschlossen]" : " [aufgeschlossen]") : "");
    }

    static void ReconcileItems() {
        oCWorld* world = ogame->GetGameWorld();
        zCArray<zCVob*> items;
        world->SearchVobListByClass(oCItem::classDef, items, NULL);
        std::map<std::string, std::vector<oCItem*>> mine;
        int total = 0;
        std::set<zCVob*> slots;
        CoopCollectSlotVobs(slots);
        for (int i = 0; i < items.GetNumInList(); i++) {
            oCItem* it = zDYNAMIC_CAST<oCItem>(items.GetSafe(i));
            if (!CoopItemLiegtFrei(it, &slots))
                continue;
            mine[CoopItemInst(it)].push_back(it);
            total++;
        }

        int hostTotal = 0, hostOnly = 0;
        for (auto const& kv : ItemBulk.host) {
            hostTotal += (int)kv.second.size();
            if (!mine.count(kv.first))
                hostOnly += (int)kv.second.size();
        }

        std::vector<oCItem*> weg;
        for (auto& kv : mine) {
            auto h = ItemBulk.host.find(kv.first);
            const size_t n = h == ItemBulk.host.end() ? 0 : h->second.size();
            std::vector<bool> used(n, false);
            for (oCItem* it : kv.second) {
                const zVEC3 p = it->GetPositionWorld();
                int best = -1;
                float bestD = 250.f;
                for (size_t k = 0; k < n; k++) {
                    if (used[k])
                        continue;
                    const float d = CoopDist(p, h->second[k]);
                    if (d < bestD) {
                        bestD = d;
                        best = (int)k;
                    }
                }
                if (best >= 0)
                    used[best] = true;
                else
                    weg.push_back(it);
            }
            for (size_t k = 0; k < n; k++)
                if (!used[k])
                    hostOnly++;
        }

        if (weg.size() > 50 && weg.size() * 4 > (size_t)total) {
            WorldSyncLog("Gegenstaende: ABGLEICH ABGEBROCHEN - hier %d, Host %d, %d fehlen beim Host (anderer Spielstand?)",
                total, hostTotal, (int)weg.size());
            ChatLog(CoopTr("Welt-Abgleich: Gegenst" "\xE4" "nde weichen stark vom Host ab (anderer Spielstand?) - nicht angeglichen.", "World sync: items differ a lot from the host (other save?) - not matched."));
            return;
        }
        for (oCItem* it : weg) {
            if (WorldSyncLines < 300)
                WorldSyncLog("Gegenstand %s bei %s entfernt (beim Host nicht mehr da)", CoopItemInst(it).c_str(), CoopPosKey(it->GetPositionWorld()).c_str());
            RemoveWorldItem(it);
        }
        WorldSyncLog("Gegenstaende abgeglichen: hier %d, Host %d, entfernt %d, nur beim Host %d (nicht erzeugt)",
            total, hostTotal, (int)weg.size(), hostOnly);
    }

    static void ReceiveItemBulk(const json& e) {
        const int part = e.at("part").get<int>();
        const int of = e.at("of").get<int>();
        if (part == 0) {
            ItemBulk = CoopItemBulk();
            ItemBulk.expected = of;
        }
        if (ItemBulk.expected != of)
            return;   // Teil eines alten Durchlaufs
        const std::string s = e.at("s").get<std::string>();
        size_t start = 0;
        while (start < s.size()) {
            size_t end = s.find(';', start);
            if (end == std::string::npos)
                end = s.size();
            const std::string entry = s.substr(start, end - start);
            start = end + 1;
            const size_t c1 = entry.find(',');
            if (c1 == std::string::npos)
                continue;
            long x = 0, y = 0, z = 0;
            if (sscanf(entry.c_str() + c1 + 1, "%ld,%ld,%ld", &x, &y, &z) != 3)
                continue;
            ItemBulk.host[entry.substr(0, c1)].push_back(zVEC3((float)x, (float)y, (float)z));
        }
        ItemBulk.received++;
        if (part == of - 1) {
            if (ItemBulk.received == of)
                ReconcileItems();
            else
                WorldSyncLog("Gegenstaende: nur %d von %d Teilen empfangen - kein Abgleich", ItemBulk.received, of);
            ItemBulk = CoopItemBulk();
        }
    }

    void CoopWorldSyncReceive(json& e, const std::string& id) {
        const int type = e["type"].get<int>();
        try {
            if (type == SYNC_MOB_STATE || type == SYNC_MOB_BULK) {   // u137: eigene Regel und eigene Weltpruefung
                MobReceive(e, id, type);
                return;
            }
            if (!WorldSyncAktiv())
                return;
            const std::string w = e.contains("w") && e["w"].is_string() ? e["w"].get<std::string>() : std::string();
            if (w != CoopWorldName())
                return;   // andere Welt (z. B. Mine) - gilt hier nicht
            zVEC3 p;

            switch (type) {
            case SYNC_CHEST_OPEN:
            {
                if (!CoopPosFromJson(e["k"], p))
                    return;
                ChestLocks[CoopPosKey(p)] = { id, CurrentMs };
                break;
            }
            case SYNC_CHEST_CLOSE:
            {
                if (!CoopPosFromJson(e["k"], p))
                    return;
                const std::string key = CoopPosKey(p);
                ReleaseLock(key, id);
                if (!ServerThread)
                    break;   // Clients warten auf SYNC_CHEST_STATE vom Host
                oCMobContainer* c = FindChest(p);
                if (!c) {
                    WorldSyncLog("Truhe %s von %s: hier nicht gefunden", key.c_str(), id.c_str());
                    break;
                }
                const std::string diff = e["d"].get<std::string>();
                if (ChestOpenSnapshot.count(c)) {
                    ChestPendingDiffs[c].push_back(diff);   // Host hat sie gerade selbst offen
                    WorldSyncLog("Truhe %s von %s: %s zurueckgestellt (Host hat sie offen)", key.c_str(), id.c_str(), diff.c_str());
                    break;
                }
                const int fehlt = ChestApplyDiff(c, diff);
                if (e.contains("lk") && e["lk"].is_number() && e["lk"].get<int>() == 0 && c->locked)
                    c->SetLocked(0);
                WorldSyncLog("Truhe %s von %s: %s angewendet%s", key.c_str(), id.c_str(), diff.empty() ? "-" : diff.c_str(),
                    fehlt ? " (KONFLIKT: Teile fehlten beim Host)" : "");
                SendChestState(c, id);
                break;
            }
            case SYNC_CHEST_STATE:
            {
                if (!CoopPosFromJson(e["k"], p))
                    return;
                if (e.contains("by") && e["by"].is_string())
                    ReleaseLock(CoopPosKey(p), e["by"].get<std::string>());
                if (ServerThread)
                    break;
                oCMobContainer* c = FindChest(p);
                if (!c) {
                    WorldSyncLog("Truhe %s (Host-Stand): hier nicht gefunden", CoopPosKey(p).c_str());
                    break;
                }
                ApplyChestState(c, e, "Host-Stand");
                break;
            }
            case SYNC_CHEST_BULK:
            {
                if (ServerThread || !e["s"].is_array())
                    break;
                int found = 0, missing = 0;
                for (auto const& s : e["s"]) {
                    if (!s.is_object() || !s.contains("k") || !CoopPosFromJson(s.at("k"), p))
                        continue;
                    oCMobContainer* c = FindChest(p);
                    if (!c) {
                        missing++;
                        continue;
                    }
                    found++;
                    ApplyChestState(c, s, "Abgleich");
                }
                if (missing)
                    WorldSyncLog("Truhen-Abgleich: %d zugeordnet, %d hier nicht gefunden", found, missing);
                break;
            }
            case SYNC_ITEM_TAKE:
            {
                if (!ServerThread || !CoopPosFromJson(e["p"], p))
                    break;
                const std::string inst = e["i"].get<std::string>();
                const int amount = e.contains("n") && e["n"].is_number() ? e["n"].get<int>() : 1;
                const std::string obj = e.contains("o") && e["o"].is_string() ? e["o"].get<std::string>() : std::string();
                oCItem* it = obj.empty() ? FindLooseItem(inst, p, 200.f) : FindDroppedItem(obj, p);
                if (it) {
                    RemoveWorldItem(it);
                    WorldSyncLog("%s nahm %s bei %s%s - beim Host entfernt", id.c_str(), inst.c_str(), CoopPosKey(p).c_str(), obj.empty() ? "" : " (fallen gelassen)");
                }
                else if (!obj.empty()) {
                    WorldSyncLog("%s nahm fallen gelassenes %s (%s) - beim Host nicht gefunden", id.c_str(), inst.c_str(), obj.c_str());
                }
                else {
                    std::string who;
                    const int n = RemoveFromDeadNpcNear(inst, amount, p, who);
                    if (!n) {
                        WorldSyncLog("%s nahm %s bei %s - beim Host nicht (mehr) vorhanden", id.c_str(), inst.c_str(), CoopPosKey(p).c_str());
                        break;
                    }
                    WorldSyncLog("%s nahm %s bei %s - lag beim Host nicht am Boden, aus der Leiche %s entfernt (%d/%d)",
                        id.c_str(), inst.c_str(), CoopPosKey(p).c_str(), who.c_str(), n, amount);
                }
                json j;
                j["i"] = inst;
                j["p"] = e["p"];
                j["n"] = amount;
                if (!obj.empty())
                    j["o"] = obj;
                j["by"] = id;
                WorldSyncSend(j, SYNC_ITEM_GONE);
                break;
            }
            case SYNC_ITEM_GONE:
            {
                if (ServerThread || !CoopPosFromJson(e["p"], p))
                    break;
                const std::string by = e.contains("by") && e["by"].is_string() ? e["by"].get<std::string>() : std::string();
                if (by == MyselfId.ToChar())
                    break;   // selbst genommen, hier schon weg
                const std::string inst = e["i"].get<std::string>();
                const int amount = e.contains("n") && e["n"].is_number() ? e["n"].get<int>() : 1;
                const std::string obj = e.contains("o") && e["o"].is_string() ? e["o"].get<std::string>() : std::string();
                oCItem* it = obj.empty() ? FindLooseItem(inst, p, 200.f) : FindDroppedItem(obj, p);
                if (it) {
                    RemoveWorldItem(it);
                    if (by != "HOST_NPC")
                        WorldSyncLog("%s nahm %s bei %s%s - hier entfernt", by.c_str(), inst.c_str(), CoopPosKey(p).c_str(), obj.empty() ? "" : " (fallen gelassen)");
                    break;
                }
                if (!obj.empty()) {
                    WorldSyncLog("%s nahm fallen gelassenes %s (%s) - hier nicht gefunden", by.c_str(), inst.c_str(), obj.c_str());
                    break;
                }
                std::string who;
                const int n = RemoveFromDeadNpcNear(inst, amount, p, who);
                if (n)
                    WorldSyncLog("%s nahm %s bei %s - lag hier nicht am Boden, aus der Leiche %s entfernt (%d/%d)",
                        by.c_str(), inst.c_str(), CoopPosKey(p).c_str(), who.c_str(), n, amount);
                else {
                    WorldSyncLog("%s nahm %s bei %s - hier nicht gefunden, suche 2 min weiter", by.c_str(), inst.c_str(), CoopPosKey(p).c_str());
                    CoopPendingGone g;   // u70
                    g.inst = inst;
                    g.by = by;
                    g.pos = p;
                    g.sinceMs = CurrentMs;
                    if (PendingGone.size() < 200)
                        PendingGone.push_back(g);
                }
                break;
            }
            case SYNC_ITEM_BULK:
            {
                if (!ServerThread)
                    ReceiveItemBulk(e);
                break;
            }
            case SYNC_NPC_LOOT:
            {
                // Client -> Host ohne "by"; der Host wendet an und verteilt mit "by". Die automatisch
                // weitergeleitete Client-Fassung (ohne "by") ignorieren die anderen Clients.
                const std::string name = e.at("n").get<std::string>();
                const std::string d = e.at("d").get<std::string>();
                const bool relay = e.contains("by") && e["by"].is_string();
                const std::string by = relay ? e["by"].get<std::string>() : id;
                if (!ServerThread && !relay)
                    break;
                if (by == MyselfId.ToChar())
                    break;   // selbst gepluendert, hier schon weg
                auto f = UniqueNameToNpcList.find(string(name.c_str()));
                oCNpc* npc = f == UniqueNameToNpcList.end() ? NULL : f->second;
                if (!npc) {
                    WorldSyncLog("%s pluenderte %s (%s) - NPC hier nicht gefunden", by.c_str(), name.c_str(), d.c_str());
                }
                else {
                    std::string done;
                    char num[16];
                    for (auto const& kv : BestandAusText(d)) {
                        const int n = NpcRemoveItems(npc, kv.first, kv.second);
                        // Fehlt etwas im Inventar, liegt es hier vielleicht neben der Leiche (beim Sterben fallen gelassen).
                        const int boden = n < kv.second ? RemoveLooseNear(kv.first, kv.second - n, npc->GetPositionWorld()) : 0;
                        if (boden)
                            _snprintf(num, sizeof(num), "%d+%d/%d", n, boden, kv.second);
                        else
                            _snprintf(num, sizeof(num), "%d/%d", n, kv.second);
                        num[sizeof(num) - 1] = 0;
                        done += kv.first + ":" + num + ";";
                    }
                    WorldSyncLog("%s pluenderte %s - hier entfernt %s", by.c_str(), name.c_str(), done.c_str());
                }
                if (ServerThread && !relay) {
                    json j;
                    j["n"] = name;
                    j["d"] = d;
                    j["by"] = id;
                    WorldSyncSend(j, SYNC_NPC_LOOT);
                }
                break;
            }
            case SYNC_WORLD_REQUEST:
            {
                if (ServerThread) {
                    WorldBulkAtMs = CurrentMs + 500;
                    WorldSyncLog("%s bittet um den Weltbestand", id.c_str());
                }
                break;
            }
            }
        }
        catch (std::exception& ex) {
            CoopErrorLog("Welt-Sync", ex.what(), CoopUpdateTypeName(type));
        }
    }

    void CoopWorldSyncPeerGone(const std::string& name) {
        for (auto it = ChestLocks.begin(); it != ChestLocks.end();) {
            if (it->second.owner == name)
                it = ChestLocks.erase(it);
            else
                ++it;
        }
    }

    void CoopWorldSyncWorldLoaded() {
        for (auto const& kv : RecentWorldRemovals)
            kv.first->Release();
        RecentWorldRemovals.clear();
        for (auto const& kv : ReportedStillInWorld)
            kv.first->Release();
        ReportedStillInWorld.clear();
        ChestDirty.clear();
        ChestLocks.clear();
        ChestOpenSnapshot.clear();
        ChestPendingDiffs.clear();
        ItemBulk = CoopItemBulk();
        PendingGone.clear();   // u70: der geladene Stand gilt, der Weltbestand gleicht gleich ab
        WorldRequestSent = false;
        WorldLoadedMs = GetCurrentMs();
        WorldBulkAtMs = ServerThread ? WorldLoadedMs + 3000 : 0;
    }

    static void CoopInstallDisableVobHook();
    void CoopResyncRequest(const char* why);   // CoopSaveSync.cpp (v39)

    // v39: nach "Restoring world synchronization" den Weltbestand gleich neu anfragen
    void CoopWorldSyncRequestAgain() {
        WorldRequestSent = false;
        WorldLoadedMs = CurrentMs - 3000;
    }

    void CoopWorldSyncPulse() {
        CoopInstallDisableVobHook();   // einmal, im Hauptfaden, nach dem Laden aller Plugins
        ExpireWorldRemovals();
        if (MobSyncBase())
            MobPendingFlush();   // u137: in dieser Welt Benutztes, waehrend ich anderswo war
        // v75: Regel "Gegenstaende/Truhen" - wieder an: Bestand neu anfragen; aus: den vollen Stand (Lage, HP, Ausruestung) nach dem
        // Laden/Verbinden trotzdem anfordern (hing bisher an der Weltbestand-Anfrage)
        static bool ruleWas = true;
        const bool rule = CoopRuleWorld();
        if (rule && !ruleWas)
            CoopWorldSyncRequestAgain();
        ruleWas = rule;
        if (!rule && WorldSyncBase() && ClientThread && !ServerThread && !WorldRequestSent && CurrentMs > WorldLoadedMs + 3000) {
            WorldRequestSent = true;
            WorldSyncLog("Gegenst" "\xE4" "nde/Truhen nicht abgeglichen (Einstellung) - nur den vollen Stand angefordert");
            CoopResyncRequest("nach dem Laden bzw. Verbinden");
        }
        if (!WorldSyncAktiv()) {
            ChestDirty.clear();
            LootRemovals.clear();
            PlayerReceived.clear();
            return;
        }
        FlushChestDirty();
        FlushNpcLoot();
        if (!PendingGone.empty() && CurrentMs >= PendingGoneCheckMs && !IsLoadingLevel) {   // u70: verspaetete Entfernungen
            PendingGoneCheckMs = CurrentMs + 2000;
            for (size_t i = PendingGone.size(); i-- > 0;) {
                const CoopPendingGone& g = PendingGone[i];
                oCItem* it = FindLooseItem(g.inst, g.pos, 200.f);
                if (it) {
                    RemoveWorldItem(it);
                    WorldSyncLog("%s nahm %s bei %s - hier nachtraeglich entfernt (%lld s nach der Meldung)", g.by.c_str(), g.inst.c_str(),
                        CoopPosKey(g.pos).c_str(), (CurrentMs - g.sinceMs) / 1000);
                }
                if (it || CurrentMs > g.sinceMs + 120000)
                    PendingGone.erase(PendingGone.begin() + i);
            }
        }
        if (ClientThread && !WorldRequestSent && CurrentMs > WorldLoadedMs + 3000) {
            json j;
            WorldSyncSend(j, SYNC_WORLD_REQUEST);
            WorldRequestSent = true;
            WorldSyncLog("Weltbestand beim Host angefragt (%s)", CoopWorldName().c_str());
            CoopResyncRequest("nach dem Laden bzw. Verbinden");   // v39: Lage, HP, Ausruestung aller Figuren/NPCs neu
        }
        if (ServerThread && WorldBulkAtMs && CurrentMs >= WorldBulkAtMs) {
            WorldBulkAtMs = 0;
            SendWorldBulk();
        }
    }

    // --- Hooks ----------------------------------------------------------------------------------

    void __fastcall oCMobContainer_Open_Coop(oCMobContainer* _this, void* vtable, oCNpc* npc);
    CInvoke<void(__thiscall*)(oCMobContainer*, oCNpc*)> Ivk_oCMobContainer_Open_Coop(CoopAddr::oCMobContainer_Open, &oCMobContainer_Open_Coop, COOP_IVK(World));
    void __fastcall oCMobContainer_Open_Coop(oCMobContainer* _this, void* vtable, oCNpc* npc) {
        const bool warOffen = _this && _this->items;
        Ivk_oCMobContainer_Open_Coop(_this, npc);
        if (_this && _this != CoopTruheObj && !warOffen && _this->items && npc && npc == player && WorldSyncAktiv())   // u97
            ChestOpenedLocal(_this);
    }

    void __fastcall oCMobContainer_Close_Coop(oCMobContainer* _this, void* vtable, oCNpc* npc);
    CInvoke<void(__thiscall*)(oCMobContainer*, oCNpc*)> Ivk_oCMobContainer_Close_Coop(CoopAddr::oCMobContainer_Close, &oCMobContainer_Close_Coop, COOP_IVK(World));
    void __fastcall oCMobContainer_Close_Coop(oCMobContainer* _this, void* vtable, oCNpc* npc) {
        Ivk_oCMobContainer_Close_Coop(_this, npc);
        if (_this && _this != CoopTruheObj && npc && npc == player)   // u97: die Coop-Truhe gleicht CoopTruhe.cpp ab
            ChestClosedLocal(_this);
    }

    // v14: Beim Aufheben entfernt die Engine den Gegenstand NICHT mit RemoveVob, sondern schaltet ihn mit
    // oCWorld::DisableVob ab (vtable +70h; DoTakeVob 0x006A0DB2, PutInInv 0x006A5135; keine direkten
    // Aufrufer). Den Eingang (0x006D7250) hat schon der D3D11-Renderer (g1_avx2.dll, E9-Sprung) - darum
    // nicht dort einhaken, sondern den vtable-Platz umbiegen und an den bisherigen Inhalt weiterreichen.
    typedef void(__thiscall* CoopDisableVobFn)(oCWorld*, zCVob*);
    static CoopDisableVobFn CoopOrigDisableVob = NULL;

    static void __fastcall CoopDisableVobHook(oCWorld* world, void* edx, zCVob* vob) {
        CoopWorldSyncItemLeavesWorld(vob, _ReturnAddress(), "DisableVob");
        CoopOrigDisableVob(world, vob);
    }

    static void CoopInstallDisableVobHook() {
        static bool done = false;
        if (done)
            return;
        done = true;
        DWORD* const vtable = (DWORD*)CoopAddr::vt_oCWorld();   // oCWorld-vtable (G1 0x007DE8CC), Platz 0 = _GetClassDef
        DWORD* const slot = vtable + CoopAddr::vtslot_oCWorld_DisableVob / 4;   // u2: G1 +0x70, G2/G2A +0x74 (SDK)
        if (!IsReadableMemory(vtable, CoopAddr::vtslot_oCWorld_DisableVob + 4) || vtable[0] != CoopAddr::oCWorld_GetClassDef) {
            OthelloCompatLog("Welt-Sync: oCWorld-vtable nicht erkannt (%08X) - DisableVob nicht eingehakt", IsReadableMemory(vtable, 4) ? vtable[0] : 0);
            return;
        }
        const DWORD before = *slot;
        DWORD old = 0;
        if (!VirtualProtect(slot, 4, PAGE_READWRITE, &old)) {
            OthelloCompatLog("Welt-Sync: VirtualProtect fuer DisableVob-Platz fehlgeschlagen (%lu)", GetLastError());
            return;
        }
        CoopOrigDisableVob = (CoopDisableVobFn)before;
        *slot = (DWORD)&CoopDisableVobHook;
        VirtualProtect(slot, 4, old, &old);
        char mod[96];
        CoopModuleOf((const void*)before, mod, sizeof(mod));
        // u13: Platz aus der Engine-Tabelle (bis u12 stand hier fest die G1-Stelle 007DE93C; G2A 0083E1E8)
        OthelloCompatLog("Welt-Sync: oCWorld::DisableVob (vtable-Platz %08X) eingehakt, bisher %08X (%s)", (unsigned)(DWORD)slot, before, mod);
    }

    // v14: Gegenstand kommt ins Inventar (Quickloot ruft das direkt, die Engine ueber die vtable ->
    // zUnionUtils -> Original).
    oCItem* __fastcall oCNpcInventory_Insert_Coop(oCNpcInventory* _this, void* vtable, oCItem* item);
    CInvoke<oCItem*(__thiscall*)(oCNpcInventory*, oCItem*)> Ivk_oCNpcInventory_Insert_Coop(CoopAddr::oCNpcInventory_Insert, &oCNpcInventory_Insert_Coop, COOP_IVK(World));
    oCItem* __fastcall oCNpcInventory_Insert_Coop(oCNpcInventory* _this, void* vtable, oCItem* item) {
        if (_this && item)
            ItemIntoInventory(_this->owner, item, "Inventar-Insert", _ReturnAddress(), true);
        return Ivk_oCNpcInventory_Insert_Coop(_this, item);
    }

    // Zweite Stelle (normales Aufheben: DoTakeVob -> RemoveVob -> PutInInv), falls Insert nicht erreicht wird.
    oCItem* __fastcall oCNpc_PutInInv_Coop(oCNpc* _this, void* vtable, oCItem* item);
    CInvoke<oCItem*(__thiscall*)(oCNpc*, oCItem*)> Ivk_oCNpc_PutInInv_Coop(CoopAddr::oCNpc_PutInInv, &oCNpc_PutInInv_Coop, COOP_IVK(World));
    oCItem* __fastcall oCNpc_PutInInv_Coop(oCNpc* _this, void* vtable, oCItem* item) {
        const void* ret = _ReturnAddress();
        oCItem* r = Ivk_oCNpc_PutInInv_Coop(_this, item);
        if (_this && item && RecentWorldRemovals.count(item))   // nur Zeigervergleich, die Liste haelt eine Referenz
            ItemIntoInventory(_this, item, "PutInInv", ret, false);
        return r;
    }

    // Truhen ohne Fenster (Quickloot): Inhalt vor der Aenderung merken, gemeldet wird im naechsten Frame.
    void __fastcall oCMobContainer_Insert_Coop(oCMobContainer* _this, void* vtable, oCItem* item);
    CInvoke<void(__thiscall*)(oCMobContainer*, oCItem*)> Ivk_oCMobContainer_Insert_Coop(CoopAddr::oCMobContainer_Insert, &oCMobContainer_Insert_Coop, COOP_IVK(World));
    void __fastcall oCMobContainer_Insert_Coop(oCMobContainer* _this, void* vtable, oCItem* item) {
        if (_this && _this == CoopTruheObj) {   // u97
            Ivk_oCMobContainer_Insert_Coop(_this, item);
            return;
        }
        ChestChangeBegin(_this);
        Ivk_oCMobContainer_Insert_Coop(_this, item);
        ChestChangeEnd();
    }

    void __fastcall oCMobContainer_Remove_Coop(oCMobContainer* _this, void* vtable, oCItem* item);
    CInvoke<void(__thiscall*)(oCMobContainer*, oCItem*)> Ivk_oCMobContainer_Remove_Coop(CoopAddr::oCMobContainer_Remove, &oCMobContainer_Remove_Coop, COOP_IVK(World));
    void __fastcall oCMobContainer_Remove_Coop(oCMobContainer* _this, void* vtable, oCItem* item) {
        if (_this && _this == CoopTruheObj) {   // u97
            Ivk_oCMobContainer_Remove_Coop(_this, item);
            return;
        }
        ChestChangeBegin(_this);
        Ivk_oCMobContainer_Remove_Coop(_this, item);
        ChestChangeEnd();
    }

    oCItem* __fastcall oCMobContainer_RemoveN_Coop(oCMobContainer* _this, void* vtable, oCItem* item, int n);
    CInvoke<oCItem*(__thiscall*)(oCMobContainer*, oCItem*, int)> Ivk_oCMobContainer_RemoveN_Coop(CoopAddr::oCMobContainer_RemoveN, &oCMobContainer_RemoveN_Coop, COOP_IVK(World));
    oCItem* __fastcall oCMobContainer_RemoveN_Coop(oCMobContainer* _this, void* vtable, oCItem* item, int n) {
        if (_this && _this == CoopTruheObj)   // u97
            return Ivk_oCMobContainer_RemoveN_Coop(_this, item, n);
        ChestChangeBegin(_this);
        oCItem* r = Ivk_oCMobContainer_RemoveN_Coop(_this, item, n);
        ChestChangeEnd();
        return r;
    }

    // v15: Aus dem Inventar eines toten/bewusstlosen NPCs genommen (Pluender-Fenster: oCNpcContainer::Remove
    // 0x0066B310 -> RemoveFromInv -> vtable +34h; Quickloot: RemoveFromInv). Nur der aeussere Aufruf zaehlt.
    oCItem* __fastcall oCNpcInventory_RemoveN_Coop(oCNpcInventory* _this, void* vtable, oCItem* item, int n);
    CInvoke<oCItem*(__thiscall*)(oCNpcInventory*, oCItem*, int)> Ivk_oCNpcInventory_RemoveN_Coop(CoopAddr::oCNpcInventory_RemoveN, &oCNpcInventory_RemoveN_Coop, COOP_IVK(World));
    oCItem* __fastcall oCNpcInventory_RemoveN_Coop(oCNpcInventory* _this, void* vtable, oCItem* item, int n) {
        oCNpc* owner = NULL;
        const bool rec = NpcInvHookDepth == 0 && item && n > 0 && NpcLootRelevant(_this, owner);
        const std::string inst = rec ? CoopItemInst(item) : std::string();
        NpcInvHookDepth++;
        oCItem* r = Ivk_oCNpcInventory_RemoveN_Coop(_this, item, n);
        NpcInvHookDepth--;
        if (rec && r)
            NpcLootRecord(owner, inst, n);
        return r;
    }

    void __fastcall oCNpcInventory_Remove_Coop(oCNpcInventory* _this, void* vtable, oCItem* item);
    CInvoke<void(__thiscall*)(oCNpcInventory*, oCItem*)> Ivk_oCNpcInventory_Remove_Coop(CoopAddr::oCNpcInventory_Remove, &oCNpcInventory_Remove_Coop, COOP_IVK(World));
    void __fastcall oCNpcInventory_Remove_Coop(oCNpcInventory* _this, void* vtable, oCItem* item) {
        oCNpc* owner = NULL;
        const bool rec = NpcInvHookDepth == 0 && item && NpcLootRelevant(_this, owner);
        const std::string inst = rec ? CoopItemInst(item) : std::string();
        const int n = rec ? CoopItemAmount(item) : 0;
        NpcInvHookDepth++;
        Ivk_oCNpcInventory_Remove_Coop(_this, item);
        NpcInvHookDepth--;
        if (rec)
            NpcLootRecord(owner, inst, n);
    }

    // u13 (E6b, Gothic 2): Teilmenge aus dem Pluender-Fenster. G2A oCItemContainer::Remove(item, n) (0x007094E0): ist der Stapel
    // groesser als n, setzt sie nur item->amount herab (0x00709524) und legt fuer den Spieler einen neuen Gegenstand an - ohne
    // oCNpcContainer::Remove -> RemoveFromInv -> oCNpcInventory::Remove, die Haken oben sehen das nicht (ganze Stapel schon:
    // 0x0070BAE3 RemoveFromInv). G2 nimmt beim Pluendern je Tastendruck eines - Gold und Pfeile gingen so verloren.
    // Nur fuer das Fenster einer Leiche bzw. eines Bewusstlosen (vtable oCNpcContainer). G1: Adresse 0, kein Haken.
    oCItem* __fastcall oCItemContainer_RemoveN_Coop(oCItemContainer* _this, void* vtable, oCItem* item, int n);
    CInvoke<oCItem*(__thiscall*)(oCItemContainer*, oCItem*, int)> Ivk_oCItemContainer_RemoveN_Coop(CoopAddr::oCItemContainer_RemoveN,
        &oCItemContainer_RemoveN_Coop, CoopAddr::oCItemContainer_RemoveN ? COOP_IVK(World) : IVK_DISABLED);
    oCItem* __fastcall oCItemContainer_RemoveN_Coop(oCItemContainer* _this, void* vtable, oCItem* item, int n) {
        oCNpc* owner = NULL;
        bool rec = false;
        if (NpcInvHookDepth == 0 && _this && item && n > 0 && item->amount > n && *(DWORD*)_this == (DWORD)vftable_oCNpcContainer::GetAddress()) {
            owner = ((oCNpcContainer*)_this)->owner;
            oCNpcInventory* inv = owner ? &owner->inventory2 : NULL;
            oCNpc* o2 = NULL;
            rec = inv && NpcLootRelevant(inv, o2) && o2 == owner;
        }
        const std::string inst = rec ? CoopItemInst(item) : std::string();
        NpcInvHookDepth++;
        oCItem* r = Ivk_oCItemContainer_RemoveN_Coop(_this, item, n);
        NpcInvHookDepth--;
        if (rec && r)
            NpcLootRecord(owner, inst, n);
        return r;
    }

    // Truhe, die ein anderer Spieler gerade offen hat: nicht benutzbar.
    int __fastcall oCMobLockable_CanInteractWith_Coop(oCMobLockable* _this, void* vtable, oCNpc* npc);
    CInvoke<int(__thiscall*)(oCMobLockable*, oCNpc*)> Ivk_oCMobLockable_CanInteractWith_Coop(CoopAddr::oCMobLockable_CanInteractWith, &oCMobLockable_CanInteractWith_Coop,
        COOP_IVK(World));
    int __fastcall oCMobLockable_CanInteractWith_Coop(oCMobLockable* _this, void* vtable, oCNpc* npc) {
        if (npc && npc == player && !ChestLocks.empty() && WorldSyncAktiv()) {
            oCMobContainer* c = zDYNAMIC_CAST<oCMobContainer>(_this);
            if (c && !ChestOpenSnapshot.count(c)) {
                auto f = ChestLocks.find(CoopPosKey(c->GetPositionWorld()));
                if (f != ChestLocks.end()) {
                    if (CurrentMs > f->second.since + 300000) {
                        ChestLocks.erase(f);   // Sperre veraltet (5 min)
                    }
                    else {
                        if (CurrentMs > ChestBusyMsgMs + 3000) {
                            ChestBusyMsgMs = CurrentMs;
                            ChatLog(string::Combine(CoopTr("Diese Truhe benutzt gerade %s.", "%s is using this chest."), string(f->second.owner.c_str())));
                        }
                        return FALSE;
                    }
                }
            }
        }
        return Ivk_oCMobLockable_CanInteractWith_Coop(_this, npc);
    }

    // u137 (P134): Mechanismus benutzt. Alle Mobs laufen hier durch (oCMobLockable::OnEndStateChange ruft das Original mit auf). Der eigene Held immer;
    // NPCs des Hosts nur mit Ausloeser - Tueren benutzen sie je PC fuer sich, ein Hebel aendert die Welt. NPCs beim Client sind Abbilder.
    void __fastcall oCMobInter_OnEndStateChange_Coop(oCMobInter* _this, void* vtable, oCNpc* npc, int from, int to);
    CInvoke<void(__thiscall*)(oCMobInter*, oCNpc*, int, int)> Ivk_oCMobInter_OnEndStateChange_Coop(CoopAddr::oCMobInter_OnEndStateChange, &oCMobInter_OnEndStateChange_Coop,
        COOP_IVK(World));
    void __fastcall oCMobInter_OnEndStateChange_Coop(oCMobInter* _this, void* vtable, oCNpc* npc, int from, int to) {
        Ivk_oCMobInter_OnEndStateChange_Coop(_this, npc, from, to);
        if (!_this || !npc || from == to || !MobSyncBase() || !MobSyncRelevant(_this))
            return;
        if (npc == player || (ServerThread && !PlayerNpcs.count(npc) && !_this->triggerTarget.IsEmpty()))
            MobSendUse(_this, from, to);
    }

    static bool MobChainNow(zCVob* instigator) {
        return !instigator && MobChainUntilMs && GetCurrentMs() < MobChainUntilMs;
    }

    // Skript in der Kette beim Partner: nur weiterreichen (zCTrigger::TriggerTarget), nicht ausfuehren
    void __fastcall oCTriggerScript_TriggerTarget_Coop(oCTriggerScript* _this, void* vtable, zCVob* instigator);
    CInvoke<void(__thiscall*)(oCTriggerScript*, zCVob*)> Ivk_oCTriggerScript_TriggerTarget_Coop(CoopAddr::oCTriggerScript_TriggerTarget, &oCTriggerScript_TriggerTarget_Coop,
        COOP_IVK(World));
    void __fastcall oCTriggerScript_TriggerTarget_Coop(oCTriggerScript* _this, void* vtable, zCVob* instigator) {
        if (MobChainNow(instigator)) {
            CoopMobSkipScripts++;
            WorldSyncLog("Mechanismus: Skript %s (%s) laeuft nur beim Benutzer", _this->scriptFunc.ToChar(), _this->GetObjectName().ToChar());
            _this->zCTrigger::TriggerTarget(instigator);
            return;
        }
        Ivk_oCTriggerScript_TriggerTarget_Coop(_this, instigator);
    }

    void __fastcall zCCSCamera_OnTrigger_Coop(zCCSCamera* _this, void* vtable, zCVob* other, zCVob* instigator);
    CInvoke<void(__thiscall*)(zCCSCamera*, zCVob*, zCVob*)> Ivk_zCCSCamera_OnTrigger_Coop(CoopAddr::zCCSCamera_OnTrigger, &zCCSCamera_OnTrigger_Coop, COOP_IVK(World));
    void __fastcall zCCSCamera_OnTrigger_Coop(zCCSCamera* _this, void* vtable, zCVob* other, zCVob* instigator) {
        if (MobChainNow(instigator)) {
            CoopMobSkipCams++;
            WorldSyncLog("Mechanismus: Kamerafahrt %s nur beim Benutzer", _this->GetObjectName().ToChar());
            return;
        }
        Ivk_zCCSCamera_OnTrigger_Coop(_this, other, instigator);
    }

    void __fastcall zCCSCamera_OnUntrigger_Coop(zCCSCamera* _this, void* vtable, zCVob* other, zCVob* instigator);
    CInvoke<void(__thiscall*)(zCCSCamera*, zCVob*, zCVob*)> Ivk_zCCSCamera_OnUntrigger_Coop(CoopAddr::zCCSCamera_OnUntrigger, &zCCSCamera_OnUntrigger_Coop, COOP_IVK(World));
    void __fastcall zCCSCamera_OnUntrigger_Coop(zCCSCamera* _this, void* vtable, zCVob* other, zCVob* instigator) {
        if (MobChainNow(instigator)) {
            CoopMobSkipCams++;
            return;
        }
        Ivk_zCCSCamera_OnUntrigger_Coop(_this, other, instigator);
    }

    // u138 (P136): Absturz nach dem Laden - vom Test-Fenster (G2, u136 + b59, 31.10. 00:32): Der Weltprozess lud Slot 6 und stuerzte im ersten Bild ab, C0000005 bei
    // Gothic2.exe+0xD65D8 (zCKBSpline::Interpolate0, Lesen von 0xFFFFFFF8) aus zCCSCamera::OnTick. Slot 6 entstand 6 s, nachdem der Host den Portalring eingesetzt hatte: mitten in der Kamerafahrt.
    // zCCSCamera::Unarchive (G2A 0x004BFBC0, G1 0x004B6930) liest aus einem Spielstand "started", die Kurven posSpl/targetSpl entstehen aber nur in EV_Play; OnTick rechnet dann mit der leeren
    // Kurve. Das Spiel selbst speichert in einer Kamerafahrt nicht, gemeinsames Speichern und Auto-Sicherung schon - und so ein Slot stuerzt bei jedem Laden ab. Beim Lesen stoppen:
    // heilt auch vorhandene Spielstaende. Das Gleiche im Spielstand zu verhindern reichte nicht, die Slots gibt es schon.
    void __fastcall zCCSCamera_Unarchive_Coop(zCCSCamera* _this, void* vtable, zCArchiver& arc);
    CInvoke<void(__thiscall*)(zCCSCamera*, zCArchiver&)> Ivk_zCCSCamera_Unarchive_Coop(CoopAddr::zCCSCamera_Unarchive, &zCCSCamera_Unarchive_Coop, COOP_IVK(World));
    void __fastcall zCCSCamera_Unarchive_Coop(zCCSCamera* _this, void* vtable, zCArchiver& arc) {
        Ivk_zCCSCamera_Unarchive_Coop(_this, arc);
        if (_this && _this->started) {
            _this->started = FALSE;
            _this->paused = FALSE;
            _this->gotoTimeMode = FALSE;
            CoopCamStopped++;
            OthelloCompatLog("[%lld s, %s] Absturzschutz: Kamerafahrt %s stand im Spielstand als laufend - gestoppt (ohne ihre Kurve stuerzt sie im ersten Bild ab)", DiagSeconds(), DiagRole(),
                _this->GetObjectName().ToChar());
        }
    }

#else   // Modul in dieser Engine nicht freigegeben (CoopModulesReport)
    void CoopWorldSyncItemLeavesWorld(zCVob*, const void*, const char*) {}
    bool CoopWorldSyncHandles(int) { return false; }
    void CoopWorldSyncReceive(json&, const std::string&) {}
    void CoopWorldSyncPeerGone(const std::string&) {}
    void CoopWorldSyncWorldLoaded() {}
    void CoopWorldSyncPulse() {}
    void CoopWorldSyncRequestAgain() {}   // u12: Testlauf (G2A)
    void CoopWorldSyncSaveLoaded() {}     // u137
#endif
}
