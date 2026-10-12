# Near misses

C that compiles close to a function but not to its bytes: a starting point, never linked
and never counted. Move a file into `src/` once `tools/build.py` places it.

| Function | Size | Bytes that differ |
| --- | ---: | ---: |
| [`func_0012FB48`](boot/func_0012FB48.c) | 20 | 3 |
| [`func_00278E78`](game/func_00278E78.c) | 48 | C candidate emits 40 bytes; control flow and global address sequence differ |
| [func_00282C50](game/func_00282C50.c) | 8 | 0 of 28 exact; jr ra matches, but retail delay slot is raw 0x460D6029 and C emits abs.s f0,f13 |
| [`func_00288970`](game/func_00288970.c) | 64 | Candidate is 68 bytes; retail branch-likely delay-slot store is missing |
| [`func_0028B288`](boot/func_0028B288.c) | 44 | 14 |
| [`func_00295870`](game/func_00295870.c) | 68 | Candidate is 64 bytes; mask is not in the retail branch delay slot |
| [`func_0029B658`](game/func_0029B658.c) | 60 | 60-byte candidate swaps the retail global-pointer and saved-object registers |
| [`func_0029B810`](game/func_0029B810.c) | 52 | Candidate is 48 bytes; retail has an extra pointer copy before jr |
| [`func_0029DC58`](game/func_0029DC58.c) | 52 | 26 bytes differ; retail uses bnel with a zero delay-slot result then tests bit 27 |
| [`func_0029FAE0`](game/func_0029FAE0.c) | 60 | 4 bytes differ; retail uses literal lui high halves, not %hi relocations |
| [`func_002A0380`](game/func_002A0380.c) | 60 | 2 bytes differ; table load and branch use v1 instead of retail v0 |
| [`func_002A0488`](game/func_002A0488.c) | 48 | 4 bytes differ; store placement at +0x24 is outside jr delay slot |
| [`func_002C38A0`](game/func_002C38A0.c) | 64 | Candidate is 56 bytes; retail begins with stack adjustment and nop |
| [`func_002C5020`](game/func_002C5020.c) | 68 | Candidate is 72 bytes; retail GP-relative load needs file-level -G8 |
| [`func_002C9A60`](boot/func_002C9A60.c) | 20 | 9 |
| [`func_002C9C60`](game/func_002C9C60.c) | 44 | 8 |
| [`func_002CC528`](game/func_002CC528.c) | 40 | 4 |
| [`func_002D33C0`](game/func_002D33C0.c) | 60 | 2 instructions differ; global-address lui and stack adjustment order differs |
| [`func_002E5F40`](game/func_002E5F40.c) | 32 | 1 |
| [`func_002FCF90`](game/func_002FCF90.c) | 52 | 12 bytes differ; retail schedules zero stores in the return delay slot |
| [func_0031AAB0](game/func_0031AAB0.c) | 88 | check.py 3/3, but level 00 cannot place it: needs .text.func_0031AB08 at its place, which is not C |
| [`func_0031E528`](game/func_0031E528.c) | 52 | 19 bytes differ; loop pointer schedule and beql delay-slot store differ |
| [`func_00327D08`](game/func_00327D08.c) | 72 | Candidate is 76 bytes; retail threshold load is GP-relative |
| [func_00341E58](game/func_00341E58.c) | 52 | check.py 28/28, but level 00 cannot place it: needs .text.func_00341E90 at its place, which is not C |
| [func_0034E478](game/func_0034E478.c) | 64 | check.py 28/28, but level 00 cannot place it: needs .text.func_0034E4B8 at its place, which is not C |
| [func_00351A38](game/func_00351A38.c) | 72 | check.py 28/28, but level 00 cannot place it: needs .text.func_00351938 at its place, which is not C |

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
- [`func_00288BE8`](game/func_00288BE8.c) â€” recovered in [maintained C](../src/game/00288BB8-g8.c); the earlier draft is retained.
- [`func_00288D70`](game/func_00288D70.c)
- [`func_00290670`](game/func_00290670.c)
- [`func_002985F0`](game/func_002985F0.c) â€” recovered in [maintained C](../src/game/002985F0-g8.c); the earlier draft is retained.
- [`func_00298780`](game/func_00298780.c) â€” recovered in [maintained C](../src/game/00298780-g8.c); the earlier draft is retained.
- [`func_L00_00309AE8`](levels/shared/func_L00_00309AE8.c) â€” recovered in [maintained C](../src/levels/shared/L00_00309AE8-g8.c); the earlier draft is retained.
- [`func_L00_00309B10`](levels/shared/func_L00_00309B10.c) â€” recovered in [maintained C](../src/levels/shared/L00_00309AE8-g8.c); the earlier draft is retained.
- [`func_L00_00309B58`](levels/shared/func_L00_00309B58.c) â€” recovered in [maintained C](../src/levels/shared/L00_00309AE8-g8.c); the earlier draft is retained.
- [`func_L00_00309B80`](levels/shared/func_L00_00309B80.c) â€” recovered in [maintained C](../src/levels/shared/L00_00309AE8-g8.c); the earlier draft is retained.
- [`func_L03_0043ACD8`](levels/shared/func_L03_0043ACD8.c) â€” recovered in [maintained C](../src/levels/shared/L03_003C5B28-g8.c); the earlier draft is retained.
- [`func_002985F0`](game/func_002985F0.c)
- [`func_00298780`](game/func_00298780.c)
- [`func_L00_00309AE8`](levels/shared/func_L00_00309AE8.c)
- [`func_L00_00309B10`](levels/shared/func_L00_00309B10.c)
- [`func_L00_00309B58`](levels/shared/func_L00_00309B58.c)
- [`func_L00_00309B80`](levels/shared/func_L00_00309B80.c)
- [`func_L03_0043ACD8`](levels/shared/func_L03_0043ACD8.c)

## Small level functions, first Haiku trial (2026-10-12)

Left by 16 Haiku workers after a bounded effort each (docs/AGENT_WORKFLOW.md); the note is the
worker's own. Several are not C problems: rows of the catalogue that are not functions, and
handwritten routines.

| Function | Size | What the worker found |
| --- | ---: | --- |
| [`func_L00_00309DB0`](levels/func_L00_00309DB0.c) | 28 | 7 instructions in retail, 12 in ours; no jr ra in the retail bytes (addiu sp,+0x20 / daddu / addiu sp,+0x40), so no C of this shape can equal them. Catalogue size or split is suspect. Stub below is a guess only. |
| [`func_L00_0031BE88`](levels/func_L00_0031BE88.c) | 36 | 28 bytes against retail's 36: retail is a beql on the null test with the 0 result in its delay slot, then a bnez with addiu 1 / daddu 0 tail; the compiler gives beqz or bnel plus sltu, so the branch-likely shape is missing. |
| [`func_L00_0032A088`](levels/func_L00_0032A088.c) | 48 | 1 of 12 instructions differs (27 places, all at +0x10): retail adds the element address as base + index (addu a2,a1); the compiler emits index + base (addu a1,a2). Reordering the operands, a cast, or a base local did not change it. |
| [`func_L00_00341798`](levels/func_L00_00341798.c) | 28 | No attempt: all 7 instructions would differ. The retail bytes at 0x341798 are not code: seven words 0x27bd0030, 0, 0x27bd0050, 0, 0x27bd0080, 0, 0x27bd0020 (addiu sp,sp,imm with nops between), directly after the epilogue of the function end… |
| [`func_L00_0037F790`](levels/func_L00_0037F790.c) | 84 | 84 bytes, 1 instruction differs: retail loads $a0 = 0 again before $a1/$a2 for the second call (the first call's delay slot holds one $a0 = 0 too), so retail has 21 instructions and this has 20; GCC puts that move in the delay slot of the s… |
| [`func_L00_00380058`](levels/func_L00_00380058.c) | 116 | 116 bytes, 3 instructions differ: the loop table pointer and the counter swap registers (retail: pointer in $s0 loaded with lui/addiu, counter in $s1; this gives counter $s0, pointer $s1 via a copy from $v0). Same result with for, do-while … |
| [`func_L00_00384C20`](levels/func_L00_00384C20.c) | 132 | 132 bytes, 4 instructions differ (128 bytes here): the two do-while loops have the right shape, but GCC loads each entry into $a0 and tests it there, while retail loads it into $v0, tests it, then copies it to $a0 in the branch delay slot, … |
| [`func_L00_0042A4E8`](levels/func_L00_0042A4E8.c) | 24 | 0 of 6 instructions match: retail is three `jr $ra; nop` pairs (24 bytes) with no prologue. A single C function cannot emit three bare returns; the catalogue row likely merged three empty stubs. |
| `func_L00_00428EE0` | 32 | no attempt: the bytes start mid-function, probably the tail of its neighbour |
| [`func_L00_0042A500`](levels/func_L00_0042A500.c) | 32 | 8 instructions; 3 differ: the table-base load comes out as `lui $v1,0; lw $v0,0($v1)` (retail: `lui $v0,0x1B; lw $v0,0x1844($v0)`) and the sll/lw order differs. The two stores already match. |
| [`func_L00_0042B6A8`](levels/func_L00_0042B6A8.c) | 32 | 8 instructions, 40 bytes emitted against 32: retail computes the data address gp-relative (`addiu $v0,$gp,-0x1388`) and stores it in the call delay slot; -G0 gives lui/addiu and a frame that saves a0 in $v1. |
| [`func_L00_0042B6C8`](levels/func_L00_0042B6C8.c) | 32 | 8 instructions, 40 bytes emitted against 32: same shape as func_L00_0042B6A8 (`addiu $v0,$gp,-0x13A8`), same gp-relative address problem under -G0. |
| [`func_L00_002EE058`](levels/func_L00_002EE058.c) | 24 | from src/levels/shared/L00_002E5918.c: 6 retail instructions, C compiles to 5 (ld/sd pairs). Missing: retail's two ldc2 quad loads, a COP2 VU0 macro-mode op (.word 0x4BE2086B) and sdc2 quad store, which plain C cannot emit. Not matchable in… |
| [`func_L00_002EE078`](levels/func_L00_002EE078.c) | 24 | from src/levels/shared/L00_002E5918.c: 6 retail instructions, C compiles to 5 (ld/sd pairs). Missing: retail's two ldc2 quad loads, a COP2 VU0 macro-mode op (.word 0x4BE2086F) and sdc2 quad store, which plain C cannot emit. Not matchable in… |
| [`func_L00_002EE290`](levels/func_L00_002EE290.c) | 32 | from src/levels/shared/L00_002E5918.c: no C attempt; 8 retail instructions (ldc2, four COP2 macro-mode ops, mtc1, jr), a plain C body compiles to 2 and cannot emit the VU0 ops. Placeholder only. |
| [`func_L00_003B9290`](levels/func_L00_003B9290.c) | 64 | 72 bytes vs retail 64 (18 instructions vs 16): the loop body matches retail instruction for instruction; the difference is the first load. Retail reads the count straight off $gp (lw a0,0xD8C($gp)); cc1 emits lui+lw of the symbol address. S… |
| [`func_L00_003B60E8`](levels/func_L00_003B60E8.c) | 108 | 108 bytes: prologue, field loads and register use match retail; about 7 loop instructions differ (retail: beql on *w with the s+=0x150 in its delay slot, bgezl with the next *w load in its delay slot). Missing: the source shape that makes G… |
| [`func_L00_003B4E88`](levels/func_L00_003B4E88.c) | 112 | 120 bytes vs retail 112 (30 instructions vs 28), and not reachable as written: retail loads two words off $gp in ordinary flow (lw v0,0xA28($gp), lw v1,0xA2C($gp)) and cc1 emits lui+lw of their addresses instead (same boundary as func_L00_0… |
| [`func_L00_003B8158`](levels/func_L00_003B8158.c) | 116 | 88 bytes vs retail 116 (22 instructions vs 29): the control flow and the first call match; missing the else arm, which retail builds as an 8-byte local copied from 0x1AFBC8 with ldl/ldr/sdl/sdr (unaligned doubleword pairs, the struct's type… |
| [`func_L00_003B7908`](levels/func_L00_003B7908.c) | 148 | 156 bytes vs retail 148 (39 instructions vs 37): the loop, the calls and the frame match retail instruction for instruction; the difference is that retail takes the addresses of two $gp-resident globals with one addiu each (addiu s1,$gp,0xB… |
| [`func_L00_0035F9F0`](levels/func_L00_0035F9F0.c) | 24 | near miss: 4 of 6 retail instructions (retail 24 bytes, this 16). Missing the dsll32/dsra32 sign extension of the int parameter before bnel. |
| [`func_L00_0036BE28`](levels/func_L00_0036BE28.c) | 40 | near miss: 9 of 10 instructions (ours 36 bytes, retail 40). Retail loads through $at with hand-placed nops and ends in jr ra + lui ra, 0x7000 (looks like hand asm); plain C gives $v0 and fewer nops. |
| [`func_L00_0036C9AC`](levels/func_L00_0036C9AC.c) | 40 | near miss: 9 of 10 instructions (ours 36 bytes, retail 40). Same wait-loop family as func_L00_0036BE28: retail loads through $at, hand-placed nops, jr ra then lui ra, 0x7000. |
| [`func_L00_0036C9D4`](levels/func_L00_0036C9D4.c) | 40 | near miss: 9 of 10 instructions (ours 36 bytes, retail 40). Same wait-loop family as func_L00_0036BE28: retail loads through $at, hand-placed nops, jr ra then lui ra, 0x7000. |
| [`func_L00_0036D274`](levels/func_L00_0036D274.c) | 40 | near miss: 9 of 10 instructions (ours 36 bytes, retail 40). Same wait-loop family as func_L00_0036BE28: retail loads through $at, hand-placed nops, jr ra then lui ra, 0x7000. |
| [`func_L00_0036D29C`](levels/func_L00_0036D29C.c) | 40 | near miss: 9 of 10 instructions (ours 36 bytes, retail 40). Same wait-loop family as func_L00_0036BE28: retail loads through $at, hand-placed nops, jr ra then lui ra, 0x7000. |
| [`func_L00_003F34D8`](levels/func_L00_003F34D8.c) | 64 | func_L00_003F34D8: 8 instructions differ (register choice: retail keeps the count in $t1 and the quad temp in $v0; ours uses $v1/$a0). Quad copy is lq/sq via TImode. Missing: the allocator order. |
| [`func_L00_003F8DC8`](levels/func_L00_003F8DC8.c) | 64 | func_L00_003F8DC8: 72 bytes against 64 (retail uses $gp for the global at 0x1B0A20 via "sw ...,0x1A30($gp)"; this file builds -G0, so the sda global becomes lui/sw). Needs the file-level -G8 that the file does not have; not matched. |
| [`func_L00_003F0130`](levels/func_L00_003F0130.c) | 104 | func_L00_003F0130: 104 bytes, 27 places; the loop, prologue and call match, but the final read of the base global (0x1B2BB4) is "lw $v0,0x3BC4($gp)" in retail, which -G0 (this file) cannot emit: needs -G8 for L00_003EE100.c. Missing: the fi… |
| [`func_L00_003EEE20`](levels/func_L00_003EEE20.c) | 108 | func_L00_003EEE20: 100 bytes against 108; the whole body matches except that retail has two nops between cvt.s.w and div.s (the int divisor converted to float), which this compiler does not emit for any form tried (variable divisor, (float)… |
| [`func_L00_003F09A0`](levels/func_L00_003F09A0.c) | 124 | func_L00_003F09A0: 124 bytes, 1st differing instruction at +0x2c: retail schedules the counter increment before the $a3 constant and puts the element address in $v1 (ours in $v0); the loop, the call to func_L00_003F0B10 and the gp reload ma… |
| [`func_L00_002B5590`](levels/func_L00_002B5590.c) | 92 | 19 instructions (76 bytes) against retail's 23 (92 bytes): the frame is 0x30 not 0x40 and s1 is never saved; retail keeps &a in s1 across the first call (hoisted before it), GCC rematerializes sp+0 after it. Copy/zero order and the calls ma… |
| [`func_L00_00424A50`](levels/func_L00_00424A50.c) | 52 | 3 instructions differ, all store order (52 bytes): the field stores match retail (float b at +84, sh 1 at +62, float a at +76 of the block at +352) but GCC emits a, b, sh where retail has b, sh, a; missing: the source shape that makes the s… |
| [`func_L00_00403BF8`](levels/func_L00_00403BF8.c) | 52 | 52 bytes: under a -G8 probe only the first two instructions differ (retail does li v1,15 before the lui of the base address into v1; the compiler emits lui v0 first); the loop and store match |
| [`func_L00_00403C30`](levels/func_L00_00403C30.c) | 56 | 56 bytes: under a -G8 probe only the first two instructions differ (retail does daddu a0,zero,zero before the lui of the base into v1; the compiler emits lui v0 first and copies it); the loop matches |
| [`func_L00_004028C0`](levels/func_L00_004028C0.c) | 96 | 96 bytes: under a -G8 probe the compile is 80 bytes; the prefix through the jal and the sb agree, the tail differs (retail stores -1 through a beq and keeps a dead li 10 before a b in the else path); also needs config/cflags.txt -O2 -G8 |
| [`func_L00_00403CB8`](levels/func_L00_00403CB8.c) | 160 | 152 of 160 bytes: the found-slot loop and the call sequence match in shape; retail keeps arg in s1 and the slot value in s0 across the calls, ours assigns the registers the other way round (and the else path ends with the value stored back) |
| [`func_L00_002D40A0`](levels/func_L00_002D40A0.c) | 76 | 72 bytes vs 76 (19 instructions): near miss. Retail keeps the 16-byte lq/sq temp in $v0 and folds the copy destination (0x18B6C0) into $v1, then rebases $v1 by -0x18A0 for the other stores; ours keeps the base in $v0 or $a2, so the temp reg… |
| [`func_L00_002D5E30`](levels/func_L00_002D5E30.c) | 132 | 172 bytes vs 132 (not matched): the loop is misread. Retail stores the running sum to *a0 on each pass, adds *a2 from the flag (M>>5 < sum) and walks halfwords from 0x262262; the early exits and the sum update order are not right here. Star… |
| [`func_L01_003246D0`](levels/func_L01_003246D0.c) | 36 | 4 of 11 instructions differ (36 bytes). Retail passes the 64-bit return of func_3246F8 straight into func_3246A0 (daddu, no conversion). Any C prototype narrower than unsigned long makes GCC sign-extend it (dsll32/dsra32); with unsigned lon… |
| [`func_L01_00304270`](levels/func_L01_00304270.c) | 52 | Does not match: the attempt compiles to 36 bytes, retail is 52 (13 instructions). Retail loads its pointer from $v1, a register that is not an argument, and tests with a2; the self-jump (j func_304270) on a 0x100 test suggests a caller-set … |
| [`func_L00_003E6710`](levels/func_L00_003E6710.c) | 104 | 0 of 27 exact: about 10 of 26 instructions differ. Loop shape and calls match (func_L00_003E6F48, the back-edge reload of D_L00_001B2B50 via sda); we keep the base load before the saves and swap the counter/offset registers (retail: counter… |
| [`func_L00_003E4B80`](levels/func_L00_003E4B80.c) | 108 | 0 of 27 exact: 100 bytes, retail 108. Missing two nops between cvt.s.w and div.s (retail: mtc1, nop, cvt.s.w, nop, nop, div.s); the early return and the join through a0 match. |
| [`func_L00_003E6F48`](levels/func_L00_003E6F48.c) | 124 | 0 of 27 exact: the loop, the call and its arguments match; the base-pointer load is hoisted above the saves (retail: after s1=a0) and the back-edge reload uses v1 where retail uses v0 (lw v0,0x3B64(gp) in the bgez-like delay slot). |
| [`func_L00_003E80A8`](levels/func_L00_003E80A8.c) | 164 | 0 of 27 exact: 144 bytes, retail 164 (5 instructions short: retail keeps base, offset and record pointer live across the calls and saves s0-s4). The calls and the 96.0f scale match; the compiler folds base+off and loses the saved registers. |
| [`func_L00_003E9490`](levels/func_L00_003E9490.c) | 172 | 0 of 27 exact: 164 bytes, retail 172. The 128-bit copies (lq/sq) and the calls match; the copy store lands in the branch delay slot (retail: lq, sq, b, ld in the delay slot), so the two copy paths are 2 instructions off. |
| [func_L01_003115E8](shared/func_L01_003115E8.c) | 88 | Candidate is 84 bytes; table pointer register selection and small global load order differ |
| [`func_L05_0030C930`](shared/func_L05_0030C930.c) | 24 | 0 of 4 places: store order and global-base relocation differ; candidate stores fields at +0x40/+0x44 in reverse order and the declared base symbol did not resolve. |
| [`func_L05_00324DB8`](shared/func_L05_00324DB8.c) | 48 | 0 of 4 places: two C forms emitted 64 and 60 bytes; both missed the call/branch and return shape. |
| [func_L00_002EDCB8](levels/func_L00_002EDCB8.c) | 16 | 12 bytes vs retail 16: candidate is one instruction short; object comparison cannot validate the relocation until the size matches. |
| [func_L00_002EE0C0](levels/func_L00_002EE0C0.c) | 16 | 12 bytes vs retail 16: the stores compile in the wrong order or without the retail nop; no plain C form found that preserves the exact delay-slot layout. |
| [func_L08_003CCD28](levels/func_L08_003CCD28.c) | 48 | 44-byte candidate; if-conversion changes the final branch and register allocation. |
| [func_L08_003D0098](levels/func_L08_003D0098.c) | 48 | 44-byte G8 candidate; branch and pointer calculation differ. |
| [func_L08_003D0508](levels/func_L08_003D0508.c) | 192 | 152-byte candidate; vector copy, frame and register scheduling differ. |
| [func_L08_003D3D80](levels/func_L08_003D3D80.c) | 128 | 120-byte candidate; retail materializes vector pointers before zero-offset quadword copies. Aggregate and memcpy variants also failed. |
| [func_L08_003D8290](levels/func_L08_003D8290.c) | 96 | 96-byte candidate; local quadword copy order and registers differ. |
| [func_L08_003D82F0](levels/func_L08_003D82F0.c) | 84 | 84-byte candidate; vector pointer formation and copy/FP scheduling differ. |

## Level functions from the six agent waves (2026-10-12)

Left by the six waves of workers that landed 521 functions in `src/` (PRs #149, #151,
#153, #155, #156 and #159). Bodies that did not match at every place, kept as starting
points; the note in each file is the worker's own, and this column says how far it got and
what was tried. None of these links, and none is counted. Move a file into `src/` once
`tools/build.py` places it.

| Function | Size | What the worker found |
| --- | ---: | --- |
| [`func_L06_00444068`](levels/func_L06_00444068.c) | 256 | ABANDON: best 268 bytes vs 256; three calls, float clamps to 0x2C0/0x2C8 and constants 0x3F49FDC0/0x3F060A93 recovered, but cc1 keeps float v in $f20 (frame 0x40) where retail uses $f0, and mis-rounds 0.7890167f to 0x3F49FD00. |
| [`func_L00_002B5590`](levels/func_L00_002B5590.c) | 92 | NEAR MISS: 76 bytes/19 instructions vs 92/23; gcc folds &buf[1] into sq $zero,0x10($sp) and does not keep &buf[0] live across the calls (frame 0x30, no s1 save), 4 instructions short; TI128 typedef shapes and store swaps tried. |
| [`func_L12_004163E8`](levels/func_L12_004163E8.c) | 1044 | kept as the worker's whole-file draft; no note was left |
| [`func_L12_0041A5F8`](levels/func_L12_0041A5F8.c) | 160 | Best attempt, not matching: 156 bytes vs 160 in 12_todano; node pointer in $v1 where retail has $a3, and the 128-bit copy folds offsets where retail pre-computes both bases (addiu $v1,$a3,0x210; lq $v0,0($v1)); one instruction short. |
| [`func_L12_0041BDF8`](levels/func_L12_0041BDF8.c) | 104 | Best attempt: 8 bytes short; retail keeps a separate $a1 copy of $a0 made at entry (daddu $a1,$a0,$zero), gcc folds the local onto $a0; twin BE60 also wants daddu $a0,$a1,$zero before func_41BED8; all spellings coalesce. |
| [`func_L00_003C6A88`](levels/func_L00_003C6A88.c) | 268 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003CA688`](levels/func_L00_003CA688.c) | 164 | kept as the worker's whole-file draft; no note was left |
| [`func_L06_003660F0`](levels/func_L06_003660F0.c) | 112 | Draft, best attempt: retail loads the flag as lui $v0,0x1A; lbu $v0,0x7A47($v0) and tests $v0; this source gives lbu $v1,%lo($v0) after the prologue saves; store order 0x20/0x24/0x28 matches; 0x001A7A47 is BOOT data, no sda. |
| [`func_L06_0037E6B0`](levels/func_L06_0037E6B0.c) | 280 | Draft, one instruction short (276 vs 280); retail materialises m+0x10 in $v1 and stores sq $v0,0($v1), this source folds the offset into sq $v0,0x10($s1); TI128 pointer shape tried; next idea a BLKmode struct assignment. |
| [`func_00308600`](game/func_00308600.c) | 360 | kept as the worker's whole-file draft; no note was left |
| [`func_0030E8A8`](game/func_0030E8A8.c) | 1052 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_004096B0`](levels/func_L00_004096B0.c) | 1944 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_002B3D10`](levels/func_L00_002B3D10.c) | 172 | NEAR MISS: 168 bytes/42 instructions vs 172; only the first branch is wrong: retail's bc1fl takes its delay slot from the branch target (lw $v1,0x2294($a0)), ours keeps bc1f with the fall-through lwc1 $f2,0($gp); sda and multiply order tried. |
| [`func_L00_002B5A88`](levels/func_L00_002B5A88.c) | 212 | NEAR MISS: 200 bytes/50 instructions vs 212/53; gcc rotates the loop and computes the D_00189E20 base inline instead of keeping lui $s3,0x19 / addiu $a1,$s3,-0x61E0; the (x + 31) % 32 part matches; no shape yet stops the rotation. |
| [`func_L05_0037CE08`](levels/func_L05_0037CE08.c) | 176 | Best attempt, not a match: 180 bytes vs 176; retail reloads lui+lw of D_L05_001B251C before each of the two loop calls, cc1 hoists the lui into $s3 (plus s4 and a bigger frame); same wall as func_L05_003D02C8. |
| [`func_L05_0037D6E8`](levels/func_L05_0037D6E8.c) | 128 | Best attempt: retail bases the walk on &elem.f14 (lw -0x8($v1) / sw 0($v1)) and keeps %hi of D_L05_002A1800 in $s0; this source materialises the full address, so cc1 emits daddu $s0,$v0 (124/128 with the struct form). |
| [`func_L05_003CFC48`](levels/func_L05_003CFC48.c) | 168 | Structure and size exact (168 bytes); residual is register allocation: retail keeps the D_L05_00221D10 base in $v1 and the value in $v0, this source the reverse, with the +0x10/+0x14 loads in opposite order; D[i]++ changes nothing. |
| [`func_L05_003D02C8`](levels/func_L05_003D02C8.c) | 88 | Near miss: 84 of 88 bytes; retail loads D_L05_001B251C with lui+lw inside the loop, cc1 hoists the lui into $a2, so the loop loses one instruction and the 8-byte alignment nop before the loop label never appears. |
| [`func_L05_00418440`](levels/func_L05_00418440.c) | 156 | Best of 6 compiles: exact 156 bytes, 38 of 39 instructions in retail order; only the daddu $a2,$s0,$zero third-argument copy sits between the halves of the lui/addiu address pair rather than after it; belongs in the -g8 sibling. |
| [`func_L05_0041C118`](levels/func_L05_0041C118.c) | 212 | Best of 2 compiles: 204 vs 212 bytes; the linked-list block matches, but the 16-byte copy at +0x10 folds into lq/sq where retail materialises both addresses, and a $gp load vs swc1 schedule flip remains; needs the -g8 sibling. |
| [`func_L05_0041C7E8`](levels/func_L05_0041C7E8.c) | 116 | Attempts 1-4: 100 vs 116 bytes; retail keeps a signed counter 0,1,2 with slti $v0,$a1,0x3 + bnez, while every source shape becomes a countdown (addiu $a2,$zero,0x2 ... bgez $a2); unsigned int emits sltiu instead. |
| [`func_L05_00421860`](levels/func_L05_00421860.c) | 168 | Best of 8 compiles: exact 168 bytes, all instructions in retail order; only the counter blocks use base=$v0/value=$v1 where retail has base=$v1/value=$v0; shared increments give 164 bytes, a char[] base 172. |
| [`func_L09_00326E90`](levels/func_L09_00326E90.c) | 108 | Kept for the next worker: two instructions over; gcc substitutes the incoming parameter for the loaded control word (daddu $v0,$a0,$0) instead of returning the word already in $v0. |
| [`func_L09_0033E658`](levels/func_L09_0033E658.c) | 56 | Kept for the next worker: layout is right, but gcc puts the -1 constant in the annulled delay slot (beql) where retail keeps a plain beqz + nop, and swaps lui/sll in the first delay slot. |
| [`func_L09_003EA2D0`](levels/func_L09_003EA2D0.c) | 140 | Kept for the next worker: the descriptor now lives in $s3 across the calls; something in the call registers or the epilogue still differs around +0x34. |
| [`func_L09_003F7C50`](levels/func_L09_003F7C50.c) | 140 | Kept for the next worker: one instruction out of place; retail loads the descriptor before the $s0 comparisons, gcc sinks that lw into the bne delay slot. |
| [`func_L09_003F87D8`](levels/func_L09_003F87D8.c) | 124 | Kept for the next worker: retail re-materialises the D_L09_001B285C address inside the loop, gcc hoists the lui out of it; the loop tail also differs. |
| [`func_L07_003F8C38`](levels/func_L07_003F8C38.c) | 152 | Wave 3 w05: size and register allocation identical (s0=a0, s2=0x68, s1=result); only schedule transpositions: reversed addiu $a0,0x5 / daddu $a1,0 for func_002E5CF8(5,0,a0), base in $a1 not $v0, daddu $a0,$s1 after the table load. |
| [`func_L07_003F9028`](levels/func_L07_003F9028.c) | 92 | Wave 3 w05: 92 bytes, 3 of 23 instructions off; the byte at 0x43($a0) is read into $a1 instead of $v1 (retail reuses the flag register, dead after the bnez); u8 c, s32 c and earlier declaration all still give $a1. |
| [`func_L07_003F9088`](levels/func_L07_003F9088.c) | 92 | Wave 3 w05: twin of 003F9028 with constants 3/9 instead of 2/8; same residual, the lbu lands in $a1 instead of $v1. |
| [`func_L07_003FF618`](levels/func_L07_003FF618.c) | 56 | Wave 3 w05: size OK (56 bytes); same transposition as 0040FC50: retail addiu $sp,$sp,-0x10 ; lui $v0,0x1A ; lbu $v0,0x7BD0($v0) versus candidate lui $v1,0x1A first; polarity fixed, != 0 gives ori 2, else the call. |
| [`func_L07_0040FC50`](levels/func_L07_0040FC50.c) | 52 | Wave 3 w05: size OK (52 bytes); only the first two instructions are transposed (retail addiu $v1,$zero,0xF before lui $v0,0x1B); declaration initialiser, comma expression and do/while all still put the lui first. |
| [`func_L11_00434AA8`](levels/func_L11_00434AA8.c) | 80 | kept as the worker's whole-file draft; no note was left |
| [`func_L11_00436850`](levels/func_L11_00436850.c) | 192 | kept as the worker's whole-file draft; no note was left |
| [`func_L06_0041FD60`](levels/func_L06_0041FD60.c) | 44 | ABANDON, best of 3 shapes: same size and sequence; retail addiu $v1,$zero,0xFF ; lbu $v0,0x21($a0) ; beq $v0,$v1 with daddu $a0,$v0 in the delay slot, ours coalesces the load into $a0; unsigned char, int and named-constant forms identical. |
| [`func_L06_0044B250`](levels/func_L06_0044B250.c) | 136 | ABANDON: only three words differ (first at +0x20); retail lw $v0,0x68($s0) ; addiu $v1,$zero,-1 ; lw $a1,0($v0) ; beq $a1,$v1, ours puts -1 in $a0; direct, named and reversed-operand shapes give the same allocation. |
| [`func_L06_0044B2D8`](levels/func_L06_0044B2D8.c) | 116 | ABANDON, 3 words off (see func_L06_0044B250.c); best shape tried was reversed operands -1 == **(int **)(a0 + 0x68). |
| [`func_L06_0044B350`](levels/func_L06_0044B350.c) | 116 | ABANDON, 3 words off (see func_L06_0044B250.c); best shape tried was a named int v assigned from *(int *)*(char **)(a0+0x68). |
| [`func_L06_004504C8`](levels/func_L06_004504C8.c) | 120 | ABANDON, 3 words off (see func_L06_0044B250.c); everything after the -1 test matches, including the branch-likely beql $v0,$zero for the second guard and the shared epilogue; best shape was char *p plus *(int *)p. |
| [`func_L06_00457920`](levels/func_L06_00457920.c) | 168 | ABANDON (172 bytes, LICM wall): retail materialises D_L06_001A8F20 inside the loop and hoists only the function pointer into $s3; gcc hoists the address too (pointer in $s4) and fills the back-edge slot with a hoisted lh $a2,0xAA($s1) bnel. |
| [`func_L06_00457AD0`](levels/func_L06_00457AD0.c) | 300 | ABANDON (304 bytes): every call, constant and address matches, including the three-way switch, the 0xA8 = 0xFF tail and the -0.033333335f/3.0f/6.0f arguments; one extra instruction and a callee-saved swap ($s0/$s1) remain. |
| [`func_L00_00426680`](levels/func_L00_00426680.c) | 100 | Best attempt: %hi at +0x38 differs; the seven stores are exact (source order must be 0,2,1,3,4,5,6 because cc1 rotates them); only the %hi/%lo pair of D_L00_001B1844 remains. |
| [`func_L00_0042A500`](levels/func_L00_0042A500.c) | 32 | Best attempt: only the base register allocation remains; retail lui $v0,%hi / lw $v0,%lo($v0) with one register, cc1 pins lui $v1,%hi / lw $v0,%lo($v1); same gap at -G0 and -G8, char* or int, with or without a temporary. |
| [`func_L00_00429048`](levels/func_L00_00429048.c) | 120 | Best attempt (-g8 sibling): 116 bytes vs 120; the whole structure (bne, bnel, bc1tl, xori/sltiu) is in place; the %hi/%lo pair of D_L00_001B8E10 differs (cc1 uses one register) and one store is missing; gp = 0x1AEFF0. |
| [`func_L00_0042A460`](levels/func_L00_0042A460.c) | 132 | Best attempt: instruction at +0x8 differs; everything is exact except the %hi/%lo pair of D_L00_001B1844, which retail holds in one register; the two stores are already in retail order. |
| [`func_L00_0042BAD0`](levels/func_L00_0042BAD0.c) | 52 | Family of four identical bodies (0042BAD0/_BCF8/_BFF8/_C290): cc1 hoists ld $ra,0x8($sp) before the store and interposes daddu $v0,$s0,$zero between lui and addiu, where retail keeps lui / addiu / daddu / sw / ld $s0 / ld $ra. |
| [`func_L16_004056A8`](levels/func_L16_004056A8.c) | 228 | kept as the worker's whole-file draft; no note was left |
| [`func_L16_004050D8`](levels/func_L16_004050D8.c) | 208 | Creates the handle's 0x274 record on demand, or copies the source windows into it and follows the flag's polarity on the record's byte. |
| [`func_L16_00412BA8`](levels/func_L16_00412BA8.c) | 176 | the worker's note describes the function only: selects one of two measured value tables by index and arms the resident channel. |
| [`func_L16_00412C58`](levels/func_L16_00412C58.c) | 240 | Walks a rotating candidate index over the measured byte table until a free slot is found, arms it, and clears the whole window when none is. |
| [`func_L11_0044C220`](levels/func_L11_0044C220.c) | 68 | Best attempt; instruction at +0x14 differs: ordering of the 8 integer arguments and the 0x68 base (retail $v0, ours $a3); the argument-to-register map is right; retail ABI is n64/n32 (a0-a3 + t0-t3, floats in f12-f19). |
| [`func_L00_003EEE20`](levels/func_L00_003EEE20.c) | 108 | Draft, not a match: 100 bytes vs 108; only the two nops retail carries between cvt.s.w and div.s, and the branch offset that follows, differ; the frame, div $zero,$a0,$s0 guard, mfhi test and delay-slot ld $s0 all match. |
| [`func_L05_00432158`](levels/func_L05_00432158.c) | 68 | Abandoned: 60 bytes vs 68; every version came out strength-reduced (addiu $v1,$zero,0x2 / daddiu $v1,$v1,-0x1 / bgez) and the padding nop landed inside the loop instead of before it; pointer, indexed, int and long counter forms tried. |
| [`func_L05_00441EA0`](levels/func_L05_00441EA0.c) | 140 | Abandoned: 128 bytes vs 140; keeps a parameter copy in $t0 where retail re-reads 0x84($a0) and 0x70($a0), swaps the table/record registers, and rematerialises 0x20($a3) instead of reusing $a1; declaration-order swaps did not help. |
| [`func_L05_00442720`](levels/func_L05_00442720.c) | 136 | Abandoned: 128 bytes vs 136; byte-exact shapes but the twin's destination-register-scratch difference, and retail keeps lui $at ; sw $a1,0x3644($at) followed by jr $ra ; nop where this folds the store into the delay slot as a $gp access. |
| [`func_L05_004427A8`](levels/func_L05_004427A8.c) | 128 | Exact size (128 bytes), 32 instructions in retail order; only the GPRs differ: retail $v0,$v1,$a0 for the byte fields and $a1 for the word, ours $a2,$a3,$v0,$v1; sda yields $at in ordinary flow, and one attribute cannot serve both mirrors. |
| [`func_L05_00442828`](levels/func_L05_00442828.c) | 52 | Belongs in the -g8 sibling: 48 bytes vs 52; the body is verified, but the constant addiu $v0,$zero,1 must come first, the 0x1B3610 address before the $gp stores, the value register is $v0 not $v1, and retail leaves the jr $ra delay slot empty. |
| [`func_L14_00423E50`](levels/func_L14_00423E50.c) | 144 | NEAR MISS: 136 bytes vs 144; the compiler folds idx*4 to 0x3FC in the idx == 0xFF arm and materialises the base without keeping the %hi, where retail recomputes sll and the base at L2; the loop and the 0xF91 test match. |
| [`func_L14_00425C08`](levels/func_L14_00425C08.c) | 164 | NEAR MISS: 168 bytes vs 164; retail materialises the constant 1 once (addiu $t0,$zero,0x1, used for sb $t0,0x31($v1) and as argument 5), this compiler rematerialises it for the field write; literal and shared int one = 1 tried. |
| [`func_L14_00426DE8`](levels/func_L14_00426DE8.c) | 116 | NEAR MISS: 112 bytes vs 116; the missing word is retail's write-back copy (sll $s0,$s0,24 in the jal delay slot then daddu $t0,$s0,$zero), which gcc forwards into the argument register (sll $t0,$s0,24); both shift spellings forward. |
| [`func_L14_00427210`](levels/func_L14_00427210.c) | 60 | NEAR MISS: 68 bytes vs 60; retail keeps the result in $a1 with one shared return block and addiu $a1,$zero,1 in the beql delay slot, but the trailing else-if is if-converted to sltu $v0,$zero,$v0; the if-conversion wall is documented. |
| [`func_L14_004286F8`](levels/func_L14_004286F8.c) | 124 | NEAR MISS: 120 bytes vs 124, belongs in the -g8 sibling (lw $a2,0x35AC($gp)); the missing word is retail's daddu $v1,$v0,$zero before the base clobbers $v0; loop constants land in $a3/$a2 and p/q are swapped; the loop shape matches. |
| [`func_L00_0034C090`](levels/func_L00_0034C090.c) | 132 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003588A8`](levels/func_L00_003588A8.c) | 80 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_00358D50`](levels/func_L00_00358D50.c) | 68 | kept as the worker's whole-file draft; no note was left |
| [`func_L08_003D8A28`](levels/func_L08_003D8A28.c) | 132 | Wave 6 w04: 128 of 132 bytes match, differing at +0x68 in the order of sb $v0,0xBC($s1) against the epilogue ld $s0,0x0($sp); the twin func_L08_00422AD8 fails identically; the if (v0 == -1) v0 = 1; else { call; v0 = 1; } form reproduces retail best. |
| [`func_L08_003D9208`](levels/func_L08_003D9208.c) | 208 | Wave 6 w04: cause is callee-saved allocation (retail $s1 = parameter copy, $s0 = moby pointer, ours reversed), first diff at +0x4; declaration order changes nothing; func_L08_004237F0 also needed the saturation constant 0x7FFD, not 0xFFFD. |
| [`func_L08_004238A0`](levels/func_L08_004238A0.c) | 204 | Wave 6 w04: 202 of 204 bytes match (+0x38); only the temp register differs (lw $v1,0x24($s1) vs retail lw $v0,0x24($s1)); lessons: the <= form gives c.ole.s + bc1f, and 0.8377582f is needed for 0x3F567752. |
| [`func_L00_002A7958`](levels/func_L00_002A7958.c) | 180 | 0 of 27 places match (+0x44 differs); 43 of 45 instructions: retail loads the fade value into $f1 via $at and issues addiu $a0,$a0,0x18E0 before the $gp load, here it is $f0 and the addiu follows the load. |
| [`func_L00_002A7DE8`](levels/func_L00_002A7DE8.c) | 120 | 0 of 27 places match (+0x30 differs); 33 of 36 instructions; three addu put the scaled index first where retail puts the base first (addu $a2,$s0,$a2 vs addu $a2,$a2,$s0); the 128-bit copy with qcopy_nc is exact. |
| [`func_L00_002AE070`](levels/func_L00_002AE070.c) | 88 | 0 of 27 places match (+0x4c differs); 21 of 22 instructions; the tail pointer addu has its operands reversed (retail addu $a2,$a2,$v1, here addu $v1,$v1,$a2); flat, [idx], int* and index-first spellings all give the same order. |
| [`func_L00_003D3DE0`](levels/func_L00_003D3DE0.c) | 456 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003D3FB0`](levels/func_L00_003D3FB0.c) | 292 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003D4958`](levels/func_L00_003D4958.c) | 200 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003D5320`](levels/func_L00_003D5320.c) | 52 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003D5358`](levels/func_L00_003D5358.c) | 336 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003D7250`](levels/func_L00_003D7250.c) | 636 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003DB0A8`](levels/func_L00_003DB0A8.c) | 108 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003DBF50`](levels/func_L00_003DBF50.c) | 624 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003DC1C0`](levels/func_L00_003DC1C0.c) | 268 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003DC2D0`](levels/func_L00_003DC2D0.c) | 128 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003DC350`](levels/func_L00_003DC350.c) | 140 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003DC3E0`](levels/func_L00_003DC3E0.c) | 604 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003DC640`](levels/func_L00_003DC640.c) | 476 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003DC910`](levels/func_L00_003DC910.c) | 436 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003DD2B8`](levels/func_L00_003DD2B8.c) | 964 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003DDEC8`](levels/func_L00_003DDEC8.c) | 444 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003DE088`](levels/func_L00_003DE088.c) | 252 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003DE490`](levels/func_L00_003DE490.c) | 376 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003DF030`](levels/func_L00_003DF030.c) | 212 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003DF338`](levels/func_L00_003DF338.c) | 884 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003DF6B0`](levels/func_L00_003DF6B0.c) | 96 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003DFDC0`](levels/func_L00_003DFDC0.c) | 468 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003E0088`](levels/func_L00_003E0088.c) | 436 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003E0240`](levels/func_L00_003E0240.c) | 960 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003E0F58`](levels/func_L00_003E0F58.c) | 436 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003E2180`](levels/func_L00_003E2180.c) | 912 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003E2BD0`](levels/func_L00_003E2BD0.c) | 684 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003E2E80`](levels/func_L00_003E2E80.c) | 984 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_003B4E88`](levels/func_L00_003B4E88.c) | 112 | Near miss: exact 112 bytes, only allocation differs (+0x20); retail counter $v0 (=3, bgez) with pointers $v1/$a0/$a1 and constants $a2=2, $a3=1, this source rotates temps as $a0/$v1/$v0; four statement orders tried; needs -g8 plus nosda on 0x28D0/0x28D4. |
| [`func_L00_003B60E8`](levels/func_L00_003B60E8.c) | 108 | Exact 108 bytes but wrong loop codegen (+0x2c): retail uses beql $v0,$zero with the duplicated addiu $s0,$s0,0x150 in its delay slot, this source emits a plain beqz with both increments hoisted above the branch; four loop shapes tried. |
| [`func_L00_003B69D8`](levels/func_L00_003B69D8.c) | 180 | Near miss: 172 bytes vs 180; structure right but p->$s1, q->$s2 where retail has p->$s0, q->$s1 (both declaration orders tried), one instruction short; the single-base D_L00_001B8FF0 form is required, two D_ symbols give 184 bytes. |
| [`func_L05_004157B0`](levels/func_L05_004157B0.c) | 256 | Draft, twin of func_L05_0041CA70: exact size with matching frame and registers; retail stores swc1 $f1,0x180($s1) before the 0x270/0x26C/0x268 stores and has 20.0f in $f1, 0.02f in $f0 (ours reversed); the 0.02f store must be written first. |
| [`func_L05_0041CA70`](levels/func_L05_0041CA70.c) | 244 | Draft: 244 bytes, 3 words differ; frame 0x40 and callee-saved allocation now match, but the func_37AEC0(obj,1) argument setups are reversed (retail addiu $a1,$zero,1 before daddu $a0,$s0,$zero) and addu $s1,$s1,$v0 is one slot late; scheduling-only. |
| [`func_L05_0041FC60`](levels/func_L05_0041FC60.c) | 228 | kept as the worker's whole-file draft; no note was left |
| [`func_L01_00397950`](levels/func_L01_00397950.c) | 212 | Worker w13: body identical instruction by instruction except the callee-saved pair (retail $s1 = $68 pointer, $s2 = loop counter, ours reversed); no source lever found; the source store order 50,54,5C,58 emits retail 5C,58,50,54. |
| [`func_L01_003990E8`](levels/func_L01_003990E8.c) | 132 | Worker w13: only one instruction differs, the preheader constants order (retail addiu $t1,$0,1 ; lhu $v0,0x34 ; addiu $t0,$0,2, ours reverses the two addiu); stores 0x98/0x34/0x31 and the loop match; store orders and an explicit i = 2 tried. |
| [`func_L01_003EE8E0`](levels/func_L01_003EE8E0.c) | 112 | Worker w13: 116 bytes vs 112; only one instruction too many, gcc rematerialises 0xFF for the return (addiu $v0,$0,0xFF again after sb $v0,0xA8($s0)) instead of reusing $v0; return 0xFF and r = 0xFF; return r tried. |
| [`func_L11_00456378`](levels/func_L11_00456378.c) | 192 | Draft w17: 184 bytes vs 192; allocation conforms (p->$s1, q->$s0) and the body matches except the branch (ours beq $v0,$s2 + nop, retail bnel $v0,$s2) and the then-block side; one-call and two-call variants give 160 or 200 bytes. |
| [`func_L11_00456438`](levels/func_L11_00456438.c) | 264 | Draft w17: 268 bytes vs 264, one instruction too many; the constant 1 is materialised twice ($t0 for the sb, $a3 for the argument) where retail reuses $a3. |
| [`func_L11_00456540`](levels/func_L11_00456540.c) | 148 | Draft w17: same size (148 bytes) but instruction at +0x8 differs; only the two callee-saved registers are swapped (retail p->$s0, q->$s1, ours p->$s1, q->$s0). |
| [`func_L11_00456938`](levels/func_L11_00456938.c) | 268 | Draft w17: reconnaissance stub only; uses $gp (0x2DB0/0x2DB4/0x2DB8) and 128-bit copies, so a -g8 sibling file is required. |
| [`func_L11_00456EA0`](levels/func_L11_00456EA0.c) | 220 | Draft w17: reconnaissance stub only; would use $gp (0x2DC4/0x2DC8/0x2DCC), so a -g8 sibling file is required. |
| [`func_L11_00458A98`](levels/func_L11_00458A98.c) | 232 | Draft w17: reconnaissance stub only; would use $gp (lhu 0x2F94($gp)) and lq/sq. |
| [`func_L11_00458C38`](levels/func_L11_00458C38.c) | 200 | Draft w17: reconnaissance stub only; would use $gp (0x2FCC/0x2FD0/0x2FD8/0x2FDC), so a -g8 sibling file is required. |
| [`func_L11_00459D48`](levels/func_L11_00459D48.c) | 252 | Draft w17: reconnaissance stub only; would use $gp (lwc1 0x300C($gp)). |
| [`func_L11_0045B5C8`](levels/func_L11_0045B5C8.c) | 256 | Draft w17: 252 bytes vs 256, one instruction missing; retail keeps $a0/$a1 for the two globals and puts the q->f0 store in the beq delay slot. |
| [`func_L07_003E5AC8`](levels/func_L07_003E5AC8.c) | 168 | Chain-walk family: shape A (goto rotation plus sda) reaches 168 bytes with body and test block byte-identical, but the +0x10..+0x20 preheader swaps $v0/$v1 and the dead nop at +0x2C is missing; shape B (local id) is 164 bytes. |
| [`func_L07_003E6F40`](levels/func_L07_003E6F40.c) | 168 | Chain-walk family: shape A (goto rotation plus sda) reaches 168 bytes with body and test block byte-identical, but the +0x10..+0x20 preheader swaps $v0/$v1 and the dead nop at +0x2C is missing; shape B (local id) is 164 bytes. |
| [`func_L07_003EB790`](levels/func_L07_003EB790.c) | 168 | Chain-walk family: shape A (goto rotation plus sda) reaches 168 bytes with body and test block byte-identical, but the +0x10..+0x20 preheader swaps $v0/$v1 and the dead nop at +0x2C is missing; shape B (local id) is 164 bytes. |
| [`func_L07_003EE640`](levels/func_L07_003EE640.c) | 200 | Exact 200 bytes; only the five-instruction preheader at +0x54..+0x64 differs (ours orders p = o + 8 and i = 7 before -1 and D_00189EA0); a countdown o[15 - i] form and hoisted locals both drop to 184 bytes. |
| [`func_L06_00458E10`](levels/func_L06_00458E10.c) | 288 | kept as the worker's whole-file draft; no note was left |
| [`func_L06_00458F30`](levels/func_L06_00458F30.c) | 236 | kept as the worker's whole-file draft; no note was left |
| [`func_L06_00459B18`](levels/func_L06_00459B18.c) | 144 | kept as the worker's whole-file draft; no note was left |
| [`func_L06_0045A490`](levels/func_L06_0045A490.c) | 112 | kept as the worker's whole-file draft; no note was left |
| [`func_L06_00464900`](levels/func_L06_00464900.c) | 236 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_0039F300`](levels/func_L00_0039F300.c) | 116 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_00390E10`](levels/func_L00_00390E10.c) | 276 | Restores the third-level object's transform and flags from the level's own globals, marks the level as initialised, publishes the default geometry constants and calls the boot helper with the three default words. |
| [`func_L00_00390F28`](levels/func_L00_00390F28.c) | 208 | Publishes the third-level object's transform and flags into the level's own globals, then, in the second mode, publishes five more constants and calls the boot helper with three words of the 0x1BD2C0 array. |
| [`func_L00_003FC108`](levels/func_L00_003FC108.c) | 284 | Wave 6 w28 best attempt: 272 bytes vs 284; the callee-saved mapping swaps $s2/$s3 (o vs a1) and the entry block order of the five stores differs; file src/levels/shared/L00_003F8E08.c. |
| [`func_L00_003ACC88`](levels/func_L00_003ACC88.c) | 112 | ABANDON, compiler-profile wall: retail materialises the 128-bit zero with pcpyld $v0,$zero,$zero and stores sq $v0,0x10($sp), but rac2_fold_zero_ti_store folds it to sq $zero whenever the register is dead afterwards, which it is; +0x1c differs. |
| [`func_L00_003ACCF8`](levels/func_L00_003ACCF8.c) | 104 | Nearest: exact 104 bytes, 25 of 26 instructions identical; only the position of mtc1 $zero,$f12 against lw $a2,0x7B8($gp) differs (retail emits the float constant first); a local float does not move it; needs the -g8 sibling. |
| [`func_L00_003B0A28`](levels/func_L00_003B0A28.c) | 136 | ABANDON: 116 bytes vs 136; retail walks the table with the cursor in $a1 and invariant copies in $a3/$t1, storing the cursor twice (bnel delay slot and after the wrap-around exit); the duplicate store is reproduced, the register copies are not. |
| [`func_L00_003B0AB0`](levels/func_L00_003B0AB0.c) | 108 | Nearest: exact 108 bytes, 26 of 27 instructions identical; index in $a3 and tag in $a2 (retail index $a2, tag $v0) flips bnel $v0,$a2 to bnel $a2,$a3; writing c == s->f17 fixes the registers but flips the branch back. |
| [`func_L00_003B0CE8`](levels/func_L00_003B0CE8.c) | 184 | ABANDON: 168 bytes vs 184; the shape is reproduced but not the two distinct 0x1C materialisations ($t1/$t2) nor the $t4 = base+0x14 register; retail clears slot[idx], rewrites bytes equal to idx and calls func_L00_003B0A28 on wrap-around. |
| [`func_L01_003D9830`](levels/func_L01_003D9830.c) | 172 | Best attempt: 176 bytes vs 172 in 01_oozla (+0x4 differs); retail keeps the func_312B68 result in $v0 with no daddu $v1,$v0,$zero, and its second test is beql with the 0xFF materialisation in the delay slot. |
| [`func_L01_003DA300`](levels/func_L01_003DA300.c) | 236 | Best attempt: exact 236 bytes (+0x4 differs); only the $s3/$s4 assignment of the $a0/$a1 copies is mirrored, pure register allocation (retail a0->$s3, a1->$s4). |
| [`func_L06_003E3ED0`](levels/func_L06_003E3ED0.c) | 88 | 8 bytes short (80 vs 88): the return 0/1 pair folds to lw $v0,0x588 ; xor $v0,$v0,$a0 ; jr $ra ; sltiu $v0,$v0,1 where retail keeps an explicit bne to the shared epilogue; both polarities and cross-jump blocks tried; if-conversion wall. |
| [`func_L06_003E4F08`](levels/func_L06_003E4F08.c) | 152 | 4 bytes short (148 vs 152): CSE keeps the 0.000290888245f constant in $f20 across call 1 (swc1/lwc1 $f20, frame -0x30, 37 words) where retail re-materialises lui/ori/mtc1 per call (frame -0x20, 38 words); sda and argument order tried. |
| [`func_L00_00401D50`](levels/func_L00_00401D50.c) | 212 | 204 vs 212 bytes; the missing pair is retail's redundant beql $s0,$zero to the loop test with lw $s0,0x28($s0) in the delay slot, before node->obj; while, for forms, && chains and continues tried; belongs in the -g8 sibling. |
| [`func_L00_00403BF8`](levels/func_L00_00403BF8.c) | 52 | Exact size and body; only the preamble order differs (retail addiu $v1,$zero,0xF before lui $v0,0x1B); int i = 15 first, while/do-while, comma init and array form tried; the same symptom appears on 00403C30 and 00403CB8. |
| [`func_L00_00403C30`](levels/func_L00_00403C30.c) | 56 | Same order wall as 00403BF8: the counter addiu $a0,$zero,$zero must precede the address, and cc1 builds it in $v0 then copies to $v1 where retail writes lui $v1,0x1B ; addiu $v1,$v1,0x2C30; array and scalar forms identical. |
| [`func_L00_00403CB8`](levels/func_L00_00403CB8.c) | 160 | Exact 160 bytes with the right branch polarity, but the scanning loop (else branch) is unrolled once by cc1 where retail keeps a simple two-pointer loop ($a0 loads, $a2 stores) plus counter $a1; the $a0/$a1 roles are reversed. |
| [`func_L00_00409480`](levels/func_L00_00409480.c) | 212 | 216 vs 212 bytes; only c is born in $v1 then copied to $a2 for the call (one daddu $a2,$v1,$zero too many) where retail makes it born in $a2 (addiu $a2,$zero,0x80 ; subu $a2,$a2,$v0); moving the call earlier gives 224 bytes. |
| [`func_L04_00441100`](levels/func_L04_00441100.c) | 116 | the worker's note describes the function only: wave 6 worker w38, file src/levels/04_barlow/L04_00422A00.c: best attempts for the four functions left as INCLUDE_ASM; each entry names the last check.py line observed. |
| [`func_L01_003BDB28`](levels/func_L01_003BDB28.c) | 132 | The note holds only the function name; no difference or attempt is recorded. |
| [`func_L01_003BE4D0`](levels/func_L01_003BE4D0.c) | 340 | The note holds only the function name; no difference or attempt is recorded. |
| [`func_L01_003C08A0`](levels/func_L01_003C08A0.c) | 288 | The note holds only the function name; no difference or attempt is recorded. |
| [`func_L19_0041C328`](levels/func_L19_0041C328.c) | 176 | Draft, one attempt: 160 bytes vs 176 in 19_grelbin; code and flow reconstructed, but retail places the body at 0x41C358 before the test block at 0x41C380 while ours puts the tests first; a loop form for that layout remains. |
| [`func_L19_00423CC0`](levels/func_L19_00423CC0.c) | 508 | Draft, two attempts: 460 bytes vs 508 in 19_grelbin; the body is rebuilt but the switch dispatch tree (order and form of the 4 head branches) does not reproduce with a 2-case switch or permuted cases. |
| [`func_L00_0041AD40`](levels/func_L00_0041AD40.c) | 228 | kept as the worker's whole-file draft; no note was left |
| [`func_L00_00424978`](levels/func_L00_00424978.c) | 144 | NEAR MISS, 3 instructions: everything from 0x4249C0 matches; only the 0x4249B0 pair differs (ours bne $v0,$a2, retail bnel $v0,$a2 with lh $v0,0x84($v0) in the slot); pointer, array and key-local variants give the same pair. |
| [`func_L00_00426248`](levels/func_L00_00426248.c) | 248 | NEAR MISS, 8 bytes over (256 vs 248, LICM wall): both loop bodies match, but the 0x1B1880 address is hoisted into $s7 (one extra sd/ld pair) where retail re-materialises lui $v1,0x1B ; lw $v1,0x1880($v1) in the inner loop; a local base costs 268. |
| [`func_L13_0040A2C8`](levels/func_L13_0040A2C8.c) | 232 | Best attempt, not marked: 228 vs 232 bytes, and a (t, v, q) local variant drops to 220; the 128-bit zero store folds to sq $zero,0x20($sp) where retail keeps por $v0,$zero,$zero ; sq $v0,0x20($sp); declaration order (t, v, q) still to try. |
| [`func_L08_003D4600`](levels/func_L08_003D4600.c) | 144 | Draft, not matching (36 words, +0x2c): the beqz $s0 delay slot holds the constant 1 where retail fills it with addiu $a2,$s0,0x10, and the stores come out 0x31/0x30/0x20/0x32 against retail sb 0x30 / sb 0x31 / sh 0x32 / sb 0x20. |
| [`func_L08_003F6568`](levels/func_L08_003F6568.c) | 168 | Draft, not matching: 152 vs 168 bytes; the 4-instruction gap is the accumulator sel, kept as -1 in $s2 across both func_002A8600 calls in retail (sd $s2,0x20($sp), sra $s2,$v0,8) but rematerialised by GCC in the bnez slot. |
| [`func_L08_00435600`](levels/func_L08_00435600.c) | 160 | Draft, not matching, belongs in the -g8 sibling (lwc1 $f1,0x2FA4($gp)); at +0x3c one store pair is swapped (retail sb $v1,0x30($s0) then sh $a0,0x32($s0)); three source orders tried, one folds the 0xFF and gives 156. |
| [`func_L00_003EA7D0`](levels/func_L00_003EA7D0.c) | 140 | Draft w49: exact 140 bytes, one instruction out of place (+0x34); 0x1B2BA0 must be a symbol (a literal emits ori), but symbolic forms grow the frame to 144/148/152 and the literal schedules lui $a1,0x1B one step early in the first call. |
| [`func_L00_003ECB00`](levels/func_L00_003ECB00.c) | 52 | Draft w49: exact 52 bytes, only the first two instructions permuted (retail addiu $v1,$zero,0x3 before lui $v0,0x1B, ours emits ori $v0,$v0,0x7A4); the matching twin func_L00_003FF7B8 starts from a register, so cc1 always hoists this pair. |
| [`func_L00_003ECC90`](levels/func_L00_003ECC90.c) | 236 | Draft w49: exact 236 bytes; everything matches except a $s1/$s2 permutation (retail $s1 = i, $s2 = pa, ours reversed); declaration order, parameter copies and do/while do not change it; the addresses must stay symbols (addiu, not ori). |

## Linked at some of their places only (2026-10-12)

C that was in `src/` and links in some programs but not in all: a level address written into
it, a jump table it needs, or two different functions the catalogue holds as one. A function
counts when it links at every place it has.

| Function | Places it links at | What the build says elsewhere |
| --- | --- | --- |
| [`func_002DCAB8`](game/func_002DCAB8.c) | 26 of 28 | instruction at +0x38 differs |
| [`func_L00_002B1308`](levels/func_L00_002B1308.c) | 0 of 27 | needs .rodata at its place, which is not C |
| [`func_L00_002D4A38`](levels/func_L00_002D4A38.c) | 26 of 27 | instruction at +0x2c differs |
| [`func_L00_003DC350`](levels/func_L00_003DC350.c) | 2 of 27 | instruction at +0x4 differs |
| [`func_L00_00424BB8`](levels/func_L00_00424BB8.c) | 23 of 27 | instruction at +0x28 differs: `16_snivelak`, `22_dobbo_orbit`, `23_damosel_orbit` and `26_jamming_array` hold other constants |
| [`func_L02_003F9AF0`](levels/func_L02_003F9AF0.c) | 0 of 1 | needs .rodata at its place, which is not C |
| [`func_L02_003F9B70`](levels/func_L02_003F9B70.c) | 0 of 1 | needs .rodata at its place, which is not C |
| [`func_L02_003FE1B0`](levels/func_L02_003FE1B0.c) | 0 of 1 | needs .rodata at its place, which is not C |
| [`func_L11_0043E090`](levels/func_L11_0043E090.c) | 0 of 1 | needs .rodata at its place, which is not C |
| [`func_L11_0043E108`](levels/func_L11_0043E108.c) | 0 of 1 | needs .rodata at its place, which is not C |
| [`func_L11_004433D8`](levels/func_L11_004433D8.c) | 0 of 1 | needs .rodata at its place, which is not C |
