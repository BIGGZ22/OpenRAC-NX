/* Near miss from src/levels/05_feltzin_system/L05_00314E28.c: the build refuses this body at its
   place — 188 bytes, expected 192. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/05_feltzin_system/L05_00314E28.c. */
extern void func_306630(char *dst, char *src, char *src2);

extern void func_306660(char *dst, char *src, char *src2);

extern void func_3066A0(char *dst, char *src, float value);

extern void func_306AD8(char *dst, char *src, char *object);

extern void func_32E728(char *object, int flag, int *mode, char *out);

void func_L05_00416EC0(char *arg0, int arg1) {
    char *sub = *(char **)(arg0 + 0x68);
    int mode;
    char d[16];
    char c[16];
    char a[16];
    char b[16];
    char out[48];

    mode = arg1;
    func_32E728(arg0, 1, &mode, out);
    func_3066A0(a, b, *(float *)(arg0 + 0x2C) * 0.00097656f);
    func_306AD8(c, a, sub);
    func_306AD8(d, a, arg0 + 0xC0);
    func_306660(d, c, d);
    func_306630(arg0 + 0x10, arg0 + 0x10, d);
}
