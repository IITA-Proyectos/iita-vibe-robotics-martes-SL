@echo off
rem CARGAR-ROBOT.bat - carga en el DELANTERO el programa del arco que toque.
rem Doble clic y elegir 1 (ARCO AZUL) o 2 (ARCO AMARILLO). Todo lo hace
rem cargar-robot.ps1, que esta al lado. Tambien: CARGAR-ROBOT.bat azul
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0cargar-robot.ps1" %*
set RC=%ERRORLEVEL%
if "%~1"=="" pause
exit /b %RC%
