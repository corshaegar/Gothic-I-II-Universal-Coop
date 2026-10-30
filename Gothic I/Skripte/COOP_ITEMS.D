// Coop-Gegenstaende (GothicCoop, Gothic 1 ohne Mod). Ab u5 - erzeugt von tools\skripte_g1.py.
// Gehoert nach system\Autorun (zParserExtender, Game-Parser wie PC_MULTIPLAYER_HERO.D). Beide PCs brauchen dieselbe Datei.

// Aussehen der Coop-Rune. Vorbelegt mit Gothics gruener Rune (ItArRuneTeleport5); GothicCoop legt beim Start ein eigenes
// Modell an (ITRU_COOP_TELEPORT.3DS, Bild "F") und setzt diese Konstante darauf, sobald Gothic es laden kann.
const string coop_rune_visual = "ItAr_Rune_39.3ds";

// Teleport-Rune zum Mitspieler. Sie nutzt Gothics Teleport-Zauber SPL_TELEPORT5 (Aufladen, Effekt und Animation wie jede
// Teleport-Rune). GothicCoop setzt beim Wirken das Ziel auf den Mitspieler (statt des Wegpunkts aus Spell_Logic_Teleport5)
// und zieht dafuer kein Mana ab. Jeder Held bekommt sie nach dem Laden, falls er sie nicht hat (GothicCoop).
instance itru_coop_teleport(c_item) {
    name = "Teleport zum Mitspieler";
    mainflag = item_kat_rune;
    flags = 0;
    value = 0;
    visual = coop_rune_visual;
    material = mat_stone;
    spell = spl_teleport5;
    description = name;
    text[0] = "Bringt dich zu deinem Mitspieler (Coop).";
    text[1] = name_mana_needed;
    count[1] = 0;
};

// Coop-Inventar (ab u97 als Coop-Truhe, u101 umbenannt; die Instanz heisst weiter so - Spielstaende behalten den Gegenstand):
// gemeinsames Inventar beider Helden. Benutzen oeffnet das Fenster direkt (Inventar daneben, Gegenstaende hin und her) - GothicCoop
// faengt das Benutzen ab, ohne Bewegung, nichts verbraucht; ohne diesen Haken setzt es nur den Merker coop_truhe_oeffnen. Ebenso die
// Taste aus den COOP-Einstellungen. Den Inhalt fuehrt der Host, er wird mit dem Spielstand gespeichert. Jeder Held bekommt es,
// falls er es nicht hat (GothicCoop). Gothic 1 hat keinen Beutel - Aussehen: das Welt-Modell DT_CHAINBOX (meshes.VDF).
var int coop_truhe_oeffnen;
func void use_coop_truhe() {
    coop_truhe_oeffnen = 1;
};
instance itmi_coop_truhe(c_item) {
    name = "Coop-Inventar";
    mainflag = item_kat_none;
    flags = 0;
    value = 0;
    visual = "DT_CHAINBOX.3DS";
    material = mat_wood;
    scemename = "MAPSEALED";
    on_state[0] = use_coop_truhe;
    description = name;
    text[0] = "Gemeinsames Inventar beider Helden (Coop).";
    text[1] = "Benutzen oder Taste aus den COOP-Einstellungen.";
};

// Merker fuer den Coop-Testlauf (Shift+F11, Schritte "Skript-Variable", wie Othello COOP_ITEMS-v107.D). Kein Skript liest ihn;
// GothicCoop setzt ihn waehrend des Laufs und danach wieder auf 0 - er prueft damit den Abgleich von Skript-Variablen.
var int coop_test_var;
