@echo off
rem MEDIR-ROBOT-SIN-POWERSHELL.bat - lo mismo que MEDIR-TUTI-ROBOT.bat, para
rem una PC donde Windows PowerShell no arranca.
rem
rem Doble clic: menu con las pruebas, L para LEER lo que el robot tiene para
rem decir (lo guarda en mediciones\) y V para ver que robot hay enchufado.
rem Todo lo hace robot-sin-powershell.py, con el Python de PlatformIO.
rem
rem NO es el de partido. Para jugar: CARGAR-ROBOT-SIN-POWERSHELL.bat.
"%USERPROFILE%\.platformio\penv\Scripts\python.exe" "%~dp0robot-sin-powershell.py" medir %*
set RC=%ERRORLEVEL%
if "%~1"=="" pause
exit /b %RC%
