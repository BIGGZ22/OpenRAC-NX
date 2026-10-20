/* 256 bytes against retail's 268; level 2 only. Places a point around a centre: arg0 is moved to arg1's quadword,
   then displaced by `dist` (jittered) along the bearing from arg1 to arg0's old position (also jittered by an
   angle given in degrees), with z jittered separately. The two quadword copies are the only difference. The
   first, `vec = *arg1`, is the compiler's own and is exact (`lq $v0,0($a1)` as the first instruction, the
   `sq` into the local at sp+0). The second, `*arg0 = vec`, is `lq $v0,0($s1)` / `sq $v0,0($s0)` in retail with
   $s1 = sp + 0 held in a callee-saved register since the prologue (`daddu $s1,$sp,$zero`, one more saved
   register, frame 0x60); this cc1 reads the local directly (`lq $v0,0($sp)`), because cse replaces a copy of
   the frame pointer by the frame pointer, so no pointer is held and the frame is 0x50.
   Tried: a `TI128 *p = &vec` variable (replaced by the frame pointer), the struct-free copy `*(TI128 *)out = vec`.
   Measured, not usable: an `__asm__("lq $2,0(%1)\n\tsq $2,0(%0)" : : "r"(out), "r"(p) : "$2", "memory")` for the
   second copy (with `TI128 *p = &vec`) makes the whole body byte-identical (tools/check.py: 1 of 1 places match).
   The float constant needs the spelling 0.017453294f (0x3C8EFA36); 0.017453293f encodes 0x3C8EFA35. */
typedef int TI128 __attribute__((mode(TI)));

extern float func_00283248(float, float);

extern float func_002A7878(float, float);

extern float func_00283180(float);

extern float func_00283198(float);

void func_L02_003D5E40(float *out, TI128 *src, float dist, float ang_jit, float dist_jit, float z_jit) {
    TI128 vec = *src;
    TI128 *p = &vec;
    float *v = (float *)&vec;
    float ang;

    ang = func_00283248(out[0] - v[0], out[1] - v[1]);
    ang_jit *= 0.017453294f;
    ang += func_002A7878(-ang_jit, ang_jit);
    dist += func_002A7878(-dist_jit, dist_jit);
    *(TI128 *)out = *p;
    out[0] += dist * func_00283180(ang);
    out[1] += dist * func_00283198(ang);
    out[2] = v[2] + func_002A7878(-z_jit, z_jit);
}
