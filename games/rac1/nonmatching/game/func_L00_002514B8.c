/* Moby animation advance: steps the moby's keyframe time (m+0x54) by its
   speed pair (m+0x58 / m+0x5C) through the sequence in its class table,
   leaving the playback indices at m+0x50/0x51 and the flags at m+0x70.
   Written as the retail state machine: each jump of the assembly is a goto,
   including the paths that retail reaches from the next functions. */
extern int D_0015F6B0 MACRO_ADDR;
extern void func_0020D790(unsigned char *s);
extern int func_0022ED80(int, int, char *);

void func_L00_002514B8(void *a0)
{
    union { float f; s32 i; } U;
    char *m = (char *)a0;
    char *S = 0, *p5 = 0, *p6 = 0, *p7 = 0;
    char *pc;
    s32 r1, r2, r3, r4, r5, r6, r8, r9, r10, r11, r12, r13, r14, r15;
    float f1, f2, f3, f4, f12;
    s64 a, b, c, e;

    r15 = 0;
    pc = *(char **)(m + 0x24);
    r10 = *(u8 *)(m + 0x52);
    r11 = *(u8 *)(m + 0x53);
    f1 = *(float *)(m + 0x54);
    r14 = *(s32 *)(m + 0x7C) & 0xFFFFFF;
    r13 = 0xFFFF;
    r1 = 0x3F800000;
    r2 = *(s32 *)(m + 0x58);
    r3 = *(s32 *)(m + 0x5C);
    f2 = *(float *)(m + 0x58);
    if (r3 == 0)
        goto L758;
    f3 = *(float *)(m + 0x5C);
    if (r10 != r11)
        goto L570;

    f1 = f1 + f2 * f3;
    r3 = 0x3F800000;
    r4 = 0x3F7F0000;
    goto L510;

L570:
    f1 = f1 + f3;
    r3 = 0x3F800000;
    r4 = 0x3F7F0000;

L510:
    r8 = *(u8 *)(m + 0x50);
    if (r2 == 0)
        goto L758;
    r9 = *(u8 *)(m + 0x51);
    f12 = *(float *)(m + 0x54);
    r12 = r8;
    U.f = f1;
    r2 = U.i;
    r3 |= 0x8000;
    U.i = r1;
    f4 = U.f;
    r5 = r11 << 2;
    a = (s64)r3 - (s64)r2;
    b = (s64)r2 - (s64)r4;
    c = a | b;
    p7 = pc + r5;
    e = (s64)r2 - (s64)r1;
    if (c > 0)
        goto L580;
    S = *(char **)(p7 + 0x48);
    if (e > 0)
        goto L588;
    if (r2 < 0)
        goto L640;
    *(float *)(m + 0x54) = f1;
    *(u8 *)(m + 0x70) = r15;
    if (r13 != r14)
        goto L16C8;
    return;

L580:
    U.i = r1;
    f1 = U.f;
    S = *(char **)(p7 + 0x48);

L588:
    f1 = f1 - f4;
    r8 = r9;
    r9 = r9 + 1;
    p5 = *(char **)(m + 0x6C);
    if (r10 != r11)
        goto L628;

L5A0:
    f1 = f1 / f3;
    r15 |= 1;
    r3 = *(u8 *)(S + 0x10);
    p6 = S + (r9 << 2);
    r4 = *(s32 *)(S + 0x18);
    r3 = r3 - r9;
    p6 = *(char **)(p6 + 0x1C);
    U.i = r4;
    f3 = U.f;
    if (r3 > 0)
        goto L5D8;
    r9 = 0;
    r15 |= 2;
    p6 = *(char **)(S + 0x1C);

L5D8:
    if (r4 != 0)
        goto L5E8;
    f3 = *(float *)p5;

L5E8:
    f1 = f1 * f3;
    *(u8 *)(m + 0x52) = r10;
    *(u8 *)(m + 0x50) = r8;
    *(u8 *)(m + 0x51) = r9;
    *(char **)(m + 0x68) = p5;
    *(char **)(m + 0x6C) = p6;
    U.f = f1;
    r2 = U.i;
    r1 = 0x3F800000;
    *(float *)(m + 0x5C) = f3;
    a = (s64)r2 - (s64)r1;
    *(float *)(m + 0x54) = f1;
    if (a > 0)
        goto L588;
    *(u8 *)(m + 0x70) = r15;
    if (r13 != r14)
        goto L16C8;
    return;

L628:
    r1 = *(u8 *)(S + 0x12);
    r10 = r11;
    r14 = r13;
    *(u8 *)(m + 0x7E) = r1;
    goto L5A0;

L640:
    f1 = f1 / f3;
    r9 = r8;
    r8 = r8 - 1;
    p6 = *(char **)(m + 0x68);
    r15 |= 1;
    r4 = *(s32 *)(S + 0x18);
    if (r8 < 0) {
        r8 = *(u8 *)(S + 0x10);
        r15 |= 2;
        r8 = r8 - 1;
    }

    p5 = S + (r8 << 2);
    U.i = r4;
    f3 = U.f;
    p5 = *(char **)(p5 + 0x1C);
    if (r4 == 0)
        f3 = *(float *)p5;

    f1 = f1 * f3;
    *(char **)(m + 0x68) = p5;
    *(u8 *)(m + 0x50) = r8;
    *(u8 *)(m + 0x51) = r9;
    f1 = f1 + f4;
    *(char **)(m + 0x6C) = p6;
    U.f = f1;
    r1 = U.i;
    *(float *)(m + 0x5C) = f3;
    *(float *)(m + 0x54) = f1;
    if (r1 < 0)
        goto L640;
    *(u8 *)(m + 0x70) = r15;
    if (r13 != r14)
        goto L16C8;
    return;

L16C8:
    *(u8 *)(m + 0x70) = r15;
    if (r10 != r11)
        goto L758;
    r6 = r14 >> 16;
    f3 = 16.0f;
    if (r6 <= 0)
        goto L758;
    r1 = r8 << 4;
    r2 = r12 << 4;
    f1 = f1 * f3;
    f2 = f12 * f3;
    r3 = (s32)f1;
    r4 = (s32)f2;
    r5 = *(u8 *)(S + 0x10);
    r1 = r1 + r3;
    r2 = r2 + r4;
    r3 = r1 - r2;
    r5 = r5 << 2;
    p5 = S + r5;
    if (r3 <= 0)
        goto L758;
    r3 = *(u16 *)(p5 + 0x1E);
    r2 = r2 + 1;

L728:
    if (r6 <= 0)
        goto L758;
    r6 = r6 - 1;
    r4 = r3 - r2;
    r3 = r1 - r3;
    r3 = r3 | r4;
    p5 = p5 + 4;
    if (r3 < 0) {
        r3 = *(u16 *)(p5 + 0x1E);
        goto L728;
    }
    r4 = *(u16 *)(p5 + 0x18);
    func_0022ED80(r4, 0, m);
    return;

L758:
    *(u8 *)(m + 0x70) = r15;
    if ((((u32)r14) << 16) == (((u32)r13) << 16))
        return;
    r1 = (r14 >> 8) & 0xFF;
    r2 = 0xFF;
    if (r1 != r2)
        goto L790;
    r3 = D_0015F6B0;
    r2 = ((u32)(s32)m >> 8);
    r3 = r3 & 3;
    r2 = r2 & 3;
    if (r2 != r3)
        return;

L790:
    func_0020D790((unsigned char *)m);
}
