@echo off
rem TABLERO.bat - la VENTANA del DELANTERO: ver la pelota y los sensores, y
rem probar y ajustar los movimientos con botones, sin escribir comandos.
rem [2026-10-27]
rem
rem   1. Mira que robot hay enchufado. Si NO es el delantero, si no hay
rem      ninguno o si no se sabe cual es, no sigue: pide enchufar SOLO el
rem      delantero y vuelve a mirar.
rem   2. Pregunta si carga el programa del tablero (pruebas\tablero).
rem      Enter, S o SI = carga.  N o NO = no carga.  Otra cosa = pregunta
rem      de nuevo. Si el robot YA lo tiene cargado, contestar N.
rem   3. Justo antes de cargar, mira OTRA VEZ que siga siendo el delantero.
rem   4. Abre la ventana (pruebas\tablero\tablero.py).
rem
rem NO TOCA el programa principal (funciona\delantero). Pero el robot queda
rem con el TABLERO cargado: al terminar hay que volver a cargar el de partido
rem con CARGAR-ROBOT.bat (1 azul / 2 amarillo).
rem
rem Sin robot: TABLERO-DEMO.bat abre la misma ventana con un robot de mentira.
setlocal
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
set "PYW=%USERPROFILE%\.platformio\penv\Scripts\pythonw.exe"
set "PIO=%USERPROFILE%\.platformio\penv\Scripts\pio.exe"
set "LOADER=%USERPROFILE%\.platformio\packages\tool-teensy\teensy.exe"
set "TABLERO=%~dp0pruebas\tablero"
set "VER=%TEMP%\tablero-que-robot.txt"

echo ============================================================
echo    DELANTERO  -  TABLERO  (la ventana con botones)
echo ============================================================
echo    NO es el programa de partido. Despues de probar, volver a
echo    cargar con CARGAR-ROBOT.bat.
echo.
if not exist "%PY%" (
  echo   XXX  No encuentro el Python de PlatformIO en:
  echo        "%PY%"
  pause
  exit /b 1
)

rem Solo se sigue si "ver" dice que el enchufado es el DELANTERO. El tablero
rem tiene los pines del delantero, y el cargador no elige a cual le carga.
:verificar
call :mirar
if not "%QUIEN%"=="DELANTERO" goto noconfirmado

echo    Si el robot YA tiene el tablero cargado, contesta N.
echo    Si hay una ventana del tablero abierta, cerrala ANTES de cargar:
echo    con la ventana abierta el cargador no puede hablar con el robot.
:preguntar
set "RESP="
set /p "RESP=   Cargar el programa del tablero al robot? (S/N) [S]: "
rem Enter solo = S. Las comillas se borran para que no rompan los if.
if not defined RESP goto cargar
set "RESP=%RESP:"=%"
if /i "%RESP%"=="S" goto cargar
if /i "%RESP%"=="SI" goto cargar
if /i "%RESP%"=="N" goto abrir
if /i "%RESP%"=="NO" goto abrir
echo    No entendi. Contesta S (o Enter) para cargar, o N para no cargar.
goto preguntar

:cargar
echo.
rem Mientras esperaba la respuesta pudieron cambiar el robot: se mira de nuevo.
echo    Antes de cargar, miro de nuevo:
call :mirar
if not "%QUIEN%"=="DELANTERO" goto noconfirmado
echo    Cargando (la primera vez tarda ~1 min, despues ~10 s)...
tasklist | findstr /i "teensy.exe" >nul || start "" "%LOADER%"
timeout /t 2 >nul

"%PIO%" run -d "%TABLERO%" -t upload
if errorlevel 1 (
  echo.
  echo   XXX  LA CARGA FALLO. El robot sigue con el programa que tenia.
  echo        No abro la ventana. Si habia una ventana del tablero
  echo        abierta, cerrala y volve a abrir TABLERO.bat.
  pause
  exit /b 1
)

:abrir
echo.
echo ============================================================
echo    Abriendo la ventana del tablero...
echo.
echo    Arriba tiene que decir  DELANTERO  y  "Programa del tablero
echo    cargado". Si dice que el robot no contesta como tablero, la
echo    carga no entro: no confiar en el SUCCESS.
echo.
echo    OJO: en estas pruebas el robot NO mira la linea blanca.
echo    En el piso, con lugar alrededor, y el cable flojo.
echo.
echo    AL TERMINAR: cerrar la ventana y volver a cargar el programa
echo    de partido con CARGAR-ROBOT.bat (1 azul / 2 amarillo).
echo    Si despues van a cargar el ARQUERO en esta PC, cerrar antes
echo    el Teensy Loader.
echo ============================================================
if exist "%PYW%" (
  start "" "%PYW%" "%TABLERO%\tablero.py"
) else (
  start "Tablero del delantero" "%PY%" "%TABLERO%\tablero.py"
)
echo.
echo    (Esta ventana negra se puede cerrar: la del tablero sigue.)
pause
exit /b 0

rem ---------------------------------------------------------------------
rem No es (o no se sabe si es) el delantero: no se ofrece cargar. Se pide
rem enchufar SOLO el delantero y se vuelve a mirar.
:noconfirmado
echo.
if "%QUIEN%"=="OTRO" goto otrorobot
if "%QUIEN%"=="NINGUNO" goto ningunrobot
echo   XXX  No pude confirmar que el robot enchufado sea el delantero.
echo        No le cargo nada.
goto enchufar
:otrorobot
echo   XXX  Hay enchufado un robot que NO es el delantero, o no pude leer
echo        su numero. No le cargo nada: el tablero tiene los pines del
echo        delantero, y el cargador no elige a cual le carga.
goto enchufar
:ningunrobot
echo   XXX  No veo ningun robot enchufado por USB. Si esta enchufado y
echo        no aparece, desenchufalo y volvelo a enchufar.
:enchufar
echo.
echo        Deja enchufado SOLO el DELANTERO, espera unos segundos y
echo        apreta una tecla para mirar de nuevo.
echo        Para salir: cerra esta ventana negra.
echo        Para ver la ventana sin robot: TABLERO-DEMO.bat.
echo.
pause
echo.
goto verificar

rem ---------------------------------------------------------------------
rem Corre "ver" y deja en QUIEN: DELANTERO, OTRO, NINGUNO o NOSE.
rem Es DELANTERO solo si "ver" dice "->  DELANTERO" (con los dos espacios,
rem tal cual) y NINGUN robot dice "NO es el delantero". Si "ver" falla o
rem dice otra cosa, queda NOSE.
rem (findstr y no find: en una PC con las herramientas de Git en el PATH,
rem "find" puede ser otro programa y el control no anda.)
:mirar
echo    Que robot hay enchufado:
"%PY%" "%~dp0robot-sin-powershell.py" ver > "%VER%" 2>&1
type "%VER%"
echo.
set "QUIEN=NOSE"
findstr /l /c:"->  DELANTERO" "%VER%" >nul
if not errorlevel 1 set "QUIEN=DELANTERO"
findstr /i /l /c:"No hay ninguna Teensy" "%VER%" >nul
if not errorlevel 1 set "QUIEN=NINGUNO"
findstr /i /l /c:"NO es el delantero" "%VER%" >nul
if not errorlevel 1 set "QUIEN=OTRO"
exit /b 0
