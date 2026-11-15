namespace GOTHIC_ENGINE {
    // u171: Aufnahme (Nutzer 4.11.2026: "Am besten die Debug Nachrichten die immer angezeigt werden ausblenden") - mit -coopaufnahme
    // in der Kommandozeile zeichnet GT weder die Meldungszeilen oben links (ChatLog) noch die Fassungszeile (Plugin.cpp); ins Log
    // geht alles wie sonst. Der Host eines Testlaufs gibt den Schalter an den Client weiter (CoopAutoTest.cpp). Allgemein.
    static bool CoopRecordMode() {
        static int on = -1;
        if (on < 0)
            on = strstr(GetCommandLineA(), "-coopaufnahme") ? 1 : 0;
        return on == 1;
    }

    struct ChatLine
    {
        string text;
        zCOLOR color;
        long long ms = 0;   // u267 (P329): seit wann sichtbar
    };

    // u267 (P329, Nutzer 15.11.: "die Meldungen so machen dass sie eingeblendet und wieder ausgeblendet werden, weil wie es jetzt ist
    // stacken sich die Nachrichten oben links am Bildschirm"): jede Zeile blendet in 0,3 s ein, steht 8 s und blendet in 1,5 s aus, danach
    // faellt sie weg; hoechstens 8 Zeilen gleichzeitig (die aelteste geht zuerst)
    static const long long ChatEinMs = 300, ChatStehtMs = 8000, ChatAusMs = 1500;
    static const size_t ChatHoechstens = 8;

    class Chat
    {
    private:
        bool isShowing = true;
        unsigned int chatLines = 50;
        std::vector<ChatLine> lines;
        SafeQueue<ChatLine> readyToBeDisplatedLines;
    public:
        void Render() {
            const long long now = (long long)GetTickCount64();
            while (!readyToBeDisplatedLines.isEmpty()) {
                ChatLine l = readyToBeDisplatedLines.dequeue();
                l.ms = now;
                lines.push_back(l);
            }
            while (!lines.empty() && (lines.size() > ChatHoechstens || now - lines.front().ms > ChatStehtMs + ChatAusMs))
                lines.erase(lines.begin());   // u267 (P329)

            if (this->IsShowing() == true && !CoopRecordMode())   // u171: in Aufnahmen keine Meldungszeilen
            {
                zSTRING font = screen->GetFontName();
                zCOLOR color = zCOLOR(255, 255, 255, 255);
                screen->SetFont(zSTRING("Font_Old_10_White_Hi.TGA"));
                int lineDistance = 150;

                for (unsigned int i = 0; i < lines.size(); i++)
                {
                    if (lines[i].text.Length() > 0)
                    {
                        // u267 (P329): einblenden bzw. ausblenden ueber die Deckkraft der Schrift
                        const long long age = now - lines[i].ms;
                        int a = 255;
                        if (age < ChatEinMs)
                            a = (int)(255 * age / ChatEinMs);
                        else if (age > ChatStehtMs)
                            a = (int)(255 - 255 * (age - ChatStehtMs) / ChatAusMs);
                        a = a < 0 ? 0 : (a > 255 ? 255 : a);
                        zCOLOR c = lines[i].color;
                        c.alpha = (unsigned char)(c.alpha * a / 255);
                        screen->SetFontColor(c);
                        screen->Print(50, lineDistance, zSTRING(lines[i].text));

                        screen->SetFontColor(color);
                    }
                    lineDistance = lineDistance + 150;
                }

                screen->SetFont(font);
            }
        };

        bool IsShowing() {
            return isShowing;
        }

        void ToggleShowing() {
            isShowing = !isShowing;
        }

        unsigned int GetLines() {
            return lines.size();
        }

        void AddLine(string text, zCOLOR color = zCOLOR(255, 255, 255, 255)) {
            ChatLine line;
            line.text = text;
            line.color = color;

            readyToBeDisplatedLines.enqueue(line);
        }

        void Clear() {
            lines.clear();
        }
    };
}
