@echo off
rem PROBAR-SAQUE.bat - carga en el DELANTERO la prueba pruebas\saque-rebote
rem y abre la consola. [2026-11-08]
rem
rem Al prenderse hace UN saque: golpe derecho para donde mira (el giroscopio
rem lo mantiene derecho), retrocede y se queda quieto. El robot se apunta A
rem MANO hacia la pared, con la pelota adelante, para que rebote al arco.
rem Para repetir: apagar y prender, o la tecla p en la consola.
rem
rem NO TOCA el programa principal (funciona\delantero). Pero el robot queda
rem con la PRUEBA cargada: para jugar hay que volver a cargar el de partido
rem con CARGAR-ROBOT.bat (1 azul / 2 amarillo).
setlocal
set "PIO=%USERPROFILE%\.platformio\penv\Scripts\pio.exe"
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
set "LOADER=%USERPROFILE%\.platformio\packages\tool-teensy\teensy.exe"
set "PRUEBA=%~dp0pruebas\saque-rebote"

echo ============================================================
echo    DELANTERO  -  PRUEBA DEL SAQUE CON REBOTE
echo ============================================================
echo    NO es el programa de partido. Despues, para jugar, volver
echo    a cargar con CARGAR-ROBOT.bat.
echo.
echo    OJO: a los ~5 s de cargarse PATEA. Robot en el PISO, con
echo    lugar adelante y el cable flojo.
echo.
echo    Que robot hay enchufado:
"%PY%" "%~dp0robot-sin-powershell.py" ver
echo.
echo    Tiene que ser SOLO el delantero. Si no lo es, cerra esta ventana.
pause

tasklist | find /i "teensy.exe" >nul || start "" "%LOADER%"
timeout /t 2 >nul

"%PIO%" run -d "%PRUEBA%" -t upload
if errorlevel 1 (
  echo.
  echo   XXX  LA CARGA FALLO. El robot sigue con el programa que tenia.
  pause
  exit /b 1
)

echo.
echo ============================================================
echo    Cargado. Ahora se abre la consola del robot.
echo    Tiene que aparecer el cartel  SAQUE CON REBOTE - DELANTERO.
echo    Si NO aparece, la carga no entro: no confiar en el SUCCESS.
echo.
echo    p = otro saque     Para salir: Ctrl + C
echo ============================================================
timeout /t 3 >nul
"%PIO%" device monitor -d "%PRUEBA%"
pause
