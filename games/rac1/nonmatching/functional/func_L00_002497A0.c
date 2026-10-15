/* func_L00_002497A0 -- src/overlays/shared/menu_00249720.c (functional C for the port, not a match)
 * Menu hit test: below item 0x100 it is true in movement groups 0x11-0x12 or with hero byte +0x12E4
 * at 1; from 0x100 it is x (the fourth float argument) >= 95. Retail falls into func_L00_002497E0
 * for the second half (a linker-joined remnant); this is the whole test.
 * equiv: DIFFERENT by construction: the extras are the x >= 95 test retail keeps in
 * func_L00_002497E0; the head's loads match up to the base register being known after a label.
 */
typedef struct {
    unsigned char pad0[0x12E4];
    unsigned char b12E4;
    unsigned char pad12E5[0x208C - 0x12E5];
    int group;
} Hero_2497A0;
extern Hero_2497A0 G_2497A0 __asm__("D_0013F450");

int func_L00_002497A0(int item, float a, float b, float x) {
    int r;
    if (item >= 0x100) {
        return x >= 95.0f;
    }
    r = 0;
    if ((unsigned int)(G_2497A0.group - 0x11) < 2 || G_2497A0.b12E4 == 1) {
        r = 1;
    }
    return r;
}
