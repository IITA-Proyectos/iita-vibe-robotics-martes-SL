@echo off
REM ===================================================================
REM  VELOCIDAD DE GIRO — ¿a que fuerza empieza a girar sobre su eje?
REM  IITA Salta — taller de los martes — Roboliga 2026
REM ===================================================================
REM  Arranca PARADO. Con las teclas subis y bajas la fuerza, y el
REM  programa mide con el giroscopio cuantos grados por segundo gira
REM  de verdad. Lleva solo la cuenta del MINIMO al que giro.
REM
REM    espacio  girar / parar      s / a  fuerza +-5
REM    d        sentido            S / A  fuerza +-1
REM    0        borrar minimos     ?      ayuda
REM
REM  🚨 Gira sobre su eje, NO se traslada. Mejor en el piso de la cancha:
REM     el roce de la mesa es otro y el numero no serviria.
REM  🚨 La bateria prendida, y prendida ANTES del USB.
REM
REM  Se cierra con Ctrl+C.
REM ===================================================================

title Velocidad de giro sobre el eje
chcp 65001 >nul
mode con: cols=80 lines=30

setlocal
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
if not exist "%PY%" set "PY=python"

"%PY%" "%~dp0..\herramientas\mirar.py" %*

echo.
pause
