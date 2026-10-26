@echo off
setlocal
rem GothicCoop - Coop-GD3D11 coop3 installieren, 19.10.2026
rem Liegt in system\Autorun\GD3D11-coop3\ und kopiert g1_avx2.dll.coop3 nach system\GD3D11\Bin\g1_avx2.dll.
rem Sicherung vorher: ein Renderer ohne Coop-Zweitbild wird g1_avx2.dll.original, solange es die noch nicht gibt;
rem eine aeltere Coop-Fassung oder ein spaeteres Update wird g1_avx2.dll.vorher.
rem Ohne Klammerbloecke mit Pfaden, damit auch Pfade wie "Program Files (x86)" gehen.
set "SRC=%~dp0g1_avx2.dll.coop3"
set "BIN=%~dp0..\..\GD3D11\Bin"
set "DLL=%BIN%\g1_avx2.dll"
set "HASH_NEU=23380e15ece8a63ec88609fb0066ec78dc1171faa80f410f77785214bc2d43d3"
echo.
echo Coop-GD3D11 coop3 installieren
echo Ziel: %DLL%
echo.
tasklist /FI "IMAGENAME eq GothicMod.exe" 2>nul | find /I "GothicMod.exe" >nul
if not errorlevel 1 goto laeuft
if not exist "%SRC%" goto fehltquelle
if not exist "%BIN%\" goto fehltziel
if exist "%DLL%" goto pruefen
if exist "%BIN%\g1_avx2.dll.original" goto kopieren
goto fehltziel

:pruefen
certutil -hashfile "%DLL%" SHA256 | find /I "%HASH_NEU%" >nul
if not errorlevel 1 goto schonda
findstr /M /C:"GDX_CoopSecondView" "%DLL%" >nul
if not errorlevel 1 goto sichernvorher
if exist "%BIN%\g1_avx2.dll.original" goto sichernvorher
copy /Y "%DLL%" "%BIN%\g1_avx2.dll.original" >nul
if errorlevel 1 goto fehler
echo Original gesichert: g1_avx2.dll.original
goto kopieren

:sichernvorher
copy /Y "%DLL%" "%BIN%\g1_avx2.dll.vorher" >nul
if errorlevel 1 goto fehler
echo Bisherige Fassung gesichert: g1_avx2.dll.vorher

:kopieren
copy /Y "%SRC%" "%DLL%" >nul
if errorlevel 1 goto fehler
certutil -hashfile "%DLL%" SHA256 | find /I "%HASH_NEU%" >nul
if errorlevel 1 goto pruefsumme
echo Fertig: coop3 ist installiert, Pruefsumme stimmt.
echo Im Coop-Log steht nach dem naechsten Start: Renderer g1_avx2.dll mit GDX_CoopSecondView.
goto ende

:schonda
echo coop3 ist schon installiert - nichts zu tun.
goto ende
:laeuft
echo Gothic laeuft noch - bitte zuerst beenden und dann noch einmal starten.
goto ende
:fehltquelle
echo Datei fehlt: %SRC%
goto ende
:fehltziel
echo GD3D11-Ordner oder g1_avx2.dll nicht gefunden: %BIN%
echo Liegt dieser Ordner in system\Autorun\ des Gothic-Ordners?
goto ende
:pruefsumme
echo WARNUNG: Pruefsumme der neuen g1_avx2.dll passt nicht.
goto ende
:fehler
echo Kopieren fehlgeschlagen - Schreibrechte im Gothic-Ordner pruefen.
:ende
echo.
pause
