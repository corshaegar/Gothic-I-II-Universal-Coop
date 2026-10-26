@echo off
setlocal
rem GothicCoop - Original-GD3D11 wiederherstellen: g1_avx2.dll.original zurueck nach g1_avx2.dll, 19.10.2026
rem Ohne Klammerbloecke mit Pfaden, damit auch Pfade wie "Program Files (x86)" gehen.
set "BIN=%~dp0..\..\GD3D11\Bin"
echo.
tasklist /FI "IMAGENAME eq GothicMod.exe" 2>nul | find /I "GothicMod.exe" >nul
if not errorlevel 1 goto laeuft
if not exist "%BIN%\g1_avx2.dll.original" goto keinesicherung
copy /Y "%BIN%\g1_avx2.dll.original" "%BIN%\g1_avx2.dll" >nul
if errorlevel 1 goto fehler
echo Original-GD3D11 ist wieder aktiv.
goto ende
:laeuft
echo Gothic laeuft noch - bitte zuerst beenden.
goto ende
:keinesicherung
echo Keine Sicherung g1_avx2.dll.original gefunden in: %BIN%
goto ende
:fehler
echo Kopieren fehlgeschlagen - Schreibrechte im Gothic-Ordner pruefen.
:ende
echo.
pause
