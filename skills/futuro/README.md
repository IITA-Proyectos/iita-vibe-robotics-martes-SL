# 🔮 Skills Futuros

Estos skills están pensados para cuando el taller escale más allá de Spike Prime + Pybricks hacia hardware más avanzado (Arduino, Raspberry Pi, ESP32, cámaras, LiDAR).

**Hoy no los usamos**, pero la estructura está lista para que cuando llegue el momento, solo haya que completar los detalles técnicos.

## Roadmap

| Skill | Necesita | Para qué competencia | Prioridad |
|-------|----------|----------------------|:---------:|
| `vision-pipeline-designer` | Cámara (RPi/ESP32-CAM/OpenMV) | RCJ Soccer Open, WRO Future Engineers | Alta |
| `trajectory-control-planner` | Encoders + IMU + odometría | WRO FE, FTC, robots rápidos | Media |
| `multi-robot-coordinator` | 2+ robots con comunicación | RCJ Soccer, WRO RoboSports | Media |
| `platform-adapter` | Múltiples plataformas | Todos | Alta |
| `telemetry-log-analyzer` | Serial/BT logging | Todos | Alta |

## Cuándo activarlos

1. **Cuando un alumno quiera usar Arduino/RPi** → activar `platform-adapter`
2. **Cuando conecten una cámara** → activar `vision-pipeline-designer`
3. **Cuando hagan RoboCup Soccer con robots custom** → activar `multi-robot-coordinator`
4. **Cuando los programas se vuelvan complejos y difíciles de debuggear** → activar `telemetry-log-analyzer`
5. **Cuando compitan en FTC/FRC o WRO Future Engineers** → activar `trajectory-control-planner`
