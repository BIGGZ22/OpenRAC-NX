/* func_L23_0035CE90 -- NEAR MISS, not integrated. THE CATALOGUE ROW IS WRONG.
   The row starts at 0x0035CE90 with 288 bytes, but its first 16 bytes are padding after
   the previous function's `jr $ra` at 0x0035CE84: `sw $t3,0x40($v1); nop;
   addiu $sp,$sp,0x20; nop`. The function is 0x0035CEA0..0x0035CFAF, 272 bytes
   (tools/levels.py should bind that).
   state: this C compiles to 264 bytes against the 272 of the real body, 8 bytes short.
   The two missing instructions are retail's `nop` at +0x44 (after the table loop's
   addiu) and its alignment `nop` at +0xDC (before the search loop's head, which is
   8-byte aligned in retail). The rest is instruction order and register numbering.
   Belongs in src/levels/23_damosel_orbit/L23_0035CE90-g8.c: it reads $gp
   (`D_L23_001AA770` is a gp-relative pointer), so the game compiler builds it with -G8.
*/

struct Rec {
    unsigned short a;
    unsigned short b;
    int c;
    unsigned short d;
    unsigned short e;
};

struct Tbl {
    unsigned short x;
    unsigned short p;
    unsigned short y;
    unsigned short q;
    int z;
};

extern int D_00139948[];

extern struct Rec D_L23_0021E770[];

extern struct Tbl D_L23_0028FA20[];

extern char D_L23_0028B4E0[];

extern int *D_L23_001AA770 __attribute__((sda));

extern char D_L23_001CAC20[];

int func_L23_0035CE90(char *arg0) {
    struct Rec *r = D_L23_0021E770;
    int *p = D_00139948;
    int n = 0;

    if (*p != 0) {
        do {
            int v = *p;

            r->b = 3;
            p++;
            r->c = (int)D_L23_0028B4E0;
            r->d = D_L23_0028FA20[v].y;
            r->a = D_L23_0028FA20[v].x;
            n++;
            r++;
            if (n >= 28)
                break;
        } while (*p != 0);
    }
    D_L23_0021E770[n].a = 0;
    *(int *)(arg0 + 0x40) = 0;
    *(int *)(arg0 + 0x30) |= 0x8000;
    if (D_L23_001AA770[0] != 0) {
        if (*(int *)(D_L23_001CAC20 + 0x230) == D_L23_001AA770[0]) {
            *(int *)(arg0 + 0x40) = 0;
        } else {
            int i = 1;

            for (;;) {
                if (D_L23_001AA770[i] == 0)
                    break;
                if (D_L23_001AA770[i] == *(int *)(D_L23_001CAC20 + 0x230)) {
                    *(int *)(arg0 + 0x40) = i;
                    break;
                }
                i++;
            }
        }
    }
    return 0;
}
