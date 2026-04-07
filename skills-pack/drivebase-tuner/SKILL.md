---
name: drivebase-tuner
description: tune straight driving and turning accuracy for lego spike pybricks drive bases. use when the user wants to measure wheel diameter, axle track, drift, overshoot, drivebase settings, or convert test results into corrected constants for competition robots.
---

# Drivebase Tuner

## Overview

Convert repeatable field measurements into better `DriveBase` constants and testing decisions. Prioritize repeatability first, raw speed second.

## Workflow

1. Collect the minimum robot profile.
   - current wheel diameter and axle track.
   - drive motor ports and directions.
   - tire type and support wheel or caster.
   - floor type used for testing.
2. Separate the problem.
   - distance error in straight runs.
   - heading drift in straight runs.
   - turn angle error.
   - instability caused by acceleration or wheel slip.
3. If the user provides test data, run `scripts/fit_drivebase.py`.
4. Interpret results conservatively.
   - fix geometry constants before changing speed.
   - change one family of constants at a time.
   - only increase speed after geometry is credible.
5. Return the result in this order.
   - corrected constants.
   - what the measurements imply.
   - one next calibration test.
   - one match-speed confirmation test.

## Rules

- Do not mix straight-distance calibration and turn calibration into one correction step.
- If drift is directional and not symmetric, call out possible mechanical causes before blaming constants.
- Prefer multiple short repeated tests over one long heroic test.
- If floor conditions changed, warn that constants may not transfer perfectly.

## Output Pattern

### Current constants
### Measured behavior
### Recommended new constants
### Next tests

## Resources

- Use `references/calibration-procedure.md` for the field procedure.
- Use `scripts/fit_drivebase.py` when the user gives measured straight and turn outcomes.
- Use `assets/test_card.md` as the measurement worksheet.
