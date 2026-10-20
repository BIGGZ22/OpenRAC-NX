/* func_L23_003C1E88 (668 bytes, level 23 only) -- NEAR MISS, not integrated.
   State: 668 bytes against retail's 668; 21 words differ, the first at +0x98 (the start of the
   first quadword copy). Needs the -G8 unit (src/levels/23_damosel_orbit/L23_0035CE90-g8.c, the
   $gp loads). The body keeps retail's calls, frame (q at sp+0, v90..vE0 at sp+0x90..0xE0), clamp,
   packet stores and $gp loads; one `movz $a2,$a2,$s0` (a no-op conditional move left by the
   pointer trick below) stands in for retail's `addiu $v1,$s0,0xE0`, so the size is equal and the
   instruction multiset is not.

   Cause of the residue (measured): retail's two 16-byte copies are volatile inline asm with
   register operands and a $v0 scratch, i.e.
       #define COPY128(dst, src) \
           __asm__ __volatile__("lq $2,0(%1)\n\tsq $2,0(%0)" :: "r"(dst), "r"(src) : "$2")
   Scratch variant (not committable, inline asm is not allowed here): this body with the four
   pointer variables and the `if (obj)` trick removed, the copies written
       COPY128(&v90, obj + 0x10);   (right after `f20 = D_L23_001AFA18;`, args of the call: obj + 0x10)
       COPY128(&vB0, obj + 0xE0);   (after the func_00282CC0(&vA0, &vA0, &v90) call)
   and `&v90` / `&vB0` used directly in the later calls. With that, tools/check.py printed
       func_L23_003C1E88: 1 of 1 places match (668 of 668)
   (a temporary edit of the -g8 unit, reverted). The packet-store order below is the one that
   was exact there.
   What the asm does that plain C cannot: (1) a volatile asm with no memory clobber makes cse
   forget every known address and is a full scheduling barrier: retail's `lwc1 $f20` is the first
   instruction of the block and the `addiu`s that feed the later calls stay after the `sq`, where
   the volatile-TImode copies here let them be hoisted; that shorter/longer live range is what flips
   local-alloc's priority between the v90 pointer and the vC0 pointer (`$s3`/`$s4` swapped here);
   (2) its explicit `$2` clobber keeps `$v0` out of the operand registers (retail: `lq $v0,0($v1)`,
   the address in `$v1`; ours `lq $v1,0($v0)`).

   Plain-C levers measured on the way: a volatile TI128 access reaches lq/sq; `if (c) p = &x;
   else p = &x;` (or `p = &x; if (c) p = &x;`) makes the pointer multi-assigned, so neither cse nor
   reload folds the offset into the access: `addiu $a2,$s0,0x10 ; lq $v0,0($a2)` like retail
   instead of `lq $v0,0x10($s0)`. Which register is the condition decides whether a no-op `movz`
   stays. Scalar TI128 locals take their stack slot where their address is first taken, arrays
   where they are declared, in ascending order. */

typedef int TI128 __attribute__((mode(TI)));

extern float D_L23_001AFA18 __attribute__((sda));
extern float D_L23_001AFA1C __attribute__((sda));
extern float D_L23_001AFA20 __attribute__((sda));
extern int D_L23_001AFA24 __attribute__((sda));
extern int D_L23_001AFA28 __attribute__((sda));
extern int D_L23_001AFA2C __attribute__((sda));
extern int D_L23_001AFA30 __attribute__((sda));
extern int D_L23_001AFA34 __attribute__((sda));
extern int D_L23_001AFA38 __attribute__((sda));
extern float D_L23_001AFA3C __attribute__((sda));
extern float D_L23_001AFA40 __attribute__((sda));
extern float D_L23_001AFA44 __attribute__((sda));
extern int D_L23_001AFA48 __attribute__((sda));

extern char D_L23_001BA640[];

extern float func_002A7878(float, float);
extern void func_00282CF0(void *, void *, void *);
extern void func_00282D30(void *, void *, float);
extern void func_00282CC0(void *, void *, void *);
extern void func_00282DD8(void *, void *, void *);
extern void func_00282F20(void *, void *, float);
extern int func_0027C878(int);
extern void func_002FC560(unsigned int, unsigned long);
extern void func_00280B90(void *, int, int);

void func_L23_003C1E88(char *obj) {
    TI128 vE0;
    TI128 vD0;
    TI128 vC0;
    TI128 vB0;
    TI128 vA0;
    TI128 v90;
    char q[0x90];
    char *state = *(char **)(obj + 0x68);
    TI128 *p90;
    TI128 *pB0;
    char *o10;
    char *oE0;
    float f20;
    float f;
    float lim;
    long x;
    int c;
    float z;
    long long bits;

    if (*(short *)(state + 0x3BC) > 0) {
        f = *(float *)(state + 0x3B8) + func_002A7878(D_L23_001AFA40, D_L23_001AFA44);
        lim = D_L23_001AFA3C;
        *(float *)(state + 0x3B8) = f;
        if (lim < f) {
            *(float *)(state + 0x3B8) = lim;
            *(short *)(state + 0x3BC) = -1;
        }
    } else {
        f = *(float *)(state + 0x3B8) - func_002A7878(D_L23_001AFA40, D_L23_001AFA44);
        lim = (float)D_L23_001AFA38;
        *(float *)(state + 0x3B8) = f;
        if (f < lim) {
            *(float *)(state + 0x3B8) = lim;
            *(short *)(state + 0x3BC) = 1;
        }
    }
    if (obj)
        o10 = obj + 0x10;
    else
        o10 = obj + 0x10;
    p90 = &v90;
    *(volatile TI128 *)p90 = *(volatile TI128 *)o10;
    f20 = D_L23_001AFA18;
    func_00282CF0(&vA0, *(char **)(state + 0x2A0) + 0x10, o10);
    func_00282D30(&vA0, &vA0, D_L23_001AFA1C);
    func_00282CC0(&vA0, &vA0, p90);
    oE0 = obj + 0xE0;
    pB0 = &vB0;
    *(volatile TI128 *)pB0 = *(volatile TI128 *)oE0;
    func_00282CF0(&vC0, p90, D_L23_001BA640);
    func_00282DD8(&vD0, pB0, &vC0);
    func_00282F20(&vD0, &vD0, f20);
    func_00282F20(&vE0, &vC0, -D_L23_001AFA20);
    func_00282CC0(p90, p90, &vE0);
    func_00282CC0(&vA0, &vA0, &vE0);
    func_00282CC0(q + 0x00, p90, &vD0);
    func_00282CF0(q + 0x10, p90, &vD0);
    func_00282CC0(q + 0x20, &vA0, &vD0);
    func_00282CF0(q + 0x30, &vA0, &vD0);
    x = func_0027C878(D_L23_001AFA24);
    c = D_L23_001AFA48;
    *(int *)(q + 0x4C) = c;
    *(long *)(q + 0x78) = x;
    *(unsigned long long *)(q + 0x80) = 0xFF9000000260ULL;
    *(long *)(q + 0x70) = 0;
    *(int *)(q + 0x48) = c;
    *(int *)(q + 0x44) = c;
    *(int *)(q + 0x40) = c;
    func_002FC560(0x47, 0x53001);
    bits = D_L23_001AFA28;
    bits |= (long long)D_L23_001AFA2C << 2;
    bits |= (long long)D_L23_001AFA30 << 4;
    bits |= (long long)D_L23_001AFA34 << 6;
    bits |= (long long)(int)*(float *)(state + 0x3B8) << 32;
    *(int *)(q + 0x54) = 0;
    *(float *)(q + 0x5C) = 1.0f;
    *(int *)(q + 0x50) = 0;
    *(long long *)(q + 0x88) = bits;
    *(int *)(q + 0x64) = 0;
    z = *(float *)(q + 0x50);
    *(float *)(q + 0x68) = z;
    *(float *)(q + 0x6C) = 1.0f;
    *(float *)(q + 0x60) = z;
    *(float *)(q + 0x58) = 0.99f;
    *(float *)(q + 0x50) = 0.99f;
    func_00280B90(q, 0, 0);
}
