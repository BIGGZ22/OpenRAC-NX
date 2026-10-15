/* Near miss from src/levels/11_joba/L11_002DE9E0.c: the build refuses this body at its
   place — instruction at +0x8 differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/11_joba/L11_002DE9E0.c. */
extern void func_00282CF0(void *, void *, void *);

extern void func_L02_00333480(char *, char *, void *);

void func_L11_00340500(char *a0, char *a1) {
    char buf[0x10];

    func_00282CF0(buf, a1, a0 + 0x10);
    func_L02_00333480(a0, a0 + 0x10, buf);
}
