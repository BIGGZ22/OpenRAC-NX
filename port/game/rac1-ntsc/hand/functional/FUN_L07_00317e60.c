/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/l07_umbris/func_L07_00319230.c (games/rac1): func_L07_00319230, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* carry, not from a `same` function or an `exact` data address (read them):
 *   0x00404040: no place found, left as it is
 *   D_L07_00161C10 -> 00161B80: between
 *   D_L07_00161C14 -> 00161B84: between
 */

/* The quadword helpers in plain C: hostgen maps only the decompilation's own qcopy, qcopy_nc and
 * qzero to the runtime's, and wrote these renamed copies' inline assembly as nothing (every
 * copy through them was lost). Four words in order, as the lq/sq pair moves them. */
static __inline__ void qcopy_c07_317e60(void *dst, void *src) {
    int *d_ = (int *)dst, *s_ = (int *)src;
    d_[0] = s_[0]; d_[1] = s_[1]; d_[2] = s_[2]; d_[3] = s_[3];
}
extern float func_001FA6C0(int);
extern int func_001FA6D0(float);
extern int func_001F9688(int);
extern float func_001F9DE0(float);
extern void func_001F9A28(void *, void *, void *);
extern float func_002132A8(float, float);
extern void func_001F9A28(void *, void *, void *);
extern void func_001F9A68(void *, void *, float);
extern float func_001FA5C8(float, float);
extern float func_001F9AF0(void *);
extern float func_001F99C0(float);
extern int func_001FA6D0(float);
extern void func_001F4600(void (*)(), void *);
extern int func_00213260(int);
extern float func_001F9DE0(float);
extern float func_001F9AB0(void *, void *);
extern float func_001FA6C0(int);
extern float func_001FA580(float, float);
extern int func_001F9688(int);
extern float D_0015ED60 ;
typedef int u128_319080_c07_317e60 __attribute__((mode(TI)));
typedef float RopeVec_319080_c07_317e60[4] __attribute__((aligned(16)));
extern int D_L07_00161C10_319080_c07_317e60 __asm__("D_L07_00161B80" "__gp") ;
extern float D_L07_00161C40_319080_c07_317e60 __asm__("D_L07_00161BB0") ;
extern char D_L07_00161C20_319080_c07_317e60[] __asm__("D_L07_00161B90") ;
extern char D_L07_00161C30_319080_c07_317e60[] __asm__("D_L07_00161BA0") ;
extern RopeVec_319080_c07_317e60 D_L07_0020DDB0_319080_c07_317e60[] __asm__("D_L07_0020DAF0");
extern RopeVec_319080_c07_317e60 D_L07_0020DDC0_319080_c07_317e60[] __asm__("D_L07_0020DB00");
extern short D_L07_0020DE88_319080_c07_317e60[] __asm__("D_L07_0020DBC8");
extern void norm_319080_c07_317e60(void *, void *, float) __asm__("func_L00_001FF500");
extern void add_319080_c07_317e60(void *, void *, void *) __asm__("func_001F9A10");
extern void clear_319080_c07_317e60(void *) __asm__("func_001F99F8");
extern void look_319080_c07_317e60(void *, void *, void *, float) __asm__("func_00214890");

void func_L07_00317CB0(char *moby, void *anchor);
extern void func_001F9A28(void *, void *, void *);
extern int func_001F9688(int);
typedef struct {
    short idx;
    short idx2;
    float f4;
    float f8;
    float fC;
    float f10;
    char *moby;
} RopePart_319230_c07_317e60;
typedef struct {
    char pad0[4];
    int mode;
    char pad8[2];
    unsigned char tint;
    char fade;
    char padC[4];
    float vel[4];
} RopeTail_319230_c07_317e60;
typedef struct {
    char pad0[0xA];
    short life;
    char padC[0x14];
    RopeTail_319230_c07_317e60 tail;
} RopePuff_319230_c07_317e60;
extern RopePart_319230_c07_317e60 D_L07_0020CB00_319230_c07_317e60[] __asm__("D_L07_0020C840");
extern RopeVec_319080_c07_317e60 D_L07_0020DDA0_319230_c07_317e60[] __asm__("D_L07_0020DAE0");
extern RopeVec_319080_c07_317e60 D_L07_0020DEA0_319230_c07_317e60[][20] __asm__("D_L07_0020DBE0");
extern RopeVec_319080_c07_317e60 D_L07_0020DD60_319230_c07_317e60[][20] __asm__("D_L07_0020DAA0");
extern RopeVec_319080_c07_317e60 D_L07_0020C9C0_319230_c07_317e60[] __asm__("D_L07_0020C700");
extern float D_L07_0020DE60_319230_c07_317e60[] __asm__("D_L07_0020DBA0");
extern float D_L07_00161C14_319230_c07_317e60 __asm__("D_L07_00161B84" "__gp") ;
extern char D_0013F4D0_319230_c07_317e60[] __asm__("D_0013F3D0");
extern void cross_319230_c07_317e60(void *, void *, void *) __asm__("func_001F9AD8");
extern void rot_319230_c07_317e60(void *, void *, void *) __asm__("func_001F9CF8");
extern float acos_319230_c07_317e60(float) __asm__("func_001F9DF8");
extern float smooth_319230_c07_317e60(void *, float, float, float, float, float) __asm__("func_L00_001EB328");
extern void madd_319230_c07_317e60(void *, void *, void *, float) __asm__("func_001F9A40");
extern float seg_319230_c07_317e60(char *, char *, char *, char *, char *, float) __asm__("func_L07_0029E6D8");
extern RopePuff_319230_c07_317e60 *spawn_319230_c07_317e60(void *, float, float, float, int, void *, float, int) __asm__("func_L00_0026D000");
extern void kill_319230_c07_317e60(void *) __asm__("func_L00_00267A08");
extern void lerp_319230_c07_317e60(void *, void *, void *, float) __asm__("func_L00_0025B6B8");
extern void func_L07_00317910();
float FUN_L07_00317e60(void *moby, void *anchor) {
    RopeVec_319080_c07_317e60 v00;
    RopeVec_319080_c07_317e60 v10;
    RopeVec_319080_c07_317e60 v20;
    RopeVec_319080_c07_317e60 v30;
    RopeVec_319080_c07_317e60 v40;
    RopeVec_319080_c07_317e60 v50;
    RopeVec_319080_c07_317e60 v60;
    RopeVec_319080_c07_317e60 v70;
    RopeVec_319080_c07_317e60 v80;
    RopeVec_319080_c07_317e60 v90;
    RopeVec_319080_c07_317e60 vA0;
    RopeVec_319080_c07_317e60 vB0;
    RopeVec_319080_c07_317e60 vC0;
    RopeVec_319080_c07_317e60 vD0;
    RopeVec_319080_c07_317e60 vE0;
    RopeVec_319080_c07_317e60 vF0;
    RopeVec_319080_c07_317e60 v100;
    RopeVec_319080_c07_317e60 v110;
    RopeVec_319080_c07_317e60 v120;
    RopeVec_319080_c07_317e60 v130;
    RopeVec_319080_c07_317e60 v140;
    float best;
    best = 1000.0f;
    func_L07_00317CB0(moby, anchor);
    qcopy_c07_317e60(v00, D_0013F4D0_319230_c07_317e60);
    v00[2] += 0.6f;
    D_L07_0020DE88_319080_c07_317e60[0] += 4;
    {
        int n;
        for (n = 1; n < 10; n++) {
            D_L07_0020DE88_319080_c07_317e60[n] = D_L07_0020DE88_319080_c07_317e60[n - 1] - 0x20;
        }
    }
    func_001F9A28(v10, D_0013F4D0_319230_c07_317e60, anchor);
    v10[2] += 0.6f;
    norm_319080_c07_317e60(v10, v10, 2.8f);
    cross_319230_c07_317e60(v20, v10, (char *)moby + 0xD0);
    norm_319080_c07_317e60(v20, v20, 1.0f);
    qcopy_c07_317e60(D_L07_0020DDC0_319080_c07_317e60, anchor);
    rot_319230_c07_317e60(v30, D_L07_00161C20_319080_c07_317e60, (char *)moby + 0xC0);
    add_319080_c07_317e60(v30, v30, v10);
    rot_319230_c07_317e60(v40, D_L07_00161C30_319080_c07_317e60, (char *)moby + 0xC0);
    add_319080_c07_317e60(v40, v40, v10);
    {
        float t = (float)D_L07_00161C10_319080_c07_317e60 / func_001FA6C0(func_001F9688(60));
        D_L07_00161C10_319080_c07_317e60++;
        v10[0] = v30[0] + (v40[0] - v30[0]) * t;
        v10[1] = v30[1] + (v40[1] - v30[1]) * t;
        v10[2] = v30[2] + (v40[2] - v30[2]) * t;
    }
    if (D_L07_00161C10_319080_c07_317e60 >= func_001F9688(60)) {
        qcopy_c07_317e60(D_L07_00161C20_319080_c07_317e60, D_L07_00161C30_319080_c07_317e60);
        norm_319080_c07_317e60(v50, (char *)moby + 0xC0, 2.8f);
        *(u128_319080_c07_317e60 *)v60 = 0;
        *(u128_319080_c07_317e60 *)v70 = 0;
        v60[2] = 0.1f;
        v60[3] = 1.0f;
        v70[0] = 1.0f;
        v70[3] = 1.0f;
        {
            float r3 = func_001FA580(D_L07_00161C40_319080_c07_317e60, func_002132A8(45.0f, 315.0f) * 0.017453292f);
            look_319080_c07_317e60(D_L07_00161C30_319080_c07_317e60, v60, v70, r3);
            D_L07_00161C40_319080_c07_317e60 = r3;
        }
        D_L07_00161C10_319080_c07_317e60 = 0;
    }
    D_L07_00161C14_319230_c07_317e60 = func_001FA580(D_L07_00161C14_319230_c07_317e60, 0.1f);
    norm_319080_c07_317e60(v50, D_0013F4D0_319230_c07_317e60 + 0x210, -1.0f);
    qcopy_c07_317e60(vA0, v50);
    cross_319230_c07_317e60(v90, v10, vA0);
    norm_319080_c07_317e60(v90, v90, 1.0f);
    cross_319230_c07_317e60(v80, vA0, v90);
    {
        int m;
        for (m = 0; m < 20; m++) {
            func_001F9A68(v60, D_L07_0020C9C0_319230_c07_317e60[m], 0.02f);
            rot_319230_c07_317e60(v60, v60, v80);
            add_319080_c07_317e60(D_L07_0020DEA0_319230_c07_317e60[0][m], D_L07_0020DDC0_319080_c07_317e60[0], v60);
        }
    }
    {
    float dot;
    float len;
    float ang;
    float sm;
    float df;
    float fi;
    int i;
    for (i = 1; i < 10; i++) {
        func_001F9A28(v60, D_L07_0020DDC0_319080_c07_317e60[i], D_L07_0020DDB0_319080_c07_317e60[i]);
        qcopy_c07_317e60(v70, v10);
        cross_319230_c07_317e60(vB0, v70, v60);
        dot = func_001F9AB0(v60, v70);
        len = func_001F9AF0(v60);
        ang = 1.5707964f - acos_319230_c07_317e60(dot / (len * 2.8f));
        sm = smooth_319230_c07_317e60(&D_L07_0020DE60_319230_c07_317e60[i], 0.0f, ang, 0.06f, 0.2f, 0.0f);
        df = func_001FA5C8(ang, sm);
        if (func_001F99C0(df) > 0.31415927f) {
            sm = func_001FA580(sm, func_001FA5C8(df, 0.31415927f));
        }
        look_319080_c07_317e60(vC0, v60, vB0, sm);
        norm_319080_c07_317e60(vD0, vC0, 2.8f);
        madd_319230_c07_317e60(vC0, vC0, vD0, 1.0f);
        {
            float l2 = func_001F9AF0(vC0);
            if (l2 > 3.9f) {
                func_001F9A68(vC0, vC0, 3.9f / l2);
            }
        }
        if (i >= 6) {
            madd_319230_c07_317e60(vC0, vC0, v60, (float)i / func_001FA6C0(10) + 0.0f);
        }
        fi = i;
        add_319080_c07_317e60(D_L07_0020DDC0_319080_c07_317e60[i], D_L07_0020DDB0_319080_c07_317e60[i], vC0);
        {
            float dd = seg_319230_c07_317e60(vE0, v00, v00, D_L07_0020DDB0_319080_c07_317e60[i], D_L07_0020DDC0_319080_c07_317e60[i], 0.0f);
            if (dd < best) {
                best = dd;
            }
        }
        qcopy_c07_317e60(vE0, v10);
        if (func_001F99C0(v10[2]) > 2.1f) {
            qcopy_c07_317e60(v110, v50);
            cross_319230_c07_317e60(v120, v110, v10);
            func_001F9A28(v140, D_L07_0020DDA0_319230_c07_317e60[i], D_L07_0020DDB0_319080_c07_317e60[i]);
            cross_319230_c07_317e60(v130, v110, v140);
            if (func_001F9AB0(v120, v130) > 0.0f) {
                func_001F9A68(v110, v110, -1.0f);
            }
        } else {
            norm_319080_c07_317e60(v110, D_0013F4D0_319230_c07_317e60 + 0x210, -1.0f);
        }
        norm_319080_c07_317e60(vF0, vE0, 1.0f);
        cross_319230_c07_317e60(v100, vF0, v110);
        norm_319080_c07_317e60(v100, v100, 1.0f);
        cross_319230_c07_317e60(v110, v100, vF0);
        qcopy_c07_317e60(v50, v110);
        {
            int m;
            float sc;
            for (m = 0; m < 20; m++) {
                sc = fi * 0.2f + 0.02f;
                sc += sc * 0.25f * func_001F9DE0(func_001FA580(D_L07_00161C14_319230_c07_317e60 + fi * 0.7f, 0.0f));
                func_001F9A68(v120, D_L07_0020C9C0_319230_c07_317e60[m], sc);
                rot_319230_c07_317e60(v120, v120, vF0);
                add_319080_c07_317e60(D_L07_0020DEA0_319230_c07_317e60[i][m], D_L07_0020DDC0_319080_c07_317e60[i], v120);
            }
        }
        norm_319080_c07_317e60(v10, vC0, 2.8f);
    }
    }
    func_001F4600(func_L07_00317910, moby);
    {
    int nspawn;
    int nfly;
    int kn;
    int j;
    float t;
    float len;
    float s;
    float sc;
    int r;
    int idx2;
    RopePuff_319230_c07_317e60 *p;
    RopePuff_319230_c07_317e60 *q;
    int k;
    nspawn = 0;
    nfly = 0;
    for (k = 0; k < 200; k = kn) {
        kn = k + 1;
        if (D_L07_0020CB00_319230_c07_317e60[k].moby == 0 && nspawn < 5) {
            for (j = 9; j >= 0; j--) {
                if (D_L07_0020DE88_319080_c07_317e60[j] >= 0x40) {
                    break;
                }
            }
            if (j > 0) {
                if (j >= 2 && nfly < 2) {
                    r = func_00213260(20);
                    s = func_002132A8(0.2f, 1.0f);
                    madd_319230_c07_317e60(v60, D_L07_0020DEA0_319230_c07_317e60[0][r], D_L07_0020DEA0_319230_c07_317e60[j][r], s);
                    madd_319230_c07_317e60(vB0, D_L07_0020DDC0_319080_c07_317e60[0], D_L07_0020DDC0_319080_c07_317e60[j], s);
                    sc = func_002132A8(60000.0f, 180000.0f);
                    p = spawn_319230_c07_317e60(v60, 0.0f, 1.0f, 1.03f, func_00213260(5) + 1, v70, sc, 0x404040);
                    if (p != 0) {
                        RopeTail_319230_c07_317e60 *tail = &p->tail;
                        func_001F9A28(vC0, v60, vB0);
                        nfly++;
                        norm_319080_c07_317e60(tail->vel, vC0, 0.01f);
                        tail->vel[2] += 0.02f;
                    }
                }
                qcopy_c07_317e60(v60, D_L07_0020DDC0_319080_c07_317e60[j]);
                clear_319080_c07_317e60(v70);
                sc = func_002132A8(60000.0f, 180000.0f);
                q = spawn_319230_c07_317e60(v60, 0.0f, 1.0f, 1.0f, 6, v70, sc, 0x7F404040);
                D_L07_0020CB00_319230_c07_317e60[k].moby = (char *)q;
                if (q != 0) {
                    RopeTail_319230_c07_317e60 *tail;
                    q->life = 200;
                    tail = &q->tail;
                    tail->mode = 2;
                    nspawn++;
                    tail->tint = 0x7F;
                    tail->fade = -0x38;
                }
                D_L07_0020CB00_319230_c07_317e60[k].idx = j;
                D_L07_0020CB00_319230_c07_317e60[k].f4 = 0.0f;
                D_L07_0020CB00_319230_c07_317e60[k].f8 = 0.0f;
                D_L07_0020CB00_319230_c07_317e60[k].idx2 = func_00213260(20);
                D_L07_0020CB00_319230_c07_317e60[k].fC = func_002132A8(0.1f, 0.8f);
                D_L07_0020CB00_319230_c07_317e60[k].f10 = func_002132A8(0.1f, 0.4f);
            }
        }
        if (D_L07_0020CB00_319230_c07_317e60[k].moby != 0) {
            func_001F9A28(v60, D_L07_0020DDB0_319080_c07_317e60[D_L07_0020CB00_319230_c07_317e60[k].idx],
                          D_L07_0020DDC0_319080_c07_317e60[D_L07_0020CB00_319230_c07_317e60[k].idx]);
            len = func_001F9AF0(v60);
            D_L07_0020CB00_319230_c07_317e60[k].f4 += D_L07_0020CB00_319230_c07_317e60[k].f10;
            *(short *)(D_L07_0020CB00_319230_c07_317e60[k].moby + 0xA) = 200;
            if (len < D_L07_0020CB00_319230_c07_317e60[k].f4) {
                if (--D_L07_0020CB00_319230_c07_317e60[k].idx <= 0) {
                    kill_319230_c07_317e60(D_L07_0020CB00_319230_c07_317e60[k].moby);
                    D_L07_0020CB00_319230_c07_317e60[k].moby = 0;
                } else {
                    D_L07_0020CB00_319230_c07_317e60[k].f4 -= len;
                    func_001F9A28(v60, D_L07_0020DDB0_319080_c07_317e60[D_L07_0020CB00_319230_c07_317e60[k].idx],
                                  D_L07_0020DDC0_319080_c07_317e60[D_L07_0020CB00_319230_c07_317e60[k].idx]);
                    len = func_001F9AF0(v60);
                }
            }
            if (D_L07_0020CB00_319230_c07_317e60[k].moby != 0) {
                t = D_L07_0020CB00_319230_c07_317e60[k].f4 / len;
                if (D_L07_0020CB00_319230_c07_317e60[k].idx == 2) {
                    *(short *)(D_L07_0020CB00_319230_c07_317e60[k].moby + 0xA) = 100;
                } else if (D_L07_0020CB00_319230_c07_317e60[k].idx < 2) {
                    *(short *)(D_L07_0020CB00_319230_c07_317e60[k].moby + 0xA) = func_001FA6D0((1.0f - t) * 100.0f);
                    if (*(short *)(D_L07_0020CB00_319230_c07_317e60[k].moby + 0xA) <= 60) {
                        *(short *)(D_L07_0020CB00_319230_c07_317e60[k].moby + 0xA) = 60;
                    }
                }
                if (D_L07_0020CB00_319230_c07_317e60[k].idx == 1) {
                    func_001F9A28(v70, anchor, D_L07_0020CB00_319230_c07_317e60[k].moby + 0x10);
                    norm_319080_c07_317e60(v70, v70, D_L07_0020CB00_319230_c07_317e60[k].f10);
                    add_319080_c07_317e60(D_L07_0020CB00_319230_c07_317e60[k].moby + 0x10, D_L07_0020CB00_319230_c07_317e60[k].moby + 0x10, v70);
                    *(float *)(D_L07_0020CB00_319230_c07_317e60[k].moby + 0xC) *= D_0015ED60 * -0.050000012f + 1.0f;
                } else {
                    func_001F9A68(v60, v60, t);
                    add_319080_c07_317e60(D_L07_0020CB00_319230_c07_317e60[k].moby + 0x10,
                               D_L07_0020DDC0_319080_c07_317e60[D_L07_0020CB00_319230_c07_317e60[k].idx], v60);
                    idx2 = D_L07_0020CB00_319230_c07_317e60[k].idx2;
                    func_001F9A28(v70, D_L07_0020DEA0_319230_c07_317e60[D_L07_0020CB00_319230_c07_317e60[k].idx][idx2],
                                  D_L07_0020DDC0_319080_c07_317e60[D_L07_0020CB00_319230_c07_317e60[k].idx]);
                    func_001F9A28(vB0, D_L07_0020DD60_319230_c07_317e60[D_L07_0020CB00_319230_c07_317e60[k].idx][idx2],
                                  D_L07_0020DDB0_319080_c07_317e60[D_L07_0020CB00_319230_c07_317e60[k].idx]);
                    lerp_319230_c07_317e60(vC0, v70, vB0, t);
                    idx2 = (idx2 + 19) % 20;
                    func_001F9A28(vD0, D_L07_0020DEA0_319230_c07_317e60[D_L07_0020CB00_319230_c07_317e60[k].idx][idx2],
                                  D_L07_0020DDC0_319080_c07_317e60[D_L07_0020CB00_319230_c07_317e60[k].idx]);
                    func_001F9A28(vE0, D_L07_0020DD60_319230_c07_317e60[D_L07_0020CB00_319230_c07_317e60[k].idx][idx2],
                                  D_L07_0020DDB0_319080_c07_317e60[D_L07_0020CB00_319230_c07_317e60[k].idx]);
                    lerp_319230_c07_317e60(vF0, vD0, vE0, t);
                    lerp_319230_c07_317e60(vC0, vC0, vF0, D_L07_0020CB00_319230_c07_317e60[k].f8);
                    D_L07_0020CB00_319230_c07_317e60[k].f8 += D_L07_0020CB00_319230_c07_317e60[k].fC;
                    if (D_L07_0020CB00_319230_c07_317e60[k].f8 >= 1.0f) {
                        D_L07_0020CB00_319230_c07_317e60[k].f8 -= 1.0f;
                        D_L07_0020CB00_319230_c07_317e60[k].idx2 = idx2;
                    }
                    add_319080_c07_317e60(D_L07_0020CB00_319230_c07_317e60[k].moby + 0x10, D_L07_0020CB00_319230_c07_317e60[k].moby + 0x10, vC0);
                }
            }
        }
    }
    }
    return best;
}
