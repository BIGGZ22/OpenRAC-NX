/* func_L11_0031A738 -- src/overlays/shared/vendor_002C99E0.c (functional C for the port, not a match)
 * Draws the trails of a group of mobys (levels 11 and 17): unless the moby's group byte (+0x21) is
 * 0xFF, builds 16 quad packets whose uvs come from D_L11_001F1750 and whose colours fade along the
 * trail (func_001FA8A8 between 0x8C28AA28 and 0x144646, by step and frame; the step width is
 * D_L11_00162480 = 16 or 5 by D_0015EEB0[2]), then walks the group's moby list (D_L11_001AC540,
 * last entry has bit 15 set): for each other moby of the same class that is visible (+0x31), it
 * draws one quad per recorded segment from its 16-entry position ring (pvars +0xC0, two rows of
 * 0x100, count +0x2C0, head +0x2C4), each corner offset by +-0.25 in z. equiv: FUNCTIONAL. */
typedef unsigned int Q_31A738 __attribute__((mode(TI), aligned(16)));
typedef struct {
    float v[4][4];
    int col[4];
    float uv[4][2];
    long pk[4];
} Quad_31A738;

extern unsigned char D_0015EEB0_31A738[] __asm__("D_0015EEB0");
extern float D_L11_00162480_31A738 SDATA(D_L11_00162480);
extern int D_L11_0015F6B0_31A738 __asm__("D_L11_0015F6B0") MACRO_ADDR;
extern float D_L11_001F1750_31A738[][2] __asm__("D_L11_001F1750");
extern unsigned short *D_L11_001AC540_31A738[] __asm__("D_L11_001AC540");
extern char *D_L11_00160058_31A738 __asm__("D_L11_00160058") MACRO_ADDR;

extern long func_001F4868_31A738(int) __asm__("func_001F4868");
extern float func_001FA888_31A738(int) __asm__("func_001FA888");
extern int func_001FA8A8_31A738(int, int, float) __asm__("func_001FA8A8");
extern void func_001153FC_31A738(void *, int, int) __asm__("func_001153FC");
extern void func_001F9BD8_31A738(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_L00_001FD1D8_31A738(void *, int, int) __asm__("func_L00_001FD1D8");

void func_L11_0031A738(char *m) {
    Quad_31A738 q[16];
    float off[2][4];
    int i, j, k, h, s, cont, nk, nh, skip, c1, c2, vis, jj;
    float base, a;
    unsigned short *p;
    char *o, *pv;

    if (*(unsigned char *)(m + 0x21) == 0xFF) {
        return;
    }
    if (D_0015EEB0_31A738[2] != 0) {
        D_L11_00162480_31A738 = 16.0f;
    } else {
        D_L11_00162480_31A738 = 5.0f;
    }

    for (i = 0; i < 16; i++) {
        long r = func_001F4868_31A738(0x13);
        Quad_31A738 *e = &q[i];
        e->pk[1] = r;
        e->pk[3] = 0x8000000048L;
        e->pk[2] = 0xFF9000000260L;
        e->pk[0] = 0;
        base = func_001FA888_31A738((D_L11_0015F6B0_31A738 + 3) & 3) / (D_L11_00162480_31A738 * 4.0f);
        for (j = 0; j < 4; j++) {
            jj = (j >> 1) - 1;
            q[i].uv[j][0] = D_L11_001F1750_31A738[j][0];
            q[i].uv[j][1] = D_L11_001F1750_31A738[j][1];
            a = func_001FA888_31A738(i - jj) / D_L11_00162480_31A738 + base;
            if (1.0f < a) {
                a = 1.0f;
            } else if (a < 0.0f) {
                a = 0.0f;
            }
            q[i].col[j] = func_001FA8A8_31A738(0x8C28AA28, 0x144646, a);
        }
    }

    func_001153FC_31A738(off, 0, 0x20);
    off[0][2] = 0.25f;
    off[1][2] = -0.25f;

    skip = 1;
    p = D_L11_001AC540_31A738[*(unsigned char *)(m + 0x21)];
    do {
        o = D_L11_00160058_31A738 + ((*p & 0x7FFF) << 8);
        c1 = *(short *)(o + 0xA6);
        c2 = *(short *)(m + 0xA6);
        vis = *(unsigned char *)(o + 0x31);
        if ((c1 ^ c2) == 0) {
            skip = 0;
        }
        if (vis == 0) {
            skip = 1;
        }
        if (!skip) {
            pv = *(char **)(o + 0x78);
            if (*(int *)(pv + 0x2C0) > 0) {
                k = 0;
                do {
                    nk = k + 1;
                    s = (*(int *)(pv + 0x2C4) - k + 15) % 16;
                    h = 0;
                    do {
                        nh = h + 1;
                        for (j = 0; j < 4; j++) {
                            func_001F9BD8_31A738(q[k].v[j], pv + 0xC0 + h * 0x100 + ((s + (j >> 1)) % 16) * 16,
                                                 off[j & 1]);
                        }
                        func_L00_001FD1D8_31A738(&q[k], 0, 0);
                        h = nh;
                    } while (h < 2);
                    k = nk;
                } while (k < *(int *)(pv + 0x2C0));
            }
        }
        skip = 1;
        cont = *(short *)p >= 0;
        p++;
    } while (cont);
}
