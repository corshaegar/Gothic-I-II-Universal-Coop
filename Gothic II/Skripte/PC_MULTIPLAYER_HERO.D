// Coop-Figur des Mitspielers (GothicCoop, Gothic 2 Die Nacht des Raben ohne Mod). Ab u8 - erzeugt von tools\skripte_g2.py.
// Gehoert nach system\Autorun (zParserExtender, Game-Parser). Beide PCs brauchen dieselbe Datei.
// Das Aussehen setzt GothicCoop zur Laufzeit (COOP-Menue "Deine Figur"); hier nur die Vorgabe wie GTs G2-Werte
// (Koerper 9, Hautton 0, Gesicht 18 - Gothic 2 hat Gesichter 0-162 und Koerper 0-12, jeweils nur C0).
instance pc_multiplayer_hero(npc_default) {
    name = "Player";
    guild = gil_none;
    id = 0;
    voice = 15;
    level = 0;
    npctype = npctype_main;
    exp = 0;
    exp_next = 500;
    lp = 0;

	attribute[ATR_STRENGTH] 		= 10;
	attribute[ATR_DEXTERITY] 		= 10;
	attribute[ATR_MANA_MAX] 		= 10;
	attribute[ATR_MANA] 			= 10;
	attribute[ATR_HITPOINTS_MAX]	= 40;
	attribute[ATR_HITPOINTS] 		= 40;

    mdl_setvisual(self, "HUMANS.MDS");
	mdl_setvisualbody(self, "hum_body_Naked0", 9, 0, "Hum_Head_Pony", 18, 0, -1);
};
