// Mod-Profile - Verteiler (docs\MODULE.md). Das Fundament ruft nur diese Einstiege. Was ein Profil an Code braucht, steht in
// Mods\Mod<Kuerzel>.cpp; was es an Daten braucht, im Format der Profil-Datei (Mods\CoopModProfile.h) - eingebaut oder als
// system\GothicCoopMod-<Ini>.json (u194, P196).

namespace GOTHIC_ENGINE {

    void CoopModLoAInit();   // Mods\ModLoA.cpp

    static const CoopModData& CoopModDataGet() {
        (void)CoopProfile();   // liest eingebautes Profil und Datei beim ersten Zugriff
        return CoopModDataRef();
    }

    // u188 (P190), u194 (P196): Skript-Ganzzahlen, die in dieser Mod je PC bleiben - "perPcVars" des Profils (CoopDialogSync.cpp,
    // DlgNeverSync: nie im Story-Abgleich und nicht in der Pruefrunde "Alle Skript-Variablen gleich")
    bool CoopModNeverSync(const char* nm) {
        if (!nm || !*nm)
            return false;
        const CoopModData& d = CoopModDataGet();
        if (!d.perPcExact.empty() && std::binary_search(d.perPcExact.begin(), d.perPcExact.end(), std::string(nm)))
            return true;
        for (const std::string& s : d.perPcPrefix)
            if (!strncmp(nm, s.c_str(), s.size()))
                return true;
        return false;
    }

    // u197 (P207): "heroVars" des Profils - Werte des eigenen Helden. CoopDialogSync.cpp nimmt sie zu den Heldenwerten (StoryHeroVars):
    // nie im Abgleich, gesichert in COOPHERO.TXT und beim Laden/Uebernehmen vom eigenen Helden zurueckgeschrieben.
    bool CoopModHeroVar(const char* nm) {
        if (!nm || !*nm)
            return false;
        const CoopModData& d = CoopModDataGet();
        if (!d.heroExact.empty() && std::binary_search(d.heroExact.begin(), d.heroExact.end(), std::string(nm)))
            return true;
        for (const std::string& s : d.heroPrefix)
            if (!strncmp(nm, s.c_str(), s.size()))
                return true;
        return false;
    }

    // u194 (P196): "configDefaults" des Profils in die gelesene GothicCoopConfig.json - nur Schalter, die die Datei des Spielers nicht
    // setzt (Plugin.cpp, Game_Entry gleich nach dem Lesen, vor allen anderen Lesern). Gemeldet wird in CoopModsInit.
    void CoopModApplyConfigDefaults() {
        (void)CoopProfile();
        CoopModData& d = CoopModDataRef();
        d.applied.clear();
        for (auto it = d.configDefaults.begin(); it != d.configDefaults.end(); ++it) {
            if (CoopConfig.contains(it.key()))
                d.applied.push_back(it.key() + " (GothicCoopConfig.json gilt)");
            else {
                CoopConfig[it.key()] = it.value();
                d.applied.push_back(it.key() + "=" + it.value().dump());
            }
        }
    }

    // u194 (P196): Profil mit Pruefsumme seiner Daten - Abgleich mit dem Partner (Hauptmenue, Selbsttest "Mod-Profil gleich")
    std::string CoopModProfileStamp() {
        const CoopProfileInfo& p = CoopProfile();
        const CoopModData& d = CoopModDataGet();
        if (!d.stamp)
            return p.name;
        char b[16];
        _snprintf(b, sizeof(b), "%08X", d.stamp);
        b[sizeof(b) - 1] = 0;
        return p.name + " #" + b;
    }

    static std::string CoopModJoin(const std::vector<std::string>& v, const char* tail, size_t max) {
        std::string s;
        for (size_t i = 0; i < v.size() && i < max; i++)
            s += (i ? ", " : "") + v[i] + tail;
        if (v.size() > max)
            s += ", ... (" + std::to_string(v.size() - max) + " weitere)";
        return s;
    }

    // u187: einmal je Start (Plugin.cpp, Game_Init nach CoopModulesReport); u194: Daten des Profils und die Profil-Datei ins Log
    void CoopModsInit() {
        const CoopProfileInfo& p = CoopProfile();
        const CoopModData& d = CoopModDataGet();
        switch (p.id) {
        case COOP_PROFILE_LOA:
            CoopModLoAInit();
            break;
        default:
            break;
        }
        if (!d.source.empty()) {
            std::string vars = CoopModJoin(d.perPcExact, "", 12);
            const std::string pre = CoopModJoin(d.perPcPrefix, "*", 8);
            if (!pre.empty())
                vars += (vars.empty() ? "" : ", ") + pre;
            const unsigned n = (unsigned)(d.perPcExact.size() + d.perPcPrefix.size());
            std::string hero = CoopModJoin(d.heroExact, "", 12);   // u197 (P207)
            const std::string hpre = CoopModJoin(d.heroPrefix, "*", 8);
            if (!hpre.empty())
                hero += (hero.empty() ? "" : ", ") + hpre;
            const unsigned nh = (unsigned)(d.heroExact.size() + d.heroPrefix.size());
            OthelloCompatLog("Mod-Profil %s (%s): %u Werte je PC%s%s, %u Heldenwerte%s%s, %u Vorgaben, %u Hinweise - Kennung \"%s\"",
                p.name.c_str(), d.source.c_str(), n, n ? ": " : "", vars.c_str(), nh, nh ? ": " : "", hero.c_str(),
                (unsigned)d.configDefaults.size(), (unsigned)d.notes.size(), CoopModProfileStamp().c_str());
        }
        else if (p.id == COOP_PROFILE_MOD)
            OthelloCompatLog("Mod-Profil: %s (%s) - kein eigenes Profil, es laeuft das Fundament %s; Profil als Datei: %s (docs\\MODULE.md)",
                p.name.c_str(), p.modIni.c_str(), COOP_FASSUNG, d.fileShown.c_str());
        if (d.fileFound && !d.fileUsed)
            OthelloCompatLog("Mod-Profil-Datei %s NICHT benutzt: %s", d.fileShown.c_str(), d.fileError.c_str());
        for (const std::string& w : d.warnings)
            OthelloCompatLog("Mod-Profil: %s", w.c_str());
        if (!d.applied.empty())
            OthelloCompatLog("Mod-Profil: Vorgaben %s", CoopModJoin(d.applied, "", 20).c_str());
        for (const std::string& t : d.notes)
            OthelloCompatLog("Hinweis des Mod-Profils: %s", t.c_str());
    }
}
