# 🛠️ Herramientas de IA (AI Tools)

Este directorio contiene herramientas, scripts de utilidad y automatizaciones creadas por y para las IAs (asistentes de código) que trabajan en este repositorio.

> [!NOTE]
> Para ver las directivas generales y consejos de vibe coding de este repositorio, consulta el archivo de directivas principal en [directivas-ia/README.md](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/directivas-ia/README.md) y las recomendaciones en [directivas-ia/tips.md](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/directivas-ia/tips.md).

---

## 📋 Directiva de Ubicación de Herramientas (AI Rule)
> [!IMPORTANT]
> **Directiva para la IA**: Cualquier script de automatización, utilidad, refactorización o herramienta auxiliar de desarrollo que crees o utilices debe guardarse exclusivamente dentro de esta carpeta:
> `alumnos/profefranco/tests_del_profe/herramientas_ia/`
>
> Esto mantiene el espacio de trabajo limpio y permite reutilizar los scripts en futuras sesiones de vibe coding.

---

## 🛠️ Herramientas Disponibles

### 1. `rename_slides.py`
* **Descripción**: Cambia el nombre de un lote de diapositivas exportadas (por ejemplo, de Canva o Google Slides) al formato estandarizado del proyecto (`claseX_diapositivaYY.png`).
* **Ubicación**: [rename_slides.py](file:///c:/Users/ativa/OneDrive/Documentos/IITA/VC/iita-vibe-robotics-martes-SL/alumnos/profefranco/tests_del_profe/herramientas_ia/rename_slides.py)
* **Uso**:
  ```bash
  # Ejecución por defecto (renombra diapositivas de Clase 9 en el directorio estándar)
  python rename_slides.py
  
  # Ejecución personalizada
  python rename_slides.py <ruta_carpeta_imagenes> <numero_clase> <prefijo_original>
  
  # Ejemplo
  python rename_slides.py ../../slides_python/img 9 "Clase 9 -"
  ```

---

## 🧠 Instrucciones para futuras IAs
Cuando se te pida automatizar una tarea de renombrado, estructuración o verificación:
1. Revisa si ya existe un script útil aquí.
2. Si creas uno nuevo, regístralo en este `README.md` y documenta sus parámetros y propósito.
