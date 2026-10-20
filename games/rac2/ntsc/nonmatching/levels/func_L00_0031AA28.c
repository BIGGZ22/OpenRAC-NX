/* func_L00_0031AA28 (288 bytes, 27 places) -- NEAR MISS, not integrated.
   state: 288 bytes, the whole instruction sequence matches retail's; the fault is the
   floating-point register allocation.
   check.py (00_aranos_tutorial), with the source below:
     0031AA80  lwc1  $f4, 0x0($a1)    |  lwc1  $f6, 0x0($a1)   (p0.x)
     0031AA84  lwc1  $f2, 0x0($a2)    |  lwc1  $f4, 0x0($a2)   (p1.x)
     0031AA8C  sub.s $f1, $f4, $f5    |  sub.s $f1, $f3, $f6   (p2.x - p0.x)
     0031AA90  mul.s $f3, $f6, $f12   |  mul.s $f2, $f5, $f12  (t*t*t)
     0031AA98  sub.s $f2, $f2, $f4    |  sub.s $f4, $f4, $f3   (p1.x - p2.x)
     0031AA9C  sub.s $f7, $f0, $f1    |  sub.s $f7, $f0, $f1   ((p3.x-p1.x) - (p2.x-p0.x))
   Every opcode, every offset, every branch and the 16-byte `lq`/`sq` copies at the two
   phase tests are retail's; only the FPR numbers differ, and the difference is a
   rotation of five of them:
     retail  f0=p3.x  f1=a  f2=t^3  f3=p2.x  f4=p1.x  f5=t^2  f6=p0.x  f7=c
     this    f0=p3.x  f1=a  f2=p1.x f3=t^3   f4=p2.x  f5=p0.x  f6=t^2   f7=c
   with a = p2.x - p0.x and c = (p3.x - p1.x) - a.  So the compiler agrees on f0, f1 and
   f7 and puts p1.x, t^3, p2.x, p0.x, t^2 in f2..f6 in another order.

   What is measured and right:
   - the formula: out = c*t^3 + (a-c)*t^2 + (p1-p2)*t + p0, summed left to right, with
     c = (p3-p1) - (p2-p0) and a = p2-p0; the fourth component is cleared to 0.0.
   - the 16-byte copies at the two phase tests are `lq`/`sq`, which only this compiler
     reaches through a 128-bit integer type (`typedef int X __attribute__((mode(TI)))`),
     not through a struct assignment (that gives ldl/ldr/sdl/sdr).
   - the copies must be volatile: without it the compiler moves the `sq` into the branch
     delay slot and the two blocks come out two words short.
   - the second phase test is a `bc1fl` whose annulled delay slot holds the load of
     p2.x; this compiler does that by itself when the first test's copy ends in `jr $ra`.
   - the phase parameter is the 6th argument in the float register $f12 and the fourth
     control point is the 5th argument in $t0 (the 5th and 6th integer arguments go to
     $t0 and $t1 in this ABI, measured).
   Tried and not enough: nine spellings of the expression (a, b, c, d named or inline,
     t^2 and t^3 named or inline, the vectors as a struct or as a float array, the
     float argument first in the signature).  They all give the same instruction
     sequence; the five FPRs above move as a block.
   Next idea: a source shape that raises the pressure by one at the point where p1.x is
   read, so that the allocator has to take f2 for t^3 before p1.x. */

typedef int V4TI __attribute__((mode(TI)));

typedef struct {
    float x, y, z, w;
} V4;

/* cubic curve through four control vectors: at a phase of zero the first is copied and
   at one the second, in between the three first components follow the cubic built from
   the four vectors and the fourth component is cleared */
void func_L00_0031AA28(V4 *out, V4 *p0, V4 *p1, V4 *p2, float *p3, float t)
{
    float t2;
    float t3;
    float c;

    if (t == 0.0f) {
        *(volatile V4TI *)out = *(V4TI *)p0;
        return;
    }
    if (t == 1.0f) {
        *(volatile V4TI *)out = *(V4TI *)p1;
        return;
    }
    t2 = t * t;
    t3 = t2 * t;
    c = (p3[0] - p1->x) - (p2->x - p0->x);
    out->x = c * t3 + ((p2->x - p0->x) - c) * t2 + (p1->x - p2->x) * t + p0->x;
    c = (p3[1] - p1->y) - (p2->y - p0->y);
    out->y = c * t3 + ((p2->y - p0->y) - c) * t2 + (p1->y - p2->y) * t + p0->y;
    c = (p3[2] - p1->z) - (p2->z - p0->z);
    out->z = c * t3 + ((p2->z - p0->z) - c) * t2 + (p1->z - p2->z) * t + p0->z;
    out->w = 0.0f;
}
