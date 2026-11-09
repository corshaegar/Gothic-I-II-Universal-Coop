// Supported with union (c) 2020 Union team
// Union SOURCE file

#include <intrin.h>   // _ReturnAddress

namespace GOTHIC_ENGINE {

    // --- Dialog-Waechter (18.10.2026) -----------------------------------------------------------
    //
    // Fehlerbild (Nutzer): Beim Ansprechen eines NPCs oeffnet sich der Dialog, es erscheinen aber keine
    // Zeilen, der Held haengt fest (Cayden, ein Gardist; auch im Einzelspieler).
    //
    // Befund (dialog_sonde.py im haengenden Dialog, nur gelesen): oCInformationManager (0x009D9808)
    // Npc=Cayden, IsDone=0, IsWaitingForScript=1, Info=0; KI-Warteschlangen von Cayden und Held leer,
    // Caydens EventManager "cleared"=1, Zustand ZS_TALK. Den Dialog treibt allein die Nachricht
    // EV_PROCESSINFOS in der Warteschlange des NPCs: oCNpc::EV_ProcessInfos (0x006B3DD0) ruft, solange
    // sie "inUse" ist, je Frame oCInformationManager::Update (0x0072BE90); oCGame::Render ruft Update nur
    // bei IsWaitingForEnd (0x0063DEC0). Leert jemand waehrend des Dialogs die Warteschlange des NPCs
    // (oCNpc::ClearEM 0x006A2610, unter Othello mit umgeleitetem GetEM), ist die Nachricht weg, Update
    // laeuft nie wieder, IsDone bleibt 0 und der Held kommt nicht heraus.
    //
    // Abhilfe: Fehlt die Nachricht bei offenem Dialog 1,5 s lang, eine neue EV_PROCESSINFOS (Ziel = Held,
    // inUse = 1, also ohne neues SetNpc) beim NPC einreihen; der Dialog laeuft dann normal weiter. Nach
    // drei erfolglosen Versuchen oCInformationManager::Exit() (Dialog beenden, Held wieder frei).
    // Dazu Protokoll, wer ClearEM auf den Gespraechspartner eines offenen Dialogs ruft (Ruecksprung +
    // Modul), damit die eigentliche Ursache gefunden werden kann. Ausgabe system\GothicCoopOthello.log.
    //
    // u155 (P160, Testinstanz b156/b159 2.11., G2A-Client): Der Host lud, waehrend der Client mit Andre sprach. Gothic setzt den
    // oCInformationManager beim Laden nicht zurueck (aus dem Esc-Menue laedt man nie im Dialog): Npc und Player zeigten danach auf
    // den alten Andre und den alten Helden, IsDone blieb 0 - SetNpc tut dann nichts, mit niemandem mehr ein Gespraech. Der
    // Waechter reihte EV_PROCESSINFOS beim alten Andre ein und rief Exit; ab da ruft oCGame::Render in jedem Bild Update (G2A
    // 0x00660BB0), und Update las die Warteschlange des freigegebenen Helden: Absturz C0000005 an einer Adresse im Heap.
    // Jetzt: Beginnt ein Ladevorgang (Spielstand, neues Spiel, Weltwechsel) mitten in einem Dialog, beendet GT ihn, solange beide
    // Figuren noch leben, mit Gothics eigenem Abschluss - Exit (Auswahl- und Handelsfenster zu; fasst weder NPC noch Held an) und
    // OnTermination (Statusanzeige, Kamera, Bewegungssperre des Helden, Npc/Player leer, IsDone 1; G2A 0x006631A0, G1 0x0072E430;
    // der NPC wird nicht angefasst). Und der Waechter prueft vor jedem Zugriff, ob der Dialog noch zu dieser Welt gehoert (Player
    // ist der Held, der NPC steht in der NPC-Liste der Welt); sonst beendet er ihn ebenso, ohne die alten Figuren.

#if COOP_BUILD(DialogWatch)   // u3: bis u2 nur G1 - Freigabe je Engine in CoopCore.cpp

    static unsigned long DialogWatchLines = 0;
    int CoopDialogLoadEnds = 0;          // u155 (P160): beim Ladebeginn beendete Dialoge (Testlauf)
    int CoopDialogStaleEnds = 0;         //              vom Waechter beendete Dialoge, deren Figuren nicht mehr zur Welt gehoeren
    std::string CoopDialogLastEnded;     //              NPC des zuletzt beim Ladebeginn beendeten Dialogs (CDiagName)

    static bool DialogWatchDarfLoggen() {
        return DialogWatchLines++ < 200;
    }

    // u155 (P160): gehoert der offene Dialog noch zu dieser Welt? heldWeg: Player ist nicht der Held (neu geladen)
    static bool DialogWatchVeraltet(const oCInformationManager& im, bool& heldWeg) {
        heldWeg = im.Player != player;
        if (heldWeg)
            return true;
        if (im.Npc == player || !ogame || !ogame->GetGameWorld())
            return false;   // Herstellung (AI_ProcessInfos(hero)) bzw. ohne Welt nicht beurteilbar
        for (auto* l = CoopWorldNpcListFirst(ogame->GetGameWorld()); l; l = l->next)
            if (l->data == im.Npc)
                return false;
        return true;
    }

    // u155 (P160): Dialog mit Gothics eigenem Abschluss beenden; ohne den alten Helden, wenn er nicht mehr der Held ist
    static void DialogWatchEnd(oCInformationManager& im, bool heldWeg) {
        im.Exit();
        if (heldWeg)
            im.Player = NULL;   // OnTermination ruft SetMovLock und GetInteractMob nur mit Player
        im.OnTermination();
        im.IsWaitingForScript = im.IsWaitingForOpen = im.IsWaitingForClose = im.IsWaitingForSelection = 0;
    }

    // UnionHooks.cpp _LoadBegin: vor dem Abbau der Welt
    void CoopDialogWatchLoadBegin() {
        oCInformationManager& im = oCInformationManager::GetInformationManager();
        if (im.IsDone || (!im.Npc && !im.Player))
            return;
        bool heldWeg = false;
        const bool veraltet = DialogWatchVeraltet(im, heldWeg);   // z. B. ein zweites Laden: die alten Figuren nicht anfassen
        char name[64] = "-";
        if (im.Npc && !veraltet)
            CDiagName(im.Npc, name, sizeof(name));
        char flags[96];
        _snprintf(flags, sizeof(flags), "WartetSkript=%d WartetEnde=%d WartetAuswahl=%d%s", im.IsWaitingForScript, im.IsWaitingForEnd,
            im.IsWaitingForSelection, veraltet ? " - Figuren schon veraltet" : "");
        flags[sizeof(flags) - 1] = 0;
        DialogWatchEnd(im, heldWeg || !im.Player);
        CoopDialogLoadEnds++;
        CoopDialogLastEnded = name;
        if (DialogWatchDarfLoggen())
            OthelloCompatLog("[%lld s, %s] Dialog-Waechter: Laden beginnt mitten im Dialog mit %s (%s) - beendet (Exit, OnTermination)",
                DiagSeconds(), DiagRole(), name, flags);
    }

    static bool DialogWatchHatProcessInfos(oCNpc* npc) {
        zCEventManager* em = npc->GetEM(0);
        if (!em)
            return false;
        const int n = em->GetNumMessages();
        for (int i = 0; i < n; i++) {
            oCMsgConversation* m = zDYNAMIC_CAST<oCMsgConversation>(em->GetEventMessage(i));
            if (m && m->subType == oCMsgConversation::EV_PROCESSINFOS && !m->IsDeleted())
                return true;
        }
        return false;
    }

    // u216 (P253): Gespraechssperre des Helden. Archolos-Bloecke 4-6 (u213-u215): "redet nicht" auch bei NPCs mit offener wichtiger
    // Info - Test-Diagnose b326: BAU_2244_TERRY "DIA_TERRY_HELLO W ja", AssessTalk_S 1, er blieb in ZS_OBSERVEPLAYER; Rupert am Halt.
    // Archolos' B_ASSESSTALK bricht ab, wenn ISHEROINVISIBLE() (= OTHER.AIVAR[AIV_INVINCIBLE] beim Spieler), ZS_TALK ebenso. Gesetzt
    // wird das Flag in ZS_TALK fuer NPC und Held, zurueck nur in ZS_TALK_LOOP beim regulaeren Ende (InfoManager_HasFinished) - nicht in
    // ZS_TALK_END. Endet ein Gespraech anders (Laden mitten im Dialog, der Waechter beendet ihn, Abbruch), bleibt der Held fuer alle
    // Gespraeche gesperrt, auch im Spielstand. Absichtlich setzt Archolos es beim Helden nur in Zwischensequenzen und im Teleport-Dialog
    // (beides vor einem Ende), beim Goldhacken (Mob) und fuer Unsichtbarkeit (mit Durchsichtigkeit, MAKEHEROGHOST). Jetzt: 10 s nach
    // jedem Gespraechsende und nach jedem Laden prueft GT - ist das Flag des Helden noch gesetzt, ohne offenen Dialog, ohne Mob und
    // ohne Durchsichtigkeit, kommt es zurueck. Index aus der Skript-Konstante AIV_INVINCIBLE (fehlt sie, tut GT nichts). Allgemein.
    int CoopTalkLockResets = 0;
    static long long CoopTalkLockCheckMs = 0;   // wann pruefen (0 = nichts geplant)
    static bool CoopTalkLockOpen = false;       // war im vorigen Takt ein Dialog offen?
    static bool CoopTalkLockAfterLoadPending = false;
    static const char* CoopTalkLockWhy = "";

    static int CoopAivInvincible() {
        static zCParser* forParser = NULL;
        static int idx = -1;
        if (parser != forParser) {
            forParser = parser;
            idx = -1;
            zCPar_Symbol* s = parser ? parser->GetSymbol(zSTRING("AIV_INVINCIBLE")) : NULL;
            if (s && s->type == zPAR_TYPE_INT && (s->flags & zPAR_FLAG_CONST) && s->ele <= 1 && s->single_intdata >= 0 &&
                s->single_intdata < (int)(sizeof(player->aiscriptvars) / sizeof(player->aiscriptvars[0])))
                idx = s->single_intdata;
        }
        return idx;
    }

    void CoopTalkLockAfterLoad() {   // Plugin.cpp Game_LoadEnd_SaveGame
        CoopTalkLockAfterLoadPending = true;
    }

    // u242 (P289, Othello-Lauf b361 am 09.11.: 9 von 10 Kaempfen des Clients ohne einen Treffer - "Zwischensequenz (Held unverwundbar)" beim
    // Client ab 77 s, als der Host mit Bloodwyn sprach. Othellos ZS_MM_ATTACK geht bei OTHER.AIVAR[AIV_INVINCIBLE] in den Tagesablauf zurueck:
    // die Monster griffen den Helden des Clients nicht an und liefen weiter, er schlug 48-mal ins Leere, "Gegner holte 0-mal aus"): ein
    // Gespraech des Partners laeuft hier mit ZS_TALK des NPCs nach, dessen Start setzt AIV_INVINCIBLE auch beim eigenen Helden. Zurueck kommt
    // es nur am Ende im eigenen InfoManager - hier nie. Das Ende eines Gespraechs des Partners plant dieselbe Pruefung wie P253 (10 s, kein
    // eigener Dialog, kein Mob, nicht durchsichtig). Allgemein
    void CoopTalkLockCheckSoon(const char* why) {   // CoopDialogSync.cpp, SYNC_DIALOG_END
        CoopTalkLockCheckMs = CurrentMs + 10000;
        CoopTalkLockWhy = why;
    }

    static void CoopTalkLockPulse(oCInformationManager& im) {
        const bool open = !im.IsDone && (im.Npc || im.Player);
        if (CoopTalkLockOpen && !open) {
            CoopTalkLockCheckMs = CurrentMs + 10000;
            CoopTalkLockWhy = "10 s nach dem Ende eines Gespraechs";
        }
        CoopTalkLockOpen = open;
        if (CoopTalkLockAfterLoadPending) {
            CoopTalkLockAfterLoadPending = false;
            CoopTalkLockCheckMs = CurrentMs + 10000;
            CoopTalkLockWhy = "10 s nach dem Laden";
        }
        if (!CoopTalkLockCheckMs || CurrentMs < CoopTalkLockCheckMs)
            return;
        CoopTalkLockCheckMs = 0;
        const int idx = CoopAivInvincible();
        if (idx < 0 || !player || open || player->interactMob || player->aiscriptvars[idx] == 0)
            return;
        if (player->visualAlphaEnabled && player->visualAlpha < 0.99f)
            return;   // durchsichtig: gewollte Unsichtbarkeit
        player->aiscriptvars[idx] = 0;
        CoopTalkLockResets++;
        OthelloCompatLog("[%lld s, %s] Gespraechssperre: der Held hatte AIV_INVINCIBLE (AIVAR %d) gesetzt, ohne Gespraech, Mob oder Unsichtbarkeit (%s)"
            " - zurueckgesetzt, NPCs reden wieder mit ihm (P253)", DiagSeconds(), DiagRole(), idx, CoopTalkLockWhy);
    }

    // u217 (P254): Abblendsperre. Archolos-Block 7 (u216): Rupert (offene wichtige Info), Harry, Viktor und Markus redeten nicht, das
    // Flag des Helden (P253) stand auf 0. Im Spielstand des Hosts stand SCREENFADED = 1 - Archolos' B_ASSESSTALK bricht bei
    // (FADESCREENSTATE != 0) || SCREENFADED ab: solange redet kein NPC mit dem Helden, auch keine wichtige Info (B_ASSESSPLAYER ->
    // B_ASSESSTALK). So in allen 209 Staenden seit dem 07.11. 22:39: Slot 6 war mitten im Abblenden gespeichert (FADESCREENSTATE 1;
    // FADESCREENTOBLACKF legt den Rueckruf - hier Q102_RUPERTDISTANCELIMIT_TELEPORT, die Abstandsgrenze in Ruperts Geleit - als
    // Ikarus-Funktionszeiger FADEOUTFUNCPTR ab), die Sitzung um 22:58 lud ihn mit u210, also ohne P247: der Rueckruf mit dem Aufblenden
    // (FADESCREENFROMBLACK) lief ins Leere, FADEOUTFINISH setzte SCREENFADED, und das stand danach in jedem Spielstand. P247 verhindert
    // diesen Weg; es bleiben schon so gespeicherte Staende und jeder andere Weg, auf dem das Aufblenden ausfaellt (im Coop speichert der
    // Partner jederzeit). Jetzt: stehen SCREENFADED/FADESCREENSTATE 60 s unveraendert auf "abgeblendet", ohne offenen Dialog, ruft GT
    // FADESCREENFROMBLACKNOW der Mod (Ansicht klar, Zustand 0, FADEINFINISH: SCREENFADED aus, Bewegungssperre des Helden aus). Ein
    // regulaeres Abblenden dauert wenige Sekunden. Allgemein: nur in Skripten mit diesen Namen (Archolos' Abblend-Funktionen).
    static int CoopFadeLockResets = 0;

    struct CoopFadeLockSymbols { zCPar_Symbol* faded; zCPar_Symbol* state; int nowFunc; };

    static const CoopFadeLockSymbols& CoopFadeLockSyms() {
        static zCParser* forParser = NULL;
        static CoopFadeLockSymbols s = { NULL, NULL, -1 };
        if (parser != forParser) {
            forParser = parser;
            s = { NULL, NULL, -1 };
            if (parser) {
                auto intVar = [](zCPar_Symbol* x) { return x && x->type == zPAR_TYPE_INT && !(x->flags & zPAR_FLAG_CONST) && x->ele == 1; };
                zCPar_Symbol* faded = parser->GetSymbol(zSTRING("SCREENFADED"));
                zCPar_Symbol* state = parser->GetSymbol(zSTRING("FADESCREENSTATE"));
                const int fn = parser->GetIndex(zSTRING("FADESCREENFROMBLACKNOW"));
                zCPar_Symbol* fs = fn >= 0 ? parser->GetSymbol(fn) : NULL;
                if (intVar(faded) && fs && fs->type == zPAR_TYPE_FUNC) {
                    s.faded = faded;
                    s.state = intVar(state) ? state : NULL;
                    s.nowFunc = fn;
                }
            }
        }
        return s;
    }

    static void CoopFadeLockPulse(const oCInformationManager& im) {
        static long long nextCheck = 0, since = 0;
        static int lastFaded = 0, lastState = 0;
        if (CurrentMs < nextCheck)
            return;
        nextCheck = CurrentMs + 1000;
        const CoopFadeLockSymbols& s = CoopFadeLockSyms();
        if (!s.faded || !player || !ogame || !ogame->GetGameWorld())
            return;
        int faded = 0, state = 0;
        s.faded->GetValue(faded, 0);
        if (s.state)
            s.state->GetValue(state, 0);
        const bool open = !im.IsDone && (im.Npc || im.Player);
        if ((!faded && !state) || open || faded != lastFaded || state != lastState || !since) {
            lastFaded = faded;
            lastState = state;
            since = CurrentMs;
            return;
        }
        const long long stuckS = (CurrentMs - since) / 1000;
        if (stuckS < 60)
            return;
        parser->CallFunc(s.nowFunc);
        int fadedAfter = 0, stateAfter = 0;
        s.faded->GetValue(fadedAfter, 0);
        if (s.state)
            s.state->GetValue(stateAfter, 0);
        CoopFadeLockResets++;
        since = CurrentMs;
        if (CoopFadeLockResets <= 20)
            OthelloCompatLog("[%lld s, %s] Abblendsperre: SCREENFADED=%d, FADESCREENSTATE=%d seit %lld s unveraendert, ohne Dialog - "
                "FADESCREENFROMBLACKNOW aufgerufen (danach %d/%d), NPCs reden wieder mit dem Helden (P254, %d.)", DiagSeconds(), DiagRole(),
                faded, state, stuckS, fadedAfter, stateAfter, CoopFadeLockResets);
    }

    void CoopDialogWatchPulse() {
        static oCNpc* watchNpc = NULL;
        static long long since = 0;
        static int tries = 0;

        oCInformationManager& im = oCInformationManager::GetInformationManager();
        CoopTalkLockPulse(im);   // u216 (P253)
        CoopFadeLockPulse(im);   // u217 (P254)
        if (!im.Npc || !im.Player || im.IsDone) {
            watchNpc = NULL;
            tries = 0;
            return;
        }
        bool heldWeg = false;
        if (DialogWatchVeraltet(im, heldWeg)) {   // u155 (P160): vor jedem Zugriff auf NPC und Held
            DialogWatchEnd(im, heldWeg);
            CoopDialogStaleEnds++;
            if (DialogWatchDarfLoggen())
                OthelloCompatLog("[%lld s, %s] Dialog-Waechter: offener Dialog gehoert nicht mehr zu dieser Welt (%s) - beendet, ohne die alten Figuren anzufassen",
                    DiagSeconds(), DiagRole(), heldWeg ? "Player ist nicht der Held" : "NPC nicht in der NPC-Liste");
            watchNpc = NULL;
            tries = 0;
            return;
        }
        if (DialogWatchHatProcessInfos(im.Npc)) {
            watchNpc = NULL;
            tries = 0;
            return;
        }
        if (watchNpc != im.Npc) {          // Zustand neu gesehen: erst beobachten
            watchNpc = im.Npc;
            since = CurrentMs;
            tries = 0;
            return;
        }
        if (CurrentMs < since + 1500)
            return;

        char name[64];
        CDiagName(im.Npc, name, sizeof(name));
        char flags[96];
        _snprintf(flags, sizeof(flags), "WartetSkript=%d WartetEnde=%d WartetAuswahl=%d Wichtig=%d/%d",
            im.IsWaitingForScript, im.IsWaitingForEnd, im.IsWaitingForSelection, im.ImportantCurrent, im.ImportantMax);
        flags[sizeof(flags) - 1] = 0;
        if (tries < 3) {
            oCMsgConversation* msg = new oCMsgConversation(oCMsgConversation::EV_PROCESSINFOS, im.Player);
            msg->SetInUse(TRUE);
            im.Npc->GetEM()->OnMessage(msg, im.Player);
            tries++;
            if (DialogWatchDarfLoggen())
                OthelloCompatLog("[%lld s, %s] Dialog-Waechter: Dialog mit %s ohne EV_PROCESSINFOS (%s) - Nachricht neu eingereiht (Versuch %d)",
                    DiagSeconds(), DiagRole(), name, flags, tries);
        }
        else {
            im.Exit();
            if (DialogWatchDarfLoggen())
                OthelloCompatLog("[%lld s, %s] Dialog-Waechter: Dialog mit %s haengt weiter (%s) - beendet (Exit)", DiagSeconds(), DiagRole(), name, flags);
            watchNpc = NULL;
            tries = 0;
        }
        since = CurrentMs;
    }

    // Protokoll: wer leert die Warteschlange des Gespraechspartners waehrend eines offenen Dialogs?
    void __fastcall oCNpc_ClearEM_DialogWatch(oCNpc* _this, void* vtable);
    CInvoke<void(__thiscall*)(oCNpc*)> Ivk_oCNpc_ClearEM_DialogWatch(CoopAddr::oCNpc_ClearEM, &oCNpc_ClearEM_DialogWatch,
        COOP_IVK(DialogWatch));
    void __fastcall oCNpc_ClearEM_DialogWatch(oCNpc* _this, void* vtable) {
        oCInformationManager& im = oCInformationManager::GetInformationManager();
        if (_this && im.Npc == _this && !im.IsDone) {
            const void* ret = _ReturnAddress();
            char name[64], modul[96];
            CDiagName(_this, name, sizeof(name));
            ModuleName(ret, modul, sizeof(modul));
            if (DialogWatchDarfLoggen())
                OthelloCompatLog("[%lld s, %s] Dialog-Waechter: ClearEM auf Gespraechspartner %s waehrend des Dialogs, Aufrufer %s %p",
                    DiagSeconds(), DiagRole(), name, modul, ret);
        }
        Ivk_oCNpc_ClearEM_DialogWatch(_this);
    }

#else   // Modul in dieser Engine nicht freigegeben (CoopModulesReport)
    int CoopDialogLoadEnds = 0;
    int CoopDialogStaleEnds = 0;
    std::string CoopDialogLastEnded;
    int CoopTalkLockResets = 0;
    void CoopDialogWatchPulse() {}
    void CoopDialogWatchLoadBegin() {}
    void CoopTalkLockAfterLoad() {}
    void CoopTalkLockCheckSoon(const char*) {}
#endif
}
