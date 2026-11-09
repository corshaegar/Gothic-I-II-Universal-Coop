namespace GOTHIC_ENGINE {
    void UpdateVisibleNpc() {
        PluginState = "UpdateVisibleNpc";

        if (ServerThread) {
            auto npcs = GetVisibleNpcs();
            std::map<string, LocalNpc*> updatedBroadcastNpcs;

            for each (auto npc in npcs)
            {
                if (NpcToUniqueNameList.count(npc) == 0) {
                    continue;
                }
                if (CoopLeadIsRemote(npc)) {   // v72: den fuehrt gerade ein Client im Kampf - der sendet ihn
                    continue;
                }

                auto npcUniqueName = NpcToUniqueNameList[npc];
                if (npcUniqueName.IsEmpty())   // u51 (Befund 3): ohne Namen nicht senden (mehrere teilten sich sonst "")
                    continue;
                if (BroadcastNpcs.count(npcUniqueName) == 0) {
                    updatedBroadcastNpcs[npcUniqueName] = new LocalNpc(npc, npcUniqueName);
                }
                else {
                    // u230 (P274): der Name gehoert jetzt einem anderen Objekt (Trialog: LeGos _TRIA_COPY tauscht zwei NPCs, GT ihre
                    // Namen) - der Sender folgt dem Namen, sonst gingen Lage und Animation des alten Objekts unter diesem Namen hinaus
                    LocalNpc* l = BroadcastNpcs[npcUniqueName];
                    if (l && l->npc != npc)
                        l->npc = npc;
                    updatedBroadcastNpcs[npcUniqueName] = l;
                }
            }

            BroadcastNpcs = updatedBroadcastNpcs;
        }
    }
}