---
name: advanced-pid-optimization
description: Técnicas avanzadas de optimización de controladores PID para robótica de competencia — más allá del tuning empírico básico. Cubre gain scheduling por régimen, derivative filtering (D-on-measurement, D-low-pass), anti-windup con back-calculation, feedforward del modelo, two-degree-of-freedom (setpoint weighting), control bang-bang en saturación, deadband, métricas formales (ITAE, ISE, IAE), gain scheduling adaptativo, y consideraciones de tiempo de muestreo. Usar SIEMPRE que se requiera optimizar un PID que ya funciona pero hay que pulirlo, exprimir velocidad sin perder estabilidad, manejar régimenes muy distintos (recta vs curva), saturación de actuadores, ruido alto en la medición, o se mencione 'gain scheduling', 'derivative kick', 'D-on-measurement', 'derivative filter', 'back-calculation anti-windup', 'feedforward', 'two-degree-of-freedom PID', 'setpoint weighting', 'ITAE', 'ISE', 'control optimizado', 'PID adaptativo'. NO usar para el primer tuning empírico básico (eso es robotics-control-theory).
---

# PID optimizado para robots de competencia

Esta skill cubre las técnicas que toman un PID que **ya funciona** y lo llevan al siguiente nivel: más velocidad, menos overshoot, mejor desempeño en condiciones cambiantes, robustez frente a ruido y saturación. Asume conocimiento de `robotics-control-theory` (los fundamentos: qué es P, I, D, anti-windup básico, Ziegler-Nichols).

## El problema con el PID "naïve"

```python
# PID clásico (lo que la mayoría implementa)
error = setpoint - measurement
integral += error * dt
derivative = (error - last_error) / dt
last_error = error
output = Kp*error + Ki*integral + Kd*derivative
```

Este código funciona, pero tiene **cuatro problemas serios** que aparecen en competencia real:

1. **Derivative kick**: cuando el setpoint cambia bruscamente, la derivada del error explota → spike enorme en el output → robot da tirones.
2. **Integral windup**: si el actuador satura (motor a 100%), el integral sigue creciendo, y cuando el error revierte tarda MUCHO en bajarse el integral acumulado.
3. **D amplifica ruido**: la derivada del error es muy sensible a ruido de medición. Si la señal del sensor tiene 2% de ruido, Kd=10 lo amplifica a 20% en el output.
4. **No usa el conocimiento del modelo**: el PID descubre cada curva como si fuera nueva. Si sabés la geometría, podés hacer feedforward.

Vamos por cada uno.

## 1. Derivative on measurement (mata el "derivative kick")

**Insight**: la derivada del error = derivada del (setpoint - measurement). Si el setpoint es constante, da igual derivar el error o `-measurement`. Pero cuando el setpoint cambia, derivar el error genera un impulso. **Derivar solo la medición** evita el impulso.

```python
# Forma "D-on-measurement" — preferida en industria
error = setpoint - measurement
integral += error * dt
derivative = -(measurement - last_measurement) / dt   # OJO el signo
last_measurement = measurement
output = Kp*error + Ki*integral + Kd*derivative
```

**Cuándo sí cambia**: en line following el setpoint es constante (50, o 35 si es array de 8), no hay derivative kick. Pero en sistemas con setpoint dinámico (control de posición a un target que cambia, gyro-driving a heading variable) esta forma es **estrictamente mejor**.

## 2. Derivative low-pass filter (mata el ruido amplificado)

El término D amplifica ruido de medición. Si tu sensor lee con ±2% de ruido y Kd=10, tu output tiembla ±20% solo por ruido. Solución: **filtro low-pass sobre la derivada**.

```python
class PIDFilteredD:
    def __init__(self, kp, ki, kd, d_alpha=0.4):
        self.kp, self.ki, self.kd = kp, ki, kd
        self.d_alpha = d_alpha       # 0.2-0.5 típico; menos = más suavizado
        self.integral = 0
        self.last_meas = None
        self.filtered_d = 0
    
    def update(self, setpoint, measurement, dt):
        error = setpoint - measurement
        self.integral += error * dt
        
        if self.last_meas is None:
            raw_d = 0
        else:
            raw_d = -(measurement - self.last_meas) / dt
        self.last_meas = measurement
        
        # Low-pass sobre la derivada
        self.filtered_d = self.d_alpha * raw_d + (1 - self.d_alpha) * self.filtered_d
        
        return self.kp * error + self.ki * self.integral + self.kd * self.filtered_d
```

**Regla práctica**: si Kd>0 y ves jitter en el output sin razón aparente (sin que se mueva el error), agregá un low-pass con `alpha=0.3`. Vas a poder subir Kd 2-3× sin temblequera.

## 3. Anti-windup avanzado — back-calculation

El anti-windup "clamp" (`if integral > MAX: integral = MAX`) es lo básico. Funciona, pero deja al integral pegado en el límite hasta que el error revierte completo.

**Back-calculation** descarga el integral progresivamente cuando el actuador satura, devolviendo control suave en cuanto el sistema se descomprime.

```python
class PIDBackCalc:
    def __init__(self, kp, ki, kd, out_min, out_max, kbc=None):
        self.kp, self.ki, self.kd = kp, ki, kd
        self.out_min, self.out_max = out_min, out_max
        # Coeficiente back-calculation. Regla práctica: 1/Tt donde Tt ≈ sqrt(Ti*Td)
        self.kbc = kbc if kbc is not None else 1.0
        self.integral = 0
        self.last_meas = None
    
    def update(self, setpoint, measurement, dt):
        error = setpoint - measurement
        
        if self.last_meas is None:
            d = 0
        else:
            d = -(measurement - self.last_meas) / dt
        self.last_meas = measurement
        
        # Output sin saturar
        out_unsat = self.kp * error + self.ki * self.integral + self.kd * d
        
        # Saturar
        out_sat = max(self.out_min, min(self.out_max, out_unsat))
        
        # Actualizar integral con back-calc: si saturó, "tira" el integral
        # hacia abajo proporcionalmente al excess
        saturation_excess = out_sat - out_unsat
        self.integral += (error + self.kbc * saturation_excess) * dt
        
        return out_sat
```

**Cuándo importa**: cuando el actuador satura por períodos largos. En line following normal no es crítico (el output rara vez satura). En control de velocidad con cargas variables, sí.

## 4. Two-degree-of-freedom PID (setpoint weighting)

Cuando querés que el PID **rechace bien las perturbaciones** (es decir, vuelva al setpoint rápido si algo lo empuja) PERO **no ataque agresivo al cambio de setpoint**, usás dos ganancias distintas para cada caso.

```python
class PID2DOF:
    """
    PID con pesos b, c separados para el setpoint en P y D.
    output = Kp*(b*setpoint - measurement) + Ki*integral_error + Kd*(c*d_sp/dt - d_meas/dt)
    
    b=1, c=1 → PID clásico.
    b<1, c=0 → setpoint weighting típico (suaviza respuesta a step en setpoint).
    b=0, c=0 → I-PD (clásico industrial).
    """
    def __init__(self, kp, ki, kd, b=0.5, c=0.0):
        self.kp, self.ki, self.kd = kp, ki, kd
        self.b, self.c = b, c
        self.integral = 0
        self.last_sp = None
        self.last_meas = None
    
    def update(self, setpoint, measurement, dt):
        error = setpoint - measurement
        self.integral += error * dt
        
        if self.last_meas is None:
            d_meas = 0
            d_sp = 0
        else:
            d_meas = (measurement - self.last_meas) / dt
            d_sp = (setpoint - self.last_sp) / dt
        self.last_meas = measurement
        self.last_sp = setpoint
        
        p_term = self.kp * (self.b * setpoint - measurement)
        d_term = self.kd * (self.c * d_sp - d_meas)
        return p_term + self.ki * self.integral + d_term
```

**Aplicación típica**: control de posición de un brazo robótico donde a veces el target cambia (no querés tirones) y a veces hay carga variable (querés rechazo fuerte).

Para line following el setpoint es constante, así que da igual. Para gyro-driving con cambios de target (girar a 90°, después a -45°) **es clave** — evita que cada cambio de target genere un spike.

## 5. Feedforward — el upgrade silencioso

Feedforward = **calcular el control que se necesita SIN esperar al error**. Es proactivo, el PID solo corrige residuos.

### Feedforward en curva conocida

Si vas a hacer una curva de radio R a velocidad v, sabés que el turn rate ideal es `ω = v/R`. Setealo directamente; el PID solo compensa drift.

```python
def follow_curve_feedforward(radius_mm, length_mm, base_speed):
    drive.reset()
    # Velocidad angular teórica para la curva
    ff_turn_rate = base_speed * 1000 / radius_mm * 180 / 3.14159  # deg/s
    
    while drive.distance() < length_mm:
        cal = ll.calibrated()
        pos = pos_x10(cal) or 35
        error = pos - 35
        pid_correction = KP * error
        drive.drive(base_speed, ff_turn_rate + pid_correction)
        wait(10)
```

### Feedforward de fricción (compensación)

Motores reales tienen **fricción estática** (Coulomb friction) — necesitan un mínimo de potencia para arrancar. PID puro lo "descubre" lento.

```python
FRICTION_OFFSET = 5  # PWM units necesarios para vencer fricción

def velocity_pid_with_ff(target_v, measured_v, dt):
    error = target_v - measured_v
    pid_out = Kp * error + Ki * integral + Kd * derivative
    # FF: dirección de la velocidad target + offset por fricción
    ff = sign(target_v) * FRICTION_OFFSET + Kv * target_v
    return ff + pid_out
```

Donde `Kv` es el inverso de la ganancia velocidad↔PWM del motor (medible empíricamente: ¿qué PWM te da 100 deg/s en velocidad estable?).

## 6. Gain scheduling — múltiples PIDs por régimen

Un solo PID raramente es óptimo para todos los regímenes. **Gain scheduling** = cambiar las ganancias según el "régimen" actual.

### Caso típico en line following: recta vs curva

```python
def select_gains(error):
    """Switch entre dos sets de ganancias según magnitud del error."""
    if abs(error) < 8:    # casi recto
        return (1.0, 0.0, 6.0)   # KP, KI, KD agresivo pero limpio
    elif abs(error) < 20: # curva moderada
        return (1.4, 0.0, 10.0)
    else:                 # curva cerrada
        return (2.0, 0.0, 15.0)
```

### Versión continua (smooth gain scheduling)

Saltos discretos pueden generar discontinuidades. Mejor interpolación:

```python
def gains_smooth(error):
    """Interpolación lineal de Kp, Kd según |error|."""
    e = abs(error)
    if e <= 8:
        t = e / 8
        kp = 1.0 + t * 0.4
        kd = 6.0 + t * 4.0
    elif e <= 20:
        t = (e - 8) / 12
        kp = 1.4 + t * 0.6
        kd = 10.0 + t * 5.0
    else:
        kp = 2.0
        kd = 15.0
    return kp, kd
```

### Schedule por velocidad

Tu Kp óptimo escala con la velocidad. Más velocidad = más Kp (porque el error crece más rápido).

```python
def gains_by_speed(speed_mm_s):
    """Kp lineal con speed. Calibrado: Kp=0.8 @ 200 mm/s, Kp=1.8 @ 500 mm/s."""
    kp = 0.8 + (speed_mm_s - 200) / 300 * 1.0
    kd = 5.0 * kp                            # Kd típicamente escala con Kp
    return kp, 0, kd
```

## 7. Control bang-bang en saturación

Cuando el error es **enorme** (línea perdida, robot girado 90° de su heading), el PID puede mandar un output muy chico relativo al máximo y eso es **ineficiente**. Detectar saturación y mandar full power.

```python
def hybrid_control(error, deriv):
    if abs(error) > 50:                  # error gigante = saturar
        return 100 if error > 0 else -100  # bang-bang
    return Kp * error + Kd * deriv       # PID normal
```

**Trampa**: el switch entre bang-bang y PID es discontinuo. Si el sistema oscila alrededor del umbral (50), va a chattering. Mejor con histéresis:

```python
class HybridController:
    def __init__(self, threshold_in=60, threshold_out=40):
        self.in_bang = False
        self.t_in, self.t_out = threshold_in, threshold_out
    
    def update(self, error, deriv):
        if not self.in_bang and abs(error) > self.t_in:
            self.in_bang = True
        elif self.in_bang and abs(error) < self.t_out:
            self.in_bang = False
        
        if self.in_bang:
            return 100 if error > 0 else -100
        return Kp * error + Kd * deriv
```

## 8. Deadband — zona muerta intencional

Si tu sensor tiene resolución limitada y el error es minúsculo (1-2 unidades), correr el PID con un error tan chico es **mover el actuador por ruido**. Definir un deadband evita micro-correcciones.

```python
def deadband_pid(error, deriv, deadband=2):
    if abs(error) < deadband:
        return 0
    # Que el output sea continuo en el borde:
    adjusted_error = error - sign(error) * deadband
    return Kp * adjusted_error + Kd * deriv
```

Usar con cuidado en line following — un deadband muy grande causa zigzag suave.

## 9. Métricas formales para comparar tunings

Cuando comparás dos sets de ganancias, "se ve mejor" no escala. Usar métricas numéricas hace el tuning **comparable y reproducible**.

| Métrica | Fórmula | Penaliza | Cuándo usar |
|---|---|---|---|
| **IAE** (Integral Absolute Error) | `Σ |error|·dt` | Error sostenido | Métrica general, balanceada |
| **ISE** (Integral Squared Error) | `Σ error²·dt` | Errores grandes | Cuando overshoots son inaceptables |
| **ITAE** (Integral Time-weighted AE) | `Σ t·|error|·dt` | Errores que persisten | Cuando importa el settling time |
| **IST** (Integral Squared Output Change) | `Σ (Δoutput)²·dt` | Actuador "nervioso" | Para minimizar uso de actuador |

```python
class PIDMetrics:
    def __init__(self):
        self.iae = 0
        self.ise = 0
        self.itae = 0
        self.t = 0
        self.last_output = 0
        self.iso = 0
    
    def update(self, error, output, dt):
        self.t += dt
        self.iae += abs(error) * dt
        self.ise += error * error * dt
        self.itae += self.t * abs(error) * dt
        d_out = output - self.last_output
        self.iso += d_out * d_out * dt
        self.last_output = output

# Después de una corrida:
print("IAE:", metrics.iae, "ITAE:", metrics.itae, "ISE:", metrics.ise)
```

Correr 5 vueltas con cada set de ganancias, calcular ITAE promedio, elegir el mínimo. **Es así de simple y funciona**.

## 10. Tiempo de muestreo (dt) — la trampa que arruina tunings

Los términos I y D dependen de `dt`. Si tu loop a veces tarda 10 ms y a veces 25 ms, las ganancias efectivas cambian.

**Tres reglas**:

1. **Medir `dt` real, no asumirlo**:
```python
from pybricks.tools import StopWatch
watch = StopWatch()
last = watch.time()
while True:
    now = watch.time()
    dt = (now - last) / 1000.0   # segundos
    last = now
    ...
```

2. **Loop rate constante**: forzar `wait(target_ms - elapsed)` al final del loop.

3. **Si el loop varía mucho, normalizar las ganancias para dt fijo**:
```python
DT_NOMINAL = 0.02
factor = dt / DT_NOMINAL
integral += error * dt  # ya usa dt
derivative = (error - last_error) / dt  # ya usa dt
# Si tunearon con dt=20 ms, las fórmulas con dt explícito son robustas
```

**Regla práctica**: 50-100 Hz suele ser suficiente para robots LEGO. Más alto, el muestreo del sensor empieza a ser el cuello de botella; más bajo, perdés ancho de banda de control.

## 11. PID con saturación dinámica del integral

Más robusto que clamp fijo: limitar el integral según cuánto "headroom" queda en el output.

```python
def smart_clamp_integral(integral, error, ki, out_min, out_max):
    """Limita el integral para que NUNCA pueda saturar por sí solo."""
    headroom_max = (out_max - 0) / ki   # 0 = sin P ni D
    headroom_min = (out_min - 0) / ki
    return max(headroom_min, min(headroom_max, integral))
```

Más sofisticado: el "**conditional integration**" — solo integrar cuando el output NO está saturado:

```python
out_unsat = Kp*error + Ki*integral + Kd*deriv
out_sat = clamp(out_unsat, out_min, out_max)
if out_sat == out_unsat:           # no saturó
    integral += error * dt          # integrar normal
# si saturó, no se integra → no hay windup
```

## Checklist de "PID optimizado" para competencia

Tildalo todo cuando salgas a competir con un PID serio:

- [ ] **D-on-measurement** en vez de D-on-error (evita derivative kick si setpoint cambia)
- [ ] **Low-pass sobre la derivada** (alpha 0.3-0.4)
- [ ] **Anti-windup**: clamp, mejor back-calculation o conditional integration
- [ ] **Saturación explícita del output** (`max(out_min, min(out_max, output))`)
- [ ] **Loop rate constante** medido (no esperás "≈10 ms")
- [ ] **Gain scheduling** si los regímenes son MUY distintos
- [ ] **Feedforward** donde haya información a priori (radio de curva, peso del brazo)
- [ ] **Métricas (ITAE)** grabadas para comparar tunings
- [ ] **Ganancias guardadas como constantes named** en un solo lugar, no hard-coded
- [ ] **DataLog grabando**: error, P, I, D, output, dt para análisis post-corrida

## Patrón completo — "production-grade PID"

```python
from pybricks.tools import StopWatch, DataLog

class AdvancedPID:
    def __init__(self, kp, ki, kd,
                 out_min=-100, out_max=100,
                 d_alpha=0.4,
                 deadband=0):
        self.kp, self.ki, self.kd = kp, ki, kd
        self.out_min, self.out_max = out_min, out_max
        self.d_alpha = d_alpha
        self.deadband = deadband
        self.integral = 0
        self.last_meas = None
        self.filtered_d = 0
    
    def update(self, setpoint, measurement, dt):
        error = setpoint - measurement
        
        # Deadband
        if abs(error) < self.deadband:
            return 0
        
        # D-on-measurement con filtro
        if self.last_meas is None:
            raw_d = 0
        else:
            raw_d = -(measurement - self.last_meas) / dt
        self.last_meas = measurement
        self.filtered_d = self.d_alpha * raw_d + (1 - self.d_alpha) * self.filtered_d
        
        # Output sin saturar
        out_unsat = self.kp * error + self.ki * self.integral + self.kd * self.filtered_d
        
        # Saturación
        out_sat = max(self.out_min, min(self.out_max, out_unsat))
        
        # Conditional integration (no integrar si saturó en la dirección del error)
        if not ((out_sat >= self.out_max and error > 0) or
                (out_sat <= self.out_min and error < 0)):
            self.integral += error * dt
        
        return out_sat
    
    def reset(self):
        self.integral = 0
        self.last_meas = None
        self.filtered_d = 0
```

Uso en line follower con array de 8:

```python
pid = AdvancedPID(kp=1.5, ki=0, kd=12, out_min=-200, out_max=200, d_alpha=0.35)
watch = StopWatch()
log = DataLog('t', 'error', 'output', 'speed', name='run')
last_t = watch.time() / 1000.0

while distance < target:
    now = watch.time() / 1000.0
    dt = now - last_t
    last_t = now
    
    cal = ll.calibrated()
    pos = pos_x10(cal) or 35
    error = pos - 35
    
    output = pid.update(0, error, dt)   # setpoint=0 porque error ya viene relativo
    speed = adaptive_speed(error)
    
    drive.drive(speed, output)
    log.log(now, error, output, speed)
    
    # Loop rate constante
    elapsed_ms = (watch.time() / 1000.0 - now) * 1000
    if elapsed_ms < 20:
        wait(int(20 - elapsed_ms))
```

## Errores típicos

| Síntoma | Causa probable | Solución |
|---|---|---|
| Output tiembla aún con error chico | D amplifica ruido del sensor | Low-pass sobre D, alpha=0.3 |
| Tras saturar, tarda mucho en estabilizar | Windup | Back-calc o conditional integration |
| Spike de output al cambiar setpoint | Derivative kick | D-on-measurement |
| PID funciona bien en recta, mal en curva (o viceversa) | Régimen único no le sirve a ambos | Gain scheduling |
| Tuning óptimo en lab, no en competencia | Variables ambientales (batería, luz, fricción) | Re-tunear o agregar adaptive scheduling |
| ITAE no baja con más Kp | Sistema en su límite físico | El cuello de botella es mecánico (gear backlash, fricción, latencia sensor) |
| Output queda pegado en el límite | Integral windup masivo | Conditional integration |
| Funciona dt=10ms, falla dt=20ms | Ganancias no normalizadas por dt | Verificar dt en update y rehacer con dt explícito |

## Recursos

- "PID Controllers: Theory, Design, and Tuning" — Åström & Hägglund (referencia canónica industrial)
- "Tuning of PID Controllers Based on Bode's Ideal Transfer Function" — papers de O'Dwyer
- Brett Beauregard — "Improving the Beginner's PID" (serie de blog posts, base de las mejoras prácticas): http://brettbeauregard.com/blog/2011/04/improving-the-beginner%E2%80%99s-pid-introduction/
- Tim Wescott — "PID Without a PhD" (PDF clásico)
- "Feedback Systems" (Åström & Murray) — libre: https://fbsbook.org/
- Pybricks control internals (la propia DriveBase implementa muchas de estas técnicas): https://docs.pybricks.com/en/latest/parameters/control.html
