/* func_L00_003AB8B8: the retail body (264 bytes) rebuilt from its bytes.
 *
 * The candidate is the exact size and the exact instruction sequence of the tail; what is
 * left is the compiler's choice of registers and the order of a few independent
 * instructions:
 *   - the global base 0x189E20 sits in $t0 for the early reads (0x2294, 0x24EC, 0x760)
 *     and in $s0 for the late ones (0xBF8); this cc1 keeps one base in $s0 throughout;
 *   - the first parameter is copied into $s1 and the second into $s2; this cc1 gives
 *     $s1 to the second parameter (its first use is the earlier one, in the else arm);
 *   - the index byte is loaded into $v1 and the 0xE0 stride into $a3; this cc1 uses $v1
 *     and $a3 the other way round, and materialises the float constant before the
 *     multiply where retail does it after.
 * The register allocation follows the first textual use of a value (measured), so the
 * order of the arms has to change for the copy order to change with it.
 */
typedef struct {
    char pad[0x50];
    float angle;
    char tail[0x8C];
} AngleRecord_262570;

typedef struct {
    char pad[0x760];
    char pad2[0xBF8 - 0x760];
    void *pBF8;
    char pad3[0x2294 - 0xBFC];
    int i2294;
    char pad4[0x24EC - 0x2298];
    short h24EC;
} Res_189E20;

extern unsigned char D_00139580[];
extern AngleRecord_262570 D_L00_00262570[];
extern Res_189E20 D_00189E20;
extern char D_L00_001B8FF0[];
extern void func_00282F20(void *, void *, float);
extern void func_L00_0032A1A8(void *, void *, void *, float);
extern void func_L00_003AA4C8(void *, void *, void *, void *);

void func_L00_003AB8B8(char *a0, char *a1) {
    float buf[2];
    float angle;
    AngleRecord_262570 *e;
    Res_189E20 *g = &D_00189E20;

    e = &D_L00_00262570[D_00139580[0]];
    angle = e->angle * 0.01666666753590107f;
    if (g->i2294 == 0x1D) {
        func_00282F20(buf, D_L00_001B8FF0, angle);
    } else if (g->h24EC == 1) {
        func_00282F20(buf, a1 + 0xD0, -angle);
    } else {
        func_00282F20(buf, (char *)g + 0x760, angle);
    }
    if (g->pBF8 != 0) {
        func_L00_0032A1A8(g->pBF8, a0, buf, angle);
    }
    func_L00_003AA4C8(a0, buf, a1, g->pBF8);
}
