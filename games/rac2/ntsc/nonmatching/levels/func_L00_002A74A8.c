/* func_L00_002A74A8, 300 bytes, 20 placements.  ABANDON - register-pair and address-form
   wall.
   Two independent blocks. The first runs while the mode word (+0x2294) is 125 or 127 and the
   state counter (+0x1B8) is below 30: three ints are read from the owner into a stack array,
   then either cleared or set to (0x90, 0x90, 0xF0) depending on whether the counter's signed
   remainder by four is under three, and handed back to the owner with func_3120B8 before
   func_2A7070 runs. The second, while +0x1248 is not negative, indexes the resident byte table
   at 0x139568 with it to reach a 0xE0-byte record, and when that record's byte +4 is set and
   +0x1220 is not zero runs func_2A70F0 with both.
   Candidate below is 272 bytes (seven instructions short). Exact: both blocks' call inventory,
   the signed `% 4` (movn) sequence, the (old & ~a3) | a2 splice, the v[3] array in the frame.
   Remaining:
   - retail keeps the resident high half in $s1 across the whole body and completes the address
     twice (`addiu $a0,$a1,-0x61E0`, `addiu $s0,$s1,-0x61E0`); this cc1 materialises the base
     per use and needs no $s1 (frame 0x20 against 0x30);
   - the `(A || B) && C` gate is emitted with $a1 as the phi in retail and with direct branches
     here;
   - the signed `% 4` gives the operands to $v1/$v0 the other way round;
   - three stores of the (0x90,0x90,0xF0) arm come out in a different order.
   Unblocking: a spelling that keeps the high half live, e.g. two named base pointers derived
   from one loaded value. */

#include "include_asm.h"

typedef unsigned char u8;

extern char D_00189E20_2[];
extern unsigned char D_00139568[];
extern char D_L00_00262570[];
extern void func_3120E8(void *owner, int *a1, int *a2, int *a3);
extern void func_3120B8(void *owner, int a1, int a2, int a3);
extern void func_L00_002A7070(void);
extern void func_2A70F0(int first, void *second) __asm__("func_L00_002A70F0");

void func_L00_002A74A8(void)
{
    char *base = D_00189E20_2;

    if ((*(int *)(base + 0x2294) == 125 || *(int *)(base + 0x2294) == 127)
        && *(int *)(base + 0x1B8) < 30) {
        int v[3];

        func_3120E8(*(void **)(base + 0x2290), &v[0], &v[1], &v[2]);

        if (*(int *)(base + 0x1B8) % 4 < 3) {
            v[0] = 0;
            v[1] = 0;
            v[2] = 0;
        } else {
            v[1] = 0x90;
            v[2] = 0xF0;
            v[0] = 0x90;
        }

        func_3120B8(*(void **)(base + 0x2290), v[0], v[1], v[2]);
        func_L00_002A7070();
    }

    if (*(int *)(base + 0x1248) >= 0) {
        unsigned char index = D_00139568[*(int *)(base + 0x1248)];
        char *record = D_L00_00262570 + index * 0xE0;

        if (*(u8 *)(record + 4) != 0) {
            int state = *(int *)(base + 0x1220);

            if (state != 0)
                func_2A70F0(state, *(void **)(base + 0x2290));
        }
    }
}
