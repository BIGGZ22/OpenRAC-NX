/* NON_MATCHING func_L08_002D36F8 -- src/overlays/l08_batalia/vendor_002B9438.c
 * Best so far: SIZE ours 1760 / retail 1792, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Bomb (class 248) update on level 08: a state machine on moby[0x20] (0x37 skip, 1 = fuse/arm with random-spread
 *   Differences: the prologue saves a different set of $s registers (ours saves $s4/$s5 where retail saves $s0/$s1
 *   Unblock: a form of the select that compiles to a branch with a single shared store (the movn/movz tie), then t
 */
typedef struct { char b[16]; } blk16;

extern int func_L00_001F10E0(float, void *, int, void *);
extern void func_L00_001FF610(void *, void *, void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern float func_001F9CB8(void *a);
extern float func_001FA748(float, float);
extern float func_002140F8(float, float);
extern s32 func_002140B0(s32);
extern int func_001F9850(int);
extern void func_L00_0026A7F8(void *, void *, int, int, int, int, int, int);
extern void func_001F9BD8(void *, void *, void *);
extern int func_L00_001EFFF0(void *, void *, int, void *, int);
extern int func_0022ED80(int, int, void *);
extern void func_001F9CA0(void *, void *, void *);
extern void func_001F9C30(void *, void *, float);
extern void func_001F9BC0(void *);
extern int func_001F9908(void *);
extern void func_L00_0025E4B0(void *m, short *p);
extern void func_L00_0025E590(void *, void *);
extern int func_L00_001F2BE8_2FB898(float, void *, int, void *, void *) __asm__("func_L00_001F2BE8");
extern void func_L00_0025F4A8(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int);
extern void func_L00_00260108(void *, void *, int, float, float);
extern void func_0020D678(void *);
extern void func_L00_0025B040(unsigned char *, float);
extern float D_0015EE6C_d __asm__("D_0015EE6C") MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern char D_L08_001746C0[];
extern float D_L08_0015F660[];
extern char D_L08_001746E0[];
extern char D_L08_001618F8[];
extern short D_L08_001618F0;
extern int D_L08_001746D8 MACRO_ADDR;

/* Bomb (class 248) update: fuse and blast state machine on the moby's state byte */
void func_L08_002D36F8(char *moby) {
    char *d;
    char *p;
    char *dv;
    char *tbl;
    unsigned char st;
    int r, g, v, n, hit;
    float t, f20, f21;
    blk16 t20, t70, t80, t90, t70b;
    char s30[0x34];
    char buf[0x24];

    d = *(char **)(moby + 0x78);
    dv = d + 0x10;
    st = *(unsigned char *)(moby + 0x20);
    if (st != 0x37) {
        p = moby + 0x10;
        r = func_L00_001F10E0(0.5f, p, 0x11, moby);
        if (r) {
            g = D_L08_001746D8;
            if (g != 0 && (g != *(int *)(d + 0x30) || *(int *)(d + 0x24) != 0)) {
                if (*(unsigned char *)(moby + 0xBC) & 2) st = 0x63; else st = 0x62;
                *(unsigned char *)(moby + 0x20) = st;
                qcopy(p, D_L08_001746E0);
                func_L00_001FF610(dv, dv, D_L08_001746E0 + 0x20);
                func_L00_001FF4B0(dv, dv, D_0015EE6C_d + D_0015EE6C_d);
            }
        }
        st = *(unsigned char *)(moby + 0x20);
    }

    if (st == 0x62 || st == 0x63) {
        func_0022ED80(0, 0, moby);
        *(char **)(buf + 0x10) = moby;
        *(int *)(buf + 0x14) = 0x810001;
        *(float *)(buf + 0x1C) = 1.0f;
        func_001F9BC0(buf);
        *(int *)(buf + 0x20) = 0;
        func_L00_001F2BE8_2FB898(1.0f, moby + 0x10, 0x10, moby, buf);
        if (st == 0x62) {
            func_L00_0025F4A8(moby, D_L08_0015F660, 0, 0.0f, 0.0f, 7, 10, 20, 3.0f, 1.7f, 4.0f, 1.0f, -1, 7.0f, 0, 7, -1, 0);
        } else {
            func_L00_00260108(moby, moby + 0x10, -1, 0.5f, 13.0f);
        }
        func_0020D678(moby);
        return;
    }

    if (st == 2) {
        if (func_001F9908(d + 0x20)) {
            *(unsigned char *)(moby + 0x20) = 0x63;
        } else {
            hit = 0;
            if (func_001F9850(60) == *(int *)(d + 0x20)) hit = 1;
            else if (func_001F9850(40) == *(int *)(d + 0x20)) hit = 1;
            else if (func_001F9850(20) == *(int *)(d + 0x20)) hit = 1;
            if (hit) {
                *(unsigned char *)(d + 7) = 0xFA;
                func_L00_0025E4B0(moby, (short *)d);
            }
        }
        func_L00_0025E590(moby, d);
    } else if (st == 1) {
        p = moby + 0x10;
        t = func_001F9CB8(dv);
        *(float *)(moby + 0x40) = func_001FA748(*(float *)(moby + 0x40), t * 0.5f);
        if (*(int *)(d + 0x24) == 0) {
            f21 = -1.0f;
            f20 = 1.0f;
            *(blk16 *)t70.b = *(blk16 *)D_L08_001618F8;
            qzero(t90.b);
            *(float *)(t90.b + 0) = func_002140F8(f21, f20);
            *(float *)(t90.b + 4) = func_002140F8(f21, f20);
            *(float *)(t90.b + 8) = func_002140F8(f21, f20);
            qcopy(t80.b, t90.b);
            func_L00_001FF4B0(t80.b, t80.b, D_0015EE6C_d);
            {
                int idx = func_002140B0(4) * 4;
                int *q = (int *)(t70.b + idx);
                int s = func_001F9850(45);
                func_L00_0026A7F8(p, t80.b, *q, 0x4FFF, s, 0x32, 0x78, 1);
            }
        }
        /* L392C */
        *(blk16 *)t20.b = *(blk16 *)p;
        f20 = 0.16f;
        *(float *)(t20.b + 8) = *(float *)(t20.b + 8) - f20;
        func_001F9BD8(p, p, dv);
        *(float *)(d + 0x18) = *(float *)(d + 0x18) - *(float *)&D_L08_001618F0 * D_0015EE70;
        *(blk16 *)s30 = *(blk16 *)p;
        *(float *)(s30 + 8) = *(float *)(s30 + 8) - f20;
        *(blk16 *)(s30 + 0x10) = *(blk16 *)dv;
        *(char **)(s30 + 0x20) = moby;
        *(int *)(s30 + 0x24) = 0x810000;
        *(float *)(s30 + 0x2C) = 1.0f;
        *(int *)(s30 + 0x30) = st;
        r = func_L00_001EFFF0(t20.b, s30, 0, moby, 0);
        if (r == 0) {
            if (func_001F9908(d + 0x20)) {
                if (*(unsigned char *)(moby + 0xBC) & 2) st = 0x63; else st = 0x62;
                *(unsigned char *)(moby + 0x20) = st;
            }
        } else {
            tbl = D_L08_001746C0;
            v = *(int *)(tbl + 0x18);
            if (v == 0) {
                if (*(int *)(tbl + 0x1C) > 0) {
                    if (*(unsigned char *)(moby + 0xBC) & 1) {
                        func_0022ED80(1, 0, moby);
                        *(blk16 *)p = *(blk16 *)(tbl + 0x20);
                        t = *(float *)(moby + 0x18) + f20;
                        func_L00_001FF610(dv, dv, tbl + 0x40);
                        *(float *)(moby + 0x18) = t;
                        func_001F9CA0(t70b.b, dv, tbl + 0x40);
                        {
                            float k = D_0015EE6C_d * 1.5f;
                            float res = func_002140F8(-k, k);
                            func_L00_001FF4B0(t70b.b, t70b.b, res);
                        }
                        func_001F9BD8(dv, dv, t70b.b);
                        func_001F9C30(dv, dv, 0.4f);
                        *(int *)(d + 0x24) += 1;
                        *(int *)(d + 0x20) = func_001F9850(180);
                        t = func_001F9CB8(dv);
                        if (t < D_0015EE6C_d) {
                            *(unsigned char *)(moby + 0x20) = 2;
                            r = func_001F9850(120);
                            *(int *)(d + 0x20) = r;
                            *(float *)(d + 0x28) = 254.0f / (float)r;
                            func_001F9BC0(dv);
                        }
                    } else {
                        *(unsigned char *)(moby + 0x20) = 0x62;
                        qcopy(p, tbl + 0x20);
                        func_L00_001FF610(dv, dv, tbl + 0x40);
                        func_L00_001FF4B0(dv, dv, D_0015EE6C_d + D_0015EE6C_d);
                    }
                }
            } else if (v != *(int *)(d + 0x30) || *(int *)(d + 0x24) != 0) {
                if (*(unsigned char *)(moby + 0xBC) & 2) st = 0x63; else st = 0x62;
                *(unsigned char *)(moby + 0x20) = st;
                qcopy(p, D_L08_001746E0);
                func_L00_001FF610(dv, dv, D_L08_001746E0 + 0x20);
                func_L00_001FF4B0(dv, dv, D_0015EE6C_d + D_0015EE6C_d);
            }
        }
    }
    func_L00_0025B040((unsigned char *)moby, 0.2f);
}
