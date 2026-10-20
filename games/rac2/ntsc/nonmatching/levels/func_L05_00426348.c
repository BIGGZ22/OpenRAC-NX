/* func_L05_00426348 (540 bytes, 4 places) -- NEAR MISS, not integrated.
   The same body serves func_L05_004268E8 and func_L05_00426E70; they differ
   from this one only in the two `ori` immediates of the colour call
   (0x20FF here, 0x7FFF and 0x4FFF in the two siblings).

   The body, as read from the retail bytes:
   - the object's own 16-byte record (object + 0x10) is copied into a stack
     buffer (lq/sq pair, 0x78820010 / 0x7FA20020), its third float gets 0.25f
     added, and the buffer is blended with the level's own table by
     func_00282CF0; then func_00282F20 twice (with -0.3f and 0.1f) and
     func_00282CC0.
   - a four-step walk over the four records of the state's own block: the first
     float of each is turned by the float 0x10 further on, wrapped into
     0..255, the record's id is written back as 255 when func_00282948 says the
     record is live, the blended colour is handed to func_00283D38 with the
     level's 0x502020FF/0x102020FF pair, and the level's own spawner
     func_002C2FD8 gets the record.

   Measured: 512 bytes against 540, 0 of 4 places match.  What is left:
   - the frame: the candidate is 0x80 bytes against retail's 0x90, because it
     uses seven callee-saved registers against retail's eight.  Retail keeps
     the state block base ($s4) and the end of the first float array ($s5) in
     registers of their own, because the int array is addressed with the 32-bit
     `addiu $v1,$s4,0x20 ; addu $v0,$v1,$s2`; the candidate strength-reduces
     that address into the float induction variable and gets
     `sw $s5,0x20($s1)` instead, one instruction short, and loses a register.
   - the loop test: retail compares the induction variable with the end of the
     first float array with a *signed* `slt`, and this compiler emits `sltu` for
     a pointer comparison.  Casting both sides to `int` does give the `slt`
     (measured), but then the trip count becomes known to the optimiser and the
     whole loop is rewritten.
   - the 0.0f of the `f <= 0.0f` test lands in $f2 here and in $f0 in retail,
     and retail loads it after the branch that guards it.

   What was measured on the way: the 128-bit copy needs a `mode(TI)` local
   (three of them in one array, so the frame is laid out 0x00/0x10/0x20 and the
   copy's `sq` carries the 0x20 offset); the level's own table address has to
   be an absolute `extern` (no sda), retail materialises it with
   `lui $a1,0x1C ; addiu $a1,$a1,-0x6940`; the second call's constant is
   -0.3f (0xBE99999A), not -0.2f; and func_00283CE0 returns a float.
*/
typedef int TI128 __attribute__((mode(TI)));

extern int ref_L05_00426348_00[];

extern void func_00282CF0(void *, void *, void *);

extern void func_00282F20(void *, void *, float);

extern void func_00282CC0(char *dst, char *src, char *src2);

extern int func_00282948(void *);

extern float func_00283CE0(int);

extern int func_00283CF0(float);

extern float func_00282C48(float);

extern void func_00283D38(int, int, float);

extern void *func_002C2FD8(void *, void *, int, int, int, int, int, float);

void func_L05_00426348(char *a0)
{
    TI128 buf[3];
    char *base;
    char *r;
    int i;
    float f;
    int n;
    int m;
    void *ptr;

    buf[2] = *(TI128 *)(a0 + 0x10);
    base = *(char **)(a0 + 0x68);
    r = base + 0x10;
    ((float *)&buf[2])[2] += 0.25f;
    func_00282CF0(&buf[0], ref_L05_00426348_00, &buf[2]);
    func_00282F20(&buf[0], &buf[0], -0.3f);
    func_00282F20(&buf[1], &buf[0], 0.1f);
    func_00282CC0((char *)&buf[0], (char *)&buf[0], (char *)&buf[2]);
    i = 0;
    do {
        f = *(float *)(base + i);
        f += *(float *)(r + i);
        *(float *)(base + i) = f;
        if (f >= 255.0f)
            *(float *)(base + i) = f - 255.0f;
        else if (f <= 0.0f)
            *(float *)(base + i) = f + 255.0f;
        if (func_00282948((int *)(base + 0x20 + i)) != 0)
            *(int *)((int)base + 0x20 + i) = 255;
        f = func_00283CE0(255 - *(int *)((int)base + 0x20 + i));
        f = f * (1.0f / 255.0f);
        func_00283D38(0x502020FF, 0x102020FF, func_00282C48(0.5f - f));
        n = func_00283CF0(*(float *)(base + i));
        ptr = func_002C2FD8(&buf[0], (void *)n, n & 0xFF, 0x35, 1, 2, 0,
                            *(float *)(base + 0x30 + i));
        if (ptr != 0) {
            m = func_00283CF0(1.0f);
            *(char *)(ptr + 0x9) = (char)(m + 0x60);
        }
        func_00282CC0((char *)&buf[0], (char *)&buf[0], (char *)&buf[1]);
        i += 4;
    } while ((int)(base + i) < (int)r);
}
