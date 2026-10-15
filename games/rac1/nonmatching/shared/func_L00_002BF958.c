/* NON_MATCHING func_L00_002BF958 -- src/overlays/shared/vendor_002BA7C8.c
 * Best so far: SIZE ours 568 / retail 576, checked 2026-10-05.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * No longer builds in its file (COMPILE failed, 2026-10-09): match its declarations to the file's first.
 * What the last attempts found:
 *   Draws up to three billboard quads (QuadPkt from func_L00_002ADE90): per active slot i, builds an orthogonal fr
 *   Best: p5.c (ours 568 bytes, retail 576). Whole body matches structurally; the only difference is loop-top sche
 *   Wordings tried with identical bytes: continue vs if-wrapped body, array index vs char* offset, i++ vs explicit
 *   w01 round: the candidate must reach D_L00_001DC220 as `extern float D_L00_001DC220_c[][4] __asm__("D_L00_001DC
 */
typedef struct {
    float v[4][4];
    int rgba[4];
    float uv[4][2];
    long gs[4];
} QuadPkt;
typedef struct { float x, y, z, w; } Vq __attribute__((aligned(16)));
extern char D_0013E633[] NOT_SDA;
extern int func_001F4868(int);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern void func_001F9C30(void *, void *, float);
extern void func_001F9CA0(void *, void *, void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9EE8(void *, void *, void *);
extern void func_L00_001FD1D8(void *, void *, int);
extern int D_L00_001618E8[];
extern float D_L00_001618B8[];
extern float D_L00_001618D8[];
extern float D_L00_001DD3A0[][4];
extern float D_L00_001DD3D0[][4];
extern float D_L00_001DC220_c[][4] __asm__("D_L00_001DC220");
extern short D_L00_00161834;
extern short D_L00_00161830;

/* Draws up to three billboard quads, each a scaled corner template placed around a point and oriented to the camera. */
void func_L00_002BF958(void) {
    QuadPkt q;
    Vq A, B, C, D, E;
    int base;
    int i;
    int j;
    int k;
    Vq *pd = &D;
    q.gs[1] = func_001F4868(8);
    q.gs[2] = 0xFF9000000260;
    q.gs[3] = 0x8000000048;
    q.gs[0] = 0;
    q.uv[0][0] = 0; q.uv[0][1] = 0;
    q.uv[1][0] = 0; q.uv[1][1] = 1.0f;
    q.uv[2][0] = 1.0f; q.uv[2][1] = 0;
    q.uv[3][0] = 1.0f; q.uv[3][1] = 1.0f;
    base = (((unsigned char)D_0013E633[2] != 0) ? *(int *)&D_L00_00161834 : *(int *)&D_L00_00161830) & 0xFFFFFF;
    for (i = 0; i < 3; i = k) {
        int col;
        k = i + 1;
        if (D_L00_001618E8[i] != 0) {
            col = base | (func_001FA898_r(D_L00_001618B8[i]) << 24);
            q.rgba[0] = col;
            q.rgba[3] = col;
            q.rgba[2] = col;
            q.rgba[1] = col;
            qcopy(pd, D_L00_001DD3A0[i]);
            qcopy(&A, D_L00_001DD3D0[i]);
            func_001F9C30(&C, D_0013E633 + 0x10AD, -1.0f);
            func_001F9CA0(&B, &A, &C);
            func_L00_001FF4B0(&B, &B, 1.0f);
            func_001F9CA0(&C, &B, &A);
            for (j = 0; j < 4; j++) {
                func_001F9C30(&E, D_L00_001DC220_c[j], D_L00_001618D8[i]);
                func_001F9EE8(&q.v[j], &E, &A);
            }
            func_L00_001FD1D8(&q, 0, 0);
        }
    }
}
