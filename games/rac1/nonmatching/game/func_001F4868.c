extern int D_0015F560 MACRO_ADDR;
extern char D_0018D140[];

/* GetEffectTex(idx): the 64-bit texture word of effect slot idx in D_0018D540
   (16-byte slots: +0 the cached word, +8/+A/+C/+E its inputs). The first call
   for a slot builds the word from its inputs and the global D_0015EF74 (which
   it advances), keeps it in the slot, and, while D_0015F558 is below 64, appends
   a 16-byte record to D_0018D140. */
s64 func_001F4868_GetEffectTex(int idx) __asm__("func_001F4868");

s64 func_001F4868_GetEffectTex(int idx)
{
    char *e = (char *)&D_0018D540[idx];
    u64 v;
    int s, h14, m, one, g, g11, count, sum, gnew;
    char *r;

    if (*(s64 *)e != 0) {
        return *(s64 *)e;
    }
    s = *(short *)(e + 0xC);
    h14 = *(short *)(e + 0xE);
    m = s - 6;
    if (m < 0) {
        m = 0;
    }
    g = D_0015EF74;
    one = 1 << (m & 31);
    g11 = ((int)((unsigned)g + 0x400u)) >> 8;
    v = (u64)(s64)g11;
    v |= (u64)(s64)one << 14;
    v |= (u64)(s64)s << 26;
    v |= 0x01300000u;
    v |= (u64)(s64)h14 << 30;
    v |= (u64)(s64)(g >> 8) << 37;
    v |= (u64)1 << 34;
    v |= (u64)1 << 63;
    *(u64 *)e = v;

    sum = s + h14;
    gnew = (int)((unsigned)g + 0x400u + (1u << (sum & 31)));
    D_0015EF74 = gnew;

    count = *(int *)&D_0015F558;
    if (count < 64) {
        unsigned hA = *(unsigned short *)(e + 0xA);
        unsigned h8 = *(unsigned short *)(e + 0x8);

        r = D_0018D140 + count * 16;
        *(int *)(r + 0) = D_0015F560 + (int)(hA << 4);
        *(short *)(r + 6) = (short)(g >> 8);
        *(short *)(r + 4) = 0;
        *(int *)(r + 8) = D_0015F560 + (int)(h8 << 4);
        *(short *)(r + 0xE) = (short)g11;
        *(unsigned char *)(r + 0xC) = *(unsigned char *)(e + 0xC);
        *(unsigned char *)(r + 0xD) = *(unsigned char *)(e + 0xE);
        *(int *)&D_0015F558 = count + 1;
    }
    return (s64)v;
}
