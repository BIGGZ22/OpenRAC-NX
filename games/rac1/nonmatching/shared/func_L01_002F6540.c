/* NON_MATCHING func_L01_002F6540 -- src/overlays/shared/vendor_002B90A8.c
 * Best so far: SIZE ours 3980 / retail 3984, checked 2026-10-19.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   - reaction-timer switch: default and the not-expired exits `goto dispatch`, the `rec->react = ...` assignment 
 *   - `if (dir >= 0 ? idx == cnt - 1 : idx == 0) cur = 0;` (one shared store).
 *   - f20 split into lim / spd / tilt / yaw (each set once): local-alloc then gives $f20 to tilt/yaw and the const
 *   - vB0 = vC0 = X (inner store first) and i94 stored before flags: store order matches.
 *   Remaining differences in p25:
 *   1. case 13/17 (movz): retail emits li v1,2 / andi / li a0,1 / move a2 / movz / jal / move a1 (a1 in the delay 
 *   2. debris block (t != 0): retail sets $t2 = -1 before the compare chain (delay slot of the first beq) and leav
 *   3. debris pointer registers: retail pos=$s1, &w[1]=$s0, &w[0]=$s2 loaded through $a0 first; ours pos=$s0 (loca
 */
/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Same reads, writes, calls and arguments in the same order as retail (no COP2/VU0 code; the lq/sq pairs are plain 16-byte copies, the jump table is a switch); the two divisions by a record float are guarded so that a zero divisor gives FLT_MAX as on the PS2; differs otherwise only in compiler choices. */
/* Level 1 (Novalis) per-frame update of a moby that follows a path (a "rail" mover with a state record
 * rec = m[0x78]; its follower state is rec+0x60, "pf": pf[0] = current point, pf[4] direction, pf[5] mode,
 * pf[0x10] the point list (count then 16-byte points), pf[0x14] the path index into D_L01_001B0C30). Called from the
 * level's moby update and from the Kerwan/Eudora ones. m[0x20] is its state:
 *  - 0: pick the path point list, move along it (func_L01_0028C690/C848/C958 steps, speed rec+0xFC, wobble
 *    rec+0x124/0x128/0x12C), set the facing m[0x44]/m[0x48], position m[0x10], spawn the tracked child mobys
 *    (rec+0x140..) and go to 3 while the hurt timer rec+0x11C is positive, else to 1;
 *  - 1: the follow state proper: advance by rec+0xF0, smooth the facing and roll/pitch with func_L00_00259148, and,
 *    when the moby is not the "special" kind, tilt them to the motion; collect the children (rec+0x120 bit mask); on the
 *    level kinds 3 and 9 throw debris (func_L00_0025F4A8) and run the death countdown rec+0x13A;
 *  - 2: leave to 0, freeing the effect slot rec+0x11E; 3: wait for the timer rec+0x11C then back to 1.
 * Before that: the group (m[0x21]) is put in state 3 when none of its members can see (func_L01_00276680), the
 * classes 0x33/0x46A/0x473/0x474 get the effect slot rec+0x11E, and classes 0x33, 0x3C, 0x4A and 0x40, 0x44 run the two reaction timers
 * rec+0x138 (jump table jtbl_L01_0020B700). "special" = func_L01_00276680 returned -1 (only for class != 0x336). */
typedef int Q_2F6540 __attribute__((mode(TI)));
typedef union { Q_2F6540 q; float f[4]; } U_2F6540;

extern int D_L01_00161B90_2F6540 __asm__("D_L01_00161B90") MACRO_ADDR;
extern int D_0015EE84_2F6540 __asm__("D_0015EE84") MACRO_ADDR;
extern int D_L01_0015F6B0_2F6540 __asm__("D_L01_0015F6B0") MACRO_ADDR;
extern char D_L01_001672C0_2F6540[] __asm__("D_L01_001672C0");
extern char D_L01_0020B6D0_2F6540[] __asm__("D_L01_0020B6D0");
extern char *D_L01_001B0C30_2F6540[] __asm__("D_L01_001B0C30");

extern float D_0015EE60_2F6540 __asm__("D_0015EE60") MACRO_ADDR;
extern float D_0015EE6C_2F6540 __asm__("D_0015EE6C") MACRO_ADDR;
extern float D_0015EE70_2F6540 __asm__("D_0015EE70") MACRO_ADDR;
extern int D_L01_001607F0_2F6540 __asm__("D_L01_001607F0") MACRO_ADDR;

extern float func_001FA888_2F6540(int) __asm__("func_001FA888");
extern int func_L01_00276680_2F6540(char *, float) __asm__("func_L01_00276680");
extern int func_L00_0025A208_2F6540(int *, int, int, int) __asm__("func_L00_0025A208");
extern int func_L00_0025A2F0_2F6540(int *, int, int, int) __asm__("func_L00_0025A2F0");
extern int func_001F9850_2F6540(int) __asm__("func_001F9850");
extern int func_L00_0028EB98_2F6540(void *, int) __asm__("func_L00_0028EB98");
extern int func_0022ED80_2F6540(int, int, void *) __asm__("func_0022ED80");
extern int func_001F9938_2F6540(void *) __asm__("func_001F9938");
extern float func_001F9D10_2F6540(void *, void *) __asm__("func_001F9D10");
extern int func_002140B0_2F6540(int) __asm__("func_002140B0");
extern int func_L00_00258BC8_2F6540(int, int) __asm__("func_L00_00258BC8");
extern int func_001FA898_2F6540(float) __asm__("func_001FA898");
extern void func_001E9730_2F6540(char *, int, int) __asm__("func_001E9730");   /* STUB_printf: returns nothing */
extern float func_002140F8_2F6540(float, float) __asm__("func_002140F8");
extern float func_L00_001FF860_2F6540(float, float) __asm__("func_L00_001FF860");
extern void func_L01_0028C1D8_2F6540(char *) __asm__("func_L01_0028C1D8");
extern int func_L01_0028C2D8_2F6540(void *, void *, float) __asm__("func_L01_0028C2D8");
extern void func_L01_0028C690_2F6540(void *, int *, int, float, float) __asm__("func_L01_0028C690");
extern float func_001F9CE8_2F6540(void *) __asm__("func_001F9CE8");
extern void func_L01_002F6418_2F6540(char *, void *) __asm__("func_L01_002F6418");
extern float func_00214158_2F6540(void) __asm__("func_00214158");
extern void func_L01_0028C958_2F6540(void *, int, void *, float, float) __asm__("func_L01_0028C958");
extern void func_L00_00250800_2F6540(void *, int, void *) __asm__("func_L00_00250800");
extern void func_001F9BF0_2F6540(void *, void *, void *) __asm__("func_001F9BF0");
extern void func_L00_001FF4B0_2F6540(void *, void *, float) __asm__("func_L00_001FF4B0");
extern void func_L00_0028EBF0_2F6540(int) __asm__("func_L00_0028EBF0");
extern int func_L01_0028C640_2F6540(void *) __asm__("func_L01_0028C640");
extern int func_L01_0028C5F0_2F6540(void *, int) __asm__("func_L01_0028C5F0");
extern float func_001F9FA8_2F6540(float) __asm__("func_001F9FA8");
extern float func_001F9F90_2F6540(float) __asm__("func_001F9F90");
extern float func_001FA748_2F6540(float, float) __asm__("func_001FA748");
extern float func_001FA790_2F6540(float, float) __asm__("func_001FA790");
extern void func_L01_0028C848_2F6540(float, void *, void *, void *, void *, void *) __asm__("func_L01_0028C848");
extern void func_001F9BC0_2F6540(void *) __asm__("func_001F9BC0");
extern float func_L00_00259148_2F6540(float, float, float, float, float, void *) __asm__("func_L00_00259148");
extern float func_L00_0025F368_2F6540(float) __asm__("func_L00_0025F368");
extern void func_L01_00280428_2F6540(void *, void *) __asm__("func_L01_00280428");
extern char *func_L00_0025B478_2F6540(void *, int, int) __asm__("func_L00_0025B478");
extern void func_001F9C30_2F6540(void *, void *, float) __asm__("func_001F9C30");
extern void func_001F9BD8_2F6540(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_L00_0025F4A8_2F6540(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int) __asm__("func_L00_0025F4A8");


typedef union { Q_2F6540 q; float f[4]; } V_2F6540;
typedef struct {
    int cur;
    signed char dir;
    unsigned char mode;
    char pad6[2];
    int i8;
    float dist;
    V_2F6540 *list;
    int path;
    float ang;
    int i1c;
} PF_2F6540;
typedef struct {
    char pad0[0x60];
    PF_2F6540 pf;
    char pad80[0x10];
    V_2F6540 v90, vA0, vB0, vC0, vD0, vE0;
    float prog, spdA, spdB, spdC;
    float rollRate;
    int rollSt;
    float turnRate;
    int turnSt;
    float lenD, rollK, pitchK;
    short hurt, slot;
    unsigned char bits, tick;
    short tilt;
    float wob, wobRate, wobPh, r130, r134;
    short react, death, rtA, rtB;
    int kid[4];
} Rec_2F6540;
typedef struct {
    char pad0[0x70];
    int flags;
    char pad74[0xC];
    V_2F6540 v80;
} Kid_2F6540;
typedef struct {
    char pad0[0x10];
    int v10;
} Meta_2F6540;
typedef struct Moby_2F6540 {
    char pad0[0x10];
    V_2F6540 pos;
    unsigned char state, group, pad22, alpha;
    Meta_2F6540 *meta;
    char pad28[4];
    float scale;
    unsigned char b30;
    char pad31;
    short s32;
    unsigned short flags;
    char pad36[10];
    V_2F6540 rot;
    char pad50[0x28];
    Rec_2F6540 *rec;
    char pad7C[0x18];
    int i94;
    char pad98[0xC];
    unsigned char a4;
    char padA5;
    short cls;
    char padA8[0x58];
} Moby_2F6540;

extern Moby_2F6540 *D_L01_00160058_2F6540 __asm__("D_L01_00160058") MACRO_ADDR;
extern char D_0013E633_2F6540[] __asm__("D_0013E633");


void func_L01_002F6540(Moby_2F6540 *m) {
    int special;
    Rec_2F6540 *rec = m->rec;
    PF_2F6540 *pf;
    Moby_2F6540 *iter;
    int state;
    int cls;
    V_2F6540 dir;
    V_2F6540 zero;
    V_2F6540 old[2];
    V_2F6540 w[5];

    float lim;
    float spd;
    float tilt;
    float yaw;
    float f24;
    float s;

    zero.q = 0;
    special = 0;
    if (m->cls != 0x336) {
        special = (func_L01_00276680_2F6540((char *)m, func_001FA888_2F6540(m->s32)) == -1);
    }
    if (rec == 0) {
        return;
    }
    if (D_L01_00161B90_2F6540 != 0) {
        return;
    }
    pf = &rec->pf;

    /* the group of the moby: when none of its members is allowed, all go to state 3 */
    if (m->group != 0xFF && m->state != 0 && m->state != 3) {
        int ok = 0;
        func_L00_0025A208_2F6540((int *)&iter, m->group, 0, 0);
        if (iter == m) {
            Moby_2F6540 *o;
            goto grp_test;
            for (;;) {
                func_L00_0025A2F0_2F6540((int *)&iter, (int)iter, 0, 0);
            grp_test:
                o = iter;
                if (o == 0) {
                    break;
                }
                if (func_L01_00276680_2F6540((char *)o, func_001FA888_2F6540(o->s32)) != -1) {
                    ok = 1;
                    break;
                }
            }
            if (!ok) {
                func_L00_0025A208_2F6540((int *)&iter, m->group, 0, 0);
                if (iter != 0) {
                    do {
                        Moby_2F6540 *o2 = iter;
                        Rec_2F6540 *r2 = o2->rec;
                        o2->state = 3;
                        r2->hurt = func_001F9850_2F6540(5);
                        func_L00_0025A2F0_2F6540((int *)&iter, (int)iter, 0, 0);
                    } while (iter != 0);
                }
            }
        }
    }

    cls = m->cls;
    if (cls == 0x33 || cls == 0x46A || cls == 0x473 || cls == 0x474) {
        if ((D_L01_0015F6B0_2F6540 & 7) == (((int)m >> 8) & 7)
            && rec->death <= 0
            && m->state != 0
            && func_L00_0028EB98_2F6540(m, rec->slot) == 0) {
            rec->slot = func_0022ED80_2F6540(0, 4, m);
        }
    }

    switch ((short)(m->cls - 0x33)) {
    case 0:
    case 9:
    case 0x17:
        if (rec->react != -1) {
            if (func_001F9938_2F6540(&rec->react) == 0) {
                goto dispatch;
            }
            if (func_001F9D10_2F6540(&m->pos, D_L01_001672C0_2F6540) < 30.0f) {
                func_0022ED80_2F6540(1, 0, m);
            }
        }
        break;
    case 13:
    case 17:
        if (rec->react != -1) {
            if (func_001F9938_2F6540(&rec->react) == 0) {
                goto dispatch;
            }
            func_0022ED80_2F6540((func_002140B0_2F6540(100) & 1) ? 1 : 2, 0, m);
        }
        break;
    default:
        goto dispatch;
    }
    rec->react = func_001F9850_2F6540(func_L00_00258BC8_2F6540(rec->rtA, rec->rtA + rec->rtB));

dispatch:
    state = m->state;
    switch (state) {
    case 0: {
        int pid = pf->path;
        if (pid != -1) {
            int *p = (int *)D_L01_001B0C30_2F6540[pid];
            if (func_001F9D10_2F6540(p + 4, p + (*p << 2)) < 0.5f) {
                *p = *p - 1;
            }
        } else {
            func_001E9730_2F6540(D_L01_0020B6D0_2F6540, m->cls,
                                 func_001FA898_2F6540((float)(((int)m - (int)D_L01_00160058_2F6540) >> 8)));
        }
        m->b30 = 0xFF;
        rec->slot = -1;
        lim = 500.0f;
        if (lim < rec->r130) {
            rec->r130 = func_002140F8_2F6540(-1.0f, 1.0f) * rec->r130 / 1000.0f;
        }
        if (lim < rec->r134) {
            rec->r134 = func_002140F8_2F6540(-1.0f, 1.0f) * rec->r134 / 1000.0f;
        }
        if (pf->path == -1) {
            m->state = 2;
            return;
        }
        rec->tick = (((int)m - (int)D_L01_00160058_2F6540) >> 8) / 2 % 7;
        func_L01_0028C1D8_2F6540((char *)pf);
        if (pf->mode == 0) {
            if (!(pf->cur > 0)) {
                int idx = func_L01_0028C2D8_2F6540(&m->pos, pf->list, 0.0f);
                pf->cur = idx;
                if (pf->dir >= 0 ? idx == *(int *)pf->list - 1 : idx == 0) {
                    pf->cur = 0;
                }
            }
            pf->dist = func_001F9D10_2F6540(&m->pos, &pf->list[pf->cur + 1]);
            {
                V_2F6540 *p = &pf->list[pf->cur];
                pf->ang = func_L00_001FF860_2F6540(p[1].f[0] - m->pos.f[0], p[1].f[1] - m->pos.f[1]);
            }
            pf->mode = 2;
            pf->i8 = 0;
            pf->i1c = 0;
        }
        rec->vE0.q = pf->list[pf->cur + 1].q;
        rec->prog = 1.0f;
        func_L01_0028C690_2F6540(&rec->vA0, (int *)pf, 0, rec->spdA, rec->spdB);
        rec->vB0.q = rec->vC0.q = pf->list[pf->cur + 1].q;
        *(int *)&m->rot.f[0] = 0;
        s = func_001F9CE8_2F6540(&rec->vA0);
        m->rot.f[1] = -func_L00_001FF860_2F6540(s, rec->vA0.f[2]);
        m->rot.f[2] = func_L00_001FF860_2F6540(rec->vA0.f[0] - zero.f[0], rec->vA0.f[1] - zero.f[1]);
        rec->rollSt = 0;
        rec->turnSt = 0;
        m->pos.q = rec->vE0.q;
        func_L01_002F6418_2F6540((char *)m, &m->pos);
        rec->tick = 7;
        s = rec->wobPh;
        if (6.2831855f < s) {
            rec->wobPh = s * 0.017453292f;
        } else if (!(rec->wob == 0.0f) && s == -1.0f) {
            rec->wobPh = func_00214158_2F6540();
        }
        rec->vD0.q = rec->vC0.q;
        func_L01_0028C958_2F6540(pf, rec->bits & 0x20, m, rec->spdA, rec->spdB);
        if ((rec->bits & 0x1E) != 0 && (rec->bits & 1) == 0) {
            int i;
            for (i = 0; i < 4; i++) {
                if (rec->bits & (2 << i)) {
                    int id = rec->kid[i];
                    if (id != -1) {
                        Moby_2F6540 *o = D_L01_00160058_2F6540 + id;
                        if (o != 0) {
                            Kid_2F6540 *r = (Kid_2F6540 *)o->rec;
                            if (r != 0) {
                                r->flags = r->flags | 0x1000;
                                func_L00_00250800_2F6540(m, i, &w[i]);
                                func_001F9BF0_2F6540(&w[4], &w[i], &m->pos);
                                func_L00_001FF4B0_2F6540(&w[4], &w[4], 1.0f);
                                o->pos.q = w[i].q;
                                r->v80.q = w[4].q;
                            }
                        }
                    }
                }
            }
        }
        if (rec->hurt > 0) {
            m->state = 3;
            return;
        }
        if (D_0015EE84_2F6540 == 3 || D_0015EE84_2F6540 == 9) {
            rec->death = 0;
        }
        m->state = 1;
        return;
    }
    case 2:
        if (pf->path == -1) {
            return;
        }
        m->state = 0;
        {
            int fs = rec->slot;
            if (fs != -1) {
                char *fent = D_0013E633_2F6540 + 0x1D + fs * 0x70;
                if (*(int *)(fent + 0x88) == (int)m && *(unsigned char *)(fent + 0x74) != 0) {
                    func_L00_0028EBF0_2F6540(fs);
                }
            }
            rec->slot = -1;
        }
        return;
    case 3:
        if (func_001F9938_2F6540(&rec->hurt) != 0) {
            rec->hurt = 0;
            m->state = 1;
        }
        return;
    case 1:
        break;
    default:
        return;
    }

    /* state 1 */
    old[0].q = m->rot.q;
    old[1].q = m->pos.q;
    if (1.0f <= rec->prog) {
        int idx;
        rec->prog = rec->prog - 1.0f;
        idx = func_L01_0028C640_2F6540(pf);
        rec->v90.q = rec->vA0.q;
        func_L01_0028C690_2F6540(&rec->vA0, (int *)pf, 1, rec->spdA, rec->spdB);
        rec->vB0.q = rec->vC0.q;
        idx = func_L01_0028C5F0_2F6540(pf, idx + 1);
        rec->vC0.q = pf->list[idx + 1].q;
    }
    s = func_001F9FA8_2F6540(rec->wobPh);
    spd = rec->spdC + rec->wob * D_0015EE60_2F6540 * s * D_0015EE60_2F6540;
    f24 = spd / rec->vB0.f[3];
    rec->wobPh = func_001FA748_2F6540(rec->wobPh, rec->wobRate * D_0015EE60_2F6540 * 0.017453292f);
    rec->vE0.q = rec->vD0.q;
    func_L01_0028C848_2F6540(rec->prog + f24, &rec->vD0, &rec->vB0, &rec->vC0, &rec->v90, &rec->vA0);
    if (!(rec->bits & 0x40)) {
        float f5 = func_001F9D10_2F6540(&rec->vE0, &rec->vD0);
        if (f5 != 0.0f) {
            f24 = f24 * (spd / f5);
            func_L01_0028C848_2F6540(rec->prog + f24, &rec->vD0, &rec->vB0, &rec->vC0, &rec->v90, &rec->vA0);
        }
    }
    if (!special) {
        float f5;
        tilt = (float)rec->tilt * 0.017453292f;
        func_001FA790_2F6540(m->rot.f[2], tilt); /* result not used by retail */
        s = func_L00_001FF860_2F6540(rec->vD0.f[0] - rec->vE0.f[0], rec->vD0.f[1] - rec->vE0.f[1]);
        s = func_001FA748_2F6540(s, tilt);
        s = func_001FA790_2F6540(s, m->rot.f[2]);
        f5 = 1.0f / rec->lenD;
        yaw = s * (f5 * D_0015EE60_2F6540);
        m->rot.f[2] = func_001FA748_2F6540(yaw, m->rot.f[2]);
        func_001F9BF0_2F6540(&dir, &rec->vD0, &rec->vE0);
        s = func_001F9CE8_2F6540(&dir);
        s = func_L00_001FF860_2F6540(s, dir.f[2]);
        m->rot.f[1] = func_L00_00259148_2F6540(
            m->rot.f[1], -s,
            rec->turnRate * 10.0f * 0.017453292f * D_0015EE70_2F6540,
            rec->turnRate * 5.0f * 0.017453292f * D_0015EE70_2F6540,
            rec->turnRate * 0.017453292f * D_0015EE6C_2F6540, &rec->turnSt);
        s = func_L00_0025F368_2F6540(yaw * rec->rollK);
        s = func_001FA790_2F6540(m->rot.f[0], s);
        s = func_L00_0025F368_2F6540(s * rec->pitchK);
        m->rot.f[0] = func_L00_00259148_2F6540(
            m->rot.f[0], s,
            rec->rollRate * 10.0f * 0.017453292f * D_0015EE70_2F6540,
            rec->rollRate * 5.0f * 0.017453292f * D_0015EE70_2F6540,
            rec->rollRate * 0.017453292f * D_0015EE6C_2F6540, &rec->rollSt);
    } else {
        func_001F9BC0_2F6540(&m->rot);
        m->rot.f[2] = func_L00_001FF860_2F6540(rec->vD0.f[0] - rec->vE0.f[0], rec->vD0.f[1] - rec->vE0.f[1]);
    }
    rec->prog = rec->prog + f24;
    m->pos.q = rec->vE0.q;
    if (!special) {
        func_L01_002F6418_2F6540((char *)m, &m->pos);
    }

    if ((rec->bits & 0x1F) != 0) {
        int i;
        int j = 0;
        for (i = 0; i < 5; i++) {
            if ((rec->bits >> i) & 1) {
                func_L00_00250800_2F6540(m, j, &w[0]);
                j++;
                func_001F9BF0_2F6540(&w[1], &w[0], &m->pos);
                if (!(rec->bits & 1)) {
                    int id = rec->kid[i - 1];
                    if (id != -1) {
                        Moby_2F6540 *o = D_L01_00160058_2F6540 + id;
                        if (o != 0) {
                            Kid_2F6540 *r = (Kid_2F6540 *)o->rec;
                            if (r != 0) {
                                func_L00_001FF4B0_2F6540(&w[1], &w[1], 1.0f);
                                r->flags = r->flags | 0x1000;
                                o->pos.q = w[0].q;
                                r->v80.q = w[1].q;
                            }
                        }
                    }
                } else if (rec->tick == 0) {
                    func_L00_001FF4B0_2F6540(&w[1], &w[1], func_002140F8_2F6540(0.016f, 0.031f) * D_0015EE60_2F6540);
                    func_L01_00280428_2F6540(&w[0], &w[1]);
                    func_L00_001FF4B0_2F6540(&w[1], &w[1], func_002140F8_2F6540(0.016f, 0.031f) * D_0015EE60_2F6540);
                    func_L01_00280428_2F6540(&w[0], &w[1]);
                }
            }
        }
        if (rec->tick == 0) {
            rec->tick = func_001F9850_2F6540(7);
        }
        rec->tick = rec->tick - 1;
    }

    if (D_0015EE84_2F6540 != 3 && D_0015EE84_2F6540 != 9) {
        return;
    }
    {
        char *t = func_L00_0025B478_2F6540(m, 0x210000, 0);
        if (t == 0 || *(unsigned short *)(t + 0x28) != 0x102) {
            if (t != 0) {
                func_001F9BC0_2F6540(&w[0]);
                if (m->cls == 0x33) {
                    w[0].f[0] = func_001F9F90_2F6540(m->rot.f[2]) * -2.747f;
                    w[0].f[1] = func_001F9FA8_2F6540(m->rot.f[2]) * -2.747f;
                    w[0].f[2] = 0.0f;
                }
                func_001F9BF0_2F6540(&w[2], &m->pos, &old[1]);
                w[1].q = w[2].q;
                func_001F9C30_2F6540(&w[1], &w[1], 0.5f);
                func_001F9BD8_2F6540(&w[0], &w[0], &m->pos);
                rec->death = func_001F9850_2F6540(0x258);
                cls = m->cls;
                func_L00_0025F4A8_2F6540(m, &w[1], &w[0], 0.0f, 0.0f, 0x14, 3, 4, 4.0f, 2.0f, 100000.0f, 3.0f,
                                         (cls == 0x532 || cls == 0x53B || cls == 0x53C) ? 1 : -1, 15.0f,
                                         1, 1, -1, 0);
            }
        }
    }
    D_L01_001607F0_2F6540 = (int)func_L01_002F6540;
    m->a4 = 0xFF;
    if (rec->death == 0) {
        return;
    }
    if (func_001F9938_2F6540(&rec->death) != 0 && special) {
        m->i94 = m->meta->v10;
        m->flags = (m->flags & 0xFFBE) | 0x1000;
        return;
    }
    m->i94 = 0;
    m->flags = m->flags | 0x41;
    {
        int fs = rec->slot;
        if (fs != -1) {
            char *fent = D_0013E633_2F6540 + 0x1D + fs * 0x70;
            if (*(int *)(fent + 0x88) == (int)m && *(unsigned char *)(fent + 0x74) != 0) {
                func_L00_0028EBF0_2F6540(fs);
            }
        }
        rec->slot = -1;
    }
    m->flags = m->flags & 0xEFFF;
}
