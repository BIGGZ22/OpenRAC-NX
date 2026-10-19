/* NON_MATCHING func_L00_002B8AE8 -- src/overlays/shared/vendor_002B33E8.c
 * Best so far: BYTES 1051/2624 (60.0% of the bytes match), checked 2026-10-19.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   the loop's copy of &dir in $fp; ours v $s7, &dir $fp, copy $s6. The loop-copy pseudo (refs 5, live 50,
 *   priority 0.200) outranks v (0.1996) and &dir (0.198); to fix it the loop copy needs priority < 0.198 while
 *   v stays above &dir. No source change found that does that.
 *   - p13 (gate = 1 at the start of case 3, otherwise p11) happens to have the right size but is not nearer
 *   to retail than p11: it is the only same-size candidate, so it is listed in RESULT.md.
 *   Names: the definition is `func_L00_002B8AE8_r(Moby *) __asm__("func_L00_002B8AE8")` (my choice, the file does
 *   not declare the function; plain `func_L00_002B8AE8` would do). Callees and data use `_2B8AE8` aliases.
 *   Hero/Input types come from include/hero.h; Order/Data/Light/Hit types are private.
 */
/* Pyrocitor (moby class 176) update: state machine 0 idle, 1 wait for equip, 2 ready, 3 firing flame.
   Structure follows the NTSC decomp's sketch (unmatched there). */
typedef unsigned int Q_2B8AE8 __attribute__((mode(TI)));
typedef union { struct { float x, y, z, w; } f; Q_2B8AE8 q; } V_2B8AE8 __attribute__((aligned(16)));
typedef struct { char pad0[0x1A0]; int held; int pressed; } Input_2B8AE8;
typedef struct { char pad0[0x10]; unsigned char level; } Order_2B8AE8;
typedef struct Light_2B8AE8 { float x, y, z, b; V_2B8AE8 pos; } Light_2B8AE8;
typedef struct Hit_2B8AE8 {
    V_2B8AE8 point;
    Moby *moby;
    int flags;
    unsigned char type;
    unsigned char pad[3];
    float scale;
    int mode;
} Hit_2B8AE8;
typedef struct Counts_2B8AE8 { float response[2]; int gate, initial_count, count; } Counts_2B8AE8;
typedef struct Data_2B8AE8 {
    Moby *other;
    char pad4[0x40];
    short light;
    char pad46[4];
    short voice;
    short timer;
    short spark;
    short wait;
} Data_2B8AE8;

extern Hero hero_2B8AE8 __asm__("D_0013F450");
extern Input_2B8AE8 input_2B8AE8 __asm__("D_0013CA40");
extern Order_2B8AE8 order_2B8AE8 __asm__("D_0013E620");
extern unsigned char voices_2B8AE8[] __asm__("D_0013E650");
extern Light_2B8AE8 D_L00_001803C0_2B8AE8[] __asm__("D_L00_001803C0");
extern float D_L00_0015F660_2B8AE8[] __asm__("D_L00_0015F660") MACRO_ADDR;
extern int D_L00_0015F6B0_2B8AE8 __asm__("D_L00_0015F6B0") MACRO_ADDR;
extern float D_0015EE6C_2B8AE8 __asm__("D_0015EE6C") MACRO_ADDR;
extern float D_0015EE70_2B8AE8 __asm__("D_0015EE70") MACRO_ADDR;
extern int D_001413D4_2B8AE8 __asm__("D_001413D4");
extern char D_L00_001670F0_2B8AE8[] __asm__("D_L00_001670F0");
extern char D_L00_00173F60_2B8AE8[] __asm__("D_L00_00173F60");
extern float D_L00_001615E0_2B8AE8 SDATA(D_L00_001615E0);
extern float D_L00_001615E4_2B8AE8 SDATA(D_L00_001615E4);
extern int D_L00_001615E8_2B8AE8 SDATA(D_L00_001615E8);
extern int D_L00_001615EC_2B8AE8 SDATA(D_L00_001615EC);
extern float D_L00_001615F0_2B8AE8 SDATA(D_L00_001615F0);
extern float D_L00_001615F4_2B8AE8 SDATA(D_L00_001615F4);
extern float D_L00_001615F8_2B8AE8 SDATA(D_L00_001615F8);
extern int D_L00_00161600_2B8AE8 SDATA(D_L00_00161600);
extern float D_L00_001602A8_2B8AE8 __asm__("D_L00_001602A8") MACRO_ADDR;
extern float D_L00_001615C0_2B8AE8[4] SDATA(D_L00_001615C0);
extern float D_L00_001615D0_2B8AE8[4] SDATA(D_L00_001615D0);

extern void func_L00_002B9528_2B8AE8(unsigned char *) __asm__("func_L00_002B9528");
extern void func_L00_00250800_2B8AE8(void *, int, void *) __asm__("func_L00_00250800");
extern float func_0020D830_2B8AE8(void *) __asm__("func_0020D830");
extern int func_001F9850_2B8AE8(int) __asm__("func_001F9850");
extern void func_00213DE0_2B8AE8(void *, int, int, int) __asm__("func_00213DE0");
extern Moby *func_L00_002BC668_2B8AE8(void *) __asm__("func_L00_002BC668");
extern void func_001F9C08_2B8AE8(void *, void *, void *, float) __asm__("func_001F9C08");
extern float func_001F9CB8_2B8AE8(void *) __asm__("func_001F9CB8");
extern void func_L00_0023F1D0_2B8AE8(int) __asm__("func_L00_0023F1D0");
extern int func_L00_00234718_2B8AE8(int) __asm__("func_L00_00234718");
extern void func_L00_0020EB60_2B8AE8(void) __asm__("func_L00_0020EB60");
extern int func_L00_0023EF78_2B8AE8(void *, float, float, int) __asm__("func_L00_0023EF78");
extern int func_0022ED80_2B8AE8(int, int, void *) __asm__("func_0022ED80");
extern int func_L00_0028EB98_2B8AE8(void *, int) __asm__("func_L00_0028EB98");
extern void func_001F9EC0_2B8AE8(void *, void *, void *) __asm__("func_001F9EC0");
extern void func_001F9BD8_2B8AE8(void *, void *, void *) __asm__("func_001F9BD8");
extern float func_001F9FA8_2B8AE8(float) __asm__("func_001F9FA8");
extern float func_001FA888_2B8AE8(int) __asm__("func_001FA888");
extern void func_L00_001FF4B0_2B8AE8(void *, void *, float) __asm__("func_L00_001FF4B0");
extern int func_L00_001EFFF0_2B8AE8(void *, void *, int, int, int) __asm__("func_L00_001EFFF0");
extern float func_001F9D10_2B8AE8(void *, void *) __asm__("func_001F9D10");
extern float func_00214D88_2B8AE8(float, float, float, float, float *, float *) __asm__("func_00214D88");
extern void func_L00_00258DB0_2B8AE8(void *, float, float) __asm__("func_L00_00258DB0");
extern float func_002140F8_2B8AE8(float, float) __asm__("func_002140F8");
extern void func_001F9C30_2B8AE8(void *, void *, float) __asm__("func_001F9C30");
extern void *func_L00_0026C0D0_2B8AE8(void *, void *, unsigned long, float) __asm__("func_L00_0026C0D0");
extern void func_L00_002B9A90_2B8AE8(void *, void *) __asm__("func_L00_002B9A90");
extern void func_L00_001FF500_2B8AE8(void *, void *, float) __asm__("func_L00_001FF500");
extern void func_L00_001F2BE8_2B8AE8(void *, float, int, void *, void *) __asm__("func_L00_001F2BE8");
extern int func_001F9938_2B8AE8(void *) __asm__("func_001F9938");
extern float func_001F9878_2B8AE8(float) __asm__("func_001F9878");
extern int func_001FA898_2B8AE8(float) __asm__("func_001FA898");
extern void func_L00_0026E438_2B8AE8(void *, void *, int) __asm__("func_L00_0026E438");
extern int func_L00_00234638_2B8AE8(int, int) __asm__("func_L00_00234638");
extern void func_L00_0020ED30_2B8AE8(void) __asm__("func_L00_0020ED30");
extern void func_L00_0028EBF0_2B8AE8(int) __asm__("func_L00_0028EBF0");
extern void func_001FA1F8_2B8AE8(void *, void *) __asm__("func_001FA1F8");
extern void func_001FA4F0_2B8AE8(void *, void *, void *) __asm__("func_001FA4F0");
extern void func_L00_00251E30_2B8AE8(void) __asm__("func_L00_00251E30");
extern void func_L00_002B9730_2B8AE8(unsigned char *) __asm__("func_L00_002B9730");

void func_L00_002B8AE8_r(Moby *mo) __asm__("func_L00_002B8AE8");
void func_L00_002B8AE8_r(Moby *mo) {
    unsigned char *m = (unsigned char *)mo;
    Data_2B8AE8 *v = (Data_2B8AE8 *)mo->pvars;
    V_2B8AE8 pos, dir, scratch, endpoint, ppos, rvec;
    Hit_2B8AE8 hit;
    float speed, length, a, b;
    float resp[2];
    int gate, initial_count, count;
    int i;
    int extra;
    int r;
    int h;

    func_L00_002B9528_2B8AE8(m);
    func_L00_00250800_2B8AE8(m, 0, &pos);
    switch (m[0x20]) {
    case 0:
        v->light = -1;
        v->voice = -1;
        m[0x20] = 1;
        break;
    case 1:
        if (m[0x52] == 1) {
            m[0x20] = 2;
        }
        if (m[0x52] != 0)
            break;
        if (func_0020D830_2B8AE8(m) > 10.0f &&
            (input_2B8AE8.held & hero_2B8AE8.items[0].button_mask) != 0 && m[0x53] != 1) {
            func_00213DE0_2B8AE8(m, 1, 0, func_001F9850_2B8AE8(5));
        }
        break;
    case 2:
        if (v->other == 0) {
            v->other = func_L00_002BC668_2B8AE8(m);
            if (v->other != 0) {
                v->other->flags |= 0x100;
                *(float *)((unsigned char *)v->other->pvars + 0xC) = D_L00_001615E0_2B8AE8;
            }
        }
        if (v->light != -1) {
            func_001F9C08_2B8AE8(&D_L00_001803C0_2B8AE8[v->light], &D_L00_001803C0_2B8AE8[v->light],
                                 D_L00_0015F660_2B8AE8, 0.1f);
            if (func_001F9CB8_2B8AE8(&D_L00_001803C0_2B8AE8[v->light]) < 0.001f) {
                func_L00_0023F1D0_2B8AE8(v->light);
                v->light = -1;
            }
        }
        if ((input_2B8AE8.held & hero_2B8AE8.items[0].button_mask) && !hero_2B8AE8.unk20AC &&
            func_L00_00234718_2B8AE8(-1)) {
            m[0x20] = 3;
            func_L00_0020EB60_2B8AE8();
            v->timer = func_001F9850_2B8AE8(10);
            if (v->light == -1)
                v->light = func_L00_0023EF78_2B8AE8(m + 0x10, 7.5f, 0.0f, 0);
        } else if ((input_2B8AE8.pressed & hero_2B8AE8.items[0].button_mask) &&
                   !func_L00_00234718_2B8AE8(-1)) {
            func_0022ED80_2B8AE8(0, 0, m);
        }
        break;
    case 3:
        gate = 1;
        if (!func_L00_0028EB98_2B8AE8(m, v->voice))
            v->voice = func_0022ED80_2B8AE8(1, 4, m);
        if (v->light != -1 && m[0x20] == 3) {
            dir.q = 0;
            dir.f.x = 2.0f;
            dir.f.z = 2.0f;
            func_001F9EC0_2B8AE8(&dir, &dir, &hero_2B8AE8.basis);
            func_001F9BD8_2B8AE8(&D_L00_001803C0_2B8AE8[v->light].pos, &hero_2B8AE8.motion.pos, &dir);
            D_L00_001803C0_2B8AE8[v->light].pos.f.w = 7.5f;
            func_001F9C08_2B8AE8(&scratch, D_L00_001615C0_2B8AE8, D_L00_001615D0_2B8AE8,
                                 func_001F9FA8_2B8AE8(((float)(D_L00_0015F6B0_2B8AE8 % 120) / 120.0f) * 6.28318f - 3.14159f) * 0.5f + 0.5f);
            func_001F9C08_2B8AE8(&D_L00_001803C0_2B8AE8[v->light], &D_L00_001803C0_2B8AE8[v->light], &scratch, 0.1f);
        }
        resp[0] = resp[1] = 0.0f;
        speed = func_001FA888_2B8AE8(order_2B8AE8.level) * 4.0f;
        if (D_L00_0015F6B0_2B8AE8 & 3)
            gate = 0;
        if (D_001413D4_2B8AE8 == 1 && (mo->flags & 1)) {
            func_L00_001FF4B0_2B8AE8(&scratch, D_L00_001670F0_2B8AE8, -1.0f);
            func_001F9BD8_2B8AE8(&scratch, &scratch, (D_L00_001670F0_2B8AE8 - 0x230));
            func_L00_001FF4B0_2B8AE8(&dir, D_L00_001670F0_2B8AE8 - 0x20, D_L00_001615F0_2B8AE8 + speed + 0.75f);
            func_001F9BD8_2B8AE8(&endpoint, &dir, &scratch);
        } else {
            qcopy(&scratch, &pos);
            func_L00_001FF4B0_2B8AE8(&dir, &hero_2B8AE8.unk670, D_L00_001615F0_2B8AE8 + speed + 0.75f);
            func_001F9BD8_2B8AE8(&endpoint, &dir, &pos);
        }
        if (func_L00_001EFFF0_2B8AE8(&hero_2B8AE8.motion.unkD0, &scratch, 2, 0, 0)) {
            length = 0.0f;
        } else if (func_L00_001EFFF0_2B8AE8(&scratch, &endpoint, 2, 0, 0)) {
            length = func_001F9D10_2B8AE8(&scratch, D_L00_00173F60_2B8AE8) - 0.75f;
            if (length < 0.0f)
                length = 0.0f;
        } else {
            length = D_L00_001615F0_2B8AE8 + speed;
        }
        a = D_L00_001602A8_2B8AE8 * D_0015EE70_2B8AE8;
        b = D_L00_001615E4_2B8AE8 * D_0015EE6C_2B8AE8;
        func_00214D88_2B8AE8(length, 1.0f, a, b, &resp[0], &resp[1]);
        func_L00_001FF4B0_2B8AE8(&dir, &dir, resp[1]);
        initial_count = D_L00_001615E8_2B8AE8;
        extra = D_L00_001615EC_2B8AE8;
        if (length < 4.0f || (mo->flags & 1))
            initial_count = 1;
        count = extra + initial_count;
        for (i = 0; i < count; i++) {
            void *effect;
            int fl;
            func_L00_00258DB0_2B8AE8(&rvec, 0.0f,
                ((D_L00_001615F4_2B8AE8 - D_L00_001615F8_2B8AE8) *
                 (length / (D_L00_001615F0_2B8AE8 + speed)) +
                 D_L00_001615F8_2B8AE8) * D_0015EE6C_2B8AE8);
            func_001F9BD8_2B8AE8(&rvec, &rvec, &dir);
            func_001F9C30_2B8AE8(&ppos, &dir, func_002140F8_2B8AE8(0.0f, 1.0f));
            func_001F9BD8_2B8AE8(&ppos, &ppos, &scratch);
            fl = 1;
            if (i >= initial_count)
                fl = 0;
            fl |= 4;
            effect = func_L00_0026C0D0_2B8AE8(&ppos, &rvec, fl, length);
            if (i == 2 && gate)
                func_L00_002B9A90_2B8AE8(m, effect);
        }
        ppos.q = 0;
        ppos.f.x = -0.3f;
        func_001F9EC0_2B8AE8(&rvec, &ppos, &hero_2B8AE8.basis);
        func_001F9BD8_2B8AE8(&rvec, &rvec, &pos);
        hit.moby = mo;
        hit.flags = 0x10000;
        hit.scale = func_001FA888_2B8AE8(order_2B8AE8.level) + 1.0f;
        hit.point.q = dir.q;
        hit.mode = 1;
        func_L00_001FF500_2B8AE8(&hit.point, &hit.point, 1.0f);
        hit.point.f.z = 1.0f;
        hit.point.f.w = 5627.9248f;
        hit.mode = 1;
        hit.type = 5;
        func_L00_001F2BE8_2B8AE8(&rvec, 0.42f, 5, hero_2B8AE8.moby, &hit.point);
        ppos.f.x = -0.8f;
        func_001F9EC0_2B8AE8(&rvec, &ppos, &hero_2B8AE8.basis);
        func_001F9BD8_2B8AE8(&rvec, &rvec, &pos);
        func_L00_001F2BE8_2B8AE8(&rvec, 0.42f, 5, hero_2B8AE8.moby, &hit.point);
        if (func_001F9938_2B8AE8(&v->spark)) {
            v->spark = func_001FA898_2B8AE8(func_001F9878_2B8AE8(func_002140F8_2B8AE8(10.0f, 30.0f)));
            func_L00_00258DB0_2B8AE8(&rvec, 0.0f, D_0015EE6C_2B8AE8 + D_0015EE6C_2B8AE8);
            func_001F9C30_2B8AE8(&ppos, &dir, func_002140F8_2B8AE8(0.5f, 0.1f));
            func_001F9BD8_2B8AE8(&ppos, &ppos, &rvec);
            func_L00_0026E438_2B8AE8(&scratch, &ppos, order_2B8AE8.level);
        }
        if (D_L00_0015F6B0_2B8AE8 % func_001F9850_2B8AE8(D_L00_00161600_2B8AE8) == 0)
            func_L00_00234638_2B8AE8(-1, 1);
        r = func_L00_00234718_2B8AE8(-1);
        if (func_001F9938_2B8AE8(&v->timer)) {
            if ((input_2B8AE8.held & hero_2B8AE8.items[0].button_mask) && !hero_2B8AE8.unk20AC && r)
                break;
            m[0x20] = 2;
            func_L00_0020ED30_2B8AE8();
            h = v->voice;
            if (h != -1) {
                unsigned char *voice = voices_2B8AE8 + h * 0x70;
                if (*(unsigned char **)(voice + 0x88) == m && voice[0x74])
                    func_L00_0028EBF0_2B8AE8(h);
            }
            v->voice = -1;
            if (!r)
                func_0022ED80_2B8AE8(0, 0, m);
        }
        break;
    default:
        break;
    }
    if (v->other) {
        func_001FA1F8_2B8AE8((unsigned char *)v->other + 0xC0, (unsigned char *)v->other + 0x40);
        func_001FA4F0_2B8AE8((unsigned char *)v->other + 0xC0, m + 0xC0, (unsigned char *)v->other + 0xC0);
        qcopy((unsigned char *)v->other + 0x10, &pos);
        func_L00_00251E30_2B8AE8();
    }
    func_L00_002B9730_2B8AE8(m);
}
