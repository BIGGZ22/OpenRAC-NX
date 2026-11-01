#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
#
# Summarises the OPENRAC_PACING_RING dump in a game log (written once, at the L+R+Minus exit):
# tick frames drawn at once, held back, held back and drawn, lost by why; the gaps between swaps
# (the CPU's present calls, not the display's scan-out) and between frame starts; the ticks' behind
# times; and, for the longest swap gaps, the events around them.
#
#   python3 port/tools/pacing_ring_summary.py "openrac-rac1-pal (56).log" [--worst 8]
#
# Standard library only. Reads the log as data.

import argparse
import re
import statistics
import sys

KINDS = {"tick", "drawn", "deferred", "deferred-drawn", "lost-next-tick", "lost-worker", "lost-late",
         "lost-behind", "swap"}


def read_events(path):
    events = []
    in_ring = False
    with open(path, encoding="utf-8", errors="replace") as f:
        for line in f:
            line = line.rstrip("\r\n")
            if "pacing ring:" in line:
                in_ring = True
                continue
            if in_ring:
                parts = line.split(",")
                if len(parts) == 9 and parts[0] in KINDS:
                    kind, tick, rate, at, due, a, b, c, tol = parts
                    events.append(dict(kind=kind, tick=int(tick), rate=int(rate), at=int(at), due=int(due),
                                       a=int(a), b=int(b), c=int(c), tol=int(tol)))
                    continue
                if re.match(r"^\[\w+\]", line):
                    in_ring = False
    return events


def describe(values, unit="ms"):
    if not values:
        return "none"
    v = sorted(values)
    p95 = v[int(0.95 * (len(v) - 1))]
    return (f"n {len(v)} mean {statistics.fmean(v):.2f} p50 {v[len(v) // 2]:.2f} p95 {p95:.2f} "
            f"max {v[-1]:.2f} min {v[0]:.2f} {unit}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("log")
    ap.add_argument("--worst", type=int, default=6, help="swap gaps to show with their events")
    args = ap.parse_args()
    ev = read_events(args.log)
    if not ev:
        print("no OPENRAC_PACING_RING dump found (set OPENRAC_PACING_RING=N and exit with L+R+Minus)")
        return 1
    counts = {}
    for e in ev:
        counts[e["kind"]] = counts.get(e["kind"], 0) + 1
    ticks = counts.get("tick", 0)
    print(f"events {len(ev)}, over {(ev[-1]['at'] - ev[0]['at']) / 1e6:.1f} s")
    print("counts: " + ", ".join(f"{k} {counts.get(k, 0)}" for k in sorted(KINDS)))
    lost = sum(counts.get(k, 0) for k in counts if k.startswith("lost-"))
    if ticks:
        print(f"per 120 ticks: drawn at once {120 * counts.get('drawn', 0) / ticks:.1f}, held back "
              f"{120 * counts.get('deferred', 0) / ticks:.1f}, held back then drawn "
              f"{120 * counts.get('deferred-drawn', 0) / ticks:.1f}, lost {120 * lost / ticks:.2f}, swaps "
              f"{120 * counts.get('swap', 0) / ticks:.1f}")
    swaps = [e for e in ev if e["kind"] == "swap"]
    gaps = [(swaps[i]["at"] - swaps[i - 1]["at"]) / 1000.0 for i in range(1, len(swaps))]
    # A gap over 250 ms is a load or a pause, not pacing.
    steady = [g for g in gaps if g < 250.0]
    print("swap gaps (CPU present calls): " + describe(steady))
    starts = sorted(e["due"] for e in swaps if e["due"] >= 0)
    sgaps = [(starts[i] - starts[i - 1]) / 1000.0 for i in range(1, len(starts))]
    print("frame-start gaps: " + describe([g for g in sgaps if g < 250.0]))
    print("frame start to swap: " + describe([(e["at"] - e["due"]) / 1000.0 for e in swaps if e["due"] >= 0]))
    print("CPU record (render thread): " + describe([e["b"] / 1000.0 for e in swaps if e["b"] > 0]))
    print("GPU (when timed): " + describe([e["c"] / 1000.0 for e in swaps if e["c"] > 0]))
    print("tick behind (after its work): " + describe([e["a"] / 1000.0 for e in ev if e["kind"] == "tick"]))
    for rate in sorted({e["rate"] for e in ev if e["kind"] == "tick"}):
        print(f"  ticks at {rate} a second: {sum(1 for e in ev if e['kind'] == 'tick' and e['rate'] == rate)}")
    # The worst swap gaps, with the events between the two swaps.
    order = sorted(range(1, len(swaps)), key=lambda i: swaps[i]["at"] - swaps[i - 1]["at"], reverse=True)
    shown = 0
    for i in order:
        g = (swaps[i]["at"] - swaps[i - 1]["at"]) / 1000.0
        if g >= 250.0:
            continue
        lo, hi = swaps[i - 1]["at"], swaps[i]["at"]
        between = [e for e in ev if lo <= e["at"] <= hi]
        print(f"\nswap gap {g:.2f} ms at {lo / 1e6:.3f} s:")
        for e in between:
            extra = ""
            if e["kind"] == "tick":
                extra = f" behind {e['a'] / 1000:.2f} ms"
            elif e["kind"] in ("drawn", "deferred"):
                extra = f" cap due {(e['due'] - e['at']) / 1000:+.2f} ms tolerance {e['tol'] / 1000:.2f} ms"
            elif e["kind"] == "swap":
                extra = f" started {(e['due'] - e['at']) / 1000:+.2f} ms record {e['b'] / 1000:.2f} gpu {e['c'] / 1000:.2f}"
            print(f"  {e['at'] / 1e6:10.4f} s {e['kind']:15} tick {e['tick']}{extra}")
        shown += 1
        if shown >= args.worst:
            break
    return 0


if __name__ == "__main__":
    sys.exit(main())
