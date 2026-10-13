/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native PAL hit spheres: func_L00_0025A8E8 (0x134 bytes) and func_L00_0025AA20
 * (0xA0 bytes, linked on levels 0, 6, 10, 16 and 18 only), which the decompilation
 * still has as assembly. Every attack that is a sphere goes through the first: the
 * wrench's head, the hero's ground slams, a creature's bite. Not a PS2 match.
 *
 * Adapted from ReRAC (https://github.com/re-rac/rerac),
 * crates/rc-game/src/moby_update/creature/attack.rs (sphere_hit, sphere_template,
 * joint_hit): ISC License, Copyright (c) 2026 ReRAC contributors. ReRAC describes the
 * US release (NTSC-U level 1's 0x26e830 and level 0's 0x2599e8); the calls here are
 * PAL's own, read from the matched C around them: the template's set-up
 * (func_L00_0025A8C0), the sphere against mobys (func_L00_001F2BE8) and the joint's
 * point (func_L00_00250800). */
extern void func_L00_0025A8C0(char *tmpl, int attacker, int flags, void *dir, float damage);
extern int func_L00_001F2BE8(float radius, void *centre, int flags, void *ignore, void *tmpl);
extern void func_L00_00250800(char *moby, int list, void *out);
extern float func_001F9F90(float); /* FastCos */
extern float func_001F9FA8(float); /* FastSin */

/* A sphere of `radius` at `centre` that hits every moby it touches but the attacker:
 * each gets a hit record from a template of `damage` and `flags`, pushed along the
 * attacker's facing (its first row when it keeps rows of its own, mode 0x100, else
 * its yaw) scaled by `push`, with z 1 and the push marker in w; +0x18 and +0x19 are
 * the hit's type bytes, +0x1A the attacker's class. Returns how many mobys it listed. */
int func_L00_0025A8E8(char *moby, float radius, void *centre, int flags, float damage, float push,
                      int type, int subtype, int sphere_flags) {
    char tmpl[0x30] __attribute__((aligned(16)));
    float dir[4] __attribute__((aligned(16)));

    if (*(unsigned short *)(moby + 0x34) & 0x100) {
        dir[0] = *(float *)(moby + 0xC0) * push;
        dir[1] = *(float *)(moby + 0xC4) * push;
    } else {
        dir[0] = func_001F9F90(*(float *)(moby + 0x48)) * push;
        dir[1] = func_001F9FA8(*(float *)(moby + 0x48)) * push;
    }
    dir[2] = 1.0f;
    *(unsigned int *)&dir[3] = 0x45AFDF66; /* 5627.925: the push is exact */
    func_L00_0025A8C0(tmpl, (int)moby, flags, dir, damage);
    tmpl[0x18] = type;
    tmpl[0x19] = subtype;
    *(unsigned short *)(tmpl + 0x1A) = *(unsigned short *)(moby + 0xA6);
    return func_L00_001F2BE8(radius, centre, sphere_flags, moby, tmpl);
}

/* The same sphere at the last joint of the attacker's joint list `list`, as it is
 * posed now; `n` is both the damage and the template's flags (one register serves
 * both in the retail code). The second float is not read. */
int func_L00_0025AA20(char *moby, float radius, float unused, float push, int list, int n, int type,
                      int subtype, int sphere_flags) {
    float point[4] __attribute__((aligned(16)));

    (void)unused;
    func_L00_00250800(moby, list, point);
    return func_L00_0025A8E8(moby, radius, point, n, (float)n, push, type, subtype, sphere_flags);
}
