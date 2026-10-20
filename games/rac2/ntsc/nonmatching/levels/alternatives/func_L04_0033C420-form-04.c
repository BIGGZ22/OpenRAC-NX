/* Alternative source form for func_L04_0033C420.
   Retained from w18.c, entry z1; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
typedef unsigned char u8;
typedef short s16;
typedef int s32;

extern void *memcpy(void *, const void *, unsigned int);
extern int func_002761F8(void *a, void *b, int mode, void *self, int flag);
extern u8 D_L04_001AFAC0[];

int func_L04_0033C420(void *a0, void *a1, s32 a2, float f12, float f13) {
    s32 v1[4];
    s32 v2[4];
    u8 *base = D_L04_001AFAC0;
    s32 r;

    memcpy(v1, a0, 16);
    memcpy(v2, a0, 16);
    *(float *)(v1 + 2) += f12;
    *(float *)(v2 + 2) -= f13;
    if (*(float *)(v2 + 2) < 0.0f)
        *(float *)(v2 + 2) = 0.0f;
    r = func_002761F8(v1, v2, 0x22, a1, 0);
    if (*(s32 *)(base + 0x1C) <= -1)
        r = 0;
    if (a2 != 0) {
        if (*(s32 *)(base + 0x18) != 0)
            r = 0;
    }
    return r;
}
