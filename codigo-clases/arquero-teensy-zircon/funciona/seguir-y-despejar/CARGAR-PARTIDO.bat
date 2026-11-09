@echo off
REM ===================================================================
REM  CARGAR EL PROGRAMA DE PARTIDO — doble clic
REM  IITA Salta — taller de los martes — Roboliga 2026
REM ===================================================================
REM  🚨 EL ROBOT ARRANCA SOLO apenas termina la carga.
REM     Bateria APAGADA, o el robot en el piso de la cancha.
REM
REM  Despues, para jugar: desenchufar el USB, apoyarlo quieto en la
REM  cancha y recien ahi prender la bateria.
REM ===================================================================

title Cargar la 0.1103 (seguir-y-despejar) - NO es la de jugar
chcp 65001 >nul

REM 2026-11-08: desde el 03/11 se juega con funciona/partido-0921. Para
REM cargar ESA, doble clic en PARTIDO.bat (carpeta arquero-teensy-zircon).
echo.
echo  ============================================================
echo   OJO: esto carga la 0.1103 (seguir-y-despejar).
echo   Desde el 03/11 se JUEGA con funciona\partido-0921.
echo   Para cargar la de jugar: PARTIDO.bat (arquero-teensy-zircon).
echo  ============================================================
echo   Si igual queres cargar la 0.1103, apreta una tecla.
echo   Si no, cerra esta ventana.
pause

setlocal
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
if not exist "%PY%" set "PY=python"

"%PY%" "%~dp0..\..\pruebas\herramientas\cargar.py" "%~dp0."

echo.
pause
