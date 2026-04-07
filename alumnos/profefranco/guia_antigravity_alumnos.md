# 🚀 Guía Completa: Configuración y Uso de Antigravity para Vibe Robotics

¡Bienvenidos! En esta guía paso a paso, aprenderemos a instalar Antigravity, configurarlo en español, clonar nuestro repositorio de trabajo y cómo usar esta Inteligencia Artificial para programar nuestras rutinas de Python con el robot.

---

## 1️⃣ Descargar e Instalar Antigravity

1. Descarga el instalador de **Antigravity** desde el enlace o plataforma oficial provista por el profesor.
2. Ejecuta el archivo descargado y sigue las instrucciones del asistente de instalación.
3. Una vez instalado, abre la aplicación.

---

## 2️⃣ Poner el entorno en Español

Para asegurarte de que Antigravity se comunique contigo siempre en español, tienes las siguientes opciones:

- **Instrucción directa (La más rápida):** En el chat principal de Antigravity, simplemente escríbele: *"A partir de ahora, quiero que todas tus respuestas sean en español."*
- **Reglas del usuario (Para que quede guardado siempre):** 
  1. Ve a las configuraciones de Antigravity (el ícono del engranaje / Settings).
  2. Busca la sección de **"Custom Rules"** (Reglas Personalizadas).
  3. Agrega la siguiente instrucción: `Siempre debes comunicarte y dar tus respuestas en español.`
  4. Guarda los cambios. ¡Listo!

---

## 3️⃣ Clonar nuestro Repositorio de la Clase

Para tener todos los códigos y rutinas que usamos en clase en tu propia computadora:

1. Dentro de Antigravity, abre una **Terminal** (puedes pedirle en el chat: *"Abre una terminal"*, o usar el menú superior `Terminal -> New Terminal`).
2. Escribe el siguiente comando para clonar nuestra carpeta de trabajo:
   ```bash
   git clone https://github.com/TU-USUARIO-O-IITA/iita-vibe-robotics-martes-SL.git
   ```
   *(Nota: Asegúrate de tener el enlace exacto de GitHub que te pasamos en clase).*
3. Ve a `File -> Open Folder` (Archivo -> Abrir Carpeta) y selecciona la carpeta **`iita-vibe-robotics-martes-SL`** que acabas de descargar.

---

## 4️⃣ Conectar con GitHub para guardar tus cambios

Para que tu trabajo no se pierda y el profe pueda verlo, necesitas subir tus cambios a tu repositorio:

### A. Configuración inicial (Solo la primera vez que usás Git en la compu)
Abre la terminal y escribe estos comandos con tus datos:
```bash
git config --global user.name "Tu Nombre o Usuario"
git config --global user.email "tu_correo@ejemplo.com"
```

### B. Subir tu trabajo (Cada vez que termines una clase)
¡Acá entra la magia de Antigravity! Ya no necesitas acordarte todos los comandos de memoria. 

**Opción con IA (Recomendada):**
Abre el chat de Antigravity y dile: *"Por favor, sube mis cambios de hoy a GitHub"*. La IA preparará los comandos (`git add`, `git commit` y `git push`) y te pedirá permiso para ejecutarlos.

**Opción Manual (Por si quieres hacerlo a mano):**
```bash
git add .
git commit -m "Escribe aquí lo que hiciste, por ejemplo: Agregué la rutina del cuadrado"
git push origin main
```
*(Es posible que la primera vez se abra una ventanita pidiéndote iniciar sesión en GitHub con tu navegador).*

---

## 5️⃣ Usando Antigravity para lo que hicimos hasta ahora 🤖

Hasta ahora, armamos rutinas piolas como la de hacer que el robot camine en un cuadrado (`Robotcuadrado2403.py`). Te muestro cómo podes usar tu nuevo asistente de IA para seguir aprendiendo y mejorando:

- **Entender el código:** Abre un archivo como `Robotcuadrado2403.py` y pregúntale a la IA: *"¿Me explicás línea por línea cómo funciona este script de Python para el robot?"*.
- **Crear nuevas rutinas:** Pídele ayuda para los nuevos desafíos del profe. Ejemplo: *"Ayúdame a escribir un código en Python basado en el archivo del cuadrado, pero para que el robot camine en forma de triángulo."*
- **Encontrar y solucionar errores:** Si tu código no funciona y el robot hace cualquier cosa (o tira texto rojo en la consola), copia ese error y dile a la IA: *"Recibí este error al correr el robot cuadrado, ¿cómo lo arreglo?"*. ¡Te dirá exactamente la línea en la que te equivocaste!

¡Eso es todo! Ya estás listo para aprovechar al máximo esta herramienta en tus proyectos con Vibe Robotics. 🚀 No tengas miedo de experimentar y preguntarle cualquier duda a la IA.
