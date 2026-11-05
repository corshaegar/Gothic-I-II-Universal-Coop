namespace GOTHIC_ENGINE {
    // u181 - gemeinsames Spieltempo (Zeitraffer) fuer Testlaeufe (-cooptempo, Runner --tempo)
    //
    // Laufen Host und Client verschieden schnell, sieht es beim jeweils anderen falsch aus. Darum setzt kein PC den Zeitfaktor mehr
    // selbst (zCTimer::SetMotionFactor), sondern meldet nur seinen Wunsch (CoopTempoWunsch, jedes Bild). Jeder PC schickt alle 500 ms
    // und bei Aenderung Wunsch und "beschaeftigt" (eigener Held im Dialog oder mit gezogener Waffe) an die Mitspieler (SYNC_TEMPO:
    // w Wunsch x100, b beschaeftigt). Alle laufen mit dem kleinsten Wunsch; ist einer beschaeftigt, laufen alle mit 1 - Dialoge sind zu
    // hoeren, Kaempfe normal schnell. Ohne Wunsch (normales Spiel) gilt 1: wer keinen Zeitraffer will, bremst alle. Anzeige oben rechts
    // ("Zeitraffer x3"), keine Chatzeile; im Protokoll nur die Wechsel.
    static CRITICAL_SECTION* TempoLock() {
        static CRITICAL_SECTION cs;
        static bool init = false;
        if (!init) {
            InitializeCriticalSection(&cs);
            init = true;
        }
        return &cs;
    }

    struct TempoPeer {
        int wunsch = 100;        // x100
        bool busy = false;
        long long ms = 0;
    };
    static std::map<std::string, TempoPeer> TempoPeers;   // Netz-Faden schreibt, Spieltakt liest (TempoLock)
    static float TempoWunschWert = 1.f;
    static long long TempoWunschMs = 0;
    static float TempoAktiv = 1.f;
    static long long TempoNextSendMs = 0;
    static int TempoLastSentW = -1, TempoLastSentB = -1;
    static bool TempoSentAny = false;

    // Testinstanz, jedes Bild: gewuenschter Faktor (1 = normal); ohne Aufruf fuer 2 s gilt wieder 1
    void CoopTempoWunsch(float f) {
        TempoWunschWert = f < 1.f ? 1.f : f > 8.f ? 8.f : f;
        TempoWunschMs = GetCurrentMs();
    }

    float CoopTempoAktiv() {
        return TempoAktiv;
    }

    // Netz-Faden (ProcessCoopPacket)
    void CoopTempoReceive(json& e, const std::string& id) {
        TempoPeer p;
        p.wunsch = e.contains("w") && e["w"].is_number_integer() ? e["w"].get<int>() : 100;
        p.busy = e.contains("b") && e["b"].is_number_integer() && e["b"].get<int>() != 0;
        p.ms = GetCurrentMs();
        EnterCriticalSection(TempoLock());
        TempoPeers[id] = p;
        LeaveCriticalSection(TempoLock());
    }

    static void TempoLog(const char* fmt, ...) {
        static CoopDiagBudget budget = { 0, 0 };
        if (!CoopDiagAllow(budget, 20))
            return;
        char buf[512];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        buf[sizeof(buf) - 1] = 0;
        OthelloCompatLog("[%lld s, %s] Tempo: %s", DiagSeconds(), DiagRole(), buf);
    }

    // Plugin.cpp Game_Loop, jedes Bild: senden, Faktor setzen, Anzeige
    void CoopTempoPulse() {
        if (!ztimer || !player || !ogame || IsLoadingLevel)
            return;
        const long long now = GetCurrentMs();
        const float own = now - TempoWunschMs < 2000 ? TempoWunschWert : 1.f;
        const bool busy = IsPlayerTalkingWithAnybody() || !oCInformationManager::GetInformationManager().IsDone ||
            player->GetWeaponMode() != NPC_WEAPON_NONE;
        // Partner: Wunsch und beschaeftigt (aelter als 3 s zaehlt nicht mehr)
        float eff = busy ? 1.f : own;
        bool peerTempo = false;
        std::string why = busy ? "eigener Held beschaeftigt" : own > 1.01f ? "" : "kein Wunsch";
        EnterCriticalSection(TempoLock());
        for (auto& kv : TempoPeers) {
            if (now - kv.second.ms > 3000)
                continue;
            const float w = kv.second.wunsch / 100.f;
            if (w > 1.01f)
                peerTempo = true;
            if (kv.second.busy) {
                if (eff > 1.f)
                    why = kv.first + " beschaeftigt";
                eff = 1.f;
            }
            else if (w < eff) {
                why = kv.first + " will x" + std::to_string(w).substr(0, 3);
                eff = w;
            }
        }
        LeaveCriticalSection(TempoLock());
        // senden: nur wer Tempo will oder es zuletzt wollte (sonst kein Verkehr fuer Spieler ohne Testinstanzen)
        const int w100 = (int)(own * 100.f + 0.5f);
        const int b = busy ? 1 : 0;
        if ((ServerThread || ClientThread) && (own > 1.01f || peerTempo || TempoSentAny) &&
            (now >= TempoNextSendMs || w100 != TempoLastSentW || b != TempoLastSentB)) {
            json j;
            j["type"] = SYNC_TEMPO;
            j["id"] = MyselfId.ToChar();
            j["w"] = w100;
            j["b"] = b;
            ReadyToSendJsons.enqueue(j);
            TempoNextSendMs = now + 500;
            TempoLastSentW = w100;
            TempoLastSentB = b;
            TempoSentAny = own > 1.01f || peerTempo;
        }
        // Faktor: nur, wenn hier oder beim Partner Tempo im Spiel ist (oder es gerade noch war)
        if (own > 1.01f || peerTempo || TempoAktiv != 1.f) {
            if (fabsf(ztimer->factorMotion - eff) > 0.01f)
                ztimer->SetMotionFactor(eff);
            if (fabsf(TempoAktiv - eff) > 0.01f) {
                TempoLog("x%.0f -> x%.0f (%s)", TempoAktiv, eff, eff > TempoAktiv ? "alle wollen schneller" : why.c_str());
                TempoAktiv = eff;
            }
        }
        // Anzeige oben rechts, keine Chatzeile (Nutzer: "nicht als systemnachricht")
        if (TempoAktiv > 1.01f && screen) {
            auto font = screen->GetFontName();
            auto color = screen->fontColor;
            screen->SetFont(zSTRING("Font_Old_10_White_Hi.TGA"));
            screen->SetFontColor(zCOLOR(255, 235, 150, 255));
            zSTRING t(("Zeitraffer x" + std::to_string((int)(TempoAktiv + 0.5f))).c_str());
            screen->Print(8191 - screen->FontSize(t) - 120, 120, t);
            screen->SetFont(font);
            screen->SetFontColor(color);
        }
    }
}
