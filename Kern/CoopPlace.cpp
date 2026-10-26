// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {

    // --- u10: Platz neben einem Helden, Bewegung danach zuruecksetzen ------------------------------------------------------
    //
    // Bis u9 in CoopSaveSync.cpp (v25, v89, v91). Auch die Teleport-Rune (CoopTeleport.cpp) und das gemeinsame neue Spiel
    // (CoopNewGame.cpp) setzen den Helden damit - in Gothic 2 NotR ist das Speichern-Modul noch nicht freigegeben, Neues Spiel
    // schon (Test u9: "die Spawnposition ist dann ineinander"). Darum hier, fuer alle Engines gebaut; nur SDK-Funktionen
    // (zCWorld::TraceRayNearestHit, oCVob::SearchNpcPosition) und CoopResetPosSafe (OthelloCompat.cpp).

    bool CoopSaveSyncHeroApplyPending();   // CoopSaveSync.cpp - eigener Held wird gerade wiederhergestellt

    static bool PendingMoveReset = false;             // v25: Bewegung im ersten Spielbild zuruecksetzen

    static void PlaceLog(const char* fmt, ...) {
        char buf[600];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        buf[sizeof(buf) - 1] = 0;
        OthelloCompatLog("[%lld s, %s] Platz: %s", DiagSeconds(), DiagRole(), buf);
    }

    // --- v89: Platz neben dem Helden des Stands ---------------------------------------------------------------------------
    // Nutzer (22.10.2026): "man spawnt immer noch manchmal ineinander bzw. kann einer oder beide sich nicht bewegen ... in dem Fall,
    // dass der Mitspieler fuer den Savegame keinen eigenen Standort hat, ... paar Meter neben uns spawnt" - "Der Standort neben
    // unserem Charakter sollte natuerlich legal sein, also nicht in einer Wand oder im Nichts, am besten auf dem Boden".
    // Test v88: kein eigener Held zu dieser Kennung -> zeitlich naechster aus einem anderen Stand samt dessen Lage, 0,5 m neben dem
    // Host (5811/35366 gegen 5825/35317) - beide steckten fest. Ebenso blieb ohne jeden eigenen Helden der Held genau auf dem Host.
    // Nach dem Laden einer fremden Welt steht der Spieler zunaechst dort, wo der Held dieses Stands stand (bei der Uebernahme: der
    // Host). Gesucht wird ringsum (rechts, links, hinten, schraeg, vorn; 2,5 / 3,5 / 1,8 m) eine Stelle, die
    //  - vom Helden des Stands aus in Brusthoehe frei zu sehen ist (keine Wand: zCWorld::TraceRayNearestHit 0x005F46E0, statische
    //    Polygone und Vobs, ohne Figuren),
    //  - Boden hat (Strahl bis 4 m nach unten, kein Wasser) hoechstens 1,2 m ueber/unter dessen Boden,
    //  - fuer einen NPC frei ist (oCVob::SearchNpcPosition 0x006D3B10: Boden ueber GetFloorPosition 0x006D4410, Platzpruefung
    //    0x006D3680; im Speicherabbild unveraendert) und danach noch 1,2-7 m entfernt, in Sicht und auf aehnlicher Hoehe liegt.
    // Allgemein G1.
    static bool CoopFloorBelow(zCWorld* w, const zVEC3& p, float& y, bool& water) {
        water = false;
        const zVEC3 start(p.n[0], p.n[1] + 80.f, p.n[2]);
        const zVEC3 ray(0.f, -480.f, 0.f);
        if (!w->TraceRayNearestHit(start, ray, (zCVob*)player, zTRACERAY_STAT_POLY | zTRACERAY_VOB_IGNORE_CHARACTER | zTRACERAY_POLY_TEST_WATER) ||
            !w->traceRayReport.foundHit)
            return false;
        y = w->traceRayReport.foundIntersection.n[1];
        zCPolygon* poly = w->traceRayReport.foundPoly;
        water = poly && poly->material && poly->material->matGroup == zMAT_GROUP_WATER;
        return true;
    }

    static bool CoopClearLine(zCWorld* w, const zVEC3& a, const zVEC3& b) {
        const zVEC3 ray = b - a;
        return !(w->TraceRayNearestHit(a, ray, (zCVob*)player, zTRACERAY_STAT_POLY | zTRACERAY_VOB_IGNORE_CHARACTER | zTRACERAY_POLY_IGNORE_TRANSP) &&
            w->traceRayReport.foundHit);
    }

    // v102: whom - fuer die Meldung (Teleport-Rune, CoopTeleport.cpp: "den Mitspieler")
    static bool CoopPlaceBesideHost(const zVEC3& host, const zVEC3& hostAt, std::string& how, const char* whom = "den Helden des Stands") {
        zCWorld* w = ogame ? ogame->GetGameWorld() : NULL;
        if (!w || !player) {
            how = "keine Welt - Lage unveraendert";
            return false;
        }
        float ax = hostAt.n[0], az = hostAt.n[2];
        const float al = sqrtf(ax * ax + az * az);
        if (al < 0.01f) { ax = 0.f; az = 1.f; }
        else { ax /= al; az /= al; }
        float hostFloor = host.n[1];
        bool water = false;
        const bool hostHasFloor = CoopFloorBelow(w, host, hostFloor, water);
        const float above = hostHasFloor ? host.n[1] - hostFloor : 0.f;   // Ursprung der Figur ueber dem Boden
        const zVEC3 eye(host.n[0], host.n[1] + 60.f, host.n[2]);
        static const float angles[] = { 90.f, -90.f, 180.f, 135.f, -135.f, 45.f, -45.f, 0.f };
        static const float dists[] = { 250.f, 350.f, 180.f };
        int tried = 0;
        const char* lastWhy = "-";
        for (float d : dists) {
            for (float ang : angles) {
                tried++;
                const float r = ang * 3.14159265f / 180.f;
                const float dx = ax * cosf(r) - az * sinf(r), dz = ax * sinf(r) + az * cosf(r);
                const zVEC3 c(host.n[0] + dx * d, host.n[1], host.n[2] + dz * d);
                if (!CoopClearLine(w, eye, zVEC3(c.n[0], c.n[1] + 60.f, c.n[2]))) { lastWhy = "Wand"; continue; }
                float fy = 0.f;
                if (!CoopFloorBelow(w, c, fy, water)) { lastWhy = "kein Boden"; continue; }
                if (water) { lastWhy = "Wasser"; continue; }
                if (hostHasFloor && fabsf(fy - hostFloor) > 120.f) { lastWhy = "H\xF6he"; continue; }
                zVEC3 p(c.n[0], fy + above, c.n[2]);
                if (!player->SearchNpcPosition(p)) { lastWhy = "kein freier Platz"; continue; }
                const float ex = p.n[0] - host.n[0], ez = p.n[2] - host.n[2];
                const float dist = sqrtf(ex * ex + ez * ez);
                if (dist < 120.f || dist > 700.f || fabsf(p.n[1] - host.n[1]) > 150.f) { lastWhy = "Suche zu weit/zu nah"; continue; }
                if (!CoopClearLine(w, eye, zVEC3(p.n[0], p.n[1] + 60.f, p.n[2]))) { lastWhy = "Wand nach der Suche"; continue; }
                player->SetCollDet(FALSE);
                player->SetPositionWorld(p);
                player->SetCollDet(TRUE);
                player->SetHeadingAtWorld(zVEC3(ax, 0.f, az));
                char b[200];
                _snprintf(b, sizeof(b), "neben %s gesetzt (%.1f m, Richtung %d Grad, %d. Stelle, %.0f/%.0f/%.0f)",
                    whom, dist / 100.f, (int)ang, tried, p.n[0], p.n[1], p.n[2]);
                b[sizeof(b) - 1] = 0;
                how = b;
                return true;
            }
        }
        how = std::string("keine legale Stelle neben ") + whom + " (zuletzt: " + lastWhy + ") - Lage unveraendert";
        return false;
    }

    // v91: gemeinsames neues Spiel - beide beginnen am selben Startpunkt; der Mitspieler tritt im ersten Spielbild an eine legale
    // Stelle daneben (wie v89 beim Laden ohne eigene Lage). Die Bewegung setzt CoopMoveResetPulse im naechsten Bild zurueck.
    // Bis u9 CoopSaveSyncPlaceBesideStart (CoopSaveSync.cpp).
    void CoopPlaceBesideStart() {
        if (!player)
            return;
        const zVEC3 at = player->GetPositionWorld();
        const zVEC3 dir = player->GetAtVectorWorld();
        std::string how;
        PendingMoveReset = true;
        CoopPlaceBesideHost(at, dir, how);
        PlaceLog("Neues Spiel gemeinsam: Startpunkt wie beim Host - %s", how.c_str());
    }

    // u39 (P81): Weltstand-Tausch (CoopSaveSync.cpp) - die Weltdatei, in der ein Mitspieler beim Speichern stand, enthaelt dessen
    // Helden. oCGame::ChangeLevel (G2A 0x006C7838) meldet dann nur "U: CHL: Two Players found..." und setzt den eigenen Helden
    // trotzdem ein (vtable +0x58); der aus der Datei bliebe als NPC mit der Instanz des Helden stehen. Nach jedem Laden (Spielstand,
    // Weltwechsel, auch im Weltprozess): solche NPCs aus der Welt nehmen - ausser dem eigenen Helden und den Coop-Figuren (eigene
    // Instanz). Allgemein (G1/G2); im Spiel hat nur der Held diese Instanz.
    int CoopRemoveStrayHeroes(const char* why) {
        if (!player || !ogame || !ogame->GetGameWorld())
            return 0;
        const int inst = player->GetInstance();
        std::vector<oCNpc*> stray;
        for (auto* l = CoopWorldNpcListFirst(ogame->GetGameWorld()); l; l = l->next) {
            oCNpc* n = l->data;
            if (n && n != player && n->GetInstance() == inst && !PlayerNpcs.count(n))
                stray.push_back(n);
        }
        for (oCNpc* n : stray) {   // erst sammeln, dann entfernen (die Liste aendert sich dabei)
            const zVEC3 p = n->GetPositionWorld();
            PlaceLog("%s: zweiten Helden %s bei %.0f/%.0f/%.0f aus der Welt genommen (aus der Weltdatei eines Mitspielers)", why,
                n->GetObjectName().ToChar(), p.n[0], p.n[1], p.n[2]);
            CoopDeleteNpc(ogame->spawnman, n);
        }
        return (int)stray.size();
    }

    // v25: nach dem Einsetzen des Helden die Bewegung zuruecksetzen - Original oCNpc::ResetPos an der aktuellen
    // Stelle (OthelloCompat.cpp, CoopResetPosSafe), Blickrichtung danach wieder setzen (ResetPos dreht zurueck).
    // Bis u9 HandleMoveReset (CoopSaveSync.cpp); aufgerufen von CoopSaveSyncPulse und CoopNewGamePulse (einmal je Auftrag).
    void CoopMoveResetPulse() {
        if (!PendingMoveReset || !player || IsLoadingLevel || CoopSaveSyncHeroApplyPending())
            return;
        PendingMoveReset = false;
        const zVEC3 at = player->GetAtVectorWorld();
        CoopResetPosSafe(player, player->GetPositionWorld());
        player->SetHeadingAtWorld(at);
        PlaceLog("Bewegung des eigenen Helden zurueckgesetzt (ResetPos an der eigenen Stelle)");
    }
}
