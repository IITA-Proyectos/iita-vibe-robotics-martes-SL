# Pybricks competition patterns

## Default import set

Use this compact import set unless the robot truly needs more:

```python
from pybricks.hubs import PrimeHub
from pybricks.pupdevices import Motor, ColorSensor, ForceSensor, UltrasonicSensor
from pybricks.parameters import Port, Direction, Stop
from pybricks.robotics import DriveBase
from pybricks.tools import wait, StopWatch
```

Remove unused devices before final output.

## Program shape

1. constants
2. hardware init
3. utility helpers
4. mission helpers
5. `main()`

## Competition-safe review checklist

- all ports explicit
- all directions explicit when needed
- wheel diameter and axle track visible near top
- no unused imports in final answer
- no hard-coded sleep values without explanation if accuracy matters
- no mission logic hidden inside global scope
- no unsupported libraries

## Preferred helper names

- `leave_base()`
- `align_on_line()`
- `pick_object()`
- `deliver_object()`
- `return_home()`

## When reviewing user code

Check these first:

- swapped motor direction
- wrong sign on turn angle
- stale calibration constants
- attachment angle reset missing
- line threshold hard-coded without recent calibration
