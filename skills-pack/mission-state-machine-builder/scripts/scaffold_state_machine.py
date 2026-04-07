#!/usr/bin/env python3
import argparse
import json
from pathlib import Path

TEMPLATE_HEADER = '''from pybricks.tools import StopWatch

state_watch = StopWatch()


def run_state_machine():
    state = "{first_state}"
    while state != "done":
        state_watch.reset()
        if state == "{first_state}":
            state = {first_state}_state()
{dispatch_cases}
        else:
            raise RuntimeError(f"Unknown state: {{state}}")

'''

STATE_TEMPLATE = '''def {name}_state():
    # TODO: implement {name}
    # Success -> {success}
    # Failure -> {failure}
    # Timeout -> {timeout_ms} ms
    return "{success}"

'''


def main():
    parser = argparse.ArgumentParser(description='Generate a simple pybricks state machine skeleton from a json file.')
    parser.add_argument('json_file', help='Path to a json file with a top-level states list.')
    args = parser.parse_args()

    data = json.loads(Path(args.json_file).read_text())
    states = data['states']
    if not states:
        raise SystemExit('State list is empty.')

    first = states[0]['name']
    dispatch_cases = []
    for s in states[1:]:
        dispatch_cases.append(
            f'        elif state == "{s["name"]}":\n'
            f'            state = {s["name"]}_state()'
        )

    print(TEMPLATE_HEADER.format(first_state=first, dispatch_cases='\n'.join(dispatch_cases)))
    for s in states:
        print(
            STATE_TEMPLATE.format(
                name=s['name'],
                success=s.get('success', 'done'),
                failure=s.get('failure', 'done'),
                timeout_ms=s.get('timeout_ms', 1000),
            )
        )
    print('if __name__ == "__main__":\n    run_state_machine()')


if __name__ == '__main__':
    main()
