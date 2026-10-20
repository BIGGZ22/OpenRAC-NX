/* Near miss from src/levels/20_yeedil/L20_0039C448-g8.c (the -G8 unit).

   The body has the retail size (444 bytes) and the block order, the calls
   and the arithmetic all match.  What is left are the register choices and
   one instruction more or less in the prologue.

   What the attempt gets right:
   - the two float products of the prologue (f22 = D_L20_001AED10 * 1/3600
     and f21 = D_L20_001AED0C * 1/60), the two calls of the else branch
     (f_00333100 with seven arguments and f_00332C48 with six), the
     comparison that picks between them, the f_003021D0 / f_003021B8 pair
     and the negation of the word at 0x190(state);
   - the constants: 0.00027777778450399637f (1/3600), 0.01666666753590107f
     (1/60), 3.1415927410125732f (pi), 0.01745329424738884f (pi/180),
     0.0008726647356525064f (pi/3600) and 0.05235988646745682f (pi/60);
   - the last comparison has to be written as an if/else with an empty
     then (`if ((f32)func_003021B8(...) <= p) { } else { ... }`): the
     plain `if (a > b)` gives the swapped operands `c.lt.s b,a`, the
     retail has `c.le.s a,b`.

   What is still different:
   - the frame: the retail's is 0x40 bytes, this attempt's is 0x50.  The
     retail keeps only $s0, $s1 and $s2; this attempt also keeps $s3, the
     base pointer D_L20_001B308C, which it holds across the calls.  The
     retail reads the global again at the end (`lui v0,%hi(D_1B308C) /
     lw v0,%lo(D_1B308C)(v0)`) and does not keep it in a register.  The
     index at 0x84(state) is read twice in the attempt as well, which is
     what the retail does, but the base pointer is not: it is the one value
     the compiler keeps across the calls, so it needs a saved register.
     Measured: a load of a global is not merged across a call (two loads
     come out), but here the value is live in the entry block, so it
     reaches the exit block without a phi and the reload pass keeps it in
     $s3 instead of loading it again.
   - the two constants of the prologue are in $f0/$f1 the other way round
     (the retail materialises 1/3600 into $f0 and 1/60 into $f1, this
     attempt the other way), which moves the two multiplies by one
     instruction;
   - the product of 0x194(state) and the word at 0x190(state) is in $f20
     here (a saved register, live across the f_003021D0 call) and in $f0
     in the retail, which copies it into $f20 in the delay slot of the
     f_003021B8 call.

   The attempt belongs in the -G8 unit (it reaches D_L20_001AED10 and
   D_L20_001AED0C through $gp). */

extern float func_00303270(float, float);

extern float func_00303358(float, float);

extern void func_00333100(void *, void *, int, float, float, float, float);

extern void func_00332C48(void *, void *, float, float, float, float);

extern void func_003021D0(void);

extern int func_003021B8(int);

extern void func_00302230(void *);

extern void func_00302750(void *, void *, void *);

extern char *D_L20_001B308C __attribute__((nosda));

extern float D_L20_001AED10 __attribute__((sda));

extern float D_L20_001AED0C __attribute__((sda));

void func_L20_0039E9F8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *base = D_L20_001B308C;
    f32 f22 = D_L20_001AED10 * 0.00027777778450399637f;
    f32 f21 = D_L20_001AED0C * 0.01666666753590107f;
    f32 f20 = *(f32 *)(base + (*(int *)(state + 0x84) << 7) + 0x78);
    f32 r;

    if (*(int *)(state + 0x190) < 0) {
        f20 = func_00303270(f20, 3.1415927410125732f);
    }
    r = func_00303358(*(f32 *)(arg0 + 0xF8), f20);
    if (r > 0.01745329424738884f) {
        func_00333100(arg0 + 0xF8, state + 0x19C, 0, f20, 0.0008726647356525064f,
                      0.0008726647356525064f, 0.05235988646745682f);
    } else {
        f32 p;

        func_00332C48(state + 0x194, state + 0x198, (f32)*(int *)(state + 0x190),
                      f22, f22, f21);
        p = *(f32 *)(state + 0x194) * (f32)*(int *)(state + 0x190);
        if (0.0f < p) {
            func_003021D0();
            if ((f32)func_003021B8(*(int *)(state + 0x190)) <= p) {
            } else {
                *(int *)(state + 0x190) = -*(int *)(state + 0x190);
            }
        }
    }
    {
        char *p = arg0 + 0x10;

        func_00302230(p);
        *(f32 *)p = *(f32 *)(state + 0x194);
        func_00302750(p, p, base + (*(int *)(state + 0x84) << 7));
    }
}

/* ------------------------------------------------------------------
   Third attempt (worker 07, level-20 wave 3); the C in this file is that
   third attempt, the two earlier ones are described above.  It is closer:
   the frame is the retail's 0x40 and 87 of the retail's 112 instructions are
   byte-identical; what is left are three register or scheduling choices.

   What is new in this attempt:
   - D_L20_001B308C is written as a FLOAT POINTER
     (`D_L20_001B308C_1[(*(int *)(state + 0x84) << 5) + 0x1E]` and
     `D_L20_001B308C_1[(*(int *)(state + 0x84) << 5) + 0xE] = f20`).
     That is what produces the retail's address arithmetic
     (`sll $v0,$v0,7 ; addu $v0,$v0,$a0 ; swc1 $f20,0x38($v0)`) with the
     load of the global in $a0 where the retail has it.  As a char * the
     compiler folds the whole address into the lq/sw and moves the load
     of the global to the top of the function.
   - the global is read again in the tail, after the call to
     func_00302230: the call may change it, so the reload is required.
   - the arg0 + 0x10 value is the expression at each use (not a
     variable): the retail rematerialises it into the delay slots of
     the three branches.
   - func_003021D0 returns a FLOAT (the retail copies $f0 into $f20 in
     the delay slot of the call to func_003021B8); func_003021B8 returns
     an int (the retail converts it with mtc1/cvt.s.w).
   - the comparison is `0.0f < (f32)*(int *)(state + 0x190) *
     *(f32 *)(state + 0x194)`, with the int conversion first: the
     retail's `mul.s $f0,$f12,$f0` puts the result in the register of
     the first operand.

   What is still different (three places, all of them register or
   scheduling choices):
   1. the call to func_00332C48: the retail sets up the float arguments
      as `mov.s $f13,$f22 ; mov.s $f15,$f21 ; addiu $a0 ; addiu $a1`
      and puts the copy `mov.s $f14,$f13` in the delay slot of the jal;
      this attempt sets the copy up before the two int arguments and
      leaves `addiu $a1` in the delay slot.  Measured: the int
      arguments first, the third float argument through a temporary and
      void * instead of char * do not change it.
   2. the comparison: the retail loads the float into $f12 and keeps the
      zero in $f1, this attempt loads it into $f1 and puts the zero in
      $f2.  Measured: the other operand order, a `>` comparison and a
      separate statement for the product do not change it.
   3. the tail: the retail reloads D_L20_001B308C before the copy of
      $a0 and the store of the float, this attempt reloads it after
      them.

   The attempt belongs in the -G8 unit (it reaches D_L20_001AED10 and
   D_L20_001AED0C through $gp). */

void func_L20_0039E9F8(char *arg0) {
    char *s2 = arg0;
    f32 f22 = D_L20_001AED10 * 0.00027777778f;
    f32 f21 = D_L20_001AED0C * 0.016666668f;
    char *s1 = *(char **)(s2 + 0x68);
    f32 f20;

    f20 = D_L20_001B308C_1[(*(int *)(s1 + 0x84) << 5) + 0x1E];
    if (*(int *)(s1 + 0x190) < 0) {
        f20 = func_00303270(f20, 3.1415927f);
    }
    if (0.017453294f < func_00303358(*(f32 *)(s2 + 0xF8), f20)) {
        func_00333100(f20, 0.00087266474f, 0.00087266474f, 0.052359886f,
                      s2 + 0xF8, s1 + 0x19C, 0);
    } else {
        func_00332C48((f32)*(int *)(s1 + 0x190), f22, f22, f21, s1 + 0x194, s1 + 0x198);
        if (0.0f < (f32)*(int *)(s1 + 0x190) * *(f32 *)(s1 + 0x194)) {
            f20 = func_003021D0();
            if ((f32)func_003021B8(*(int *)(s1 + 0x190)) <= f20) {
                *(int *)(s1 + 0x190) = -*(int *)(s1 + 0x190);
            }
        }
    }
    func_00302230(s2 + 0x10);
    *(f32 *)(s2 + 0x10) = *(f32 *)(s1 + 0x194);
    func_00302750(s2 + 0x10, s2 + 0x10, (char *)D_L20_001B308C_1 + (*(int *)(s1 + 0x84) << 7));
}
