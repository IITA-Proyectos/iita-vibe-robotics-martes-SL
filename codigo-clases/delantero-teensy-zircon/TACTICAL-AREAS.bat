@echo off
rem TACTICAL-AREAS.bat - carga en el DELANTERO el desafio del 13/11
rem tactical-areas\ y abre la consola para ver que hace. [2026-11-13]
rem
rem El desafio: 1) ENTRAR al area AMARILLA  2) ENTRAR al area AZUL y quieto.
rem Siempre en ese orden. Arranca solo a los 2 s de prenderlo.
rem Al ver la linea del area sigue derecho un poco y frena adentro; del area
rem amarilla sale para atras. El Tactical del 08/11 (TACTICAL-CHALLENGE.bat)
rem no se toco.
rem
rem NO TOCA el programa principal (funciona\delantero). Pero el robot queda
rem con el DESAFIO cargado: para jugar hay que volver a cargar el de partido
rem con CARGAR-ROBOT.bat (1 azul / 2 amarillo).
setlocal
set "PIO=%USERPROFILE%\.platformio\penv\Scripts\pio.exe"
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
set "LOADER=%USERPROFILE%\.platformio\packages\tool-teensy\teensy.exe"
set "PROGRAMA=%~dp0tactical-areas"

echo ============================================================
echo    DELANTERO  -  TACTICAL AREAS  (13/11)
echo    1) ENTRAR al area AMARILLA   2) ENTRAR al area AZUL y quieto
echo ============================================================
echo    NO es el programa de partido. Despues, para jugar, volver
echo    a cargar con CARGAR-ROBOT.bat.
echo.
echo    Que robot hay enchufado:
"%PY%" "%~dp0robot-sin-powershell.py" ver
echo.
echo    Tiene que ser SOLO el delantero. Si no lo es, cerra esta ventana.
pause

tasklist | find /i "teensy.exe" >nul || start "" "%LOADER%"
timeout /t 2 >nul

"%PIO%" run -d "%PROGRAMA%" -t upload
if errorlevel 1 (
  echo.
  echo   XXX  LA CARGA FALLO. El robot sigue con el programa que tenia.
  pause
  exit /b 1
)

echo.
echo ============================================================
echo    Cargado. Ahora se abre la consola del robot.
echo    Tiene que aparecer el cartel  TACTICAL AREAS - DELANTERO.
echo    Si NO aparece, la carga no entro: no confiar en el SUCCESS.
echo.
echo    Para repetir el desafio: apagar y prender el robot.
echo    Para salir: Ctrl + C
echo ============================================================
timeout /t 3 >nul
"%PIO%" device monitor -d "%PROGRAMA%"
pause
