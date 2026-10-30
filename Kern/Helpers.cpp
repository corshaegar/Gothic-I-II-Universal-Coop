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

    static void CoopVobVtablesBuild() {
        CoopVobVtablesMs = GetCurrentMs();
        oCWorld* w = ogame ? ogame->GetGameWorld() : NULL;
        if (!w)
            return;
        zCArray<zCVob*> all;
        w->SearchVobListByClass(zCVob::classDef, all, NULL);
        for (int i = 0; i < all.GetNumInList(); i++) {
            zCVob* v = all.GetSafe(i);
            if (!v)
                continue;
            CoopPartLearn(CoopVisualVtables, v->visual);   // u89
            CoopPartLearn(CoopAiVtables, v->callback_ai);
            if (CoopVtableAdd(CoopVobVtables, *(void**)v) && !CoopVobVtablesItem && zDYNAMIC_CAST<oCItem>(v))
                CoopVobVtablesItem = true;
        }
    }

    // NULL = v lebt, sonst der Grund
    static const char* CoopVobDeadWhy(const void* v, void** vt, int* ref) {
        *vt = NULL;
        *ref = 0;
        if (!CoopVobPeek(v, vt, ref))
            return "nicht lesbar";
        if (*ref <= 0)
            return "Referenzzaehler <= 0";
        if (std::binary_search(CoopVobVtables.begin(), CoopVobVtables.end(), *vt))
            return NULL;
        if (GetCurrentMs() - CoopVobVtablesMs > 1000)   // neue Klasse oder andere Welt: Liste hoechstens einmal je Sekunde ergaenzen
            CoopVobVtablesBuild();
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
        if (set.empty() || std::binary_search(set.begin(), set.end(), *vt))
            return NULL;
        if (GetCurrentMs() - CoopVobVtablesMs > 1000)
            CoopVobVtablesBuild();
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
            const char* why = CoopVobDeadWhy(s->vob, &vt, &ref);
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
        const char* why = CoopVobDeadWhy(vob, &vt, &ref);
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

        list = CoopWorldNpcListFirst(ogame->GetGameWorld());
        while (list) {
            auto npc = list->data;
            diagWorldNpcs++;

            if (npc->IsAPlayer() || IsCoopPlayer(npc->GetObjectName()) || IgnoredSyncNpc(npc) || NpcToUniqueNameList.count(npc)) {
                list = list->next;
                continue;
            }

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

            list = list->next;
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

    long long GetCurrentMs() {
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
            );

        return ms.count();
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