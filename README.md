# SISAL

SISAL — Streams and Iteration in a Single Assignment Language — is a functional
language for numerical computing, developed at Lawrence Livermore National
Laboratory from the mid-1980s. Programs are written without mutable state, and
the compiler extracts parallelism from the dataflow rather than from
annotations: a `for` loop over independent work becomes parallel because
nothing in it can alias.

This is the Sisal 14.1 compiler and runtime. `sisalc` compiles `.sis` source
through a series of intermediate forms (IF1 and IF2) into C, then hands that to
your C compiler and links it against the SISAL runtime.

See the [licence](LICENSE.LLNL) and [copyright](COPYRIGHT.1993) for terms.

## Building

You need a C compiler, `make`, and the autotools if you are building from a git
checkout rather than a release tarball.

```sh
./configure
make
make install          # may need sudo, depending on --prefix
```

`make install` is not optional: `sisalc` looks for its parse tables, headers
and runtime library under the install prefix, so it cannot run from the build
directory. Run the test suite afterwards:

```sh
make check
```

### Platforms

CI builds and runs the test suite on Linux, macOS, FreeBSD and OpenBSD (GCC
and Clang), and on Windows under MSYS2/MinGW. There is nothing platform-specific in the
compiler or generated code; the one genuinely platform-sensitive piece is
`sisalc` itself, which spawns each compiler phase as a subprocess -- `fork()`
plus `exec()` everywhere with one, `_spawnvp()` on Windows, which has neither.

Windows needs MSYS2's MinGW64 environment (`pacman -S autoconf automake make
mingw-w64-x86_64-gcc`, then build from an "MSYS2 MinGW64" shell) rather than a
plain Command Prompt or PowerShell: `configure` is a shell script, and the
runtime's parallelism is POSIX threads, which MinGW-w64 provides but MSVC does
not.

**Known limitation:** on macOS (`macos-latest` runners, both GCC and Clang),
`sisalc`'s frontend fails to parse its own test input at `make check` time --
a lexer/parser error that shows up only at runtime on real Darwin hardware.
It does not reproduce on Linux, under QEMU's ARM64 emulation, or under any
`zig cc` cross-compile targeting Darwin, so it appears to be a genuine
Darwin libSystem runtime difference rather than an ARM64 or compile-time
issue; root cause is still open. CI marks the macOS job non-fatal
(`continue-on-error`) so this doesn't block the rest of the matrix while
it's investigated.

A plain `./configure` builds with no warnings on current GCC and Clang. If you
have seen instructions elsewhere passing `-std=gnu89`, `-fcommon`,
`-Wno-implicit-function-declaration`, `-Wno-implicit-int`, `-Wno-int-conversion`
or `LIBS=-lm`, none of those are needed any more.

Useful options:

| Option | Effect |
| --- | --- |
| `--prefix=DIR` | Install under `DIR` instead of `/usr/local` |
| `--disable-fortran` | Skip the Fortran interface if you have no `f77` |
| `--enable-warnings` | Build with a wider warning set, for working on the code |
| `CC=...` | Choose the compiler, e.g. `CC=clang` |

### Choosing a C compiler, and tuning

Whatever compiler `configure` picks is also what `sisalc` invokes on the C it
generates, so it decides how fast your SISAL programs run. A SISAL program's
hot loops end up in that generated C, which is why tuning is set once at
configure time rather than on every compile:

```sh
./configure CC="zig cc" --with-tuning=aggressive
```

`--with-tuning` takes three values:

| Profile | Adds | Use when |
| --- | --- | --- |
| `baseline` (default) | nothing | portable binaries, reproducible builds |
| `native` | `-O3 -march=native`, loop interchange/distribution, vector width and interleave tuning | you are running on the machine you built on |
| `aggressive` | the above plus thin LTO, `-ffast-math`, `-fno-math-errno` | numerical work where relaxed IEEE semantics are acceptable |

Every flag is probed and dropped if your compiler rejects it, so the profiles
are safe to use with GCC — it simply declines the LLVM-specific ones.

`native` and `aggressive` produce binaries tied to the building CPU, and
`aggressive` relaxes floating-point semantics: it permits reassociation and
assumes no NaNs or infinities. Results can differ from a `baseline` build.

Building with clang through [Zig](https://ziglang.org) is worth it for
numerical code. Measured on the benchmarks in
[patrickm663/sisal-benchmarks](https://github.com/patrickm663/sisal-benchmarks),
single worker, `zig cc` 0.16 (clang 21) at `--with-tuning=aggressive` against
gcc 13 at `-O2`, on a Xeon with AVX-512:

| benchmark | gcc `-O2` | `zig cc` aggressive | speedup |
| --- | --- | --- | --- |
| matmul_tr (1000×1000) | 1115 ms | 392 ms | **2.8×** |
| dcf (100k × 360) | 30.0 ms | 7.7 ms | **3.9×** |
| matmul (800×800) | 1498 ms | 1111 ms | **1.35×** |
| integral (4×10⁸) | 531 ms | 504 ms | 1.05× |
| stream (2×10⁷) | 308 ms | 290 ms | 1.06× |
| mandelbrot, nbodies | — | — | no change |

All seven produce results identical to the GCC build.

The profile matters more than it looks: with only `-O3 -march=native
-funroll-loops`, matmul_tr came out *26% slower* than GCC. The loop and
vector-width flags are what turn it around, which is why these ship as whole
profiles rather than a single "optimise harder" switch.

You can still override per compilation:

```sh
sisalc CC=clang CFLAGS="-O3 -march=native" -o prog prog.sis
```

`LD` follows `CC` unless you set it, which is what makes `-flto=thin` link.

## Writing and running a program

```sisal
% Generate an array of squares
define Main

type IntArray = array [integer];

function square(x: integer returns integer)
  x * x
end function

function Main(N: integer returns IntArray)
  for i in 1, N
    returns array of square(i)
  end for
end function
```

Compile it, then feed it its arguments on standard input:

```sh
sisalc -o squares squares.sis
echo 8 | ./squares
```

`sisalc` is quiet on success. Older versions printed a wall of warnings from
the C compiler on every build — pointers formatted with `%x`, a mutex passed
to `%d` — which came from the runtime header every generated program includes.
That header is fixed, so warnings you see now are about your own program.

```
x86_64-unknown-linux-gnu SISAL 1.2 (PThreads) ?.?
[ 1,8: 1 4 9 16 25 36 49 64 ]
```

Input uses FIBRE, a textual format for SISAL values: a function taking two
integers reads two whitespace-separated integers, and an array argument is
written `[ lo,hi: v1 v2 ... ]`.

By default, a compiled program prints its return values as JSON: a scalar
or array of scalars (`integer`, `real`, `double`, `boolean`, `char` or
`null`) prints as the matching JSON type, arrays and streams as JSON arrays
(nested arbitrarily deep), and a `char` array as a JSON string when FIBRE
would also print it as one (turn that off with `-nostrings`). Multiple
return values come out as one JSON array holding all of them, in order, so
the output is always exactly one JSON document. Records and unions still
print as FIBRE regardless -- that part of the format isn't implemented yet.

```
$ echo 8 | ./squares
[[1,4,9,16,25,36,49,64]]
```

Pass `-fibre` to get output in FIBRE's own syntax instead:

```
$ echo 8 | ./squares -fibre
[ 1,8: 1 4 9 16 25 36 49 64 ]
```

`real` and `double` values, in either format, always print in full decimal
notation -- never scientific/exponential -- with as many fractional digits
as the value's own magnitude needs, so very large or very small values
print exactly rather than as `0.000000` or losing precision to a fixed
field width. `-fformat`/`-dformat` still let you pick your own `printf`-style
format string (including scientific notation) for FIBRE output if you want
it; passing either opts that value's FIBRE output out of the new default
formatting.

### Running in parallel

The compiled program takes runtime options of its own. `-usage` lists them all;
the ones you will want first:

| Option | Effect |
| --- | --- |
| `-w<n>` | Use `n` worker threads |
| `-z` | Suppress the program's output (for timing) |
| `-r` | Append resource usage to `s.info` |
| `-gss` | Guided self-scheduling for loops |
| `-cached` | Cache-oriented loop scheduling |
| `-strided` | Strided loop scheduling |

```sh
echo 1000 | ./matmul -w$(nproc) -gss -z
```

### Compiler options

`sisalc --help` lists everything. `-IF1`, `-OPT`, `-MEM`, `-UP`, `-PART` and
`-C` each stop after the corresponding stage, which is how you look at what the
optimiser did; `-keep` retains the intermediates rather than deleting them.

Every array read and write is bounds-checked by default, each one costing a
pair of comparisons plus the error-reporting path they guard. On an
array-heavy program this is not incidental overhead: profiling a 500×500
matrix multiply showed bounds checks alone accounting for roughly 60% of
the instructions executed in the hot loop, and `sisalc -no-bounds` took
that program from 0.39s to 0.18s, a 2.2x speedup, with identical results.
`-no-bounds` is worth trying on any array-bound program once it's already
correct -- an out-of-bounds access becomes undefined behaviour instead of
a clean `ARRAY SUBSCRIPT VIOLATION` diagnostic, so it trades a safety net
for speed rather than being free.

**Known limitation: no way to split a program across multiple `.sis`
files.** Each file compiles as a fully independent unit -- there is no
`import`/`module`/`use` keyword, and one file's `Main` cannot call a
function defined in another file by name (confirmed directly: pre-compiling
the callee to `.if1` and linking it in, and passing both files to `sisalc`
in either order, both fail at the frontend with `Function 'x' is
undefined`). The only supported way for independently-compiled SISAL code
to call other independently-compiled SISAL code is through the C FLI
(`-forC`/`-externC`; see `Tests/modules`), which means going through a
pointer-based C calling convention rather than a native SISAL call.

The backend does have an internal "module database" (`if2part -X<file>`,
tested directly: it does write and read a small file recording each
compile's top-level names), but it isn't what the name suggests. It's
reachable only by hand-driving `if2part` directly on a `.up` intermediate
file -- `sisalc` never exposes it, and never passes a `-X` flag through
to `if2part` (confirmed by reading `Tools/sisalc/if2part.c`'s fixed
argument list) -- and even then it's read and written entirely within
`Backend/If2part`, a stage that runs after the frontend (`Frontend/Front1`)
has already fully parsed and type-checked the whole program. Frontend
`Function undefined` errors happen before that stage ever runs, so this
mechanism can't resolve them; it looks like bookkeeping for splitting one
already-compiled program's generated C across multiple `if2part`/`if2gen`
invocations, not a cross-file import mechanism. A real module system
would need new frontend support (forward-declaring an external function's
signature, most likely) -- a genuinely large change to
`Frontend/Front1/sisal.c`, the biggest and least-tested file in this tree,
not an extension of the existing module database.

`Stdlib/` is a source-level workaround in the meantime: a script that
concatenates SISAL modules into one compilation unit before handing it to
`sisalc`, since a single file with several functions calling each other
already works. See [Stdlib/README.md](Stdlib/README.md) for how it works
and what it doesn't give you.

## Examples

- [patrickm663/hello-sisal](https://github.com/patrickm663/hello-sisal) —
  small programs to start from
- [patrickm663/sisal-benchmarks](https://github.com/patrickm663/sisal-benchmarks)
  — matrix multiply, mandelbrot, n-body, STREAM, discounted cash flow, with
  timings and a container build
- [Examples/](Examples/) in this repo — larger, real-world-shaped programs,
  not run by `make check`
- `Tests/` in this repo — the programs `make check` runs

`Benchmarks/run-benchmarks.py` compiles the same programs with two or more
installed builds and times them side by side; see
[Benchmarks/README.md](Benchmarks/README.md). It is how the table above was
produced.

## Further reading

Raymond's [SISAL page](https://kestrel.nmt.edu/~raymond/software/sisal/sisal.xhtml)
collects the original manuals and papers.
