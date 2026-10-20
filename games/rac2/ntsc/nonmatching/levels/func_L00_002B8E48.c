/* 0 of 27 places match - last line: 00_aranos_tutorial: 164 bytes (plain store) or 172 bytes,
   11 words differ (the shape below). The retail body is understood: a 16-byte vector is copied
   with lq/sq, run through three transform helpers (the hero transform at g_pHeroMoby + 0xF0),
   then out[0] = atan2 of the horizontal run against y and out[1] = the negated atan2 of z
   against x; every call and both float computations land instruction for instruction.
   The eleven words left are the prologue order: retail saves $s1 and $s2, materialises the
   vector address (sp+0x80) and the output pointer, and only then saves $s0 and $ra, while this
   cc1 saves s0, s1, s2 in declaration order before either setup. The quad store needs the
   volatile qualifier (without it this cc1 sinks the store into the first call's delay slot);
   the pointer local is what reproduces the lq/sq pair.
   Checked against src/levels/shared/L00_002B7B80.c (the draft was removed from the file).
   Two 64-byte locals are the transform buffers: first at sp+0, second at sp+0x40, vector at
   sp+0x80, same layout here. */

typedef struct { float x, y, z, w; } Vec4_L00_002B8E48;

/* The 16-byte copy reaches lq/sq only through the 128-bit integer type (the aggregate path
   builds an ldl/ldr pair run). */
typedef int V4_L00_002B8E48 __attribute__((mode(TI)));

extern float func_00282C10(float);

extern float func_00283248(float, float);

extern void func_00283410(void *, void *);

extern void func_00283698(void *, void *);

extern void func_002830C0(void *, void *, void *);

extern void *g_pHeroMoby;

void func_L00_002B8E48(Vec4_L00_002B8E48 *in, float *out)
{
    Vec4_L00_002B8E48 first[4];
    Vec4_L00_002B8E48 second[4];
    Vec4_L00_002B8E48 vector;
    Vec4_L00_002B8E48 *vp = &vector;

    *(volatile V4_L00_002B8E48 *)vp = *(V4_L00_002B8E48 *)in;
    func_00283410(first, (char *)g_pHeroMoby + 0xF0);
    func_00283698(second, first);
    func_002830C0(vp, vp, second);
    out[0] = func_00283248(func_00282C10(vector.x * vector.x + vector.z * vector.z), vector.y);
    out[1] = -func_00283248(vector.z, vector.x);
}
