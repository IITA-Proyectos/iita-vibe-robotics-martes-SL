@echo off
rem PROBAR-PATADA.bat - carga en el DELANTERO la prueba pruebas\patada-derecha
rem y abre la consola para manejarla con teclas. [2026-10-13]
rem
rem La prueba no busca la pelota: espera teclas. Con "p" hace el golpe y el
rem retroceso como en el partido y dice cuanto se torcio. Los numeros de la
rem patada se cambian con teclas, sin recargar.
rem
rem NO TOCA el programa principal (funciona\delantero). Pero el robot queda
rem con la PRUEBA cargada: al terminar hay que volver a cargar el de partido
rem con CARGAR-ROBOT.bat (1 azul / 2 amarillo).
setlocal
set "PIO=%USERPROFILE%\.platformio\penv\Scripts\pio.exe"
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
set "LOADER=%USERPROFILE%\.platformio\packages\tool-teensy\teensy.exe"
set "PRUEBA=%~dp0pruebas\patada-derecha"

echo ============================================================
echo    DELANTERO  -  PRUEBA DE PATADA  (golpe + retroceso)
echo ============================================================
echo    NO es el programa de partido. Despues de probar, volver a
echo    cargar con CARGAR-ROBOT.bat.
echo.
echo    Robot en el PISO, con lugar adelante y el cable flojo.
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
echo    Tiene que aparecer el cartel  PATADA DERECHA - golpe + retroceso.
echo    Si NO aparece, la carga no entro: no confiar en el SUCCESS.
echo.
echo    p = patear     ? = ayuda     Para salir: Ctrl + C
echo ============================================================
timeout /t 3 >nul
"%PIO%" device monitor -d "%PRUEBA%"
pause
