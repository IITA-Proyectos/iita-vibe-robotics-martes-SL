# Rescue line patterns

## Useful modules

- `track_line()`
- `recover_line()`
- `cross_gap()`
- `handle_intersection()`
- `ramp_mode()`
- `pickup_victim()`
- `drop_victim()`

## Event design questions

For each event decide:

- what sensor signature identifies it?
- when does the robot slow down?
- what confirms success?
- what fallback happens if confirmation fails?

## Test ladder

1. straight line
2. corner
3. gap only
4. intersection only
5. ramp only
6. full combined sequence
