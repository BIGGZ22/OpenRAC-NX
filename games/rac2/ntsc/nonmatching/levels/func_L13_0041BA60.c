/* Near miss from src/levels/shared/L13_0041B670.c: the build refuses this body at its
   place — 124 bytes, expected 128. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L13_0041B670.c. */
typedef unsigned char u8;

extern void func_0032F620(int, int, int);

void func_L13_0041BA60(u8 *a0) {
    int st = *(int *)(*(void **)(a0 + 0x68) + 0x10);

    switch (st) {
    case 0:
        func_0032F620(0xB, 0x2800, 1);
        break;
    case 1:
        func_0032F620(0xB, 0x27B9, 1);
        break;
    case 2:
    case 3:
    case 4:
        func_0032F620(0xB, 0x2793, 1);
        break;
    default:
        func_0032F620(0xB, 0x2793, 1);
        break;
    }
}
