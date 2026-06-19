# Guía de Carga y Ejecución de Código en EV3

Esta guía contiene los comandos manuales para subir y ejecutar el código en el robot EV3 desde tu terminal de Windows. También incluye un script automatizado para hacerlo todo en un solo comando.

---

## Opción A: Script Automatizado (Recomendada)

He creado un archivo llamado `deploy.bat` en esta misma carpeta. Cuando quieras cargar y correr tu código:

1. Abre la terminal en esta carpeta.
2. Escribe el siguiente comando y presiona **Enter**:
   ```cmd
   .\deploy.bat
   ```
3. Ingresa la contraseña `maker` cuando te lo solicite. El script detendrá ejecuciones colgadas anteriores, subirá el archivo actual `main.py` y lo ejecutará en el robot en un solo paso.

---

## Opción B: Comandos Manuales Paso a Paso

ping ev3dev.local

Si prefieres ingresar los comandos manualmente uno por uno en la terminal:

### Paso 1: Subir el archivo `main.py`
Envía tu código desde la PC al robot con el comando:
```powershell
scp main.py robot@ev3dev.local:/home/robot/tests_clase/

scp C:\Users\ativa\OneDrive\Documentos\IITA\VC\iita-vibe-robotics-martes-SL\codigo-ejemplo\movimiento\seguidor-lineleader-v2\main.py  robot@ev3dev.local:/home/robot/tests_clase/
```
*(Contraseña: `maker`)*

### Paso 2: Ejecutar el archivo en el robot
Inicia la ejecución usando `brickrun` mediante SSH:
```powershell
ssh robot@ev3dev.local "brickrun --directory=/home/robot/tests_clase /home/robot/tests_clase/main.py"
```
*(Contraseña: `maker`)*

---

## Resolución de Problemas Frecuentes

### 1. Error: `Text file busy` (El archivo de texto está ocupado)
Este error significa que el programa anterior sigue ejecutándose en el robot. Para forzar su cierre, ejecuta este comando en tu terminal antes de volver a subir el archivo:
```powershell
ssh robot@ev3dev.local "killall -9 python3; killall -9 brickrun; pkill -9 -f main.py"
```
*(Contraseña: `maker`)*

Una vez hecho esto, puedes volver al **Paso 1**.

### 2. Error: `Host key verification failed` o problemas de huella digital
Si te aparece una advertencia sobre la autenticidad del host, añade la opción `-o StrictHostKeyChecking=no` al comando:
```powershell
scp -o StrictHostKeyChecking=no main.py robot@ev3dev.local:/home/robot/tests_clase/
```
