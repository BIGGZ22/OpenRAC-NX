/* NON_MATCHING func_L15_0029E8A0 -- src/overlays/shared/vendor_00298BB8.c
 * Best so far: SIZE ours 4916 / retail 4920, checked 2026-10-14.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   The reference counts for m and t are the same as retail (46 and 8), so it is the allocation order.
 *   - the shake struct's 0x168 store: retail rematerializes the base (lui+addiu+sw 0x168), ours folds it
 *   (lui+sw %lo), which is 1 instruction short. Using a cam pointer for 0x168 puts cam in an s-reg instead.
 *   - the K skew-matrix stores (16 sw/swc1 at sp+0x110) come out in another order (scheduler).
 *   - D_L15_0015F40C load: ours lw $v1 vs retail lw $v0 (MACRO_ADDR fixes this but reorders the spill slots).
 *   Layout lessons (gcc 2.95 temp slots): block arrays are reused only when no expression statement at
 *   their own level runs after their address is taken (expand_expr_stmt preserve_temp_slots bumps them a
 *   level); a for-loop increment is such a statement, hence loop1 as while inside its own block.
 */
typedef int u128_29E8A0 __attribute__((mode(TI)));
typedef struct { int w[20]; } __attribute__((aligned(16))) Tbl_29E8A0;
typedef struct { int c[6]; } Cols_29E8A0;
typedef struct { char pad0[2]; unsigned char f2; unsigned char f3; } Part_29E8A0;
typedef struct {
    float v[4];
    char *moby;
    int flags;
    char kind;
    char one;
    unsigned short id;
    float radius;
    int count;
} Coll_29E8A0;
typedef struct {
    float v[4];
    char *owner;
    int timer;
    struct M_29E8A0 *target;
    float zoff;
    float speed;
    float yawvel;
    float pitchvel;
    float yaw0;
    float pitch0;
    float speed1;
    int f38;
    short f3C;
    short f3E;
} D_29E8A0;
typedef struct M_29E8A0 {
    char pad0[0x10];
    float pos[4];
    signed char state;
    char pad21[0x1F];
    float rot[4];
    char pad50[0x28];
    D_29E8A0 *data;
    char pad7C[0x2A];
    unsigned short id;
    char padA8[0x14];
    unsigned char bc;
    char padBD[3];
    float mtx[16];
} M_29E8A0;
extern Tbl_29E8A0 D_L15_001E2910;
extern Cols_29E8A0 D_L15_001E2960;
extern Cols_29E8A0 D_L15_001E2978;
extern int D_L15_0015F40C;
extern float D_L15_0015F6B4 MACRO_ADDR;
extern unsigned char *D_L15_001B29DC;
extern char D_L15_001B1070[];
extern char D_L15_001744C0[];
extern char *D_L15_00178580[];
extern char D_L15_00167300_x[] __asm__("D_L15_00167300");
extern float D_0015EE64 MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern int func_001F9938(void *);
extern float func_001FA888(int);
extern void func_001F9C30(void *, void *, float);
extern float func_001F9D10(void *, void *);
extern float func_001F9B88(float);
extern void func_L00_001EE2E0(void *, int, float, float, void *, int, int, float, int);
extern int func_001F9850(int);
extern void func_001F9BF0(void *dst, void *a, void *b);
extern void func_L00_001FF270(void *, void *, void *);
extern float func_L00_001FF3E0(void *);
extern float func_001F9B50(float);
extern void func_001F9BD8(void *, void *, void *);
extern float func_L00_001FF860(float, float);
extern float func_001F9CE8(void *);
extern float func_L00_00259148(float *vel, float cur, float target, float k, float d, float max);
extern void func_00215C00(void *, float, float, float);
extern float func_002140F8(float, float);
extern void func_L00_001FF4B0(void *, void *, float);
extern unsigned char *func_L00_00272158(void *pos, float *vec, int s, int a, int col, int n, float x, float y, float z, float w, float pw);
extern void func_001F9EC0(void *, void *, void *);
extern float func_00214158(void);
extern void func_002156E0(void *dst, void *vec, void *axis, float angle);
extern void func_L00_0026DA50(void *, void *, int, int, int, int, float);
extern void func_L00_001FF240(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern int func_L00_001EFFF0(void *, void *, int, void *, void *);
extern void func_L00_001FF610(void *, void *, void *);
extern int func_001F9908(void *);
extern float func_001F9D48(float *, float *);
extern void func_L00_00260878(void *, void *);
extern void func_0020D678(void *);
extern int func_L00_001F2BE8_29FF90(void *, float, int, void *, void *) __asm__("func_L00_001F2BE8");
extern void func_L00_0025BA50(void *, void *, void *, int, int, int, int, int, float, float, float);
extern void func_001FA540(void *, void *, void *);
extern float func_001F9F90(float x);
extern void func_L00_001FFCF8(void *, void *, float);
extern float func_001F9FA8(float);
extern void func_L00_001FFDD0(void *, void *, void *);
extern void func_001FA190(void *);
extern void func_001F9EE8(void *, void *, void *);
extern int func_L00_00258BC8(int, int);
extern void *func_L00_0026CA10_29FF90(void *, void *, int, int, float, int, int, int, int) __asm__("func_L00_0026CA10");
extern float func_001F9CB8(void *);
extern void func_L00_001FF548(void *, void *, float);
extern char *func_L00_002B0738(char *, char *, int, int, int);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern int func_002140B0(int);
extern void func_L00_0026B890(void *, void *, int, int, float, int, int, float, int, int);
extern void func_L00_0026B368(void *pos, void *vel, int color, int tag, int life, float size);
extern void func_L00_002ADBB0(char *, char *, int, int, float, int, int, int, int);
extern int func_0022ED80(int, int, int);
extern void func_L00_002D4CE8(void *, void *, int, int);

/* update of moby class 256: a homing missile that leads its target, trails smoke and explodes on contact */
void func_L15_0029E8A0(M_29E8A0 *m) {
    Tbl_29E8A0 tbl;
    float v50[4];
    float v60[4];
    float v70[4];
    float v80[4];
    float v90[4];
    float vel[4];
    float vB0[4];
    float up[4];
    char *owner;
    D_29E8A0 *d;
    M_29E8A0 *t;
    char *hero;
    float yaw;
    float pitch;
    int k;

    tbl = D_L15_001E2910;
    owner = 0;
    *(u128_29E8A0 *)v80 = 0;
    v80[2] = D_0015EE6C * 8.0f;
    *(u128_29E8A0 *)v70 = *(u128_29E8A0 *)v80;
    *(u128_29E8A0 *)v80 = 0;
    v80[2] = 1.0f;
    d = m->data;
    t = d->target;
    d->speed = d->speed + (d->speed1 - d->speed) / 10.0f * D_0015EE64;
    func_001F9938(&d->f3C);
    {
        float a = func_001FA888(d->f3C);
        float b = func_001FA888(d->f3E);
        func_001F9C30(up, D_0013E633 + 0xF5D, a / b);
    }
    {
        float delta[4];
        float dv[4];
        float tp[4];
        float ip[4];
        if (D_L15_0015F40C != 0 && t != 0) {
            float f = func_001F9D10(D_L15_00167440, t->pos) / 50.0f;
            qcopy(delta, t->pos);
            delta[2] = delta[2] + d->zoff;
            func_L00_001EE2E0(m, 0xFF0FFFFF, func_001F9B88(0.5f - f * 0.35f), 0.0f, delta, 0xD, -1, 90.0f, 4);
        }
        if (t != 0 && d->timer < func_001F9850(0x96) - func_001F9850(0) && t->state >= 0) {
            float s = 0.0f;
            qcopy(tp, t->pos);
            tp[2] = tp[2] + d->zoff;
            func_001F9BF0(delta, tp, m->pos);
            if (d->f38 != 0) {
                float a, b, c, r1, r2, q;
                func_001F9BF0(dv, tp, d->v);
                func_L00_001FF270(ip, dv, up);
                qcopy(d->v, tp);
                c = func_L00_001FF3E0(dv);
                b = (dv[0] * delta[0] + dv[1] * delta[1] + dv[2] * delta[2]) * -2.0f;
                a = d->speed * d->speed - c;
                c = -func_L00_001FF3E0(delta);
                q = func_001F9B50(b * b - a * 4.0f * c);
                r1 = (-b + q) / (a + a);
                r2 = (-b - q) / (a + a);
                if (r1 > 0.0f && r2 > 0.0f) {
                    if (r2 < r1) s = r1;
                    else s = r2;
                } else if (r1 > 0.0f) {
                    s = r1;
                } else if (r2 > 0.0f) {
                    s = r2;
                } else {
                    s = -1.0f;
                }
            }
            if (s > 0.0f && s < (float)func_001F9850(0x96)) {
                func_001F9C30(ip, dv, s);
                func_001F9BD8(ip, ip, delta);
                yaw = func_L00_001FF860(ip[0], ip[1]);
                pitch = -func_L00_001FF860(func_001F9CE8(ip), ip[2]);
            } else {
                yaw = func_L00_001FF860(delta[0], delta[1]);
                pitch = -func_L00_001FF860(func_001F9CE8(delta), delta[2]);
            }
        } else {
            if (d->timer < func_001F9850(0x96) - func_001F9850(0)) {
                yaw = d->yaw0;
                pitch = d->pitch0;
            } else {
                yaw = m->rot[2];
                pitch = m->rot[1];
            }
        }
        if (t != 0) {
            m->rot[2] = func_L00_00259148(&d->yawvel, m->rot[2], yaw,
                D_0015EE70 * 6.2831855f, D_0015EE70 * 3.1415927f, D_0015EE6C * 6.2831855f);
            m->rot[1] = func_L00_00259148(&d->pitchvel, m->rot[1], pitch,
                D_0015EE70 * 6.2831855f, D_0015EE70 * 3.1415927f, D_0015EE6C * 6.2831855f);
        } else {
            m->rot[2] = func_L00_00259148(&d->yawvel, m->rot[2], yaw,
                D_0015EE70 * 6.2831855f, D_0015EE70 * 3.1415927f, D_0015EE6C * 4.712389f);
            m->rot[1] = func_L00_00259148(&d->pitchvel, m->rot[1], pitch,
                D_0015EE70 * 6.2831855f, D_0015EE70 * 3.1415927f, D_0015EE6C * 4.712389f);
        }
        {
            Part_29E8A0 *p;
            func_00215C00(vel, d->speed, m->rot[2], -m->rot[1]);
            *(u128_29E8A0 *)dv = 0;
            dv[0] = func_002140F8(-1.0f, 1.0f);
            dv[1] = func_002140F8(-1.0f, 1.0f);
            dv[2] = func_002140F8(-1.0f, 1.0f);
            *(u128_29E8A0 *)delta = *(u128_29E8A0 *)dv;
            func_L00_001FF4B0(delta, delta, func_002140F8(0.1f, 0.2f) * D_0015EE6C);
            func_L00_001FF4B0(v90, vel, -func_002140F8(D_0015EE6C * 0.1f, D_0015EE6C));
            func_001F9BD8(delta, delta, v90);
            func_L00_001FF4B0(v90, vel, func_002140F8(0.0f, 1.0f) * d->speed);
            func_001F9BD8(v90, v90, m->pos);
            p = (Part_29E8A0 *)func_L00_00272158(v90, delta, func_001F9850(0x3C), 0x7F, 0x606060, 3,
                40000.0f, 1000.0f, 1.0f, -0.0002f, 0.0f);
            if (p != 0) {
                p->f2 = *D_L15_001B29DC;
                p->f3 = 0x44;
            }
            func_L00_00272158(m->pos, delta, func_001F9850(6), 0x7F, 0xB0B0B0, 3, 40000.0f, 1000.0f, 1.0f, -0.0004f, 0.0f);
            *(u128_29E8A0 *)dv = 0;
            dv[2] = 0.02f;
            func_001F9EC0(dv, dv, m->mtx);
            func_002156E0(dv, dv, m->mtx, func_00214158());
            func_L00_0026DA50(v90, dv, 0x4F007FFF, 0x1FFFFFFF, func_001F9850(5), 1, 20000.0f);
            qcopy(v60, m->pos);
            func_L00_001FF240(delta, vel, up);
        }
    }
    func_001F9BD8(m->pos, m->pos, vel);
    if (m->pos[0] < 0.0f || m->pos[1] < 0.0f || m->pos[2] < 0.0f) {
        func_L00_00260878(m, D_L15_001B1070);
        func_0020D678(m);
        return;
    }
    {
        Coll_29E8A0 coll;
        coll.flags = 0x830000;
        coll.radius = 3.0f;
        coll.moby = (char *)m;
        coll.count = 1;
        qcopy(coll.v, vel);
        func_L00_001FF500(coll.v, coll.v, 1.0f);
        hero = D_0013E633 + 0xE1D;
        coll.v[3] = 5627.9248f;
        coll.kind = 3;
        coll.id = m->id;
        coll.v[2] = 1.0f;
        coll.one = 1;
        if (func_L00_001EFFF0(v60, m->pos, 0, *(char **)(hero + 0x2080), &coll) != 0) {
            char *hit = D_L15_001744C0;
            char *hm = *(char **)(hit + 0x18);
            if (hm != 0) {
                if (hm != *(char **)(hero + 0x2080) && hm != d->owner) {
                    m->bc = 2;
                    qcopy(m->pos, hit + 0x20);
                    func_001F9BF0(vB0, hit + 0x50, hit + 0x60);
                    owner = (char *)m;
                    func_L00_001FF4B0(vB0, vB0, 1.0f);
                    func_L00_001FF610(v70, vel, hit + 0x40);
                    func_L00_001FF4B0(v70, v70, D_0015EE6C + D_0015EE6C);
                    func_L00_001FF4B0(v80, hit + 0x40, 1.0f);
                }
            } else if (*(int *)(hit + 0x1C) > 0) {
                m->bc = 1;
                qcopy(m->pos, hit + 0x20);
                func_001F9BF0(vB0, hit + 0x50, hit + 0x60);
                func_L00_001FF4B0(vB0, vB0, 1.0f);
                func_L00_001FF610(v70, vel, hit + 0x40);
                func_L00_001FF4B0(v70, v70, D_0015EE6C + D_0015EE6C);
                func_L00_001FF4B0(v80, hit + 0x40, 1.0f);
            }
        } else if (func_001F9908(&d->timer) != 0 || func_001F9D48(m->pos, (float *)(hero + 0x80)) > 260.0f) {
            func_L00_00260878(m, D_L15_001B1070);
            func_0020D678(m);
            return;
        }
        if (m->bc != 0) {
            float v100[4];
            float *nv;
            float dist;
            char *cam;
            int i;
            int r = func_L00_001F2BE8_29FF90(m->pos, 2.0f, 0x10, m, 0);
            *(u128_29E8A0 *)v100 = *(u128_29E8A0 *)m->pos;
            nv = v100;
            func_L00_0025BA50(m, nv, D_L15_00178580, r, (int)owner, 0x810000, 3, 1, 3.0f, 1.0f, 1.0f);
            {
            float K[16];
            float K2[16];
            float M3[16];
            float M2[16];
            float M1[16];
            K[0] = 0.0f;
            K[1] = v80[2];
            K[2] = -v80[1];
            K[3] = 0.0f;
            K[4] = -v80[2];
            K[5] = 0.0f;
            K[6] = v80[0];
            K[7] = 0.0f;
            K[8] = v80[1];
            K[9] = -v80[0];
            K[10] = 0.0f;
            K[11] = 0.0f;
            K[12] = 0.0f;
            K[13] = 0.0f;
            K[14] = 0.0f;
            K[15] = 0.0f;
            func_001FA540(K2, K, K);
            for (k = 0; k < 10; k++) {
                if (m->bc == 1) {
                    float ang = func_00214158();
                    func_L00_001FFCF8(M1, K2, 1.0f - func_001F9F90(ang));
                    func_L00_001FFCF8(M2, K, func_001F9FA8(ang));
                    func_L00_001FFDD0(M2, M2, M1);
                    func_001FA190(M1);
                    func_L00_001FFDD0(M3, M2, M1);
                    func_001F9EE8(nv, vB0, M3);
                    func_L00_001FF4B0(nv, nv, func_002140F8(8.5f, 16.5f) * D_0015EE6C);
                } else {
                    float sp = func_002140F8(8.5f, 16.5f) * D_0015EE6C;
                    float a1 = func_00214158();
                    func_00215C00(v100, sp, a1, func_00214158());
                }
                v100[2] = v100[2] + D_0015EE6C * 5.0f;
                func_001F9BD8(nv, nv, v70);
                func_L00_0026CA10_29FF90(m->pos, nv, 0x4F007FFF, 0x1F00007F, 40000.0f,
                    func_L00_00258BC8(func_001F9850(0x3C), func_001F9850(0x78)), 1, -1, -1);
            }
            {
                float r4[4];
                {
                    *(u128_29E8A0 *)r4 = 0;
                    r4[0] = func_002140F8(-1.0f, 1.0f);
                    r4[1] = func_002140F8(-1.0f, 1.0f);
                    r4[2] = func_002140F8(-1.0f, 1.0f);
                    *(u128_29E8A0 *)v100 = *(u128_29E8A0 *)r4;
                }
            }
            func_001F9BF0(v50, D_L15_00167440, m->pos);
            dist = func_001F9CB8(v50);
            v50[2] = v50[2] + dist * 0.5f;
            func_L00_001FF4B0(v100, v100, dist / 5.0f * D_0015EE6C);
            func_L00_001FF4B0(v50, v50, (dist + dist) * D_0015EE6C);
            func_001F9BD8(v100, v100, v50);
            func_L00_001FF548(v100, v100, D_0015EE6C * 10.0f);
            func_L00_002B0738((char *)m->pos, (char *)v100, func_L00_00258BC8(func_001F9850(0x3C), func_001F9850(0x5A)), 0, 0);
            {
                int n;
                float under;
                if (dist < 6.0f) {
                    n = func_001FA898_r(dist) / 2;
                } else {
                    n = 3;
                }
                under = 0.0f;
                if (dist < 7.0f) under = 7.0f - dist;
                i = 0;
                while (i < n) {
                    float sz = func_002140F8(8.0f, 10.0f) * D_0015EE6C - under * D_0015EE6C;
                    Cols_29E8A0 c1 = D_L15_001E2960;
                    Cols_29E8A0 c2 = D_L15_001E2978;
                    int *p1 = &c1.c[func_002140B0(6)];
                    int *p2 = &c2.c[func_002140B0(6)];
                    int l1 = func_L00_00258BC8(func_001F9850(0xF), func_001F9850(0x14));
                    int l2 = func_L00_00258BC8(func_001F9850(0x19), func_001F9850(0x1E));
                    int l3, l4;
                    func_L00_0026B890(m->pos, v70, *p1, *p2, 400000.0f, l1, l2, sz, 0, 0);
                    l3 = func_L00_00258BC8(func_001F9850(5), func_001F9850(0xA));
                    l4 = func_L00_00258BC8(func_001F9850(0xF), func_001F9850(0x14));
                    func_L00_0026B890(m->pos, v70, 0x7FFFFFFF, 0xFFFFFF, 400000.0f, l3, l4, sz * 0.5f, 0, 0);
                    i++;
                }
            }
            for (i = 0; i < 10; i++) {
                Cols_29E8A0 c1 = D_L15_001E2960;
                Cols_29E8A0 c2 = D_L15_001E2978;
                float dir[4];
                {
                    float r5[4];
                    *(u128_29E8A0 *)r5 = 0;
                    r5[0] = func_002140F8(-1.0f, 1.0f);
                    r5[1] = func_002140F8(-1.0f, 1.0f);
                    r5[2] = func_002140F8(-1.0f, 1.0f);
                    *(u128_29E8A0 *)dir = *(u128_29E8A0 *)r5;
                }
                func_L00_001FF4B0(dir, dir, func_002140F8(0.0f, 3.0f) * D_0015EE6C);
                {
                    int *p1 = &c1.c[func_002140B0(6)];
                    int *p2 = &c2.c[func_002140B0(6)];
                    func_L00_0026B368(m->pos, dir, *p1, *p2, func_L00_00258BC8(func_001F9850(0x14), func_001F9850(0x23)), 200000.0f);
                }
            }
            if (D_L15_0015F6B4 < 0.95f && 9.0f < dist) {
                func_L00_002ADBB0((char *)m, (char *)m->pos, (int)v70, func_001F9850(0xF), 4.0f, 0x7F, 0x7F, 0x7F, 0x20);
                func_L00_002ADBB0((char *)m, (char *)m->pos, (int)v70, func_001F9850(0x18), 4.0f, 0x7F, 0x7F, 0x7F, 0x20);
            }
            func_L00_002ADBB0((char *)m, (char *)m->pos, (int)v70, func_001F9850(0x14), 4.0f, 0x7F, 0x7F, 0, 0x30);
            func_L00_002ADBB0((char *)m, (char *)m->pos, (int)v70, func_001F9850(0x13), 2.0f, 0xFF, 0xFF, 0xFF, 0x20);
            vel[0] = 0.0f;
            cam = D_L15_00167300_x;
            m->bc = 0;
            if (dist < 20.0f) {
                *(float *)(cam + 0x160) = 0.4f - dist * 0.0175f;
            } else {
                *(float *)(cam + 0x160) = 0.050000012f;
            }
            *(int *)(D_L15_00167300_x + 0x168) = func_001F9850(0x19);
            func_0022ED80(0, 0, (int)m);
            func_L00_002D4CE8(&tbl, m->pos, 0, 0);
            func_L00_00260878(m, D_L15_001B1070);
            func_0020D678(m);
            }
        }
    }
}
