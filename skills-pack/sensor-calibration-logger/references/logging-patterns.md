# Sensor logging patterns

## Reflection threshold

Use repeated labeled samples:

- black samples from the darkest intended line region
- white samples from the surrounding floor or mat

Compute:

- `black_mean`
- `white_mean`
- `threshold = (black_mean + white_mean) / 2`
- `margin = min(white_min - threshold, threshold - black_max)`

If margin is small, reduce speed, lower sensor height variation, or recalibrate on-site.

## Logging advice

Log only what helps the next decision:

- timestamp
- reflection
- current state or mode
- speed or turn command

## Event-day checklist

- confirm sensor height
- confirm lighting on table
- confirm line color or mat print quality
- verify threshold with at least one quick black and one quick white sample
