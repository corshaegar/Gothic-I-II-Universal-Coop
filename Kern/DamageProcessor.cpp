namespace GOTHIC_ENGINE {
    // u51 (Durchsicht u50, Befund 3): eindeutigen Namen eines NPCs fuer ein Trefferpaket holen. Bis u50 legte
    // NpcToUniqueNameList[npc] fuer einen noch unbenannten NPC (hoechstens 1 s alt: Beschwoerung, Wld_InsertNpc, Spawn) einen
    // Eintrag "" an - BuildGlobalNpcList (Helpers.cpp) ueberspringt jeden NPC mit Eintrag, er blieb bis zum naechsten Laden ohne
    // Namen: mehrere solche teilten sich BroadcastNpcs[""], der Host meldete "" als neuen NPC (SpawnPulse), Treffer gingen mit Ziel
    // "" hinaus. Jetzt: unbenannt -> die Namensliste gleich nachfuehren; ist er dann noch unbekannt (nicht mehr in der Welt,
    // Figur, ausgenommener NPC), zaehlt der Treffer nicht (der Partner koennte ihn ohnehin keinem NPC zuordnen). Erst danach den
    // NPC anfassen - ein Treffer aus dem vorigen Bild kann einem inzwischen geloeschten NPC gelten. Allgemein (G1/G2).
    static bool CoopHitNpcUniqueName(oCNpc* npc, string& out, const char* role) {
        if (!npc)
            return false;
        auto it = NpcToUniqueNameList.find(npc);
        if (it == NpcToUniqueNameList.end()) {
            BuildGlobalNpcList();
            PluginState = "DamageProcessorLoop";
            it = NpcToUniqueNameList.find(npc);
        }
        if (it == NpcToUniqueNameList.end() || it->second.IsEmpty()) {
            static CoopDiagBudget budget = { 0, 0 };
            if (CoopDiagAllow(budget, 10))
                OthelloCompatLog("[%lld s, %s] Treffer: %s %p ohne Namen (nicht in der NPC-Liste) - nicht gemeldet", DiagSeconds(), DiagRole(),
                    role, (void*)npc);
            return false;
        }
        out = it->second;
        return true;
    }

    void DamageProcessorLoop() {
        PluginState = "DamageProcessorLoop";
        CoopDamageFinalizePending();   // Treffer aus dem letzten Durchlauf endgueltig messen
        CoopSwingPulse();              // v53: Ergebnisse bestaetigter NPC-Schlaege an den Host (CoopHitAuthority.cpp)
        CoopPunishPulse();             // v54: Bestrafung wegen eines Client-Treffers geht auf dessen Figur (CoopHitAuthority.cpp)
        CoopKoWatchPulse();            // v100: K.o.-Wache - kommt der Todesstoss? (nur Log, CoopCombatLead.cpp)

        if (ReadyToSyncDamages.isEmpty()) {
            return;
        }

        auto hit = ReadyToSyncDamages.dequeue();

        if (!ClientThread && !ServerThread)
        {
            return;
        }

        if (Myself && hit.attacker == Myself->npc) {
            if (!CoopHitNpcUniqueName(hit.npc, hit.npcUniqueName, "Ziel"))   // u51
                return;
            CoopDiagClientHit(hit.npc);

            if (hit.npc->IsDead()) {
                KilledByPlayerNpcNames[hit.npcUniqueName] = hit.npc;
            }

            Myself->hitsToSync.push_back(hit);
            return;
        }

        if (ServerThread) {
            if (PlayerNpcs.count(hit.attacker)) {
                return;
            }

            string uniqueNpcName;
            if (!CoopHitNpcUniqueName(hit.attacker, uniqueNpcName, "Angreifer"))   // u51
                return;
            if (BroadcastNpcs.count(uniqueNpcName)) {
                if (hit.npc == player) {
                    hit.npcUniqueName = "HOST";
                    BroadcastNpcs[uniqueNpcName]->hitsToSync.push_back(hit);
                }
                else if (PlayerNpcs.count(hit.npc)) {
                    hit.npcUniqueName = PlayerNpcs[hit.npc];
                    BroadcastNpcs[uniqueNpcName]->hitsToSync.push_back(hit);
                }
                else if (CoopHitNpcUniqueName(hit.npc, hit.npcUniqueName, "Ziel")) {   // u51
                    BroadcastNpcs[uniqueNpcName]->hitsToSync.push_back(hit);
                }
            }
        }
    }
}
