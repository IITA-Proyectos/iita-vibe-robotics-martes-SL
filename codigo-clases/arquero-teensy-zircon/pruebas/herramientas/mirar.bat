@echo off
REM Monitor del robot en vivo, para abrir con doble clic o desde cmd.
REM Muestra todo lo que dice el robot y deja escribirle teclas.
REM Se cierra con Ctrl+C.

chcp 65001 >nul
setlocal

REM El Python del taller: viene adentro de PlatformIO y tiene pyserial.
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"

if not exist "%PY%" (
    echo No encontre el Python del taller en:
    echo   %PY%
    echo Probando con el python del sistema...
    set "PY=python"
)

"%PY%" "%~dp0mirar.py" %*

echo.
pause
