// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {

    // --- Pruefebene 2: Schnappschuesse -------------------------------------------------------
    //
    // Schnappschuss (alle 5 s, sobald verbunden), system\GothicCoopSnapshot.log:
    //   H <ms> <Rolle> <Tag> <hh:mm> <eigene Kennung>                       Kopf
    //   S <ms> <Rolle> <Name> <x> <y> <z> <HP> <HPmax> <Waffenmodus> <tot> <bewusstlos> <KI-Zustand> <Animation Ebene 1>
    //   Z <ms> <Rolle> <Art> <Update-Typ> <Gesamt>                          laufende Zaehler (je Schnappschuss)
    // Der Host schreibt alle NPCs, die er sendet, und die Coop-Figuren; der Client alle NPCs, fuer
    // die er Daten empfaengt, und die Figur des Hosts; beide sich selbst unter der eigenen Kennung.
    // Gleicher Name = gleicher NPC (NPC-Namen auf beiden Seiten identisch, siehe CoopDiag.cpp).
    // Auswertung: _othello_backup\compare_sync.py (Host- und Client-Datei nebeneinander).
    //
    // u237 (Nutzer 09.11.: "Die tests sollten auch nicht per hotkey erreichbar sein"): der gefuehrte Testablauf mit der Taste F7
    // (Konfiguration "testStepKey", Schritte im Chat, Marker M/C je Schritt) ist entfernt - Tests laufen ueber die automatischen Laeufe.
    //
    // u253 (P304, Nutzer 12.11.: "komplett raus damit wir brauchen es ja nicht, fuer Othello kann es ja bleiben, aber abgeschaltet"):
    // abgeschaltet ueber COOP_READY_Trace = 0 (CoopCore.cpp), der Code bleibt. Kein Werkzeug wertet die Datei mehr aus, und jede Zeile
    // oeffnet und schliesst sie neu - in den eigenen Logs 60 bis 120 Zeilen alle 5 s (Figuren und Zaehler). Anlass: ein Spieler meldete
    // beim Mitspieler (Gothic 1) etwa 1 s Stillstand alle 5 s.

#if COOP_BUILD(Trace)   // u3: bis u2 nur G1 - Freigabe je Engine in CoopCore.cpp

    static long long CoopNextSnapshotMs = 0;
    static unsigned long CoopSnapshotLines = 0;
    static bool CoopSnapshotSessionWritten = false;

    static void SnapLog(const char* fmt, ...) {
        if (CoopWpIsWorld())
            return;   // u33: Weltprozess - die Datei gehoert dem Hauptspiel
        CoopLogArchiveOnce();   // v35
        FILE* f = fopen("system\\GothicCoopSnapshot.log", "a");
        if (!f)
            return;
        va_list ap;
        va_start(ap, fmt);
        vfprintf(f, fmt, ap);
        va_end(ap);
        fputc('\n', f);
        fclose(f);
        CoopSnapshotLines++;
    }

    static void SnapNpc(long long now, const char* role, const char* name, oCNpc* npc) {
        if (!npc)
            return;
        zVEC3 p = npc->GetPositionWorld();
        const char* state = npc->state.curState.name.ToChar();
        SnapLog("S\t%lld\t%s\t%s\t%.0f\t%.0f\t%.0f\t%d\t%d\t%d\t%d\t%d\t%s\t%s",
            now, role, name, p.n[0], p.n[1], p.n[2],
            npc->GetAttribute(NPC_ATR_HITPOINTS), npc->GetAttribute(NPC_ATR_HITPOINTSMAX),
            npc->GetWeaponMode(), npc->IsDead() ? 1 : 0, npc->IsUnconscious() ? 1 : 0,
            (state && state[0]) ? state : "-", CoopBaseLayerAni(npc));
    }

    static void CoopWriteSnapshot() {
        const long long now = GetCurrentMs();
        const char* role = DiagRole();
        if (!CoopSnapshotSessionWritten) {
            CoopSnapshotSessionWritten = true;
            SYSTEMTIME st;
            GetLocalTime(&st);
            SnapLog("# Sitzung %04d-%02d-%02d %02d:%02d:%02d, Rolle %s, Kennung %s", st.wYear, st.wMonth,
                st.wDay, st.wHour, st.wMinute, st.wSecond, role, zSTRING(MyselfId).ToChar());
        }
        int day = 0, h = 0, m = 0;
        ogame->GetTime(day, h, m);
        SnapLog("H\t%lld\t%s\t%d\t%02d:%02d\t%s", now, role, day, h, m, zSTRING(MyselfId).ToChar());
        if (player)
            SnapNpc(now, role, zSTRING(MyselfId).ToChar(), player);
        if (ServerThread) {
            for (auto& p : BroadcastNpcs)
                if (p.second && !p.second->destroyed)
                    SnapNpc(now, role, zSTRING(p.first).ToChar(), p.second->npc);
            for (auto& p : PlayerNpcs)
                SnapNpc(now, role, zSTRING(p.second).ToChar(), p.first);
        }
        else if (ClientThread) {
            for (auto& p : SyncNpcs)
                if (p.second && !p.second->destroyed)
                    SnapNpc(now, role, zSTRING(p.first).ToChar(), p.second->npc);
        }
        // Laufende Gesamtzaehler je Art und Update-Typ - damit lassen sich fuer jeden Zeitraum beide Seiten vergleichen.
        static const char* kinds[] = { "gesendet", "empfangen", "angewendet", "uebersprungen" };
        for (int k = 0; k < TRACE_KINDS; k++)
            for (int t = 0; t < TRACE_TYPES; t++)
                if (CoopTraceCounts[k][t])
                    SnapLog("Z\t%lld\t%s\t%s\t%s\t%lu", now, role, kinds[k], CoopUpdateTypeName(t), CoopTraceCounts[k][t]);
    }

    // Aus Game_Loop.
    void CoopTracePulse() {
        if (!ServerThread && !ClientThread)
            return;
        if (CurrentMs >= CoopNextSnapshotMs && CoopSnapshotLines < 80000) {
            CoopNextSnapshotMs = CurrentMs + 5000;
            CoopWriteSnapshot();
        }
    }

#else   // Modul in dieser Engine nicht freigegeben (CoopModulesReport)

    void CoopTracePulse() {}

#endif
}
