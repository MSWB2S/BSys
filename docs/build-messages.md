# Build Messages

BSys prints two kinds of message, both driven by the same `${...}`
template engine described in
[manifest-format.md#variable-expansion](./manifest-format.md#variable-expansion).

| Message | Key | Section | Fires |
| --- | --- | --- | --- |
| Global | `message` | `[project]` | once, before the first file compiles |
| Per-file | `compile_msg` | `[file:<path>]` | once per file, right before that file compiles |

## Global message

```ini
[project]
message=${company}: Building ${name} ${version} (${configuration}) for ${architecture}
```

```text
NoJuo: Building WBSys 1.0 (Debug) for amd64
```

Printed a single time at the top of the build, regardless of how many
files there are.

## Per-file message

```ini
[file:src\main.cpp]
compile_msg=[CC] Compiling ${file} -> ${object}
```

```text
[CC] Compiling src\main.cpp -> build\obj\src\main.obj
```

`${file}` and `${object}` are **only** populated for `compile_msg` — using
them in the global `message` leaves them blank, since there's no single
file to resolve them against at that point in the build.

> [!IMPORTANT]
> Setting `compile_msg` on a file changes more than just what prints — it
> also switches that file's default `[cl]   src\...` line and raw
> compiler console output **off**, unless overridden by `verbosity`. See
> [verbosity.md](./verbosity.md) for the full interaction table.

## Per-file overrides the global message where it matters

<table>
<tr><th>Has <code>compile_msg</code>?</th><th><code>verbosity</code></th><th>What prints for that file</th></tr>
<tr><td>No</td><td><code>normal</code> (default)</td><td><code>[cl]   src\file.cpp</code> + live compiler output</td></tr>
<tr><td>Yes</td><td><code>normal</code> (default)</td><td>only the <code>compile_msg</code> line</td></tr>
<tr><td>either</td><td><code>all</code></td><td>everything — <code>compile_msg</code> (if set) + <code>[cl]</code> line + live compiler output</td></tr>
<tr><td>either</td><td><code>errors</code></td><td>nothing on success; on failure, the <code>[cl]</code> line + captured compiler output</td></tr>
</table>

## Worked example

```ini
[project]
name=WBSys
message=${company}: Building ${name} ${version}

[variables]
company=NoJuo

[file:src\main.cpp]
compile_msg=[CC] main -> ${object}

[file:src\utility.cpp]
```

```text
NoJuo: Building WBSys 1.0
[CC] main -> build\obj\src\main.obj
[cl]   src\utility.cpp
Microsoft (R) C/C++ Optimizing Compiler ...
utility.cpp
```

`main.cpp` used its own message and stayed quiet otherwise;
`utility.cpp` had no `compile_msg`, so it fell back to the default
`[cl]` line with normal compiler output.
