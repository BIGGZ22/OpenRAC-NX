/* NON_MATCHING func_L09_002F1198 -- src/overlays/l09_gaspar/vendor_002C2B08.c
 * Best so far: BYTES 18/500 (96.4% of the bytes match), checked 2026-10-12.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   func_L09_002F1198: effect moby spawner (class 0x1A1): random angle and speed, direction from atan2 of the velo
 *   gen5/p07 = 55/500 (size ok). Left: pi/ang/dir float regs rotated (retail pi=f21 ang=f22 dir=f23), second pi-di
 *   gen8/f4 = 18/500 (size ok). Fix that got there: final scale as one expression
 *   `obj->scale * ((pi - dir) + (pi - dir) + 1.0f)` (no t var) -> pi/lo/hi regs match.
 *   Left: tail consts. Retail: li 255, li 255 (two regs: sb30, sh32), li a1,1 shared with
 *   sb31 and the call arg. Ours: one 255 reg for both, li 1 for sb31 + separate li a1.
 *   cse1 keeps two SI const-1 loads; tiny tt/z_*.c tests share a1 only when the block has
 *   no extra float store/ops (q2/t3), so the sharing is pass-order/alloc dependent.
 */
extern Moby *func_0020D348_a(int) __asm__("func_0020D348");
extern float func_002140F8_a(float, float) __asm__("func_002140F8");
extern float func_L00_001FF860_a(float, float) __asm__("func_L00_001FF860");
extern float func_001FA850_a(float, float) __asm__("func_001FA850");
extern float func_001F9F90_a(float) __asm__("func_001F9F90");
extern float func_001F9FA8_a(float) __asm__("func_001F9FA8");
extern float D_0015EE6C_a __asm__("D_0015EE6C") MACRO_ADDR;
extern void func_L00_001FF4B0_a(void *, void *, float) __asm__("func_L00_001FF4B0");
extern void func_001F9BD8_a(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_001F9C30_a(void *, void *, float) __asm__("func_001F9C30");
extern void func_L00_00251E30_a(void *, int) __asm__("func_L00_00251E30");

char *func_L09_002F1198(void *a0, void *a1, void *a2, float lo, float hi) {
    Vec4f *src = (Vec4f *)a1;
    Vec4f *vel = (Vec4f *)a2;
    Moby *obj;
    Vec4f *out;
    Vec4f *p;
    Vec4f tmp;
    float ang;
    float spd;
    float dir;
    float t;
    float q;
    float pi;

    obj = func_0020D348_a(0x1A1);
    if (obj != 0) {
        p = &obj->pos;
        out = (Vec4f *)obj->pvars;
        pi = 3.1415927f;
        ang = func_002140F8_a(-3.1415927f, pi);
        spd = func_002140F8_a(lo, hi);
        dir = func_001FA850_a(func_L00_001FF860_a(vel->x, vel->y), ang);
        obj->pos.x = func_001F9F90_a(ang) * spd;
        obj->pos.y = func_001F9FA8_a(ang) * spd;
        obj->pos.z = 0;
        func_L00_001FF4B0_a(out, p, dir * D_0015EE6C_a);
        q = pi - dir;
        q = q * q * 0.25f * D_0015EE6C_a;
        func_L00_001FF4B0_a(&tmp, vel, q);
        func_001F9BD8_a(out, out, &tmp);
        out->z = 0;
        func_001F9BD8_a(p, p, src);
        func_001F9BD8_a(p, p, vel);
        func_001F9C30_a(out, out, 0.35f);
        obj->unk58 = obj->unk58 * 0.5f;
        obj->rot.z = func_L00_001FF860_a(out->x, out->y);
        obj->unk32 = 0xFF;
        obj->unk30 = 0xFF;
        obj->unk31 = 1;
        obj->scale = obj->scale * ((pi - dir) + (pi - dir) + 1.0f);
        func_L00_00251E30_a(obj, 1);
    }
    return (char *)obj;
}
