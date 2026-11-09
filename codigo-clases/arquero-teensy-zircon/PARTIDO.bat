@echo off
rem PARTIDO.bat - abre el TABLERO directo en la pestana PARTIDO: ver y cambiar
rem los valores del programa de partido (funciona\partido-0921) y cargarlo al
rem robot con el boton verde, que avisa antes. [2026-11-08]
rem Esto NO carga nada solo. Sin robot tambien abre (para cambiar valores).
setlocal
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
set "PYW=%USERPROFILE%\.platformio\penv\Scripts\pythonw.exe"
set "TABLERO=%~dp0pruebas\tablero"
if exist "%PYW%" (
  start "" "%PYW%" "%TABLERO%\tablero.py" --pestana partido
) else if exist "%PY%" (
  start "Tablero del arquero" "%PY%" "%TABLERO%\tablero.py" --pestana partido
) else (
  echo   XXX  No encuentro el Python de PlatformIO en:
  echo        "%PY%"
  pause
  exit /b 1
)
exit /b 0
