@echo off
REM ===================================================================
REM  AJUSTAR LOS MOVIMIENTOS — doble clic
REM  IITA Salta — taller de los martes — Roboliga 2026
REM ===================================================================
REM  Carga la herramienta de ajuste al arquero y abre la consola.
REM
REM  Antes:
REM    1) bateria PRENDIDA (el giroscopio y los motores comen de ahi)
REM    2) despues el USB
REM    3) el robot en el piso de la cancha: SE MUEVE al apretar 1..7
REM
REM  Al terminar: CARGAR-PARTIDO.bat, si no el robot no juega.
REM  Si se corta la consola y la herramienta ya esta cargada: MIRAR.bat.
REM ===================================================================

title Ajustar movimientos del arquero
chcp 65001 >nul

setlocal
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
if not exist "%PY%" set "PY=python"

"%PY%" "%~dp0..\herramientas\cargar.py" "%~dp0." --mirar

echo.
pause
