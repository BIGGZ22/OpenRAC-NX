/* Near miss from src/levels/01_oozla/L01_002BEB90.c: the build refuses this body at its
   place — 244 bytes, expected 276. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/01_oozla/L01_002BEB90.c. */
typedef int TI128 __attribute__((mode(TI)));

extern int D_L01_001A79F0;

extern int D_L01_001BD1F4;

extern char D_L01_001BD1C0[];

extern void func_2E3738(void *, void *, float);

extern void func_2E3470(void *, void *, void *);

extern void func_2E2608(void *, int, int, unsigned int, int);

void func_L01_002F21F0(char *arg0) {
    char buffer[0x50];
    char *end = buffer + 0x40;
    int kind;
    int i;

    if (D_L01_001A79F0 != 1)
        return;
    if ((unsigned)(D_L01_001BD1F4 - 0x2A0) < 0xB5)
        kind = 0x44;
    else
        kind = (0xBC < (unsigned)(D_L01_001BD1F4 - 0x355)) ? -1 : 0x43;
    if (kind == -1)
        return;
    func_2E3738(end, arg0 + 0xC0, 0.05000000074505806f);
    for (i = 3; i >= 0; i--) {
        *(TI128 *)&buffer[(3 - i) * 0x10] =
            *(TI128 *)(*(char **)(D_L01_001BD1C0 + 0x50) + i * 0x10 + 0x20);
        func_2E3470(&buffer[(3 - i) * 0x10], &buffer[(3 - i) * 0x10], end);
    }
    func_2E2608(buffer, kind, 2, 0xFF808080u, 0);
}
