/* func_L23_00428490 (2152 bytes, 1 place: 23:00428490) -- NEAR MISS, not integrated.
   state: 2152 bytes against retail's 2152 (538 instructions), compiled in the -G8 unit (it reads
   about thirty words through $gp, D_L23_001B1A4C..D_L23_001B1AD4, and D_L23_001B1AE0 through
   `lui $v1,0x1B`): it belongs in L23_0041E418-g8.c. 519 of the 538 instructions align with
   retail; the first difference is at +0x194.
   What it is: a beam drawer (the object arrives in the 6th argument, $t1): six textured quads
   built with the boot vector helpers (func_00282CF0 sub, func_00282CC0 add, func_00282DD8 cross,
   func_00282F20 scale-to-length) and drawn by func_00280B90, the (u, v) pairs scrolled by
   vars->scroll and rotated by vars->spin, then two glows (func_00281EB8) at the object's position.
   It is func_L23_00429430 with fields of the object's variables where that one reads globals.
   What differs (one cause, measured):
   - the quad copy after the first draw: retail issues the four colour moves, then
     `lq $v0,0($s6); sq $v0,0($sp); lw $v1,0xF0($sp); lq $v0,0($s7); sq $v0,0($v1)`, and only then
     the argument moves of the next call; this cc1 schedules the argument moves into the free ALU
     slots between the stores (8 words in another order, and $a3/$v0 instead of $a0/$v0 for the
     two colour loads).
   - the object position after the second loop: retail keeps obj + 0x10 in $s0 to the end. Here
     the reaching register GCSE makes for the later `obj + 0x10` is set at the end of the first
     draw block, but sched1 hoists that copy to just after the first computation, so the two
     conflict and the second one is spilled: `sw $s0,0xFC($sp)`, six `lw $aN,0xFC($sp)` instead of
     `daddu $aN,$s0,$zero`, and the second loop's alignment nop one word later.
   The cause: with the two copies written as an inline-asm quad copy after the four colour moves,
   `__asm__ __volatile__ ("lq $2, 0(%1)\n\tsq $2, 0(%0)" : : "r"(dst), "r"(src) : "$2", "memory")`,
   corner[0] = corner[2] first, then corner[1] = corner[3], the 0x801010 stores as rgba[3] then
   rgba[2], and the corner addresses written as &q.corner[k] (no c1..c3), this C compiles to
   retail's bytes: tools/check.py reported "1 of 1 places match" (a scratch copy, reverted). The
   asm is a scheduling barrier: it keeps the copies and the argument moves in source order and
   keeps sched1 from hoisting the GCSE copy of obj + 0x10 above it. This is the qcopy macro of
   docs/COMPILER-NOTES.md ("Quadword copies through a held pointer reproduce retail exactly when
   written as inline asm"), which src/ does not allow. An empty `do {} while (0)` between the
   statements gives the same barrier (6 lines left on the twin) but is a steering construct; a
   `do { copy } while (0)` macro doubles the references inside (loop depth), keeps c1 out of the
   stack and spills other registers (112+ lines).
   What the source needs, each measured on this function or its twin:
   - each colour built by sequential `|=` in rgba[1]/rgba[3], then copied to rgba[0]/rgba[2]: one
     `(a << 24) | (b << 16) | (g << 8) | r` expression is reassociated by fold into
     ((a << 24) | r) | ((g << 8) | (b << 16)), and accumulating in rgba[0]/rgba[2] reverses the
     store pairs.
   - the variables through a struct (vars->spin): the next call's global argument load then moves
     above the vars->spin store (a fixed scalar against a varying struct field).
   - a second counter for the second loop: one `i` is one pseudo, $t1 in both loops; retail has
     $t1 then $a0.
   - the corner pointers c1..c3 and the vector pointers rel/axis/tip/side/base, set in that order
     at the end of the first block: the copies then go through held pointers, and the address
     computations come out in retail's order (&q.corner[k] gives direct stack offsets for plain C
     copies; plain arrays without the pointers put c2/c3 before the vectors).
   - obj + 0x10 written at each use: a `pos` variable is one long-lived pseudo below the five
     vector pointers in priority, $s5 instead of $s0 (153 of 538 instructions unaligned).
   - 0.017453294f (0x3C8EFA36) for pi/180; 180 orders of the six copy statements: the best leaves
     the eight words above.
*/

typedef unsigned char u8;

typedef unsigned short u16;

typedef short s16;

typedef int s32;

typedef long long s64;

typedef unsigned int u32;

typedef int TI128 __attribute__((mode(TI)));

typedef struct {
    TI128 corner[4];
    s32 rgba[4];
    float uv[4][2];
    s64 d70;
    s64 d78;
    s64 d80;
    s64 d88;
} L23Quad;

/* the beam's own variables (the object's +0x68 block) */
typedef struct {
    float dir[4];
    float pad10[3];
    float spin;
    float pad20[2];
    float scroll;
    s32 fade;
    float glow;
    float len;
    float back;
    float width;
} L23BeamVars;

extern float D_L23_001B1A4C __attribute__((sda));
extern float D_L23_001B1A50 __attribute__((sda));
extern float D_L23_001B1A54 __attribute__((sda));
extern s32 D_L23_001B1A58 __attribute__((sda));
extern u32 D_L23_001B1A5C __attribute__((sda));
extern s32 D_L23_001B1A60 __attribute__((sda));
extern s32 D_L23_001B1A64 __attribute__((sda));
extern s32 D_L23_001B1A68 __attribute__((sda));
extern float D_L23_001B1A6C __attribute__((sda));
extern s32 D_L23_001B1A70 __attribute__((sda));
extern s32 D_L23_001B1A74 __attribute__((sda));
extern s32 D_L23_001B1A80 __attribute__((sda));
extern s32 D_L23_001B1A84 __attribute__((sda));
extern s32 D_L23_001B1A88 __attribute__((sda));
extern s32 D_L23_001B1A8C __attribute__((sda));
extern s32 D_L23_001B1A90 __attribute__((sda));
extern s32 D_L23_001B1A94 __attribute__((sda));
extern s32 D_L23_001B1A98 __attribute__((sda));
extern s32 D_L23_001B1A9C __attribute__((sda));
extern s32 D_L23_001B1AA0 __attribute__((sda));
extern s32 D_L23_001B1AA4 __attribute__((sda));
extern s32 D_L23_001B1AA8 __attribute__((sda));
extern s32 D_L23_001B1AB0 __attribute__((sda));
extern s32 D_L23_001B1AB4 __attribute__((sda));
extern s32 D_L23_001B1AB8 __attribute__((sda));
extern s32 D_L23_001B1AC0 __attribute__((sda));
extern s32 D_L23_001B1AC4 __attribute__((sda));
extern s32 D_L23_001B1AC8 __attribute__((sda));
extern float D_L23_001B1AD0 __attribute__((sda));
extern float D_L23_001B1AD4 __attribute__((sda));
extern float D_L23_001B1AE0 __attribute__((nosda));
extern float D_L23_001BA640[];

extern void func_L00_00412088(L23Quad *);
extern void func_00281DE8(L23Quad *, s32, s32, s32);
extern void func_00280B90(L23Quad *, void *, s32);
extern void func_00282CC0(float *, float *, float *);
extern void func_00282CF0(float *, float *, float *);
extern void func_00282DD8(float *, float *, float *);
extern void func_00282F20(float *, float *, float);
extern float func_00283180(float);
extern float func_00283198(float);
extern float func_00283B98(float, float);
extern float func_002A7940(void);
extern void func_00281EB8(u8 *, s32, u32, s32, float, float, float);

void func_L23_00428490(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, u8 *obj) {
    L23Quad q;
    float vrel[4];
    float vaxis[4];
    float vtip[4];
    float vside[4];
    float vbase[4];
    float t[2];
    float *rel;
    float *axis;
    float *tip;
    float *side;
    float *base;
    float *c1;
    float *c2;
    float *c3;
    L23BeamVars *vars;
    float angle;
    float s;
    float c;
    int i;
    int j;

    vars = *(L23BeamVars **)(obj + 0x68);
    func_L00_00412088(&q);
    func_00281DE8(&q, D_L23_001B1A70, 1, 0x80);
    q.rgba[1] = D_L23_001B1A8C << 24;
    q.rgba[1] |= D_L23_001B1A88 << 16;
    q.rgba[1] |= D_L23_001B1A84 << 8;
    q.rgba[1] |= D_L23_001B1A80;
    q.rgba[3] = D_L23_001B1A9C << 24;
    q.rgba[3] |= D_L23_001B1A98 << 16;
    q.rgba[3] |= D_L23_001B1A94 << 8;
    q.rgba[3] |= D_L23_001B1A90;
    q.rgba[0] = q.rgba[1];
    q.rgba[2] = q.rgba[3];
    q.uv[0][0] = 0.0f;
    q.uv[0][1] = 0.0f;
    q.uv[1][0] = 0.0f;
    q.uv[1][1] = 1.0f;
    q.uv[2][0] = 1.0f;
    q.uv[2][1] = 0.0f;
    q.uv[3][0] = 1.0f;
    q.uv[3][1] = 1.0f;
    rel = vrel;
    axis = vaxis;
    tip = vtip;
    side = vside;
    base = vbase;
    c1 = (float *)&q.corner[1];
    c2 = (float *)&q.corner[2];
    c3 = (float *)&q.corner[3];
    for (i = 0; i < 4; i++) {
        q.uv[i][0] -= vars->scroll;
        if (q.uv[i][0] <= 7.0f) {
            q.uv[i][0] += 7.0f;
        }
    }
    vars->scroll += 0.1f;
    if (vars->scroll >= 4.0f) {
        vars->scroll -= 4.0f;
    }
    func_00282CF0(rel, (float *)(obj + 0x10), D_L23_001BA640);
    func_00282F20(axis, vars->dir, -vars->len);
    func_00282CC0(tip, (float *)(obj + 0x10), axis);
    func_00282DD8(side, rel, axis);
    func_00282F20(side, side, vars->width);
    func_00282F20(base, vars->dir, D_L23_001B1AE0);
    func_00282CC0(base, (float *)(obj + 0x10), base);
    func_00282CC0((float *)&q.corner[0], base, side);
    func_00282CF0(c1, base, side);
    func_00282CC0(c2, tip, side);
    func_00282CF0(c3, tip, side);
    func_00280B90(&q, 0, 0);

    q.rgba[1] = q.rgba[3];
    q.corner[0] = *(TI128 *)c2;
    q.rgba[0] = q.rgba[2];
    q.rgba[3] = 0x801010;
    q.rgba[2] = 0x801010;
    *(TI128 *)c1 = *(TI128 *)c3;
    func_00282CC0(c2, c2, axis);
    func_00282CC0(c3, c3, axis);
    func_00280B90(&q, 0, 0);

    func_00282F20(axis, vars->dir, -vars->back);
    q.uv[0][0] = 0.0f;
    q.uv[0][1] = 0.0f;
    q.uv[1][0] = 0.0f;
    q.uv[1][1] = 1.0f;
    q.uv[2][0] = 1.0f;
    q.uv[2][1] = 0.0f;
    q.uv[3][0] = 1.0f;
    q.uv[3][1] = 1.0f;
    angle = vars->spin;
    s = func_00283180(angle);
    c = func_00283198(angle);
    for (j = 0; j < 4; j++) {
        t[0] = q.uv[j][0] - 0.5f;
        t[1] = q.uv[j][1] - 0.5f;
        q.uv[j][0] = t[0] * s - t[1] * c;
        q.uv[j][1] = t[0] * c + t[1] * s;
        q.uv[j][0] += 0.5f;
        q.uv[j][1] += 0.5f;
    }
    vars->spin = func_00283B98(vars->spin, D_L23_001B1A6C * 0.017453294f);
    func_00281DE8(&q, D_L23_001B1A74, 1, 0x80);
    q.d70 = 5;

    q.rgba[1] = D_L23_001B1A60 << 24;
    q.rgba[1] |= D_L23_001B1AA8 << 16;
    q.rgba[1] |= D_L23_001B1AA4 << 8;
    q.rgba[1] |= D_L23_001B1AA0;
    q.rgba[3] = D_L23_001B1AB8 << 16;
    q.rgba[3] |= D_L23_001B1AB4 << 8;
    q.rgba[3] |= D_L23_001B1AB0;
    q.rgba[0] = q.rgba[1];
    q.rgba[2] = q.rgba[3];
    func_00282F20(side, side, D_L23_001B1A4C);
    func_00282F20(base, vars->dir, D_L23_001B1AE0);
    func_00282CC0(base, (float *)(obj + 0x10), base);
    func_00282CC0((float *)&q.corner[0], base, side);
    func_00282CF0(c1, base, side);
    func_00282CC0(tip, base, axis);
    func_00282CC0(tip, tip, axis);
    func_00282CC0(c2, tip, side);
    func_00282CF0(c3, tip, side);
    func_00280B90(&q, 0, 0);

    q.rgba[1] = D_L23_001B1A60 << 24;
    q.rgba[1] |= D_L23_001B1AA8 << 16;
    q.rgba[1] |= D_L23_001B1AA4 << 8;
    q.rgba[1] |= D_L23_001B1AA0;
    q.rgba[3] = D_L23_001B1AB8 << 16;
    q.rgba[3] |= D_L23_001B1AB4 << 8;
    q.rgba[3] |= D_L23_001B1AB0;
    q.rgba[0] = q.rgba[1];
    q.rgba[2] = q.rgba[3];
    func_00282F20(side, side, D_L23_001B1A50);
    func_00282F20(base, vars->dir, D_L23_001B1AD0);
    func_00282CC0(base, (float *)(obj + 0x10), base);
    func_00282CC0((float *)&q.corner[0], base, side);
    func_00282CF0(c1, base, side);
    func_00282CC0(tip, base, axis);
    func_00282CC0(tip, tip, axis);
    func_00282CC0(c2, tip, side);
    func_00282CF0(c3, tip, side);
    func_00280B90(&q, 0, 0);

    q.rgba[1] = D_L23_001B1A68 << 24;
    q.rgba[1] |= D_L23_001B1AA8 << 16;
    q.rgba[1] |= D_L23_001B1AA4 << 8;
    q.rgba[1] |= D_L23_001B1AA0;
    q.rgba[3] = D_L23_001B1AB8 << 16;
    q.rgba[3] |= D_L23_001B1AB4 << 8;
    q.rgba[3] |= D_L23_001B1AB0;
    q.rgba[0] = q.rgba[1];
    q.rgba[2] = q.rgba[3];
    func_00282F20(side, side, D_L23_001B1A54);
    func_00282F20(base, vars->dir, D_L23_001B1AD0);
    func_00282CC0(base, (float *)(obj + 0x10), base);
    func_00282CC0((float *)&q.corner[0], base, side);
    func_00282CF0(c1, base, side);
    func_00282CC0(tip, base, axis);
    func_00282CC0(tip, tip, axis);
    func_00282CC0(c2, tip, side);
    func_00282CF0(c3, tip, side);
    func_00280B90(&q, 0, 0);

    q.rgba[1] = D_L23_001B1A64 << 24;
    q.rgba[1] |= D_L23_001B1AC8 << 16;
    q.rgba[1] |= D_L23_001B1AC4 << 8;
    q.rgba[1] |= D_L23_001B1AC0;
    q.rgba[3] = D_L23_001B1AB8 << 16;
    q.rgba[3] |= D_L23_001B1AB4 << 8;
    q.rgba[3] |= D_L23_001B1AB0;
    q.rgba[0] = q.rgba[1];
    q.rgba[2] = q.rgba[3];
    func_00282F20(side, side, D_L23_001B1A4C);
    func_00282F20(base, vars->dir, D_L23_001B1AD4);
    func_00282CC0(base, (float *)(obj + 0x10), base);
    func_00282CC0((float *)&q.corner[0], base, side);
    func_00282CF0(c1, base, side);
    func_00282F20(axis, vars->dir, -0.6f);
    func_00282CC0(tip, base, axis);
    func_00282CC0(tip, tip, axis);
    func_00282CC0(c2, tip, side);
    func_00282CF0(c3, tip, side);
    func_00280B90(&q, 0, 0);

    func_00281EB8(obj + 0x10, D_L23_001B1A58, D_L23_001B1A5C, 1, vars->glow, 0.08f, func_002A7940());
    func_00281EB8(obj + 0x10, D_L23_001B1A58, 0x80808080, 1, vars->glow * 0.4f, 0.08f, func_002A7940());
}
