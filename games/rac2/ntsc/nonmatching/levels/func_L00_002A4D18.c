/* func_L00_002A4D18 (152 bytes, 25 places) -- NEAR MISS, not integrated.
   state: 152 bytes, 30 of 38 words identical; the fault is instruction scheduling.
   check.py (00_aranos_tutorial), with the source below:
     002A4D94  sw    $zero, 0x2294($v0)   |  swc1  $f3, 0x250($v0)
     002A4DA0  nop                        |  sw    $zero, 0x2294($v0)   <- delay slot
   Everything through the `sb` at +0x5C matches; ours then emits the 0x2294 store three
   slots early, so the jal's delay slot gets a nop and the block is 39 instructions
   against retail's 38. Forcing that store's position is what is left.

   What is measured and right:
   - the file must build at -O2 -G8 -ffunction-sections (src/levels/shared/L00_002A3B80.c):
     at -G0 the D_001A79F0 load comes out `lui $a0; lw $v1,0x0($a0)` where retail has
     `lui $v1; lw $v1,%lo`, so -G8 is not optional here.
   - `volatile` on D_001A73A0 (0x1A73A0) is what makes the compiler emit retail's pair
     `lui $at,0x1A; sw $zero,0x73A0($at)`; a plain declaration gives the one-instruction
     `sw $zero,0x0($gp)` (148 bytes, tested), `__attribute__((nosda))` gives
     `lui $a0; sw $zero,0x0($a0)` (hoisted above the whole store block), and the address
     as a number gives three instructions (156 bytes).
   - the source order of the two 0.8f/0.7f pairs decides the schedule: with the
     0x250/0x254 pair written FIRST (this file) the compiler emits retail's
     264, C24, 258, 25C, sb order; with 0x258/0x25C first it emits sb third.
   Tried and not enough: eight source orders of the seven stores; the 0x2294 store
   written before/after the pairs and before/after the 0x73A0 store; both zero-stores
   volatile (the 0x73A0 pair is then hoisted to the top of the store block); a local
   pointer for 0x2294; the 2294 store after the first call (grows the frame instead).
   Next idea: a lever that pins the 0x2294 store's slot - it always lands immediately
   after the byte store whatever its source position. */

extern char D_00189E20[];

extern int D_001A79F0;

extern volatile int D_001A73A0;

extern void func_L00_002D24B8(void);

extern void func_L00_002C42E0(int, int);

extern void func_L00_003B09F8(void);

/* clears two resident words, publishes a mode-derived byte and four floats into the
   resident block, then re-runs three engine routines */
void func_L00_002A4D18(void)
{
    char *base = D_00189E20;

    *(float *)(base + 0x264) = 0.45f;
    *(float *)(base + 0xC24) = 1.0f;
    *(float *)(base + 0x250) = 0.8f;
    *(float *)(base + 0x254) = 0.7f;
    *(float *)(base + 0x258) = 0.8f;
    *(float *)(base + 0x25C) = 0.7f;
    *(unsigned char *)(base + 0x24F0) = (D_001A79F0 == 0x1A);
    D_001A73A0 = 0;
    *(int *)(base + 0x2294) = 0;
    func_L00_002D24B8();
    func_L00_002C42E0(0, 1);
    func_L00_003B09F8();
}
