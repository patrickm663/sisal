# Benchmarks

`run-benchmarks.py` compiles the same SISAL programs with two or more installed
builds and times them side by side, so changes to the compiler, the runtime or
the toolchain can be compared rather than guessed at.

It does not ship the programs. Point it at a checkout of
[patrickm663/sisal-benchmarks](https://github.com/patrickm663/sisal-benchmarks),
or any directory containing `.sis` files whose names it recognises (`matmul`,
`matmul_tr`, `nbodies`, `stream`, `mandelbrot`, `integral`, `dcf`).

## Usage

Build and install each configuration you want to compare, to its own prefix:

```sh
./configure --prefix=$HOME/sisal-gcc --disable-fortran
make && make install

./configure --prefix=$HOME/sisal-zig --disable-fortran \
            CC="zig cc" --with-tuning=aggressive
make && make install
```

Then:

```sh
./Benchmarks/run-benchmarks.py \
    --build gcc=$HOME/sisal-gcc \
    --build zig=$HOME/sisal-zig \
    --programs ../sisal-benchmarks
```

```
benchmark    input                       gcc min/med ms          zig min/med ms       zig/gcc
---------------------------------------------------------------------------------------------
matmul_tr    1000                   1028.9 /    1099.1      382.0 /     388.9        0.354x
integral     400000000               523.7 /     532.8      467.1 /     502.9        0.944x
dcf          100000 360               24.7 /      24.8        6.5 /       7.6        0.308x
```

Every build's output is compared against the first build's before anything is
timed, and programs that disagree are reported and excluded. A faster wrong
answer is not a result.

## Reading the numbers

**Compare medians, not minima.** These are wall-clock timings of whole
processes, including startup, on whatever else the machine happens to be doing.

**The default is one worker (`-w1`), on purpose.** That measures code
generation rather than the thread scheduler. Turning up `--workers` on a small
machine adds a lot of variance: on a 4-core box `nbodies` at `-w4` has a
coefficient of variation of 8–16%, which is enough to manufacture an apparent
8% regression that vanishes at `-w1`. If you do raise `--workers`, raise
`--runs` to match.

**Sizes are tuned for a few hundred milliseconds at `-w1`** on a mid-range x86
core. On much faster or slower hardware, edit `DEFAULT_CASES`; anything under
roughly 50 ms is mostly process startup.

`stream`, `mandelbrot` and `dcf` are run with a raised `-ds` (shared data pool),
because they exceed the default and would otherwise fail with `ALLOCATION
FAILURE: increase -ds value`.

## Useful options

| Option | Effect |
| --- | --- |
| `--only NAME ...` | run a subset |
| `--runs N`, `--warmup N` | timed and untimed repetitions |
| `--workers N` | worker threads passed as `-wN` |
| `--sisalc-flags '...'` | extra flags for every `sisalc` invocation |
| `--workdir DIR` | where compiled binaries go |
