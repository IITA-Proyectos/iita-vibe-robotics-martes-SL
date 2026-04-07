---
name: line-follower-tuner
description: build and tune fast, stable line following for lego spike robots programmed with pybricks. use when the user asks for proportional or pd control, gap recovery, intersections, edge selection, rescue line behavior, or precision line following on black or colored tracks.
---

# Line Follower Tuner

## Overview

Create stable line-following code and tuning plans that match the actual course, sensor geometry, and required competition speed.

## Workflow

1. Classify the course.
   - simple precision line.
   - fast line with smooth turns.
   - rescue line with gaps, branches, or obstacles.
2. Collect baseline parameters.
   - black and white reflection values.
   - chosen edge to follow.
   - sensor placement and height.
   - current base speed.
3. Choose the controller.
   - proportional only for simple moderate-speed tasks.
   - pd when oscillation or corner overshoot matters.
4. Start from `assets/pd_line_follow_template.py` and adapt it.
5. Tune in this order.
   - threshold.
   - base speed.
   - kp.
   - kd.
   - recovery logic for line loss, gaps, or intersections.
6. Return the result in this order.
   - assumptions.
   - code.
   - initial tuning values.
   - next five test runs.

## Rules

- Use reflection measurements, not visual guesses.
- Keep recovery logic separate from the main tracking loop.
- If the robot hunts left and right on straights, reduce speed or increase damping before adding complexity.
- If the robot misses sharp corners, lower speed before increasing kp aggressively.
- For rescue-style tracks, describe event detectors for gaps, branches, and intersections as separate modules.

## Resources

- Use `references/control-patterns.md` for tuning logic.
- Use `assets/pd_line_follow_template.py` as the code base.
