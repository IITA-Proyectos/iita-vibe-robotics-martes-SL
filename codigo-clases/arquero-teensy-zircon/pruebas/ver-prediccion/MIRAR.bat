@echo off
REM ===================================================================
REM  VER-PREDICCION — mirar si la prediccion de la pelota acierta
REM  IITA Salta — taller de los martes — Roboliga 2026
REM ===================================================================
REM  El robot NO se mueve. Se apoya en la mesa y se hace rodar la pelota
REM  cruzando el campo de vision de la camara.
REM
REM  🎯 LA PRUEBA DEL SIGNO: rodando la pelota de IZQUIERDA a DERECHA, la
REM     P (prediccion) tiene que quedar a la DERECHA de la o (donde esta).
REM     Si queda a la izquierda, la prediccion apunta DETRAS de la pelota.
REM
REM  🚨 La bateria prendida (la camara come de ahi), y prendida ANTES del USB.
REM
REM  Se cierra con Ctrl+C.
REM ===================================================================

title Ver prediccion de la pelota
chcp 65001 >nul
mode con: cols=90 lines=32

setlocal
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
if not exist "%PY%" set "PY=python"

"%PY%" "%~dp0..\herramientas\mirar.py" %*

echo.
pause
