# State machine patterns

## Good state names

Use mission meaning:

- `leave_base`
- `align_to_line`
- `pickup_sample`
- `deliver_sample`
- `return_home`

## Transition questions

For each state answer:

- what marks success?
- what marks failure?
- how long can this state run?
- what is the smallest useful retry?
- where does the robot go next if this state fails?

## Recommended output table

| state | action | success next | failure next | timeout ms |
|---|---|---|---|---|
