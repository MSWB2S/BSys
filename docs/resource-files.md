# Resource Files (`.rc`)

Any `[file:...]` section whose path ends in `.rc` (case-insensitive) is
compiled with `rc.exe` instead of `cl.exe`. No separate section syntax —
it's the same `[file:<path>]` block you'd use for a `.cpp`, dispatched by
extension.

```ini
[file:src\app.rc]
compile_msg=[RC] app -> ${object}
```

## What changes vs a `.cpp` file

| | `.cpp` | `.rc` |
| --- | --- | --- |
| Compiler | `project.cl` (`cl.exe`) | `project.rc` (`rc.exe`) |
| Object output | `<objdir>\...\file.obj` | `<objdir>\...\file.res` |
| Define flag | `/D` | `/d` |
| Include flag | `/I` | `/i` |
| `project.defaultFlags` applied? | yes | **no** — `cl` flags like `/EHsc /std:c++17` aren't valid `rc.exe` arguments |
| `file.flags` applied? | yes | yes (use this for `rc`-specific flags like `/v`) |
| `project.pdb` (`/Zi /Fd`)? | yes | no — resource compilation has no debug symbols |
| Progress line | `[cl]   src\file.cpp` | `[rc]   src\file.rc` |
| Result fed to `link.exe` | as an object | as a `.res` — MSVC's linker accepts `.res` files directly alongside `.obj` files, no extra linker flag needed |

`project.defaultDefines` and `project.defaultIncludes` **are** shared
between both — a `WIN32` define or a shared `include\` path usually
applies equally to your resource script and your C++.

```mermaid
flowchart LR
    A["[file:src\\app.rc]"] --> B{".rc"?}
    B -->|yes| C["rc.exe /nologo /d.. /i.. /fo\"...res\" app.rc"]
    B -->|no| D["cl.exe /c ... /Fo\"...obj\" file.cpp"]
    C --> E["...res"]
    D --> F["...obj"]
    E --> G["link.exe ... \"...res\" \"...obj\" /OUT:..."]
    F --> G
```

## Example

```ini
[project]
name=app
output=build\app.exe
rc=rc.exe

[defaults]
flags=/EHsc /std:c++17
includes=include

[file:src\main.cpp]

[file:src\app.rc]
compile_msg=[RC] Compiling ${file} -> ${object}
```

```text
[cl]   src\main.cpp
[RC] Compiling src\app.rc -> build\obj\src\app.res
[link] -> build\app.exe
Build OK: build\app.exe
```

`main.cpp`'s `.obj` and `app.rc`'s `.res` both end up in `objectFiles`
and both get passed to `link.exe` on the same command line — the linker
doesn't care which compiler produced which input.

> [!NOTE]
> A typical `.rc` file `#include`s both its own header (icon/version IDs)
> and `<windows.h>`-family headers like `winres.h`. Those come from the
> Windows SDK, not from your project's `includes=` — make sure `rc.exe`
> is being run from a Developer Command Prompt (or equivalent
> environment) where the SDK's include paths are already set, same as
> `cl.exe` needs for `<windows.h>`.

See
[`../Test/multiproject/app/app.ini`](../Test/multiproject/app/app.ini)
for a full working example that compiles a version-info resource
alongside `main.cpp` into the same `.exe`.
