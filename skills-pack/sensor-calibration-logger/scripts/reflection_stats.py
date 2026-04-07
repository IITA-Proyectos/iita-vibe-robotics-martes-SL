#!/usr/bin/env python3
import argparse
import csv
import statistics


def load_rows(path):
    rows = []
    with open(path, newline='') as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(row)
    return rows


def describe(values):
    return {
        'count': len(values),
        'min': min(values),
        'max': max(values),
        'mean': statistics.fmean(values),
    }


def main():
    parser = argparse.ArgumentParser(description='Analyze labeled reflection samples from a csv file.')
    parser.add_argument('csv_file', help='CSV with columns label,value where label is black or white.')
    args = parser.parse_args()

    rows = load_rows(args.csv_file)
    black = [float(r['value']) for r in rows if r['label'].strip().lower() == 'black']
    white = [float(r['value']) for r in rows if r['label'].strip().lower() == 'white']

    if not black or not white:
        raise SystemExit('Need at least one black and one white sample.')

    b = describe(black)
    w = describe(white)
    threshold = (b['mean'] + w['mean']) / 2.0
    margin = min(w['min'] - threshold, threshold - b['max'])

    print(f"black_mean={b['mean']:.3f}")
    print(f"black_min={b['min']:.3f}")
    print(f"black_max={b['max']:.3f}")
    print(f"white_mean={w['mean']:.3f}")
    print(f"white_min={w['min']:.3f}")
    print(f"white_max={w['max']:.3f}")
    print(f"threshold={threshold:.3f}")
    print(f"noise_margin={margin:.3f}")


if __name__ == '__main__':
    main()
