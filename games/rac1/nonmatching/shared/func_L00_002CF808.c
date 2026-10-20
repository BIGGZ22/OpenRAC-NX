/* NON_MATCHING func_L00_002CF808 -- src/overlays/shared/vendor_002C96D0.c
 * Best so far: BYTES 78/3776 (97.9% of the bytes match), checked 2026-10-20.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * Cannot land as written (#define in a candidate): rewrite that in plain C first.
 * What the last attempts found:
 *   2026-10-20 (pm/p25.c): BYTES 78/3776 (97.9% match). Reached via struct field access for the slot list,
 *   lever 1 for &m->pos, block-local temporaries, and aligned stack layout.
 *   Remaining 78 bytes are local register assignment permutations ( vs  in slot scan, // in
 *   intermediate vectors,  vs  arg copies, and a daddu/addiu emission order).
 */
extern char D_0013E633[];
typedef struct { Vec4f v0; Vec4f v10; float f20, f24, f28; int pad2C; char *tgt; float f34; int pad38; float f3C, f40; int pad44; int cls48; int pad4C[2]; short slot54; short hp56; float vel58, vel5C; } DV;
typedef struct { char pad[0x1FF4]; unsigned char idx, pad1, n, cnt; float f; int list[9]; int slots[6]; } HH;
typedef struct { char pad[0x1FF6]; unsigned char cnt2; char pad2[0x2020 - 0x1FF7]; int slots2[6]; } HH5;
extern int D_L00_0015F6A8 MACRO_ADDR;
extern float D_0015EE60 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern float D_L00_00166EC0[];
extern int D_L00_00173F40[];
extern char D_L00_00173F70[];
extern char *func_L00_002CF3D8(char *, int *);
extern int func_L00_0025D390(char *);
extern float func_001F9D48(void *, void *);
#define func_001F9D48_07408 ((float (*)(void *, void *))func_001F9D48)
extern int func_001160D8(void);
extern void func_0022ED80(int, int, char *);
extern float func_001FA888(int);
extern int func_001F9938(void *);
extern void func_L00_002CF328(char *);
extern float func_001FA748(float, float);
extern float func_001FA790(float, float);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern void func_001FA218(void *, void *);
extern void func_001F9EE8(void *, void *, void *);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9BF0(void *, void *, void *);
extern void func_001F9C30(void *, void *, float);
extern void func_L00_001FF548(void *, void *, float);
extern int func_L00_00259868(void *, void *, float, float, float, int);
extern int func_L00_001F10E0(float, void *, int, void *);
extern void func_L00_0025A890(char *a, int b, int c, float d);
extern void func_L00_001FF500(void *, void *, float);
extern void func_L00_0025AAC0(void *, void *);
extern int func_001FA898(float);
#define func_001FA898_r func_001FA898
extern float func_L00_001FF860(float, float);
extern float func_L00_00259148(float *vel, float cur, float target, float k, float d, float max);
extern void func_0020D678(void *);
extern void func_L00_0025B040(unsigned char *, float);
extern void func_L00_0025F4A8(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int);

static inline char *tg18(void) {
    char *T = (char *)D_L00_00173F40;
    return *(char **)(T + 0x18);
}

/* Drone moby update (class 479): target search, spawn timer and the per-state chase logic. */
void func_L00_002CF808(Moby *m) {
    DV *p;
    char *P;
    char *G;
    char *G2;
    char *best;
    char *mm;
    char *r19;
    char *obj;
    char *q;
    char *pos;
    char *pb4;
    DV *p2;
    char *cls;
    char *hp;
    char *v;
    int st;
    int i, k;
    int i22;
    int i6;
    int *p4;
    int n16;
    int n18;
    int cnt;
    int idx;
    int k16;
    int t;
    int r2;
    int r3;
    int r5;
    int r16;
    int rr;
    float f20;
    float d;
    float fc;
    float tpi;
    float pi;
    float a1, b1;
    float A, C;
    Q4 t20, t30, t40, t50, t60, t70, t80, t90, ta0;

    st = D_L00_0015F6A8;
    p = (DV *)m->pvars;
    if (st == 2 || st == 5 || st == 6) goto L_884;
    P = D_0013E633 + 0xE1D;
    if (*(int *)(P + 0x2084) == 0x32) goto L_884;
    goto L_894;
L_884:
    m->flags |= 0x41;
    return;
L_894:
    m->flags &= 0xFFBE;
    if (m->state < 5) {
        HH *h = (HH *)P;
        if (h->n < ++h->idx) {
            h->idx = 1;
            best = 0;
            i22 = 0;
            h->cnt = 0;
            h->f = func_001FA748(h->f, D_0015EE60 * 0.05f);
            f20 = 1000000.0f;
            for (i6 = 0; i6 < 6; i6++) {
                HH *h = (HH *)(D_0013E633 + 0xE1D);
                mm = (char *)h->slots[i6];
                if (mm == 0) continue;
                if (*(short *)(mm + 0xA6) == 0x1DF && *(unsigned char *)(mm + 0x20) != 0xFE && *(unsigned char *)(mm + 0x20) != 0xFD) {
                    h->list[i22++] = *(int *)(*(char **)(mm + 0x78) + 0x30);
                    if (*(unsigned char *)(mm + 0x20) < 4) {
                        h->cnt += 1;
                    }
                    if (!(i22 < 9)) break;
                } else {
                    h->slots[i6] = 0;
                }
            }
            for (k = i22; k < 9; k++) { HH *hz = (HH *)(D_0013E633 + 0xE1D); hz->list[k] = 0; }
            G = D_0013E633 + 0xE9D;
            r19 = func_L00_002CF3D8(G, (int *)(G + 0x1F7C));
            if (r19 != 0) {
                r5 = func_L00_0025D390(r19);
                qcopy(&t30, r19 + 0x10);
                pb4 = r19 + 0x10;
                if (r5) t30.v[2] = t30.v[2] + *(float *)(r5 + 0x10);
                p4 = (int *)(G + 0x1FA0);
                for (i = 0; i < 6; i++, p4++) {
                    char *q;
                    mm = (char *)*p4;
                    if (mm == 0) continue;
                    q = *(char **)(*(char **)(mm + 0x78) + 0x30);
                    if (q != 0 && *(unsigned char *)(q + 0x20) != 0xFE && *(unsigned char *)(q + 0x20) != 0xFD) continue;
                    d = func_001F9D48_07408(mm + 0x10, &t30);
                    if (d < f20) {
                        best = mm;
                        f20 = d;
                    }
                }
                if (best != 0) {
                    p2 = *(DV **)(best + 0x78);
                    p2->tgt = r19;
                    p2->cls48 = *(short *)(r19 + 0xA6);
                    r3 = func_L00_0025D390(r19);
                    qcopy(&t30, pb4);
                    if (r3) t30.v[2] = t30.v[2] + *(float *)(r3 + 0x10);
                    qcopy(&p2->v10, &t30);
                    *(unsigned char *)(best + 0x20) = 3;
                    r2 = func_001160D8();
                    rr = r2 % 3;
                    if (rr > 0) rr++;
                    func_0022ED80(rr, 0, (char *)m);
                    { HH *hz = (HH *)(D_0013E633 + 0xE1D); hz->list[i22] = (int)p2->tgt; }
                }
            }
        }
        {
            HH *h2 = (HH *)(D_0013E633 + 0xE1D);
            int n = h2->n;
            int ix = h2->idx;
            int kk = n + 1;
            int c = n ? n : 1;
            float dd;
            if (kk == 0) kk = 1;
            dd = func_001FA888(ix);
            p->f40 = dd / (float)c * 6.2831855f - 3.1415927f;
            dd = func_001FA888(ix * (kk / 5) % kk);
            p->f34 = (dd / (float)c * 6.2831855f - 3.1415927f) * 0.25f;
        }
        r5 = (int)func_L00_0025B478((char *)m, 1, 0);
        if (r5 != 0) {
            q = *(char **)(r5 + 0x20);
            t = 0;
            if (q != 0 && *(char **)(q + 0x24) != 0) t = *(short *)(*(char **)(q + 0x24) + 0x46);
            if (t < 9) {
                if (t >= 5) {
                    if (r5 == 0 || q == 0 || *(short *)(q + 0xA6) != 0x4D6 || func_001F9938(&p->hp56) != 0) {
                        m->state = 4;
                    }
                }
            }
        }
    }
    switch (m->state) {
    case 2:
        m->collision = 0;
        qcopy(&t30, (char *)&m->pos);
        func_L00_002CF328((char *)m);
        {
            float kk = D_0015EE60 * 0.05f;
            m->scale = m->scale + (*(float *)((char *)m->pclass + 0x24) - m->scale) * kk;
            p->f20 = p->f20 + (1.0f - p->f20) * kk;
        }
        G2 = D_0013E633 + 0xE1D;
        d = func_001FA748(p->f40, *(float *)(G2 + 0x1FF8));
        A = func_001FA790(d, p->f28);
        p->f28 = func_001FA748(p->f28, A * (D_0015EE60 * 0.05f));
        C = func_001FA790(p->f34, p->f24);
        p->f24 = func_001FA748(p->f24, C * (D_0015EE60 * 0.01f));
        t40.v[0] = func_001F9F90(p->f28) * p->f20;
        t40.v[1] = func_001F9FA8(p->f28) * p->f20;
        t40.v[2] = 0.0f;
        t50.v[0] = 0.0f;
        t50.v[1] = p->f24;
        t50.v[2] = 0.0f;
        t50.v[3] = 0.0f;
        func_001FA218(t70.v, t50.v);
        v = (char *)&t40;
        func_001F9EE8(v, v, t70.v);
        hp = v;
        v = (char *)&t60;
        func_001F9BD8(v, G2 + 0x80, *(char **)(G2 + 0x2080) + 0xE0);
        t60.v[0] = t60.v[0] + t40.v[0];
        t60.v[1] = t60.v[1] + t40.v[1];
        p->f3C = p->f3C + (t60.v[2] - p->f3C) * (D_0015EE60 * 0.05f);
        t60.v[2] = p->f3C + t40.v[2];
        func_001F9BF0(v, v, (char *)&m->pos);
        func_L00_001FF548(v, v, D_0015EE6C * 10.0f);
        if ((func_L00_00259868((char *)m, v, 0.0f, 0.18f, 0.0f, 0) & 1) && m->visible == 0) {
            func_001F9BD8((char *)&m->pos, D_L00_00166EC0, (char *)D_L00_00166EC0 + 0x230);
            func_001F9BF0((char *)&m->pos, (char *)&m->pos, (char *)D_L00_00166EC0 + 0x210);
        }
        qcopy(&t20, (char *)&m->pos);
        if (func_L00_001F10E0(0.2f, &t20, 0, (char *)m) != 0) {
            obj = tg18();
            if (obj != 0) {
                if (*(short *)(obj + 0xA6) == 0 && *(unsigned char *)(obj + 0x20) != 0xFE && *(unsigned char *)(obj + 0x20) != 0xFD) goto L_D0F4;
                if (tg18() == 0 || *(char **)(tg18() + 0x24) == 0)
                    n16 = 0;
                else
                    n16 = *(unsigned char *)(*(char **)(tg18() + 0x24) + 0x46);
            } else {
                n16 = 0;
            }
            if (tg18() != 0 && *(unsigned char *)(tg18() + 0x20) != 0xFE && *(unsigned char *)(tg18() + 0x20) != 0xFD
                && (n16 == 5 || n16 == 7 || n16 == 8)) {
                func_L00_0025A890(hp, (int)m, 0x10000, 1.0f);
                func_L00_001FF500(hp, p, 1.0f);
                t40.v[2] = 1.0f;
                t40.v[3] = 5627.9248046875f;
                ((char *)&t40)[0x18] = 1;
                ((char *)&t40)[0x19] = 2;
                *(unsigned short *)((char *)&t40 + 0x1A) = m->oclass;
                func_L00_0025AAC0(tg18(), hp);
                if (n16 == 5) {
                    if (tg18() != 0 && *(short *)(tg18() + 0xA6) == 0x4D6 && (r16 = p->hp56) > 0) {
                        if (func_001FA898(*(float *)((char *)&t40 + 0x1C)) > 0) {
                            r2 = r16 - func_001FA898(*(float *)((char *)&t40 + 0x1C));
                        } else {
                            r2 = r16 - 1;
                        }
                        p->hp56 = r2;
                    } else {
                        func_L00_0025F4A8((char *)m, p, 0, 0.0f, 0.0f, 0, 1, 1, 0.4f, 0.2f, 4.0f, 1.0f, 1, 8.0f, 1, 1, -1, 0);
                        m->state = 4;
                    }
                }
            }
        }
L_D0F4:
        pi = 3.1415927f;
        func_001F9BF0(p, (char *)&m->pos, t30.v);
        tpi = 6.2831855f;
        a1 = func_L00_001FF860(m->pos.x - t30.v[0], m->pos.y - t30.v[1]);
        b1 = -func_L00_001FF860(func_001F9D48_07408(t30.v, (char *)&m->pos), m->pos.z - t30.v[2]);
        m->rot.z = ((float (*)(float, float, float *, float, float, float))func_L00_00259148)(m->rot.z, a1, &p->vel58, D_0015EE70 * tpi, D_0015EE70 * pi, D_0015EE6C * tpi);
        m->rot.y = ((float (*)(float, float, float *, float, float, float))func_L00_00259148)(m->rot.y, b1, &p->vel5C, D_0015EE70 * tpi, D_0015EE70 * pi, D_0015EE6C * tpi);
        break;

    case 3:
        cls = (char *)m->pclass;
        m->collision = *(int *)(cls + 0x10);
        qcopy(&t30, (char *)&m->pos);
        m->scale = m->scale + (*(float *)(cls + 0x24) - m->scale) * (D_0015EE60 * 0.05f);
        m->collision = *(int *)(cls + 0x10);
        func_L00_002CF328((char *)m);
        q = p->tgt;
        if (q != 0 && *(short *)(q + 0xA6) == p->cls48 && *(unsigned char *)(q + 0x20) != 0xFE && *(unsigned char *)(q + 0x20) != 0xFD) {
            int rz = func_L00_0025D390(q);
            char *w;
            qcopy(&t60, p->tgt + 0x10);
            if (rz) t60.v[2] = t60.v[2] + *(float *)(rz + 0x10);
            w = (char *)&t40;
            func_001F9BF0(w, t60.v, &p->v10);
            func_001F9C30(w, w, 3.1f);
            func_001F9BD8(t50.v, t60.v, w);
            func_001F9BF0(p, t50.v, (char *)&m->pos);
            func_001F9C30(p, p, D_0015EE60 * -0.8f + 1.0f);
            func_001F9BD8((char *)&m->pos, (char *)&m->pos, p);
            qcopy(&p->v10, &t60);
            qcopy(&t20, (char *)&m->pos);
            if (func_L00_001F10E0(0.25f, &t20, 0, (char *)m) != 0) {
                char *T3 = (char *)D_L00_00173F40;
                obj = *(char **)(T3 + 0x18);
                if (obj == 0 || *(unsigned char *)(obj + 0x20) == 0xFE || *(unsigned char *)(obj + 0x20) == 0xFD
                    || obj != p->tgt || *(short *)(obj + 0xA6) == 0) {
                    qcopy((char *)&m->pos, D_L00_00173F70);
                } else {
                    n18 = 0;
                    if (*(char **)(obj + 0x24) != 0) n18 = *(unsigned char *)(*(char **)(obj + 0x24) + 0x46);
                    func_L00_0025A890((char *)&t40, (int)m, 0x10000, 1.0f);
                    func_L00_001FF500((char *)&t40, p, 1.0f);
                    t40.v[2] = 1.0f;
                    t40.v[3] = 5627.9248046875f;
                    ((char *)&t40)[0x18] = 1;
                    ((char *)&t40)[0x19] = 2;
                    *(unsigned short *)((char *)&t40 + 0x1A) = m->oclass;
                    func_L00_0025AAC0(*(char **)(T3 + 0x18), (char *)&t40);
                    if (n18 == 5) {
                        if (*(char **)(T3 + 0x18) != 0 && *(short *)(*(char **)(T3 + 0x18) + 0xA6) == 0x4D6 && (r16 = p->hp56) > 0) {
                            if (func_001FA898(*(float *)((char *)&t40 + 0x1C)) > 0) {
                                r2 = r16 - func_001FA898(*(float *)((char *)&t40 + 0x1C));
                            } else {
                                r2 = r16 - 1;
                            }
                            p->hp56 = r2;
                        } else {
                            func_L00_0025F4A8((char *)m, p, 0, 0.0f, 0.0f, 0, 1, 1, 0.4f, 0.2f, 4.0f, 1.0f, 1, 8.0f, 1, 1, -1, 0);
                            m->state = 4;
                        }
                    }
                }
            }
        } else {
            m->state = 2;
            p->cls48 = -1;
            p->tgt = 0;
        }
        m->rot.z = func_L00_001FF860(m->pos.x - t30.v[0], m->pos.y - t30.v[1]);
        m->rot.y = -func_L00_001FF860(func_001F9D48_07408(t30.v, (char *)&m->pos), m->pos.z - t30.v[2]);
        break;

    case 4:
        q = p->tgt;
        if (q != 0 && *(short *)(q + 0xA6) == p->cls48 && *(unsigned char *)(q + 0x20) != 0xFE && *(unsigned char *)(q + 0x20) != 0xFD) {
            int rz = func_L00_0025D390(q);
            qcopy(&t30, p->tgt + 0x10);
            if (rz) t30.v[2] = t30.v[2] + *(float *)(rz + 0x10);
                func_001F9BF0(p, t30.v, (char *)&m->pos);
            func_001F9C30(p, p, D_0015EE60 * -0.8f + 1.0f);
            func_L00_001FF548(p, p, D_0015EE60 * 0.9f);
            func_001F9BD8((char *)&m->pos, (char *)&m->pos, p);
        } else {
            p->tgt = 0;
        }
        {
            if ((m->alpha -= 4) < 6) m->state = 5;
        }
        break;

    case 5:
        {
        char *P5 = D_0013E633 + 0xE1D;
        HH5 *h5 = (HH5 *)P5;
        h5->cnt2 -= 1;
        h5->slots2[p->slot54] = 0;
        }
        func_0020D678((char *)m);
        return;
    }
    func_L00_0025B040((unsigned char *)m, 0.17f);
}
