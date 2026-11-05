namespace GOTHIC_ENGINE {
    void CoopLog(std::string l)
    {
        CoopLogArchiveOnce();   // v35
        std::ofstream CoopLog(GothicCoopLogPath, std::ios_base::app | std::ios_base::out);
        CoopLog << l;
    }

    void ChatLog(string text, zCOLOR color = zCOLOR(255, 255, 255, 255)) {
        GameChat->AddLine(text, color);
    };

    int GetFreePlayerId() {
        LastFreePlayerId += 1;
        return LastFreePlayerId;
    }

    bool IsCoopPlayer(std::string name) {
        string cStringName = name.c_str();
        return cStringName == "HOST" || cStringName.StartWith("FRIEND_");
    }

    bool IsCoopPlayer(string name) {
        return name == "HOST" || name.StartWith("FRIEND_");
    }

    // u149 (P152, Testlauf b118, Gothic 1, ITARSCROLLTRFMEATBUG): Verwandelt oder fremdgesteuert ist player das neue Wesen, der Held liegt ausserhalb der Welt (oCSpell::saveNpc). Das Spiel speichert dann nicht
    // (CGameManager::MenuEnabled, G2A 0x0042AAF0, dieselben drei Bits), GTs eigene Wege kannten das nicht
    bool CoopHeroSwapped() {
        return player && (player->HasBodyStateModifier(BS_MOD_TRANSFORMED) || player->HasBodyStateModifier(BS_MOD_CONTROLLED) || player->HasBodyStateModifier(BS_MOD_CONTROLLING));
    }

    oCItem* CreateCoopItem(int insIndex) {
        return zfactory->CreateItem(insIndex);
    }

    // --- u88 (P112): ungueltige Vobs in NPC-Slots (Haende, Waffen, Ruestung) ---------------------------------------------
    // Dauerlauf u87: der Host hing 14:10 in Gothics Fenster "An Assertion Failed" (refCtr<=0 - "tried to delete zCObject with
    // refCtr greater than 0", zObject.cpp 131): oCNpc::DoDropVob (ueber GTs Haken, Hooks.cpp) -> oCAIVobMove::Init ->
    // zCWorld::AddVob -> AddVobAsChild; dort landete der virtuelle Aufruf am Gegenstand (SetCollDetDyn) im Destruktor eines
    // oCCSPlayer - der Gegenstand war schon geloescht, sein Speicher neu belegt. Dasselbe Muster bei den Client-Abstuerzen u83
    // (oCNpc::UpdateSlots) und u84 (oCNpc::HasTorch aus oCAIHuman::ChangeCamModeBySituation, prueft jedes Bild die Haende des
    // eigenen Helden). Wer die Gegenstaende loescht, ist offen - die Zeilen "Absturzschutz: ... im Slot" zeigen kuenftig, wann und
    // bei wem. Lebendig ist ein Slot-Vob, wenn sein Referenzzaehler > 0 ist (der Slot haelt einen) und seine vtable die einer
    // Vob-Klasse, die in der Welt vorkommt (ein zerstoertes Objekt traegt die von zCObject, ein neu belegtes eine fremde). Sonst
    // traegt GT nur den Zeiger aus, ohne das Objekt anzufassen, und laesst DoDropVob aus. Allgemein (G1/G2).
    long long GetCurrentMs();   // weiter unten
    static std::vector<void*> CoopVobVtables;   // vtables lebender Vob-Klassen (sortiert, waechst nur - vtables sind fest)
    static bool CoopVobVtablesItem = false;     // eine oCItem-vtable ist dabei - erst dann zaehlt "fremde vtable"
    static long long CoopVobVtablesMs = -100000;

    // vtable und Referenzzaehler (zCObject +04h) lesen, ohne bei einem ungueltigen Zeiger abzustuerzen
    static bool CoopVobPeek(const void* p, void** vt, int* ref) {
        __try {
            *vt = *(void* const*)p;
            *ref = ((const zCObject*)p)->refCtr;
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    // u89 (P113): dasselbe fuer Visuals und AIs - AddVobAsChild ruft visual->HostVobAddedToWorld und ai->HostVobAddedToWorld
    // (dort landete der Aufruf in u87/u88 im Destruktor eines oCCSPlayer, obwohl der Gegenstand selbst lebte)
    static std::vector<void*> CoopVisualVtables, CoopAiVtables;

    static bool CoopVtableAdd(std::vector<void*>& set, void* vt) {
        auto it = std::lower_bound(set.begin(), set.end(), vt);
        if (it != set.end() && *it == vt)
            return false;
        set.insert(it, vt);
        return true;
    }

    // vtable eines Teils lernen, wenn es lesbar ist und lebt (Referenzzaehler > 0)
    static void CoopPartLearn(std::vector<void*>& set, const void* p) {
        void* vt = NULL;
        int ref = 0;
        if (p && CoopVobPeek(p, &vt, &ref) && ref > 0)
            CoopVtableAdd(set, vt);
    }

    // --- u166 (P173, Liste 1c): Slot-Schutz kennt alle Vob-Klassen und erkennt neu belegten Speicher ----------------------------
    // Bis u165 lernte CoopVobVtablesBuild mit SearchVobListByClass(zCVob::classDef) - das nimmt nur genau diese Klasse, Gegenstaende,
    // NPCs, Mobs kamen nie dazu. CoopVobVtablesItem blieb falsch, "fremde vtable" griff nie; der Schutz pruefte nur Lesbarkeit und
    // Zaehler. Ein freigegebener Block, in dem schon ein Netz-Paket liegt (P142: "{\"by" statt vtable), hat einen lesbaren, oft
    // positiven "Zaehler" und kam durch. Jetzt: (1) jede vtable muss im Speicher eines geladenen Moduls liegen (VirtualQuery, MEM_IMAGE,
    // gemerkt) - Daten auf dem Heap tun das nie, echte Objekte immer, also ohne Fehlalarm, auch im Takt; (2) gelernt wird der ganze
    // Vob-Baum der Welt und das eigene Inventar, "fremde vtable" (ein lebendes Objekt einer Klasse, die in der Welt nicht vorkommt) gilt
    // nur vor DoDropVob/RemoveVobFromWorld (strict), wo die Engine den Vob gleich aufruft - im Takt verschwindet so kein echter
    // Gegenstand aus einem Slot. Allgemein (G1/G2)
    static std::vector<void*> CoopImageVtables;     // vtables im Speicher eines geladenen Moduls (sortiert)
    static std::vector<void*> CoopUnknownVtables;   // nach einem Neulernen noch fremd - dafuer nicht wieder neu lernen

    static bool CoopVtableInImage(void* vt) {
        if (std::binary_search(CoopImageVtables.begin(), CoopImageVtables.end(), vt))
            return true;
        MEMORY_BASIC_INFORMATION mbi;
        if (!vt || !VirtualQuery(vt, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT || mbi.Type != MEM_IMAGE)
            return false;
        CoopVtableAdd(CoopImageVtables, vt);
        return true;
    }

    static void CoopVobLearn(zCVob* v) {
        if (!v)
            return;
        CoopPartLearn(CoopVisualVtables, v->visual);   // u89
        CoopPartLearn(CoopAiVtables, v->callback_ai);
        if (CoopVtableAdd(CoopVobVtables, *(void**)v) && !CoopVobVtablesItem && zDYNAMIC_CAST<oCItem>(v))
            CoopVobVtablesItem = true;
    }

    static void CoopVobTreeLearn(zCTree<zCVob>* node) {
        for (; node; node = node->GetNextChild()) {
            CoopVobLearn(node->GetData());
            CoopVobTreeLearn(node->GetFirstChild());
        }
    }

    static void CoopVobVtablesBuild() {
        CoopVobVtablesMs = GetCurrentMs();
        oCWorld* w = ogame ? ogame->GetGameWorld() : NULL;
        if (!w)
            return;
        CoopVobTreeLearn(w->globalVobTree.GetFirstChild());
        if (player) {   // Gegenstaende auch dann, wenn keiner in der Welt liegt
#if ENGINE >= Engine_G2
            for (zCListSort<oCItem>* n = player->inventory2.inventory.next; n; n = n->next)
                CoopVobLearn(n->data);
#else
            for (int c = 0; c < INV_MAX; c++)
                for (zCListSort<oCItem>* n = player->inventory2.inventory[c].next; n; n = n->next)
                    CoopVobLearn(n->data);
#endif
        }
    }

    // NULL = v lebt, sonst der Grund. strict: auch ein lebendes Objekt einer Klasse, die in der Welt nicht vorkommt, gilt als tot
    // (vor DoDropVob, wo die Engine den Vob gleich aufruft); sonst nur sichere Zeichen (u166, P173)
    static const char* CoopVobDeadWhy(const void* v, void** vt, int* ref, bool strict) {
        *vt = NULL;
        *ref = 0;
        if (!CoopVobPeek(v, vt, ref))
            return "nicht lesbar";
        if (*ref <= 0)
            return "Referenzzaehler <= 0";
        if (!CoopVtableInImage(*vt))
            return "vtable ausserhalb des Programms";
        if (!strict || std::binary_search(CoopVobVtables.begin(), CoopVobVtables.end(), *vt))
            return NULL;
        // neue Klasse oder andere Welt: hoechstens einmal je Sekunde neu lernen, fuer eine schon erfolglos gesuchte vtable nicht wieder
        if (GetCurrentMs() - CoopVobVtablesMs > 1000 && !std::binary_search(CoopUnknownVtables.begin(), CoopUnknownVtables.end(), *vt)) {
            CoopVobVtablesBuild();
            if (!std::binary_search(CoopVobVtables.begin(), CoopVobVtables.end(), *vt))
                CoopVtableAdd(CoopUnknownVtables, *vt);
        }
        if (!CoopVobVtablesItem || std::binary_search(CoopVobVtables.begin(), CoopVobVtables.end(), *vt))
            return NULL;
        return "fremde vtable";
    }

    static std::string CoopSlotOwnerName(oCNpc* npc) {
        return npc == player ? std::string("eigenem Helden") : std::string(npc->GetObjectName().ToChar());
    }

    // u89: NULL = Teil (Visual/AI) lebt, sonst der Grund; ohne gelernte vtables gilt es als lebendig
    static const char* CoopPartDeadWhy(const void* p, std::vector<void*>& set, void** vt, int* ref) {
        *vt = NULL;
        *ref = 0;
        if (!CoopVobPeek(p, vt, ref))
            return "nicht lesbar";
        if (*ref <= 0)
            return "Referenzzaehler <= 0";
        if (!CoopVtableInImage(*vt))   // u166 (P173): neu belegter Speicher - ein sicheres Zeichen
            return "vtable ausserhalb des Programms";
        if (set.empty() || std::binary_search(set.begin(), set.end(), *vt))
            return NULL;
        if (GetCurrentMs() - CoopVobVtablesMs > 1000 && !std::binary_search(CoopUnknownVtables.begin(), CoopUnknownVtables.end(), *vt)) {
            CoopVobVtablesBuild();   // u166: ganzer Vob-Baum - fuer eine schon erfolglos gesuchte vtable nicht wieder
            if (!std::binary_search(set.begin(), set.end(), *vt))
                CoopVtableAdd(CoopUnknownVtables, *vt);
        }
        return std::binary_search(set.begin(), set.end(), *vt) ? NULL : "fremde vtable";
    }

    // u89 (P113): Visual und AI eines lebenden Vobs pruefen - tote nur austragen (ohne Release); ein Gegenstand laedt sein Modell
    // ueber seinen Dateinamen neu (oCItem::CreateVisual), dessen vtable gilt dann als bekannt. strict: auch eine fremde vtable gilt
    // als tot (vor DoDropVob, wo der Aufruf landet); sonst nur sichere Zeichen (nicht lesbar, Zaehler <= 0) - eine seltene, aber
    // gueltige Visual-Klasse (in der Welt nicht vorhanden) soll im Takt jedes Bildes nicht verschwinden. Rueckgabe: etwas ausgetragen
    static bool CoopVobPartsCheck(zCVob* v, const char* where, oCNpc* owner, bool strict) {
        if (!v)
            return false;
        if (CoopVobVtables.empty())
            CoopVobVtablesBuild();
        bool fixed = false;
        void* vt = NULL;
        int ref = 0;
        static CoopDiagBudget budget = { 0, 0 };
        auto dead = [&](const void* p, std::vector<void*>& set) -> const char* {
            const char* why = CoopPartDeadWhy(p, set, &vt, &ref);
            return why && (strict || strcmp(why, "fremde vtable") != 0) ? why : NULL;
        };
        if (v->visual) {
            if (const char* why = dead(v->visual, CoopVisualVtables)) {
                void* old = v->visual;
                v->visual = NULL;
                oCItem* it = zDYNAMIC_CAST<oCItem>(v);
                if (it)
                    it->CreateVisual();
                CoopPartLearn(CoopVisualVtables, v->visual);
                fixed = true;
                if (CoopDiagAllow(budget, 20))
                    OthelloCompatLog("[%lld s, %s] Absturzschutz: %s an %s - %s %s mit ungueltigem Visual %p (%s; vtable %p, Zaehler %d) - "
                        "ausgetragen%s", DiagSeconds(), DiagRole(), where, owner ? CoopSlotOwnerName(owner).c_str() : "-",
                        it ? "Gegenstand" : "Vob", it ? it->GetInstanceName().ToChar() : v->GetObjectName().ToChar(), old, why, vt, ref,
                        v->visual ? ", Modell neu geladen" : "");
            }
        }
        if (v->callback_ai) {
            if (const char* why = dead(v->callback_ai, CoopAiVtables)) {
                void* old = v->callback_ai;
                v->callback_ai = NULL;
                fixed = true;
                if (CoopDiagAllow(budget, 20))
                    OthelloCompatLog("[%lld s, %s] Absturzschutz: %s an %s - %s mit ungueltiger AI %p (%s; vtable %p, Zaehler %d) - ausgetragen",
                        DiagSeconds(), DiagRole(), where, owner ? CoopSlotOwnerName(owner).c_str() : "-", v->GetObjectName().ToChar(), old,
                        why, vt, ref);
            }
        }
        return fixed;
    }

    // u89: nach DoDropVob - die neue Fall-AI (oCAIVobMove) und das Visual gelten als bekannt
    void CoopVobPartsLearn(zCVob* v) {
        if (!v)
            return;
        CoopPartLearn(CoopVisualVtables, v->visual);
        CoopPartLearn(CoopAiVtables, v->callback_ai);
    }

    // tote Vobs aus den Slots von npc austragen (nur der Zeiger - das Objekt gibt es nicht mehr); only: nur dieser Vob
    static int CoopScrubDeadSlots(oCNpc* npc, const void* only, bool quiet) {
        if (!npc)
            return 0;
        int n = 0;
        for (int i = 0; i < npc->invSlot.GetNumInList(); i++) {
            TNpcSlot* s = npc->invSlot.GetSafe(i);
            if (!s || !s->vob || (only && (const void*)s->vob != only))
                continue;
            void* vt = NULL;
            int ref = 0;
            const char* why = CoopVobDeadWhy(s->vob, &vt, &ref, false);
            if (!why) {
                if (!only)   // u89: Visual/AI des lebenden Slot-Vobs (nur sichere Zeichen)
                    CoopVobPartsCheck(s->vob, (std::string("Slot ") + s->name.ToChar()).c_str(), npc, false);
                continue;
            }
            static CoopDiagBudget budget = { 0, 0 };
            if (!quiet && CoopDiagAllow(budget, 20))
                OthelloCompatLog("[%lld s, %s] Absturzschutz: ungueltiger Vob %p im Slot %s von %s (%s; vtable %p, Zaehler %d, "
                    "Gegenstand %s) - nur ausgetragen", DiagSeconds(), DiagRole(), (void*)s->vob, s->name.ToChar(),
                    CoopSlotOwnerName(npc).c_str(), why, vt, ref, s->itemName.IsEmpty() ? "-" : s->itemName.ToChar());
            s->vob = NULL;
            n++;
        }
        return n;
    }

    // vor DoDropVob/RemoveVobFromWorld an einem Vob von npc: lebt er? Sonst aus dessen Slots austragen und false
    bool CoopSlotVobAlive(oCNpc* npc, const void* vob, const char* where) {
        if (!vob)
            return false;
        void* vt = NULL;
        int ref = 0;
        const char* why = CoopVobDeadWhy(vob, &vt, &ref, true);
        if (!why) {
            CoopVobPartsCheck((zCVob*)vob, where, npc, true);   // u89 (P113): Visual/AI, die AddVobAsChild gleich aufruft
            return true;
        }
        const int n = CoopScrubDeadSlots(npc, vob, true);
        static CoopDiagBudget budget = { 0, 0 };
        if (CoopDiagAllow(budget, 20))
            OthelloCompatLog("[%lld s, %s] Absturzschutz: %s an %s mit ungueltigem Vob %p (%s; vtable %p, Zaehler %d) - ausgelassen%s",
                DiagSeconds(), DiagRole(), where, npc ? CoopSlotOwnerName(npc).c_str() : "-", vob, why, vt, ref,
                n ? ", aus dem Slot ausgetragen" : "");
        return false;
    }

    // u142 (P142): oCMsgManipulate::targetVob haelt keinen Verweis. Testlaeufe G1 und Othello, immer beim Client (u134, u136; sechs
    // Abstuerze bei GothicMod.exe+0x2AFF9E, z. B. Lesen von 0x7962227B = "{\"by" - dort lag schon ein Netz-Paket): Der Gegenstand eines
    // laufenden EV_USEITEMTOSTATE war frei und oCNpc::interactItem leer. Die Engine holt ihn dann neu aus der Nachricht und ruft seine
    // vtable. Es traf nie den eigenen Helden (dann stuerzte schon TruheIstGegenstand). Wer freigibt, ist offen - die Zeile nennt NPC,
    // Gegenstand und Zustand. Ohne Zeiger beendet die Engine die Nachricht selbst. Allgemein: G2A ist gleich gebaut, nur traf es dort
    // bisher keinen neu belegten Speicher
    int CoopMsgVobDeadCount = 0;

    // vtable von oCItem aus dem ersten Gegenstand der Welt (von oCItem leitet nichts ab). SearchVobListByClass nimmt nur genau die
    // angegebene Klasse: CoopVobVtablesBuild findet mit zCVob::classDef keine Gegenstaende, "fremde vtable" greift dort darum nicht
    static void* CoopItemVtable() {
        static void* vt = NULL;
        static long long nextMs = 0;
        const long long now = GetCurrentMs();
        if (vt || now < nextMs || !ogame || !ogame->GetGameWorld())
            return vt;
        nextMs = now + 1000;
        zCArray<zCVob*> items;
        ogame->GetGameWorld()->SearchVobListByClass(oCItem::classDef, items, NULL);
        if (items.GetNumInList() > 0 && items.GetSafe(0))
            vt = *(void**)items.GetSafe(0);
        return vt;
    }

    void CoopMsgVobCheck(oCNpc* npc, oCMsgManipulate* msg, const char* where) {
        if (!msg || !msg->targetVob)
            return;
        void* vt = NULL;
        int ref = 0;
        const char* why = !CoopVobPeek(msg->targetVob, &vt, &ref) ? "nicht lesbar" : ref <= 0 ? "Referenzzaehler <= 0" :
            !CoopVtableInImage(vt) ? "vtable ausserhalb des Programms" : NULL;   // u166 (P173)
        if (!why && CoopItemVtable() && vt != CoopItemVtable())
            why = "fremde vtable";
        if (!why)
            return;
        CoopMsgVobDeadCount++;
        static CoopDiagBudget budget = { 0, 0 };
        if (CoopDiagAllow(budget, 20))
            OthelloCompatLog("[%lld s, %s] Absturzschutz: %s an %s - Gegenstand \"%s\" der Nachricht ist schon frei (%p, %s; vtable %p, Zaehler %d), "
                "Zustand %s - Zeiger geloescht", DiagSeconds(), DiagRole(), where, npc ? CoopSlotOwnerName(npc).c_str() : "-", msg->name.ToChar(),
                (void*)msg->targetVob, why, vt, ref, npc ? npc->state.curState.name.ToChar() : "-");
        msg->targetVob = NULL;
    }

    // u169 (P176, Liste 1): Wer gibt den Gegenstand einer laufenden Nachricht frei? Testlaeufe b258/b259: Gothic 1 etwa 15 Faelle je Minute
    // ("Referenzzaehler <= 0", vor allem Buddler beim Essen im Alten Lager), Othello weniger, Gothic II selten. Der Haken auf
    // oCNpc::EV_UseItemToState merkt sich jeden Gegenstand mit NPC und Nachricht (CoopMsgItemNote). Gibt die Engine ihn frei (oCItem::~oCItem,
    // CoopTruhe.cpp), waehrend die Nachricht noch in der Befehlsliste des NPCs steht und nicht gerade selbst laeuft (das waere der normale
    // Verbrauch beim Benutzen), schreibt GT NPC, Gegenstand, Zustand und die Aufrufer ins Log. Nur Diagnose, aendert nichts
    struct CoopMsgItemSeen { oCNpc* npc; const void* msg; std::string npcName, item, state; long long ms; };
    static std::map<const void*, CoopMsgItemSeen> CoopMsgItems;
    const void* CoopMsgItemInUse = NULL;   // Gegenstand der Nachricht, die gerade laeuft
    int CoopMsgItemFreed = 0;
    std::string CoopMsgItemLast;           // letzte Zeile (Testschritt)

    void CoopMsgItemNote(oCNpc* npc, oCMsgManipulate* msg) {
        if (!npc || !msg || !msg->targetVob)
            return;
        const long long now = GetCurrentMs();
        if (CoopMsgItems.size() > 256)
            for (auto it = CoopMsgItems.begin(); it != CoopMsgItems.end();)
                it = now - it->second.ms > 30000 ? CoopMsgItems.erase(it) : std::next(it);
        CoopMsgItemSeen& e = CoopMsgItems[msg->targetVob];
        if (e.npc != npc || e.msg != msg) {
            e.npc = npc;
            e.msg = msg;
            e.npcName = CoopSlotOwnerName(npc);
            e.item = msg->name.ToChar();
            e.state = npc->state.curState.name.ToChar();
        }
        e.ms = now;
    }

    // Ruecksprungadressen auf dem Stapel (Wert in einem Modul, davor ein call), innen zuerst - wie im Weltprozess (CoopWorldProcess.cpp)
    static void CoopStackTrail(char* out, size_t size, int max) {
        size_t len = 0;
        out[0] = 0;
        const DWORD* p = (const DWORD*)_AddressOfReturnAddress();
        const NT_TIB* tib = (const NT_TIB*)NtCurrentTeb();
        const DWORD* top = (const DWORD*)tib->StackBase;
        int found = 0;
        for (int words = 0; p < top && found < max && words < 2048; p++, words++) {
            const DWORD v = *p;
            HMODULE m = NULL;
            if (v < 0x10000 || !GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                (LPCSTR)(ULONG_PTR)v, &m) || !m || v < (DWORD)(ULONG_PTR)m + 0x1000)
                continue;
            MEMORY_BASIC_INFORMATION mbi;
            if (!VirtualQuery((LPCVOID)(ULONG_PTR)(v - 6), &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT ||
                !(mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)))
                continue;
            const unsigned char* c = (const unsigned char*)(ULONG_PTR)v;
            const bool call = c[-5] == 0xE8 || (c[-6] == 0xFF && (c[-5] & 0x38) == 0x10) || (c[-2] == 0xFF && (c[-1] & 0x38) == 0x10) ||
                (c[-3] == 0xFF && (c[-2] & 0x38) == 0x10);
            if (!call)
                continue;
            char mod[128];
            CoopModuleOf((const void*)(ULONG_PTR)v, mod, sizeof(mod));
            const int n = _snprintf(out + len, size - len - 1, "%s%s", found ? " < " : "", mod);
            if (n < 0)
                break;
            len += (size_t)n;
            found++;
        }
        out[size - 1] = 0;
        if (!found)
            _snprintf(out, size - 1, "keine gefunden");
    }

    void CoopMsgItemsWorldGone() {   // LoadBegin (Plugin.cpp): die Zeiger gehoeren zur alten Welt
        CoopMsgItems.clear();
        CoopMsgItemInUse = NULL;
    }

    void CoopMsgItemFreeing(const void* item) {
        auto it = CoopMsgItems.find(item);
        if (it == CoopMsgItems.end())
            return;
        const CoopMsgItemSeen e = it->second;
        CoopMsgItems.erase(it);
        if (item == CoopMsgItemInUse || !player)
            return;   // in der eigenen Nachricht verbraucht
        void* vt = NULL;
        int ref = 0;
        if (!CoopVobPeek(e.npc, &vt, &ref) || ref <= 0 || vt != *(void**)player)
            return;   // der NPC ist schon weg (oder an seiner Stelle liegt etwas anderes)
        zCEventManager* em = e.npc->eventManager;   // nicht GetEM: das legte eine Liste an
        bool held = false;
        for (int i = 0; em && i < em->messageList.GetNumInList() && !held; i++)
            held = (const void*)em->messageList[i] == e.msg;
        if (!held)
            return;   // Nachricht schon fertig
        CoopMsgItemFreed++;
        static char trail[900], line[1400];   // nicht auf dem Stapel - sonst liest CoopStackTrail alte Werte darin mit
        CoopStackTrail(trail, sizeof(trail), 16);
        _snprintf(line, sizeof(line), "Gegenstand \"%s\" (%p) der laufenden Nachricht von %s wird freigegeben, Zustand %s (beim Merken %s, vor %lld ms) - "
            "Aufrufer: %s", e.item.c_str(), item, e.npcName.c_str(), e.npc->state.curState.name.ToChar(), e.state.c_str(), GetCurrentMs() - e.ms, trail);
        line[sizeof(line) - 1] = 0;
        CoopMsgItemLast = line;
        static CoopDiagBudget budget = { 0, 0 };
        if (CoopDiagAllow(budget, 40))
            OthelloCompatLog("[%lld s, %s] Absturzschutz-Diagnose: %s", DiagSeconds(), DiagRole(), line);
    }

    // --- u90 (P114): Gegenstand stirbt mitten in DoDropVob -------------------------------------------------------------------
    // Dauerlauf u88, dritter Haenger: Disassembly von zCWorld::AddVobAsChild (G2A 0x006245A0) - die Ruecksprungadresse +0x147
    // steht hinter "mov eax,[esi]; push 1; call [eax+40h]", ein virtueller Aufruf am eingefuegten Vob selbst, nicht an Visual oder
    // AI (u89 lag damit falsch). Der Waechter (P112) sah den Gegenstand vor DoDropVob lebendig - er starb also darin: DoDropVob gibt
    // ihn aus Inventar und Slot frei, oCAIVobMove::Init laesst alte Verweise los, erst dann folgt AddVob. War sein Zaehler vorher um
    // eins zu niedrig (wer zu oft freigibt, ist offen), loescht ihn eine dieser regulaeren Freigaben; AddVob lief dann ueber die kurze
    // vtable des zerstoerten zCObject hinaus in die des Cutscene-Players (Destruktor -> Assertion "refCtr<=0"). Jetzt haelt GT waehrend
    // DoDropVob einen eigenen Verweis und gibt ihn danach nur frei, wenn noch jemand haelt - sonst bleibt er (ein kleines Leck statt
    // eines geloeschten Objekts, auf das die Welt noch zeigt). Vorher zaehlt GT die Halter (Slots, Inventar, Welt) und meldet einen
    // zu niedrigen Zaehler - die Zeilen zeigen, welche Gegenstaende es trifft. Allgemein (G1/G2).
    static bool CoopInInventory(oCNpc* npc, const void* item) {
#if ENGINE >= Engine_G2
        for (zCListSort<oCItem>* n = npc->inventory2.inventory.next; n; n = n->next)
            if ((const void*)n->data == item)
                return true;
#else
        for (int c = 0; c < INV_MAX; c++)
            for (zCListSort<oCItem>* n = npc->inventory2.inventory[c].next; n; n = n->next)
                if ((const void*)n->data == item)
                    return true;
#endif
        return false;
    }

    static std::string CoopVobLabel(zCVob* v) {
        oCItem* it = zDYNAMIC_CAST<oCItem>(v);
        return it ? std::string(it->GetInstanceName().ToChar()) : std::string(v->GetObjectName().ToChar());
    }

    // vor DoDropVob: eigener Verweis; meldet, wenn der Zaehler unter der Zahl der Halter liegt
    void CoopDropHold(oCNpc* npc, zCVob* v, const char* where) {
        if (!v)
            return;
        int slots = 0, inv = 0;
        if (npc) {
            for (int i = 0; i < npc->invSlot.GetNumInList(); i++) {
                TNpcSlot* s = npc->invSlot.GetSafe(i);
                if (s && (const void*)s->vob == (const void*)v)
                    slots++;
            }
            inv = CoopInInventory(npc, v) ? 1 : 0;
        }
        const int world = v->GetHomeWorld() ? 1 : 0;
        if (v->refCtr < slots + inv + world) {
            static CoopDiagBudget budget = { 0, 0 };
            if (CoopDiagAllow(budget, 20))
                OthelloCompatLog("[%lld s, %s] Absturzschutz: %s an %s - Gegenstand %s (%p) mit Zaehler %d bei %d Haltern (Slot %d, "
                    "Inventar %d, Welt %d) - zu niedrig, GT haelt einen Verweis", DiagSeconds(), DiagRole(), where,
                    npc ? CoopSlotOwnerName(npc).c_str() : "-", CoopVobLabel(v).c_str(), (void*)v, v->refCtr, slots + inv + world, slots,
                    inv, world);
        }
        v->AddRef();
    }

    // nach DoDropVob: den eigenen Verweis nur freigeben, wenn danach noch jemand haelt
    void CoopDropRelease(oCNpc* npc, zCVob* v, const char* where) {
        if (!v)
            return;
        if (v->refCtr > 1) {
            v->Release();
            return;
        }
        static CoopDiagBudget budget = { 0, 0 };
        if (CoopDiagAllow(budget, 20))
            OthelloCompatLog("[%lld s, %s] Absturzschutz: %s an %s - Gegenstand %s (%p) haette danach keinen Verweis mehr (Zaehler %d, Welt %s) "
                "- GT behaelt ihn, statt ihn zu loeschen", DiagSeconds(), DiagRole(), where, npc ? CoopSlotOwnerName(npc).c_str() : "-",
                CoopVobLabel(v).c_str(), (void*)v, v->refCtr, v->GetHomeWorld() ? "ja" : "nein");
    }

    // jedes Bild nach den Paketen (Plugin.cpp): eigener Held und Coop-Figuren, alle NPCs der Welt alle 250 ms
    void CoopSlotGuardPulse() {
        if (!ogame || !player || !ogame->GetGameWorld())
            return;
        CoopScrubDeadSlots(player, NULL, false);
        for (auto& kv : PlayerNpcs)
            CoopScrubDeadSlots(kv.first, NULL, false);
        static long long next = 0;
        const long long now = GetCurrentMs();
        if (now < next)
            return;
        next = now + 250;
        for (zCListSort<oCNpc>* l = CoopWorldNpcListFirst(ogame->GetGameWorld()); l; l = l->next)
            if (l->data && l->data != player && !PlayerNpcs.count(l->data))
                CoopScrubDeadSlots(l->data, NULL, false);
    }

    // --- u164 (P171): NPC mit ungueltiger Lage (NaN) -------------------------------------------------------------------------
    // Dauerlauf u163 (Gothic II): Seit dem 02.11. schlugen in fast jeder Pruefrunde vier Vergleiche "NPCs um mich" fehl, immer an
    // BDT_10314_ADDON_RANGERBANDIT_M - Lage und Blickrichtung NaN auf beiden PCs, so auch im Spielstand. Entstanden nach einer
    // Kampf-Uebergabe (Testlauf u152): Der Client gab ihn "zu weit" zurueck, die Rueckgabe trug die Blickrichtung aus GetHeading (asin
    // eines Matrixwerts knapp ueber 1 = NaN), der Host drehte ihn damit (RotateWorldY), beim naechsten Schritt wurde auch die Lage NaN.
    // Ein NPC mit NaN-Lage ist fuer jede Abstandsfrage unsichtbar (jeder Vergleich mit NaN ist falsch) und bleibt es ueber jedes Speichern.
    // Jetzt, allgemein (GT): GetHeading begrenzt (unten), der Empfang verwirft Pakete mit NaN/unendlich (CoopPacketFinite, auch der
    // Weltprozess), und einmal je Sekunde sucht dieser Schutz NPCs, deren Matrix nicht endlich ist: Drehung zurueck und an ihren Ort
    // (Wegpunkt aus wpname, sonst der erste des Tagesablaufs, sonst ein Vob dieses Namens), Kollision dabei aus, Geschwindigkeit und
    // Fallhoehe des Bewegungsreglers neu. oCNpc::ResetPos taugt dafuer nicht: Es dreht nur mit gedrueckter Alt-Taste zurueck und stoesst
    // den NPC mit seinem (NaN-)Blickvektor an. Je NPC hoechstens drei Versuche bis zum naechsten Laden. Pruefsignal: "Lage-Schutz: ..."
    static int CoopNpcLageFixed = 0;                 // Reparaturen seit dem Start (Testschritt)
    static std::map<oCNpc*, int> CoopNpcLageTries;   // Versuche je NPC bis zum naechsten Laden
    static long long CoopNpcLageNextMs = -1;         // -1: nach dem Laden erst 2 s warten
    static oCNpc* CoopNpcLageLastNpc = NULL;         // u166: letzte Reparatur, NPC und Ziel (Testschritt - wpname kann bis dahin wechseln)
    static zVEC3 CoopNpcLageLastPos;
    static volatile LONG CoopNetNanDropped = 0;      // verworfene Pakete mit NaN/unendlich (Empfang, Weltprozess)

    static bool CoopFiniteVec(const zVEC3& v) {
        return _finite(v.n[0]) && _finite(v.n[1]) && _finite(v.n[2]);
    }

    bool CoopTrafoFinite(zCVob* v) {
        const float* m = (const float*)&v->trafoObjToWorld;
        for (int i = 0; i < 12; i++)   // Drehung und Lage (die vierte Zeile ist fest)
            if (!_finite(m[i]))
                return false;
        return true;
    }

    // true, wenn alle Zahlen endlich sind; sonst steht in field der Schluessel des ersten Werts mit NaN/unendlich
    bool CoopJsonFinite(const json& j, std::string& field) {
        if (j.is_number_float())
            return _finite(j.get<double>()) != 0;
        if (j.is_object()) {
            for (auto it = j.begin(); it != j.end(); ++it)
                if (!CoopJsonFinite(it.value(), field)) {
                    if (field.empty())
                        field = it.key();
                    return false;
                }
        }
        else if (j.is_array()) {
            for (auto& x : j)
                if (!CoopJsonFinite(x, field))
                    return false;
        }
        return true;
    }

    // Empfang (PacketProcessor.cpp): false = verwerfen, gezaehlt und gemeldet
    bool CoopPacketFinite(const json& j, const char* from) {
        std::string field;
        if (CoopJsonFinite(j, field))
            return true;
        const LONG n = InterlockedIncrement(&CoopNetNanDropped);
        static CoopDiagBudget budget = { 0, 0 };
        if (CoopDiagAllow(budget, 20)) {
            const int type = j.contains("type") && j["type"].is_number_integer() ? j["type"].get<int>() : -1;
            const std::string id = j.contains("id") && j["id"].is_string() ? j["id"].get<std::string>() : std::string("-");
            OthelloCompatLog("[%lld s, %s] Netz: Paket von %s verworfen - Feld %s ist NaN/unendlich (Typ %d, id %s), zusammen %ld",
                DiagSeconds(), DiagRole(), from, field.c_str(), type, id.c_str(), (long)n);
        }
        return false;
    }

    // Ort fuer einen NPC ohne gueltige Lage: Wegpunkt aus wpname, sonst der erste seines Tagesablaufs, sonst ein Vob dieses Namens (Freipunkt)
    bool CoopNpcHomePos(oCNpc* npc, zVEC3& pos, std::string& where) {
        oCWorld* w = ogame ? ogame->GetGameWorld() : NULL;
        if (!npc || !w)
            return false;
        std::vector<std::string> names;
        if (!npc->wpname.IsEmpty())
            names.push_back(npc->wpname.ToChar());
        auto r = NpcToFirstRoutineWp.find(npc);
        if (r != NpcToFirstRoutineWp.end() && !r->second.IsEmpty())
            names.push_back(r->second.ToChar());
        for (auto& n : names) {
            if (zCWaypoint* wp = w->wayNet ? w->wayNet->GetWaypoint(zSTRING(n.c_str())) : NULL) {
                pos = wp->GetPositionWorld();
                where = "Wegpunkt " + n;
            }
            else if (zCVob* v = w->SearchVobByName(zSTRING(n.c_str()))) {
                pos = v->GetPositionWorld();
                where = "Ort " + n;
            }
            else
                continue;
            if (CoopFiniteVec(pos))
                return true;
        }
        return false;
    }

    // NPC hart an eine Stelle setzen, Kollision aus (wie RemoteNpc::UpdateNpcPosition bei grossem Abstand); resetRot: Drehung zurueck
    void CoopNpcPlace(oCNpc* npc, const zVEC3& pos, bool resetRot) {
        const bool inMove = npc->isInMovementMode != 0;
        if (inMove) {
#if ENGINE >= Engine_G2
            npc->EndMovement(false);
#else
            npc->EndMovement();
#endif
        }
        npc->SetCollDet(FALSE);
        if (resetRot)
            npc->ResetRotationsWorld();
        npc->SetPositionWorld(pos);
        npc->SetCollDet(TRUE);
        if (inMove)
            npc->BeginMovement();
    }

    static void CoopNpcLageRepair(oCNpc* npc) {
        int& tries = CoopNpcLageTries[npc];
        if (tries >= 3)
            return;
        tries++;
        auto nm = NpcToUniqueNameList.find(npc);
        const std::string name = nm != NpcToUniqueNameList.end() ? std::string(nm->second.ToChar()) : std::string(npc->GetObjectName().ToChar());
        const zVEC3 bad = npc->GetPositionWorld();
        zVEC3 pos;
        std::string where;
        static CoopDiagBudget budget = { 0, 0 };
        if (!CoopNpcHomePos(npc, pos, where)) {
            tries = 3;
            if (CoopDiagAllow(budget, 30))
                OthelloCompatLog("[%lld s, %s] Lage-Schutz: %s hat eine ungueltige Lage (%.0f/%.0f/%.0f, Zustand %s) - kein Ort gefunden (Wegpunkt %s), "
                    "bleibt so", DiagSeconds(), DiagRole(), name.c_str(), bad.n[0], bad.n[1], bad.n[2], npc->state.curState.name.ToChar(),
                    npc->wpname.IsEmpty() ? "-" : npc->wpname.ToChar());
            return;
        }
        CoopNpcPlace(npc, pos, true);
        if (oCAniCtrl_Human* ac = npc->GetAnictrl()) {   // was der Bewegungsregler aus der alten Lage weitertraegt
            if (!CoopFiniteVec(ac->velocity) || !_finite(ac->velocityLen2)) {
                ac->velocity = zVEC3(0.f, 0.f, 0.f);
                ac->velocityLen2 = 0.f;
            }
            if (!_finite(ac->fallDownStartY) || !_finite(ac->fallDownDistanceY)) {
                ac->fallDownStartY = pos.n[1];
                ac->fallDownDistanceY = 0.f;
            }
        }
        if (npc->physicsEnabled)
            if (zCRigidBody* rb = npc->GetRigidBody())
                rb->SetVelocity(zVEC3(0.f, 0.f, 0.f));
        const bool ok = CoopTrafoFinite(npc);
        if (ok) {
            CoopNpcLageFixed++;
            CoopNpcLageLastNpc = npc;
            CoopNpcLageLastPos = pos;
        }
        if (CoopDiagAllow(budget, 30))
            OthelloCompatLog("[%lld s, %s] Lage-Schutz: %s hatte eine ungueltige Lage (%.0f/%.0f/%.0f, Zustand %s) - %s %s (%.0f/%.0f/%.0f), Versuch %d",
                DiagSeconds(), DiagRole(), name.c_str(), bad.n[0], bad.n[1], bad.n[2], npc->state.curState.name.ToChar(),
                ok ? "gesetzt an" : "NICHT zu reparieren an", where.c_str(), pos.n[0], pos.n[1], pos.n[2], tries);
    }

    // jedes Bild nach dem Slot-Waechter (Plugin.cpp): alle NPCs der Welt einmal je Sekunde, nach dem Laden erst nach 2 s
    void CoopNpcLagePulse() {
        if (!ogame || !player || !ogame->GetGameWorld())
            return;
        const long long now = GetCurrentMs();
        if (CoopNpcLageNextMs < 0)
            CoopNpcLageNextMs = now + 2000;
        if (now < CoopNpcLageNextMs)
            return;
        CoopNpcLageNextMs = now + 1000;
        for (zCListSort<oCNpc>* l = CoopWorldNpcListFirst(ogame->GetGameWorld()); l; l = l->next) {
            oCNpc* npc = l->data;
            if (npc && npc != player && !PlayerNpcs.count(npc) && !CoopTrafoFinite(npc))
                CoopNpcLageRepair(npc);
        }
    }

    // beim Laden (Plugin.cpp LoadBegin): Zeiger der alten Welt verwerfen
    void CoopNpcLageWorldGone() {
        CoopNpcLageTries.clear();
        CoopNpcLageNextMs = -1;
    }

    // Testschritt (CoopSelfTest.cpp): Versuche fuer diesen NPC vergessen
    void CoopNpcLageForget(oCNpc* npc) {
        CoopNpcLageTries.erase(npc);
    }

    // v39 (Nutzer: "nur noch mit Shift+F1, Shift+F2, Shift+F3, Shift+F4, damit man nicht aus Versehen disconnected"):
    // Shift gedrueckt? Wie CoopSimShiftDown (CoopDialogSync.cpp): zinput, sonst Windows-Taste, wenn das Spielfenster vorn ist.
    bool CoopShiftDown() {
        if (zinput->KeyPressed(KEY_LSHIFT) || zinput->KeyPressed(KEY_RSHIFT))
            return true;
        HWND fg = GetForegroundWindow();
        DWORD pid = 0;
        if (fg)
            GetWindowThreadProcessId(fg, &pid);
        return pid == GetCurrentProcessId() && (GetAsyncKeyState(VK_SHIFT) & 0x8000);
    }

    // v51 (O1): Test v50 - beim Client wurde die Host-Figur in 5 min viermal geloescht und neu erzeugt (neue Adresse, gleiche
    // Lage), das Zuschau-Fenster ging jedes Mal aus. Ursache: Othellos Test-Modus legt Noclip auf Shift+F3 (OTHELLO_ABI:
    // "noclip (SHIFT + F3)", "deactivate (F3)", angezeigt nur bei ogame->game_testmode = oCGame+0xB0, wie im SDK) - dieselbe
    // Taste wie GTs "Figuren neu" (reinitPlayersKey, Standard F3). Jedes Ein-/Ausschalten von Noclip erzeugte alle Coop-Figuren
    // neu (Schnappschuss des Clients: S_NOCLIP genau zu diesen Zeiten). Allgemein: im Test-Modus belegt schon die Engine F2-F12
    // (oCGame::TestKeys 0x00660000, Schalter ueber 0x006625D0/0x00662538; Othello ersetzt den F3-Zweig 0x00660DF1 durch
    // OTHELLO_ABI+0x89180). Jetzt: liegt "Figuren neu" auf F2-F12 und ist der Test-Modus an, tut GT nichts. Beides wird
    // protokolliert, damit ein Neuaufbau der Figuren im Log zu sehen ist.
    bool CoopReinitKeyAllowed(int key) {
        const bool fkey = (key >= KEY_F2 && key <= KEY_F10) || key == KEY_F11 || key == KEY_F12;
        const bool testmode = ogame && ogame->game_testmode != 0;
        const bool allowed = !(fkey && testmode);
        static CoopDiagBudget logged = { 0, 0 };
        if (CoopDiagAllow(logged, 10)) {
            if (allowed)
                OthelloCompatLog("[%lld s, %s] Figuren neu (Taste reinitPlayersKey mit Shift): %u Figur(en) werden neu erzeugt",
                    DiagSeconds(), DiagRole(), (unsigned)PlayerNpcs.size());
            else
                OthelloCompatLog("[%lld s, %s] Shift+F-Taste im Test-Modus gehoert dem Spiel (Othello: Shift+F3 = Noclip) -"
                    " Figuren neu nicht ausgeloest", DiagSeconds(), DiagRole());
        }
        return allowed;
    }

    int CoopFollowPoints(zVEC3* out, int max);   // v41: CoopFarSync.cpp

    std::vector<oCNpc*> GetVisibleNpcs() {
        std::vector<oCNpc*> npcs;
        auto* list = CoopWorldNpcListFirst(ogame->GetGameWorld());
        // v41: auch NPCs im Umkreis der Figuren (Verfolgungspunkt beim Mitspieler, nur Host)
        zVEC3 follow[8];
        const int followCount = CoopFollowPoints(follow, 8);

        while (list) {
            auto npc = list->data;
            auto npcPosition = npc->GetPositionWorld();
            auto playerPosition = player->GetPositionWorld();

            if (npc->vobLeafList.GetNum() == 0) {
                list = list->next;
                continue;
            }

            bool nearFigure = false;
            for (int f = 0; f < followCount && !nearFigure; f++)
                nearFigure = follow[f].Distance(npcPosition) < BROADCAST_DISTANCE;

            if (playerPosition.Distance(npcPosition) < BROADCAST_DISTANCE || nearFigure) {
                if (!npc->IsAPlayer() && !IsCoopPlayer(npc->GetObjectName())) {
                    npcs.push_back(npc);
                }
            }
            list = list->next;
        }

        return npcs;
    }

    float GetHeading(oCNpc* npc)
    {
        float x = *(float*)((DWORD)npc + 0x44);
        // u164 (P171): asin nur von -1 bis 1. Ist die Matrix minimal ueber 1 skaliert und schaut der NPC fast genau entlang x, liegt
        // der Wert knapp ueber 1 - asin lieferte NaN, und die Rueckgabe der Kampf-Uebergabe drehte den NPC beim Host damit (Testlauf
        // u152, CoopNpcLagePulse oben). Eine NaN-Matrix ergibt hier einen endlichen Wert statt NaN
        if (!(x >= -1.f && x <= 1.f))
            x = x > 1.f ? 1.f : x < -1.f ? -1.f : 0.f;   // NaN -> 0
        float rotx = asin(x) * 180.0f / 3.14f;
        float y = *(float*)((DWORD)npc + 0x64);
        if (y > 0)
        {
            if (x < 0)
                rotx = 360 + rotx;
        }
        else
        {
            if (rotx > 0)
                rotx = 180 - rotx;
            else
            {
                rotx = 180 + rotx;
                rotx = 360 - rotx;
            }
        }
        return rotx;
    };

    int ReadConfigKey(std::string key, string _default) {
        auto stringKey = CoopConfig.contains(key) ?
            string(CoopConfig[key].get<std::string>().c_str()).ToChar() :
            _default;

        return GetEmulationKeyCode(stringKey);
    }

    bool IsPlayerTalkingWithAnybody() {
        return ogame->GetCameraAI()->GetMode().Compare("CAMMODDIALOG");
    }

    bool IsPlayerTalkingWithNpc(zCVob* npc) {
        if (!ogame->GetCameraAI()->GetMode().Compare("CAMMODDIALOG")) {
            return false;
        }

        if (player->talkOther == npc) {
            return true;
        }

        if (ogame->GetCameraAI()->targetVobList.GetNum() > 0) {
            for (int i = 0; i < ogame->GetCameraAI()->targetVobList.GetNum(); i++) {
                auto vob = ogame->GetCameraAI()->targetVobList[i];
                if (npc == vob) {
                    return true;
                }
            }
        }

        return false;
    }

    // Fuehrt Gothics Dialogverwaltung diesen NPC gerade als Gespraechspartner des Spielers?
    // IsPlayerTalkingWithNpc erkennt den Dialog erst am Kameramodus CAMMODDIALOG; der springt
    // aber erst an, wenn sich der NPC schon zum Spieler gedreht hat. Vorher blockierte GT auf dem
    // Client die Dreh-Animationen - die Drehung wurde nie fertig, der Dialog hing.
    //
    // Bewusst NICHT ueber talkOther: Gothic setzt es auch bei Drohung/Kampfansage und loescht es
    // danach nicht (gemessen: talkOther=CS_ESKORTE2 bei IsDone=1). In v6 fuehrte das dazu, dass
    // der Client alle Host-Daten dieses NPCs verwarf - Position, Animation und Treffer.
    bool CoopNpcInDialogWithPlayer(zCVob* vob) {
        if (!vob || !player || vob->GetCharacterClass() != 2)
            return false;
        oCInformationManager& im = oCInformationManager::GetInformationManager();
        return im.Npc == (oCNpc*)vob && !im.IsDone;
    }

    float GetDistance3D(float aX, float aY, float aZ, float bX, float bY, float bZ)
    {
        float distX = aX - bX;
        float distY = aY - bY;
        float distZ = aZ - bZ;
        return sqrt(distX * distX + distY * distY + distZ * distZ);
    };

    bool IgnoredSyncNpc(zCVob* npc) {
        auto name = npc->GetObjectName();

        for (unsigned int i = 0; i < IgnoredSyncNpcsCount; i++)
        {
            if (strcmp(name.ToChar(), IgnoredSyncNpcs[i]) == 0)
                return true;
        }

        return false;
    }

    // u150 (P153): Reihenfolge fuer die Nummern gleichnamiger NPCs. Bis u149 zaehlte die Weltliste, und deren Reihenfolge weicht nach dem Laden
    // zwischen den PCs ab, weil jeder seine eigene Weltdatei laedt (Testlauf: 71 % der Gruppen in Gothic 1, 18 % in Othello, 22 % in Gothic 2
    // vertauscht). Derselbe Name meinte dann zwei verschiedene Figuren: Der Tod eines Monsters traf beim Partner sein Gegenstueck, und das blieb
    // in dessen Spielstand. Jetzt zaehlen Lebende zuerst, danach die Lage. Allgemein.
    void CoopNpcNameOrder(std::vector<oCNpc*>& order) {
        std::stable_sort(order.begin(), order.end(), [](oCNpc* a, oCNpc* b) {
            const bool deadA = a->IsDead(), deadB = b->IsDead();
            if (deadA != deadB)
                return deadB;
            const zVEC3 pa = a->GetPositionWorld(), pb = b->GetPositionWorld();
            if (pa.n[0] != pb.n[0])
                return pa.n[0] < pb.n[0];
            if (pa.n[2] != pb.n[2])
                return pa.n[2] < pb.n[2];
            return pa.n[1] < pb.n[1];
        });
    }

    void BuildGlobalNpcList() {
        PluginState = "BuildGlobalNpcList";

        auto* list = CoopWorldNpcListFirst(ogame->GetGameWorld());
        auto firstRun = NpcToFirstRoutineWp.size() == 0;
        int diagRtnEntries = 0;
        int diagWorldNpcs = 0;

        if (firstRun) {
            auto* rtnList = rtnMan->rtnList.next;
            while (rtnList) {
                auto rtn = rtnList->data;
                diagRtnEntries++;
                if (NpcToFirstRoutineWp.count(rtn->npc) == 0) {
                    NpcToFirstRoutineWp[rtn->npc] = rtn->wpname.ToChar();
                }
                rtnList = rtnList->next;
            }

            while (list) {
                auto npc = list->data;
                NamesCounter[string(npc->GetObjectName())] += 1;
                list = list->next;
            }
        }

        std::vector<oCNpc*> order;
        for (list = CoopWorldNpcListFirst(ogame->GetGameWorld()); list; list = list->next)
            order.push_back(list->data);
        if (firstRun)
            CoopNpcNameOrder(order);

        for (oCNpc* npc : order) {
            diagWorldNpcs++;

            if (npc->IsAPlayer() || IsCoopPlayer(npc->GetObjectName()) || IgnoredSyncNpc(npc) || NpcToUniqueNameList.count(npc))
                continue;

            auto objectName = string(npc->GetObjectName());
            CStringA name = "";
            if (NamesCounter[objectName] == 1) {
                name = objectName;
            }
            else {
                auto secondUniquePart = npc->wpname ? string(npc->wpname) : string("UNKNOW");
                if (NpcToFirstRoutineWp.count(npc) > 0 && !NpcToFirstRoutineWp[npc].IsEmpty()) {
                    secondUniquePart = NpcToFirstRoutineWp[npc];
                }
                if (!firstRun) {
                    secondUniquePart = npc->wpname ? string::Combine("DYNAMIC-%s", string(npc->wpname)) : string("DYNAMIC");
                }
                name = string::Combine("%s-%s", string(npc->GetObjectName()), secondUniquePart);
                NamesCounter[name] += 1;
            }

            auto uniqueName = string::Combine("%s-%i", name, NamesCounter[name]);
            UniqueNameToNpcList[uniqueName] = npc;
            NpcToUniqueNameList[npc] = uniqueName;
            CoopDiagNameAssigned(uniqueName, npc, firstRun);
        }

        CoopDiagListBuilt(firstRun, diagWorldNpcs, diagRtnEntries);
    }

    // u51 (Durchsicht u50, Befund 1): Uhrzeit des Hosts samt Spieltag uebernehmen. Bis u50 kamen nur Stunde und Minute an -
    // oCWorldTimer::SetTime (G2A 0x00780E40, G1 0x006D7C00) schreibt nur worldTime (+0), der Tag (+4) blieb. Stand der Client kurz
    // nach Mitternacht vor dem Host, setzte ihn der Host auf 23:59 des schon neuen Tags zurueck, und er zaehlte Mitternacht ein
    // zweites Mal (Pruefrunde Lauf 5, Schritt 114: Host Tag 0 01:40, Client Tag 2 01:39); auch allein Schlafen blieb im Tag stehen.
    // Jetzt auch der Tag (oCWorldTimer::SetDay G2A 0x00780DE0, G1 0x006D7BA0 - schreibt nur +4). day < 0: Partner vor u51, wie
    // bisher nur die Uhrzeit. Allgemein (G1/G2).
    // u83: Abstand der Uhr des Hosts zur eigenen in Spielminuten (Tag mitgerechnet, also auch ueber Mitternacht; day < 0: nur die Uhrzeit)
    int CoopHostTimeDiffMin(int day, int h, int m) {
        int d0 = 0, h0 = 0, m0 = 0;
        if (ogame)
            ogame->GetTime(d0, h0, m0);
        return ((day >= 0 ? day : d0) * 1440 + h * 60 + m) - (d0 * 1440 + h0 * 60 + m0);
    }

    // u83 (Nutzer: "sollten wir mal die Uhrzeit des Clients tatsaechlich mit dem Host synchronisieren"): t = genaue Spielzeit des
    // Hosts (oCWorldTimer::worldTime). Bis u82 kamen alle 10 s nur Stunde und Minute - der Client sprang auf den Minutenanfang
    // zurueck und lief dazwischen frei (Dauerlauf u78 mit dem Host auf x2, u80: bis 5 Spielminuten voraus; an einer Grenze des
    // Tagesablaufs wechselten seine NPCs so im 10-s-Takt hin und her, Harad pendelte 40 m - harte Spruenge). Jetzt jede Sekunde genau;
    // ein Sprung ueber 30 Spielminuten (der Host hat geschlafen) startet die Tagesablaeufe neu wie im Weltprozess. t < 0: Partner vor
    // u83, wie bisher nur Stunde und Minute.
    void CoopApplyHostTime(int day, int h, int m, const char* why, float t = -1.f) {
        if (!ogame || h < 0 || h > 23 || m < 0 || m > 59)
            return;
        oCWorldTimer* timer = ogame->GetWorldTimer();
        if (!timer)
            return;
        int d0 = 0, h0 = 0, m0 = 0;
        ogame->GetTime(d0, h0, m0);
        const int diff = CoopHostTimeDiffMin(day, h, m);
        if (day >= 0 && day != d0) {
            timer->SetDay(day);
            static CoopDiagBudget budget = { 0, 0 };
            if (CoopDiagAllow(budget, 10))
                OthelloCompatLog("[%lld s, %s] Uhrzeit: Spieltag %d -> %d wie beim Host (%02d:%02d -> %02d:%02d, %s)", DiagSeconds(),
                    DiagRole(), d0, day, h0, m0, h, m, why);
        }
        if (t >= 0.f)
            timer->worldTime = t;
        else
            timer->SetTime(h, m);
        if (abs(diff) >= 2) {   // u83: groessere Korrektur melden (zeigt, wie weit die Uhren auseinanderlaufen)
            static CoopDiagBudget big = { 0, 0 };
            if (CoopDiagAllow(big, 20))
                OthelloCompatLog("[%lld s, %s] Uhrzeit: %+d Spielminuten zum Host - uebernommen (hier Tag %d %02d:%02d, Host Tag %d %02d:%02d, %s)%s",
                    DiagSeconds(), DiagRole(), diff, d0, h0, m0, day, h, m, why, abs(diff) > 30 ? " - Tagesablaeufe neu" : "");
        }
        if (abs(diff) > 30 && rtnMan)
            rtnMan->RestartRoutines();
    }

    // u83: kleine Korrektur (hoechstens 2 Spielminuten) - die gilt auch im Gespraech, eine grosse wartet bis zu dessen Ende
    bool CoopHostTimeSmall(int day, int h, int m) {
        return abs(CoopHostTimeDiffMin(day, h, m)) <= 2;
    }

    // u186 (Test-Fenster 05.11.: die PC-Uhr sprang um 14:31 eine Stunde zurueck - Zeitzone geaendert, danach Abgleich mit time.windows.com;
    // in den laufenden Spielen verschoben sich damit alle Fristen um diese Stunde: Test-Ende, Stillstand, Gespraechssperren, Kapitel-Warten):
    // GetCurrentMs misst nur noch Abstaende, mit der monotonen Uhr (steady_clock, auf Windows QueryPerformanceCounter - auch zwischen den
    // Prozessen eines PCs gleich, z. B. Weltprozess). Geprueft: Kein Zeitpunkt geht ans Netz oder in eine Datei (nur Abstaende wie
    // "vor_s"), die Truhen-Epoche wird nur auf Gleichheit verglichen; Uhrzeiten (Spielstaende, Logs, Dateien) kommen weiter aus
    // GetLocalTime, time und FILETIME. Allgemein.
    // u187 (Hinweis Test-Fenster): steady_clock zaehlt ab dem Hochfahren des PCs - eine Frist "jetzt > x + N" mit x = 0 ("noch nie") griffe
    // dann in den ersten N ms nach dem Hochfahren nicht. Darum ein fester Versatz (Uhrzeit beim ersten Aufruf minus steady): Die Werte
    // liegen wie bisher in der Groessenordnung der Unix-Zeit, der Gang bleibt monoton.
    long long GetCurrentMs() {
        using namespace std::chrono;
        static const long long offset = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()
            - duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
        return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count() + offset;
    }

    static int GetPartyMemberID() {
        zCPar_Symbol* sym = parser->GetSymbol("AIV_PARTYMEMBER");
        if (!sym)
#if ENGINE >= Engine_G2
            return 15;
#else
            return 36;
#endif
        int id;
        sym->GetValue(id, 0);
        return id;
    }

    static int GetFriendDefaultInstanceId() {
        auto instId = parser->GetIndex(FriendInstanceId);

        return instId;
    }
}