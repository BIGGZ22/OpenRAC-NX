/* func_L23_00429DC8 (560 bytes, 1 place: 23:00429DC8) -- NEAR MISS, not integrated.
   This C compiles to 552 bytes against retail's 560 (-g8 unit, -O2 -G8): two words short, and
   both missing words are the two walls below; the rest of the body is the same code.
   Right: the 0x30 frame with s0-s2, the five `$gp`/absolute stores, the three-vector float
   build, the three nested loops with retail's exact shape (outer counter NOT strength-reduced
   and the `i+1` / `j+1` copied early into $t4/$t1, the peeled first element of the found-search,
   `$a3 = n` copied in the delay slot of `blez`, the `sym` / `sym+2` base pair, the table
   address pairs), and the closing count-down loop `out[m] = (m * n) / 16`.
   What differs:
   1. `por $v0,$zero,$zero` (retail +0x44) + `sq $v0,0($sp)` (+0x7C): this cc1 emits only
      `sq $zero,0($sp)`. rac2_fold_zero_ti_store (tools/compiler/transformers) folds a zero
      materialisation into its store whenever the register is dead after the store, and runs
      before sched2. Retail keeps the `por` (sched2 hoists it away from its store) for this STACK
      slot; every `sq $zero` witness the fold pass was built for stores through a pointer
      register (func_00282C88, func_002E5FE0). func_L23_0042ADB0 has both in one body:
      `por ; sq $v0,0xE0($sp)` and `addiu $v0,$sp,0xE0 ; sq $zero,0($v0)` (the second may be an
      inline `vec4_zero`). Untested hypothesis: a fold pass that skips sp-based destinations
      would give retail's form here.
   2. the copy into obj+0x10: retail `addiu $a1,$a2,0x10 ; ... ; lq $v0,0($sp) ; sq $v0,0($a1)`,
      this cc1 `sq $v0,0x10($obj)` (cse folds the constant into the quad store). Every `lq`/`sq`
      pair in the four L23 bodies of this batch has that shape: both addresses in registers
      computed by their own `addiu` and the pairs kept strictly in source order, whereas cc1's own
      TImode copy folds the offsets (func_L00_00395240: `lq $v1,0xC0($s1) ; sq $v1,0xC0($s0)`).
      About 570 INCLUDE_ASM functions in this level's program (boot and shared included) show
      the same `addiu`-materialised zero-offset pair, so one compiler lever would pay back widely;
      an inline-asm 16-byte copy macro (volatile, register operands) would explain the shape,
      including the scheduling barrier it puts between the neighbouring stores.
   3. knock-on: obj stays in $t1 (retail $a2), the early stores are scheduled differently, and
      the last loop has n in $a1 / counter in $a2 (retail $a2 / $a1) and the out-array address as
      `lui $v0` ... `addiu $t0,$v0,..` (retail `lui $t0 ; addiu $t0,$t0,..`).
   Levers measured on the way (each changes the whole loop nest):
   - the last loop must NOT reuse `i`: with `for (i = 0; i < 16; i++)` gcse stops hoisting
     `i+1` and the outer loop is strength-reduced (rows kept in pointer registers).
   - `float *p = D_L23_002A7170[i]; p[2]` keeps retail's `lwc1 $f0,0x8($v0)` on the row base;
     `D_L23_002A7170[i][2]` folds the +8 into a hoisted `sym+8`.
   - `((s16 *)D_L23_002A7A74)[j * 4 + 1]` (the extern typed as an 8-byte struct array, walked
     through the cast) keeps retail's separate `sym+2` base register; the struct field `.b`, or
     the extern typed `s16 []` (540 bytes), folds it differently.
   - the append re-reads the count (`D[count] = j; count = count + 1`) so gcse copies the first
     load into `$a3`; but then the later words of the same count must be a second NAME for the
     same address (`D_L23_001B3934_2`, a different `sda` array symbol, so gcse neither hoists the
     closing load above the loops nor merges it with the in-loop loads). With one name the
     closing load is PRE'd and the in-loop loads move (the size and the loop shape both drift).
   Source of the data (level 23): D_L23_002A7170 = 0x48 records of 16 bytes (float at +8),
   D_L23_0025E8B0 = 0x48 flag words, D_L23_002A7A74 = 0x9C entries of 8 bytes ({s16 a, s16 b, ..}),
   D_L23_0025E9D0 = list of indices, D_L23_001B3934 = its count, D_L23_001B3980 = 16 results. */

typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef int s32;
typedef int TI128 __attribute__((mode(TI)));

typedef union { TI128 q; float f[4]; } V4;

/* the -g8 sibling's externs, plus the three new ones */
extern s16 D_L23_001B1B94 __attribute__((sda));
extern s16 D_L23_001B1B96 __attribute__((sda));
extern s32 D_L23_001B1B98 __attribute__((sda));
extern s32 D_L23_001B3934[4] __attribute__((sda));
extern s32 D_L23_001B3938[4] __attribute__((sda));
extern s32 D_L23_001B3934_2[4] __attribute__((sda)); /* the same word as D_L23_001B3934 */
extern s32 D_L23_001B3980[16];
extern float D_L23_002A7170[][4];
extern s32 D_L23_0025E8B0[];
typedef struct { s16 a; s16 b; s32 c; } Edge; /* 8 bytes; the array is walked through an s16 cast */
extern Edge D_L23_002A7A74[];
extern s32 D_L23_0025E9D0[];
extern void func_002A7750(void *, s32, s32);

/* arms the object once: clears the list, builds the 500/500/575 start position and the 16 slots
   (the count, spread over the list of used edges, scaled by sixteenths) */
void func_L23_00429DC8(u8 *obj) {
    s32 *payload;
    s32 i, j, k, m, n, found;
    V4 v;

    if (obj[0x20] == 0) {
        payload = *(s32 **)(obj + 0x68);
        D_L23_001B3934_2[0] = 0;
        D_L23_001B1B94 = -1;
        D_L23_001B1B96 = -1;
        D_L23_001B1B98 = 0;
        D_L23_001B3938[0] = 0;
        *(s16 *)(obj + 0x32) = 0x50;
        obj[0x30] = 0xFF;
        payload[1] = 0;
        payload[0] = 0;
        payload[2] = 0;
        v.q = 0;
        v.f[0] = 500.0f;
        v.f[1] = 500.0f;
        v.f[2] = 575.0f;
        *(TI128 *)(obj + 0x10) = v.q;
        func_002A7750(obj, 1, 0);
        for (i = 0; i < 0x48; i++) {
            float *p = D_L23_002A7170[i];

            if (0.0f < p[2]) {
                D_L23_0025E8B0[i] = 1;
                for (j = 0; j < 0x9C; j++) {
                    if (((s16 *)D_L23_002A7A74)[j * 4] == i || ((s16 *)D_L23_002A7A74)[j * 4 + 1] == i) {
                        n = D_L23_001B3934[0];
                        found = 0;
                        for (k = 0; k < n; k++) {
                            if (D_L23_0025E9D0[k] == j) {
                                found = 1;
                                break;
                            }
                        }
                        if (!found) {
                            D_L23_0025E9D0[D_L23_001B3934[0]] = j;
                            D_L23_001B3934_2[0] = D_L23_001B3934[0] + 1;
                        }
                    }
                }
            } else {
                D_L23_0025E8B0[i] = 0;
            }
        }
        n = D_L23_001B3934_2[0];
        for (m = 0; m < 16; m++) {
            D_L23_001B3980[m] = (m * n) / 16;
        }
    }
}
