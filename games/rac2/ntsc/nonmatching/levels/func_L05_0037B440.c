/* Near miss from src/levels/shared/L05_0037AEC0-g8.c: the build refuses this body at its
   place — 224 bytes, expected 248. Kept as a starting point;
   not linked, not counted. */

/* Declaration recovered from src/levels/shared/L05_0037AEC0.c. */
extern void func_L05_0037AEC0(char *object, int flag);


/* Standalone declarations recovered from src/levels/shared/L05_0037AEC0-g8.c. */
extern int D_L05_001AC4D8 __attribute__((sda));

extern char *D_L05_001B251C __attribute__((sda));

extern int D_L05_00225DD0[];

extern int D_L05_001FB220[];

extern char *func_379CA0(char *object);

char *func_L05_0037B440(int id, int flag)
{
    int i;
    int count = D_L05_001AC4D8;
    char *spawned = 0;

    for (i = 0; i < count; i++) {
        char *obj = (char *)D_L05_00225DD0[i];

        if (*(short *)(obj + 0xAA) == id) {
            short *entry = *(short **)(D_L05_001FB220 + *(unsigned char *)(obj + 0x21));

            if (entry == 0)
                return 0;
            do {
                unsigned int offset = (*entry & 0x7FFF) << 8;
                char *v = func_379CA0(D_L05_001B251C + offset);

                if (v != 0) {
                    if (*(int *)(v + 0x110) == 0) {
                        spawned = D_L05_001B251C + offset;
                        func_L05_0037AEC0(spawned, flag);
                        return spawned;
                    }
                }
            } while (*entry++ >= 0);
            return spawned;
        }
    }
    return spawned;
}
