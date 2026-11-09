@echo off
rem TABLERO-DEMO.bat - abre la ventana del tablero del ARQUERO con un ROBOT
rem DE MENTIRA, para verla y practicar SIN el robot. [2026-11-03]
rem
rem No carga nada ni abre el USB. Todo lo que "mide" es inventado (las
rem mediciones de CANCHA dan los numeros de las bitacoras, y SIN CABLE se
rem desenchufa y se enchufa solo): sirve para aprender a usar la ventana,
rem no para ajustar numeros.
rem Con el robot de verdad: TABLERO.bat.
setlocal
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
set "PYW=%USERPROFILE%\.platformio\penv\Scripts\pythonw.exe"
set "TABLERO=%~dp0pruebas\tablero"

if exist "%PYW%" (
  start "" "%PYW%" "%TABLERO%\tablero.py" --demo
) else if exist "%PY%" (
  start "Tablero de mentira (arquero)" "%PY%" "%TABLERO%\tablero.py" --demo
) else (
  echo   XXX  No encuentro el Python de PlatformIO en:
  echo        "%PY%"
  pause
  exit /b 1
)
exit /b 0
