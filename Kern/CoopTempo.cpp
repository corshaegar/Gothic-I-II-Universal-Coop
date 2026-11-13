namespace GOTHIC_ENGINE {
    // u253 (P304): oeffentlicher Bau - der Zeitraffer gehoert zu den automatischen Laeufen des Entwicklungsbaus; hier laeuft das
    // Spiel immer mit normalem Tempo, ein Tempo-Paket eines Partners wird ignoriert.
    void CoopTempoReceive(json&, const std::string&) {}   // PacketProcessor.cpp
    void CoopTempoPulse() {}                              // Plugin.cpp
}
