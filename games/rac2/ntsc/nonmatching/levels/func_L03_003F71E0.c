/* Near miss from src/levels/03_endako/L03_003F09C0.c — register allocation only.
 *
 * Kept as a starting point; not linked, not counted.
 *
 * 03_endako, 108 bytes. Every control-flow decision, every offset, every store and
 * the store order all match retail. Only the choice of registers differs:
 *
 *   retail  $t0 = index, $t1 = D_001A63A8, $a3 = object, $a2 = object + index
 *   here    $a2 = index, $a3 = D_001A63A8, $a0 = object, $v0 = object + index
 *
 * So this one is *semantically* solved; what is left is GCC picking the argument
 * registers instead of the temporaries the retail code uses.
 *
 * What the retail bytes settle, all of it worth keeping:
 *   - the three guards short-circuit to the function's single exit, and the body
 *     runs only when all three hold:
 *         D_001A63A8[0x68] == 0 && D_001A63A8[0x24] == -1 && object[0x40C] != index
 *     The `sll $v1,$t0,2` at 0x3F7210 sits in the `beq` delay slot and therefore
 *     runs on the skip path too; the compiler produces it for free from the body.
 *   - the last three stores go through `object + index`, not `object`. The read at
 *     0x3F7208 is `lw $v0,0x40C($a3)` against the object, but the writes at
 *     0x3F7238-40 are against `$a2`, which is `object + index`. Reading that as
 *     three writes to the object is wrong and was the first thing that had to be
 *     put right.
 *   - GCC emits the three final stores in the *reverse* of C source order. Writing
 *     them 0x408, 0x40C, 0x410 in that order is what produces retail's
 *     sb 0x410 / sw 0x40C / sw 0x408. The addresses are distinct, so the C is
 *     equivalent either way; only the emitted order changes.
 *
 * Six wordings were tried: the index folded into each store address, a separate
 * `e = object + index` pointer, that pointer hoisted above the guard, and the
 * store order reversed. The last one fixed the order and left the allocation.
 *
 * What is left is the register allocator's preference. Not yet tried: giving the
 * globals their own named locals and using a fourth parameter, so that GCC has
 * fewer candidates to reuse $a0-$a3 for; or spelling the guards so the object
 * pointer stays live across the guard (it currently dies at the first guard).
 */
extern char D_001A63A8[];

void func_L03_003F71E0(char *a0, s32 a1) {
    char *g = D_001A63A8;
    char *o = *(char **)(a0 + 0x68);
    char *e;
    s32 t = a1;
    s32 v;
    if (*(s32 *)(g + 0x68) == 0 && *(s32 *)(g + 0x24) == -1 && *(s32 *)(o + 0x40C) != t) {
        v = *(s32 *)(o + t * 4 + 0x3D0);
        *(s32 *)(g + 0x28) = 0;
        v = (v & 0x3FFF) + 0x7D0;
        *(s32 *)(g + 0x24) = v;
        e = o + t;
        *(s32 *)(e + 0x408) = 0x258;
        *(s32 *)(e + 0x40C) = t;
        *(u8 *)(e + 0x410) = 1;
    }
}