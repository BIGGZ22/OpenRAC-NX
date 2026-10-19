/* functional: high. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). The near miss nonmatching/l15_quartu/func_L15_002C6950.c (26 of 4,816 bytes off, equiv.py FUNCTIONAL), with one call written for the port (the goal passed by value as Goal_52b8, which is how the definition of func_L15_002C65D8 takes it; retail passes a pointer to the goal in the frame). Re-checked: case 2 heading block (path segment, speed ramp, turn-to-heading), the call sites of func_L15_002C65D8/62A8 and the exit tests against the assembly. Needs the declarations of its source file (vendor_0029C1D0.c) above it, as hostgen appends it there; it does not compile alone. */
/* NON_MATCHING func_L15_002C6950 -- src/overlays/l15_quartu/vendor_0029C1D0.c
 * Best so far: BYTES 26/4816 (99.5% of the bytes match), checked 2026-10-14.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   (free regalloc probes on the p12 shape: ternary/reordered-prototype forms fix pos/v90 priority but lose the cr
 *   Stopped: same differences after many changes. Best: p14.c, BYTES 26/4816.
 *   Left (26 bytes, all in case 2's heading block): retail calls func_L00_001FF860 from both branches of
 *   `seg+1 < n-8` and cross-jumps the call; with that source (p12) the allocator swaps pos (m+0x10) and &v90
 *   between $fp/$s7 (truncated priorities 940 vs 937) and the d+0xD0/d+0x110 spill slots. With the single-call
 *   form (p14) the two sub.s feeding $f12/$f13 use other temporaries, and one store/addiu pair is swapped.
 *   equiv.py p14.c: FUNCTIONAL. Note: sp+0x70/0x74 are goal.target/goal.mode inside the 0x50-byte goal struct
 *   filled by func_L00_00260D30/00260FB0 (not an uninitialised local).
 */
typedef struct HMoby_6950 HMoby_6950;
struct HMoby_6950 {
    char pad0[0x10];
    Vec4 pos;               /* 0x10 */
    unsigned char state;    /* 0x20 */
    char pad21[3];
    char *cls;              /* 0x24 */
    char pad28[4];
    float scale;            /* 0x2C */
    unsigned char b30;      /* 0x30 */
    unsigned char b31;      /* 0x31 */
    unsigned short h32;     /* 0x32 */
    unsigned short flags;   /* 0x34 */
    char pad36[0xA];
    Vec4 rot;               /* 0x40 */
    char pad50[0x28];
    void *data;             /* 0x78 */
    char pad7C[3];
    unsigned char b7F;      /* 0x7F */
    char pad80[0x14];
    int w94;                /* 0x94 */
    char pad98[0xE];
    short oclass;           /* 0xA6 */
    char padA8[0xA];
    unsigned short id;      /* 0xB2 */
    char padB4[8];
    unsigned char bBC;      /* 0xBC */
    char padBD[3];
    char mtx[0x40];         /* 0xC0 */
};
typedef struct {
    char pad0[0x20];
    float f20;              /* 0x20 */
    short h24;              /* 0x24 */
    char pad26[2];
    unsigned char b28;      /* 0x28 */
    char pad29[5];
    unsigned char b2E;      /* 0x2E */
    char pad2F;
    float f30;              /* 0x30 */
    char pad34[0x24];
    unsigned char b58;      /* 0x58 */
    unsigned char b59;      /* 0x59 */
    unsigned char b5A;      /* 0x5A */
    char pad5B[0x75];
    char anim0[0x40];       /* 0xD0 */
    char anim1[0x50];       /* 0x110 */
    HMoby_6950 *child;      /* 0x160 */
    HMoby_6950 *shot;       /* 0x164 */
    short h168;             /* 0x168 */
    short h16A;             /* 0x16A */
    float bob;              /* 0x16C */
    float speed;            /* 0x170 */
    float f174;             /* 0x174 */
    float f178;             /* 0x178 */
    float f17C;             /* 0x17C */
    float yaw;              /* 0x180 */
    float yawVel;           /* 0x184 */
    float pitch;            /* 0x188 */
    float pitchVel;         /* 0x18C */
    int w190;               /* 0x190 */
    int w194;               /* 0x194 */
    float f198;             /* 0x198 */
    float f19C;             /* 0x19C */
    float f1A0;             /* 0x1A0 */
    int w1A4;               /* 0x1A4 */
    float f1A8;             /* 0x1A8 */
    int w1AC;               /* 0x1AC */
    int w1B0;               /* 0x1B0 */
} HData_6950;
typedef struct {
    Vec4 pos;               /* 0x00 */
    char pad10[0x30];
    HMoby_6950 *target;     /* 0x40 */
    int mode;               /* 0x44 */
    char pad48[8];
} HGoal_6950;
typedef struct { int n; int pad[3]; Vec4 pt[1]; } HPath_6950;
typedef int u128_6950 __attribute__((mode(TI)));

extern char *func_L12_00309A50(char *a0);
extern void func_L00_00260D30_t(void *, float *, float) __asm__("func_L00_00260D30");
extern int func_L00_00260FB0(float, void *, void *, int, int, void *, int);
extern float func_001F9D10(void *, void *);
extern void func_0020D678(void *);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern void func_0020D960(char *, int, unsigned char *);
extern int func_L01_0026EFB8(int, int);
extern void func_L00_0025EFC0(void *, void *, void *, int *, float *, int, float, float, float);
extern float func_001FA888(int);
extern int func_L00_0025A778(void *, void *, int);
extern float func_L00_00259148(float *vel, float cur, float target, float k, float d, float max);
extern float func_001F9CB8(void *);
extern void func_L15_002C62A8_6950(void *, void *, void *) __asm__("func_L15_002C62A8");
extern void func_L00_00250800(void *, int, void *);
/* port: the goal goes by value as the file's Goal_52b8 (same 0x50-byte layout), which the definition of func_L15_002C65D8 takes */
extern int func_001F9938(void *);
extern int func_001F9850(int);
extern void func_L00_001FF500(void *, void *, float);
extern char *func_L03_002D65D0(char *a, char *pos, int c, int d, float f);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern float func_001FA790(float, float);
extern float func_00214158(void);
extern void func_L00_001FF240(void *, void *, void *);
extern void func_L12_00309AE8(void *, struct Moby *);
extern void func_L00_0025F4A8_2E9358(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int) __asm__("func_L00_0025F4A8");
extern void func_L00_002584A8(void *, int, int);
extern void func_0020D9D8(void *, void *);
extern void func_L00_0025B178(void *);
extern float func_001F9B88(float);
extern float func_001F9878(float);
extern void func_L00_002514B8(void *);
extern void func_L00_00251E30_u(void *) __asm__("func_L00_00251E30");
extern void func_001FA4A0(void *, void *);
extern void func_001F9EE8(void *, void *, void *);
extern float func_001F9CE8(void *);
extern void func_L00_001FFED8(void *, int, float);
extern unsigned char D_0013E633[];
extern unsigned char D_0014171B[] NOT_SDA;
extern HPath_6950 *D_L15_001B0DB0_h[] __asm__("D_L15_001B0DB0");
extern float D_0015EE70 MACRO_ADDR;
extern int D_0015EE84 MACRO_ADDR;
typedef struct { char pad0[0x454]; unsigned char collected[1]; } HLevel_6950;
extern HLevel_6950 D_L15_001BAEE0_h __asm__("D_L15_001BAEE0");
extern HLevel_6950 D_L15_001BBB40_h __asm__("D_L15_001BBB40");
extern int D_L15_001BADE0[];
extern short D_L15_00161A90;
extern float D_L15_00161A8C_f SDATA(D_L15_00161A8C);

/* Update function of the helicopter (moby class 233): waits, flies its path, hovers near and fires at its target, tilts and bobs, aims its turret, and dies. */
void func_L15_002C6950(HMoby_6950 *m) {
    Vec4 oldpos;
    HGoal_6950 goal;
    Vec4 v80;
    Vec4 v90;
    Vec4 vA0;
    Vec4 vB0;
    Vec4 vC0;
    Vec4 vD0;
    Vec4 vE0;
    int seg;
    float frac;
    HData_6950 *d;
    HMoby_6950 *tp;
    float dist;
    float len;
    float a;
    float r;

    oldpos.q = m->pos.q;
    d = m->data;
    if (d->child == 0) {
        d->child = (HMoby_6950 *)func_L12_00309A50((char *)m);
    }
    if (m->state != 0 && m->state != 0x40 && d->w1A4 != 0 && D_0013E633[0x2EC1] != 2) {
        m->w94 = 0;
        m->flags = (m->flags | 0x41) & 0xEFFF;
        d->b58 = 0;
        if (d->child != 0) d->child->flags |= 0x41;
        m->state = 1;
        return;
    }
    func_L15_002C7C20((char *)m);
    goal.mode = 0;
    if (m->state == 1) {
        d->f1A0 = d->f198;
        goto near;
    }
    if (d->w194 == -1) {
        d->f1A0 = d->f19C;
    near:
        func_L00_00260D30_t(m, goal.pos.f, d->f1A0);
    } else {
        HPath_6950 *path;
        d->f1A0 = d->f19C;
        path = D_L15_001B0DB0_h[d->w194];
        func_L00_00260FB0(d->f19C, m, &goal, 0, 0, path->pt, path->n);
    }
    if (d->f1A0 < func_001F9D10(&m->pos, &goal)) goal.mode = 2;
    if (goal.target == 0) goal.target = *(HMoby_6950 **)(D_0013E633 + 0x2E9D);
    tp = goal.target;
    if (m->state != 0 && d->b2E != 1) {
        if (d->child != 0) func_0020D678(d->child);
        func_0020D678(m);
        return;
    }
    switch (m->state) {
    case 0:
        m->pos.f[2] += 7.5f;
        m->b7F = 0x20;
        d->f1A8 = m->pos.f[2];
        d->f20 = 3.0f;
        d->h24 = func_001FA898_r(3.0f);
        d->b28 = 2;
        d->b58 = 0x19;
        d->f30 = 1.0f;
        d->b2E = 1;
        d->b59 = 1;
        d->b5A = 6;
        m->b30 = 0xFF;
        m->h32 = 0x80;
        if (d->w1AC != -1) {
            HPath_6950 *path;
            int i;
            path = D_L15_001B0DB0_h[d->w1AC];
            for (i = 0; i < path->n - 2; i++) {
                path->pt[i].f[3] = func_001F9D10(&path->pt[i], &path->pt[i + 1]);
            }
            m->pos.q = path->pt[0].q;
        }
        m->state = 1;
        d->speed = 0.0f;
        d->f174 = 0.0f;
        d->f178 = 0.0f;
        d->f17C = 0.0f;
        d->f19C = 60.0f;
        func_0020D960((char *)m, 0, d->anim0);
        func_0020D960((char *)m, 1, d->anim1);
        m->scale *= 0.5f;
        if (d->child != 0) {
            d->child->b30 = 0;
            d->child->flags &= 0xFFBE;
            d->child->scale *= 0.5f;
        }
        m->w94 = 0;
        m->flags = (m->flags | 0x41) & 0xEFFF;
        d->b58 = 0;
        if (d->child != 0) d->child->flags |= 0x41;
        break;
    case 1:
        if (d->w1A4 != 0 && D_0013E633[0x2EC1] != 2) {
            m->w94 = 0;
            m->flags = (m->flags | 0x41) & 0xEFFF;
            d->b58 = 0;
            if (d->child != 0) d->child->flags |= 0x41;
            return;
        }
        if (d->w190 != -1 || (func_00215570(D_0013E633 + 0xE9D, d->w1B0) && goal.mode != 2)) {
            if (d->w190 != -1 && func_L01_0026EFB8(d->w190, 0x40)) break;
            m->w94 = *(int *)(m->cls + 0x10);
            m->flags = (m->flags & 0xFFBE) | 0x1000;
            d->b58 = 0x19;
            if (d->w1AC != -1) m->state = 2; else m->state = 3;
        } else {
            if (d->w1B0 != -1) break;
            if (goal.mode == 2) break;
            m->w94 = *(int *)(m->cls + 0x10);
            m->flags = (m->flags & 0xFFBE) | 0x1000;
            d->b58 = 0x19;
            if (d->w1AC != -1) m->state = 2; else m->state = 3;
        }
        if (d->child != 0) d->child->flags &= 0xFFBE;
        break;
    case 2: {
        HPath_6950 *path;
        path = D_L15_001B0DB0_h[d->w1AC];
        func_L00_0025EFC0(path, &m->pos, &v90, &seg, &frac, 0, 999.0f, 5.0f, 0.0f);
        dist = func_001FA888(seg) * path->pt[0].f[3] + frac + 2.0f;
        if (func_001F9D10(&m->pos, &path->pt[path->n - 1]) <= d->speed * d->speed / (D_0015EE70 * 10.0f + D_0015EE70 * 10.0f)) {
            d->speed -= D_0015EE70 * 10.0f;
            if (d->speed < 0.0f) d->speed = 0.0f;
        } else {
            d->speed += D_0015EE70 * 10.0f;
            if (D_0015EE6C * 10.0f < d->speed) d->speed = D_0015EE6C * 10.0f;
        }
        seg = func_001FA898_r(dist / path->pt[0].f[3]);
        frac = (dist - func_001FA888(seg) * path->pt[0].f[3]) / path->pt[0].f[3];
        if (seg >= path->n - 1) {
            seg = path->n - 2;
            frac = 1.0f;
        }
        func_001F9BF0(&v90, &path->pt[seg + 1], &path->pt[seg]);
        func_001F9C30(&v90, &v90, frac);
        func_001F9BD8(&v90, &v90, &path->pt[seg]);
        if (func_001F9D48(&path->pt[path->n - 1], &m->pos) < 1.0f ||
            func_L00_0025A778(&m->pos, D_L15_001B0DB0_h[d->w194]->pt, D_L15_001B0DB0_h[d->w194]->n)) {
            if (goal.mode != 2) {
                m->state = 5;
                d->h168 = 0;
            } else {
                m->state = 4;
            }
        }
        {
            float dx;
            float dy;
            if (seg + 1 < path->n - 8) {
                dx = path->pt[seg + 1].f[0] - m->pos.f[0];
                dy = path->pt[seg + 1].f[1] - m->pos.f[1];
            } else {
                dx = goal.pos.f[0] - m->pos.f[0];
                dy = goal.pos.f[1] - m->pos.f[1];
            }
            a = func_L00_001FF860(dx, dy);
        }
        m->rot.f[2] = func_L00_00259148(&d->f17C, m->rot.f[2], a, 0.01f, 0.3f, 0.1f);
        func_001F9BF0(&v80, &v90, &m->pos);
        len = func_001F9CB8(&v80);
        if (d->speed < len) len = d->speed;
        func_L00_001FF4B0(&v80, &v80, len);
        func_001F9BD8(&m->pos, &m->pos, &v80);
        {
        float ang;
        float tx;
        float ty;
        ang = func_001FA790(func_L00_001FF860(v80.f[0], v80.f[1]), m->rot.f[2]);
        tx = d->speed * 0.34906584f * func_001F9F90(ang) / (D_0015EE6C * 10.0f);
        ty = d->speed * -0.34906584f * func_001F9FA8(ang) / (D_0015EE6C * 10.0f);
        m->rot.f[1] = func_L00_00259148(&d->f178, m->rot.f[1], tx, D_0015EE70 * 0.5235988f, D_0015EE70 * 1.0471976f, D_0015EE6C * 0.7853982f);
        m->rot.f[0] = func_L00_00259148(&d->f174, m->rot.f[0], ty, D_0015EE70 * 1.0471976f, D_0015EE70 * 2.0943952f, D_0015EE6C * 1.5707964f);
        }
        break;
    }
    case 3: {
        HPath_6950 *path;
        int i;
        float best = 9999.999f;
        path = D_L15_001B0DB0_h[d->w194];
        v80.q = tp->pos.q;
        for (i = 0; i < path->n; i++) {
            if (func_001F9D48(&path->pt[i], &m->pos) < best) v80.q = path->pt[i].q;
        }
        v80.f[2] = tp->pos.f[2] + 7.5f;
        v90.q = tp->pos.q;
        func_L15_002C62A8_6950(m, &v80, &v90);
        if (func_001F9D48(&v80, &m->pos) < 1.0f || func_L00_0025A778(&m->pos, path->pt, path->n)) {
            if (goal.mode != 2) {
                m->state = 5;
                d->h168 = 0;
            } else {
                m->state = 4;
            }
        }
        break;
    }
    case 4: {
        HPath_6950 *path;
        path = D_L15_001B0DB0_h[d->w194];
        v80.q = m->pos.q;
        v90.q = tp->pos.q;
        func_L15_002C62A8_6950(m, &v80, &v90);
        if (goal.mode == 2) break;
        if (func_L00_0025A778(&m->pos, path->pt, path->n)) {
            m->state = 5;
            d->h168 = 0;
        } else {
            m->state = 3;
        }
        break;
    }
    case 5:
        func_L00_00250800(m, 0, &v90);
        if (goal.mode != 2) {
            func_L15_002C65D8((struct Moby *)m, (float *)&v80, *(Goal_52b8 *)&goal);
            vA0.q = tp->pos.q;
            func_L15_002C62A8_6950(m, &v80, &vA0);
            if (func_001F9938(&d->h168) && d->shot == 0) {
                d->h168 = func_001F9850(0x3C);
                qcopy(&vB0, &goal);
                func_001F9BF0(&vA0, &vB0, &v90);
                func_L00_001FF500(&vA0, &vA0, D_0015EE6C * 6.0f);
                vA0.i[2] = 0;
                d->shot = (HMoby_6950 *)func_L03_002D65D0((char *)&vA0, (char *)&v90, (int)m, func_001F9850(0xF0), goal.pos.f[2]);
            }
        } else {
            vA0.q = m->pos.q;
            vB0.q = tp->pos.q;
            func_L15_002C62A8_6950(m, &vA0, &vB0);
            m->state = 4;
        }
        break;
    case 6:
        if (func_001F9938(&d->h16A)) {
            m->state = m->bBC;
            break;
        }
        d->speed -= D_0015EE70 * 4.0f;
        if (d->speed < 0.0f) d->speed = 0.0f;
        v80.f[0] = func_001F9F90(m->rot.f[2]) * -d->speed;
        v80.f[1] = func_001F9FA8(m->rot.f[2]) * -d->speed;
        v80.f[2] = 0.0f;
        func_001F9BD8(&m->pos, &m->pos, &v80);
        {
        float ang;
        float tx;
        float ty;
        ang = func_001FA790(func_L00_001FF860(v80.f[0], v80.f[1]), m->rot.f[2]);
        tx = d->speed * 0.34906584f * func_001F9F90(ang) / (D_0015EE6C * 10.0f);
        ty = d->speed * -0.34906584f * func_001F9FA8(ang) / (D_0015EE6C * 10.0f);
        m->rot.f[1] = func_L00_00259148(&d->f178, m->rot.f[1], tx, D_0015EE70 * 0.5235988f, D_0015EE70 * 1.0471976f, D_0015EE6C * 0.7853982f);
        m->rot.f[0] = func_L00_00259148(&d->f174, m->rot.f[0], ty, D_0015EE70 * 1.0471976f, D_0015EE70 * 2.0943952f, D_0015EE6C * 1.5707964f);
        }
        break;
    case 7: {
        int i;
        v80.q = 0;
        r = func_00214158();
        func_L00_00250800(m, 4, &v90);
        for (i = 0; i < 4; i++) {
            vA0.f[0] = func_001F9F90(r) * 1.5f;
            vA0.f[1] = func_001F9FA8(r) * 1.5f;
            vA0.i[2] = 0;
            r = func_001FA748(r, 1.5707964f);
            func_L00_001FF240(&vB0, &vA0, &v90);
        }
        func_L00_00250800(m, 5, &vA0);
        if (d->child != 0) {
            func_L12_00309AE8(m, (struct Moby *)d->child);
            d->child = 0;
        }
        func_L00_0025F4A8_2E9358(m, &v80, &m->pos, 0.0f, 0.0f, 0x14, 3, 4, 4.0f, 2.0f, 100000.0f, 3.0f, 2, 15.0f, 1, 1, -1, 0);
        func_L00_002584A8(m, 0x200, -1);
        func_0020D9D8(m, d->anim0);
        func_0020D9D8(m, d->anim1);
        m->state = 0x40;
        return;
    }
    case 0x40:
        m->w94 = 0;
        m->flags = (m->flags | 0x41) & 0xEFFF;
        d->b58 = 0;
        if (d->child != 0) d->child->flags |= 0x41;
        D_L15_001BAEE0_h.collected[(short)m->id] = 0;
        D_L15_001BBB40_h.collected[(short)m->id] = 0;
        *(int *)(D_0014171B + 0xAB75 + (((short)m->id >> 5) * 4 + (D_0015EE84 << 8))) &= ~(1 << (m->id & 0x1F));
        D_L15_001BADE0[(short)m->id >> 5] &= ~(1 << (m->id & 0x1F));
        if (D_0013E633[0x2EC1] != 2) {
            m->pos.f[2] = d->f1A8 - 7.5f;
            m->scale *= 2.0f;
            m->state = 0;
        }
        return;
    }
    if (m->b31 != 0 && func_001F9D10(&m->pos, D_L15_00167440) < 40.0f) {
        func_L00_0025B178(m);
    }
    if (m->state != 1) {
        float dz = func_001F9B88(m->pos.f[2] - oldpos.f[2]);
        float lim = D_0015EE6C * 10.0f;
        if (lim < dz) {
            if (oldpos.f[2] < m->pos.f[2]) m->pos.f[2] = oldpos.f[2] + lim;
            else m->pos.f[2] = oldpos.f[2] - lim;
        }
    }
    m->pos.f[2] -= *(float *)&D_L15_00161A90 * func_001F9F90(d->bob);
    d->bob = func_001FA748(d->bob, 6.2831855f / func_001F9878(D_L15_00161A8C_f * 60.0f));
    m->pos.f[2] += *(float *)&D_L15_00161A90 * func_001F9F90(d->bob);
    if (d->child != 0) {
        d->child->pos.q = m->pos.q;
        d->child->rot.q = m->rot.q;
        func_L00_002514B8(d->child);
        func_L00_00251E30_u(d->child);
        d->child->flags |= 6;
    }
    if (d->shot == 0 || d->shot->oclass != 0x350 || d->shot->state == 0xFE || d->shot->state == 0xFD) {
        func_L00_00250800(m, 0, &vC0);
        func_001F9BF0(&vE0, &goal.pos, &vC0);
        vD0.q = vE0.q;
        func_001FA4A0(&v80, m->mtx);
        func_001F9EE8(&vD0, &vD0, &v80);
        a = func_L00_001FF860(vD0.f[0], vD0.f[1]);
        d->yaw = func_L00_00259148(&d->yawVel, d->yaw, a, D_0015EE70 * 6.2831855f, D_0015EE70 * 12.566371f, D_0015EE6C * 6.2831855f);
        a = func_L00_001FF860(func_001F9CE8(&vD0), vD0.f[2]);
        d->pitch = func_L00_00259148(&d->pitchVel, d->pitch, -a, D_0015EE70 * 3.1415927f, D_0015EE70 * 6.2831855f, D_0015EE6C * 1.5707964f);
        func_L00_001FFED8(d->anim0 + 0x10, 2, d->yaw);
        func_L00_001FFED8(d->anim1 + 0x10, 2, d->pitch);
        d->shot = 0;
    } else {
        func_L00_00250800(m, 0, &vC0);
        func_001F9BF0(&vE0, &d->shot->pos, &vC0);
        vD0.q = vE0.q;
        func_001FA4A0(&v80, m->mtx);
        func_001F9EE8(&vD0, &vD0, &v80);
        a = func_L00_001FF860(vD0.f[0], vD0.f[1]);
        d->yaw = func_L00_00259148(&d->yawVel, d->yaw, a, D_0015EE70 * 6.2831855f, D_0015EE70 * 12.566371f, D_0015EE6C * 6.2831855f);
        a = func_L00_001FF860(func_001F9CE8(&vD0), vD0.f[2]);
        d->pitch = func_L00_00259148(&d->pitchVel, d->pitch, -a, D_0015EE70 * 3.1415927f, D_0015EE70 * 6.2831855f, D_0015EE6C * 1.5707964f);
        func_L00_001FFED8(d->anim0 + 0x10, 2, d->yaw);
        func_L00_001FFED8(d->anim1 + 0x10, 2, d->pitch);
    }
}
