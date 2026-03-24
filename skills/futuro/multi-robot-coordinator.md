# 🤝 Skill Futuro: Multi-Robot Coordinator

> **Estado**: 🔮 FUTURO — Para RCJ Soccer y WRO RoboSports

## Cuándo activar

- RoboCup Junior Soccer (2 robots por equipo)
- WRO RoboSports Double Tennis (2 robots autónomos)
- Cualquier competencia multi-robot

## El problema

2 robots del mismo equipo en la cancha. Sin coordinación:
- Los dos van a la pelota → chocan entre sí
- Los dos esperan → nadie va
- Los dos atacan → nadie defiende
- Se estorban en vez de colaborar

## Estrategias de coordinación

### 1. Roles fijos (más simple)
```
Robot A = Atacante (siempre va a la pelota)
Robot B = Arquero (siempre defiende el arco)
```
**Pro**: Simple, predecible. **Contra**: Inflexible.

### 2. Roles por zona
```
Si pelota en zona de ataque → A ataca, B cubre medio
Si pelota en zona de defensa → B defiende, A espera
Si pelota en medio campo → El más cercano ataca
```
**Pro**: Adaptable. **Contra**: Necesita saber posición de la pelota Y del compañero.

### 3. Roles dinámicos con comunicación
```
Cada robot broadcast: {mi_posición, veo_pelota, mi_distancia_a_pelota}
Regla: el más cercano ataca, el otro defiende
Anti-deadlock: si ambos a igual distancia, atacante = el de mayor ID
```
**Pro**: Óptimo. **Contra**: Necesita comunicación confiable.

## Comunicación entre robots

### RCJ Soccer 2025+
- Communication Module obligatorio en internacionales
- GPIO pin para start/stop desde referee
- UART para comunicación más compleja (futuro)
- [GitHub: soccer-communication-module](https://github.com/robocup-junior/soccer-communication-module)

### Opciones de comunicación

| Método | Latencia | Rango | Complejidad |
|--------|:--------:|:-----:|:-----------:|
| Bluetooth (Spike hub) | ~50ms | 10m | Baja |
| IR broadcast | ~10ms | 3m | Media |
| Radio (nRF24L01) | ~1ms | 30m | Media |
| WiFi (ESP32) | ~5ms | 50m | Alta |

## Prompt (para cuando se active)

```
Sos un diseñador de sistemas multi-robot para competencia.

EQUIPO: 2 robots
COMPETENCIA: [Soccer / RoboSports]
COMUNICACIÓN: [tipo y velocidad]
SENSORES: [por robot]

Necesito:
1. Asignación de roles (fijo / zona / dinámico)
2. Protocolo de comunicación (qué datos, cada cuánto)
3. Anti-colisión entre compañeros
4. Transición de roles (cuándo cambiar atacante/defensor)
5. Fallback si se pierde comunicación
6. Estrategias contra tipos comunes de oponente
```

## Recursos

- [RCJ Soccer Rules](https://robocup-junior.github.io/soccer-rules/)
- [RCJ Communication Module](https://github.com/robocup-junior/soccer-communication-module)
- [STP Architecture (CMU)](https://www.cs.cmu.edu/~mmv/papers/09aamas-skills.pdf) — Skills, Tactics, Plays
