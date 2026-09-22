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

## csv.sis

Loading a CSV dataset in SISAL. There's no in-language file-open call --
SISAL is purely functional, and the runtime's `_READ`/`_PIPE`/`_STDIN`
builtins (`Runtime/Sharedlib/READ.c`) exist only for a hand-written C
driver to call; nothing in the frontend resolves those names from SISAL
source. The way that *is* supported is `sisalc`'s `-fileio` flag: compiled
in, it changes a `character` array parameter of `Main` from FIBRE's usual
quoted-string input format to a raw slurp of stdin, so redirecting a file
at the program's stdin hands it the file's exact bytes as an ordinary
SISAL string. Everything from there -- splitting into lines, splitting
each line into fields, parsing the numbers, skipping the header row and
any blank lines -- is plain SISAL, built from a handful of small, reusable
pieces (`Slice`, `SplitOnChar`, `ParseReal`, `ParseCSV`), then column sums
and means over the parsed table as a minimal demonstration that the
parsed values are real numbers, not just correctly-shaped text.

```sh
sisalc -fileio -o csv csv.sis
./csv < claims.csv
```

`claims.csv` is `-text` in `.gitattributes`: `csv.sis` splits lines on a
literal `character(10)` (LF), so a CRLF-converted Windows checkout would
leave a stray `\r` on every field.

Writing this surfaced a real bug, now fixed in `Runtime/Sharedlib/vectorIO.c`:
`WriteCharVector`'s JSON output only rendered a `character` array as a JSON
string when its lower bound was exactly 1, which is true of most arrays
built by a `for ... returns array of` loop but not of a substring sliced
out at some other position -- exactly what `Slice` produces for every field
after the first. Confirmed against a standalone test (splitting
`"12,34,foo,"` on `,`): before the fix, only the first and the (empty,
trivially LoBound-1) last field printed as strings, the rest as
`["3","4"]`-style arrays of one-character strings. FIBRE's own text output
was and still is unaffected by design -- `ReadCharVector`'s quoted-string
case always rebuilds at LoBound 1, so quoting a non-1-bound array there
would silently change its bounds on a read back through FIBRE, which
doesn't apply to JSON (there's no JSON read path).
