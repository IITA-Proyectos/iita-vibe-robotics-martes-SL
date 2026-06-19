# 💡 Tips para Vibe Coding con Robótica

## ¿Qué es Vibe Coding?

Programar pidiéndole a una IA que escriba el código por vos.
Vos le decís QUÉ querés, la IA escribe el CÓMO.
Después lo probás en el robot real y pedís ajustes.

## El ciclo

```
DESCRIBIR → GENERAR → PROBAR → OBSERVAR → AJUSTAR → repetir
```

## Tips

### 1. Siempre decile qué robot tenés
**Mal:** "Hacé que el robot gire 90 grados"
**Bien:** Copiá `robots/spike-2wd-basico.py` y pegalo antes de pedir código.

### 2. Pedí código COMPLETO
Decile: "Hacé el programa completo para copiar y pegar en Pybricks"

### 3. Si no funciona, decile QUÉ pasó
**Mal:** "No funciona"
**Bien:** "El robot patina cuando arranca" o "gira de más, se pasa 5 grados"

### 4. Pedile que explique
"¿Qué hace la función clamp?" o "¿Por qué usás 0.7 como error?"

### 5. Guardá lo que funciona
Tu carpeta `alumnos/tunombre/` + `rutinas-exitosas/`

### 6. Compartí tus prompts
Si encontrás una buena forma de pedirle algo → `directivas-ia/`

### 7. Herramientas y scripts de automatización de la IA
Cualquier script de automatización o herramienta utilitaria creada por o para la IA debe ir en `alumnos/profefranco/tests_del_profe/herramientas_ia/` para poder ser reutilizado fácilmente.
