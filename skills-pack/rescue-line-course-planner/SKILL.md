---
name: rescue-line-course-planner
description: adapt lego spike pybricks robots for robocup junior rescue line and advanced precision line courses. use when the user wants to handle gaps, intersections, sharp turns, obstacles, ramps, pickup zones, victim logic, or line-loss recovery in autonomous rescue-line style events.
---

# Rescue Line Course Planner

## Overview

Break rescue-line style courses into reusable perception and recovery modules instead of one continuous fragile loop.

## Workflow

1. Identify the ruleset and course features.
   - precision line only.
   - rescue line with gaps.
   - branches or intersections.
   - obstacles, ramps, or victim zones.
2. Separate the course into modules.
   - line tracking.
   - line-loss recovery.
   - gap crossing.
   - intersection decision.
   - ramp mode.
   - pickup or drop logic if needed.
3. Use `assets/rescue_line_modules.py` as the module skeleton.
4. Define how the robot exits each event.
   - back on line.
   - new heading confirmed.
   - object loaded or released.
5. Return the result in this order.
   - course assumptions.
   - module list.
   - code structure.
   - prioritized test ladder.

## Rules

- Keep event detectors independent from the main controller when possible.
- Recovery behavior must be explicit; do not rely on lucky reacquisition.
- If the course includes ramps or obstacles, lower speed before adding logic.
- When the user mentions victims or evacuation behavior, separate transport logic from line logic.

## Resources

- Use `references/rescue-line-patterns.md` for module design.
- Use `assets/rescue_line_modules.py` as the code scaffold.
