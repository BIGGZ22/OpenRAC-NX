/* func_L23_00425AF8 (1516 bytes, level 23 only) -- NEAR MISS, not integrated.
   Belongs in a unit of its own, src/levels/23_damosel_orbit/L23_00425AF8-g8.c, with the
   config/cflags.txt line `-O2 -G8 -ffunction-sections -g`. It needs -G8 for the bare $gp load
   of D_L23_001B19DC (0x29EC($gp)) and -g for the two nops before each of the three div.s. Its
   marker stays in L23_0041E418.c (-G0, no -g); the caller func_L23_004259E0 declares the same
   argument order.
   This C compiles to 1464 bytes against 1516 (check.py: "1464 bytes, expected 1516"). With an
   alignment on whole words, 263 of retail's 379 match; on opcodes alone, 338. Most of the rest
   is register allocation.
   Right:
   - the frame (0x270), with draw records A (sp+0) and B (sp+0x90), each 0x90 bytes;
   - the ten vector temporaries at 0x120..0x1B0 in declaration order. They must be float[4]
     arrays: TI16 scalars get their slots in order of first use (v140 lands at 0x120);
   - the five saved float registers. Each normalisation needs its own float variable; one
     shared `len` takes a sixth saved register;
   - the initialisation, the three normalise-and-scale blocks (retail's bc1tl and the -g nops),
     both loops' calls, the colour selection and the closing call.
   What differs:
   - four quadword copies. In loop 1 they are `A.v[0] = A.v[2]; A.v[1] = A.v[3];` after the
     draw call. Retail has `lq v0,0(s2); sq v0,0(sp); addiu v1,sp,0x10; lq v0,0(s3);
     sq v0,0(v1)`, with s2/s3 set right after the A.v[3] call and kept across the colour chain
     and the call. In loop 2 they are `v190 = v180` / `v180 = v190`. Retail has
     `bc1fl {a0}; lq v0,0(s3); sq v0,0(s1); b {addiu s0,sp,0x160}`. Retail's copies take
     pointer operands, never sit in a delay slot and have nothing scheduled into them: the
     inline-asm quadword helpers again (see func_L23_002BBC50). This cc1 makes them
     frame-relative, schedules them and puts them in delay slots;
   - register allocation, which follows from that. Retail spends all nine saved registers in
     loop 1 (s2/s3 on the copy pointers), so the frame addresses PRE makes available there are
     spilled and rematerialised (`addiu a0,sp,0x140` at each use). Here loop.c hoists two of
     them into the two free registers. obj/d/colA/&v140 take s6/s3/s4/fp where retail has
     fp/s4/s6/s3. Copy pointers in plain C give retail's loop-1 rematerialisation and its
     obj/d/colA registers, but cost the first block's float constants (253 words). Those
     pointers are sources set after the A.v[3] call and read after the colour join, plus a
     destination pointer set before the loop;
   - loop 2: retail keeps pts+i spilled at 0x1E4 and pts+i+1 in $fp. Here recombine_givs
     derives pts+i+1 from pts+i, because their lifetimes do not overlap. Explicit pointers,
     with q set before p, keep both but allocate worse (233);
   - retail copies colB into $v0 before the four B.col stores, a reload choice for the spilled
     colB. A second variable, a chained assignment and `t = ...; colB = t` all compile
     identically to this draft.
   Measured: 32 combinations of four toggles. They are the loop-1 copy spelling (4), pointer
   variables for v180/v190, explicit p/q in loop 2 and a pointer for n. All score at or below
   this draft. -fno-gcse gives 152 words.
*/

typedef unsigned char u8;
typedef int s32;
typedef unsigned long long u64;
typedef int TI16 __attribute__((mode(TI)));

typedef struct {
    TI16 v[4];
    s32 col[4];
    float uv[8];
    u64 f70;
    u64 tex;
    u64 f80;
    u64 f88;
} Quad_00425AF8;

extern s32 D_L23_001B19DC __attribute__((sda));
extern float D_L23_001BA640[];

extern u64 func_0027C878(s32);
extern void func_00282CF0(void *, void *, void *);
extern void func_00282DD8(void *, void *, void *);
extern float func_00282DF0(void *);
extern void func_00282D30(void *, void *, float);
extern void func_00282CC0(void *, void *, void *);
extern void func_00280B90(void *, s32, s32);
extern void func_002FC560(s32, u64);

void func_L23_00425AF8(TI16 *pts, s32 a, s32 b, s32 c, float x, float y, float z, s32 d, s32 e) {
    Quad_00425AF8 A;
    Quad_00425AF8 B;
    float n[4];
    float v130[4];
    float v140[4];
    float v150[4];
    float v160[4];
    float v170[4];
    float v180[4];
    float v190[4];
    float v1A0[4];
    float v1B0[4];
    s32 i;
    s32 colA;
    s32 colB;
    float l0;
    float l1;
    float la;
    float lb;
    float l3;

    A.tex = func_0027C878(D_L23_001B19DC);
    colA = d | (a << 24);
    A.f88 = 0x8000000048LL;
    A.f80 = 0xFF9000000260LL;
    A.f70 = 0;
    A.col[3] = colA;
    A.col[2] = colA;
    A.col[1] = colA;
    A.col[0] = colA;
    B.tex = func_0027C878(16);
    colB = e | (b << 24);
    B.f88 = A.f88;
    B.f80 = A.f80;
    B.f70 = A.f70;
    B.col[3] = colB;
    B.col[2] = colB;
    B.col[1] = colB;
    B.col[0] = colB;
    B.uv[0] = 0.0f;
    B.uv[1] = 0.0f;
    B.uv[2] = 0.0f;
    B.uv[3] = 1.0f;
    B.uv[4] = 1.0f;
    B.uv[5] = 0.0f;
    B.uv[6] = 1.0f;
    B.uv[7] = 1.0f;
    A.uv[0] = 0.0f;
    A.uv[1] = 0.0f;
    A.uv[2] = 0.0f;
    A.uv[3] = 1.0f;
    A.uv[4] = 1.0f;
    A.uv[5] = 0.0f;
    A.uv[6] = 1.0f;
    A.uv[7] = 1.0f;
    func_00282CF0(v140, &pts[0], D_L23_001BA640);
    func_00282CF0(v150, &pts[1], &pts[0]);
    func_00282DD8(n, v150, v140);
    l0 = func_00282DF0(n);
    if (l0 != 0.0f) {
        l0 = 1.0f / l0;
    }
    func_00282D30(n, n, x * l0);
    func_00282CC0(&A.v[0], &pts[0], n);
    func_00282CF0(&A.v[1], &pts[0], n);
    for (i = 1; i < c - 1; i++) {
        func_00282CF0(v140, &pts[i], D_L23_001BA640);
        func_00282CF0(v150, &pts[i + 1], &pts[i]);
        func_00282DD8(n, v150, v140);
        l1 = func_00282DF0(n);
        if (l1 != 0.0f) {
            l1 = 1.0f / l1;
        }
        func_00282D30(n, n, x * l1);
        func_00282CC0(&A.v[2], &pts[i], n);
        func_00282CF0(&A.v[3], &pts[i], n);
        if (i == 1) {
            A.col[1] = d;
            A.col[0] = d;
        } else if (i == c - 2) {
            A.col[3] = d;
            A.col[2] = d;
        } else if (i == 2) {
            A.col[1] = colA;
            A.col[0] = colA;
        }
        func_00280B90(&A, 0, 0);
        A.v[0] = A.v[2];
        A.v[1] = A.v[3];
    }
    for (i = 0; i < c - 1; i++) {
        func_00282CF0(v180, &pts[i], D_L23_001BA640);
        func_00282CF0(v190, &pts[i + 1], D_L23_001BA640);
        la = func_00282DF0(v180);
        lb = func_00282DF0(v190);
        if (lb < la) {
            if (lb == 0.0f) {
                *(TI16 *)v190 = *(TI16 *)v180;
            } else {
                func_00282D30(v190, v190, la / lb);
            }
        } else {
            if (la == 0.0f) {
                *(TI16 *)v180 = *(TI16 *)v190;
            } else {
                func_00282D30(v180, v180, lb / la);
            }
        }
        func_00282CC0(v160, D_L23_001BA640, v180);
        func_00282CC0(v170, D_L23_001BA640, v190);
        func_00282CF0(v150, v170, v160);
        func_00282D30(v1B0, v150, z);
        func_00282CF0(v160, v160, v1B0);
        func_00282CC0(v170, v170, v1B0);
        func_00282CF0(v1A0, D_L23_001BA640, v160);
        func_00282DD8(n, v1A0, v150);
        l3 = func_00282DF0(n);
        if (l3 != 0.0f) {
            l3 = 1.0f / l3;
        }
        func_00282D30(v130, n, y * l3);
        func_00282CC0(&B.v[0], v160, v130);
        func_00282CF0(&B.v[1], v160, v130);
        func_00282CC0(&B.v[2], v170, v130);
        func_00282CF0(&B.v[3], v170, v130);
        if (i == 0) {
            B.col[1] = e;
            B.col[0] = e;
        } else if (i == c - 2) {
            B.col[3] = e;
            B.col[2] = e;
        } else if (i == 1) {
            B.col[1] = colB;
            B.col[0] = colB;
        }
        func_00280B90(&B, 0, 0);
    }
    func_002FC560(0x47, 0x5360B);
}
