---
name: wro-robomission-strategist
description: plan wro robomission runs for lego spike robots programmed with pybricks. use when the user wants to convert the yearly mission rules into scoring strategy, route order, attachments, risk analysis, run segmentation, or code priorities for autonomous wro attempts.
---

# WRO RoboMission Strategist

## Overview

Turn WRO RoboMission game documents into a scoring strategy, attachment plan, and code roadmap that favors points per unit of risk.

## Workflow

1. Anchor the season first.
   - ask for the year and age group.
   - ask for the mission sheet, q and a updates, or photos if the user has them.
   - do not assume old season details are still valid.
2. Extract competition constraints.
   - start conditions.
   - time limit.
   - robot size checks.
   - randomizations and surprise elements.
3. Score each mission by four factors.
   - points.
   - travel cost.
   - attachment burden.
   - failure risk.
4. Produce three plans.
   - safe run.
   - aggressive run.
   - fallback if one high-value task is dropped.
5. Convert strategy into code priorities.
   - calibrations.
   - navigation modules.
   - attachments.
   - verification steps.

## Rules

- Favor missions with strong points and low reset cost early.
- Penalize missions that require precise alignment after long travel unless the robot already proves that accuracy.
- Keep the first competition-ready run simpler than the final ambition run.
- Highlight any season rule that changes the route logic.

## Output Pattern

### Constraints extracted
### Mission ranking
### Safe run
### Aggressive run
### Code priorities

## Resources

- Use `references/wro-run-planning.md` for scoring logic.
- Use `assets/wro_run_card.md` as the planning template.
