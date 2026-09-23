@echo off
rem MEDIR-ROBOT.bat - carga en el DELANTERO un programa de MEDICION.
rem Doble clic y elegir del menu. Todo lo hace medir-robot.ps1, al lado.
rem
rem NO es el de partido. Para jugar: CARGAR-ROBOT.bat (1 azul / 2 amarillo).
rem Ninguno de estos programas juega: solo miden.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0medir-robot.ps1" %*
set RC=%ERRORLEVEL%
if "%~1"=="" pause
exit /b %RC%
