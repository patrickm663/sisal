# Examples

Larger, real-world-shaped SISAL programs than `Tests/` carries -- useful as
something bigger to compile and run by hand, not wired into `make check`.

## chainladder.sis

The chain ladder method, used in short-term (general/non-life) insurance to
estimate outstanding claim reserves from a cumulative claims development
triangle. Takes no input -- a synthetic 10x10 triangle is built into the
program -- and returns the age-to-age development factors, the completed
triangle, the per-accident-year reserves, the ultimate claims, and the total
reserve, in that order.

Beyond being a plausible-sized program to throw at the compiler, it exercises
a loop shape nothing else in this repo does: projecting an accident year's
unobserved tail is inherently sequential (development period j+1 depends on
the just-computed, possibly-already-projected value at j, not on the
original -- unobserved -- triangle entry), so `CompleteRow` uses a
carry-forward `for initial ... while ... repeat ... returns value of` loop
building up an array with `array_addh`, rather than the independent-iteration
`returns array of` every other array-producing loop in this repo uses.

```sh
sisalc -o chainladder chainladder.sis
./chainladder            # JSON by default
./chainladder -fibre     # or FIBRE
```

Verified against an independent NumPy float32 reimplementation (SISAL's
`real` is single precision) -- exact match on every development factor,
completed-triangle cell, and reserve -- and confirmed bit-identical output
across `-w1`, `-w4` and `-w8`.
