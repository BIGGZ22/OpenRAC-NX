/* func_L00_00249720 -- src/overlays/shared/menu_00249720.c (functional C for the port, not a match)
 * Menu hit test: below item 0x100 it is the D_0013D49E flag; from 0x100 it is x (the fourth float
 * argument) in [58, 86] while Ratchet is not in movement group 0x10. Retail falls into
 * func_L00_00249750 for the second half (a linker-joined remnant); this is the whole test.
 * equiv: DIFFERENT by construction: every operation of retail's head matches; the extra items
 * are the x-range test that retail keeps in func_L00_00249750.
 */
extern int D_001414DC_249720 __asm__("D_001414DC");
extern unsigned char D_0013D49E_249720 __asm__("D_0013D49E") NOT_SDA;

int func_L00_00249720(int item, float a, float b, float x) {
    int flag = D_001414DC_249720 == 0x10;
    if (item < 0x100) {
        if (D_0013D49E_249720 == 0) {
            return 0;
        }
        return 1;
    }
    if (58.0f <= x && x <= 86.0f && flag == 0) {
        return 1;
    }
    return 0;
}
