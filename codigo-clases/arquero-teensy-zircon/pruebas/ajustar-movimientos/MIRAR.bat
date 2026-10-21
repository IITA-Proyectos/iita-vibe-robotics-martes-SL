@echo off
REM ===================================================================
REM  Solo abrir la consola, SIN volver a cargar nada.
REM  Para cuando la herramienta ya esta cargada (por ejemplo, despues de
REM  una prueba sin cable: se vuelve a enchufar y se aprieta r).
REM ===================================================================

title Ajustar movimientos del arquero
chcp 65001 >nul

setlocal
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
if not exist "%PY%" set "PY=python"

"%PY%" "%~dp0..\herramientas\mirar.py" %*

echo.
pause
