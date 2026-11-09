@echo off
rem PARTIDO-CON-SAQUE.bat - carga en el DELANTERO el PARTIDO CON SAQUE: al
rem arrancar rodea la pelota, patea fuerte contra la pared para que rebote al
rem arco, y despues juega normal. [2026-11-09]
rem Doble clic y elegir 1 (ARCO AZUL) o 2 (ARCO AMARILLO). Todo lo hace
rem cargar-partido-con-saque.ps1, que esta al lado. Tambien: PARTIDO-CON-SAQUE.bat azul
rem El de partido sin saque sigue siendo CARGAR-ROBOT.bat (no se toco).
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0cargar-partido-con-saque.ps1" %*
set RC=%ERRORLEVEL%
if "%~1"=="" pause
exit /b %RC%
