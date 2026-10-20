/* func_L23_003C1420 (692 bytes, level 23 only) -- NEAR MISS, not integrated.
   State: 692 bytes against retail's 692, 55 words differ, the first at +0x40 (the order of the two
   `por`-fed zero stores around the `lwc1`s). The equal size is a coincidence: the body has one `por`
   fewer than retail (retail keeps `por $v0,$zero,$zero ; ... ; sq $v0,0x50($sp)` for the zero of the
   vector at sp+0x50, ours is folded to `sq $zero,0x50($sp)`) and one extra `movz` (a no-op left by
   the pointer trick below). The registers of the vector pointers (`$s2`/`$s3`) are also swapped.
   Needs the -G8 unit (src/levels/23_damosel_orbit/L23_0035CE90-g8.c, for `lw 0xA64($gp)`).

   Cause of the residue (measured): the three copies/clears that go through materialised pointers are
   volatile inline asm in retail, not compiler TImode moves:
     - `lq $v0,0($s4) ; sq $v0,0($s3)` (the vector at sp+0x20 = D_L23_001C0AC0, both pointers kept in
       callee-saved registers for the later calls) and `lq $v0,0($s0) ; sq $v0,0($s5)` (state+0x2D0 =
       the vector at sp+0x70), both of the shape
           __asm__ __volatile__("lq $2,0(%1)\n\tsq $2,0(%0)" :: "r"(dst), "r"(src) : "$2")
       -- a scratch variant of this body with those two copies written that way has the right
       registers and structure (688 bytes, 36 words differ: the missing `por`, the order of two
       stores, and the `$s2`/`$s3` roles);
     - the `por`-kept zero store at 0x50: the project's zero fold pass (a plain TImode zero store whose
       register is dead after it becomes `sq $zero`) never lets a compiler-generated store keep its
       `por`, so that store in retail is not compiler-generated either (same finding as in
       func_L23_003C16D8).
   The two TImode copies between locals (`lq 0x10($sp) ; sq 0($sp)` with the `sq` in a delay slot,
   `lq 0x20($sp) ; sq 0x10($sp)`) are ordinary compiler TImode moves and are exact here. The first
   `por` feeds two stores (sp+0x10, sp+0x20), which this compiler also does.
   Inline asm is not allowed here, so this stays a near miss.

   Plain-C levers used: a volatile TI128 access reaches lq/sq; `p = &x; if (c) p = &x;` makes the
   pointer multi-assigned so the access stays `0($reg)` (retail) instead of folding `0x20($sp)`.
   Arrays/aggregates take their stack slot where declared, scalars where their address is first
   taken, both ascending (loc[0..7] at sp+0x00..0x70 as retail). */

typedef int TI128 __attribute__((mode(TI)));

extern float D_L23_001B36A4 __attribute__((nosda));
extern int D_L23_001AFA54 __attribute__((sda));

extern TI128 D_L23_001C0AA0[];
extern char D_00189ED0[];

extern int func_002A77E0(int);
extern int func_002A7820(int, int);
extern void func_00282D30(void *, void *, float);
extern void func_00282CC0(void *, void *, void *);
extern void func_00282CF0(void *, void *, void *);
extern void func_00283098(void *, void *, void *);
extern void func_00282F20(void *, void *, float);
extern float func_00282DB0(void *, void *);
extern void func_L22_002F3B38(void *, void *, void *);
extern int func_002761F8(void *, void *, int, int, int);

#define F(i, k) (((float *)&loc[i])[k])

int func_L23_003C1420(char *obj) {
    TI128 loc[8];
    char *state = *(char **)(obj + 0x68);
    int r;
    TI128 *pg;
    TI128 *pl;
    TI128 *ps;
    TI128 *pd;

    loc[1] = 0;
    loc[2] = 0;
    F(1, 2) = *(float *)(state + 0x54) + *(float *)(state + 0x50);
    F(2, 0) = 4.0f;
    loc[0] = loc[1];
    r = func_002A77E0(8);
    F(2, 1) = (float)r * 0.4285714626f + -1.5f;
    F(2, 2) = *(float *)(state + 0x54) + *(float *)(state + 0x50);
    loc[1] = loc[2];
    func_00282D30(&loc[0], obj + 0xE0, F(0, 2));
    func_00282CC0(&loc[0], &loc[0], obj + 0x10);
    func_00283098(&loc[1], &loc[1], obj + 0xC0);
    func_00282CC0(&loc[1], &loc[1], obj + 0x10);
    if (func_002761F8(&loc[0], &loc[1], 0x12, 0, 0) == 0)
        return 0;
    pg = &D_L23_001C0AA0[2];
    if (obj)
        pg = &D_L23_001C0AA0[2];
    pl = &loc[2];
    if (obj)
        pl = &loc[2];
    *pl = *pg;
    loc[5] = 0;
    F(5, 0) = 0.0009765625f;
    F(5, 1) = 0.0009765625f;
    F(5, 2) = 0.0009765625f;
    func_L22_002F3B38(pl, pl, &loc[5]);
    pg -= 2;
    func_00282F20(pl, pl, 1.0f);
    func_00282CF0(&loc[3], &loc[0], pg);
    func_00282D30(&loc[4], pl, func_00282DB0(&loc[3], pl));
    func_00282CF0(&loc[6], &loc[3], &loc[4]);
    func_00282F20(&loc[6], &loc[6], -30.0f);
    ps = (TI128 *)(state + 0x2D0);
    if (state)
        ps = (TI128 *)(state + 0x2D0);
    pd = &loc[7];
    if (state)
        pd = &loc[7];
    func_00282CC0(ps, obj + 0x10, &loc[6]);
    func_00282CF0(pd, ps, D_00189ED0);
    func_00282F20(pd, pd, D_L23_001B36A4);
    func_00282CC0(pd, pd, D_00189ED0);
    *ps = *pd;
    *(short *)(state + 0x26C) = 1;
    r = D_L23_001AFA54;
    *(unsigned short *)(state + 0x26E) = (unsigned short)D_L23_001AFA54 + func_002A7820(-r / 2, r / 2);
    func_00282F20(pl, pl, 10.0f);
    func_00282CC0(pl, pl, pg);
    return 1;
}
