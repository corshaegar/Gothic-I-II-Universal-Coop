// Coop-Skripte (GothicCoop, Gothic 2 Die Nacht des Raben ohne Mod). Ab u12 - erzeugt von tools\skripte_g2.py.
// Gehoert nach system\Autorun (zParserExtender, Game-Parser wie PC_MULTIPLAYER_HERO.D). Beide PCs brauchen dieselbe Datei.

// Aussehen der Coop-Rune: Vorgabe Gothics Rune "Teleport zu Xardas" (ab u17). Ab u21 setzt GothicCoop beim Start ein eigenes
// G2-Modell ein (ITRU_COOP_TELEPORT); klappt das nicht, bleibt diese Vorgabe.
const string coop_rune_visual = "ItRu_TeleportXardas.3ds";

// Teleport-Rune zum Mitspieler (ab u17). Sie nutzt Gothics Zauber SPL_TELEPORTXARDAS (Aufladen, Effekt und Animation wie jede
// Teleport-Rune). GothicCoop setzt beim Wirken das Ziel auf den Mitspieler (statt Xardas' Turm), zieht kein Mana ab und
// unterdrueckt die Meldung "Zu weit entfernt". Jeder Held bekommt sie nach dem Laden, falls er sie nicht hat (GothicCoop).
instance itru_coop_teleport(c_item) {
    name = "Teleport zum Mitspieler";
    mainflag = item_kat_rune;
    flags = 0;
    value = 0;
    visual = coop_rune_visual;
    material = mat_stone;
    spell = spl_teleportxardas;
    wear = wear_effect;
    effect = "SPELLFX_WEAKGLIMMER";
    description = name;
    text[0] = "Bringt dich zu deinem Mitspieler (Coop).";
    text[1] = name_manakosten;
    count[1] = 0;
};

// Merker fuer den Coop-Testlauf (Shift+F11, Schritte "Skript-Variable", wie in Gothic 1). Kein Skript liest ihn;
// GothicCoop setzt ihn waehrend des Laufs und danach wieder auf 0 - er prueft damit den Abgleich von Skript-Variablen.
var int coop_test_var;
