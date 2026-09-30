@echo off
REM ===================================================================
REM  CUANDO SE CAE — correr la prueba varias veces, con doble clic
REM  IITA Salta — taller de los martes — Roboliga 2026
REM ===================================================================
REM  Le sube la potencia a los motores de a poco y anota en que PWM se
REM  muere el giroscopio. Lo hace varias veces y al final dice si el
REM  umbral se repite (corriente) o esta desparramado (vibracion).
REM
REM  🚨 LAS RUEDAS GIRAN: el robot tiene que estar sujeto con cinta,
REM     levantado, o agarrado con la mano.
REM  🚨 La bateria prendida, y prendida ANTES de enchufar el USB.
REM
REM  Se corta con Ctrl+C.
REM ===================================================================

title Cuando se cae - prueba del giroscopio
chcp 65001 >nul
mode con: cols=80 lines=45

setlocal
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
if not exist "%PY%" set "PY=python"

REM Sin numero son 3 vueltas. Para otra cantidad: PROBAR.bat 5
"%PY%" "%~dp0probar.py" %*

echo.
pause
