---
name: competition-debugger
description: debug inconsistent autonomous runs on lego spike pybricks robots. use when a competition robot works sometimes, misses turns, drifts, loses the line, mis-times an attachment, or behaves differently between practice and match conditions.
---

# Competition Debugger

## Overview

Diagnose inconsistent behavior by narrowing the fault tree quickly and recommending the smallest next experiment that can prove or disprove a likely cause.

## Workflow

1. Collect evidence in this order.
   - symptom.
   - frequency.
   - last successful version.
   - recent mechanical or code changes.
   - whether the failure is directional, speed-dependent, battery-dependent, or table-dependent.
2. Match the symptom against `references/fault-tree.md`.
3. Rank causes by probability and test cost.
4. Recommend one change at a time.
5. Return the result in this order.
   - likely causes ranked.
   - quickest discriminating test.
   - code or build change only if justified.
   - validation steps before the next official run.

## Rules

- Do not suggest ten simultaneous changes.
- Prefer tests that separate mechanical, sensing, and logic faults.
- If the robot fails only after a first mission, inspect reset assumptions first.
- If the robot fails only at higher speed, mention slip and settling time early.
- Ask for a short video or exact failure moment when the description is too vague to rank causes confidently.

## Resources

- Use `references/fault-tree.md` as the main diagnosis map.
