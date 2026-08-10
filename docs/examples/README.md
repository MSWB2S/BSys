# Example Manifests

Each `.ini` here is complete and runnable — copy one next to a `src\`
folder and adjust `[file:...]` entries to match your sources.

| File | Demonstrates | Run |
| --- | --- | --- |
| [`minimal.ini`](./minimal.ini) | Smallest possible manifest | `wbsys minimal.ini build` |
| [`dev-verbose.ini`](./dev-verbose.ini) | `pdb`, `compile_msg`, `verbosity=all`, per-file flag overrides | `wbsys dev-verbose.ini build` |
| [`ci-quiet.ini`](./ci-quiet.ini) | `verbosity=errors` for silent-on-success CI logs | `wbsys ci-quiet.ini build` |

<details>
<summary><strong>Diff: minimal → dev-verbose</strong></summary>

```diff
 [project]
 name=hello
 output=build\hello.exe
+objdir=build\obj
+pdb=build\hello.pdb
+verbosity=all
+message=Building ${name} ${version}
+
+[variables]
+version=1.0

 [defaults]
 flags=/EHsc /std:c++17
+defines=WIN32;_DEBUG
+includes=include

 [file:src\main.cpp]
+compile_msg=[CC] Compiling ${file} -> ${object}
```

</details>

See [`../verbosity.md`](../verbosity.md) and
[`../debug-symbols.md`](../debug-symbols.md) for what each added key
does.
