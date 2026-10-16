/* NON_MATCHING func_L00_00273090 -- src/overlays/shared/partupd_00272158.c
 * Best so far: SIZE ours 572 / retail 576, checked 2026-10-16.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Type 53 particle (spawned by func_L00_00272F00): data at +0x20 as a struct (sizes, life, spin, alpha, vel, fal
 *   Alpha is a lerp. Written ((a >> 1) - a) * t + a, CSE leaves retail's first daddu copy (568 -> 572).
 *   Size 0 is read through m + 0x20 as retail does, not through the struct pointer.
 *   Left: retail copies the truncate_float_to_s32 result to $3 before the shift and loads the mask into $2.
 *   Tried for it: setter inline (int, u8, float param), u8/unsigned locals, mask first, & 0xFF. All 572.
 */
typedef struct {
    float size0;
    float size1;
    short life;
    unsigned char spin;
    unsigned char alpha;
    float vel[3];
    float fall;
} Part53_273090;

/* type 53 particle: spins, grows from size1 toward the middle of size0 and size1 while its alpha
   halves over the first five ticks, then shrinks back toward size0 and fades out; drifts and
   falls, and dies outside 2..1021 or when its timer runs out */
void func_L00_00273090(char *m) {
    Part53_273090 *q = (Part53_273090 *)(m + 0x20);
    int age;
    float t;
    float a;
    float *w;
    float v[4];

    m[8] += q->spin;
    age = q->life - *(short *)(m + 0xA);
    if (age < 6) {
        t = age / 5.0f;
        *(float *)(m + 0xC) = q->size1 + ((*(float *)(m + 0x20) + q->size1) * 0.5f - q->size1) * t;
        a = ((q->alpha >> 1) - q->alpha) * t + q->alpha;
    } else {
        t = *(short *)(m + 0xA) / func_001FA888(q->life - 6);
        *(float *)(m + 0xC) = *(float *)(m + 0x20) + ((*(float *)(m + 0x20) + q->size1) * 0.5f - *(float *)(m + 0x20)) * t;
        a = (q->alpha >> 1) * t + 0.0f;
    }
    *(int *)(m + 4) = (truncate_float_to_s32(a) << 24) | (*(int *)(m + 4) & 0xFFFFFF);
    w = q->vel;
    v[0] = w[0];
    v[1] = w[1];
    v[3] = 0.0f;
    v[2] = w[2];
    FastVecAdd(m + 0x10, m + 0x10, v);
    q->vel[2] -= q->fall;
    if (*(float *)(m + 0x10) < 2.0f || *(float *)(m + 0x10) > 1021.0f ||
        *(float *)(m + 0x14) < 2.0f || *(float *)(m + 0x14) > 1021.0f ||
        *(float *)(m + 0x18) < 2.0f || *(float *)(m + 0x18) > 1021.0f) {
        KillPart(m);
    } else if (FastDecTimer(m + 0xA)) {
        KillPart(m);
    }
}
