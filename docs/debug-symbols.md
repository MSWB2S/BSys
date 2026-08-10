# Debug Symbols (`pdb`)

Setting `pdb` under `[project]` turns on program database generation for
the whole build — every file compiles with `/Zi`, and the linker emits a
matching `/DEBUG /PDB:` pair.

```ini
[project]
pdb=build\WBSys.pdb
```

## What it expands to

| Stage | Flag added | Only when |
| --- | --- | --- |
| Compile (`cl.exe`, per file) | `/Zi /Fd"build\WBSys.pdb"` | `pdb` is non-empty |
| Link (`link.exe`) | `/DEBUG /PDB:"build\WBSys.pdb"` | `pdb` is non-empty |

Leaving `pdb` unset (the default) omits both flags entirely — release
builds don't pay for symbol generation.

```mermaid
flowchart LR
    subgraph "pdb set"
    A1["cl.exe /c ... /Zi /Fd\"build\\WBSys.pdb\" ..."] --> A2["link.exe ... /DEBUG /PDB:\"build\\WBSys.pdb\""]
    end
    subgraph "pdb unset"
    B1["cl.exe /c ..."] --> B2["link.exe ..."]
    end
```

> [!NOTE]
> All object files compiled in the same build share **one** `.pdb`
> (`/Fd` points every translation unit at the same file) — this is the
> standard MSVC pattern for multi-file projects and avoids
> `fatal error C1041` from conflicting PDB writers.

## Example: debug vs release manifests

<table>
<tr><th>Debug</th><th>Release</th></tr>
<tr valign="top">
<td>

```ini
[project]
name=WBSys
output=build\debug\WBSys.exe
objdir=build\debug\obj
pdb=build\debug\WBSys.pdb

[defaults]
flags=/EHsc /std:c++17 /Od /MDd
```

</td>
<td>

```ini
[project]
name=WBSys
output=build\release\WBSys.exe
objdir=build\release\obj
; no pdb= key — no symbols emitted

[defaults]
flags=/EHsc /std:c++17 /O2 /MD
```

</td>
</tr>
</table>

Swap between them with two manifests and `wbsys <manifest>.ini build`,
or by templating one manifest through your own tooling and rewriting
just the `[variables]` block.
