# Manejar el robot sin abrir el Arduino IDE

Dos scripts para compilar, cargar y hablarle al robot desde la consola. Sirven para trabajar
rápido (no hay que apuntar con el mouse) y para que quede **escrito** qué se le mandó y qué
contestó — que es justo lo que después va a la bitácora.

No reemplazan al Arduino IDE: los sketches se siguen abriendo con el IDE igual que siempre.

---

## `robot.ps1` — compilar y cargar

```powershell
.\robot.ps1 compilar pruebas\ver-camara
.\robot.ps1 cargar   pruebas\cuadrado-giroscopo
.\robot.ps1 cargar   funciona\despeje-pelota
```

Compila siempre primero y **si la compilación falla no carga nada**. Detecta solo el puerto del
Teensy, así que no hay que elegirlo a mano.

Por debajo usa el `arduino-cli` que viene adentro del Arduino IDE 2.x — no es un programa
aparte que haya que instalar.

> 🚨 **Cargar un sketch REINICIA el Teensy.** Si el sketch arranca solo (como
> `cuadrado-lento` y `cuadrado-giroscopo`, que empiezan la cuenta regresiva al recibir energía),
> el robot va a empezar a moverse ~10 s después de que termine la carga. Cargá con el robot en
> el piso o con la batería apagada, no sobre la mesa.

## `serie.py` — mandarle teclas y leer lo que contesta

```powershell
python serie.py --escuchar 3                # solo escuchar
python serie.py --enviar 3 --escuchar 4     # mandar la tecla '3' y escuchar
python serie.py --enviar 0                  # PARAR (emergencia)
```

Reemplaza al Monitor Serie. Necesita `pyserial`.

---

## Las rutas de esta máquina

Los dos scripts tienen rutas adentro que son **de la compu del taller**. En otra máquina hay que
cambiarlas. Anotadas acá para no volver a buscarlas:

| Qué | Dónde |
|---|---|
| Arduino IDE 2.x (portable) | `C:\Users\alumnos\Documents\OTTO\arduino\Arduino IDE\` |
| `arduino-cli` | `...\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe` |
| Config del IDE | `C:\Users\alumnos\.arduinoIDE\arduino-cli.yaml` |
| Core Teensy | `%LOCALAPPDATA%\Arduino15\packages\teensy\hardware\avr\1.62.0` |
| Librerías | `C:\Users\alumnos\Documents\Arduino\libraries` |
| Python con pyserial | `C:\Users\alumnos\.platformio\penv\Scripts\python.exe` |
| Placa | `teensy:avr:teensy41` — apareció en COM5 |

> El Arduino IDE está **portable dentro de `Documents\OTTO`**, no en `Archivos de programa`.
> Buscarlo donde uno espera da "no está instalado" cuando en realidad está todo.

## Ejemplo completo

```powershell
$py = "$env:USERPROFILE\.platformio\penv\Scripts\python.exe"
.\robot.ps1 cargar "funciona\despeje-pelota"
& $py .\serie.py --puerto COM5 --enviar "0" --escuchar 3   # frenarlo: arranca solo
& $py .\serie.py --puerto COM5 --enviar "?" --escuchar 3
& $py .\serie.py --puerto COM5 --enviar "i" --escuchar 2   # que ve la camara
```

## Si el puerto no es COM5

El Teensy no siempre cae en el mismo puerto — cambia si se enchufa en otro conector USB.
Para encontrarlo:

```powershell
Get-CimInstance Win32_PnPEntity | Where-Object { $_.DeviceID -match 'VID_16C0' -and $_.Name -match 'COM\d+' } | Select-Object Name
```

`VID_16C0` es el identificador de PJRC, el fabricante del Teensy.
