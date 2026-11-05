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
    };

    class Chat
    {
    private:
        bool isShowing = true;
        unsigned int chatLines = 50;
        std::vector<ChatLine> lines;
        SafeQueue<ChatLine> readyToBeDisplatedLines;
    public:
        void Render() {
            while (!readyToBeDisplatedLines.isEmpty()) {
                if (lines.size() < chatLines)
                    lines.push_back(readyToBeDisplatedLines.dequeue());
                else
                {
                    lines.erase(lines.begin());
                    lines.push_back(readyToBeDisplatedLines.dequeue());
                }
            }

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
                        screen->SetFontColor(lines[i].color);
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
