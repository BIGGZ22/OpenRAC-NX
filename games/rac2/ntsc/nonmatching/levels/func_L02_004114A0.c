/* Near miss from src/levels/shared/L02_00410450.c: the build refuses this body at its
   place — needs .rodata at its place, which is not C. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L02_00410450.c. */
typedef unsigned char u8;

extern void func_0032B890(int, int, int);

void func_L02_004114A0(u8 *a0) {
    int v = *(int *)(*(void **)(a0 + 0x68) + 0x10);
    int msg;

    switch (v) {
    case 1:
        msg = 0x27B9;
        break;
    case 3:
        msg = 0x27E9;
        break;
    case 4:
        msg = 0x27F6;
        break;
    case 5:
        msg = 0x27EB;
        break;
    case 6:
        msg = 0x27F2;
        break;
    case 7:
        msg = 0x27F3;
        break;
    case 8:
        msg = 0x27F4;
        break;
    case 9:
        msg = 0x27F5;
        break;
    case 0:
    case 2:
    default:
        msg = 0x2793;
        break;
    }
    func_0032B890(0xB, msg, 1);
}
