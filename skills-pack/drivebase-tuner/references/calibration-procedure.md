# Drivebase calibration procedure

## Straight distance

1. Mark a start line.
2. Command a repeatable straight distance such as 1000 mm.
3. Measure actual distance traveled.
4. Repeat at least 3 times.
5. Update wheel diameter using:

`new_wheel_diameter = old_wheel_diameter * actual_distance / commanded_distance`

## Turn angle

1. Command a simple in-place turn such as 360 deg.
2. Measure actual angle.
3. Repeat at least 3 times.
4. Update axle track using:

`new_axle_track = old_axle_track * commanded_angle / actual_angle`

## Interpreting drift

- same distance error in both directions: geometry constant issue likely.
- different behavior left vs right: friction, wheel seating, bent frame, or support wheel bias likely.
- accurate at low speed but poor at high speed: slip or acceleration issue likely.

## Practical sequence

1. geometry
2. acceleration
3. speed
4. mission confirmation
