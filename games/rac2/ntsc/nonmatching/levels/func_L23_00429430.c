/* func_L23_00429430 (2192 bytes, 1 place: 23:00429430) -- NEAR MISS, not integrated.
   state: 2192 bytes against retail's 2192 (548 instructions), compiled in the -G8 unit (it reads
   about forty words through $gp, D_L23_001B1AF0..D_L23_001B1B88, and D_L23_001B1B90 through
   `lui $v1,0x1B`): it belongs in L23_0041E418-g8.c. 540 of the 548 instructions align with
   retail; everything else is exact. The first difference is at +0x288.
   What it is: func_L23_00428490 (the beam drawer) reading its tuning from globals, plus a fade
   prologue that writes the colours' alpha words from vars->fade (n * 4, minus 1 when non-zero;
   n * 2; n; n; n).
   What differs: only the quad copy after the first draw. Retail issues the four colour moves,
   then `lq $v0,0($s6); sq $v0,0($sp); lw $v1,0xF0($sp); lq $v0,0($s7); sq $v0,0($v1)`, then the
   three argument moves of the next call (`daddu $a0,$s6; daddu $a1,$s6; jal; daddu $a2,$s4`);
   this cc1 schedules the argument moves into the free ALU slots between the stores, which also
   moves the copies and gives the first colour load $a3 instead of $a0 (8 words).
   The cause, measured: with the two copies written as an inline-asm quad copy after the four
   colour moves, `__asm__ __volatile__ ("lq $2, 0(%1)\n\tsq $2, 0(%0)" : : "r"(dst), "r"(src)
   : "$2", "memory")`, corner[0] = corner[2] first, the 0x801010 stores as rgba[3] then rgba[2],
   and the corner addresses written as &q.corner[k] (no c1..c3), this C compiles to retail's
   bytes: tools/check.py reported "1 of 1 places match" (a scratch copy, reverted). The asm is a
   scheduling barrier; it is the qcopy macro of docs/COMPILER-NOTES.md, which src/ does not
   allow. An empty `do {} while (0)` before and after the copies gives 6 lines (a steering
   construct, not used); `do { copy } while (0)` macros double the references inside and change
   the allocation (112+ lines); 180 orders of the six copy statements leave these 8 words.
   What the source needs, each measured: the colours built by sequential `|=` in rgba[1]/rgba[3]
   then copied to rgba[0]/rgba[2]; the variables through a struct (the global argument load of
   func_00281DE8 moves above the vars->spin store); a second loop counter ($t1 then $a0); the
   corner and vector pointers set at the end of the first block in this order; `pos` computed
   after func_L00_004181A8 (obj then crosses that call and lives in $s0; before the call obj stays
   in $t1 and the body is 8 bytes short); the fade block as written (the store of n * 4 fills the
   branch delay slot); 0.017453294f for pi/180.
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

extern float D_L23_001B1AF0 __attribute__((sda));
extern float D_L23_001B1AF4 __attribute__((sda));
extern float D_L23_001B1AF8 __attribute__((sda));
extern float D_L23_001B1AFC __attribute__((sda));
extern s32 D_L23_001B1B00 __attribute__((sda));
extern float D_L23_001B1B04 __attribute__((sda));
extern u32 D_L23_001B1B08 __attribute__((sda));
extern s32 D_L23_001B1B0C __attribute__((sda));
extern s32 D_L23_001B1B10 __attribute__((sda));
extern s32 D_L23_001B1B14 __attribute__((sda));
extern float D_L23_001B1B18 __attribute__((sda));
extern float D_L23_001B1B1C __attribute__((sda));
extern float D_L23_001B1B20 __attribute__((sda));
extern s32 D_L23_001B1B24 __attribute__((sda));
extern s32 D_L23_001B1B28 __attribute__((sda));
extern s32 D_L23_001B1B34 __attribute__((sda));
extern s32 D_L23_001B1B38 __attribute__((sda));
extern s32 D_L23_001B1B3C __attribute__((sda));
extern s32 D_L23_001B1B40 __attribute__((sda));
extern s32 D_L23_001B1B44 __attribute__((sda));
extern s32 D_L23_001B1B48 __attribute__((sda));
extern s32 D_L23_001B1B4C __attribute__((sda));
extern s32 D_L23_001B1B50 __attribute__((sda));
extern s32 D_L23_001B1B54 __attribute__((sda));
extern s32 D_L23_001B1B58 __attribute__((sda));
extern s32 D_L23_001B1B5C __attribute__((sda));
extern s32 D_L23_001B1B60 __attribute__((sda));
extern s32 D_L23_001B1B64 __attribute__((sda));
extern s32 D_L23_001B1B68 __attribute__((sda));
extern s32 D_L23_001B1B6C __attribute__((sda));
extern s32 D_L23_001B1B70 __attribute__((sda));
extern s32 D_L23_001B1B74 __attribute__((sda));
extern s32 D_L23_001B1B78 __attribute__((sda));
extern s32 D_L23_001B1B7C __attribute__((sda));
extern s32 D_L23_001B1B80 __attribute__((sda));
extern float D_L23_001B1B84 __attribute__((sda));
extern float D_L23_001B1B88 __attribute__((sda));
extern float D_L23_001B1B90 __attribute__((nosda));
extern float D_L23_001BA640[];

extern void func_L00_004181A8(L23Quad *);
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

void func_L23_00429430(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, u8 *obj) {
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
    u8 *pos;
    s32 n;
    float angle;
    float s;
    float c;
    int i;
    int j;

    vars = *(L23BeamVars **)(obj + 0x68);
    n = vars->fade;
    D_L23_001B1B40 = n * 4;
    if (D_L23_001B1B40 != 0) {
        D_L23_001B1B40--;
    }
    D_L23_001B1B50 = n * 2;
    D_L23_001B1B60 = n;
    D_L23_001B1B70 = n;
    D_L23_001B1B80 = n;
    func_L00_004181A8(&q);
    pos = obj + 0x10;
    func_00281DE8(&q, D_L23_001B1B24, 1, 0x80);
    q.rgba[1] = D_L23_001B1B40 << 24;
    q.rgba[1] |= D_L23_001B1B3C << 16;
    q.rgba[1] |= D_L23_001B1B38 << 8;
    q.rgba[1] |= D_L23_001B1B34;
    q.rgba[3] = D_L23_001B1B50 << 24;
    q.rgba[3] |= D_L23_001B1B4C << 16;
    q.rgba[3] |= D_L23_001B1B48 << 8;
    q.rgba[3] |= D_L23_001B1B44;
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
    func_00282CF0(rel, (float *)pos, D_L23_001BA640);
    func_00282F20(axis, vars->dir, -D_L23_001B1B1C);
    func_00282CC0(tip, (float *)pos, axis);
    func_00282DD8(side, rel, axis);
    func_00282F20(side, side, D_L23_001B1AFC);
    func_00282F20(base, vars->dir, D_L23_001B1B90);
    func_00282CC0(base, (float *)pos, base);
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

    func_00282F20(axis, vars->dir, -D_L23_001B1B20);
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
    vars->spin = func_00283B98(vars->spin, D_L23_001B1B18 * 0.017453294f);
    func_00281DE8(&q, D_L23_001B1B28, 1, 0x80);
    q.d70 = 5;

    q.rgba[1] = D_L23_001B1B0C << 24;
    q.rgba[1] |= D_L23_001B1B5C << 16;
    q.rgba[1] |= D_L23_001B1B58 << 8;
    q.rgba[1] |= D_L23_001B1B54;
    q.rgba[3] = D_L23_001B1B6C << 16;
    q.rgba[3] |= D_L23_001B1B68 << 8;
    q.rgba[3] |= D_L23_001B1B64;
    q.rgba[0] = q.rgba[1];
    q.rgba[2] = q.rgba[3];
    func_00282F20(side, side, D_L23_001B1AF0);
    func_00282F20(base, vars->dir, D_L23_001B1B90);
    func_00282CC0(base, (float *)pos, base);
    func_00282CC0((float *)&q.corner[0], base, side);
    func_00282CF0(c1, base, side);
    func_00282CC0(tip, base, axis);
    func_00282CC0(tip, tip, axis);
    func_00282CC0(c2, tip, side);
    func_00282CF0(c3, tip, side);
    func_00280B90(&q, 0, 0);

    q.rgba[1] = D_L23_001B1B0C << 24;
    q.rgba[1] |= D_L23_001B1B5C << 16;
    q.rgba[1] |= D_L23_001B1B58 << 8;
    q.rgba[1] |= D_L23_001B1B54;
    q.rgba[3] = D_L23_001B1B6C << 16;
    q.rgba[3] |= D_L23_001B1B68 << 8;
    q.rgba[3] |= D_L23_001B1B64;
    q.rgba[0] = q.rgba[1];
    q.rgba[2] = q.rgba[3];
    func_00282F20(side, side, D_L23_001B1AF4);
    func_00282F20(base, vars->dir, D_L23_001B1B84);
    func_00282CC0(base, (float *)pos, base);
    func_00282CC0((float *)&q.corner[0], base, side);
    func_00282CF0(c1, base, side);
    func_00282CC0(tip, base, axis);
    func_00282CC0(tip, tip, axis);
    func_00282CC0(c2, tip, side);
    func_00282CF0(c3, tip, side);
    func_00280B90(&q, 0, 0);

    q.rgba[1] = D_L23_001B1B14 << 24;
    q.rgba[1] |= D_L23_001B1B5C << 16;
    q.rgba[1] |= D_L23_001B1B58 << 8;
    q.rgba[1] |= D_L23_001B1B54;
    q.rgba[3] = D_L23_001B1B6C << 16;
    q.rgba[3] |= D_L23_001B1B68 << 8;
    q.rgba[3] |= D_L23_001B1B64;
    q.rgba[0] = q.rgba[1];
    q.rgba[2] = q.rgba[3];
    func_00282F20(side, side, D_L23_001B1AF8);
    func_00282F20(base, vars->dir, D_L23_001B1B84);
    func_00282CC0(base, (float *)pos, base);
    func_00282CC0((float *)&q.corner[0], base, side);
    func_00282CF0(c1, base, side);
    func_00282CC0(tip, base, axis);
    func_00282CC0(tip, tip, axis);
    func_00282CC0(c2, tip, side);
    func_00282CF0(c3, tip, side);
    func_00280B90(&q, 0, 0);

    q.rgba[1] = D_L23_001B1B10 << 24;
    q.rgba[1] |= D_L23_001B1B7C << 16;
    q.rgba[1] |= D_L23_001B1B78 << 8;
    q.rgba[1] |= D_L23_001B1B74;
    q.rgba[3] = D_L23_001B1B6C << 16;
    q.rgba[3] |= D_L23_001B1B68 << 8;
    q.rgba[3] |= D_L23_001B1B64;
    q.rgba[0] = q.rgba[1];
    q.rgba[2] = q.rgba[3];
    func_00282F20(side, side, D_L23_001B1AF0);
    func_00282F20(base, vars->dir, D_L23_001B1B88);
    func_00282CC0(base, (float *)pos, base);
    func_00282CC0((float *)&q.corner[0], base, side);
    func_00282CF0(c1, base, side);
    func_00282F20(axis, vars->dir, -0.6f);
    func_00282CC0(tip, base, axis);
    func_00282CC0(tip, tip, axis);
    func_00282CC0(c2, tip, side);
    func_00282CF0(c3, tip, side);
    func_00280B90(&q, 0, 0);

    func_00281EB8(pos, D_L23_001B1B00, D_L23_001B1B08, 1, D_L23_001B1B04, 0.08f, func_002A7940());
    func_00281EB8(pos, D_L23_001B1B00, 0x80808080, 1, D_L23_001B1B04 * 0.4f, 0.08f, func_002A7940());
}
