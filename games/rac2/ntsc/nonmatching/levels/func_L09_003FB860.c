/* func_L09_003FB860 (164 bytes, 1 place) -- measured 0 of 1.
   (Wave 6 w08, Docker-free ruler; provisional.)

   Six shapes measured in one run:
     v1/v2/v3/v4 (a named pointer for the 8-byte pair, and full expressions):
       160 bytes, one word short -- this cc1 computes the pair base once and reuses
       it with +0/+4, where retail recomputes it (addu $v0,$a0,$a1 then addu $a1,$a1,$a0).
     v5 (pair[0], table, pair[1], full expressions): EXACT SIZE 164, one ordering
       difference left at +0x8: retail materialises D_001B36E0 as an adjacent
       lui/addiu pair before the first save, this cc1 splits the D_00255E90 pair
       around `sd $s0,0x0($sp)` instead.

   Retail clears four things for the index -- the word of D_001B35E0, both words of
   the 8-byte entry of D_00255E90, releases D_001B36E0[index] (setting it to -1) and
   the same for D_001B3720[index] -- with the second pair store in the beq delay slot.
*/

typedef unsigned short u16;

extern int D_001B35E0[];
extern int D_001B36E0[];
extern int D_001B3720[];
extern char D_00255E90[];
extern void func_00291450(int);

void func_L09_003FB860(int index) {
    int h = D_001B36E0[index];

    *(int *)(D_00255E90 + index * 8) = 0;
    D_001B35E0[index] = 0;
    *(int *)(D_00255E90 + index * 8 + 4) = 0;
    if (h != -1) {
        func_00291450(h);
        D_001B36E0[index] = -1;
    }
    if (D_001B3720[index] != -1) {
        func_00291450(D_001B3720[index]);
        D_001B3720[index] = -1;
    }
}
