/* Near miss from src/levels/shared/L05_00428350.c: the build refuses this body at its
   place — 232 bytes, expected 240. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L05_00428350.c. */
extern void func_3066A0(char *, char *, float);

extern void func_306630(char *, char *, char *);

void func_L05_00443C18(char *a0, char *a1)
{
    char *s0 = *(char **)(a1 + 0x68);
    char *a2 = a0;
    char *v0 = *(char **)(a0 + 0x70) + 0x50;
    char buf[16];
    char buf2[16];

    *(float *)(s0 + 0x0) = (*(float *)(v0 + 0x68)
                            / (*(float *)(v0 + 0x78) * 0.017453294f))
                           * *(float *)(s0 + 0x244);
    *(float *)(s0 + 0x4) = (-*(float *)(v0 + 0x64)
                            / (*(float *)(v0 + 0x74) * 0.017453294f))
                           * *(float *)(s0 + 0x248);
    func_3066A0(buf, a2, *(float *)(s0 + 0x240));
    func_306630(buf, a2 + 0x30, buf);
    func_3066A0(buf2, a2 + 0x10, *(float *)(s0 + 0x0));
    func_306630(buf, buf, buf2);
    func_3066A0(buf2, a2 + 0x20, *(float *)(s0 + 0x4));
    func_306630(s0 + 0x20, buf, buf2);
}
