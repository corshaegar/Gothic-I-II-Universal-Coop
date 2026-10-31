// Supported with union (c) 2020 Union team
// Union HEADER file

namespace GOTHIC_ENGINE {

    // --- Engine-Tabelle (u2, Plan Phase 1 Schritt 3) ------------------------------------------------------------------
    //
    // Jede feste Engine-Adresse, die GT ausserhalb von Othellos Nachbildungen benutzt, steht hier mit Namen und mit dem Wert je
    // Engine: COOP_ENGINE_SWITCH(G1, G1A, G2 Classic, G2A). Code schreibt nie mehr eine nackte Adresse, sondern CoopAddr::Name.
    // 0 = fuer diese Engine nicht bestimmt bzw. dort nicht vorhanden - wer den Wert nutzt, prueft auf 0.
    //
    // Herkunft:
    //  - Funktionen: Union-SDK (ZenGin\<Engine>\API\*.h, zCall), zugeordnet ueber Header, Klasse und Signatur
    //    (tools\coopaddr_gen.py, wie tools\g1g2map.py). Die SDK-Werte nutzt Union selbst; G1 ist zusaetzlich gegen
    //    GothicMod.exe 1.08k_mod geprueft (byte-gleich mit der Exe unter Othello).
    //  - Daten, Aufrufstellen, Helfer ohne SDK-Namen: von Hand an der Exe belegt (tools\datref.py, tools\gdis.py; Beleg im
    //    Kommentar). Gepruefte Exes: GothicMod.exe 1.08k_mod (G1), Gothic2.exe 2.6 fix (G2A). G1A und G2 Classic nur aus dem
    //    SDK bzw. aus COOP-FIXES.md Abschnitt 6, ungeprueft.
    //  - vtables: SDK VFTable_<Engine>.h (vftable_<Klasse>::GetAddress()).
    // Othellos Nachbildungen (OthelloCompat.cpp, nur G1) und die Erwartungsliste der Selbstpruefung (CoopAudit.cpp) behalten
    // ihre Adressen dort - sie gelten nur fuer das Othello-Profil.
    // Neue Adresse: Zeile hier, Zeile in docs\COOP-FIXES.md Abschnitt 6, Beleg im Kommentar.

    namespace CoopAddr {

        // Funktionen (SDK) ------------------------------------------------------------------------------------------------
        static const uint oCNpc_ChangeAttribute            = COOP_ENGINE_SWITCH(0x0068D7C0, 0x006BE040, 0x006D1B90, 0x0072FF60);   // oCNpc::void ChangeAttribute( int, int ) - Haken HP-Abzug skalieren/messen (C8, H16; CoopDamage.cpp)
        static const uint oCNpc_EV_Output                  = COOP_ENGINE_SWITCH(0x006B1A70, 0x006E5CA0, 0x006F88C0, 0x007576F0);   // oCNpc::int EV_Output( oCMsgConversation* ) - Haken Dialogzeile (D3; CoopDialogSync.cpp)
        static const uint oCNpc_EV_OutputSVM               = COOP_ENGINE_SWITCH(0x006B15A0, 0x006E5750, 0x006F83C0, 0x007571F0);   // oCNpc::int EV_OutputSVM( oCMsgConversation* ) - Haken SVM-Zeile (D3)
        static const uint oCNpc_AssessTalk_S               = COOP_ENGINE_SWITCH(0x006B5F50, 0x006EA7E0, 0x006FD910, 0x0075C890);   // oCNpc::int AssessTalk_S( oCNpc* ) - Haken Ansprech-Sperre (D3)
        static const uint oCNpc_States_DoAIState           = COOP_ENGINE_SWITCH(0x006C5C60, 0x006FB5B0, 0x0070DC40, 0x0076D1A0);   // oCNpc_States::int DoAIState() - Haken Tagesablauf anhalten (D3)
        static const uint oCInformationManager_OnInfo      = COOP_ENGINE_SWITCH(0x0072D560, 0x0076BB30, 0x00779070, 0x00662290);   // oCInformationManager::void __fastcall OnInfo( oCInfo* ) - Haken Dialog-Fortschritt (D4)
        static const uint oCInformationManager_OnChoice    = COOP_ENGINE_SWITCH(0x0072DC70, 0x0076C2A0, 0x00779780, 0x006629A0);   // oCInformationManager::void __fastcall OnChoice( oCInfoChoice* ) - Haken Dialog-Fortschritt (D4)
        static const uint oCNpc_ClearEM                    = COOP_ENGINE_SWITCH(0x006A2610, 0x006D4E70, 0x006E7960, 0x00746400);   // oCNpc::void ClearEM() - Haken Dialog-Waechter (CoopDialogWatch.cpp)
        static const uint zCMenuItemChoice_InitMenuItem    = COOP_ENGINE_SWITCH(0x004D7F70, 0x004E8F60, 0x004E2800, 0x004E51B0);   // zCMenuItemChoice::void InitMenuItem() - direkt: Auswahlfeld neu aus Gothic.ini (H8; CoopLook/CoopDifficulty)
        static const uint oCSpawnManager_CheckInsertNpcs   = COOP_ENGINE_SWITCH(0x006CF9A0, 0x007063D0, 0x00718650, 0x00777BE0);   // oCSpawnManager::void CheckInsertNpcs() - Haken Spawn-Takt (I1; CoopFarSync.cpp)
        static const uint oCSpawnManager_CheckRemoveNpc    = COOP_ENGINE_SWITCH(0x006D0A80, 0x007075C0, 0x007198F0, 0x007792E0);   // oCSpawnManager::int CheckRemoveNpc( oCNpc* ) - Haken Entfernen (I1)
        static const uint oCAniCtrl_Human_CreateHit        = COOP_ENGINE_SWITCH(0x00627C80, 0x0064CCF0, 0x00653EF0, 0x006B0830);   // oCAniCtrl_Human::void CreateHit( zCVob* ) - bestaetigten Schlag ausfuehren (C11; CoopHitAuthority.cpp)
        static const uint zCMenu_HandleEvent               = COOP_ENGINE_SWITCH(0x004CEE10, 0x004DF6A0, 0x004D94D0, 0x004DBB70);   // zCMenu::int HandleEvent( int ) - vtable-Pruefung zCMenu (H25; CoopMenuInject.cpp)
        static const uint zCMenu_HandleSelAction           = COOP_ENGINE_SWITCH(0x004CF6E0, 0x004DFF70, 0x004D9D90, 0x004DC430);   // zCMenu::int HandleSelAction( zTMenuItemSelAction, zSTRING const&, zCMenuItem* ) - Haken Menue-Auswahl (H5) + vtable-Pruefung
        static const uint zCMenu_SetByScript               = COOP_ENGINE_SWITCH(0x004CE720, 0x004DEFC0, 0x004D8C80, 0x004DB320);   // zCMenu::void SetByScript( zSTRING const& ) - vtable-Platz +0x40 (H25)
        static const uint zCMenu_ScreenInit                = COOP_ENGINE_SWITCH(0x004CFF30, 0x004E08E0, 0x004DA740, 0x004DCDE0);   // zCMenu::void ScreenInit() - vtable-Platz +0x30 (u21: Eintraege nur im Spiel/nur ausserhalb ausblenden, CoopMenuInject.cpp)
        static const uint zCMenuItemList_DrawFront         = COOP_ENGINE_SWITCH(0x004D9330, 0x004EA450, 0x004E3BA0, 0x004E6550);   // zCMenuItemList::void DrawFront() - Vergleich: Themenzeilen zeichnet Gothic selbst (u21); Haken markierte Zeile gruen (u22, CoopMenuMouse.cpp)
        static const uint zCMenuItem_UpdateFX              = COOP_ENGINE_SWITCH(0x004D4AB0, 0x004E55F0, 0x004DF0B0, 0x004E1A60);   // zCMenuItem::void UpdateFX() - Haken: markierter Eintrag gruen statt leuchtend (u22, CoopMenuMouse.cpp)
        static const uint zCMenuItemChoice_DrawFront       = COOP_ENGINE_SWITCH(0x004D79E0, 0x004E8920, 0x004E2270, 0x004E4C20);   // zCMenuItemChoice::void DrawFront() - Haken: "<" ">" am Auswahlfeld (u23, CoopMenuMouse.cpp)
        static const uint zCMenuItemText_SetText           = COOP_ENGINE_SWITCH(0x004D6430, 0x004E7170, 0x004E0B40, 0x004E34F0);   // zCMenuItemText::void SetText( zSTRING const&, int, int ) - direkt: Statuszeile (H5; CoopMenuLobby.cpp)
        static const uint zCView_SetPos                    = COOP_ENGINE_SWITCH(0x006FDA10, 0x00737E00, 0x00747AB0, 0x007A75B0);   // zCView::void SetPos( int, int ) - direkt: Vorschaubild (H23)
        static const uint zCView_SetSize                   = COOP_ENGINE_SWITCH(0x006FDC00, 0x00737FF0, 0x00747CA0, 0x007A77A0);   // zCView::void SetSize( int, int ) - direkt: Vorschaubild (H23)
        static const uint oCMenuSavegame_GetMenuItemSlotNr = COOP_ENGINE_SWITCH(0x0042D6E0, 0x00430930, 0x0042FC10, 0x0042FF30);   // oCMenuSavegame::int GetMenuItemSlotNr( zCMenuItem* ) - direkt: Slot unter der Maus (H9; CoopMenuMouse.cpp)
        static const uint zCVideoPlayer_Stop               = COOP_ENGINE_SWITCH(0x004696A0, 0x004717E0, 0x0046DD50, 0x0046E980);   // zCVideoPlayer::int Stop() - Intro abbrechen (H30b; CoopNewGame.cpp)
        static const uint oCGame_TriggerChangeLevel        = COOP_ENGINE_SWITCH(0x0063D480, 0x00664100, 0x0066AD80, 0x006C7AF0);   // oCGame::void TriggerChangeLevel( zSTRING const&, zSTRING const& ) - Haken gemeinsamer Weltwechsel (u32, P73; CoopWorlds.cpp)
        static const uint oCGame_ChangeLevel               = COOP_ENGINE_SWITCH(0x0063CD60, 0x00663950, 0x0066A660, 0x006C7290);   // oCGame::void ChangeLevel( zSTRING const&, zSTRING const& ) - Haken Lade-Beginn/-Ende, wenn Union sie nicht meldet (u64, Othello-Coop v114, B10; UnionHooks.cpp)
        static const uint winMain_LockJne                  = COOP_ENGINE_SWITCH(0x004F3FB2, 0, 0, 0x00502F22);   // WinMain: jne nach CreateFileMappingA("Lock_%s") + GetLastError == ERROR_ALREADY_EXISTS -> ExitProcess(1) (G2A 0x00502F24, G1 GothicMod.exe WinMain 0x004F3F60, gleiche Bytes 75 08 6A 01 FF 15, u42); nur im Weltprozess im Speicher jne -> jmp (u37, P79; CoopWorldProcess.cpp)
        static const uint oCGame_CheckObjectRoutines       = COOP_ENGINE_SWITCH(0x00640160, 0x00667180, 0x0066DE30, 0x006CABB0);   // oCGame::void CheckObjectRoutines() - Haken Kartenpfeile (O4; CoopPartnerMarker.cpp)
        static const uint oCSpell_Invest                   = COOP_ENGINE_SWITCH(0x0047CD40, 0x00487520, 0x00483A70, 0x004850D0);   // oCSpell::int Invest() - Haken Coop-Rune ohne Mana (N1; CoopTeleport.cpp)
        static const uint oCSpell_GetName                  = COOP_ENGINE_SWITCH(0x0047E1B0, 0x00488AE0, 0x00484E90, 0x004864B0);   // oCSpell::zSTRING GetName() - Haken Name der Coop-Rune (N1b)
        static const uint CGameManager_Menu                = COOP_ENGINE_SWITCH(0x004279F0, 0x0042A4F0, 0x00428F40, 0x004292D0);   // CGameManager::void Menu( int ) - direkt: Startmenue aus dem Spiel (H26; CoopTitle.cpp)
        static const uint oCMobContainer_Open              = COOP_ENGINE_SWITCH(0x006843D0, 0x006B3FA0, 0x006C8470, 0x00726500);   // oCMobContainer::void Open( oCNpc* ) - Haken Truhe oeffnen (E1; CoopWorldSync.cpp)
        static const uint oCMobContainer_Close             = COOP_ENGINE_SWITCH(0x00684490, 0x006B4080, 0x006C85B0, 0x00726640);   // oCMobContainer::void Close( oCNpc* ) - Haken Truhe schliessen (E1)
        static const uint oCWorld_GetClassDef              = COOP_ENGINE_SWITCH(0x006D5EA0, 0x0070D850, 0x0071F500, 0x0077EF40);   // oCWorld::zCClassDef* _GetClassDef() const - Pruefung oCWorld-vtable Platz 0 (E2)
        static const uint oCNpcInventory_Insert            = COOP_ENGINE_SWITCH(0x0066C7D0, 0x00699F50, 0x006AEC50, 0x0070C730);   // oCNpcInventory::oCItem* Insert( oCItem* ) - Haken ins Inventar (E2)
        static const uint oCNpc_PutInInv                   = COOP_ENGINE_SWITCH(0x006A4FF0, 0x006D7AC0, 0x006EA870, 0x00749350);   // oCNpc::oCItem* PutInInv( oCItem* ) - Haken PutInInv (E2)
        static const uint oCMobContainer_Insert            = COOP_ENGINE_SWITCH(0x00683E80, 0x006B3A00, 0x006C7F30, 0x00725FC0);   // oCMobContainer::void Insert( oCItem* ) - Haken Truhe einlegen (E1)
        static const uint oCMobContainer_Remove            = COOP_ENGINE_SWITCH(0x00683EB0, 0x006B3A30, 0x006C7F60, 0x00725FF0);   // oCMobContainer::void Remove( oCItem* ) - Haken Truhe entnehmen (E1)
        static const uint oCMobContainer_RemoveN           = COOP_ENGINE_SWITCH(0x00683F40, 0x006B3AB0, 0x006C7FF0, 0x00726080);   // oCMobContainer::oCItem* Remove( oCItem*, int ) - Haken Truhe entnehmen mit Menge (E1)
        static const uint oCNpcInventory_RemoveN           = COOP_ENGINE_SWITCH(0x0066D140, 0x0069A8D0, 0x006AF330, 0x0070CE20);   // oCNpcInventory::oCItem* Remove( oCItem*, int ) - Haken Pluendern mit Menge (E6)
        static const uint oCNpcInventory_Remove            = COOP_ENGINE_SWITCH(0x0066CE00, 0x0069A580, 0x006AF0F0, 0x0070CBE0);   // oCNpcInventory::void Remove( oCItem* ) - Haken Pluendern (E6)
        // oCItemContainer::oCItem* Remove( oCItem*, int ) - Haken Pluendern einer Teilmenge (E6b, u13). G2A geprueft: nimmt das
        // Fenster weniger als den ganzen Stapel, setzt die Funktion nur amount herab (0x00709524) - ohne oCNpcInventory::Remove.
        // G1 nicht geprueft (dort laeuft E6 seit v15) - 0 = kein Haken (SDK-Werte: G1 0x006693D0, G1A 0x00696F30, G2 0x006AB9F0)
        static const uint oCItemContainer_RemoveN          = COOP_ENGINE_SWITCH(0, 0, 0, 0x007094E0);
        static const uint oCMobLockable_CanInteractWith    = COOP_ENGINE_SWITCH(0x00681FC0, 0x006B16C0, 0x006C5C30, 0x00723CC0);   // oCMobLockable::int CanInteractWith( oCNpc* ) - Haken Truhen-Sperre (E1)
        static const uint oCMobInter_OnEndStateChange      = COOP_ENGINE_SWITCH(0x0067F300, 0x006AE410, 0x006C2BF0, 0x00720C80);   // oCMobInter::void OnEndStateChange( oCNpc*, int, int ) - Haken Mechanismus benutzt (P134; CoopWorldSync.cpp)
        static const uint oCTriggerScript_TriggerTarget    = COOP_ENGINE_SWITCH(0x00438A30, 0x0043D420, 0x0043BE90, 0x0043C4C0);   // oCTriggerScript::void TriggerTarget( zCVob* ) - Haken Skript nur beim Benutzer (P134)
        static const uint zCCSCamera_OnTrigger             = COOP_ENGINE_SWITCH(0x004B9E20, 0x004C9170, 0x004C0D00, 0x004C3270);   // zCCSCamera::void OnTrigger( zCVob*, zCVob* ) - Haken Kamerafahrt nur beim Benutzer (P134)
        static const uint zCCSCamera_OnUntrigger           = COOP_ENGINE_SWITCH(0x004BA6F0, 0x004C9A60, 0x004C1550, 0x004C3AC0);   // zCCSCamera::void OnUntrigger( zCVob*, zCVob* ) - ebenso (P134)
        static const uint zCCSCamera_Unarchive             = COOP_ENGINE_SWITCH(0x004B6930, 0x004C5B20, 0x004BD650, 0x004BFBC0);   // zCCSCamera::void Unarchive( zCArchiver& ) - Haken Kamerafahrt im Spielstand stoppen (P136)
        static const uint oCNpc_EV_UseItemToState          = COOP_ENGINE_SWITCH(0x006AFC70, 0x006E3AF0, 0x006F6AC0, 0x007558F0);   // oCNpc::int EV_UseItemToState( oCMsgManipulate* ) - Haken Coop-Inventar direkt (u101, CoopTruhe.cpp)
        static const uint oCItemContainer_TransferItem     = COOP_ENGINE_SWITCH(0x00669780, 0x006972D0, 0x006AC450, 0x00709F40);   // oCItemContainer::int TransferItem( int, int ) - Haken Coop-Inventar nicht in sich selbst (u101)
        static const uint oCNpc_EV_WhirlAround             = COOP_ENGINE_SWITCH(0x0074ED50, 0x00790B90, 0x0079B6F0, 0x00684BF0);   // oCNpc::int EV_WhirlAround( oCMsgMovement* ) - Haken Ueberraschungs-Sprung -> Drehung (C21; Hooks.cpp)
        static const uint oCNpc_OnDamage                   = COOP_ENGINE_SWITCH(0x00730FE0, 0x0076FBF0, 0x0077CE70, 0x006660E0);   // oCNpc::void OnDamage( oSDamageDescriptor& ) - Haken Puppe nimmt keinen Schaden (C11)
        static const uint oCNpc_DropAllInHand              = COOP_ENGINE_SWITCH(0x00694230, 0x006C5A50, 0x006D8EA0, 0x007375E0);   // oCNpc::void DropAllInHand() - Haken (Hooks.cpp)
        static const uint oCNpc_CheckModelOverlays         = COOP_ENGINE_SWITCH(0x0068DA20, 0x006BE2E0, 0x006D1E00, 0x007301D0);   // oCNpc::void CheckModelOverlays() - Haken (Hooks.cpp)
        static const uint zCModel_GetProgressPercent       = COOP_ENGINE_SWITCH(0x005652C0, 0x0057DC40, 0x00579E20, 0x0057F340);   // zCModel::float GetProgressPercent( int ) const - Haken ohne laufende Animation 0 (A17)
        static const uint zCModel_GetProgressPercentName   = COOP_ENGINE_SWITCH(0x00565210, 0x0057DB80, 0x00579D70, 0x0057F290);   // zCModel::float GetProgressPercent( zSTRING const& ) const - Haken ebenso, per Name (A17)
        // oCGuilds::int GetAttitude( int, int ) - Haken negative Gilde neutral (P133, u136). G1 hat dieselbe Luecke, aber dort und in Othello gibt kein
        // Skript einem Raum die Gilde -1 - 0 = kein Haken (SDK-Werte: G1 0x00663170, G1A 0x0068FCD0, G2 0x006A32B0)
        static const uint oCGuilds_GetAttitude             = COOP_ENGINE_SWITCH(0, 0, 0, 0x00700D40);
        static const uint zCCSPlayer_StartCutscene         = COOP_ENGINE_SWITCH(0x0041CD00, 0x0041EC00, 0x0041CF10, 0x0041D240);   // zCCSPlayer::zCCSCutsceneContext* StartCutscene( zSTRING&, zCVob* ) - Haken Player ohne Manager (A19)
        static const uint zCCSPlayer_StartOutputUnit       = COOP_ENGINE_SWITCH(0x0041D2E0, 0x0041F250, 0x0041D520, 0x0041D850);   // zCCSPlayer::zCCSCutsceneContext* StartOutputUnit( int, zCVob* ) - Haken ebenso (A19)
        static const uint zCCSPlayer_CreateNewOutputUnit   = COOP_ENGINE_SWITCH(0x0041D000, 0x0041EF20, 0x0041D210, 0x0041D540);   // zCCSPlayer::zCCSCutsceneContext* CreateNewOutputUnit( int, zCVob* ) - Haken ebenso (A19)
        static const uint oCNpc_States_StartAIState        = COOP_ENGINE_SWITCH(0x006C5490, 0x006FAC30, 0x0070D2E0, 0x0076C840);   // oCNpc_States::int StartAIState( int, int, int, float, int ) - Haken StartAIState(int, ...) (C13/C21; Hooks.cpp)
        static const uint oCNpc_CanUse                     = COOP_ENGINE_SWITCH(0x0068EF00, 0x006BFB50, 0x006D3500, 0x007319B0);   // oCNpc::int CanUse( oCItem* ) - Haken-Reihenfolge (B3; OthelloHookOrder.cpp)

        // Helfer ohne SDK-Eintrag ---------------------------------------------------------------------------------------------
        // Gothics operator new (5-Byte-Sprung + nop): G1 0x0054EBA0 -> 0x0075AD60, G2A 0x00565F50 -> 0x007B4460 (gleiche Form);
        // G2 Classic aus COOP-FIXES.md (ungeprueft)
        static const uint zOperatorNew                     = COOP_ENGINE_SWITCH(0x0054EBA0, 0, 0x00560D30, 0x00565F50);
        // zCArray<int>::AllocDelta(int) - G2A 0x004184E0 gleicher Prolog; G2 Classic aus COOP-FIXES.md (ungeprueft)
        static const uint zCArray_int_AllocDelta           = COOP_ENGINE_SWITCH(0x00417FA0, 0, 0x004184E0, 0x004184E0);
        // zCArray<zCMenuItem*>::AllocDelta(int) - aus zCMenu::SetByScript (G1 Aufruf 0x004CE874, G2A an gleicher Stelle der
        // Aufruffolge 0x004DB464 -> 0x004DE310, gleicher Prolog)
        static const uint zCArray_zCMenuItem_AllocDelta    = COOP_ENGINE_SWITCH(0x004D14A0, 0, 0, 0x004DE310);
        // Aufruf von CreateHit in oCAniCtrl_Human::HitCombo (C11): G1 0x0062791F (HitCombo 0x006276F0 +0x22F, davor
        // mov edx,[esi+1C4h]; push edx), G2A 0x006B0478 (HitCombo 0x006B0260, davor mov ecx,[esi+1C4h])
        static const uint HitCombo_CallCreateHit           = COOP_ENGINE_SWITCH(0x0062791F, 0, 0, 0x006B0478);

        // Daten ohne SDK-Eintrag -----------------------------------------------------------------------------------------
        // zCInput_Win32: Tastenpuffer zCArray<int> (Daten +0, Platz +4, Anzahl +8) - GetKey +0x39 "mov ecx, Puffer"
        // (G1 0x004C88D9, G2A 0x004D53A9); G2 Classic aus COOP-FIXES.md (ungeprueft)
        static const uint zCInput_Win32_keyBuffer          = COOP_ENGINE_SWITCH(0x0086D2DC, 0, 0x008C36F0, 0x008D1D50);
        // Knopfzustand links/rechts - GetMouseButtonPressedLeft/Right lesen genau diese Stelle (G1 0x004C8C30/0x004C8C50,
        // G2A 0x004D5790/0x004D57B0)
        static const uint zCInput_Win32_mouseLeft          = COOP_ENGINE_SWITCH(0x0086CCB8, 0, 0, 0x008D1668);
        // u27: Mausbewegung dieses Bilds (DirectInput, int dx; dy +4) - zCInput_Win32::GetMousePos liest sie (G1 0x004C8BD0, G2A 0x004D5730)
        static const uint zCInput_Win32_mouseDX            = COOP_ENGINE_SWITCH(0x0086CCAC, 0, 0, 0x008D165C);
        static const uint zCInput_Win32_mouseRight         = COOP_ENGINE_SWITCH(0x0086CCC0, 0, 0, 0x008D1670);
        // Fenster, auf das die Eingabe hoert - SetFocusWindow +4 (G1 0x004C8164, G2A 0x004D4A04), GetKey +0x1F
        static const uint zCInput_Win32_focusWindow        = COOP_ENGINE_SWITCH(0x0086D30C, 0, 0, 0x008D1D8C);
        // zCInputCallback::inputList (zCList), erster Knoten (+4) - GetInput "mov esi, [..]" (G1 0x006FB9ED, G2A 0x007A55DA)
        static const uint zCInputCallback_inputListFirst   = COOP_ENGINE_SWITCH(0x008DE1A8, 0, 0, 0x00AB6464);
        // Vorzustand der linken Maustaste in zCInputCallback::GetInput (G1 +0xDD/+0x10C/+0x11D). G2A: GetInput (0x007A55C0,
        // 0x80 Bytes) hat keinen solchen Merker - entfaellt
        static const uint zCInputCallback_prevMouseLeft    = COOP_ENGINE_SWITCH(0x008DE1B0, 0, 0, 0);
        // u8: liefert GetInput Mauscodes an die Menues (0x802 links, 0x804 rechts, 0x809/0x80A Rad)? G1 ja (+0x41 Rad aus
        // GetMousePos z, Schwellen -1/+1, +0xD4 links, rechts danach). G2A nein - dort legt zCInput_Win32::ProcessInputEvents
        // die Maus selbst in den Tastenpuffer (0x004D3E6A Rad 0x20A/0x20B, 0x004D3EF5 links 0x20C, 0x004D3F2E rechts 0x20D, je
        // Druck einmal), und zCMenu::HandleEvent (0x004DBBAB) macht daraus UP/DOWN, RETURN, ESCAPE. G1A/G2 ungeprueft (0).
        static const uint zCInputCallback_mouseCodes       = COOP_ENGINE_SWITCH(1, 0, 0, 0);
        // u8b: zCInput_Win32::GetKey (SDK) - nur G2A gebraucht: dort nimmt GT Gothics Linksklick-Code 0x20C aus den Menues,
        // solange GT die Klicks steuert (wie in G1 der Merker zCInputCallback_prevMouseLeft). G1: 0 = kein Haken.
        static const uint zCInput_Win32_GetKey             = COOP_ENGINE_SWITCH(0, 0, 0, 0x004D5370);
        // zCMenu::FrameCallback (Funktionszeiger) - zCMenu::Render (G1 +0x45 0x004D0DE5, G2A +0x56 0x004DDC76)
        static const uint zCMenu_frameCallback             = COOP_ENGINE_SWITCH(0x0086D3F4, 0, 0, 0x008D1E84);
        // zMDL_NODE_NAME_HEAD (zSTRING "BIP01 HEAD") - oCAIHuman::DoAI "push" (G1 0x00615BE4, G2A 0x0069BD11/0x0069BE1D);
        // G2 Classic aus COOP-FIXES.md (ungeprueft)
        static const uint zMDL_NODE_NAME_HEAD              = COOP_ENGINE_SWITCH(0x00873BF8, 0, 0x008CA388, 0x008D8A08);
        // u28: oCDocumentManager::GetDocumentManager legt den Manager beim ersten Aufruf an (Merker Bit 0, dann ctor auf die
        // Instanz) - GT liest nur und ruft ihn nicht (sonst entstuende er vorzeitig samt Fenster). G1 0x007241D0: Merker
        // 0x009D9580, Instanz 0x009D9584; G2A 0x0065EA40: Merker 0x00AAC3B4, Instanz 0x00AAC3CC
        static const uint oCDocumentManager_initFlag       = COOP_ENGINE_SWITCH(0x009D9580, 0, 0, 0x00AAC3B4);
        static const uint oCDocumentManager_instance       = COOP_ENGINE_SWITCH(0x009D9584, 0, 0, 0x00AAC3CC);

        // Spielstand-Slots ohne Schnellspeicher (u15): G1 15, G2/G2A 20 (Menu_Savegame.d: MENUITEM_LOAD_SLOT1..20, dazu SLOT0)
        static const uint saveSlotCount                    = COOP_ENGINE_SWITCH(15, 15, 20, 20);

        // Feldabstaende ohne richtigen SDK-Namen (u3) --------------------------------------------------------------------
        // zCRnd_D3D: Bildbreite/-hoehe - XD3D_SetMode schreibt x/y/bpp nach +0x984/+0x988/+0x98C (G1 0x0071FFD2) bzw.
        // +0x98C/+0x990/+0x994 (G2A 0x006589F2). Die SDK-Namen liegen in beiden Engines um ein Feld daneben
        // (xd3d_scrStatus = in Wahrheit die Breite). Sonst nehmen Felder den SDK-Namen (tools\sdkfeld.py).
        static const uint off_zCRnd_D3D_scrWidth           = COOP_ENGINE_SWITCH(0x984, 0, 0, 0x98C);
        static const uint off_zCRnd_D3D_scrHeight          = COOP_ENGINE_SWITCH(0x988, 0, 0, 0x990);

        // vtables (SDK VFTable_*.h) ----------------------------------------------------------------------------------------
        static inline uint vt_zCMenu()            { return vftable_zCMenu::GetAddress(); }
        static inline uint vt_zCMenuItemChoice()  { return vftable_zCMenuItemChoice_for_zCViewBase::GetAddress(); }
        static inline uint vt_zCMenuItemText()    { return vftable_zCMenuItemText_for_zCViewBase::GetAddress(); }
        static inline uint vt_oCMenuSavegame()    { return vftable_oCMenuSavegame::GetAddress(); }
        static inline uint vt_oCWorld()           { return vftable_oCWorld::GetAddress(); }
        static inline uint vt_oCBinkPlayer()      { return vftable_oCBinkPlayer::GetAddress(); }   // +0x20 Stop, +0x24 IsPlaying (G1 = G2A)
        static inline uint vt_oCViewDocumentMap() { return vftable_oCViewDocumentMap_for_zCObject::GetAddress(); }   // u28: +0 der Karte (G2A ctor 0x0068D39E)

        // vtable-Plaetze (Byte-Abstand, SDK VFTable_*.h, tools\vtslots.py) - nicht in jeder Engine gleich!
        // oCWorld: in G2/G2A eine Funktion mehr vor RemoveVob
        static const uint vtslot_oCWorld_RemoveVob         = COOP_ENGINE_SWITCH(0x30, 0x30, 0x34, 0x34);
        static const uint vtslot_oCWorld_DisableVob        = COOP_ENGINE_SWITCH(0x70, 0x70, 0x74, 0x74);
        // Menueeintraege (zCMenuItem und Ableitungen, Basis zCView): in G2/G2A ab +0x28 alles um 4 weiter
        static const uint vtslot_zCMenuItem_DrawItems      = COOP_ENGINE_SWITCH(0x28, 0x28, 0x2C, 0x2C);
        static const uint vtslot_zCMenuItem_InitMenuItem   = COOP_ENGINE_SWITCH(0x2C, 0x2C, 0x30, 0x30);
        static const uint vtslot_zCMenuItem_ToggleValue    = COOP_ENGINE_SWITCH(0x30, 0x30, 0x34, 0x34);
        static const uint vtslot_zCMenuItem_SetText        = COOP_ENGINE_SWITCH(0x40, 0x40, 0x44, 0x44);   // SetText(zSTRING const&, int, int)
        static const uint vtslot_zCMenuItem_DrawFront      = COOP_ENGINE_SWITCH(0x58, 0x58, 0x5C, 0x5C);
        // In allen vier Engines gleich (SDK geprueft): zCMenu HandleEvent +0, HandleEnterItem +0x04, HandleLeaveItem +0x08,
        // HandleSelAction +0x3C, SetByScript +0x40; oCMenuSavegame HandleSlotChange +0x48; oCBinkPlayer Stop +0x20, IsPlaying
        // +0x24; zCSoundSystem PlaySound(zCSoundFX*, int) +0x28 - dort bleiben die Zahlen im Code stehen.
    }
}
