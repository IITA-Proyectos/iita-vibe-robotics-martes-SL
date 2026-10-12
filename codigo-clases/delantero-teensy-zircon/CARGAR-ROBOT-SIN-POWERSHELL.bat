@echo off
rem CARGAR-ROBOT-SIN-POWERSHELL.bat - lo mismo que CARGAR-ROBOT.bat, para una
rem PC donde Windows PowerShell no arranca (doble clic en CARGAR-ROBOT.bat y
rem sale "Se han producido uno o varios errores").
rem
rem Doble clic y elegir 1 (ARCO AZUL) o 2 (ARCO AMARILLO). Todo lo hace
rem robot-sin-powershell.py, que esta al lado, con el Python de PlatformIO.
rem Tambien: CARGAR-ROBOT-SIN-POWERSHELL.bat azul
rem
rem En una PC sana se sigue usando CARGAR-ROBOT.bat, que es el de siempre.
"%USERPROFILE%\.platformio\penv\Scripts\python.exe" "%~dp0robot-sin-powershell.py" cargar %*
set RC=%ERRORLEVEL%
if "%~1"=="" pause
exit /b %RC%
