/* NON_MATCHING func_L00_002C9DC8 -- src/overlays/shared/vendor_002C96D0.c
 * Best so far: BYTES 230/2288 (90.0% of the bytes match), checked 2026-10-19.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   - p41 (BEST): p37 + the settle test written (t16 = (s16)timer, func_001F9850(240) < t16) with a separate s32 t
 *   sll/sra come out in place in s0 like retail. Everything through the end of case 2 and the settle code matches
 *   except the two lines below.
 *   - Left: (1) collision_point: retail keeps the 3F70/3F60 base pointer in s0 (a2 = s0 + 0x10/0x20); ours holds i
 *   (not call-crossing, so call-clobbered). Declaring it at function scope (p44) does not change it.
 *   (2) case 3 class-0xBA count loop: retail has p in v1, count in a0, a1 = 0xF0 and a2 = 0xBA hoisted, and a plai
 *   bnel / addiu in the delay slot; ours if-converts count++ into xori/movn (p42 continue-form, p43 && form, p45 m
 *   variable: no change).
 */
typedef struct {
    Vec4 vel;                 /* 0x00: velocity per step */
    Vec4 landing;             /* 0x10 */
    Vec4 normal;              /* 0x20 */
    struct Moby *holder;      /* 0x30: moby it was thrown from */
    s32 puff_timer;           /* 0x34: frames to the next smoke puff */
    s16 timer;                /* 0x38: rest / spawn timer */
    s16 unk3A;                /* 0x3A: nonzero: no path trace */
    s32 spawned;              /* 0x3C: mobys spawned so far */
} BombVars_2C9DC8;
extern f32 D_0015EE60_2C9DC8 __asm__("D_0015EE60") MACRO_ADDR;
extern u8 D_0013E633_2C9DC8[] __asm__("D_0013E633");
extern u8 D_L00_00173F70_2C9DC8[] __asm__("D_L00_00173F70");
extern u8 D_L00_00173F60_2C9DC8[] __asm__("D_L00_00173F60");
extern struct Moby *D_L00_00173F58_2C9DC8 __asm__("D_L00_00173F58");
extern struct Moby *D_L00_001600A4_2C9DC8 __asm__("D_L00_001600A4") MACRO_ADDR;
extern f32 func_001F9CB8_2C9DC8(void *) __asm__("func_001F9CB8");
extern void *func_L00_0026FBC8_2C9DC8(void *) __asm__("func_L00_0026FBC8");
extern f32 func_001FA888_2C9DC8(int) __asm__("func_001FA888");
extern void func_L00_001FF240_2C9DC8(void *, void *, void *) __asm__("func_L00_001FF240");
extern int func_L00_001F10E0_2C9DC8(f32, void *, int, void *) __asm__("func_L00_001F10E0");
extern void func_L00_001FF610_2C9DC8(void *, void *, void *) __asm__("func_L00_001FF610");
extern void func_L00_002C93D8_2C9DC8(void *) __asm__("func_L00_002C93D8");
extern f32 func_00214358_2C9DC8(void *, int, f32) __asm__("func_00214358");
extern void func_L00_00261478_2C9DC8(void *, void *, void *, void *, void *, void *) __asm__("func_L00_00261478");
extern int func_001F9938_2C9DC8(short *) __asm__("func_001F9938");
extern void func_L00_002C1110_2C9DC8(void *, void *, void *) __asm__("func_L00_002C1110");
extern void func_0020D678_2C9DC8(void *) __asm__("func_0020D678");

/* Update of the bomb moby (class 230): kills it outside the level box, hides it in vehicle mode, traces its throw path
   while held, then by state: 0 arm, 1 held (grow, puff smoke), 2 flying (gravity, collide with the world, bounce and
   settle to state 3), 3 resting (spawn up to four class 0xBA mobys), 4 fading out, else reset. */
void func_L00_002C9DC8(struct Moby *m) {
    BombVars_2C9DC8 *v;
    Vec4 d, e, a, q;
    struct Moby *p;
    s32 count;
    s32 timer;
    s32 t16;

    if (!m) return;
    v = (BombVars_2C9DC8 *)m->pvars;
    if (!v) return;
    if (m->pos.x < 2.0f || 1021.0f < m->pos.x ||
        m->pos.y < 2.0f || 1021.0f < m->pos.y ||
        m->pos.z < 2.0f || 1021.0f < m->pos.z) goto remove;
    if (D_0013F450_C9820.unk20A5 && m->state < 2)
        m->flags |= 0x41;
    else
        m->flags &= 0xFFBE;
    if (v->holder->oclass == 0xE5 && v->unk3A == 0 && func_001F9CB8_2C9DC8(v) > 0.0f) {
        if (!D_0013F450_C9820.ground_moby || !func_L00_0025D390_C9820((int)D_0013F450_C9820.ground_moby))
            ThrownObject_TracePathCopy((ThrowVars_2c8440_2C9820 *)v, m, 1);
        else if (!ThrownObject_TracePathCopy((ThrowVars_2c8440_2C9820 *)v, m, 0))
            ThrownObject_TracePathCopy((ThrowVars_2c8440_2C9820 *)v, m, 2);
    }
    switch (m->state) {
    case 0:
        m->state = 1;
        v->puff_timer = func_001F9850_2C9820(4);
        v->timer = 0;
        v->spawned = 0;
        break;
    case 1: {
        v->timer = 0;
        {
            f32 f = m->pclass->scale;
            if (m->scale < f * 0.25f)
                m->scale = m->scale + f * (D_0015EE60_2C9DC8 * 0.05f);
        }
        if (func_001F9908_2C9820(&v->puff_timer)) {
            v->puff_timer = func_001F9850_2C9820(4);
            if (!D_0013F450_C9820.unk20A5) func_L00_0026FBC8_2C9DC8(m);
        }
        if (!D_0013F450_C9820.vehicle_mode && v->holder && (v->holder->state & 0xF0) != 0xF0 &&
            D_0013F450_C9820.items[0].moby && D_0013F450_C9820.items[0].moby->oclass == 0xE5) return;
        goto remove;
    }
    case 2: {
        f32 f, g, h, t;
        f32 half = 0.5f;
        u8 *collision_point;
        u8 *carry;
        struct Hero *hero;
        f = func_001FA888_2C9DC8(D_0013E633_2C9DC8[1]) + 1.0f;
        {
            f32 g2 = m->pclass->scale;
            if (m->scale < g2 * half * f)
                m->scale = m->scale + g2 * (D_0015EE60_2C9DC8 * 0.05f) * f;
        }
        if (func_001F9908_2C9820(&v->puff_timer)) {
            v->puff_timer = func_001F9850_2C9820(4);
            func_L00_0026FBC8_2C9DC8(m);
        }
        v->vel.f[2] -= D_0015EE70_2C9820 * 9.0f;
        func_001F9BD8_2C9820(&d, &m->pos, v);
        carry = (u8 *)&D_0013F450_C9820.motion.platform_carry;
        func_L00_001FF240_2C9DC8(&q, &m->pos, carry);
        qcopy_nc(&e, &m->pos);
        t = D_0015EE60_2C9DC8 * 0.15f * m->scale;
        e.f[2] -= t / m->pclass->scale;
        qcopy_nc(&a, &d);
        t = t / m->pclass->scale;
        a.f[2] -= t;
        qcopy_nc(&q, &d);
        q.f[2] += f * 0.2f;
        hero = (struct Hero *)(carry - 0x140);
        if (func_L00_001F10E0_2C9DC8(f * 0.3f, &q, 4, hero->moby)) {
            collision_point = D_L00_00173F70_2C9DC8;
            qcopy_nc(&m->pos, collision_point);
            m->pos.z -= f * 0.2f;
            collision_point += 0x10;
            h = 0.01f;
            func_L00_001FF610_2C9DC8(v, v, collision_point);
            func_001F9C30_2C9820(v, v, half);
            g = func_001F9CB8_2C9DC8(v);
            if (g < D_0015EE60_2C9DC8 * h) goto settle;
        } else {
            if (!func_L00_001EFFF0_C9820(&e, &a, 0, hero->moby, 0)) {
                goto collision_miss;
            }
            collision_point = D_L00_00173F60_2C9DC8;
            qcopy_nc(&m->pos, collision_point);
            m->pos.z += (D_0015EE60_2C9DC8 * 0.15f * m->scale) / m->pclass->scale;
            collision_point += 0x20;
            h = 0.01f;
            func_L00_001FF610_2C9DC8(v, v, collision_point);
            func_001F9C30_2C9820(v, v, half);
            g = func_001F9CB8_2C9DC8(v);
            if (g < D_0015EE60_2C9DC8 * h) goto settle;
        }
        if (hero->ground_moby) {
            if (func_L00_0025D390_C9820((int)hero->ground_moby)) {
                g = func_001F9CB8_2C9DC8(v);
                if (g < D_0015EE60_2C9DC8 * h + func_001F9CB8_2C9DC8(carry - 0x40)) {
settle:
                    timer = (u16)v->timer + 1;
                    v->timer = timer;
                    if (!D_L00_00173F40_C9820.moby || D_L00_00173F40_C9820.unk1C > 0 ||
                        (t16 = (s16)timer, func_001F9850_2C9820(240) < t16)) {
                        m->state = 3;
                        v->timer = 0;
                    }
                }
            }
            if (D_0013F450_C9820.ground_moby && func_L00_0025D390_C9820((int)D_0013F450_C9820.ground_moby))
                func_001F9BD8_2C9820(v, v, &D_0013F450_C9820.motion.displacement);
        }
        func_001F9BD8_2C9820(&m->pos, &m->pos, v);
        func_L00_002C93D8_2C9DC8(m);
        break;
collision_miss:
        qcopy_nc(&m->pos, &d);
        return;
    }
    case 3: {
        f32 f;
        f = func_00214358_2C9DC8(&m->pos, 0, 0.5f);
        if (f != 0.0f && D_L00_00173F58_2C9DC8)
            func_L00_00261478_2C9DC8(m, D_L00_00173F58_2C9DC8, &m->pos, &m->rot, &m->pos, &m->rot);
        if (func_001F9938_2C9DC8(&v->timer)) {
            count = 0;
            for (p = D_L00_001600A4_2C9DC8; p; p = p->next) {
                if (p->oclass == 0xBA) {
                    if ((p->state & 0xF0) != 0xF0) count++;
                }
            }
            if (count >= 8) {
                v->timer = func_001F9850_2C9820(15);
            } else {
                e.f[0] = func_001F9F90_2C9820((f32)v->spawned * 6.2831855f * 0.25f);
                e.f[0] = D_0015EE60_2C9DC8 * 0.05f * e.f[0];
                e.f[1] = func_001F9FA8_2C9820((f32)v->spawned * 6.2831855f * 0.25f);
                e.f[2] = D_0015EE60_2C9DC8 * 0.02f;
                e.f[1] = D_0015EE60_2C9DC8 * 0.05f * e.f[1];
                if (D_0013F450_C9820.ground_moby && func_L00_0025D390_C9820((int)D_0013F450_C9820.ground_moby))
                    func_001F9BD8_2C9820(&e, &e, &D_0013F450_C9820.motion.displacement);
                func_001F9BD8_2C9820(&d, &e, &m->pos);
                func_L00_002C1110_2C9DC8(v->holder, &d, &e);
                v->timer = func_001F9850_2C9820(8);
                count = v->spawned + 1;
                v->spawned = count;
                if (count > 3) m->state = 4;
            }
        }
        if (v->spawned < 2 && func_001F9908_2C9820(&v->puff_timer)) {
            v->puff_timer = func_001F9850_2C9820(4);
            func_L00_0026FBC8_2C9DC8(m);
        }
        break;
    }
    case 4: {
        f32 f;
        m->alpha -= 4;
        m->scale *= 0.92f;
        f = func_00214358_2C9DC8(&m->pos, 0, 0.5f);
        if (f != 0.0f && D_L00_00173F58_2C9DC8)
            func_L00_00261478_2C9DC8(m, D_L00_00173F58_2C9DC8, &m->pos, &m->rot, &m->pos, &m->rot);
        if (m->alpha) return;
remove:
        func_0020D678_2C9DC8(m);
        break;
    }
    case 5:
    default:
        m->state = 0;
    }
}
