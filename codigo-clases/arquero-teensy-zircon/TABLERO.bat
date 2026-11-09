@echo off
rem TABLERO.bat - la VENTANA del ARQUERO: ver la pelota y los sensores,
rem probar y ajustar los movimientos con botones, y preparar una cancha
rem nueva, sin escribir comandos. [2026-11-03]
rem
rem   1. Mira que robot hay enchufado, SIN abrir ningun puerto
rem      (tablero.py --quien). Si NO es el arquero, si no hay ninguno o
rem      si hay mas de uno, no sigue: pide enchufar SOLO el arquero y
rem      vuelve a mirar. Es un lazo: NUNCA carga sin confirmar.
rem   2. Pregunta si carga el programa del tablero (pruebas\tablero).
rem      Enter, S o SI = carga.  N o NO = no carga.  Otra cosa = pregunta
rem      de nuevo. Si el robot YA lo tiene cargado, contestar N.
rem   3. Justo antes de cargar, mira OTRA VEZ que siga siendo el arquero
rem      (pudieron cambiar el robot mientras esperaba la respuesta).
rem      Carga con pruebas\herramientas\cargar.py, el mismo que usa el
rem      boton CARGAR EL PARTIDO: vuelve a controlar que haya un Teensy y
rem      que sea el arquero, que el puerto este libre, cierra el Teensy
rem      Loader y carga con teensy-cli. NO abre el Teensy Loader.
rem   4. Abre la ventana (pruebas\tablero\tablero.py).
rem
rem NO TOCA el programa de partido (funciona\partido-0921, la 0921, la de
rem jugar). Pero el robot queda con el TABLERO cargado: AL TERMINAR, en la
rem ventana, pestana PARTIDO, boton CARGAR EL PARTIDO (o PARTIDO.bat).
rem Apenas termina ESA carga el robot arranca solo. [2026-11-08: ya no se
rem usa CARGAR-PARTIDO.bat de seguir-y-despejar: ese carga la 0.1103]
rem
rem Sin robot: TABLERO-DEMO.bat abre la misma ventana con un robot de mentira.
setlocal
chcp 65001 >nul
title Tablero del arquero
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
set "PYW=%USERPROFILE%\.platformio\penv\Scripts\pythonw.exe"
set "TABLERO=%~dp0pruebas\tablero"
set "CARGAR=%~dp0pruebas\herramientas\cargar.py"

echo ============================================================
echo    ARQUERO  -  TABLERO  (la ventana con botones)
echo ============================================================
echo    NO es el programa de partido. AL TERMINAR: en la ventana,
echo    pestana PARTIDO, boton CARGAR EL PARTIDO (o PARTIDO.bat).
echo.
if not exist "%PY%" goto sinpython

rem Solo se sigue si --quien dice que hay UN Teensy y es el ARQUERO
rem (13348550). Con el DELANTERO (15708680) no: los pines estan cruzados.
:verificar
echo    Que robot hay enchufado:
"%PY%" "%TABLERO%\tablero.py" --quien
set "CODIGO=%ERRORLEVEL%"
echo.
if not "%CODIGO%"=="0" goto noconfirmado

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
"%PY%" "%TABLERO%\tablero.py" --quien
set "CODIGO=%ERRORLEVEL%"
echo.
if not "%CODIGO%"=="0" goto noconfirmado
echo    Cargando (la primera vez tarda ~1 min, despues ~10 s)...
"%PY%" "%CARGAR%" "%TABLERO%"
if errorlevel 1 goto fallo

:abrir
echo.
echo ============================================================
echo    Abriendo la ventana del tablero...
echo.
echo    Arriba tiene que decir ARQUERO y "Programa del tablero
echo    cargado". Si no, la carga no entro.
echo.
echo    En el piso, con lugar alrededor, y el cable flojo (o SIN CABLE).
echo.
echo    AL TERMINAR: en la ventana, pestana PARTIDO, boton CARGAR EL
echo    PARTIDO (o PARTIDO.bat). Carga la 0921 (funciona\partido-0921),
echo    la de jugar, con los valores que estan escritos ahi. El robot
echo    arranca solo apenas termina la carga.
echo ============================================================
if exist "%PYW%" (
  start "" "%PYW%" "%TABLERO%\tablero.py"
) else (
  start "Tablero del arquero" "%PY%" "%TABLERO%\tablero.py"
)
echo.
echo    (Esta ventana negra se puede cerrar: la del tablero sigue.)
pause
exit /b 0

:fallo
echo.
echo   XXX  LA CARGA FALLO. El robot sigue con el programa que tenia.
echo        No abro la ventana. Si habia una ventana del tablero
echo        abierta, cerrala y volve a abrir TABLERO.bat.
pause
exit /b 1

:sinpython
echo   XXX  No encuentro el Python de PlatformIO en:
echo        "%PY%"
pause
exit /b 1

rem ---------------------------------------------------------------------
rem No es (o no se sabe si es) el arquero: no se ofrece cargar. Se pide
rem enchufar SOLO el arquero y se vuelve a mirar. Los codigos son los de
rem tablero.py --quien: 0 ARQUERO, 3 DELANTERO, 4 OTRO, 5 NINGUNO,
rem 6 VARIOS, 8 un Teensy en MODO CARGA (y 7 si no pudo mirar).
rem El 8 (2026-11-03): un Teensy en modo carga no tiene puerto COM y el
rem cargador le carga PRIMERO a ese, aunque sea el delantero.
:noconfirmado
if "%CODIGO%"=="3" goto esdelantero
if "%CODIGO%"=="4" goto otrorobot
if "%CODIGO%"=="5" goto ningunrobot
if "%CODIGO%"=="6" goto varios
if "%CODIGO%"=="8" goto encarga
echo   XXX  No pude confirmar que sea el arquero.
goto enchufar
:esdelantero
echo   XXX  Ese es el DELANTERO. No le cargo nada: el tablero del
echo        arquero tiene los pines cruzados para el delantero.
goto enchufar
:otrorobot
echo   XXX  Hay un robot que NO es el arquero, o no pude leer su numero.
goto enchufar
:ningunrobot
echo   XXX  No veo ningun robot enchufado por USB. Si esta enchufado y
echo        no aparece, desenchufalo y volvelo a enchufar.
goto enchufar
:varios
echo   XXX  Hay mas de un Teensy: el cargador no elige a cual le carga.
goto enchufar
:encarga
echo   XXX  Hay un Teensy en MODO CARGA (sin programa corriendo): no se
echo        que robot es, y el cargador le cargaria a ESE (puede ser el
echo        DELANTERO). Desenchufalo y cerra el Teensy Loader si quedo
echo        abierto. Si el UNICO enchufado es el ARQUERO y sigue asi,
echo        avisale al profe.
:enchufar
echo.
echo        Deja enchufado SOLO el ARQUERO, espera unos segundos y
echo        apreta una tecla para mirar de nuevo.
echo        Para salir: cerra esta ventana negra.
echo        Sin robot: TABLERO-DEMO.bat.
echo.
pause
echo.
goto verificar
