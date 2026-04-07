---
name: attachment-cycle-optimizer
description: design and code reliable attachment cycles for lego spike competition robots using pybricks. use when the user needs motorized arms, lifts, claws, sliders, homing, synchronization with drive motion, or safe reset positions between autonomous runs.
---

# Attachment Cycle Optimizer

## Overview

Design attachment logic that is reliable, easy to re-home, and safe to chain across multiple runs.

## Workflow

1. Describe the mechanism in plain language.
   - motorized arm, lift, slider, claw, trigger, or passive latch.
   - what counts as home, loaded, released, and reset.
2. Split the cycle into named phases.
   - prepare
   - acquire
   - transport hold if needed
   - deliver
   - reset
3. Use `assets/attachment_cycle_template.py` as the starting structure.
4. For each phase, define.
   - target angle or travel
   - required speed
   - whether the drive base must be stopped
   - what to do if alignment is off
5. Return the result in this order.
   - mechanism assumptions.
   - cycle table.
   - code.
   - tests for backlash, missed reset, and double-loading.

## Rules

- Always make home and reset explicit.
- Separate mechanism control from navigation whenever possible.
- Prefer one reliable motor movement over a complicated chained motion.
- Call out backlash and beam flex if timing alone will not solve the problem.

## Resources

- Use `references/attachment-patterns.md` for cycle design.
- Use `assets/attachment_cycle_template.py` as the code base.
