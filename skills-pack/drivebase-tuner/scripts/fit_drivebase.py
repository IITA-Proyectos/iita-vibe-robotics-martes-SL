#!/usr/bin/env python3
import argparse
import statistics


def mean(values):
    return statistics.fmean(values) if values else None


def parse_series(text):
    if text is None:
        return None
    return [float(x.strip()) for x in text.split(',') if x.strip()]


def main():
    parser = argparse.ArgumentParser(description='Compute corrected wheel diameter and axle track from repeat tests.')
    parser.add_argument('--wheel', type=float, required=True, help='Current configured wheel diameter in mm.')
    parser.add_argument('--axle', type=float, required=True, help='Current configured axle track in mm.')
    parser.add_argument('--straight-command', type=float, default=1000.0, help='Commanded straight distance in mm.')
    parser.add_argument('--straight-actual', required=True, help='Comma-separated measured straight distances in mm.')
    parser.add_argument('--turn-command', type=float, default=360.0, help='Commanded turn angle in degrees.')
    parser.add_argument('--turn-actual', required=True, help='Comma-separated measured turn angles in degrees.')
    args = parser.parse_args()

    straight_actual = parse_series(args.straight_actual)
    turn_actual = parse_series(args.turn_actual)

    mean_straight = mean(straight_actual)
    mean_turn = mean(turn_actual)

    new_wheel = args.wheel * mean_straight / args.straight_command
    new_axle = args.axle * args.turn_command / mean_turn

    print(f'mean_straight_mm={mean_straight:.3f}')
    print(f'mean_turn_deg={mean_turn:.3f}')
    print(f'recommended_wheel_diameter_mm={new_wheel:.3f}')
    print(f'recommended_axle_track_mm={new_axle:.3f}')


if __name__ == '__main__':
    main()
