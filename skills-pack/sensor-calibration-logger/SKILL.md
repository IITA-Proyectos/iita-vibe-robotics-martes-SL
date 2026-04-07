---
name: sensor-calibration-logger
description: calibrate spike sensors and analyze logs for pybricks competition robots. use when the user wants black and white thresholds, color detection, distance cutoffs, imu-based headings, or interpretation of reflection and timing data from repeated tests.
---

# Sensor Calibration Logger

## Overview

Turn raw sensor samples into usable thresholds, noise margins, and logging plans that survive real lighting and field conditions.

## Workflow

1. Identify the sensor and event context.
   - color sensor reflection or color detection.
   - distance trigger.
   - force trigger.
   - heading or orientation using hub sensors.
2. Ask for samples, not guesses.
   - at least 5 black and 5 white reflection samples for line work.
   - multiple measurements at match-like distance and lighting.
3. If the user provides labeled reflection csv, run `scripts/reflection_stats.py`.
4. Recommend the smallest robust trigger rule.
   - threshold.
   - hysteresis or deadband if needed.
   - recheck or confirmation sample if noise is high.
5. Return the result in this order.
   - observed ranges.
   - recommended thresholds.
   - logging plan for next test.
   - what to recalibrate on event day.

## Rules

- Never recommend a threshold from one black and one white sample.
- Mention lighting sensitivity whenever reflection ranges overlap more than expected.
- Prefer reflection over named color for line following unless the task truly depends on color classes.
- Encourage event-day spot checks on the real table or mat.

## Resources

- Use `references/logging-patterns.md` for sample collection and interpretation.
- Use `scripts/reflection_stats.py` on labeled csv data.
- Use `assets/reflection_log.csv` as the default csv format.
