// Xeres' Rueckkehr (Clockwork Origins, 1.0.9) - Mod-Profil COOP_PROFILE_XERES (docs\MODULE.md, docs\mod-analyse\Xeres-durchsicht.md).
// Start mit -game:Xeres.ini (Mods\CoopModList.h). Allgemeines aus Xeres liegt im Fundament: P293 (Zeiger-Pruefungen "> 0" ueber 2 GB,
// CoopZeiger.cpp - dieselben 20 Stellen behebt das Skript-Paket Xeres-Fixes fuer Spieler ohne Coop) und P294 (Schleifen-Trigger: Xeres
// treibt DAUERFUNC_01 und FRAMEFUNC ueber oCTriggerScript-Vobs, die sich jede Sekunde selbst wieder ausloesen - jeder PC fuer sich,
// CoopDialogSync.cpp). Xeres bringt Ikarus, LeGo und Gothic Free Aim mit - P177 (LeGo-Haken vor GT) gilt. u249 (P300, CoopBlutspritzer.cpp):
// mit dem Union-Toolkit (LeGo 2.9) riefen B_AssessDamage_Hero und LeGos _B_HeroDamage sich gegenseitig auf - Absturz bei Fernkampftreffern
// mit Blutspritzern; auch das behebt das Skript-Paket Xeres-Fixes ab 1.1.0.

namespace GOTHIC_ENGINE {

    // u246: einmal je Start (CoopModsInit) - was GT ueber Xeres weiss, steht im Log
    void CoopModXeresInit() {
        OthelloCompatLog("Mod-Profil Xeres' Rueckkehr: Fundament %s - Dauerschleifen ueber Trigger je PC (P294), Zeiger-Pruefungen ueber 2 GB "
            "(P293), Gifte/Krankheit/Uebung/Goetterbonus/Verwandlungen/Perks des eigenen Helden, Ablauf-Zaehler der Zwischensequenzen und "
            "Lorenfahrt je PC, Welt-Init-Merker je Weltdatei, Blutspritzer-Schleife mit LeGo 2.9 (P300)", COOP_FASSUNG);
        // u248 (P299): Skript-Paket Xeres-Fixes (eigener Download, XERES_FIXES.D in Data\Xeres_Fixes.vdf) - geladen, wenn sein Symbol
        // da ist (zParserExtender hat es beim Laden der Skripte schon angehaengt); u252: ab 1.2.0 mit XRFIX_BLUTALT (Blutspritzer mit
        // echtem Schaden, P302)
        const bool paket = parser && parser->GetSymbol(zSTRING("XRFIX_ZEIGER"));
        const bool paket12 = parser && parser->GetSymbol(zSTRING("XRFIX_BLUTALT"));
        OthelloCompatLog("Mod-Profil Xeres' Rueckkehr: Skript-Paket Xeres-Fixes %s", !paket ?
            "nicht geladen - die Zeiger-Pruefungen und die Blutspritzer-Schleife behebt der Coop allein (P293, P300)" : paket12 ?
            "ab 1.2.0 geladen (XERES_FIXES.D) - die Zeiger-Pruefungen stellt der Coop beim Start um, das Paket findet danach nichts mehr; "
            "die Blutspritzer-Schleife behebt, wer zuerst kommt (gleicher Fix)" :
            "vor 1.2.0 geladen - entschaerft die Blutspritzer-Schleife, indem es den Blutspritzer der Mod abschaltet; der Coop setzt den "
            "Aufruf wieder ein (P302)");
    }

    // u246 (Mod-Analyse 10.11.2026 ueber 6 583 Funktionen; alle 374 Befunde "hoch" von Hand am Code beurteilt,
    // docs\mod-analyse\Xeres-durchsicht.md). Nicht uebernommen u. a.: Gesinnung (INNOS_/ADANOS_/BELIAR_GEFALLEN, MOD_GOTTSTATUS - Gespraeche
    // fragen sie ab, sie bleibt gemeinsam wie die Geschichte), ERZHACKCHANCE (den Startwert 10 setzt nur Xardas' erstes Gespraech - je Held
    // begaenne der Partner bei 0), Erz-/Salz-/Runenvorkommen (*MOB_*_AMOUNT gehoeren allen), Verbrechens-Zaehler, Merker von Ereignis-
    // Triggern in der Welt (EVT_*_ONETIME, OLDWOLRDTHOMAS - ein Trigger in der gemeinsamen Welt loest sonst beim Zweiten noch einmal aus),
    // Rueben auf Lobarts Feld (Welt), MOD_HQ_DAEMONISCH_SP (zaehlt Ereignisse, die beide PCs gleich sehen).
    const char* CoopModXeresProfile() {
        return R"json({
            "perPcVars": [
                // Anzeigen und Handles (LeGo-Views, Balken, Puzzle-Teile), Zeiger
                "BACKGROUNDVIEW", "EXPBAR", "FUTTERBAR", "ID_NPCPTR", "MOD_HEROLD_SIEGELRING_VERLAUF", "MOD_URIZIEL_BACKGROUND",
                "MOD_URIZIEL_FRAME", "MOD_URIZIEL_PIECE1", "MOD_URIZIEL_PIECE2", "MOD_URIZIEL_PIECE3", "MOD_URIZIEL_PIECE4",
                "MOD_URIZIEL_PIECE5", "MOD_URIZIEL_PIECE6", "MOD_URIZIEL_PIECE7", "MOD_URIZIEL_PIECE8", "MOD_URIZIEL_PUZZLEDESCRIPTION1",
                "MOD_URIZIEL_PUZZLEDESCRIPTION2", "PUZZLE_ACTIVEHNDL", "SEGEN_HANDLE", "TEMPFIGUR", "VIEW_GOMEZKILLS", "VIEW_GOMEZKILLSICH",
                "VIEW_GOMEZTIMER", "VIEW_TIME",
                // Brettspiel (laeuft nur beim Spieler, der es startet; Spine-Partie online) und seine Figuren
                "MG_*", "PLAYERHAS_*", "ONLINE_HOST", "MP_TIMEOUT",
                // Einstellungen aus der Gothic.ini dieses PCs ([XERES] ...), DAUERFUNC_01 liest sie jede Sekunde neu - abgeglichen stritten
                // beide PCs darum. Beide sollten dieselbe Schwierigkeit waehlen (Lehrkosten, Werte der NPCs beim Host)
                "MOD_AUTOINVENTAR", "MOD_AUTOSAVE", "MOD_BALKEN", "MOD_BLOODSPLATS", "MOD_ESSSYSTEM", "MOD_EXP_ANZEIGE", "MOD_MAUSSTEUERUNG",
                "MOD_RESPAWN", "MOD_SCHWIERIGKEIT", "TIMESWITCH", "MOD_ACTIONKEY", "MOD_PRISMAKEY",
                // Takt, Zufall und Zustand dieses PCs in DAUERFUNC_01/FRAMEFUNC (Xeres' eigene Auto-Sicherung: Zaehler und Sperre)
                "TIMECOUNTER_REAL", "TIMECOUNTER_FOG", "FRAMECOUNTER", "TIMEAN", "SOMEUNIMPORTANTRANDOMVALUE", "MOD_QUATSCHTNICHTCOUNTER",
                "INVENTORY_OPEN", "LOGSTATE_POSY", "MOD_KAMPFLAEUFT", "FIRSTF8MALUS", "HP_PER_LEVEL", "MOD_SAVEOTHER", "HEROATLIGHT",
                "AUTOSAVE_COUNTER", "AUTOSAVE_BLOCKER", "CURRENTLEVEL2",
                // Lorenfahrt: das Rad startet sie beim Benutzer, DAUERFUNC_01 versetzt dann den eigenen Helden Wegpunkt um Wegpunkt (LOREWP) -
                // abgeglichen fuhr der Held des Partners mit
                "LOREEISGEBIETMINE01", "LOREEISGEBIETSW01", "LOREKHORINIS", "LORERELENDEL", "MOD_LORECOUNTER", "RIDINGLORRY",
                // Uebernahmen aus frueheren Xeres-/Clockwork-Spielstaenden (Spine): jeder PC aus seinen eigenen
                "MOD_IMPORT_*",
                // Ablauf-Zaehler der Zwischensequenzen und Szenen: DAUERFUNC_01 zaehlt sie auf beiden PCs je Sekunde hoch, der Start-/Ende-
                // Merker (MOD_..._SCENE = 1/2) bleibt gemeinsam. Abgeglichen sprangen sie (Schritte doppelt oder uebersprungen, "== N" verpasst)
                "DEMONKNIGHT_TALK", "FI_REDE", "GARDECUTSCENE_COUNTER", "GIDAN_COUNTER", "MOD_AL_BLOODWYNBARTHOLOGEFANGEN_COUNTER",
                "MOD_ARGEZVISION_COUNTER", "MOD_ARGEZ_COUNTER", "MOD_BDT_ALTERWALDMANN_COUNTER", "MOD_BELIARBIBSCENE_COUNTER",
                "MOD_BEL_GREGOWEN_COUNTER", "MOD_BEL_JOSEPHINA_BIB_COUNTER", "MOD_CANTHARSCHLAEGER_COUNTER", "MOD_CHANIABENTEURERSCENE_COUNTER",
                "MOD_DUNKLERPILGER_COUNTER", "MOD_FORTUNOSUMPFMENSCH_SCENE_COUNTER", "MOD_FREIBIERSCENE_COUNTER", "MOD_GESPRAECHCOUNTER_BB",
                "MOD_HAGENKOSCENE_COUNTER", "MOD_HEROFLIEGTAL", "MOD_HOFSTAAT_ALEX_INTRO_COUNTER", "MOD_HOFSTAAT_TYP_INTRO_COUNTER",
                "MOD_HQ_DAEMONISCH_CITY_COUNTER", "MOD_HQ_DAEMONISCH_ORLAN_COUNTER", "MOD_HQ_DAEMONISCH_SEQUENZ",
                "MOD_HQ_DAEMONISCH_THEKLA_COUNTER", "MOD_IR_HEBELCOUNTER", "MOD_JUDITHRUPRECHTSCENE_COUNTER", "MOD_LAWRENCELEGTBRIEF",
                "MOD_LEICHENGASESCENE_COUNTER", "MOD_LOBARTALWINSCENE_COUNTER", "MOD_LUTEROLIESTBRIEF", "MOD_MITLAWRENCE_COUNTER",
                "MOD_NAGURGESCHAEFTE_SCENE_COUNTER", "MOD_NL_BRUCETOT_COUNTER", "MOD_OTTELEPORTSCENE_COUNTER",
                "MOD_PARCIVALVORKNEIPESCENE_COUNTER", "MOD_REL_THEODORUSS_COUNTER", "MOD_REL_WENDELS_COUNTER", "MOD_RITTERSCENE_COUNTER",
                "MOD_SKIPGREG_COUNTER", "MOD_SLYKARANTO_KAMPFSCENE_COUNTER", "MOD_SL_PARTFLUFFY_COUNTER", "MOD_SUMPFBEERENTRAUM_COUNTER",
                "MOD_TRAENENKRISTALL_SEQUENZ", "MOD_UNTIER_FRISST_COUNTER", "MOD_URS_HUND_COUNTER", "MOD_VERMISSTENCOUNTER",
                "MOD_VMG_PRUEFUNG_BARRIERE_COUNTER", "MOD_VMG_PRUEFUNG_VISION", "MOD_WM_FOKUSTELEPORT", "MOD_WM_KAMERABENGAR_COUNTER",
                "MOD_WM_KURGANALARM_COUNTER", "MOD_XERES_KAMPF01_COUNTER", "MOD_XW_KAP6_SCENE01_COUNTER", "MOD_XW_KAP6_SCENE02_COUNTER",
                "MOD_XW_KAP6_SCENE03_COUNTER", "MOD_XW_KAP6_SCENE04_COUNTER", "MOD_XW_KAP6_SCENE05_COUNTER", "MOD_XW_KAP6_SCENE06_COUNTER",
                "MOD_XW_KAP6_SCENE07_COUNTER", "MOD_XW_KAP6_SCENE08_COUNTER", "MOD_XW_STARTSCENE_COUNTER", "SPEEKHEXE_COUNTER",
                "TALKESSEL_COUNTER", "TOJAN_COUNTER"
            ],
            "heroVars": [
                // Gifte und Krankheiten des eigenen Helden (GIFTE und ZUSTAENDE in DAUERFUNC_01, B_HealGift, Treffer B_AssessDamage_Hero):
                // abgeglichen zog die Schleife des Partners ihm Leben fuer ein Gift ab, das er nie bekam
                "GGD_*", "GGPF_*", "GGPI_*", "GTB_*", "GTM_*", "GTSU_*", "GTS_*", "BISSDERFAEULNIS*", "HAUCHDERPESTILENZ*", "ERKAELTUNG*",
                "BLUTFLIEGENGIFT", "GEWAECHSGIFT", "GIFT", "GIFTDAMPF", "MINECRAWLERGIFT", "MOD_COUNTGIFTE", "PFLANZENGIFT", "PILZGIFT",
                "SUMPFGASDROHNENGIFT", "SUMPFHAIGIFT", "TIERGIFT", "KBDF", "KHDP", "KRANKHEIT", "KE", "KERKAELTUNG", "EISTAUCHER",
                "FLIEGENPILZGIFT_FIRSTTIME", "HATERSTESMALNIESEN", "VERGIFTETCOUNTER", "HEROISTVERGIFTET", "MOD_GIFTCOUNTER",
                "MOD_GIFTNEUTRALISIERUNG", "MOD_FLIEGENPILZGIFT*", "MOD_FUNGOGIFT*", "MOD_MITHRIDAGIFT*", "MOD_PIANTEGIFT*", "MOD_PONZOLAGIFT*",
                "MOD_VENENAGIFT*", "GIFTCOUNTER", "KRANKHEITCOUNTER", "MOD_KRISTALLPRISMA", "MOD_NACKT", "MOD_OVERLAYMDS_WAIT",
                // Essen, Trinken, Rausch, Schwimmen (Boni des eigenen Helden)
                "MOD_ITFO_*", "MOD_ITPL_*", "MOD_MOD_REALGROG_*", "DRINKCOUNTER", "DRINKCOOLDOWN", "HATGETRUNKEN", "NEUBETRUNKEN", "MOD_BETRUNKEN",
                "MOD_GETRUNKEN", "DRINKTIME", "DRINKTIMELAST", "BETRUNKENBEI", "BETRUNKEN_LEVEL", "HATNICHTSMEHRGETRUNKEN", "MOD_ESSKO",
                "MOD_ESSPUNKTE", "MOD_NICHTSGEGESSENCOUNTER", "MOD_AFTERHERKULESINTUS", "MOD_HASHERKULESINTUS", "MOD_PURPURMOND_INTUS",
                "MOD_PURPURMOND_INTUS_TIME", "MOD_REGENERATIONSTRANK_COUNTER", "MOD_ZUFALLSDRINK_ATT", "MOD_ZUFALLSDRINK_COUNTER",
                "MOD_ZUFALLSDRINK_INTUS", "MOD_WEISENCHRONIK_INTUS", "KRAUTTABAK_BONUS", "KRAUTTABAK_BONUSTIME", "HONEY_BONUS",
                "FLIEGENPILZ_GEGESSEN", "MOD_FISCH_COUNTER", "MOD_WALDBEERE_COUNTER", "MOD_WEIDENBEERE_COUNTER", "MOD_BUDDLERFLEISCH_COUNTER",
                "MOD_JOINTCOUNTER", "FIRST2JOINT", "MOD_SCHWIMMPRAXIS", "MOD_SCHWIMMTAUCHCOUNTER", "MOD_SWIMBONUSCOUNTER",
                // Uebung im Kampf (B_PraxisErfahrung aus ZS_Dead nur beim Toetenden - abgeglichen bekam der Partner die Uebung des anderen)
                "MOD_STAERKE_PRAXIS*", "MOD_GESCHICK_PRAXIS*", "MOD_MANA_PRAXIS*", "MOD_EINHAND_PRAXIS*", "MOD_ZWEIHAND_PRAXIS*",
                "MOD_BOGEN_PRAXIS*", "MOD_ARMBRUST_PRAXIS*",
                // Goetterbonus (GOETTERBONUS in DAUERFUNC_01): welcher Bonus dem eigenen Helden gegeben wurde - abgeglichen zog die Schleife
                // des Partners ihm nachts Mana, Leben und Schutz ab, die er nie bekam
                "MOD_GOTTADANOS", "MOD_GOTTBELIAR", "MOD_GOTTDAMAGE", "MOD_GOTTDEX", "MOD_GOTTHP", "MOD_GOTTINNOS", "MOD_GOTTMANA",
                "MOD_GOTTPROTECTION", "MOD_GOTTSTR", "MOD_HATADANOSBONUS", "MOD_HATBELIARBONUS", "MOD_HATINNOSBONUS", "MOD_GOTTCOOLDOWN",
                // Verwandlungen, Auren, Licht, Seelenwanderung (SEELE_UNTERWEGS: abgeglichen toetete NEWMAGIC den Partner, weil sein Held
                // "zu weit von der Seele" stand), Waffen-Upgrades
                "PLAYERISTRANSFORMED", "TRANSFORMCOOLDOWN", "MOD_TRFWOMAN", "MOD_TRFWOMAN_TIME", "MOD_TRFSKELETON2", "MOD_TRFSKELETON2_TIME",
                "MOD_ISFLEISCHWANZE", "MOD_ISRABBIT", "MOD_ISTGOLEM", "MOD_ISTKEILER", "MOD_ISTSCHAF", "MOD_WM_ISWARG", "MOD_WM_KONTROLLED",
                "PLAYERGUILD", "RESTOREPLAYERGUILD", "MOD_FOLGSAMKEITSNAPPER_INTUS", "MOD_FOLGSAMKEITSNAPPER_TIME", "MOD_AURA*",
                "MOD_LICHTAN_NORMAL", "MOD_LICHTAN_PAL", "SEELE_UNTERWEGS", "HP_REG", "MANA_REG", "DIVIDEDHPREG", "DIVIDEDMANAREG",
                "AXTUPGRADE", "AXTDAMAGE", "DOLCHKAMPFAN", "FOKUSBLOCKADE",
                // Perks: Lehrer setzen sie direkt im Gespraech (nicht ueber B_Teach*) - abgeglichen bekam der Partner sie geschenkt
                "DEFENSEBREAK_PERK", "DELIRIUM_PERK", "DIETRICH_PERK", "DUMMHEIT_PERK", "EWIGENACHT_PERK", "HP_PERK", "KNOCKOUT_PERK",
                "KONTER_PERK", "LEISENLAUFEN_PERK", "MANA_PERK", "MENSCHTOETER_PERK", "ORKTOETER_PERK", "PARASIT_PERK", "SCHAERFEN_PERK",
                "SCHATTENLAEUFER_PERK", "SCHNELLESLERNEN_PERK", "SLOWMOTION_PERK", "SPRINTEN_PERK", "STANDFEST_PERK",
                "UEBERSCHWAENGLICHKEIT_PERK", "UNSICHTBARKEIT_PERK", "WAFFENSCHMELZEN_PERK", "WERWOLF_PERK", "ZWEIHANDALSEINHAND_PERK",
                // Rezepte, Bauplaene, Lese-Boni, Herstell-Menues (Gespraeche des Helden mit sich selbst), Erfolge (Spine je PC)
                "REZEPT_*", "SMITH_ERZSCHWERT_FEUER", "SMITH_ERZSCHWERT_WASSER", "ALMANACH_*", "PFEILEMACHEN", "BOLZENMACHEN", "FLEISCHBRATEN",
                "VERBESSERN", "STARKWAFFEN", "MOD_PFEILCOUNTER", "CURRENTOPFER", "ERFOLG_*", "MOD_ERFOLG_*"
            ],
            "deltaVars": [
                // Zaehler aus Gespraechen, Mobsis und Gegenstaenden eines Spielers (tools\mod_muster.py Abschnitt C, "rein"); FOKIEINGESETZT:
                // setzten beide fast gleichzeitig einen Fokus ein, ging ein Schritt verloren und "== 5" (Ende der Fokussuche) kam nie
                "FOKIEINGESETZT", "MOD_ECHSENQUEST_01_FLEISCHABGELEGT", "MOD_AL_ZUFLUCHTCOUNTER", "MOD_BANDITEN_ORKJAGD",
                "MOD_ECHSI_SUMPFHAIZAEHNE", "MOD_FLUGBLAETTER", "MOD_FM_TRANKCOUNTER_HEALTH", "MOD_FM_TRANKCOUNTER_MANA", "MOD_FREUDENSPENDER",
                "MOD_IR_DABEI", "MOD_IR_HEBELPROB2", "MOD_NL_STEINKREIS_01", "MOD_NL_STEINKREIS_02", "MOD_NL_STEINKREIS_03", "MOD_PAT_BETER",
                "MOD_PDV_GENUG_INFOS", "MOD_PIRATENBEFREIUNG", "MOD_PIRATENVORBEREITUNGEN", "MOD_REL_QUESTCOUNTER", "MOD_SL_PARTBLIND_COUNTER",
                "MOD_STEINSCHALE1", "MOD_STEINSCHALE2", "SOULSTONEEINGESETZT", "TABAKRECEIVED"
            ],
            "heroStates": [
                // HERO meint hier den Spieler dort: der Engel des Zaubernden folgt ihm und zieht ihm Mana ab (sonst immer dem Host-Helden),
                // Seelen-Geister, Sabitschs Raub am niedergeschlagenen Helden, Angriffe in ZS_ATTACK (B_Attack(..., hero)), Xeres' erster
                // Auftritt (ZS_MM_ATTACK: Teleport zur Festung, Schutzamulett)
                "ZS_ENGEL", "ZS_SOUL", "ZS_SEELENRAUB", "ZS_RANSACKBODY", "ZS_ATTACK", "ZS_MM_ATTACK"
            ],
            "worldVars": {
                // Waechter, die allein die Welt-Init (INIT_<Welt>, STARTUP_<Welt>) setzt und liest - abgeglichen uebersprang, wer die Welt als
                // Zweiter mit eigener Datei betrat, den Block (wie P224). Nicht: MOD_ENTER_RELENDEL_03 (liest auch INIT_RELENDEL_MINE),
                // MOD_HOSHPAKINOC, MOD_KG_MINESPAWN, URNOLWEG (setzen zwei Welten)
                "DAEMONENRITTER_AL": "MINENTAL", "GEHTERSTESMALINSMINENTAL": "MINENTAL", "HOELLE_FIRST": "MINENTAL",
                "MOD_AL_ESTEBAN_BACK": "MINENTAL", "MOD_AMQ_DRM_SPAWNED": "MINENTAL", "MOD_BK_BOOKSPAWN": "MINENTAL", "MOD_BODOINMT": "MINENTAL",
                "MOD_ENTER_MINENTAL_01": "MINENTAL", "MOD_ENTER_MINENTAL_02": "MINENTAL", "MOD_ENTER_MINENTAL_03": "MINENTAL",
                "MOD_ENTER_MINENTAL_05": "MINENTAL", "MOD_ENTER_MINENTAL_06": "MINENTAL", "MOD_JG_PASSNOVIZE": "MINENTAL",
                "MOD_NAKOSH_MT": "MINENTAL", "MOD_NL_WASSERDRACHE": "MINENTAL", "MOD_PALAMINE_LEER": "MINENTAL", "MOD_RAVENGOMEZINNW": "MINENTAL",
                "MOD_REMOVED_SONOR": "MINENTAL", "MOD_SUMPFISWEG": "MINENTAL", "URNOL_INOT": "MINENTAL",
                "MOD_ADDEDSONORATLARES": "NEWWORLD", "MOD_AL_INSERTWHISTLER": "NEWWORLD", "MOD_ENTER_NEWWORLD_02": "NEWWORLD",
                "MOD_ENTER_NEWWORLD_03": "NEWWORLD", "MOD_ENTER_NEWWORLD_04": "NEWWORLD", "MOD_ENTER_NEWWORLD_05": "NEWWORLD",
                "MOD_ENTER_NEWWORLD_06": "NEWWORLD", "MOD_FANATIKERDA": "NEWWORLD", "MOD_MYXIRNACHTMAHR": "NEWWORLD",
                "MOD_NL_DRACHENJAEGERONAR": "NEWWORLD", "MOD_NOVI_DRIN": "NEWWORLD", "MOD_ORCDOG_INSERT": "NEWWORLD", "MOD_WYVER_INSERT": "NEWWORLD",
                "XARDAS3ISTWEG": "NEWWORLD",
                "INSERT_FAKE_HERO_ONETIME": "ADDONWORLD", "MOD_BRUNHILD_AW": "ADDONWORLD", "MOD_ENTER_ADDON_01": "ADDONWORLD",
                "MOD_ENTER_ADDON_02": "ADDONWORLD", "MOD_ENTER_ADDON_03": "ADDONWORLD", "MOD_ENTER_ADDON_04": "ADDONWORLD",
                "MOD_ENTER_ADDON_05": "ADDONWORLD", "MOD_ENTER_ADDON_06": "ADDONWORLD", "MOD_JOSEPHINA_IN": "ADDONWORLD",
                "SATURASNICHTINJHARKI": "ADDONWORLD",
                "MOD_ENTER_AM_01": "OLDMINE", "MOD_ENTER_ORKFRIEDHOF_01": "ORCGRAVEYARD", "MOD_ENTER_ORKTEMPEL_01": "ORCTEMPEL",
                "MOD_ENTER_ORKTEMPEL_04": "ORCTEMPEL", "MOD_ENTER_PAT_FIRST": "PATHERION", "MOD_PAT_FUEGODA": "PATHERION",
                "MOD_SCHWARZERVERRAETERINSERT_02": "PATHERION", "MOD_PAT_FURGASFERTIG": "FEUERMAGIER_UNTERGRUND",
                "MOD_SCHWARZERVERRAETERINSERT": "FEUERMAGIER_UNTERGRUND", "MOD_ENTER_SCHATZINSEL_01": "DIEINSEL",
                "MOD_JG_AUGUST": "RELENDEL", "MOD_KIMON_REL": "RELENDEL", "RELENDEL_SPAWNED": "RELENDEL", "MOD_JG_HOCKDA": "JAEGERLAGER",
                "MONSTER_GDG_SPAWNED": "GDG_WORLD", "EISGEBIETTHOMAS": "EISGEBIET", "ENDLEVELTHOMAS": "XERES_ENDLEVEL"
            },
            "vobSync": [
                // Kollision der Plattform bei Endres (Gespraech mit Juliana, Trollfett im Klo) - sonst nur beim Sprechenden umgeschaltet
                "ENDRESPLATFORM"
            ],
            "notes": [
                "Xeres: beide Spieler sollten dieselbe Schwierigkeit und dieselbe Auto-Sicherung im Menue waehlen (je PC aus der Gothic.ini)",
                "Xeres: Xeres' ersten Angriff im Minental (Niederlage, Teleport zur Festung) sollte der Host fuehren - Npc_IsPlayer gilt dort nur fuer ihn"
            ]
        })json";
    }

} // namespace GOTHIC_ENGINE
