@echo off
REM ===================================================================
REM  CARGAR LA TECNICA CHALLENGE (el recorrido de los arcos) — doble clic
REM  IITA Salta — taller de los martes — Roboliga 2026 — 2026-11-08
REM ===================================================================
REM  🚨 ES UNA PRUEBA, NO EL PROGRAMA DE PARTIDO. Para jugar hay que
REM     volver a cargar la 0921f (funciona/partido-0921).
REM
REM  🚨 EL ROBOT ARRANCA SOLO apenas termina la carga.
REM     Cargar con la bateria APAGADA (sin bateria no hay giroscopio y
REM     el robot no se mueve: queda temblando el LED).
REM
REM  Despues, para probarlo: desenchufar el USB, apoyarlo MIRANDO A UN
REM  ARCO, en la parte del medio de la cancha (fuera de las areas y no
REM  sobre una linea), y recien ahi prender la bateria.
REM
REM  Para ver lo que piensa: pruebas\herramientas\mirar.bat
REM  (tecla 0 = parar, r = empezar de nuevo, i = informe, ? = ayuda)
REM ===================================================================

title Cargar la tecnica challenge
chcp 65001 >nul

setlocal
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
if not exist "%PY%" set "PY=python"

"%PY%" "%~dp0..\herramientas\cargar.py" "%~dp0."

echo.
pause
