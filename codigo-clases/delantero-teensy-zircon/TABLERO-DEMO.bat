@echo off
rem TABLERO-DEMO.bat - abre la ventana del tablero con un ROBOT DE MENTIRA,
rem para verla y practicar SIN el robot. [2026-10-27]
rem
rem No carga nada ni abre el USB. Todo lo que "mide" es inventado: sirve
rem para aprender a usar la ventana, no para ajustar numeros.
rem Con el robot de verdad: TABLERO.bat.
setlocal
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
set "PYW=%USERPROFILE%\.platformio\penv\Scripts\pythonw.exe"
set "TABLERO=%~dp0pruebas\tablero"

if exist "%PYW%" (
  start "" "%PYW%" "%TABLERO%\tablero.py" --demo
) else if exist "%PY%" (
  start "Tablero de mentira" "%PY%" "%TABLERO%\tablero.py" --demo
) else (
  echo   XXX  No encuentro el Python de PlatformIO en:
  echo        "%PY%"
  pause
  exit /b 1
)
exit /b 0
