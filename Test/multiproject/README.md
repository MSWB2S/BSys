# Multi-Project Test: DLL + EXE

WBSys builds exactly one linked output per manifest — one `[project]`
block, one `output=`. To produce **multiple** binaries you run multiple
manifests and wire the dependency yourself; this folder is the minimal
version of that pattern.

```text
multiproject/
├── mathlib/                shared library
│   ├── include/mathlib/mathlib.h
│   ├── src/mathlib.cpp
│   └── mathlib.ini          -> build\mathlib.dll + build\mathlib.lib
├── app/                     executable, consumes mathlib + a resource
│   ├── include/resource.h
│   ├── src/main.cpp
│   ├── src/app.rc            -> compiled to build\obj\src\app.res, linked in
│   └── app.ini                -> build\app.exe
├── build-all.bat            builds both, in order, stages the DLL
└── clean-all.bat
```

## Why order matters

`app.ini` links directly against `..\mathlib\build\mathlib.lib`, the
**import library** the DLL link step generates. That file doesn't exist
until `mathlib.ini` has been built at least once — WBSys has no
cross-manifest dependency graph, so this ordering is enforced by
`build-all.bat`, not by the tool itself.

```mermaid
flowchart LR
    A["mathlib.ini build"] -->|"mathlib.dll + mathlib.lib"| B["app.ini build"]
    B -->|"app.exe (unresolved MathAdd/MathMultiply)"| C["copy mathlib.dll next to app.exe"]
    C --> D["app.exe runs"]
```

## How the DLL side is wired

`mathlib.ini`:

```ini
[project]
output=build\mathlib.dll
linkflags=/NOLOGO /DLL /IMPLIB:build\mathlib.lib

[defaults]
defines=MATHLIB_EXPORTS
```

- `/DLL` tells `link.exe` to emit a shared library instead of an `.exe`.
- `/IMPLIB:...` is what makes the `.lib` stub appear — that's the file
  `app.ini` will link against, not the `.dll` itself.
- `MATHLIB_EXPORTS` flips `mathlib.h`'s
  `__declspec(dllexport)`/`__declspec(dllimport)` macro so the DLL's own
  compile exports the symbols instead of importing them.

## How the EXE side is wired

`app.ini`:

```ini
[project]
output=build\app.exe
linkflags=/NOLOGO ..\mathlib\build\mathlib.lib

[defaults]
includes=include;..\mathlib\include
```

- `linkflags` is passed to `link.exe` verbatim, so a raw path to another
  project's `.lib` works exactly like any other linker argument.
- `includes` adds `..\mathlib\include` so `#include "mathlib/mathlib.h"`
  resolves; `MATHLIB_EXPORTS` is **not** defined here, so the header
  correctly picks `dllimport` for this side.

## The version resource

`app.ini` also lists `src\app.rc` as a plain `[file:...]` entry — WBSys
detects the `.rc` extension and routes it through `rc.exe` instead of
`cl.exe`, producing `build\obj\src\app.res`, which then gets passed to
`link.exe` alongside `main.obj` automatically. See
[`../../docs/resource-files.md`](../../docs/resource-files.md) for the
mechanics.

## Running it

```bat
cd Test\multiproject
build-all.bat
app\build\app.exe
```

```text
=== [1/3] Building mathlib.dll ===
NoJuo: Building mathlib.dll
[CC] mathlib -> build\obj\src\mathlib.obj
[link] -> build\mathlib.dll
Build OK: build\mathlib.dll

=== [2/3] Building app.exe ===
NoJuo: Building app.exe (links mathlib.dll)
[CC] app -> build\obj\src\main.obj
[link] -> build\app.exe
Build OK: build\app.exe

=== [3/3] Staging mathlib.dll next to app.exe ===
Build complete: app\build\app.exe

3 + 4 = 7
3 * 4 = 12
```

> [!NOTE]
> The DLL must sit next to `app.exe` (or somewhere on `PATH`) at
> **runtime** — the `.lib` only satisfies the linker at build time.
> `build-all.bat` handles the copy; if you build manually, don't skip
> it.

## Extending this pattern

For a third target — say a second EXE also consuming `mathlib` — add a
sibling folder with its own manifest and one more block to
`build-all.bat`. Each new manifest is independent; the only shared state
is whatever `.lib`/`.dll` paths you reference by relative path in
`linkflags`/`includes`.
