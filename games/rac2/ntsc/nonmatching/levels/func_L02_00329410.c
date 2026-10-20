/* func_L02_00329410 (196 bytes, 10 places, default profile -O2 -G0)
   -- measured 0 of 10: candidate is 200 bytes, ONE instruction over.

   Three-way spawn parameter setup, all arms ending in func_00283248(f, v):
     - D_001A8FF0 == 0: v = func_00282E20(arg1), f = *(float *)(arg1 + 8);
     - arg2 == 0: a 0x20-byte buffer at $sp, func_002AFFD8(buf + 0x10, buf + 0x10,
       1), func_00282F20(buf, arg1, 1.0f), f = func_00282DB0(buf, buf + 0x10),
       v = func_00282C10(buf, 1.0f - f);
     - otherwise: func_00283098(buf, arg1), v = func_00282E20(buf),
       f = *(float *)(buf + 8).
   The first parameter is never read (its register is overwritten everywhere).

   Measured: the shape that gets closest is one `return func_00283248(...)` per arm
   (200 bytes), NOT a single joined `return` after an if/else-if chain (204). The
   remaining instruction is in the arg2 == 0 arm, between the func_00282DB0 and
   func_00282C10 calls: retail emits `mov.s $f20,$f0` as a separate instruction and
   puts `sub.s $f12,$f21,$f20` in the next call's delay slot, this cc1 sinks the
   `mov.s` into the func_00282DB0 delay slot and has to keep one more instruction.
   Retail also builds the first helper's arguments as `addiu $a0,$a0,0x10` (from
   `$sp`) plus a live `$s0 = $sp + 0x10` for the second, where the candidate feeds
   `$s0` to both; naming the second argument (`char *q = buf + 0x10;`) does not
   change the size. */

typedef int s32;

extern s32 D_001A8FF0;
extern float func_00282E20(void *);
extern void func_002AFFD8(void *, void *, int);
extern void func_00282F20(void *, void *, float);
extern float func_00282DB0(void *, void *);
extern float func_00282C10(void *, float);
extern void func_00283098(void *, void *);
extern float func_00283248(float, float);

float func_L02_00329410(s32 arg0, char *arg1, s32 arg2) {
    char buf[0x20];
    float v;
    float f;
    float one;

    if (D_001A8FF0 == 0) {
        v = func_00282E20(arg1);
        return func_00283248(*(float *)(arg1 + 8), v);
    }
    if (arg2 == 0) {
        one = 1.0f;
        func_002AFFD8(buf + 0x10, buf + 0x10, 1);
        func_00282F20(buf, arg1, one);
        f = func_00282DB0(buf, buf + 0x10);
        v = func_00282C10(buf, one - f);
        return func_00283248(f, v);
    }
    func_00283098(buf, arg1);
    v = func_00282E20(buf);
    return func_00283248(*(float *)(buf + 8), v);
}
