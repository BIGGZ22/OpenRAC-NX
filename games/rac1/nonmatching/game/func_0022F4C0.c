/* Resident effect draw (space.c resident_effect_entry): builds a GS packet on
 * the stack (tag, texture, A+D list), scales the object vector a0 into it,
 * then for each of the T[mode] rows (mode = D_0013E130.set, T at gp-0x6740 = D_001605C0)
 * transforms four quads of the row table (t30) by the matrices and draws
 * them with func_001F7EF8. The row colour is built from a0[0xBC] and the row
 * count; a0[0xA6] == 0x215 picks the second colour. */
extern char D_001D9C20[];
extern char D_001D9C40[];
extern char D_001D9CC0[];
extern char D_001D9CE0[];
extern char D_001D9D00[];
extern int D_001605C0_t[] __asm__("D_001605C0");
extern long func_001F4868_l(int) __asm__("func_001F4868");
extern int func_002140B0(int);
extern float func_001FA888_f(int) __asm__("func_001FA888");
extern void func_001F9C30(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9EC0(void *, void *, void *);
extern void func_001F7EF8(void *, int, int);

void func_0022F4C0(char *a0) {
    char sp[0xB0];
    char *t30;
    float *src;
    float f0;
    float f1;
    float f20;
    long tex;
    int mode;
    int count;
    int i;
    int k;
    int rem;
    int rowv;
    int color;
    int s;
    char *out;
    char *col;
    char *dst19;

    mode = *(short *)((char *)&D_0013E130 + 0x26);
    if (mode == 1) {
        t30 = D_001D9CC0;
    } else if (mode == 2) {
        t30 = D_001D9CE0;
    } else {
        t30 = D_001D9C40;
    }

    tex = func_001F4868_l(5);
    *(long *)(sp + 0x70) = 0;
    *(long *)(sp + 0x78) = tex;
    *(long *)(sp + 0x80) = 0x0000FF9000000260L;
    *(long *)(sp + 0x88) = 0x0000008000000048L;
    *(void **)(sp + 0xA0) = sp + 0x90;

    src = (float *)D_001D9C20;
    for (k = 0; k < 4; k++) {
        *(float *)(sp + 0x50 + 8 * k) = src[2 * k];
        *(float *)(sp + 0x54 + 8 * k) = src[2 * k + 1];
    }
    func_001F9C30(sp + 0x90, a0, 0.0009765625f);

    count = D_001605C0_t[*(short *)((char *)&D_0013E130 + 0x26)];
    i = 0;
    if (count > 0) {
        do {
            s = *(short *)(a0 + 0xB2);
            rowv = *(unsigned char *)(a0 + 0xBC);
            if (s != 0) {
                rowv += func_002140B0(s);
            }
            f0 = func_001FA888_f(rowv);
            f1 = *(float *)(t30 + i * 16 + 0xC);
            f1 = f1 / 40.0f;
            color = (rowv << 24) | 0x2058B0;
            f20 = f0 * f1;
            if (*(short *)(a0 + 0xA6) == 0x215) {
                color = (rowv << 24) | 0x308000;
            }

            out = sp;
            col = sp + 0x40;
            dst19 = D_001D9D00;
            rem = 3;
            do {
                *(int *)col = color;
                func_001F9C30(out, dst19, f20);
                dst19 += 0x10;
                func_001F9BD8(out, out, t30 + i * 16);
                col += 4;
                rem -= 1;
                func_001F9EC0(out, out, *(char **)&D_0013E130 + 0xC0);
                func_001F9BD8(out, out, *(void **)(sp + 0xA0));
                out += 0x10;
            } while (rem >= 0);

            func_001F7EF8(sp, 0, 0);
            i += 1;
        } while (i < D_001605C0_t[*(short *)((char *)&D_0013E130 + 0x26)]);
    }
}
