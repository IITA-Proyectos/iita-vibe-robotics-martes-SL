# 🛤️ Skill Futuro: Trajectory & Control Planner

> **Estado**: 🔮 FUTURO — Para robots rápidos con odometría avanzada

## Cuándo activar

- WRO Future Engineers (conducción autónoma)
- FTC/FRC (path planning con RoadRunner/WPILib)
- Robots con odometría de 2-3 ruedas (dead wheels)
- Cuando el perfil trapezoidal simple ya no alcanza

## Conceptos que cubre

### Odometría 2D
```
┌─────────────────────────┐
│  Encoder izq + der      │
│  + heading (gyro)       │
│         ↓               │
│  Δx = Δd × cos(θ)      │
│  Δy = Δd × sin(θ)      │
│         ↓               │
│  Posición (x, y, θ)    │
└─────────────────────────┘
```

### Perfiles de movimiento

| Perfil | Complejidad | Suavidad | Para qué |
|--------|:-----------:|:--------:|----------|
| Trapezoidal | Baja | Media | Spike Prime, movimientos simples |
| S-curve | Media | Alta | Menos vibración, más consistente |
| Motion profiling | Alta | Máxima | FTC RoadRunner, trayectorias curvas |

### Pure Pursuit (seguimiento de trayectoria)
```
1. Definir waypoints: [(x1,y1), (x2,y2), ...]
2. En cada ciclo:
   a. Encontrar punto más cercano en la trayectoria
   b. Calcular punto "lookahead" a distancia L adelante
   c. Calcular curvatura para llegar al lookahead
   d. Comandar velocidad y turn_rate
3. Ventaja: suaviza curvas, tolerante a errores de posición
```

## Prompt (para cuando se active)

```
Sos un ingeniero de control de robots móviles.
Ayudame a diseñar el sistema de navegación.

ROBOT:
- Tracción: [diferencial / mecanum / omnidireccional / Ackermann]
- Sensores de posición: [encoders, gyro, dead wheels, LiDAR, cámara]
- Velocidad máxima: [mm/s]

OBJETIVO: [describir trayectoria o misión]

Necesito:
1. Sistema de odometría (fusión de sensores)
2. Perfil de velocidad apropiado
3. Controlador de trayectoria (PID / Pure Pursuit / Ramsete)
4. Manejo de error de localización
5. Recovery cuando la confianza en posición baja
6. Parámetros a tunear con procedimiento de tuning
```

## Plataformas y frameworks

| Plataforma | Framework | Lenguaje | Para qué |
|------------|-----------|----------|----------|
| FTC | RoadRunner | Java/Kotlin | Path planning FTC |
| FRC | WPILib + PathPlanner | Java/C++/Python | Trayectorias FRC |
| Custom | ROS 2 Jazzy | Python/C++ | Robots avanzados |
| Pybricks | Manual | Python | Perfiles simples |
| Arduino | Custom | C++ | Robots intermedios |

## Recursos

- [WPILib Trajectory Tutorial](https://docs.wpilib.org/en/stable/docs/software/advanced-controls/trajectories/index.html)
- [RoadRunner (FTC)](https://learnroadrunner.com/)
- [Pure Pursuit Paper](https://www.ri.cmu.edu/pub_files/pub3/coulter_r_craig_1992_1/coulter_r_craig_1992_1.pdf)
- [ROS 2 Navigation](https://docs.nav2.org/)
