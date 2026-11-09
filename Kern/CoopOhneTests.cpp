// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {

    // --- u237: oeffentlicher Bau ohne Tests -----------------------------------------------------------------------------
    //
    // Nutzer 09.11.2026: "debug sachen nicht in der Release version ... Dazu zaehlt auch zB ... die Tests" - der Coop-Testlauf, die
    // Vorfuehrung fuer Aufnahmen und der automatische Testlauf gehoeren nur zum Entwicklungsbau und laufen dort ueber die Kommandozeile
    // der automatischen Laeufe. Der oeffentliche Bau bindet stattdessen diese Datei ein (Sources.h): nur die leeren Gegenstuecke der
    // Aufrufe aus den uebrigen Modulen.

    bool CoopSelfTestRunning() { return false; }                    // CoopSaveSync.cpp (P281)
    bool CoopSelfTestHandles(int) { return false; }                 // PacketProcessor.cpp
    void CoopSelfTestReceive(json&, const std::string&) {}          // PacketProcessor.cpp
    void CoopSelfTestPulse() {}                                     // Plugin.cpp
    bool CoopShowActive() { return false; }                         // CoopKeys.cpp (Einfuehrung)
    void CoopAutoTestInit() {}                                      // UnionHooks.cpp
    void CoopAutoTestEarly() {}                                     // Plugin.cpp
    void CoopAutoTestMenuPulse() {}                                 // UnionHooks.cpp
    void CoopAutoTestPulse() {}                                     // Plugin.cpp
}
