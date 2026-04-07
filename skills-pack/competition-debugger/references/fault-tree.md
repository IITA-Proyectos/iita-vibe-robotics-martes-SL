# Fault tree

## Misses a turn by a consistent amount

Likely causes:
- axle track constant off
- wheel slip during turn
- different tire grip left vs right

## Straight run drifts left or right

Likely causes:
- wheel diameter mismatch or seating issue
- support wheel friction bias
- frame twist
- one motor direction or mounting changed

## Line follower oscillates

Likely causes:
- kp too high
- kd too low
- sensor too far ahead for chosen speed

## Attachment sometimes misses

Likely causes:
- no home or reset confirmation
- backlash under load
- robot not aligned before action
- mechanism still moving when drive resumes

## Works in practice but not on table

Likely causes:
- threshold calibrated under different lighting
- field friction changed
- mat or model placement tolerance changed
- battery level different enough to change dynamics
