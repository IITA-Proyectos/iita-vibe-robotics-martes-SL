@echo off
REM ===================================================================
REM  MONITOR DEL ROBOT ARQUERO — abrir con doble clic
REM  IITA Salta — taller de los martes — Roboliga 2026
REM ===================================================================
REM  Abre el tablero del robot: giroscopio, sensores de luz y camara,
REM  en una pantalla que se queda quieta y se refresca en su lugar.
REM
REM  Antes de abrir esto:
REM    1) prender la bateria      (el giroscopio y la camara comen de ahi)
REM    2) enchufar el USB         (en ese orden, si no el giroscopio no
REM                                aparece aunque este perfecto)
REM
REM  Se cierra con Ctrl+C.
REM ===================================================================

title Monitor del robot arquero

REM Acentos y simbolos.
chcp 65001 >nul

REM El tablero ocupa 26 renglones. Si la ventana es mas chica, se corta y
REM se ve temblando. Esto la agranda antes de empezar.
mode con: cols=90 lines=32

setlocal

REM El Python del taller: viene adentro de PlatformIO y trae pyserial.
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"

if not exist "%PY%" (
    echo No encontre el Python del taller en:
    echo   %PY%
    echo Probando con el python del sistema...
    set "PY=python"
)

REM El monitor vive en pruebas\herramientas\ y se comparte con las demas
REM herramientas. Este .bat solo lo abre con la ventana del tamano justo.
"%PY%" "%~dp0..\herramientas\mirar.py" %*

echo.
echo --- cerrado ---
pause
