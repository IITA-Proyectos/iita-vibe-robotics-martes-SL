---
name: mission-state-machine-builder
description: turn autonomous robot tasks into robust pybricks state machines for lego spike competition robots. use when the user wants to split a run into states, retries, timeouts, fallbacks, or reusable mission subroutines for wro, fll, or similar autonomous events.
---

# Mission State Machine Builder

## Overview

Convert a fragile sequence of actions into a state machine that is easier to debug, resume, and adapt when a mission does not go exactly as planned.

## Workflow

1. Break the run into explicit states.
   - launch or leave base
   - align or detect
   - act on mission model
   - verify outcome if needed
   - return or transition
2. Define transition logic for every state.
   - success next state
   - failure next state
   - timeout behavior
   - retry count if allowed
3. If the user gives a structured state list, run `scripts/scaffold_state_machine.py` to create a python skeleton.
4. Keep mission code inside state handlers, not in the dispatcher.
5. Return the result in this order.
   - state table.
   - generated or revised code skeleton.
   - failure handling notes.
   - minimal bench tests per state.

## Rules

- Every state needs an exit condition.
- Timeouts are required for sensor waits and alignment loops.
- Retries must be limited.
- Use states only where they improve clarity; do not create ceremony without value.
- Preserve a simple path to abort safely when the robot is clearly lost.

## Resources

- Use `references/state-patterns.md` for naming and transition guidance.
- Use `scripts/scaffold_state_machine.py` when the user provides a json state list.
- Use `assets/state_machine_example.json` as the default input format.
