/* 2026-10-20, with func_L23_0041CCC0's recipe (now C in PR #221): taking the row pointer
   after the count store (`D_001B2AB8[i] = 0; row = D_0023B530[i]; for (j = 0x31; j >= 0; j--)
   row[j] = 0;`) and declaring D_001B2AC4 and D_001B2AB8 `sda` gives retail's whole loop,
   counter in place. What is left is D_001B2AB4: retail loads it after the loop as
   `lui $v1,0x1B ; lw $v1,0x2AB4($v1)` (one register). A plain extern hoists the `lui` above
   the loop (one word early, which also removes the loop label's alignment `nop`); `sda`,
   as a pointer or as an array of unknown size, hoists the whole load. */

/* func_L22_003A3CA0 (176 bytes, 2 places, default profile -O2 -G0)
   -- measured 0 of 2: best candidate is 180 bytes, one instruction over.

   Same reset family as func_L23_0041CCC0.c (read that note too): clears three
   50-word blocks at D_0023B530 + i * 0xF0 (stride 60 words), zeroes
   D_001B2AB8[3] and D_001B2AC4, then latches the object's two counters
   (+0x32A, +0x32E) into D_001B1920[2..3] and into the object D_001B2AB4 points
   at, clearing slot [0] and [1] of both.

   The 2-D form `s32 D_0023B530[3][60]` is the one that gives retail's address
   arithmetic (`addu $v1,$v0,$t1` then `addiu $v1,$v1,0xC4`). The tail must write
   the two targets with the values read once each (`b = st->32E; A[3] = b; B[3] =
   b; a = st->32A; A[2] = a; B[2] = a;`), which is the 180-byte form; interleaving
   the loads with the stores, or grouping by target, costs one or three more
   instructions (188).

   The residue is exactly func_L23_0041CCC0's: this cc1 gives the outer loop's
   counter a second pseudo, printing `addiu $v0,$a1,0x1` ... `daddu $a1,$v0,$zero`
   where retail has one in-place `addiu $a0,$a0,0x1`. Two functions of the family
   now show it, from different source shapes, so it is a property of the loop and
   not of the tail (the .loop RTL of the twin holds `(set (reg/v:SI 86) (reg:SI
   125))`). */

typedef signed short s16;
typedef signed int s32;

extern s32 D_001B2AC4;
extern s32 D_001B2AB8[3];
extern s32 D_0023B530[3][60];
extern s32 D_001B1920[4];
extern s32 *D_001B2AB4;

void func_L22_003A3CA0(char *arg0) {
    char *st = *(char **)(arg0 + 0x68);
    s32 i, j;
    s32 *q = D_001B2AB4;
    s32 a, b;

    D_001B2AC4 = 0;
    for (i = 0; i < 3; i++) {
        D_001B2AB8[i] = 0;
        for (j = 0x31; j >= 0; j--)
            D_0023B530[i][j] = 0;
    }
    a = *(s16 *)(st + 0x32A);
    b = *(s16 *)(st + 0x32E);
    D_001B1920[0] = 0;
    D_001B1920[1] = 0;
    D_001B1920[2] = a;
    D_001B1920[3] = b;
    q[0] = 0;
    q[1] = 0;
    q[2] = a;
    q[3] = b;
}
