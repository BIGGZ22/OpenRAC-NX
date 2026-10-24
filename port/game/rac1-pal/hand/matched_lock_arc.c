/* SPDX-License-Identifier: GPL-3.0-or-later */
/* func_L02_002DB810 (DrawEllipticalArc), the Trespasser lock's arc (class 615, the same code on
 * levels 2, 4, 6, 8, 11, 13 and 18): an elliptical arc from angle a to b (radii rx, ry, around the
 * screen centre) as five sprite segments in colour col, each a packet of func_L02_0020BF88. As the
 * port runs it: the decompilation's matched C (src/overlays/shared/vendor_002A5218.c) with its
 * texture a 64-bit value and its cosine terms in u32. Not a PS2 match; the matched C stays as it
 * matches (hostgen.json "overrides": this C replaces it in the port only).
 *
 * Its caller, the puzzle's draw callback func_L02_002DA820, hands on GetEffectTex's TEX0 whole,
 * and retail keeps all 64 bits in the register the matching C declares int (daddu into
 * func_L02_0020BF88, which stores it with sd): the upper word is the CLUT base, its format and load
 * control. The port's int kept only the low word, so every quad of the arc lost it (0x8006C745
 * missing on Aridia). Here the texture is a long, as the packet routine's own definition takes it.
 * The callback's other helpers (func_L02_002DBAF8, 002DBD70, 002DC2C0) call GetEffectTex themselves
 * and pass its result on from a local or directly, which hostgen already keeps 64-bit (hostgen.json
 * "returns_64"): their matched C needs nothing.
 *
 * The cosine terms are -(c << 16) - 0x80000000 in 32 bits, zero-extended (retail: sll, negu, subu,
 * dsll32/dsrl32). The matched C writes them in int, where c << 16 of a negative c and the subtraction
 * of INT_MIN overflow: undefined, and a modern GCC takes the result for non-negative and leaves the
 * zero extension out, so a sum of 0x80000000 or more borrows from the upper word (0x00FFFFEF.. for
 * retail's 0x00FFFFF0..). Here they are u32 arithmetic, the same bits without the overflow.
 * Otherwise the matched C, line for line; it goes at the end of vendor_002A5218.c. */
extern short D_L02_00161B9C, D_L02_00161BA0, D_L02_00161BA4, D_L02_00161BA8;
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern void func_L02_0020BF88(void *, void *, void *, long, s32);

void func_L02_002DB810(long tex, u32 col, float a, float b, float rx, float ry) {
    long xy[4];
    u32 cols[4];
    int uv[4];
    float step = FastSubRots(b, a) / 5.0f;
    int i;
    uv[0] = *(int *)&D_L02_00161B9C;
    uv[1] = *(int *)&D_L02_00161BA0;
    uv[2] = *(int *)&D_L02_00161BA4;
    uv[3] = *(int *)&D_L02_00161BA8;
    cols[3] = col;
    cols[2] = col;
    cols[1] = col;
    cols[0] = col;
    for (i = 0; i < 5; i++) {
        float t0 = FastAddRots(a, step * (float)i);
        float t1 = FastAddRots(a, step * (float)(i + 1));
        xy[3] = 0xFFFFF000000000L;
        xy[2] = 0xFFFFF000000000L;
        xy[1] = 0xFFFFF000000000L;
        xy[0] = 0xFFFFF000000000L;
        xy[0] += (u32)(-((u32)func_001FA898_r(FastCos(t0) * rx * 16.0f) << 16) - 0x80000000u);
        xy[0] += func_001FA898_r(FastSin(t0) * rx * 16.0f) + 0x8000;
        xy[1] += (u32)(-((u32)func_001FA898_r(FastCos(t0) * ry * 16.0f) << 16) - 0x80000000u);
        xy[1] += func_001FA898_r(FastSin(t0) * ry * 16.0f) + 0x8000;
        xy[2] += (u32)(-((u32)func_001FA898_r(FastCos(t1) * rx * 16.0f) << 16) - 0x80000000u);
        xy[2] += func_001FA898_r(FastSin(t1) * rx * 16.0f) + 0x8000;
        xy[3] += (u32)(-((u32)func_001FA898_r(FastCos(t1) * ry * 16.0f) << 16) - 0x80000000u);
        xy[3] += func_001FA898_r(FastSin(t1) * ry * 16.0f) + 0x8000;
        func_L02_0020BF88(xy, uv, cols, tex, 1);
    }
}
