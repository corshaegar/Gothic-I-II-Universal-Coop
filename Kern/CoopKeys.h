// Supported with union (c) 2020 Union team
// Union HEADER file

namespace GOTHIC_ENGINE {

    // u226 (P264): Coop-Tasten - eine Liste fuer alle Engines, alle in den COOP-Einstellungen einstellbar (CoopKeys.cpp).
    // Die Module (Plugin.cpp, CoopCamera.cpp, CoopTruhe.cpp, ReviveFriendLoop.cpp, CoopSaveSync.cpp) fragen nur noch CoopKeyHit.
    enum CoopKeyAction {
        CK_HELP,         // Coop-Hilfe (MENU_COOP_HELP)
        CK_MENU,         // Coop-Fenster (MENU_COOP)
        CK_INV,          // Coop-Inventar
        CK_RUNE,         // Teleport zum Mitspieler (Rune im Inventar)
        CK_CAM,          // Zuschau-Fenster
        CK_HOST,         // Hosten
        CK_JOIN,         // Beitreten, als Client Pause/weiter
        CK_REVIVE,       // Mitspieler wiederbeleben
        CK_REINIT,       // Mitspieler-Figuren neu einsetzen
        CK_SYNC,         // Welt neu abgleichen
        // u237 (Nutzer 09.11.: "die Coop sicherungen mit F9 und andere debug sachen ... das feature mit dem Weltprozess auf F12 ...
        // entfernt werden ... das coop sicherungen fenster ist auch schon veraltet, es ist bereits in den Menues integriert"): F9/F8
        // (Liste der Coop-Sicherungen) und Shift+F12 (Weltprozess an/aus) entfallen - die Liste steht unter Esc > Spielstand waehlen,
        // der Weltprozess startet wie bisher von selbst
        CK_COUNT
    };

    bool CoopKeyHit(int action);          // Taste mit den eingestellten Zusatztasten eben gedrueckt (einmal je Druck, nicht in Menues)
    std::string CoopKeyText(int action);  // Belegung zur Anzeige ("Shift+F5", "aus")
    int CoopKeyDik(int action);           // Haupttaste (0 = aus)
}
