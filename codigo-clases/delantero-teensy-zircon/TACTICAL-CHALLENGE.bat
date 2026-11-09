@echo off
rem TACTICAL-CHALLENGE.bat - carga en el DELANTERO el programa del desafio
rem tactical-challenge\ y abre la consola para ver que hace. [2026-11-08]
rem
rem El desafio: 1) arco AMARILLO  2) arco AZUL  3) tocar la PELOTA.
rem Siempre en ese orden. Arranca solo a los 2 s de prenderlo.
rem VERSION APROBADA el 08/11: en cada arco frena en CUALQUIER linea (despues
rem de haber visto ese arco), espera 1 s y se despega. La pelota la toca despacio.
rem Este .bat compila SIEMPRE lo que hay en tactical-challenge\ (no guarda copias).
rem
rem NO TOCA el programa principal (funciona\delantero). Pero el robot queda
rem con el DESAFIO cargado: para jugar hay que volver a cargar el de partido
rem con CARGAR-ROBOT.bat (1 azul / 2 amarillo).
setlocal
set "PIO=%USERPROFILE%\.platformio\penv\Scripts\pio.exe"
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
set "LOADER=%USERPROFILE%\.platformio\packages\tool-teensy\teensy.exe"
set "PROGRAMA=%~dp0tactical-challenge"

echo ============================================================
echo    DELANTERO  -  TACTICAL CHALLENGE
echo    1) arco AMARILLO   2) arco AZUL   3) tocar la PELOTA
echo    Version APROBADA 08/11: frena en la linea de cada arco
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
echo    Tiene que aparecer el cartel  TACTICAL CHALLENGE - DELANTERO.
echo    Si NO aparece, la carga no entro: no confiar en el SUCCESS.
echo.
echo    Para repetir el desafio: apagar y prender el robot.
echo    Para salir: Ctrl + C
echo ============================================================
timeout /t 3 >nul
"%PIO%" device monitor -d "%PROGRAMA%"
pause
