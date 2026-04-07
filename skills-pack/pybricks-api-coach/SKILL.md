---
name: pybricks-api-coach
description: generate, refactor, and review pybricks python for lego spike competition robots. use when the user wants correct pybricks code, port mapping, drivebase setup, reusable subroutines, or safe code patterns for fll, wro, robocup junior rescue line, or other autonomous lego spike tasks.
---

# Pybricks API Coach

## Overview

Turn competition intent into pybricks code that is easy to tune at the table, uses the official api correctly, and stays readable under competition pressure.

## Workflow

1. Identify the robot profile before writing code.
   - hub: primehub or essentialhub.
   - left and right drive motor ports and directions.
   - wheel diameter and axle track.
   - installed sensors and attachment motors.
   - event context: wro, fll, rescue line, precision line following, or generic autonomous run.
2. Decide the request type.
   - new program.
   - refactor an existing program.
   - review for bugs or unsupported api use.
   - extract reusable helpers from a long monolithic script.
3. Build the program around competition-safe structure.
   - constants at the top.
   - hardware initialization in one place.
   - short helper functions for each mission action.
   - one `main()` entry point.
   - explicit comments only where they clarify a design choice.
4. Validate code style against pybricks constraints.
   - use official pybricks imports only.
   - prefer `DriveBase` for two-motor chassis.
   - prefer deterministic blocking maneuvers for mission runs.
   - use `drive()` loops only for sensor-guided behavior such as line following.
   - avoid hidden magic numbers; move them to named constants.
5. Return the result in this order.
   - assumptions and robot constants.
   - complete code.
   - what must be calibrated before match day.
   - shortest next test to run.

## Code Rules

- Keep every function single-purpose.
- Name actions by field meaning, not by motor detail. Prefer `deliver_sample()` over `run_attachment_a()`.
- Put all tunable values in one block near the top.
- Keep mission code readable enough to patch at the table in less than a minute.
- When the user gives incomplete hardware info, state the assumptions explicitly instead of inventing hidden ports.
- If the user asks for line following, routing, or attachment sequencing, call out the matching specialized skill in the answer and still provide valid starter code here.

## Output Pattern

Use this structure unless the user requests a different one:

### Assumptions
- hub
- ports
- wheel diameter
- axle track
- sensors
- attachments

### Program
Provide a full runnable program.

### Tuning checklist
- constants to verify first
- likely failure points
- one quick bench test

## Resources

- Use `references/pybricks-competition-patterns.md` for standard imports, structure, and review checks.
- Use `assets/competition_base.py` as the default starting skeleton when writing a new program.
