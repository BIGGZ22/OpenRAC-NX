/* Best attempt, 6 compiles. tools/check.py: "instruction at +0x5c differs" --
   the size is exactly the retail 156 bytes and 38 of the 39 instructions are
   identical and in the retail order. The single difference is the position of
   the third-argument copy `daddu $a2,$s0,$zero`: the retail schedules it after
   the `lui $a0,%hi; addiu $a0,%lo` address pair, this source puts it between
   the two halves of that pair (so my `addiu` lands one slot later).
   BELONGS IN THE -g8 SIBLING FILE: with -G0 the cc1 address for
   D_L05_001B3500 is materialised into a general register and reused for both
   the `lwc1` and the `swc1` (144 bytes); under -G8 the assembler emits the
   retail `lui $at,%hi; lwc1 %lo($at)` ... `lui $at,%hi; swc1 %lo($at)` pair
   and the body reaches its exact size.
   Tried without effect: `local = object + 0x10` declared before the checks,
   after the checks, and inlined twice at the two call sites. */

extern void func_00282CC0(void *a, void *b, void *c);

extern float func_00282E48(char *a, char *b);

extern char D_L05_001B3510[];

extern char D_L05_001B34F0[];

extern float D_L05_001B3500;

void func_L05_00418440(char *object)
{
    char *state = *(char **)(object + 0x68);
    char *entry = *(char **)(state + 4);
    char *local;

    if (*(short *)(entry + 0xAA) != 0x957)
        return;
    local = object + 0x10;
    if (func_00282E48(local, D_L05_001B3510) > 15.0f)
        return;
    func_00282CC0(D_L05_001B34F0, D_L05_001B34F0, local);
    D_L05_001B3500 = D_L05_001B3500 + 1.0f;
}
