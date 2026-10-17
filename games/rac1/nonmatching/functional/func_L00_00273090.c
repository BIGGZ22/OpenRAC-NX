/* functional: medium. Corrected from nonmatching/shared/func_L00_00273090.c: the near miss declares none of its callees, so func_001FA888 (int to float, result in $f0), func_001FA898 (float to int, argument in $f12) and the others fall back to implicit int, which makes the life/ticks division integer and passes the alpha as a double; the body matches the assembly otherwise. Prototypes added. */
extern float func_001FA888(int);
extern int func_001FA898(float);
extern void func_001F9BD8(void *, void *, void *);
extern int func_001F9938(void *);
extern void func_L00_002688A8(void *);
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
        t = func_001FA888(q->life - 6);
        t = (t == 0.0f) ? 0.0f : *(short *)(m + 0xA) / t; /* retail's divide by zero gives the largest float; 0 keeps the host finite for the life == 6 case */
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
