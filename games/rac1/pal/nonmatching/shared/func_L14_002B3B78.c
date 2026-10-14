/* NON_MATCHING func_L14_002B3B78 -- src/overlays/shared/vendor_002B2A28.c
 * Best so far: SIZE ours 1412 / retail 1452, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Five-block GIF packet builder: each block copies a 16-byte matrix row from data+0x1E0/0x200/0x1F0/0x210/0x220 
 *   Would unblock it: a way to write the data-relative row copies so the compiler emits retail's addiu-then-lq pai
 */
typedef int u128_2B3B78 __attribute__((mode(TI)));
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9CA0(void *, void *, void *);
extern void func_001F9C30(void *, void *, float);
extern void func_001F9EE8(void *, void *, void *);
extern void func_L00_001FD1D8(void *, void *, int);
extern void func_001F9BD8(void *, void *, void *);
extern char D_L14_001675C0[];
extern char D_L14_001D8B50[];
extern char D_0013F6E0[];
extern short D_L14_00161574;
extern short D_L14_00161584;
extern short D_L14_00161588;

// Builds one GIF packet from the moby's matrix block at data+0x1E0..0x220 and runs the four row loops.
void func_L14_002B3B78(char *moby) {
    char *data = *(char **)(moby + 0x78);
    struct {
        float o[16];
        float g[12];
        u64 t[4];
    } pk;
    float m[16];
    float w[4];
    char *pp;
    char *q;
    char *r;
    char *src;
    int k;
    float *op, *mp;

    src = data + 0x1E0;
    *(u128_2B3B78 *)(m + 12) = *(u128_2B3B78 *)src;
    m[15] = 1.0f;
    func_001F9BF0(m, D_L14_001675C0, m + 12);
    func_L00_001FF4B0(m, m, 1.0f);
    func_001F9CA0(m + 4, m, D_0013F6E0);
    func_L00_001FF4B0(m + 4, m + 4, -1.0f);
    func_001F9CA0(m + 8, m + 4, m);
    pk.t[1] = func_001F4868(11);
    pk.t[3] = 0x8000000048ULL;
    pk.t[2] = 0xFF9000000260ULL;
    pk.t[0] = 5;
    *(unsigned int *)&pk.g[0] = 0x80FFFFFF;
    *(unsigned int *)&pk.g[1] = 0x80FFFFFF;
    *(unsigned int *)&pk.g[2] = 0x80FFFFFF;
    *(unsigned int *)&pk.g[3] = 0x80FFFFFF;
    pk.g[4] = 0.0f;
    pk.g[5] = 0.0f;
    pk.g[6] = 0.0f;
    pk.g[7] = 1.0f;
    pk.g[8] = 1.0f;
    pk.g[9] = 0.0f;
    pk.g[10] = 1.0f;
    pk.g[11] = 1.0f;
    op = pk.o;
    mp = m;
    k = 3;
    do {
        func_001F9C30(op, mp, 0.4f);
        mp += 4;
        k--;
        func_001F9EE8(op, op, m);
        op += 4;
    } while (k >= 0);
    func_L00_001FD1D8(pk.o, 0, 0);

    src = data + 0x200;
    *(u128_2B3B78 *)(m + 12) = *(u128_2B3B78 *)src;
    pp = data + 0x220;
    func_001F9BF0(m, D_L14_001675C0, m + 12);
    q = data + 0x1F0;
    r = data + 0x210;
    func_L00_001FF4B0(m, m, 1.0f);
    k = 3;
    func_001F9CA0(m + 4, m, D_0013F6E0);
    func_L00_001FF4B0(m + 4, m + 4, -1.0f);
    func_001F9CA0(m + 8, m + 4, m);
    *(unsigned int *)&pk.g[0] = 0x202020FF;
    *(unsigned int *)&pk.g[1] = 0x202020FF;
    *(unsigned int *)&pk.g[2] = 0x202020FF;
    *(unsigned int *)&pk.g[3] = 0x202020FF;
    op = pk.o;
    mp = (float *)D_L14_001D8B50;
    do {
        func_001F9C30(op, mp, 0.2f);
        mp += 4;
        k--;
        func_001F9EE8(op, op, m);
        op += 4;
    } while (k >= 0);
    func_L00_001FD1D8(pk.o, 0, 0);

    *(u128_2B3B78 *)(m + 12) = *(u128_2B3B78 *)q;
    func_001F9BF0(m, D_L14_001675C0, m + 12);
    func_L00_001FF4B0(m, m, 1.0f);
    k = 3;
    func_001F9CA0(m + 4, m, D_0013F6E0);
    func_L00_001FF4B0(m + 4, m + 4, -1.0f);
    func_001F9CA0(m + 8, m + 4, m);
    func_001F9C30(w, m, *(float *)&D_L14_00161574);
    func_001F9BD8(m + 12, m + 12, w);
    *(unsigned int *)&pk.g[0] = 0x4020FF20;
    *(unsigned int *)&pk.g[1] = 0x4020FF20;
    *(unsigned int *)&pk.g[2] = 0x4020FF20;
    *(unsigned int *)&pk.g[3] = 0x4020FF20;
    op = pk.o;
    mp = (float *)D_L14_001D8B50;
    do {
        func_001F9C30(op, mp, 0.2f);
        mp += 4;
        k--;
        func_001F9EE8(op, op, m);
        op += 4;
    } while (k >= 0);
    func_L00_001FD1D8(pk.o, 0, 0);

    *(u128_2B3B78 *)(m + 12) = *(u128_2B3B78 *)r;
    func_001F9BF0(m, D_L14_001675C0, m + 12);
    func_L00_001FF4B0(m, m, 1.0f);
    k = 3;
    func_001F9CA0(m + 4, m, D_0013F6E0);
    func_L00_001FF4B0(m + 4, m + 4, -1.0f);
    func_001F9CA0(m + 8, m + 4, m);
    func_001F9C30(w, m, *(float *)&D_L14_00161584);
    func_001F9BD8(m + 12, m + 12, w);
    *(unsigned int *)&pk.g[0] = 0x40FF2020;
    *(unsigned int *)&pk.g[1] = 0x40FF2020;
    *(unsigned int *)&pk.g[2] = 0x40FF2020;
    *(unsigned int *)&pk.g[3] = 0x40FF2020;
    op = pk.o;
    mp = (float *)D_L14_001D8B50;
    do {
        func_001F9C30(op, mp, *(float *)&D_L14_00161588);
        mp += 4;
        k--;
        func_001F9EE8(op, op, m);
        op += 4;
    } while (k >= 0);
    func_L00_001FD1D8(pk.o, 0, 0);

    *(u128_2B3B78 *)(m + 12) = *(u128_2B3B78 *)pp;
    func_001F9BF0(m, D_L14_001675C0, m + 12);
    func_L00_001FF4B0(m, m, 1.0f);
    k = 3;
    func_001F9CA0(m + 4, m, D_0013F6E0);
    func_L00_001FF4B0(m + 4, m + 4, -1.0f);
    func_001F9CA0(m + 8, m + 4, m);
    *(unsigned int *)&pk.g[0] = 0x40FF2020;
    *(unsigned int *)&pk.g[1] = 0x40FF2020;
    *(unsigned int *)&pk.g[2] = 0x40FF2020;
    *(unsigned int *)&pk.g[3] = 0x40FF2020;
    op = pk.o;
    mp = (float *)D_L14_001D8B50;
    do {
        func_001F9C30(op, mp, *(float *)&D_L14_00161588);
        mp += 4;
        k--;
        func_001F9EE8(op, op, m);
        op += 4;
    } while (k >= 0);
    func_L00_001FD1D8(pk.o, 0, 0);
}
