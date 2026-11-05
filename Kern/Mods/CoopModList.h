// Bekannte Mods des Coops (Fundament + Mod-Profil, docs\MODULE.md). Wird in CoopCore.cpp im Namensraum der Engine eingebunden -
// ohne #pragma once, weil Mehr-Engine-Builds Sources.h je Engine neu einbinden.
//
// u187: Eine Mod, die Gothic mit -game:<ini> startet, nennt sich im Log mit dem Titel aus [INFO] ihrer Ini. Steht ihr Ini-Name hier,
// bekommt sie ein eigenes Profil; ihre Sonderwege liegen dann in Mods\Mod<Kuerzel>.cpp und laufen nur in diesem Profil. Eine Mod, die
// hier fehlt, laeuft mit dem Fundament (COOP_PROFILE_MOD). Othello erkennt CoopProfile() weiter an OTHELLO_ABI.DLL bzw. seinen VDFs.
// u194 (P196): Was ein Profil nur an Daten braucht (Werte je PC, Vorgaben, Hinweise), steht im Format der Profil-Datei
// (Mods\CoopModProfile.h) - eingebaut ueber "builtin", fuer jede andere Mod als system\GothicCoopMod-<Ini>.json ohne neuen Bau.

const char* CoopModLoAProfile();   // u194: Mods\ModLoA.cpp - eingebautes Profil von Legend of Ahssun

struct CoopModKnown {
    int engine;            // Engine_G1 / Engine_G2A ...
    const char* iniBase;   // Ini-Name ohne ".ini" (-game:LoA.ini -> "LoA")
    CoopProfileId id;      // eigenes Profil
    const char* name;      // Anzeigename, falls die Ini keinen Titel hat
    const char* (*builtin)();   // u194 (P196): eingebautes Profil im Dateiformat, NULL = keins
};

static const CoopModKnown CoopModsKnown[] = {
    { Engine_G2A, "LoA", COOP_PROFILE_LOA, "Legend of Ahssun", CoopModLoAProfile },   // u170-u181: P177, P178, P180, P181 (allgemein), Mods\ModLoA.cpp
};

static const CoopModKnown* CoopModFind(const std::string& iniBase) {
    for (const CoopModKnown& m : CoopModsKnown)
        if (m.engine == ENGINE && !_stricmp(m.iniBase, iniBase.c_str()))
            return &m;
    return NULL;
}
