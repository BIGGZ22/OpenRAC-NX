/* Level init from the level header hdr: loads the frame and fog constants, uploads the VU0
   program, unpacks the shrub and tie records (class lists, rotations, scales, colour
   averages) into the level arena, clears the ent tables and returns the next free address. */

typedef struct { int x0; int x4; unsigned char pad08[0x2C]; int x34; unsigned char pad38[4]; int x3C; } LevelHeaderE9EC8;
typedef struct { float v[3]; float w; } FRowE9EC8;
typedef struct { float v[3]; int w; } IRowE9EC8;
typedef struct { FRowE9EC8 m[4]; unsigned char pad40[0x100]; unsigned char x140[0x80]; } ShrubDataE9EC8;
typedef struct { IRowE9EC8 m[3]; FRowE9EC8 m3; } TieDataE9EC8;
typedef struct {
    float v[3]; float scale; void *data; float x14; short x18; unsigned char cls; unsigned char x1B;
    unsigned short x1C; unsigned short x1E;
} ShrubE9EC8;
typedef struct {
    float v[3]; float scale; float radius; unsigned char pad14[3]; unsigned char x17; short x18;
    unsigned char cls; unsigned char x1B; unsigned short x1C; unsigned short x1E;
} TieE9EC8;
typedef struct {
    unsigned char pad00[0x26]; unsigned short count; ShrubE9EC8 *first; unsigned char pad2C[4];
    float x30[3]; float scale; float x40;
} ShrubClassE9EC8;
typedef struct {
    unsigned char pad00[0xC]; float scale; unsigned char pad10[6]; unsigned short count;
    TieE9EC8 *first; float *x1C; float x20;
} TieClassE9EC8;
typedef struct { unsigned char pad00[0x36]; unsigned short x36; unsigned char pad38[2]; short x3A; unsigned char pad3C[4]; } Ent40E9EC8;
typedef struct { unsigned char pad00[0x20]; unsigned char x20; unsigned char pad21[0x15]; short x36; unsigned char pad38[0xC8]; } Ent100E9EC8;
typedef struct { unsigned char pad00[0x23C]; int x23C; int x240; int x244; } DisplayE9EC8;
typedef struct { int x0; unsigned char *x4; unsigned char *x8; int xC; int x10; int x14; } BufE9EC8;
typedef struct { int x0; unsigned char *x4; } S160E9EC8;

extern void func_001F99B0_E9EC8(void *, int, int) __asm__("func_001F99B0");
extern void func_001F99D8_E9EC8(void *, int) __asm__("func_001F99D8");
extern void func_001F9A00_E9EC8(void *, void *, int) __asm__("func_001F9A00");
extern void func_001E9730_E9EC8(char *) __asm__("func_001E9730");
extern void func_002157C0_E9EC8(int) __asm__("func_002157C0");
extern void func_001F2930_E9EC8(void) __asm__("func_001F2930");
extern void func_001FB448_E9EC8(int, int, int) __asm__("func_001FB448");
extern void func_002347F0_E9EC8(void *) __asm__("func_002347F0");
extern void func_002362B0_E9EC8(void *) __asm__("func_002362B0");
extern void func_0022B8F8_E9EC8(void *, void *) __asm__("func_0022B8F8");
extern void func_00238688_E9EC8(void *, void *) __asm__("func_00238688");
extern float func_001F9CB8_E9EC8(void *) __asm__("func_001F9CB8");
extern float func_001F9B90_E9EC8(float, float) __asm__("func_001F9B90");
extern void func_001F9EC0_E9EC8(void *, void *, void *) __asm__("func_001F9EC0");
extern void func_001F9C48_E9EC8(void *, void *, float) __asm__("func_001F9C48");
extern void func_001F9BD8_E9EC8(void *, void *, void *) __asm__("func_001F9BD8");
extern int func_001FA898_E9EC8(float) __asm__("func_001FA898");
extern float func_001FA888_E9EC8(int) __asm__("func_001FA888");
extern void func_001F3B90_E9EC8(void) __asm__("func_001F3B90");
extern void func_00217EC0_E9EC8(void) __asm__("func_00217EC0");
extern void func_001F6598_E9EC8(void) __asm__("func_001F6598");

extern int D_0015EE80_E9EC8 __asm__("D_0015EE80");
extern float D_0015EE60_E9EC8 __asm__("D_0015EE60") MACRO_ADDR;
extern unsigned char D_0016044C_E9EC8 __asm__("D_0016044C");
extern unsigned char *D_001941D8_E9EC8 __asm__("D_001941D8");
extern unsigned char D_0013F450_E9EC8[] __asm__("D_0013F450");
extern unsigned char D_00187040_E9EC8[] __asm__("D_00187040");
extern unsigned char D_001AADC0_E9EC8[] __asm__("D_001AADC0");
extern BufE9EC8 D_00194200_E9EC8 __asm__("D_00194200");
extern unsigned char D_001942C0_E9EC8[] __asm__("D_001942C0");
extern unsigned char D_00194280_E9EC8[] __asm__("D_00194280");
extern unsigned char D_001B7A60_E9EC8[] __asm__("D_001B7A60");
extern unsigned char D_001C7A60_E9EC8[] __asm__("D_001C7A60");
extern DisplayE9EC8 D_0018CE00_E9EC8 __asm__("D_0018CE00");
extern unsigned char D_0015F584_E9EC8 __asm__("D_0015F584");
extern unsigned char D_0015F585_E9EC8 __asm__("D_0015F585");
extern unsigned char D_0015F586_E9EC8 __asm__("D_0015F586");
extern float D_0015F588_E9EC8 __asm__("D_0015F588");
extern float D_0015F58C_E9EC8 __asm__("D_0015F58C");
extern float D_0015F590_E9EC8 __asm__("D_0015F590");
extern float D_0015F594_E9EC8 __asm__("D_0015F594");
extern float D_00160FC0_E9EC8 __asm__("D_00160FC0");
extern float D_00161070_E9EC8 __asm__("D_00161070");
extern float D_001604E4_E9EC8 __asm__("D_001604E4");
extern int D_00160030_E9EC8 __asm__("D_00160030");
extern int D_001601BC_E9EC8 __asm__("D_001601BC");
extern unsigned char D_00100AE0_E9EC8[] __asm__("D_00100AE0");
extern unsigned char D_0019C2C0_E9EC8[] __asm__("D_0019C2C0");
extern unsigned char D_0019C4C0_E9EC8[] __asm__("D_0019C4C0");
extern unsigned char D_0019BEC0_E9EC8[] __asm__("D_0019BEC0");
extern char D_001E79D8_E9EC8[] __asm__("D_001E79D8");
extern int D_00160F90_E9EC8 __asm__("D_00160F90");
extern Ent40E9EC8 *D_00160F8C_E9EC8 __asm__("D_00160F8C");
extern int D_0016105C_E9EC8 __asm__("D_0016105C");
extern ShrubE9EC8 *D_00161050_E9EC8 __asm__("D_00161050");
extern ShrubE9EC8 *D_00161054_E9EC8 __asm__("D_00161054");
extern ShrubDataE9EC8 *D_00161058_E9EC8 __asm__("D_00161058");
extern unsigned char D_001E1D00_E9EC8[] __asm__("D_001E1D00");
extern ShrubClassE9EC8 *D_001E1A00_E9EC8[] __asm__("D_001E1A00");
extern unsigned char D_001D8440_E9EC8[] __asm__("D_001D8440");
extern TieClassE9EC8 *D_001D82C0_E9EC8[] __asm__("D_001D82C0");
extern int D_001604D0_E9EC8 __asm__("D_001604D0");
extern TieE9EC8 *D_001604D4_E9EC8 __asm__("D_001604D4");
extern TieE9EC8 *D_001604D8_E9EC8 __asm__("D_001604D8");
extern TieDataE9EC8 *D_001604DC_E9EC8 __asm__("D_001604DC");
extern unsigned char *D_001604E0_E9EC8 __asm__("D_001604E0");
extern S160E9EC8 D_00160BB0_E9EC8 __asm__("D_00160BB0");
extern Ent100E9EC8 *D_00160018_E9EC8 __asm__("D_00160018");
extern Ent100E9EC8 *D_0016001C_E9EC8 __asm__("D_0016001C");
extern Ent100E9EC8 *D_00160020_E9EC8 __asm__("D_00160020");
extern unsigned char *D_00160028_E9EC8 __asm__("D_00160028");
extern unsigned char *D_001601AC_E9EC8 __asm__("D_001601AC");
extern int D_001601B0_E9EC8 __asm__("D_001601B0");
extern int D_001601B4_E9EC8 __asm__("D_001601B4");
extern int D_001601B8_E9EC8 __asm__("D_001601B8");
extern unsigned char D_001CDB00_E9EC8[] __asm__("D_001CDB00");
extern unsigned char D_0018C418_E9EC8[] __asm__("D_0018C418");
extern int D_0015F6F0_E9EC8 __asm__("D_0015F6F0");
extern float D_0015F53C_E9EC8 __asm__("D_0015F53C");

unsigned char *func_001E9EC8(LevelHeaderE9EC8 *hdr)
{
    unsigned char *p;
    unsigned char *p2;
    unsigned char *c;
    unsigned char *lc;
    unsigned char *sc;
    unsigned char *tc;
    unsigned short *sp;
    unsigned short *spa;
    unsigned short *spb;
    int n;
    int i;
    int i0;
    int i1;
    int i2;
    int i3;
    int i5;
    int i6;
    int j;
    int k;
    int cls;
    int last;
    int lastTie;
    int size0;
    int size1;
    int size2;
    int size3;
    int size4;
    int r;
    int g;
    int bl;
    int x;
    int y;
    float s;
    float a;
    float b;
    float len0;
    float len1;
    float len2;
    float ia;
    float ib;
    float ic;
    float rad;
    ShrubE9EC8 *sh;
    ShrubDataE9EC8 *sd;
    TieE9EC8 *tie;
    TieDataE9EC8 *td;
    TieClassE9EC8 *tcl;
    ShrubClassE9EC8 *scl;
    Ent100E9EC8 *e;
    ShrubE9EC8 *o;

    /* the pool pointer is loaded where the pool starts */
    func_001F99B0_E9EC8(D_0013F450_E9EC8, 0, 0x2310);
    func_001F99B0_E9EC8(D_00187040_E9EC8, 0, 0x3A0);
    func_001F99B0_E9EC8(D_001AADC0_E9EC8, 0, 0x180);
    if (D_0015EE80_E9EC8 != 0) {
        if (D_0015EE60_E9EC8 == 1.0f) {
            func_002157C0_E9EC8(1);
        }
    } else if (D_0015EE60_E9EC8 != 1.0f) {
        func_002157C0_E9EC8(0);
    }
    D_0016044C_E9EC8 = *(unsigned char *)&D_0015EE80_E9EC8;
    D_00194200_E9EC8.x4 = D_001942C0_E9EC8;
    D_00194200_E9EC8.x8 = D_00194280_E9EC8;
    D_00194200_E9EC8.xC = 0;
    D_00194200_E9EC8.x10 = 0;
    D_00194200_E9EC8.x14 = 0;
    func_001F99D8_E9EC8(D_001B7A60_E9EC8, 0x10000);
    func_001F99D8_E9EC8(D_001C7A60_E9EC8, 0x60);

    c = (unsigned char *)hdr + hdr->x0;
    D_0018CE00_E9EC8.x23C = *(int *)c;
    c += 4;
    D_0018CE00_E9EC8.x240 = *(int *)c;
    c += 4;
    D_0018CE00_E9EC8.x244 = *(int *)c;
    c += 4;
    D_0015F584_E9EC8 = *c;
    c += 4;
    D_0015F585_E9EC8 = *c;
    c += 4;
    D_0015F586_E9EC8 = *c;
    c += 4;
    D_0015F588_E9EC8 = *(float *)c;
    D_0015F58C_E9EC8 = *(float *)(c + 4);
    D_0015F590_E9EC8 = *(float *)(c + 8);
    D_0015F594_E9EC8 = *(float *)(c + 12);
    func_001F2930_E9EC8();
    D_00160FC0_E9EC8 = 512000.0f;
    D_00161070_E9EC8 = 720.0f;
    D_001604E4_E9EC8 = 500.0f;
    D_00160030_E9EC8 = 500;
    D_001601BC_E9EC8 = 0x1F4000;
    func_001FB448_E9EC8(D_0018CE00_E9EC8.x23C, D_0018CE00_E9EC8.x240, D_0018CE00_E9EC8.x244);
    func_002347F0_E9EC8(D_00100AE0_E9EC8);
    func_001F99D8_E9EC8(D_0019C2C0_E9EC8, 0x100);
    func_001F99D8_E9EC8(D_0019C4C0_E9EC8, 0x180);
    func_001F99D8_E9EC8(D_0019BEC0_E9EC8, 0x400);

    lc = (unsigned char *)hdr + hdr->x4;
    n = *(int *)lc;
    lc += 0x10;
    if (n >= 12) {
        func_001E9730_E9EC8(D_001E79D8_E9EC8);
        n = 12;
    }
    if (n != 0) {
        func_001F9A00_E9EC8(D_0019BEC0_E9EC8, lc, n << 6);
    }

    for (i0 = 0; i0 < D_00160F90_E9EC8; i0++) {
        D_00160F8C_E9EC8[i0].x36 = 0xFFFF;
    }
    i1 = 0;
    while (i1 < D_00160F90_E9EC8) {
        sp = (unsigned short *)0x70003000;
        for (j = 0; j < 0x3FF && i1 < D_00160F90_E9EC8; j++, i1++) {
            *sp++ = (unsigned short)i1;
        }
        *sp = 0xFFFF;
        func_002362B0_E9EC8((void *)0x70003000);
    }

    p = D_001941D8_E9EC8;
    sc = (unsigned char *)hdr + hdr->x34;
    D_0016105C_E9EC8 = *(int *)sc;
    sc += 0x10;
    D_00161050_E9EC8 = (ShrubE9EC8 *)p;
    size0 = D_0016105C_E9EC8 << 5;
    p += size0;
    if (D_0016105C_E9EC8 != 0) {
        func_001F99B0_E9EC8(D_00161050_E9EC8, 0, size0);
    }
    size1 = D_0016105C_E9EC8 * 0x1C0;
    p = (unsigned char *)(((unsigned int)p + 0x3F) & ~0x3FU);
    D_00161058_E9EC8 = (ShrubDataE9EC8 *)p;
    p += size1;
    if (D_0016105C_E9EC8 != 0) {
        func_001F99B0_E9EC8(D_00161058_E9EC8, 0, size1);
    }
    last = -1;
    D_00161054_E9EC8 = &D_00161050_E9EC8[D_0016105C_E9EC8];
    for (i2 = 0; i2 < D_0016105C_E9EC8; i2++) {
        sh = &D_00161050_E9EC8[i2];
        sd = &D_00161058_E9EC8[i2];
        cls = D_001E1D00_E9EC8[*(int *)sc];
        sh->cls = (unsigned char)cls;
        if (sh->cls != last) {
            D_001E1A00_E9EC8[sh->cls]->first = sh;
            D_001E1A00_E9EC8[sh->cls]->count = 0;
            last = sh->cls;
        }
        D_001E1A00_E9EC8[sh->cls]->count++;
        sh->data = sd;
        sh->x14 = (float)*(int *)(sc + 4);
        sh->x1E = 0xFFFF;
        sh->x1B = 0;
        sh->x1C = 0;
        k = *(int *)(sc + 0xC);
        sd->m[0] = *(FRowE9EC8 *)(sc + 0x10);
        sd->m[1] = *(FRowE9EC8 *)(sc + 0x20);
        sd->m[2] = *(FRowE9EC8 *)(sc + 0x30);
        sd->m[3] = *(FRowE9EC8 *)(sc + 0x40);
        sh->x18 = (short)k;
        sc += 0x50;
        scl = D_001E1A00_E9EC8[sh->cls];
        sd->m[3].w = scl->x40;
        func_001F9EC0_E9EC8(sh, scl->x30, sd);
        len0 = func_001F9CB8_E9EC8(sd);
        len1 = func_001F9CB8_E9EC8(&sd->m[1]);
        s = func_001F9B90_E9EC8(len0, len1);
        len2 = func_001F9CB8_E9EC8(&sd->m[2]);
        s = func_001F9B90_E9EC8(s, len2);
        sh->scale = scl->scale * s;
        func_001F9C48_E9EC8(sh, sh, sd->m[3].w);
        func_001F9BD8_E9EC8(sh, sh, &sd->m[3]);
        ia = 1.0f / func_001F9CB8_E9EC8(&sd->m[0]);
        sd->m[0].w = ia;
        ib = 1.0f / func_001F9CB8_E9EC8(&sd->m[1]);
        sd->m[1].w = ib;
        ic = 1.0f / func_001F9CB8_E9EC8(&sd->m[2]);
        sd->m[2].w = ic;
        func_001F9A00_E9EC8(sd->x140, sc, 0x80);
        sc += 0x80;
        sh->x1C = *(unsigned short *)sc;
        sc += 0x10;
    }
    spa = (unsigned short *)0x70000000;
    for (i3 = 0; i3 < D_0016105C_E9EC8; i3++) {
        *spa++ = (unsigned short)i3;
    }
    *spa = 0xFFFF;
    func_00238688_E9EC8((void *)0x70000000, spa);

    tc = (unsigned char *)hdr + hdr->x3C;
    D_001604D0_E9EC8 = *(int *)tc;
    tc += 0x10;
    D_001604D4_E9EC8 = (TieE9EC8 *)p;
    size2 = D_001604D0_E9EC8 << 5;
    p += size2;
    if (D_001604D0_E9EC8 != 0) {
        func_001F99B0_E9EC8(D_001604D4_E9EC8, 0, size2);
    }
    p = (unsigned char *)(((unsigned int)p + 0x3F) & ~0x3FU);
    D_001604DC_E9EC8 = (TieDataE9EC8 *)p;
    size3 = D_001604D0_E9EC8 << 6;
    p += size3;
    if (D_001604D0_E9EC8 != 0) {
        func_001F99B0_E9EC8(D_001604DC_E9EC8, 0, size3);
    }
    size4 = D_001604D0_E9EC8 * 0x60;
    D_001604E0_E9EC8 = p;
    p += size4;
    p = (unsigned char *)(((unsigned int)p + 0x3F) & ~0x3FU);
    if (D_001604D0_E9EC8 != 0) {
        func_001F99B0_E9EC8(D_001604E0_E9EC8, 0, size4);
    }
    lastTie = -1;
    D_00160BB0_E9EC8.x4 = D_001604E0_E9EC8;
    D_001604D8_E9EC8 = D_001604D4_E9EC8 + D_001604D0_E9EC8;
    p2 = p + 0x4000;

    for (i = 0; i < D_001604D0_E9EC8; i++) {
        tie = &D_001604D4_E9EC8[i];
        td = &D_001604DC_E9EC8[i];
        cls = D_001D8440_E9EC8[*(int *)tc];
        tie->cls = (unsigned char)cls;
        if (cls != lastTie) {
            lastTie = cls;
            D_001D82C0_E9EC8[cls]->first = tie;
            D_001D82C0_E9EC8[cls]->count = 0;
        }
        D_001D82C0_E9EC8[cls]->count++;
        tie->x18 = (short)i;
        tie->radius = *(float *)(tc + 4);
        if (tie->radius < 16.0f) {
            tie->radius = 16.0f;
        }
        tie->x1B = 0;
        tie->x1E = 0xFFFF;
        tcl = D_001D82C0_E9EC8[cls];
        if (tcl->x1C != 0) {
            tie->x17 = (unsigned char)func_001FA898_E9EC8(*tcl->x1C);
            rad = func_001FA888_E9EC8(tie->x17) + 24.0f;
            if (tie->radius < rad) {
                tie->radius = rad;
            }
        }
        td->m[0] = *(IRowE9EC8 *)(tc + 0x10);
        td->m[1] = *(IRowE9EC8 *)(tc + 0x20);
        td->m[2] = *(IRowE9EC8 *)(tc + 0x30);
        td->m3 = *(FRowE9EC8 *)(tc + 0x40);
        tc += 0x50;
        td->m3.w = tcl->x20;
        len0 = func_001F9CB8_E9EC8(td);
        len1 = func_001F9CB8_E9EC8(&td->m[1]);
        a = (len0 + len1) * 0.5f;
        b = func_001F9CB8_E9EC8(&td->m[2]);
        x = func_001FA898_E9EC8(a * 4096.0f);
        y = func_001FA898_E9EC8(b * 4096.0f);
        if (x > 0x10000) {
            x = 0x10000;
        }
        if (y > 0x10000) {
            y = 0x10000;
        }
        td->m[2].w = (int)((unsigned int)x | ((unsigned int)y << 16));
        r = *(int *)tc;
        tc += 4;
        g = *(int *)tc;
        tc += 4;
        bl = *(int *)tc;
        tc += 8;
        g = (int)(((unsigned int)g << 8) | 0x80000000U);
        td->m[0].w = (int)(((unsigned int)bl << 16) | (unsigned int)g | (unsigned int)r);
        tie->x1C = *(unsigned short *)tc;
        tc += 0x10;
        func_001F9EC0_E9EC8(tie, tcl, td);
        len0 = func_001F9CB8_E9EC8(td);
        len1 = func_001F9CB8_E9EC8(&td->m[1]);
        s = func_001F9B90_E9EC8(len0, len1);
        len2 = func_001F9CB8_E9EC8(&td->m[2]);
        s = func_001F9B90_E9EC8(s, len2);
        tie->scale = tcl->scale * s;
        func_001F9C48_E9EC8(tie, tie, td->m3.w);
        func_001F9BD8_E9EC8(tie, tie, &td->m3);
    }
    spb = (unsigned short *)0x70000000;
    for (i5 = 0; i5 < D_001604D0_E9EC8; i5++) {
        *spb++ = (unsigned short)i5;
    }
    *spb = 0xFFFF;
    func_0022B8F8_E9EC8((void *)0x70000000, spb);

    for (i6 = 0; i6 < D_001604D0_E9EC8; i6++) {
        unsigned char *cp;
        int cr;
        int cg;
        int cb;

        cp = D_001604E0_E9EC8 + i6 * 0x60;
        cr = 0;
        cg = 0;
        cb = 0;
        for (k = 0; k < 24; k++) {
            cr += cp[0];
            cg += cp[1];
            cb += cp[2];
            cp += 4;
        }
        cr /= 24;
        cg /= 24;
        cb /= 24;
        *(int *)((unsigned char *)&D_001604DC_E9EC8[i6] + 0x1C) = (cb << 16) | (cg << 8) | cr;
    }

    D_00160018_E9EC8 = (Ent100E9EC8 *)p;
    func_001F99B0_E9EC8(p, 0, 0x4000);
    p = p2;
    D_0016001C_E9EC8 = D_00160018_E9EC8;
    *((unsigned char *)D_00160018_E9EC8 + 0x20) = 0xFF;
    D_00160028_E9EC8 = p;
    p += 0x2000;
    D_001601AC_E9EC8 = p;
    D_00160020_E9EC8 = (Ent100E9EC8 *)((unsigned char *)D_00160018_E9EC8 + 0x3F00);
    p += 0x20000;
    D_001601B4_E9EC8 = -1;
    D_001601B0_E9EC8 = 0;
    D_001601B8_E9EC8 = 0;
    func_001F99D8_E9EC8(D_001CDB00_E9EC8, 0x200);
    for (e = D_00160018_E9EC8; e != D_00160020_E9EC8; e++) {
        e->x36 = 0x7F80;
    }
    for (o = D_00161050_E9EC8; o != D_00161054_E9EC8; o++) {
        o->x18 = 0x7F80;
    }
    for (i0 = 0; i0 < D_00160F90_E9EC8; i0++) {
        D_00160F8C_E9EC8[i0].x3A = 0x7F80;
    }
    *(int *)(D_0018C418_E9EC8 + 0x1C) = 0;
    func_001F3B90_E9EC8();
    func_00217EC0_E9EC8();
    func_001F6598_E9EC8();
    D_0015F6F0_E9EC8 = 0;
    D_0015F53C_E9EC8 = 1.0f;
    return p;
}
