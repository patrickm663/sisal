# Stdlib

A prototype module system for SISAL, built as a source-level preprocessor
rather than a compiler change (see the "Known limitation" note in the
top-level README on why: `sisalc`'s frontend has no import/module keyword,
no cross-file symbol resolution, and the backend's "module database"
(`-mdb`) doesn't help -- confirmed directly, not assumed).

## How it works

Every example in this repo already relies on one fact: a single `.sis` file
can hold many function definitions that call each other freely, as long as
each callee is defined *before* its first use in the file (SISAL has no
forward references -- confirmed directly: swapping two functions' order in
an otherwise-working file turns a working call into `Function 'x' is
undefined`) and the file has exactly one `define X` line.

`sisal-cat` is that fact, automated: it concatenates your modules and your
program into one `define Main` compilation unit, in the order you give them,
stripping every file's own `define` line, then hands the result to `sisalc`.

```sh
Stdlib/sisal-cat -o sumlist Stdlib/strings.sis Stdlib/numbers.sis \
    Stdlib/examples/sumlist.sis --flag=-fileio
echo -n "1,2.5,-3,10" | ./sumlist
```

Order matters, in two ways:

- **Modules before the programs that use them.** `numbers.sis` uses the
  `Str` type from `strings.sis`, so `strings.sis` must come first.
- **The file defining `Main` goes last.** Nothing else should call it, so
  this is usually automatic, not something you need to think about.

Get the order wrong and `sisalc` reports a normal, if occasionally cryptic
(SISAL's parser recovers eagerly and can cascade a missing symbol into
several errors), semantic error -- there is no separate failure mode to
debug, because nothing about this is happening at a level `sisalc` doesn't
already understand. `sisal-cat --keep` keeps the concatenated source (it is
kept automatically on any failure) so you can read exactly what got handed
to the compiler.

## Naming

SISAL has no per-file namespacing: once concatenated, every module's names
live in one flat space. Two modules defining the same name, or a module
reusing a name the program using it also wants, is a real conflict `sisalc`
will catch (as a redefinition error) but that you have to resolve by
renaming. The modules here use a short prefix per module (`Str_`, `Num_`) as
a lightweight convention -- not enforced by the tool, just followed by the
files in this directory.

## What's here

| Module | Provides | Depends on |
| --- | --- | --- |
| `strings.sis` | `Str`, `StrArray` types; `Str_Slice`, `Str_SplitOnChar` | nothing |
| `numbers.sis` | `Num_IsDigit`, `Num_ParseReal` | `strings.sis` (the `Str` type) |

`examples/sumlist.sis` and `examples/wordcount.sis` are small, independent
demonstrations that two different programs can genuinely share this code:
`sumlist` pulls in both modules, `wordcount` only `strings.sis`.

`Examples/csv.sis` predates this and still carries its own copies of
`Slice`/`SplitOnChar`/`ParseReal` rather than depending on `Stdlib/` -- it
was left as-is rather than rewritten to depend on a module system that
didn't exist yet when it was written.

## What this doesn't give you

This is concatenation, not a real module system, and it's worth being
honest about the gap:

- **No encapsulation.** Every name from every module is visible everywhere
  in the combined unit, including to other modules, whether that's intended
  or not.
- **No automatic dependency resolution.** You order the files yourself;
  `sisal-cat` doesn't read a module to discover what it needs and reorder
  for you.
- **No versioning or isolation.** Two modules can't each depend on a
  same-named thing meaning something different -- there is exactly one
  namespace.

Closing any of these needs the real thing: forward-declaring an external
function's signature in `Frontend/Front1/sisal.c` so the frontend can accept
a call to a not-yet-defined (or not-yet-parsed) function, which is a
genuinely large change to the biggest, least-tested file in this tree. This
prototype exists to get real reuse across programs today, and to make that
future case for doing the bigger thing with working examples already in
hand, not to replace it.
