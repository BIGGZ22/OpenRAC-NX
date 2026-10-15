/* NON_MATCHING func_L06_00305E38 -- src/overlays/l06_blarg/vendor_002FE5D0.c
 * Best so far: SIZE ours 1532 / retail 1544, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Level 06 moby update (class 0x463): switch on moby[0x20] (0 init, 1 check/return, 2 spawn loops, 3 vector loop
 *   Best by size: p1.c (1532 vs 1544, 12 short; state-1 test first, plain if chain). p4.c is the switch form with 
 *   Unblock: a read of the state-2 spawn loops and the state-3 vector code against retail register allocation (reg
 */
extern void func_L00_0028EBF0(int);
extern int func_L00_0028EB98(void *, int);
extern void func_0022ED80(int, int, void *);
extern int func_0022ED80_0B248(int, int, int) __asm__("func_0022ED80");
extern void func_L06_00306440(char *m);
extern void func_L06_002172B0(void *);
extern void func_L00_00258DB0(float *, float, float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9C30(void *, void *, float);
extern f32 func_001F9D10_1edff8(void *, void *) __asm__("func_001F9D10");
extern void func_001F49B0(void (*)(char *), void *);
extern void func_L06_003065B0(char *m);
extern void func_001FA218(float *, float *);
extern void func_001FA1C0(float *, float);
extern void func_001FA540(void *, void *, void *);
extern float func_L00_00258C80(float lo, float hi);
extern void func_L00_001FF4B0(void *, void *, float);
extern float func_002140F8(float, float);
extern int func_001FA8A8(int, int, float);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern int func_001F9850(int);
extern int func_002140B0(int);
extern unsigned char *func_L00_00273F80(float *pos, char *vel, int color, unsigned char life, unsigned char b, int mode, float scale);
extern int D_L06_001622C8 MACRO_ADDR;
extern int D_L06_001622C4 MACRO_ADDR;
extern char *D_L06_00160058 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern char D_L06_00167640[];
extern float D_L06_001FEBE0[];
extern char D_0013E633[];
extern short D_L06_00162288;
extern short D_L06_0016228C;
extern short D_L06_00162290;
extern short D_L06_00162294;
extern short D_L06_00162298;
extern short D_L06_001622A0;
extern short D_L06_001622A4;
extern short D_L06_001622A8;
extern short D_L06_001622AC;
extern short D_L06_001622B0;
extern short D_L06_001622B4;
extern short D_L06_001622B8;
extern short D_L06_001622BC;
extern short D_L06_001622C0;

/* Level 06 update for moby class 1123: state machine on moby[0x20] that drives effect objects. */
void func_L06_00305E38(char *moby)
{
    char *data = *(char **)(moby + 0x78);
    unsigned char state = (unsigned char)moby[0x20];
    int *slot = (int *)(data + 0xF00);
    int n;

    if (state == 1) {
        char *e = D_L06_00160058 + ((*(int *)(data + 0xF10)) << 8);
        if (*(short *)(e + 0xA6) == 0x463) {
            if ((unsigned char)e[0x20] < 3) return;
        }
        moby[0x20] = 2;
        *(int *)(moby + 0x94) = *(int *)(*(char **)(moby + 0x24) + 0x10);
        return;
    }

    if (state == 0) {
        int f10;
        *(float *)(moby + 0x2C) = *(float *)(*(char **)(moby + 0x24) + 0x24) * *(float *)&D_L06_001622A0;
        D_L06_001622C8 = 0x16;
        D_L06_001622C4 = 0x2D;
        slot[1] = 0xB;
        slot[2] = 0x17;
        slot[3] = 0x22;
        slot[0] = 0;
        f10 = *(int *)(data + 0xF10);
        *(int *)(data + 0xF18) = -1;
        if (f10 != -1) {
            moby[0x20] = 1;
            *(int *)(moby + 0x94) = 0;
        } else {
            moby[0x20] = 2;
        }
        return;
    }

    if (state == 2) {
        float tmp[120];
        float f20;
        float f21;
        int i;
        float v;

        if (func_L00_0028EB98(moby, *(int *)(data + 0xF18)) == 0) {
            *(int *)(data + 0xF18) = func_0022ED80_0B248(0, 4, (int)moby);
        }
        func_L06_00306440(moby);

        v = *(float *)&D_L06_00162288 + *(float *)&D_L06_0016228C * D_0015EE6C;
        *(float *)&D_L06_00162288 = v;
        if (1.0f < v) {
            v = v - 1.0f;
            *(float *)&D_L06_00162288 = v;
        }

        f21 = *(float *)&D_L06_00162294 / 30.0f;
        f20 = -(*(float *)&D_L06_00162294 * 0.5f);

        i = 0;
        do {
            if (slot[i] == 0) {
                char *base = data + i * 0x1E0;
                char *p16 = base;
                char *p17 = base;
                char *v18 = (char *)tmp;
                for (n = 0x1D; n >= 0; n--) {
                    func_L06_002172B0(p16);
                    p16 += 0x10;
                    *(float *)p17 = f20;
                    p17 += 0x10;
                    func_L00_00258DB0((float *)v18, 0.0f, *(float *)&D_L06_00162290 * D_0015EE6C);
                    v18 += 0x10;
                    f20 = f20 + f21;
                }
                p16 = base + 0x790;
                {
                    char *a17 = (char *)tmp;
                    char *a18 = (char *)tmp + 0x10;
                    char *a19 = (char *)tmp + 0x20;
                    for (n = 0x1B; n >= 0; n--) {
                        func_001F9BD8(p16, a17, a18);
                        a17 += 0x10;
                        a18 += 0x10;
                        func_001F9BD8(p16, p16, a19);
                        a19 += 0x10;
                        func_001F9C30(p16, p16, 0.333000004f);
                        p16 += 0x10;
                    }
                }
                slot[i] = D_L06_001622C4;
            } else {
                int sv = slot[i];
                char *q = data + i * sv;
                char *p16 = q + 0x10;
                char *p17 = q + 0x790;
                for (n = 0x1B; n >= 0; n--) {
                    func_001F9BD8(p16, p16, p17);
                    p16 += 0x10;
                    p17 += 0x10;
                }
                slot[i] = slot[i] - 1;
            }
            i = i + 1;
        } while (i < 4);

        if (func_001F9D10_1edff8(moby + 0x10, D_L06_00167640) < 48.0f) {
            func_001F49B0(func_L06_003065B0, moby);
        }

        {
            int f14 = *(int *)(data + 0xF14);
            if (f14 >= 0) {
                char *blk = D_L06_00160058 + (f14 << 8);
                if (blk[0xBC]) {
                    int f18;
                    *(int *)(moby + 0x94) = 0;
                    moby[0x20] = 3;
                    f18 = *(int *)(data + 0xF18);
                    if (f18 != -1) {
                        char *e = (D_0013E633 + 0x1D) + f18 * 0x70;
                        if (*(char **)(e + 0x88) == moby && e[0x74] != 0) {
                            func_L00_0028EBF0(f18);
                        }
                    }
                    *(int *)(data + 0xF18) = -1;
                    func_0022ED80(1, 0, moby);
                }
            }
        }
        return;
    }

    if (state == 3) {
        float A[4];
        float V[4];
        float P70[4];
        float P80[4];
        float P90[4];
        float f21;
        float f20;
        int i;

        func_001FA218(A, (float *)(moby + 0x40));
        f21 = 0.0f;
        func_001FA1C0(V, *(float *)&D_L06_00162298);
        func_001FA540(A, A, V);

        i = 0;
        do {
            func_001FA218(V, &D_L06_001FEBE0[i * 4]);
            func_001FA540(V, A, V);
            func_001F9BD8(P70, P70, moby + 0x10);
            P70[3] = 1.0f;
            P70[2] = P70[2] + ((float)i * 1.700000048f + 0.600000024f);
            f20 = 0.0f;
            n = 0x31;
            do {
                float f0 = *(float *)&D_L06_00162294;
                float g;
                float h;
                int r17;
                int s;
                int t;
                unsigned char life;
                int u;
                f0 = func_L00_00258C80(f21, f0 * 0.75f);
                func_L00_001FF4B0(P80, V, f0);
                func_001F9BD8(P80, P80, P70);
                f0 = func_002140F8(f21, 1.0f);
                r17 = func_001FA8A8(*(int *)&D_L06_001622A4, *(int *)&D_L06_001622A8, f0);
                g = func_002140F8(*(float *)&D_L06_001622AC, *(float *)&D_L06_001622B0);
                f20 = g;
                h = func_002140F8((float)*(int *)&D_L06_001622B4, (float)*(int *)&D_L06_001622B8);
                s = func_001FA898_r(h);
                func_L00_00258DB0(P90, *(float *)&D_L06_001622BC * D_0015EE6C,
                                  *(float *)&D_L06_001622C0 * D_0015EE6C);
                t = func_001F9850(s);
                life = (unsigned char)t;
                u = func_002140B0(0xFF);
                func_L00_00273F80((float *)P80, (char *)P90, r17, life, (unsigned char)u, 0, f20);
                n--;
            } while (n >= 0);
            i = i + 1;
        } while (i < 4);
        moby[0x20] = 4;
        return;
    }
}
