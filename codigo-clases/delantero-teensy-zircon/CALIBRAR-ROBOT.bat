@echo off
rem CALIBRAR-ROBOT.bat - el DELANTERO en una cancha que no conocemos.
rem Doble clic. Te dice que medir y en que orden, y lo carga.
rem
rem Los tres .bat, para no confundirse:
rem   CARGAR-ROBOT.bat     el PARTIDO (arco fijo azul o amarillo)
rem   MEDIR-ROBOT.bat      las pruebas de a una, para investigar
rem   CALIBRAR-ROBOT.bat   ESTE: cancha nueva, que medir y en que orden
rem
rem Todo lo hace calibrar-robot.ps1, que esta al lado.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0calibrar-robot.ps1" %*
set RC=%ERRORLEVEL%
if "%~1"=="" pause
exit /b %RC%
