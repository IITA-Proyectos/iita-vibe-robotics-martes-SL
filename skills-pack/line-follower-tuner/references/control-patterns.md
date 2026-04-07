# Line following control patterns

## Core formula

`error = threshold - reflection`

For the opposite edge, flip the sign of the error.

`turn = kp * error + kd * (error - last_error)`

## Tuning order

1. valid threshold
2. stable base speed
3. kp until robot follows the line consistently
4. kd until oscillation reduces and corner entry improves

## Failure signatures

- wide lazy turns: kp too low or speed too high
- rapid zig-zag on straights: kp too high or kd too low
- sharp overshoot in corners: speed too high or kd too low
- loses line after gap: recovery routine missing or too slow

## Recovery idea

When reflection no longer resembles line or floor expectations:

1. remember last turn direction
2. slow down
3. sweep back toward last known line side
4. if line not found, escalate to larger sweep or fallback state
