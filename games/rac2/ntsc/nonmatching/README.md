# Near misses

C that compiles close to a function but not to its bytes: a starting point, never linked
and never counted. Move a file into `src/` once `tools/build.py` places it.

| Function | Size | Bytes that differ |
| --- | ---: | ---: |
| [`func_0012FB48`](boot/func_0012FB48.c) | 20 | 3 |
| [`func_0028B288`](boot/func_0028B288.c) | 44 | 14 |
| [`func_002C9A60`](boot/func_002C9A60.c) | 20 | 9 |

## USA v2.00 trials

`v2.00/`: small functions written against the USA v2.00 release (`SCUS_972.68_v2.00.iso`, see
docs/REGIONS.md), most of them comparing equal there apart from link-dependent fields. Nothing
builds v2.00 yet; many of these functions also exist in v1.01 at other addresses, so they are
starting points for both. `FloatToInt` and the functions like it (an in-place `cvt.w.s $f12,$f12`)
are probably handwritten: no C spelling reproduces them.

## Moved from src/ (2026-10-11)

C carried over from the old layout that compiles but does not link at its places: usually a
level address written as a number, which only fits one level.

- [`func_0011EA38`](core/func_0011EA38.c)
- [`func_00121B18`](core/func_00121B18.c)
- [`func_00124C98`](core/func_00124C98.c)
- [`func_0012C680`](core/func_0012C680.c)
- [`func_0012C788`](core/func_0012C788.c)
- [`func_0012C878`](core/func_0012C878.c)
- [`func_00130A60`](core/func_00130A60.c)
- [`func_00130A70`](core/func_00130A70.c)
- [`func_00130A80`](core/func_00130A80.c)
- [`func_001317E8`](core/func_001317E8.c)
- [`func_0027F110`](game/func_0027F110.c)
- [`func_00287E98`](game/func_00287E98.c)
- [`func_00288BE8`](game/func_00288BE8.c)
- [`func_00288D70`](game/func_00288D70.c)
- [`func_00290670`](game/func_00290670.c)
- [`func_002985F0`](game/func_002985F0.c)
- [`func_00298780`](game/func_00298780.c)
- [`func_L00_00309AE8`](levels/shared/func_L00_00309AE8.c)
- [`func_L00_00309B10`](levels/shared/func_L00_00309B10.c)
- [`func_L00_00309B58`](levels/shared/func_L00_00309B58.c)
- [`func_L00_00309B80`](levels/shared/func_L00_00309B80.c)
- [`func_L03_0043ACD8`](levels/shared/func_L03_0043ACD8.c)
