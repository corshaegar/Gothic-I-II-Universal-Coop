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
                "ICEBARHANDLE", "DESERTBARHANDLE", "MAGUS_FIRE_POS", "ID_NPCPTR", "ID_UNPCPTR", "ITM_PTR", "FDLINT_*", "FDL_*",
                // u199 (P211, Durchsicht der Mod-Analyse): Modus-Merker fuer Dauerschleifen, die auf jedem PC laufen (START_SCHLEIFEN_GLOBAL:
                // EV_GIVE_ITEMS_FF, SHARP_FF, EV_DISMANTLE_RUNE_FF; SCHLEIFE_LOA/_ST/_FAST). Abgeglichen startete beim Partner der Modus mit:
                // Inventar auf, Bewegungssperre, die Aktionstaste verschenkte/verkaufte/schliff seinen Gegenstand; sein Zuruecksetzen ging
                // zurueck und brach den Modus des Spielers ab (beim Schleifen blieb dessen Sperre haengen)
                "GIVEITEM_*", "START_SHARP", "DISMANTLE_RUNE_*",
                // Zufallswurf je Abbau-Stunde (Hlp_Random, gleich danach abgefragt); eigener Held in der Heilzone des Magus (MAGUS_AREA)
                "MULTINUGGET", "HERO_ENTERED_HEAL_AREA",
                // u200 (P222, Wert-Einordnung der Analyse, am Code geprueft): Zustand dieses PCs, den der Abgleich zum Partner schickte -
                // Traum sperrt das Inventar (STARTUP_DREAM: beim Partner war es mit gesperrt), Gespraechs-Sperre aus einer Antwort
                // (NOTALK, B_ASSESSTALK), Sitzungs-Start (INIT_GLOBAL), offene Anzeigen (Tagebuch, Inventar, Buecher, Munition, Kill-Anzeige),
                // Zwischenwerte beim Brunnen, Taschendiebstahl, Schutzzauber, Gluecksspiel-Einsaetze und -Wuerfe, Zufall und Effekte des
                // Magus-Kampfs am eigenen Helden, Schuetzen-Abstimmung der NPC-Schleifen (laufen auf beiden PCs), Einstellungen aus der Ini,
                // Ikarus-Instrumentierung, Geplauder-Grenzen, Haendler-Zeiger, Anzeige-Zeilen
                "SYSVAR_INVENTORY_DISABLED", "SYSVAR_EXIT_DIALOG", "NEWSESSION", "CREATETALENTS", "NOTALK", "DIARY_OPENED", "INV_OPENED",
                "ITEMCOUNT", "OVERLAYONHERO", "PUMP_COUNT", "SETACTIVE", "TARGETFLAG", "UNEQUIPPEDAMMO", "DEFAULTAMMO", "OLD_MARK_INSTANCEID",
                "SPL_COST_SCHUTZZAUBER", "THEFTDEXGLOB", "THEFTGOLDGLOB", "THIEFLUCK",
                "AMBVAR_21_BET", "AMBVAR_21_PASQUALERESULT", "AMBVAR_21_RESULT", "AMBVAR_LUSTIGESIEBEN_BET", "AMBVAR_TWOUP_BET", "AMBVAR_TWOUP_LASTBET",
                "MAGUS_AREA_CHOSEN", "MAGUS_AREA_HEAL_TIME", "MAGUS_FEAR_COUNTER", "MAGUS_FEAR_TIME", "MAGUS_FIRE_COUNTER", "RANDOM_MAGUS_TELEPORT",
                "AMBVAR_SHOOT_START", "AMBVAR_SHOOT_TOGETHER", "AMBVAR_SHOOT_WAIT", "MAX_TELE", "LOGLEVEL", "LOGTOFILEDISABLED",
                "MULTIPAGEBOOK_*", "INSTRINT_*", "VAR_SMALLTALK_DISALLOWED_*", "TRD_LOA_*", "AMBIENTNPC_ID", "RANDOM_AMBIENT",
                "VARIABELBEERDIALOGE", "FISH_RANDOM", "NEXTLINE", "NEXTPRINTLINE", "TIMEVAR_LOGENTRY"
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
                "AMBVAR_TIPPS_COLD", "AMBVAR_TIPPS_POISON", "AMBVAR_TIPPS_TELE",
                // u199 (P211, Durchsicht der Mod-Analyse): Werte des eigenen Helden, die der Abgleich bisher zum Partner schickte -
                // Schrein-Segen (beim Ablauf zog SCHLEIFE_ALL die Punkte auf beiden PCs vom eigenen Helden ab: der Partner verlor Werte, die
                // er nie bekam), Fluch-Punkte von Ines (der Tod des Fluch-NPCs gab sie beiden Helden zurueck; der Merker MISVAR_INES_FLUCH
                // selbst bleibt gemeinsam, sonst erreichte das Aufheben durch den Partner den Verfluchten nicht), Charakterwerte aus
                // Gespraechen, Erzabbau, gesammelte Teleportpunkte (TELEARRAY ist schon Heldenwert), Karotten-Zaehler (wie APPLE_BONUS),
                // laufende Runen-Wirkung (36 Ticks Leben/Mana), Abklingzeit der Teleport-Zauber, Laehmungs-Tag, Spiegelwerte fuer Nathanael
                "AMBVAR_SHRINEBUFF*", "TIMEVAR_SHRINEBUFF", "MISVAR_INES_FLUCH_*", "ATTVAR_*", "HERO_ERZHACKCHANCE", "TELE_COLLECTED",
                "LOA_KAROTTEN_BONUS", "HPRUNE", "MANARUNE", "TIMEVAR_TELEKARL", "DAY_RITUAL_MINUS", "VAR_NATHANAEL_*",
                // Rezepte aus Schriftstuecken (USE_* setzt, PC_ITFO_*_CONDITION fragt ab) und Erzabbau-Stunden (wie GOLDCOUNTER, P214)
                "APFELKUERBIS", "APFELSCHNAPPS", "EINTOPFHONIG", "KUERBISSUPPE", "HUSTENSAFTBRAUEN", "ERZCOUNTER",
                // aus der Wert-Einordnung der Analyse, am Code geprueft: einmalige Lesepult-Boni/Gelerntes (wie Gothics *_PERMANENT, StoryHeroVarsExtra),
                // Gift-Schutz und Trank-Bonus des eigenen Helden, gesehener Hitze-Tipp, gezaehlte Kaefer-Eier, gelesene Orkschrift. Nicht:
                // BEKANNT_* (Dialoge fragen sie ab), Briefe (SCKNOWS*), Lehr-Gespraeche (TALENT_SWEET_BOOZE: gilt fuer beide als erzaehlt)
                "ARROWS_3_PERMANENT", "CHRONIC_PERMANENT", "CRIMERATE_PERMANENT", "DRESSMAKER_PERMANENT", "HISTORY_3_PERMANENT",
                "LEGENDEN1_PERMANENT", "LEGENDEN2_PERMANENT", "LEGENDEN3_PERMANENT", "MINES_PERMANENT", "OEDLAND_PERMANENT", "POISON_PERMANENT",
                "RELIGION_PERMANENT", "SHARP_PERMANENT", "AMBVAR_POISON_PROTECTED", "MISVAR_WEIRDDRINK_BONUS", "AMBVAR_TIPPS_HEAT", "AMBVAR_BUGEGGS",
                "MISVAR_ORCRITE_READ",
                // u200 (P222): Kaelte und Durst des eigenen Helden (Getraenke, Speisen; LOA_UPDATEICEBAR/_DESERTBAR zaehlen je PC nach der
                // Lage des eigenen Helden herunter - abgeglichen sprangen die Leisten zwischen beiden Helden), Wasser-Wissen und Koch-Talent,
                // Infektion des Helden (SCHLEIFE_LOA_POISONHANDLER), einmalige EP beim Lesen (wie ASTRONOMY_ONCE), Menuestand beim Beten,
                // Farbenmischen und Wasserpfeifen-Tabak (wie BOOZESTART), gezaehlte Teleportpunkte, Verwandlungs-Abklingzeit
                "AMBVAR_ICE_COUNTER", "AMBVAR_DESERT_COUNTER", "AMBVAR_KNOWS_WATERSKILLS", "PLAYER_TALENT_KOCHEN",
                "VAR_HERO_IS_INFECTED_BY_CANINE_MADNESS", "FINALDRAGONEQUIPMENT_ONCE", "FARBE_MISCHEN", "AMBVAR_PAY", "APFELSTART",
                "DOPPELAPFELSTART", "HONIGSTART", "CNT_TELE", "TIMEVAR_DRAGONTRF"
            ],
            "deltaVars": [
                // u199 (P213): nur von Mob-Skripten hochgezaehlt (+= 1) - loeschten/verbrannten beide gleichzeitig, rechneten beide vom selben
                // Stand und der Abgleich setzte den Wert: ein Schritt ging verloren, "noch 1 Brand"/Knochen == 7 kam nie (Quest haengt)
                "MISVAR_HF_BRAND_GELOESCHT", "MISVAR_ORCGHOST_BONES_BURNED",
                // u200 (P222): gute Taten (nur Gespraeche zaehlen +1, die Pruefung im Kloster fragt >= 3) und abgebautes Erz je Ader
                // (B_ERZMOB_BESTIMMUNG +1 aus dem Abbau-Gespraech). Nicht GOLDMOB_*: deren Erholung ueber Tage rechnete doppelt
                "MISVAR_GOOD_DEEDS", "ERZMOB_*",
                // u200 (P222, tools\mod_muster.py Abschnitt C, Spalte "rein"): nur += in Gespraechen, keine andere Zuweisung - jedes Info
                // gilt fuer beide als erzaehlt, zaehlt also nie doppelt; fuehrten beide zugleich verschiedene Gespraeche, fehlte ein Schritt
                "AMBVAR_FLINT_FOOD", "AMBVAR_FURAX_RUHE", "HANDWERKERRESPECT", "HEROISREADYTOBENOVICE", "KAEMPFERSTIMMEN",
                "LEWAMISKKAPONECOUNTREKRUT", "MISVAR_ARENA_KAEMPFER", "MISVAR_HAMO_DRUNKEN", "MISVAR_HELPED_TRIV", "MISVAR_KARLWORKER",
                "MISVAR_K_CRAZYMONK", "MISVAR_MERLOT_REZEPT_FIRST_TIME", "MISVAR_NEST_RECRUITS_COUNTER", "MISVAR_RACHEL_RUESTUNGEN",
                "MISVAR_REKRUTIERUNG", "MISVAR_RODRIGO_BEUTE", "MISVAR_SCHNAEPSE", "MISVAR_SURRATOR_COUNTER", "MISVAR_TRIUMVIRAT_BAD_DEEDS",
                "MISVAR_VERBANNTER_INFO", "MISVAR_WON_FIGHTS", "REKRUTENRESPECT", "VAR_CARL_BELEIDIGT", "VERTEILTESCHREIBEN"
            ],
            "heroStates": [
                // u199 (P221): HERO meint hier den Spieler dort - Beschwoerung bei SUMPF_155 (Wld_SpawnNpcRange(HERO, ZOMBIE01_NORESPAWN)),
                // Hinrichtung (AI_GotoNpc(self, hero)), Faustkampf-Arena, Kastor/Parcival und Magus-Illusionen (ZS_ATTACK_LOOP: hero als
                // Gegner/Ziel). Sonst wirkten sie immer um den Host-Helden, auch wenn nur der Client dort stand bzw. kaempfte
                "ZS_CIRCLE_LOA", "ZS_KILL_HERO", "ZS_ATTACK",
                // u200 (P223, alle 41 Zustaende mit HERO im Skript durchgesehen): Fuehrer, Begleit-Tiere und Irrlicht (AI_GotoNpc(self, hero);
                // Gruppenmitglieder nimmt GT aus - sie folgen ihrem Spieler, P217), Wachen und Wartende, die auf den nahen Spieler reagieren,
                // Geist mit Schadenszauber auf hero, Magus-Illusionen, Adrion, Hinterhalte (ZS_ATTACK_HERO: B_Attack(self, hero)), Zauber-
                // Folgen (Schrumpfen), Schlafende, die den Helden aus dem Bett jagen, Lauten-Spieler (Npc_HasItems(hero, ...)). Nicht:
                // ZS_FOLLOW_PLAYER* (Begleiter, P217), ZS_GUARD_PASSAGE/ZS_STAND_GUARDING* (Gothic-Wachen), Magie-Nachwirkungen und
                // ZS_UNCONSCIOUS (Haltung/Info - gleich fuer beide)
                "ZS_GUIDE_PLAYER", "ZS_MM_RTN_FOLLOW_ELITEGOBBO", "ZS_MM_RTN_FOLLOW_SHEEP", "ZS_MM_RTN_SCHUTZIRRLICHT",
                "ZS_STAND_ARMSCROSSED_ORDER", "ZS_STAND_WAITING_PLAYER", "ZS_USE_TEMPELSAUELE", "ZS_GHOST_MAGIC", "ZS_MM_ATTACK",
                "ZS_ADRIONFIGHT", "ZS_ATTACK_HERO", "ZS_BIGFIGHT", "ZS_MAGICSHRUNK", "ZS_MM_RTN_WATCHING", "ZS_PLAY_LUTE", "ZS_REACTTOWEAPON",
                "ZS_READIED_CROSSBOW", "ZS_SLEEP", "ZS_SLEEP_DEEP", "ZS_SLEEP_HURT", "ZS_STAND_CHEER", "ZS_STAND_WP", "ZS_STUDY_WP",
                "ZS_SUCKENERGY"
            ],
            "worldVars": {
                // u200 (P224, Liste Nr. 12): Waechter, die allein die Welt-Init setzt - abgeglichen uebersprang, wer die Welt als Zweiter mit
                // eigener Datei betrat, den Block: das Kapitel-4-Massaker in der Goldmine (B_KillNpc ..., Todesfalle, Blitzrune) bzw. das
                // Inventar des Untoten Drachen. Nicht MISVAR_WUESTE_FIRST (liest auch INIT_SUB_WORLD) und MISVAR_GM_INSERT_PARTY,
                // MISVAR_TRIV_TOT (setzen auch Gespraeche)
                "MISVAR_K4_GM_KILL_ONCE": "GOLDMINE", "MISVAR_UNDEADDRAGON_INV_GIVEN": "TOA"
            }
        })json";
    }
}
