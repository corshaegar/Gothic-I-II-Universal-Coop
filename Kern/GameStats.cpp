namespace GOTHIC_ENGINE {
    // u237 (Nutzer 09.11.: "debug sachen nicht in der Release version"): die Netzwerk-Statistik der Vorlage (Taste O, Warteschlangen,
    // eigene Lage und NPC-Listen in der Meldungszeile) ist entfernt. Es bleibt die Taste fuer die Meldungszeile (toggleGameLogKey, P).
    void GameStatsLoop() {
        PluginState = "GameStatsLoop";

        if (WorldEditMode) {
            return;
        }

        if (zinput->KeyToggled(ToggleGameLogKey)) {
            GameChat->ToggleShowing();
        }
    }
}
