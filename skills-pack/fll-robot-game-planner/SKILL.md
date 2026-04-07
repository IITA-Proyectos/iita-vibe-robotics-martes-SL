---
name: fll-robot-game-planner
description: plan first lego league robot game runs for lego spike robots using pybricks. use when the user wants to break the robot game into launch runs, prioritize missions, design attachments, or generate reliable code and practice plans for the current or past fll challenge.
---

# FLL Robot Game Planner

## Overview

Plan FLL robot game runs as a set of launch-return cycles, attachments, and code priorities that maximize reliable points inside the match time.

## Workflow

1. Confirm the season and challenge materials.
   - ask for the rulebook, updates, and mat or mission model photos if available.
   - identify whether the team wants a competitive route or a teaching route.
2. Decompose the table into launch runs.
   - missions near the same corridor.
   - missions sharing the same attachment.
   - missions that can be recovered by returning to base.
3. Rank each run.
   - expected points.
   - setup complexity.
   - reset cost.
   - practice burden.
4. Produce a table-ready plan.
   - attachment per run.
   - launch area setup.
   - main path.
   - return plan.
5. Convert the plan into code priorities and a practice script.

## Rules

- Prefer runs that start and end clearly in base when reliability matters.
- Keep attachment changes simple and fast.
- Separate exploratory high-risk missions from the first consistent scoring package.
- If a mission depends on model tolerance, call out verification and reset steps explicitly.

## Output Pattern

### Run map
### Attachment plan
### First competition-ready package
### Stretch goals
### Practice order

## Resources

- Use `references/fll-run-planning.md` for launch planning logic.
- Use `assets/fll_run_card.md` as the run card template.
