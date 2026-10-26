// Coop-Figur des Mitspielers (GothicCoop, Gothic 1 ohne Mod). Ab u5 - erzeugt von tools\skripte_g1.py.
// Gehoert nach system\Autorun (zParserExtender, Game-Parser). Beide PCs brauchen dieselbe Datei.
// Anders als unter Othello ohne aivar[49] (AIV_BOSSBEHAVIOR wertet nur Othello aus). Das Aussehen setzt GothicCoop zur Laufzeit
// (COOP-Menue "Deine Figur"); hier nur die Vorgabe mit Texturen, die Gothic 1 hat (Koerper 1, Hautton 1, Gesicht 18).
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
	mdl_setvisualbody(self, "hum_body_Naked0", 1, 1, "Hum_Head_Pony", 18, 0, -1);
};
