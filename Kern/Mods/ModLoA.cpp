// Legend of Ahssun - Mod-Profil COOP_PROFILE_LOA (docs\MODULE.md). Hier stehen nur Sonderwege, die allein diese Mod braucht.
// Was in LoA gefunden wurde, aber in jeder Mod auftreten kann, liegt im Fundament: P177 (LeGo-Haken vor GT, CoopAudit.cpp),
// P178 (Hauptmenue mit eigenen Zeilen, CoopMenuInject.cpp), P180 (DoShootArrow ohne Munition, CoopWorldSync.cpp), P181 (Fehler mitten
// im Laden, CoopSaveSync.cpp).

namespace GOTHIC_ENGINE {

    // u187: einmal je Start (CoopModsInit) - was GT ueber LoA weiss, steht im Log; der Fenstermodus wird gemeldet, weil Videos dort in
    // GD3D11 haengen koennen (COOP-OPTIMIERUNG.md, Liste 6s: Hauptfaden in zCVideoPlayer::Play -> ZMOUSEFIX.DLL -> g2a_avx2.dll)
    void CoopModLoAInit() {
        const std::string ini = CoopGameRoot() + "\\system\\Gothic.ini";
        const bool window = GetPrivateProfileIntA("VIDEO", "zStartupWindowed", 0, ini.c_str()) != 0;
        OthelloCompatLog("Mod-Profil Legend of Ahssun: Fundament %s mit LoA-Hinweisen - LeGo-Haken vor GT versetzt GT selbst (P177), "
            "Hauptmenue mit LoA-Zeilen (P178), eigene Heldenwerte, NPC-Geplauder, Uebung und Erfolge je PC (P190, P198) - Uebung, Talente, "
            "Rezepte und Erfolge bleiben auch beim Verbinden die eigenen (P207)%s", COOP_FASSUNG,
            window ? "; Fenstermodus: Videos koennen in GD3D11 haengen (Liste 6s) - randloses Vollbild empfohlen" : "");
    }

    // u194 (P196): Daten des Profils im Format der Profil-Datei (Mods\CoopModProfile.h); eine system\GothicCoopMod-LoA.json ergaenzt
    // sie. u188 (P190): Skript-Ganzzahlen von LoA, die den eigenen Helden beschreiben (Umgebungs-Gespraeche: spricht der Held gerade,
    // wie oft war er nicht im Gespraech). Der Story-Abgleich schickte sie zum Partner (LoA-Logs u177: "AMBVAR_HERO_TALKING=0",
    // "TIMES_NOT_IN_DIALOGUE=2") - sie bleiben je PC. Ikarus/LeGo-Werte schliesst schon das Fundament aus (DlgScriptLibraryVar).
    // u195 (P198): weitere Werte je PC - aus den LoA-Logs u189-u194 (der Story-Abgleich schickte sie zum Partner) und aus der Analyse
    // der LoA-Skripte (tools\mod_analyse.py, docs\mod-analyse\LoA.md: geschrieben nur von Gegenstaenden, staendigen Schleifen, eigenen
    // Engine-Haken und Gespraechen des Helden mit sich selbst, von keinem Dialog gelesen; von Hand geprueft). Nicht je PC, obwohl die
    // Analyse sie vorschlug: Erz- und Goldvorkommen der Welt (GOLDMOB_, ERZMOB_, EDELSTEIN_), Missionswerte (MISVAR_), LoAs
    // "Info erzaehlt"-Merker (BEKANNT_ - Dialoge fragen sie ab), Ereignisse der Welt und Bosse (BUGEGGS, BUGSPAWN, ST_BOSS, MAGUS_ ...).
    // u197 (P207): aufgeteilt. "heroVars" = Werte des eigenen Helden (Uebung, Talente, Rezepte, Erfolge, Boni, laufende Wirkungen) - sie
    // gehen mit dem Helden (COOPHERO.TXT) und bleiben beim Uebernehmen des Host-Stands (bei jedem Verbinden) die eigenen; bis u196 kamen
    // dabei die des Hosts. "perPcVars" = Zustand dieses PCs (Geplauder, Bewegung, Menues, Handles) - beim Laden gilt der Spielstand.
    const char* CoopModLoAProfile() {
        return R"json({
            "perPcVars": [
                // u188 (P190): beschreiben den eigenen Helden im Moment, nicht die Geschichte
                "AMBVAR_HERO_TALKING", "TIMES_NOT_IN_DIALOGUE",
                // Zufallsstand des NPC-Geplauders (B_SAY_SMALLTALK)
                "VAR_RANDOMSMALLTALK", "VAR_SMALLTALK_TOOMANYLOOPS", "VAR_LRT_*",
                // Teleport-Wuerfel-Menue, Bewegung des eigenen Helden, Inventar- und Fokus-Anzeige
                "AMBVAR_TRANSFORMTARGET", "TELE_CHANGE", "TELE_EXCHANGE",
                "HERO_GOBACKWARD", "HERO_JUMPING", "HERO_STRAFING", "PLAYER_DREWWEAPON", "NMB", "NPCHALFHP", "NPCFOURTHHP", "NPCFULLHP",
                "SAVINGDISABLED_LASTCHECK",
                // Handles und Zeiger des eigenen Prozesses (LeGo-Leisten, Anim8, Lichtsystem FDL) - gehoeren zum geladenen Stand
                "ICEBARHANDLE", "DESERTBARHANDLE", "MAGUS_FIRE_POS", "ID_NPCPTR", "ID_UNPCPTR", "ITM_PTR", "FDLINT_*", "FDL_*"
            ],
            "heroVars": [
                // Lernen durch Uebung (B_FIGHTLERN, LOA_INCREASE_TALENT), Lernbuecher, Erfolgszaehler des Spielers
                "PRAXIS_NOW", "PRAXIS_NEXT", "NOW_1H_RAISES", "NOW_2H_RAISES", "NOW_BOW_RAISES", "NOW_CROSSBOW_RAISES",
                "LERNE_ARMBRUST", "LERNE_BOGEN", "ACHVAR_*",
                // Talente, Rezepte, Bauplaene und Gelesenes des eigenen Helden
                "TALENT_GETREIDESCHNAPS", "TALENT_LOUSHAMMER", "TALENT_REISSCHNAPS", "TALENT_SUMPFKRAUT", "TALENT_WIRANONRUM",
                "REZIPE_APPLEPIE", "REZIPE_DUNKELPILZRAGOUT", "REZIPE_MEATPOT",
                "AMBVAR_KNOWS_BANDITENHAMMER", "AMBVAR_WIRANONRUM", "AMBVAR_CRAWL_L_ARMOR", "AMBVAR_TALENT_CRAWLERARMOR",
                "AMBVAR_BUDDLERBOOK_READ", "AMBVAR_EDELSTEINE_READ", "AMBVAR_EDELSTEINE0_READ", "AMBVAR_EDELSTEINE2_READ",
                "AMBVAR_EDELSTEINESCHUERFEN",
                "ITWR_VAR_MINENDORF1", "ITWR_VAR_MINENDORF2", "ITWR_VAR_MINENDORF3", "ITWR_VAR_MINENDORF4", "ITWR_VAR_MINENDORF5",
                "ITWR_VAR_AMBIENT_MINENDORF1", "ITWR_VAR_AMBIENT_MINENDORF2", "ITWR_VAR_AMBIENT_MINENDORF3", "ITWR_VAR_AMBIENT_MINENDORF4",
                // Herstell-Menues (Gespraeche des Helden mit sich selbst) und Ruestungs-Verbesserungen
                "FUR_H_START", "FUR_L_START", "FUR_M_START", "FUR_X_START", "WASSERPFEIFESTART", "PILZSTART", "ROBESSTART",
                "SUMPFSTART", "POISONSTART", "RUESTUNGEN",
                "IMPROVEMENTS_MOENCH1", "IMPROVEMENTS_MOENCH2", "IMPROVEMENTS_PRIESTER", "IMPROVEMENTS_WAMAH", "IMPROVEMENTS_WAMAL",
                "IMPROVEMENTS_WAMAM",
                // Wirkung von Getraenken, Traenken, Speisen und Gegenstaenden auf den eigenen Helden
                "HP_LOA_FRISCHEBRIESE", "HP_LOA_ORINBEER", "HP_LOA_STARKEBRIESE", "MANA_LOA_FRISCHEBRIESE", "MANA_LOA_ORINBEER",
                "MANA_LOA_STARKEBRIESE", "WINEBERRY_BONUS", "WASSERPFEIFE_DEX_COUNTER", "WASSERPFEIFE_STR_COUNTER", "SCHARFESGESCHNETZELTES",
                "AMBVAR_COINCHESTEFFECT", "AMBVAR_COINCHESTEFFECT_RANDOM", "AMBVAR_COINCHEST_BONUS", "TIMEVAR_COINCHESTEFFECT",
                "AMBVAR_GEISTTRANK_GETRUNKEN", "TIMEVAR_GEISTTRANK", "AMBVAR_PARALYSIS", "AMBVAR_REGENERATECOUNTER",
                "AMBVAR_TEMPSTEW", "TIMEVAR_TEMPSTEW", "AMBVAR_KOENIGSBEER_COUNTER", "AMBVAR_STARTED_VIOLIN", "TIMEVAR_VIOLIN_HERO",
                "TIMEVAR_POISON_PROTECTION", "TIMEVAR_SPIEGEL", "TIMEVAR_WEIRDDRINK", "TIMEVAR_TELEAMULETT",
                "AMBVAR_HERO_DROPPED_GOLD", "AMBVAR_OEDLAND_EFFECT_FIRST", "SYSVAR_OEDLAND_EFFECT", "SYSVAR_PICKUPWATER",
                // Hinweise, die der Spieler schon gesehen hat
                "AMBVAR_TIPPS_COLD", "AMBVAR_TIPPS_POISON", "AMBVAR_TIPPS_TELE"
            ]
        })json";
    }
}
