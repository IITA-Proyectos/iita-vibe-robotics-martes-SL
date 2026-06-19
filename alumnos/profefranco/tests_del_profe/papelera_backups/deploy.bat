@echo off
cls
echo ======================================================
echo          DEPLOY AUTOMATICO A EV3DEV (main.py)
echo ======================================================
echo.

echo [1/2] Subiendo main.py a ev3dev...
scp -o StrictHostKeyChecking=no main.py robot@ev3dev.local:/home/robot/tests_clase/
if %errorlevel% neq 0 (
    echo.
    echo [ERROR] No se pudo subir el archivo. Revisa que el robot este encendido y conectado.
    pause
    exit /b
)

echo.
echo [2/2] Ejecutando main.py en el robot...
ssh -o StrictHostKeyChecking=no robot@ev3dev.local "brickrun --directory=/home/robot/tests_clase /home/robot/tests_clase/main.py"

echo.
echo ======================================================
echo                  Proceso Finalizado
echo ======================================================
pause

