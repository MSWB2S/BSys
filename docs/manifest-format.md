# Manifest Format

`wbsys.ini` is a flat, INI-style file. No nesting, no includes, no
conditionals — every value is a string, expanded once through the
`${variable}` substitution engine at print time.

<details>
<summary><strong>Table of contents</strong></summary>

- [Syntax rules](#syntax-rules)
- [`[project]`](#project)
- [`[variables]`](#variables)
- [`[defaults]`](#defaults)
- [`[file:<path>]`](#filepath)
- [Variable expansion](#variable-expansion)

</details>

## Syntax rules

- One `key=value` pair per line.
- Lines starting with `;` or `#` are comments.
- Section headers are `[name]`; a file section is `[file:relative\path]`.
- Whitespace around keys and values is trimmed.
- `flags` values are space-separated (`/EHsc /std:c++17`).
- `defines` and `includes` values are **semicolon**-separated
  (`WIN32;_DEBUG`), not space-separated — this differs from `flags`.

```ini
; comment
[section]
key=value        ; inline comments are NOT supported, keep the whole line clean
```

> [!WARNING]
> There is no escaping. A value containing `=` will be split at the
> **first** `=` only, and a value containing `]` immediately before a
> newline inside a `[header]` line will terminate the header early. Keep
> section headers and keys simple.

## `[project]`

One block, global build configuration.

| Key | Type | Default | Description |
| --- | --- | --- | --- |
| `name` | string | `project` | Project display name, exposed as `${name}` |
| `output` | path | `build\out.exe` | Final linked binary |
| `cl` | path | `cl.exe` | Compiler executable |
| `link` | path | `link.exe` | Linker executable |
| `rc` | path | `rc.exe` | Resource compiler, used for any `[file:...]` ending in `.rc` |
| `objdir` | path | `build\obj` | Root directory for `.obj` output, mirrors source tree |
| `linkflags` | string | `/NOLOGO` | Flags passed verbatim to the linker |
| `message` | template | *(empty)* | Printed once, before the build starts — see [build-messages.md](./build-messages.md) |
| `pdb` | path | *(empty)* | Enables debug symbols — see [debug-symbols.md](./debug-symbols.md) |
| `verbosity` | `normal` \| `all` \| `errors` | `normal` | Console output level — see [verbosity.md](./verbosity.md) |

```ini
[project]
name=WBSys
output=build\WBSys.exe
cl=cl.exe
link=link.exe
objdir=build\obj
linkflags=/NOLOGO
pdb=build\WBSys.pdb
verbosity=normal
message=${company}: Building ${name} ${version} (${configuration}) for ${architecture}
```

## `[variables]`

One block, arbitrary `key=value` pairs available to every `${...}`
template (`message` and `compile_msg`). Not used anywhere else — they
don't affect compiler flags directly.

```ini
[variables]
company=NoJuo
version=1.0
configuration=Debug
architecture=amd64
```

## `[defaults]`

One block, applied to **every** file before its own per-file settings.

| Key | Separator | Applied as |
| --- | --- | --- |
| `flags` | space | passed through unchanged |
| `defines` | `;` | prefixed with `/D` |
| `includes` | `;` | prefixed with `/I` and quoted |

```ini
[defaults]
flags=/EHsc /std:c++17
defines=WIN32;UNICODE
includes=include;third_party\include
```

## `[file:<path>]`

One block **per translation unit**. `<path>` is the source file, relative
to the manifest, and also identifies the object output path
(`objdir`/`<path with .obj extension>`).

A file ending in `.rc` is routed to `rc.exe` instead of `cl.exe`, and
its output is a `.res` (not `.obj`) — see
[resource-files.md](./resource-files.md) for the full behavior. Every
other extension goes through the normal `cl.exe` path.

| Key | Separator | Behavior |
| --- | --- | --- |
| `flags` | space | **appended** after `defaults.flags` |
| `defines` | `;` | **appended** after `defaults.defines` |
| `includes` | `;` | **appended** after `defaults.includes` |
| `compile_msg` | — | Per-file template, see [build-messages.md](./build-messages.md) |

```ini
[file:src\main.cpp]
flags=/W4
defines=ENTRYPOINT
compile_msg=[CC] Compiling ${file} -> ${object}

[file:src\legacy.cpp]
flags=/wd4996
```

> [!NOTE]
> Per-file `flags`/`defines`/`includes` are additive, not overrides — a
> file always gets `defaults` first, then its own list appended.

## Variable expansion

Both `message` (project-level) and `compile_msg` (per-file) run through
the same `${name}` substitution. Available names:

| Variable | Scope | Value |
| --- | --- | --- |
| `${name}` | always | `project.name` |
| `${output}` | always | `project.output` |
| `${objdir}` | always | `project.objDir` |
| `${cl}` | always | `project.clExe` |
| `${link}` | always | `project.linkExe` |
| `${file}` | per-file only | source path being compiled |
| `${object}` | per-file only | resolved `.obj` output path |
| *(any key from `[variables]`)* | always | its value |

An unresolved `${name}` is left in the output **verbatim** rather than
being blanked out, so typos are easy to spot mid-build.
