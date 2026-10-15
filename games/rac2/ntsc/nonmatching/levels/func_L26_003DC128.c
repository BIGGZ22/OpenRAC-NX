/* Near miss from src/levels/26_jamming_array/L26_002EC840-g8.c: the build refuses this body at its
   place — instruction at +0x0 differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/26_jamming_array/L26_002EC840-g8.c. */
typedef unsigned char u8;

typedef unsigned short u16;

extern u8 *D_L26_001B1C1C __attribute__((sda));

extern unsigned char D_L26_001FA6E0[];

void func_L26_003DC128(u8 *a0) {
    u8 st = *(u8 *)(a0 + 0x21);

    if (st != 0xFF) {
        u8 *v0 = *(u8 **)(D_L26_001FA6E0 + (st << 2));

        if (v0 != 0) {
            u8 *a2 = D_L26_001B1C1C;
            int t0 = 0xC8E;

            do {
                int a1 = (int)(a2 + (((*(u16 *)v0) & 0x7FFF) << 8));

                if (*(signed char *)(a1 + 0x20) >= 0 && *(short *)(a1 + 0xAA) == t0)
                    *(short *)(*(int *)(a1 + 0x68) + 0x226) = 1;
                v0 += 2;
            } while (*(short *)v0 >= 0);
        }
    }
}
