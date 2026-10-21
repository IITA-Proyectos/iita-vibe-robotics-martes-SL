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

title Cargar el programa de partido
chcp 65001 >nul

setlocal
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
if not exist "%PY%" set "PY=python"

"%PY%" "%~dp0..\..\pruebas\herramientas\cargar.py" "%~dp0."

echo.
pause
