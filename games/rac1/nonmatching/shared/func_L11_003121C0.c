/* NON_MATCHING func_L11_003121C0 -- src/overlays/shared/vendor_002C99E0.c
 * Best so far: SIZE ours 852 / retail 848, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Shared DMA-packet builder (levels 11, 13, 17): float args f12..f16 then eight ints, four FA898 table lookups f
 *   Note: the vendor file declares func_L11_003121C0 later with float-first arguments and D_0013E15A as unsigned c
 *   Unblock: the allocator's choice of homes for the two arguments, and the position of the extra lui/lw pair near
 */
extern float func_001F9FA8(float);
extern float func_001F9F90(float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9BF0(void *, void *, void *);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern unsigned char D_0013E15A[];
extern char *D_L11_00161240_m __asm__("D_L11_00161240_m") MACRO_ADDR;

/* Builds one GIF/DMA packet for a level-11 effect and advances the packet pointer. */
void func_L11_003121C0(float fa, float fb, float fc, float fd, float fe, int a0, int a1, int a2, int a3,
                       int a4, int a5, int a6, int a7) {
    float s[4];
    float t[4];
    float v[4];
    float w[4];
    float x[4];
    float y[4];
    float z[4];
    u64 v19;
    u64 v20;
    u64 v21;
    u64 v22;
    u64 v4;
    u64 v7;
    u64 v8;
    long v6;
    int v5;
    long v23;
    char *old;
    char *n;
    unsigned char *tb;
    float k;
    int r1;
    int r2;

    v19 = a4 & 0xFF;
    v20 = a5 & 0xFF;
    v21 = a6 & 0xFF;
    v22 = a7 & 0xFF;

    v[0] = fa;
    v[1] = fb;
    s[0] = fd * func_001F9FA8(fa);
    s[1] = fd * func_001F9F90(fe);
    t[0] = fc * func_001F9F90(fe);
    t[1] = -fc * func_001F9FA8(fe);
    func_001F9BD8(w, v, s);
    func_001F9BF0(w, w, t);
    func_001F9BD8(x, v, s);
    func_001F9BF0(y, v, t);
    qcopy(z, v);

    *(int *)D_L11_00161240_m = 0x10000007;
    v20 = v20 << 8;
    v19 = v19 | v20;
    v5 = 0x50000007;
    v7 = ((u64)0xB400 << 48) | 0x8001;
    v4 = ((((u64)0xA6A6 << 16) | 0xA6A6) << 11) | 0x106;
    *(int *)(D_L11_00161240_m + 4) = 0;
    v21 = v21 << 16;
    v19 = v19 | v21;
    v22 = v22 << 24;
    v19 = v19 | v22;
    v6 = 0x154;
    v8 = 0x100010;
    *(int *)(D_L11_00161240_m + 8) = 0;
    k = 16.0f;
    v21 = a0 << 4;
    v23 = a1 << 20;
    v20 = (long)a3 << 32;
    *(int *)(D_L11_00161240_m + 0xC) = v5;
    old = D_L11_00161240_m;
    n = old + 0x10;
    D_L11_00161240_m = n;
    *(u64 *)(old + 0x10) = v7;
    *(u64 *)(n + 0x8) = v4;
    *(u64 *)(n + 0x18) = v6;
    *(u64 *)(n + 0x28) = v8;
    *(u64 *)(n + 0x20) = v19;
    *(u64 *)(n + 0x10) = (long)a2;

    tb = D_0013E15A + 0x4A6;
    r1 = func_001FA898_r(w[0] * k);
    r2 = func_001FA898_r(w[1] * k);
    *(u64 *)(n + 0x38) = v21;
    *(u64 *)(n + 0x30) = ((long)(r2 + *(int *)(tb + 0x14) - 8) << 16) | (long)(r1 + *(int *)(tb + 0x10) - 8) | v20;

    r1 = func_001FA898_r(x[0] * k);
    r2 = func_001FA898_r(x[1] * k);
    *(u64 *)(n + 0x48) = v23;
    *(u64 *)(n + 0x40) = ((long)(r2 + *(int *)(tb + 0x14) - 8) << 16) | (long)(r1 + *(int *)(tb + 0x10) - 8) | v20;

    r1 = func_001FA898_r(y[0] * k);
    r2 = func_001FA898_r(y[1] * k);
    v23 = v23 + v21;
    *(u64 *)(n + 0x58) = v23;
    *(u64 *)(n + 0x50) = ((long)(r2 + *(int *)(tb + 0x14) - 8) << 16) | (long)(r1 + *(int *)(tb + 0x10) - 8) | v20;

    r1 = func_001FA898_r(z[0] * k);
    r2 = func_001FA898_r(z[1] * k);
    *(u64 *)(n + 0x68) = 0;
    *(u64 *)(n + 0x60) = ((long)(r2 + *(int *)(tb + 0x14) - 8) << 16) | (long)(r1 + *(int *)(tb + 0x10) - 8) | v20;

    D_L11_00161240_m = D_L11_00161240_m + 0x70;
}
