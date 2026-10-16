/* func_L00_00277208 -- src/overlays/shared/pause_00277208.c (functional C for the port, not a match)
 * Page-menu first tick: picks the menu's first page from the entry state (or from a page queued at
 * 0xF0/0xF4), resets the menu camera rows, saves and overrides the projection scale (0.63), syncs the
 * VU1 chain, lays out the menu's frame buffers in VRAM (two layouts, one when a page was queued),
 * sets up the stream buffers, and creates the 14 preview mobys (class 0x472) for the current page.
 * Shape cross-checked with the NTSC decomp's FUN_00218f98 (MIT), the resident twin of this menu.
 * equiv: NEAR (only which call a leftover argument register is credited to). `queued` is only
 * tested for nonzero; the 14 arm sets it to 2 so the compiler keeps the two arms apart, as retail does. */
typedef unsigned int Q_277208 __attribute__((mode(TI), aligned(16)));

typedef struct {
    unsigned char pad0[0x10];
    unsigned char b10;
} AnimInfo_277208;
typedef struct {
    unsigned char pad0[0x48];
    AnimInfo_277208 *anims[1];
} Class_277208;
typedef struct {
    unsigned char pad0[0x10];
    float x, y, z;
    unsigned char pad1C[0x24 - 0x1C];
    Class_277208 *cls;
    unsigned char pad28[0x34 - 0x28];
    unsigned short flags;
    unsigned char pad36[0x40 - 0x36];
    int r40, r44, r48;
    unsigned char pad4C[0x74 - 0x4C];
    void *update;
} Moby_277208;
typedef struct {
    int state;
    int *page;
    int *next;
    int unkC;
    int unk10;
    int unk14;
    int unk18;
    unsigned char pad1C[0x30 - 0x1C];
    int unk30[4];
    Q_277208 v40;
    Q_277208 v50;
    unsigned char pad60[0xCC - 0x60];
    int unkCC;
    int unkD0;
    unsigned char padD4[0xE4 - 0xD4];
    int unkE4;
    float unkE8;
    unsigned char padEC[0xF0 - 0xEC];
    int *queued;
    int queuedState;
    unsigned char padF8[0xFC - 0xF8];
    int unkFC;
    int unk100;
    int unk104;
    int unk108;
    int unk10C;
    unsigned char pad110[0x124 - 0x110];
    int unk124;
    unsigned char pad128[0x130 - 0x128];
    int unk130;
} Menu_277208;
typedef struct {
    unsigned char pad0[4];
    int f4;
    int f8;
} Vram_277208;
typedef struct {
    unsigned char pad0[0xC];
    unsigned char bC;
} Cnt_277208;

extern Menu_277208 M_277208 __asm__("D_L00_001BA070");
extern int D_L00_00173F40_277208[] __asm__("D_L00_00173F40");
extern int D_L00_001B3898_277208[] __asm__("D_L00_001B3898");
extern int D_L00_001B6408_277208[] __asm__("D_L00_001B6408");
extern int D_L00_001B7570_277208[] __asm__("D_L00_001B7570");
extern int D_L00_001B6EB8_277208[] __asm__("D_L00_001B6EB8");
extern int D_L00_001B8A48_277208[] __asm__("D_L00_001B8A48");
extern int D_L00_001B2908_277208[] __asm__("D_L00_001B2908");
extern int D_L00_0015F68C_277208 SDATA(D_L00_0015F68C);
extern int D_0015EFD8_277208 __asm__("D_0015EFD8") MACRO_ADDR;
typedef struct {
    unsigned char pad0[0x224];
    int unk224;
    unsigned char pad228[0x240 - 0x228];
    int unk240;
    int unk244;
    int bufs[4];
} Snd_277208;
extern Snd_277208 S_277208 __asm__("D_L00_001842F0");
extern Q_277208 D_L00_00180340_277208[] __asm__("D_L00_00180340");
extern int D_L00_00160340_277208 SDATA(D_L00_00160340);
extern int D_L00_00160350_277208 SDATA(D_L00_00160350);
extern Q_277208 D_L00_00166EC0_277208[] __asm__("D_L00_00166EC0");
extern char D_L00_0016CB40_277208[] __asm__("D_L00_0016CB40");
extern int D_L00_0015F4F8_277208 __asm__("D_L00_0015F4F8") MACRO_ADDR;
extern Vram_277208 D_L00_00173F00_277208 __asm__("D_L00_00173F00");
extern int D_L00_0016128C_277208 __asm__("D_L00_0016128C") MACRO_ADDR;
extern int D_L00_0015F658_277208 SDATA(D_L00_0015F658);
extern int D_0013E604_277208 __asm__("D_0013E604");
extern int D_0015EF84_277208 __asm__("D_0015EF84") MACRO_ADDR;
extern int D_0015EF78_277208 __asm__("D_0015EF78") MACRO_ADDR;
extern Cnt_277208 *D_L00_00197680_277208 __asm__("D_L00_00197680");
extern int D_00141760_277208[] __asm__("D_00141760");
extern Moby_277208 *D_L00_001BA220_277208[] __asm__("D_L00_001BA220");
extern char D_L00_00166D80_277208[] __asm__("D_L00_00166D80");

extern void func_0020C7A0_277208(void) __asm__("func_0020C7A0");
extern void func_001F9C30_277208(void *, void *, float) __asm__("func_001F9C30");
extern void func_001F3140_277208(void) __asm__("func_001F3140");
extern void func_00219C08_277208(void) __asm__("func_00219C08");
extern void func_001F2608_277208(void) __asm__("func_001F2608");
extern void func_00234AC8_277208(int) __asm__("func_00234AC8");
extern void func_00122598_277208(int) __asm__("func_00122598");
extern void func_00226D50_277208(int) __asm__("func_00226D50");
extern void func_002348E8_277208(void) __asm__("func_002348E8");
extern void func_L00_002472D0_277208(void) __asm__("func_L00_002472D0");
extern void func_L00_00235CA0_277208(int) __asm__("func_L00_00235CA0");
extern void func_L00_00235960_277208(int) __asm__("func_L00_00235960");
extern void func_L00_002A21A8_277208(int, int, int) __asm__("func_L00_002A21A8");
extern void func_L00_00235FF8_277208(int) __asm__("func_L00_00235FF8");
extern Moby_277208 *func_00226720_277208(int) __asm__("func_00226720");
extern void func_00213D28_277208(Moby_277208 *, int, int) __asm__("func_00213D28");
extern void func_L00_002E0AE8_277208(void) __asm__("func_L00_002E0AE8");

void func_L00_00277208(void) {
    Menu_277208 *m = &M_277208;
    int *page;
    int queued = 0;
    int s, i, j, n, k, t, u;
    int a, b, c, d;
    int *p;

    D_L00_00173F40_277208[4] |= 0x80000000;
    s = m->state;
    m->unk124 = 0;
    m->unkD0 = 0;
    if (s == 10) {
        M_277208.state = 11;
        page = D_L00_001B3898_277208;
        m->next = page;
        queued = 1;
    } else if (s == 14) {
        M_277208.state = 15;
        page = D_L00_001B6408_277208;
        m->next = page;
        queued = 2;
    } else if (s == 0x21) {
        M_277208.state = 0x22;
        page = D_L00_001B7570_277208;
        m->next = page;
        m->unk124 = 1;
        D_L00_0015F68C_277208 = 1;
    } else if (s == 0x23) {
        page = D_L00_001B6EB8_277208;
        m->next = page;
    } else if (s == 0x2D) {
        page = D_L00_001B8A48_277208;
        M_277208.state = 2;
        m->next = page;
    } else {
        page = D_L00_001B2908_277208;
        M_277208.state = 2;
        m->next = page;
    }
    m->page = page;

    if (M_277208.queued != 0) {
        D_0015EFD8_277208 = M_277208.unk130;
        M_277208.next = M_277208.queued;
        M_277208.page = M_277208.queued;
        t = M_277208.queuedState;
        M_277208.state = t;
        if (t == 11 || t == 15) {
            S_277208.unk224 = M_277208.unkE4;
            func_0020C7A0_277208();
            queued = 1;
        }
        M_277208.queuedState = 0;
        M_277208.queued = 0;
    }

    {
        char *r = (char *)D_L00_00180340_277208 + 0x10;
        func_001F9C30_277208(r, &D_L00_00160340_277208, 1.0f);
        qcopy(r - 0x10, &D_L00_00160350_277208);
        r += 0x20;
        qzero(r);
    }
    qzero((char *)D_L00_00180340_277208 + 0x20);
    qcopy(&M_277208.v40, &D_L00_00166EC0_277208[0]);
    qcopy(&M_277208.v50, &D_L00_00166EC0_277208[1]);
    M_277208.unkE8 = *(float *)(D_L00_0016CB40_277208 + 0xB0);
    *(float *)(D_L00_0016CB40_277208 + 0xB0) = 0.63f;
    *(float *)(D_L00_0016CB40_277208 + 0x21C) = 524288.0f;
    *(float *)(D_L00_0016CB40_277208 + 0x228) = 255.0f;
    *(int *)(D_L00_0016CB40_277208 + 0x218) = 0;
    *(int *)(D_L00_0016CB40_277208 + 0x22C) = 0;
    func_001F3140_277208();
    func_00219C08_277208();
    func_001F2608_277208();
    func_00234AC8_277208(1);
    func_00122598_277208(0);
    D_L00_0015F4F8_277208 = D_L00_0015F4F8_277208 + 1;

    if (queued) {
        a = D_L00_00173F00_277208.f4 + 0x30000;
        b = D_L00_00173F00_277208.f8 + 0x30000;
        c = a + 0x3C000;
        d = b + 0xE0000;
        M_277208.unk100 = d;
        M_277208.unkFC = c + 0x11800;
        D_L00_0016128C_277208 = 0x30000;
        M_277208.unk104 = a;
        M_277208.unk10 = b;
        M_277208.unk108 = c;
        M_277208.unk10C = d;
        func_00226D50_277208(0);
    } else {
        b = D_L00_00173F00_277208.f8 + 0x60000;
        a = D_L00_00173F00_277208.f4 + 0x60000;
        b = b + 0xC000;
        a = a + 0xC000;
        c = a + 0x30000;
        d = b + 0xE0000;
        M_277208.unk100 = d + 0x11800;
        M_277208.unkFC = c + 0xC1000;
        D_L00_0016128C_277208 = 0x60000;
        M_277208.unk104 = a;
        M_277208.unk10 = b;
        M_277208.unk108 = c;
        M_277208.unk10C = d;
        func_00226D50_277208(1);
    }
    D_L00_0015F658_277208 = 0x2000;
    func_002348E8_277208();

    if (M_277208.state == 11 || M_277208.state == 15) {
        func_L00_002472D0_277208();
    }
    func_L00_00235CA0_277208(0);
    if (M_277208.state == 11 || M_277208.state == 15 || M_277208.state == 0x12) {
        func_L00_00235960_277208(1);
    } else {
        func_L00_00235960_277208(0);
    }
    func_L00_002A21A8_277208(M_277208.unk10, D_0015EF84_277208, D_0013E604_277208 << 11);
    t = D_0015EF78_277208;
    M_277208.unk18 = t;
    if (M_277208.state == 11 || M_277208.state == 15) {
        S_277208.unk240 = t;
        u = t + 0x40000;
        S_277208.unk244 = u;
        D_0015EF78_277208 = u + 0x400;
        p = S_277208.bufs;
        for (j = 3; j >= 0; j--) {
            t = D_0015EF78_277208;
            *p = t;
            D_0015EF78_277208 = t + 0x4000;
            p++;
        }
    } else {
        S_277208.unk240 = 0;
        S_277208.unk244 = 0;
        p = &S_277208.bufs[3];
        for (j = 3; j >= 0; j--) {
            *p = 0;
            p--;
        }
    }
    func_L00_00235FF8_277208(1);
    M_277208.unkCC = D_L00_00197680_277208->bC;
    D_L00_00197680_277208->bC += 0x1C;
    {
        int *dst = M_277208.unk30;
        int *src = D_00141760_277208;
        for (n = 3; n >= 0; n--) {
            *dst = *src;
            src++;
            dst++;
        }
    }

    if (M_277208.page != 0) {
        char *mtx = D_L00_00166D80_277208;
        Moby_277208 **slot = D_L00_001BA220_277208;
        for (i = 0; i < 14; i++, slot++) {
            Moby_277208 *o = func_00226720_277208(0x472);
            *slot = o;
            if (o != 0) {
                o->flags &= 0xFFFD;
                (*slot)->update = func_L00_002E0AE8_277208;
                (*slot)->x = *(float *)(mtx + 0x140);
                (*slot)->y = *(float *)(mtx + 0x144);
                (*slot)->z = *(float *)(mtx + 0x148);
                (*slot)->r40 = 0;
                (*slot)->r44 = 0;
                (*slot)->r48 = 0;
                k = M_277208.page[i];
                o = *slot;
                func_00213D28_277208(o, k, o->cls->anims[k]->b10 - 1);
            }
        }
    }
}
