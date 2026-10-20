/* func_L00_003F3518 (644 bytes, 12 places) - best attempt, NOT MATCHED: 632 bytes against 644
 * (tools/check.py: "632 bytes, expected 644" in every place). The 12 bytes are three compiler
 * walls, everything else is retail's sequence word for word: the 0x640 frame with every local at
 * its retail offset, the prologue (idx first, the count store between the two loads, because
 * count's address is taken and the store cannot move above the first read of *st), the id-list
 * walk (bare table read, `beql`/`bgezl`/`bnel` chain, the $gp reload of the second base in the
 * `bnez` delay slot), every call setup, the dot-product half, the colour half and the epilogue.
 *
 *  1. Source pointer of a quadword copy. Retail: `addiu $v1,$s4,0x30 ; lq $v0,0x0($v1)`.
 *     This cc1 folds the offset: `lq $v0,0x30($s4)` (one instruction short).
 *  2. The two clears of a four-float local. Retail: `por $v0,$zero,$zero ; sq $v0,0x5B0($sp)`
 *     (and the same at 0x5D0). This cc1 prints `sq $zero,0x5B0($sp)`: the rac2_fold_zero_ti_store
 *     pass folds the `por` because $v0 is dead after the store (two instructions short). Writing
 *     the clear as a plain TI128 store is the only spelling that reaches `sq` at all (a
 *     float[4] initialiser calls memset).
 *  3. Destination pointer of the copy into the two vectors whose address is also an argument.
 *     Retail: `sq $v0,0x0($a0)` (a0 = $sp+0x580) and `sq $v0,0x0($s3)` ($s3 = $sp+0x590, set by
 *     `addiu $s3,$sp,0x590` in the `bltz` delay slot). This cc1 stores at `0x580($sp)` and
 *     `0x590($sp)` and builds $s3 later, which also moves the scheduling around it; and the first
 *     float add (`pos[3] += st->f4C`) then gets its operands in $f0/$f1 where retail has
 *     $f1/$f0 (ten spellings measured, none moves it).
 *
 * This is the known cse tie that folds a register-plus-constant into a quadword operand
 * (docs/COMPILER-NOTES.md, "The constant-offset wall"); the fold pass for zero stores is
 * tools/compiler/transformers/fold_zero_ti_store.py. Pointer variables for the vectors make it
 * worse (the addresses become loop invariants in $s5/$fp, 660 bytes). The old "MMI" note was
 * wrong: the 0x7800/0x7C00 words are plain lq/sq and 0x700014A9 is `por`.
 *
 * What the function does: it is the handler func_L00_003F3160 binds to its object. For every id of
 * the record's group it copies the 16-byte point of the entity (table at D_L00_001B2514, 32-byte
 * stride), lifts its fourth float by the record's +0x4C, drops it when func_00283DB8 says it is
 * outside the view, builds a world-space point from the entity record (D_L00_001B251C, 64-byte
 * stride, +0x30), and either queues it with the record's packed colour (+0x30) when +0x6C is set,
 * or fades the colour by how well the direction to the aim vector lines up with the record's
 * +0x40..+0x48 direction (squared when +0x38 is set) before queueing it; the batch of at most 70
 * points goes to func_L00_00325CC0 at the end.
 */

typedef int TI128 __attribute__((mode(TI)));

typedef struct {
    char pad_000[0x140];
    float x_140;
    float y_144;
    float z_148;
} Aimvec_F0b10;

extern Aimvec_F0b10 D_L00_001B8C80;

extern int func_00283DB8(char *local, float value);
extern void func_00283098(char *, char *, char *);
extern void func_00282CC0(char *, char *, char *);
extern void func_00282CF0(char *off, char *v, char *pos);
extern void func_00282F20(char *dst, char *src, float angle);
extern float func_00282DB0(char *, char *);
extern int func_L00_0031C998(int, int, float);
extern void func_L00_003F34D8(void *, void *, void *, void *, void *);
extern void func_L00_00325CC0(int, void *, void *, float, float);

extern unsigned short **D_L00_001A9014 __attribute__((sda));
extern unsigned char *D_L00_001B2514 __attribute__((sda));
extern unsigned char *D_L00_001B251C __attribute__((sda));

void func_L00_003F3518(char *obj)
{
    unsigned char *st = *(unsigned char **)(obj + 0x68);
    char buf0[0x460];
    char buf1[0x120];
    char pos[16];
    char b[16];
    char d[16];
    char c[16];
    char f[16];
    char e4[16];
    int idx;
    int count;
    int color;
    unsigned short *q;
    unsigned char *e;
    float dot;

    idx = *(int *)st;
    count = 0;
    q = D_L00_001A9014[idx];
    for (; q != 0 && ((short)q[-1] >= 0 || q == D_L00_001A9014[*(int *)st]) &&
           (e = D_L00_001B251C + ((*q & 0x7FFF) << 6)) != 0;
         q++) {
        *(TI128 *)pos = *(TI128 *)(D_L00_001B2514 + ((*q & 0x7FFF) << 5));
        *(float *)(pos + 12) += *(float *)(st + 0x4C);
        if (func_00283DB8(pos, *(float *)(st + 0x60)) < 0)
            continue;
        *(TI128 *)b = *(TI128 *)(e + 0x30);
        *(TI128 *)c = 0;
        *(float *)(c + 0) = *(float *)(st + 0x50);
        *(float *)(c + 4) = *(float *)(st + 0x54);
        *(float *)(c + 8) = *(float *)(st + 0x58);
        *(TI128 *)d = *(TI128 *)c;
        func_00283098(d, d, (char *)e);
        func_00282CC0(b, b, d);
        if (*(int *)(st + 0x6C) != 0) {
            func_L00_003F34D8(b, st + 0x30, buf0, buf1, &count);
            continue;
        }
        func_00282CF0(c, (char *)&D_L00_001B8C80.x_140, b);
        func_00282F20(c, c, 1.0f);
        *(TI128 *)e4 = 0;
        *(float *)(e4 + 0) = *(float *)(st + 0x40);
        *(float *)(e4 + 4) = *(float *)(st + 0x44);
        *(float *)(e4 + 8) = *(float *)(st + 0x48);
        *(TI128 *)f = *(TI128 *)e4;
        func_00283098(f, f, (char *)e);
        func_00282F20(f, f, 1.0f);
        dot = func_00282DB0(c, f);
        if (*(float *)(st + 0x3C) < dot) {
            dot = dot * 0.5f + 0.5f;
            if (*(int *)(st + 0x38) != 0)
                dot = dot * dot;
            color = func_L00_0031C998(*(int *)(st + 0x34), *(int *)(st + 0x30), dot);
            func_L00_003F34D8(b, &color, buf0, buf1, &count);
        }
    }
    func_L00_00325CC0(count, buf0, buf1, *(float *)(st + 0x4C), *(float *)(st + 0x5C));
}
