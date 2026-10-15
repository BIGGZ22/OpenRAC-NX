/* Near miss from src/levels/shared/L13_00406F18.c: the build refuses this body at its
   place — 344 bytes, expected 348. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L13_00406F18.c. */
extern int func_L13_004094C0(char *, char *);

extern void func_003357C8(char *, char *, int, int);

extern void func_L13_004095F8(char *, char *);

extern float func_002F8938(char *, char *);

extern int D_001A79F0;

int func_L13_00409840(char *a0, char *a1)
{
    char *s0 = func_L13_004094C0(a0, a1);

    if (s0 == 0)
        return 0;

    if (a1 != *(char **)(s0 + 0xC4))
        func_003357C8(a0, a1, -1, 0);

    if (D_001A79F0 == 0x45) {
        func_L13_004095F8(a0, a1 + ((short)*(s0 + 0xC8) << 4) + 0x10);
        if (func_002F8938(a0 + 0x10, a1 + ((short)*(s0 + 0xC8) << 4) + 0x10) <= 0.5f) {
            if (*(short *)(s0 + 0xCA) == *(short *)(s0 + 0xC8))
                return 1;
            if (*(short *)(s0 + 0xCA) < *(short *)(s0 + 0xC8))
                *(short *)(s0 + 0xC8) = *(short *)(s0 + 0xC8) - 1;
            else
                *(short *)(s0 + 0xC8) = *(short *)(s0 + 0xC8) + 1;
        }
        return 0;
    }

    func_L13_004095F8(a0, a1 + ((short)*(s0 + 0xCA) << 4) + 0x10);
    if (func_002F8938(a0 + 0x10, a1 + ((short)*(s0 + 0xCA) << 4) + 0x10) <= 0.5f) {
        if (*(short *)(s0 + 0xCA) == *(short *)(s0 + 0xC8))
            return 1;
        if (*(short *)(s0 + 0xCA) < 1)
            *(short *)(s0 + 0xC8) = *(unsigned short *)(s0 + 0xC8) - 1;
        else
            *(short *)(s0 + 0xC8) = *(unsigned short *)(s0 + 0xC8) + 1;
    }

    return 0;
}
