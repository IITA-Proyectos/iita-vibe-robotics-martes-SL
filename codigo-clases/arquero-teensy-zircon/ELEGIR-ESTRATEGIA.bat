@echo off
rem ELEGIR-ESTRATEGIA.bat - elegir la estrategia del arquero (1 o 2) y
rem cargarla al robot. [2026-11-13]
rem   1 = ESTRATEGIA 1: el programa de juego       (funciona\partido-0921)
rem   2 = ESTRATEGIA 2: sobre la linea del area    (funciona\estrategia-2)
rem Mira que el robot enchufado sea el ARQUERO (tablero.py --quien), pregunta
rem otra vez (hay que escribir S), compila sin tocar el robot y recien ahi
rem carga. Enter solo o 0 = sale SIN cargar nada.
rem OJO: cargar reinicia el robot y ARRANCA SOLO. Bateria apagada.
rem Para probar la ventana sin cargar:  ELEGIR-ESTRATEGIA.bat --sin-cargar
title Elegir la estrategia del arquero
chcp 65001 >nul
setlocal
set "PY=%USERPROFILE%\.platformio\penv\Scripts\python.exe"
if not exist "%PY%" goto sinpython
"%PY%" "%~dp0pruebas\herramientas\elegir_estrategia.py" %*
echo.
pause
exit /b 0

:sinpython
echo   XXX  No encuentro el Python de PlatformIO en:
echo        "%PY%"
echo        Sin eso no se puede compilar ni cargar. Avisale al profe.
echo.
pause
exit /b 1
