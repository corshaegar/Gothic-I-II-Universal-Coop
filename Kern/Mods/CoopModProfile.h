// Mod-Profil als Datei (u194, P196; docs\MODULE.md Abschnitt 6). Wird in CoopCore.cpp im Namensraum der Engine eingebunden, nach
// Mods\CoopModList.h und vor CoopProfile() - ohne #pragma once, weil Mehr-Engine-Builds Sources.h je Engine neu einbinden.
//
// Was eine Mod nur an Daten braucht, steht in einer Datei im Ordner system\ - ohne neuen Bau der DLL:
//   system\GothicCoopMod-<Ini>.json   Mod mit eigener Spiel-Ini (-game:<Ini>.ini), z. B. GothicCoopMod-LoA.json
//   system\GothicCoopMod.json         Spiel ohne -game (etwa eine Mod mit eigenem Gothic-Ordner)
// Bekannte Mods (Mods\CoopModList.h) bringen ihr Profil im selben Format eingebaut mit; die Datei ergaenzt es. Eintraege:
//   "name"            Anzeigename (Log, Abgleich mit dem Partner) - sonst der Titel aus [INFO] der Ini
//   "engine"          "G1", "G1A", "G2" oder "G2A" - passt sie nicht, gilt die Datei nicht
//   "perPcVars"       Skript-Ganzzahlen, die je PC bleiben (nie im Story-Abgleich, P190); "NAME_*" = alle mit dieser Vorsilbe.
//                     Beim Laden und Uebernehmen gilt der Wert des geladenen Stands (Zustand dieses PCs: Anzeigen, Handles, Zeiger)
//   "heroVars"        u197 (P207): Werte des eigenen Helden (Talente, Rezepte, Uebung, Erfolge, Boni) - nie im Story-Abgleich und
//                     wie Gothics eigene Heldenwerte mit dem Helden gesichert (COOPHERO.TXT): beim Uebernehmen des Host-Stands
//                     (bei jedem Verbinden) behaelt der Client seine eigenen. Schreibweise wie "perPcVars"
//   "deltaVars"       u199 (P213): Zaehler, die nur Spieler-Skripte hochzaehlen (+= in Mob-, Gegenstands-, Dialogskripten) - der
//                     Abgleich schickt die Aenderung statt des Werts, damit gleichzeitige Schritte beider Spieler nicht verloren gehen.
//                     Nie fuer Zaehler aus Schleifen, die auf beiden PCs laufen (dort zaehlte jeder Schritt doppelt). Schreibweise wie oben
//   "startVars"       u207 (P241): Ablauf-Zaehler, die ein Skript-Takt auf jedem PC weiterschaltet (Zwischensequenzen mit dem eigenen
//                     Helden) - nur der Start (0 -> Wert) geht an den Partner, danach zaehlt jeder PC selbst; beim Empfaenger beginnt
//                     der Ablauf immer bei 1 (u220, P258 - der Absender kann zwischen zwei Abgleichen schon weiter sein). Nicht in der Pruefrunde
//                     "Alle Skript-Variablen gleich". Schreibweise wie oben
//   "heroStates"     u199 (P221): KI-Zustaende von NPCs (ZS_...), deren Skripte HERO als "den Spieler hier" meinen (Beschwoerung um den
//                     Helden, Hinrichtung, Arena-Regeln): waehrend des Zustands zeigt HERO auf den Spieler, gegen den der NPC kaempft,
//                     sonst auf den naechsten Spieler. Schreibweise wie oben ("ZS_ARENA_*")
//   "worldVars"       u200 (P224): reine Waechter aus Welt-Inits {"NAME": "WELT"} (WELT = Dateiname ohne Pfad und Endung, z. B. "GOLDMINE"):
//                     wie Gothics Eintritts-Merker (P139) nie im Abgleich, sie gehoeren zur Weltdatei - wer die Welt mit der eigenen Datei
//                     betritt, fuehrt ihren Init-Block selbst aus. Nur fuer Werte, die allein die Welt-Init setzt
//   "vobSync"         u201 (P226): Namen von Welt-Objekten, deren Sichtbarkeit, Kollision, Durchsichtigkeit und Aussehen Skripte im eigenen
//                     Prozess aendern (Ikarus: zCVob-Bits, SetVisual) - der Coop gleicht diese Felder zwischen PCs in derselben Welt ab.
//                     Nur ganze Namen (kein *)
//   "configDefaults"  Vorgaben fuer Schalter aus GothicCoopConfig.json ("coop..."), solange die Datei des Spielers sie nicht setzt
//   "notes"           Hinweise, die beim Start ins Log gehen (bekannte Grenzen der Mod)
// Kommentare (// und /* */) sind erlaubt, Eintraege mit "_" am Anfang werden uebergangen. Hier wird nur gelesen, kein Log
// geschrieben: CoopProfile() laeuft schon bei der ersten Logzeile. Die Meldungen schreibt CoopModsInit (Mods\CoopMods.cpp).

struct CoopModData {
    std::string source;                    // woher: "eingebaut", "system\GothicCoopMod-LoA.json", "eingebaut + ..." (leer = nichts)
    std::string fileShown;                 // gesuchte Datei (system\...), auch wenn sie fehlt - fuer den Hinweis im Log
    bool fileFound = false;
    bool fileUsed = false;
    std::string fileError;                 // warum eine gefundene Datei nicht gilt
    std::string name;                      // "name"
    std::vector<std::string> perPcExact;   // "perPcVars" ohne *, gross, sortiert
    std::vector<std::string> perPcPrefix;  // "perPcVars" mit * am Ende (nur die Vorsilbe), gross, sortiert
    std::vector<std::string> heroExact;    // u197 (P207): "heroVars" ohne *, gross, sortiert
    std::vector<std::string> heroPrefix;   // u197 (P207): "heroVars" mit * am Ende (nur die Vorsilbe)
    std::vector<std::string> deltaExact;   // u199 (P213): "deltaVars" ohne *
    std::vector<std::string> deltaPrefix;  // u199 (P213): "deltaVars" mit * am Ende (nur die Vorsilbe)
    std::vector<std::string> startExact;   // u207 (P241): "startVars" ohne *
    std::vector<std::string> startPrefix;  // u207 (P241): "startVars" mit * am Ende (nur die Vorsilbe)
    std::vector<std::string> zsExact;      // u199 (P221): "heroStates" ohne *
    std::vector<std::string> zsPrefix;     // u199 (P221): "heroStates" mit * am Ende (nur die Vorsilbe)
    std::map<std::string, std::string> worldVars;   // u200 (P224): "worldVars" Name -> Welt (beides gross)
    std::vector<std::string> vobSync;               // u201 (P226): "vobSync" Objektnamen (gross, sortiert)
    json configDefaults = json::object();  // "configDefaults" (nur "coop..."-Schalter)
    std::vector<std::string> notes;        // "notes"
    std::vector<std::string> warnings;     // uebergangene Eintraege (Log)
    std::vector<std::string> applied;      // was CoopModApplyConfigDefaults gesetzt hat (Log)
    unsigned stamp = 0;                    // Pruefsumme der Daten, 0 = keine Daten (Abgleich mit dem Partner)
};

static CoopModData& CoopModDataRef() {
    static CoopModData d;
    return d;
}

static const char* CoopModEngineName() {
    return COOP_ENGINE_SWITCH("G1", "G1A", "G2", "G2A");
}

// Schalter, die das COOP-Menue beim ersten Start aus GothicCoopConfig.json in Gothic.ini [GOTHICCOOP] uebernimmt (CoopDifficulty.cpp,
// CoopDiffDefaults) - aus einem Mod-Profil blieben sie dort auch fuer andere Mods derselben Installation stehen
static bool CoopModMenuKey(const std::string& k) {
    return k == "coopStorySync" || k == "coopAutoSaveMinutes" || k == "coopTakeHostStateOnConnect";
}

static std::string CoopModUpperTrim(const std::string& in) {
    const size_t a = in.find_first_not_of(" \t\r\n"), b = in.find_last_not_of(" \t\r\n");
    std::string s = a == std::string::npos ? std::string() : in.substr(a, b - a + 1);
    for (char& c : s)
        c = (char)toupper((unsigned char)c);
    return s;
}

// Ein Profil im Dateiformat lesen und in d ergaenzen. false (mit err) bei kaputtem JSON, keinem Objekt oder fremder Engine - dann
// bleibt d unberuehrt. Einzelne ungueltige Eintraege werden uebergangen und in d.warnings gemeldet.
static bool CoopModParse(const std::string& text, const std::string& what, CoopModData& d, std::string& err) {
    json j;
    try {
        j = json::parse(text, nullptr, true, true);   // Kommentare erlaubt
    }
    catch (const std::exception& e) {
        err = e.what();
        return false;
    }
    if (!j.is_object()) {
        err = "kein JSON-Objekt { ... }";
        return false;
    }
    if (j.contains("engine")) {
        if (!j["engine"].is_string()) {
            err = "\"engine\" ist kein Text";
            return false;
        }
        const std::string e = j["engine"].get<std::string>();
        if (_stricmp(e.c_str(), CoopModEngineName())) {
            err = "gilt fuer die Engine " + e + ", hier laeuft " + CoopModEngineName();
            return false;
        }
    }
    for (auto it = j.begin(); it != j.end(); ++it) {
        const std::string k = it.key();
        const json& v = it.value();
        if (k.empty() || k[0] == '_' || k == "$schema" || k == "engine")
            continue;
        if (k == "name") {
            if (v.is_string() && !v.get<std::string>().empty())
                d.name = v.get<std::string>().substr(0, 80);
            else
                d.warnings.push_back(what + ": \"name\" ist kein Text - uebergangen");
        }
        else if (k == "perPcVars" || k == "heroVars" || k == "deltaVars" || k == "heroStates" || k == "startVars") {   // u197 (P207), u199 (P213, P221), u207 (P241)
            if (!v.is_array()) {
                d.warnings.push_back(what + ": \"" + k + "\" ist keine Liste [ ... ] - uebergangen");
                continue;
            }
            std::vector<std::string>& exact = k == "heroVars" ? d.heroExact : k == "deltaVars" ? d.deltaExact : k == "heroStates" ? d.zsExact :
                k == "startVars" ? d.startExact : d.perPcExact;
            std::vector<std::string>& prefix = k == "heroVars" ? d.heroPrefix : k == "deltaVars" ? d.deltaPrefix : k == "heroStates" ? d.zsPrefix :
                k == "startVars" ? d.startPrefix : d.perPcPrefix;
            for (const json& x : v) {
                if (!x.is_string()) {
                    d.warnings.push_back(what + ": Eintrag in \"" + k + "\" ist kein Text - uebergangen");
                    continue;
                }
                std::string s = CoopModUpperTrim(x.get<std::string>());
                const size_t star = s.find('*');
                if (s.empty() || (star != std::string::npos && (star + 1 != s.size() || star < 2))) {
                    d.warnings.push_back(what + ": \"" + x.get<std::string>() + "\" in \"" + k + "\" ungueltig (ein * nur am Ende, "
                        "davor mindestens zwei Zeichen) - uebergangen");
                    continue;
                }
                if (star == std::string::npos)
                    exact.push_back(s);
                else
                    prefix.push_back(s.substr(0, star));
            }
        }
        else if (k == "vobSync") {   // u201 (P226)
            if (!v.is_array()) {
                d.warnings.push_back(what + ": \"vobSync\" ist keine Liste [ ... ] - uebergangen");
                continue;
            }
            for (const json& x : v) {
                const std::string s = x.is_string() ? CoopModUpperTrim(x.get<std::string>()) : std::string();
                if (s.empty() || s.find('*') != std::string::npos) {
                    d.warnings.push_back(what + ": Eintrag in \"vobSync\" ungueltig (ganzer Objektname als Text) - uebergangen");
                    continue;
                }
                d.vobSync.push_back(s);
            }
        }
        else if (k == "worldVars") {   // u200 (P224)
            if (!v.is_object()) {
                d.warnings.push_back(what + ": \"worldVars\" ist kein Objekt { \"NAME\": \"WELT\" } - uebergangen");
                continue;
            }
            for (auto c = v.begin(); c != v.end(); ++c) {
                const std::string n = CoopModUpperTrim(c.key());
                if (n.empty() || n.find('*') != std::string::npos || !c.value().is_string() || CoopModUpperTrim(c.value().get<std::string>()).empty()) {
                    d.warnings.push_back(what + ": \"" + c.key() + "\" in \"worldVars\" ungueltig (Name ohne *, Welt als Text) - uebergangen");
                    continue;
                }
                d.worldVars[n] = CoopModUpperTrim(c.value().get<std::string>());
            }
        }
        else if (k == "configDefaults") {
            if (!v.is_object()) {
                d.warnings.push_back(what + ": \"configDefaults\" ist kein Objekt { ... } - uebergangen");
                continue;
            }
            for (auto c = v.begin(); c != v.end(); ++c) {
                const std::string ck = c.key();
                if (ck.compare(0, 4, "coop") != 0)
                    d.warnings.push_back(what + ": Vorgabe \"" + ck + "\" - nur Coop-Schalter (\"coop...\") - uebergangen");
                else if (CoopModMenuKey(ck))
                    d.warnings.push_back(what + ": Vorgabe \"" + ck + "\" gilt ueber das COOP-Menue - uebergangen");
                else if (c.value().is_null() || c.value().is_object())
                    d.warnings.push_back(what + ": Vorgabe \"" + ck + "\" ohne gueltigen Wert - uebergangen");
                else
                    d.configDefaults[ck] = c.value();
            }
        }
        else if (k == "notes") {
            if (!v.is_array()) {
                d.warnings.push_back(what + ": \"notes\" ist keine Liste [ ... ] - uebergangen");
                continue;
            }
            for (const json& x : v)
                if (x.is_string() && !x.get<std::string>().empty() && d.notes.size() < 10)
                    d.notes.push_back(x.get<std::string>().substr(0, 300));
        }
        else
            d.warnings.push_back(what + ": unbekannter Eintrag \"" + k + "\" (fuer eine neuere Fassung?) - uebergangen");
    }
    return true;
}

static unsigned CoopModFnv(const std::string& s) {
    unsigned h = 2166136261u;
    for (unsigned char c : s) {
        h ^= c;
        h *= 16777619u;
    }
    return h;
}

// Listen ordnen, doppelte weg, Pruefsumme ueber alle Daten (gleiche Daten in anderer Reihenfolge = gleiche Pruefsumme, ausser "notes")
static void CoopModFinish(CoopModData& d) {
    for (std::vector<std::string>* v : { &d.perPcExact, &d.perPcPrefix, &d.heroExact, &d.heroPrefix, &d.deltaExact, &d.deltaPrefix,
        &d.startExact, &d.startPrefix, &d.zsExact, &d.zsPrefix, &d.vobSync }) {
        std::sort(v->begin(), v->end());
        v->erase(std::unique(v->begin(), v->end()), v->end());
    }
    if (d.perPcExact.empty() && d.perPcPrefix.empty() && d.heroExact.empty() && d.heroPrefix.empty() && d.deltaExact.empty() &&
        d.deltaPrefix.empty() && d.startExact.empty() && d.startPrefix.empty() && d.zsExact.empty() && d.zsPrefix.empty() && d.worldVars.empty() &&
        d.vobSync.empty() && d.configDefaults.empty() && d.notes.empty()) {
        d.stamp = 0;
        return;
    }
    std::string all;
    for (const std::string& s : d.perPcExact)
        all += s + ",";
    all += "|";
    for (const std::string& s : d.perPcPrefix)
        all += s + "*,";
    all += "|" + d.configDefaults.dump() + "|";
    for (const std::string& s : d.notes)
        all += s + "\n";
    if (!d.heroExact.empty() || !d.heroPrefix.empty()) {   // u197 (P207): nur mit Heldenwerten - Kennungen aelterer Profile bleiben gleich
        all += "|hero:";
        for (const std::string& s : d.heroExact)
            all += s + ",";
        for (const std::string& s : d.heroPrefix)
            all += s + "*,";
    }
    if (!d.deltaExact.empty() || !d.deltaPrefix.empty()) {   // u199 (P213): nur mit Zaehlern - aeltere Kennungen bleiben gleich
        all += "|delta:";
        for (const std::string& s : d.deltaExact)
            all += s + ",";
        for (const std::string& s : d.deltaPrefix)
            all += s + "*,";
    }
    if (!d.startExact.empty() || !d.startPrefix.empty()) {   // u207 (P241): nur mit Ablauf-Zaehlern - aeltere Kennungen bleiben gleich
        all += "|start:";
        for (const std::string& s : d.startExact)
            all += s + ",";
        for (const std::string& s : d.startPrefix)
            all += s + "*,";
    }
    if (!d.zsExact.empty() || !d.zsPrefix.empty()) {   // u199 (P221): nur mit Zustaenden - aeltere Kennungen bleiben gleich
        all += "|zs:";
        for (const std::string& s : d.zsExact)
            all += s + ",";
        for (const std::string& s : d.zsPrefix)
            all += s + "*,";
    }
    if (!d.worldVars.empty()) {   // u200 (P224): nur mit Welt-Waechtern - aeltere Kennungen bleiben gleich
        all += "|world:";
        for (auto& w : d.worldVars)
            all += w.first + "=" + w.second + ",";
    }
    if (!d.vobSync.empty()) {   // u201 (P226)
        all += "|vob:";
        for (const std::string& s : d.vobSync)
            all += s + ",";
    }
    d.stamp = CoopModFnv(all);
    if (!d.stamp)
        d.stamp = 1;
}

static bool CoopModReadFile(const std::string& path, std::string& text, std::string& err) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) {
        err = "nicht lesbar (gesperrt oder ohne Leserecht?)";
        return false;
    }
    char buf[4096];
    size_t k;
    text.clear();
    while ((k = fread(buf, 1, sizeof(buf), f)) > 0 && text.size() <= (256u << 10))
        text.append(buf, k);
    fclose(f);
    if (text.size() > (256u << 10)) {
        err = "groesser als 256 KB";
        return false;
    }
    return true;
}

// Aus CoopProfile(): eingebautes Profil der bekannten Mod, dann die Datei. iniBase = Ini-Name ohne Endung (leer ohne -game).
static void CoopModDataLoad(CoopProfileInfo& p, const CoopModKnown* known, const std::string& iniBase) {
    CoopModData& d = CoopModDataRef();
    if (known && known->builtin) {
        std::string err;
        if (CoopModParse(known->builtin(), "eingebautes Profil", d, err))
            d.source = "eingebaut";
        else
            d.warnings.push_back("eingebautes Profil nicht lesbar (" + err + ") - Baufehler, bitte melden");
    }
    d.fileShown = iniBase.empty() ? std::string("system\\GothicCoopMod.json") : "system\\GothicCoopMod-" + iniBase + ".json";
    const std::string path = CoopGameRoot() + "\\" + d.fileShown;
    if (CoopProfileFileExists(path)) {
        d.fileFound = true;
        std::string text, err;
        if (!CoopModReadFile(path, text, err))
            d.fileError = err;
        else if (CoopModParse(text, d.fileShown, d, err)) {
            d.fileUsed = true;
            d.source += (d.source.empty() ? "" : " + ") + d.fileShown;
        }
        else
            d.fileError = err;
    }
    if (!d.name.empty())
        p.name = d.name;
    if (d.fileUsed) {
        if (p.id == COOP_PROFILE_VANILLA) {   // Spiel ohne -game mit Profil-Datei: eine Mod im eigenen Ordner
            p.id = COOP_PROFILE_MOD;
            p.why = "Profil-Datei " + d.fileShown;
        }
        else if (p.id == COOP_PROFILE_MOD)
            p.why = "-game:" + p.modIni + " (Profil aus " + d.fileShown + ")";
        else
            p.why += ", dazu " + d.fileShown;
    }
    CoopModFinish(d);
}
