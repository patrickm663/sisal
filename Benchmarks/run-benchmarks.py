#!/usr/bin/env python3
"""Compare SISAL builds by compiling and timing the same programs with each.

Point it at two or more installed prefixes and a directory of .sis programs:

    ./Benchmarks/run-benchmarks.py \\
        --build gcc=$HOME/sisal-gcc \\
        --build zig=$HOME/sisal-zig \\
        --programs ../sisal-benchmarks

Each named build must be a `make install` prefix, so that <prefix>/bin/sisalc
exists.  Programs are compiled once per build and then timed; results are
checked against the first build before any timing is reported, because a
faster wrong answer is not a result.

On measurement noise: these are wall-clock timings of whole processes, so they
include process startup and are sensitive to whatever else the machine is
doing.  The default is a single worker (-w1).  That is deliberate -- it is the
setting that compares code generation rather than the thread scheduler.  On a
4-core machine, nbodies at -w4 has a coefficient of variation of 8-16%, which
is wide enough to invent an 8% "regression" that disappears at -w1.  If you
raise --workers, raise --runs with it and read the medians, not the minima.
"""

import argparse
import os
import shutil
import statistics
import subprocess
import sys
import time

# name -> (stdin fed to the program, extra runtime flags)
# Sizes aim for a few hundred milliseconds at -w1 on a mid-range x86 core.
DEFAULT_CASES = {
    "matmul":     ("800",              []),
    "matmul_tr":  ("1000",             []),
    "nbodies":    ("1000000",          []),
    "stream":     ("20000000 2",       ["-ds1200000000"]),
    "mandelbrot": ("1600 1200 1000",   ["-ds400000000"]),
    "integral":   ("400000000",        []),
    "dcf":        ("100000 360",       ["-ds400000000"]),
}


def find_sources(directory):
    """Locate .sis files, one level deep as sisal-benchmarks lays them out."""
    found = {}
    for root, _dirs, files in os.walk(directory):
        for f in files:
            if f.endswith(".sis"):
                found.setdefault(f[:-4], os.path.join(root, f))
    return found


def compile_one(prefix, source, output, extra_flags):
    sisalc = os.path.join(prefix, "bin", "sisalc")
    if not os.path.exists(sisalc):
        raise SystemExit(f"no sisalc under {prefix} -- did you make install?")
    cmd = [sisalc, "-w"] + extra_flags + ["-o", output, source]
    p = subprocess.run(cmd, capture_output=True, text=True)
    if p.returncode != 0 or not os.path.exists(output):
        sys.stderr.write(f"  compile failed: {' '.join(cmd)}\n{p.stderr[-800:]}\n")
        return False
    return True


def run_once(exe, stdin_text, workers, extra, quiet=True):
    args = [exe, f"-w{workers}"] + (["-z"] if quiet else []) + extra
    start = time.perf_counter()
    p = subprocess.run(args, input=stdin_text, capture_output=True,
                       text=True, timeout=900)
    elapsed = (time.perf_counter() - start) * 1000.0
    if p.returncode != 0:
        raise RuntimeError(f"{exe} exited {p.returncode}: {p.stderr[-300:]}")
    return elapsed, p.stdout


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--build", action="append", required=True, metavar="NAME=PREFIX",
                    help="a named install prefix; repeat for each build to compare")
    ap.add_argument("--programs", required=True, metavar="DIR",
                    help="directory containing the .sis programs")
    ap.add_argument("--workers", type=int, default=1,
                    help="worker threads (default 1; see the note on noise)")
    ap.add_argument("--runs", type=int, default=5, help="timed runs per program")
    ap.add_argument("--warmup", type=int, default=1, help="untimed runs first")
    ap.add_argument("--only", nargs="*", help="limit to these program names")
    ap.add_argument("--workdir", default="bench-work",
                    help="where to put compiled binaries")
    ap.add_argument("--sisalc-flags", default="",
                    help="extra flags passed to every sisalc invocation")
    args = ap.parse_args()

    builds = []
    for spec in args.build:
        if "=" not in spec:
            raise SystemExit(f"--build wants NAME=PREFIX, got {spec!r}")
        name, prefix = spec.split("=", 1)
        builds.append((name, os.path.expanduser(prefix)))

    sources = find_sources(args.programs)
    names = [n for n in DEFAULT_CASES if n in sources]
    if args.only:
        names = [n for n in names if n in args.only]
    if not names:
        raise SystemExit(f"no known benchmark programs found under {args.programs}")

    shutil.rmtree(args.workdir, ignore_errors=True)
    os.makedirs(args.workdir, exist_ok=True)
    extra_compile = args.sisalc_flags.split()

    print(f"compiling {len(names)} programs for {len(builds)} builds...")
    built = {}
    for name in names:
        for bname, prefix in builds:
            exe = os.path.join(args.workdir, f"{name}.{bname}")
            if compile_one(prefix, sources[name], exe, extra_compile):
                built[(name, bname)] = exe

    # Correctness before speed: every build must agree with the first.
    print("checking results agree...")
    disagreed = set()
    for name in names:
        stdin_text, extra = DEFAULT_CASES[name]
        stdin_text = "\n".join(stdin_text.split()) + "\n"
        reference = None
        for bname, _ in builds:
            exe = built.get((name, bname))
            if exe is None:
                continue
            try:
                _, out = run_once(exe, stdin_text, args.workers, extra, quiet=False)
            except Exception as exc:                       # noqa: BLE001
                print(f"  {name}/{bname}: {exc}")
                disagreed.add(name)
                continue
            # Drop the banner line, which names the build's own prefix.
            out = "\n".join(out.splitlines()[1:])
            if reference is None:
                reference = out
            elif out != reference:
                print(f"  {name}: {bname} disagrees with {builds[0][0]}")
                disagreed.add(name)
    if disagreed:
        print(f"  NOT TIMING disagreeing programs: {', '.join(sorted(disagreed))}")

    header = f"{'benchmark':<13}{'input':<18}"
    for bname, _ in builds:
        header += f"{bname + ' min/med ms':>24}"
    if len(builds) > 1:
        header += f"{builds[-1][0] + '/' + builds[0][0]:>14}"
    print()
    print(header)
    print("-" * len(header))

    for name in names:
        if name in disagreed:
            continue
        stdin_text, extra = DEFAULT_CASES[name]
        shown = stdin_text
        stdin_text = "\n".join(stdin_text.split()) + "\n"
        medians, cells = {}, ""
        for bname, _ in builds:
            exe = built.get((name, bname))
            if exe is None:
                cells += f"{'n/a':>24}"
                continue
            for _ in range(args.warmup):
                run_once(exe, stdin_text, args.workers, extra)
            times = [run_once(exe, stdin_text, args.workers, extra)[0]
                     for _ in range(args.runs)]
            medians[bname] = statistics.median(times)
            cells += f"{min(times):>11.1f} /{medians[bname]:>10.1f}"
        ratio = ""
        first, last = builds[0][0], builds[-1][0]
        if len(builds) > 1 and first in medians and last in medians:
            ratio = f"{medians[last] / medians[first]:>13.3f}x"
        print(f"{name:<13}{shown:<18}{cells}{ratio}")

    print()
    print(f"-w{args.workers}, {args.runs} runs after {args.warmup} warmup; "
          "medians are the number to compare.")


if __name__ == "__main__":
    main()
