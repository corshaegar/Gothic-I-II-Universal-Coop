// Bekannte Mods des Coops (Fundament + Mod-Profil, docs\MODULE.md). Wird in CoopCore.cpp im Namensraum der Engine eingebunden -
// ohne #pragma once, weil Mehr-Engine-Builds Sources.h je Engine neu einbinden.
//
// u187: Eine Mod, die Gothic mit -game:<ini> startet, nennt sich im Log mit dem Titel aus [INFO] ihrer Ini. Steht ihr Ini-Name hier,
// bekommt sie ein eigenes Profil; ihre Sonderwege liegen dann in Mods\Mod<Kuerzel>.cpp und laufen nur in diesem Profil. Eine Mod, die
// hier fehlt, laeuft mit dem Fundament (COOP_PROFILE_MOD). Othello erkennt CoopProfile() weiter an OTHELLO_ABI.DLL bzw. seinen VDFs.
// u194 (P196): Was ein Profil nur an Daten braucht (Werte je PC, Vorgaben, Hinweise), steht im Format der Profil-Datei
// (Mods\CoopModProfile.h) - eingebaut ueber "builtin", fuer jede andere Mod als system\GothicCoopMod-<Ini>.json ohne neuen Bau.
// u204: Mods ohne -game (eigene Installation, Mod-Dateien direkt in Data\ - The Chronicles of Myrtana: Archolos) erkennt CoopProfile()
// an einer Datei ("marker", relativ zum Spielordner); iniBase ist dann Kuerzel fuer Kennung und Profil-Datei.

const char* CoopModLoAProfile();        // u194: Mods\ModLoA.cpp - eingebautes Profil von Legend of Ahssun
const char* CoopModArcholosProfile();   // u204: Mods\ModArcholos.cpp - eingebautes Profil von Archolos

struct CoopModKnown {
    int engine;            // Engine_G1 / Engine_G2A ...
    const char* iniBase;   // Ini-Name ohne ".ini" (-game:LoA.ini -> "LoA"); ohne -game das Kuerzel der Mod
    CoopProfileId id;      // eigenes Profil
    const char* name;      // Anzeigename, falls die Ini keinen Titel hat
    const char* (*builtin)();   // u194 (P196): eingebautes Profil im Dateiformat, NULL = keins
    const char* marker;    // u204: Datei der Mod, an der sie ohne -game erkannt wird (NULL = nur ueber -game)
};

static const CoopModKnown CoopModsKnown[] = {
    { Engine_G2A, "LoA", COOP_PROFILE_LOA, "Legend of Ahssun", CoopModLoAProfile, NULL },   // u170-u181: P177, P178, P180, P181 (allgemein), Mods\ModLoA.cpp
    { Engine_G2A, "Archolos", COOP_PROFILE_ARCHOLOS, "The Chronicles of Myrtana: Archolos", CoopModArcholosProfile, "\\Data\\KM_Scripts.mod" },   // u204: P233, P234, Mods\ModArcholos.cpp
};

static const CoopModKnown* CoopModFind(const std::string& iniBase) {
    for (const CoopModKnown& m : CoopModsKnown)
        if (m.engine == ENGINE && !m.marker && !_stricmp(m.iniBase, iniBase.c_str()))
            return &m;
    return NULL;
}
