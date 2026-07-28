# 🤖 Definiciones de Robots

Acá guardamos las configuraciones de los robots del taller.
Cuando le pedís código a la IA, **pasale el archivo del robot que estás usando**
para que sepa los puertos, ruedas, y motores correctos.

## Robots disponibles

| Robot | Archivo | Descripción |
|-------|---------|-------------|
| Base 2WD | `spike-2wd-basico.py` | LEGO Spike Prime. 2 motores grandes, ruedas medianas 56mm, puertos E/F |
| ⚽ Fútbol 2025 | `teensy-zircon-soccer-2025.md` | **NO es LEGO.** Teensy 4.1 + Zircon + OpenMV, C++, 3 motores omni |

> Los archivos `.py` son robots LEGO para Pybricks. El `.md` de fútbol describe un robot de otra
> familia: se programa en C++ y se carga por USB. Para ese usá también la directiva
> [`../directivas-ia/system-prompts/soccer-teensy-zircon.md`](../directivas-ia/system-prompts/soccer-teensy-zircon.md).

## ¿Cómo agregar un robot nuevo?

1. Creá un archivo `.py` con la configuración
2. Incluí: puertos, tipo de motor, diámetro de rueda, distancia entre ejes
3. Incluí los imports y la inicialización completa
4. Actualizá esta tabla
