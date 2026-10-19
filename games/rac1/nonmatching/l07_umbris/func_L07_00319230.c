/* NON_MATCHING func_L07_00319230 -- src/overlays/l07_umbris/vendor_00313D28.c
 * Best so far: BYTES 126/4044 (96.9% of the bytes match), checked 2026-10-14.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   - entry block: order of `addiu $s5,$sp,0x80` / `addiu $s2,$sp,0x90`; loop-2 preheader copies (load order, $a0/
 *   - loop 3: reload register for the constant 24 ($v0 in retail), second spawn's result in $v1 and its store orde
 *   CB04/CB10 temporaries, one bc1f that comes out bc1fl (delay slot annulled because of the register choice).
 *   ## equiv (tools/equiv.py p12.c --show): NEAR, one item
 *   retail only: jal func_L00_002688A8 a2=#c8 / ours only: jal func_L00_002688A8 a2=#20cb04
 *   func_L00_002688A8 takes one argument (`extern void func_L00_002688A8(void *)`, a0 = the spark moby). $a2 is no
 *   argument: in retail it still holds 200 from `addiu $6,$0,0xC8 ; sh $6,0xA($2)` (moby->life = 200) earlier in t
 *   block, in ours it holds the &D_L07_0020CB04 temporary. A stale scratch register, not a value passed: non-seman
 *   fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18).
 */
typedef struct {
    short idx;
    short idx2;
    float f4;
    float f8;
    float fC;
    float f10;
    char *moby;
} RopePart_319230;
typedef struct {
    char pad0[4];
    int mode;
    char pad8[2];
    unsigned char tint;
    char fade;
    char padC[4];
    float vel[4];
} RopeTail_319230;
typedef struct {
    char pad0[0xA];
    short life;
    char padC[0x14];
    RopeTail_319230 tail;
} RopePuff_319230;
extern RopePart_319230 D_L07_0020CB00_319230[] __asm__("D_L07_0020CB00");
extern RopeVec_319080 D_L07_0020DDA0_319230[] __asm__("D_L07_0020DDA0");
extern RopeVec_319080 D_L07_0020DEA0_319230[][20] __asm__("D_L07_0020DEA0");
extern RopeVec_319080 D_L07_0020DD60_319230[][20] __asm__("D_L07_0020DD60");
extern RopeVec_319080 D_L07_0020C9C0_319230[] __asm__("D_L07_0020C9C0");
extern float D_L07_0020DE60_319230[] __asm__("D_L07_0020DE60");
extern float D_L07_00161C14_319230 SDATA(D_L07_00161C14);
extern char D_0013F4D0_319230[] __asm__("D_0013F4D0");
extern void cross_319230(void *, void *, void *) __asm__("func_001F9CA0");
extern void rot_319230(void *, void *, void *) __asm__("func_001F9EC0");
extern float acos_319230(float) __asm__("func_001F9FC0");
extern float smooth_319230(void *, float, float, float, float, float) __asm__("func_L00_001EB6A8");
extern void madd_319230(void *, void *, void *, float) __asm__("func_001F9C08");
extern float seg_319230(char *, char *, char *, char *, char *, float) __asm__("func_L07_0029F4A8");
extern RopePuff_319230 *spawn_319230(void *, float, float, float, int, void *, float, int) __asm__("func_L00_0026DEA0");
extern void kill_319230(void *) __asm__("func_L00_002688A8");
extern void lerp_319230(void *, void *, void *, float) __asm__("func_L00_0025C710");
extern void func_L07_00318CE0();

/* Updates the rope: swings its anchor frame, solves the nine segments toward the hero,
 * rebuilds each ring of twenty points, then spawns and moves the sparks running along it;
 * returns the closest segment distance to the hero. */
float func_L07_00319230(void *moby, void *anchor) {
    RopeVec_319080 v00;
    RopeVec_319080 v10;
    RopeVec_319080 v20;
    RopeVec_319080 v30;
    RopeVec_319080 v40;
    RopeVec_319080 v50;
    RopeVec_319080 v60;
    RopeVec_319080 v70;
    RopeVec_319080 v80;
    RopeVec_319080 v90;
    RopeVec_319080 vA0;
    RopeVec_319080 vB0;
    RopeVec_319080 vC0;
    RopeVec_319080 vD0;
    RopeVec_319080 vE0;
    RopeVec_319080 vF0;
    RopeVec_319080 v100;
    RopeVec_319080 v110;
    RopeVec_319080 v120;
    RopeVec_319080 v130;
    RopeVec_319080 v140;
    float best;

    best = 1000.0f;
    func_L07_00319080(moby, anchor);
    qcopy(v00, D_0013F4D0_319230);
    v00[2] += 0.6f;
    D_L07_0020DE88_319080[0] += 4;
    {
        int n;
        for (n = 1; n < 10; n++) {
            D_L07_0020DE88_319080[n] = D_L07_0020DE88_319080[n - 1] - 0x20;
        }
    }
    func_001F9BF0(v10, D_0013F4D0_319230, anchor);
    v10[2] += 0.6f;
    norm_319080(v10, v10, 2.8f);
    cross_319230(v20, v10, (char *)moby + 0xD0);
    norm_319080(v20, v20, 1.0f);
    qcopy(D_L07_0020DDC0_319080, anchor);
    rot_319230(v30, D_L07_00161C20_319080, (char *)moby + 0xC0);
    add_319080(v30, v30, v10);
    rot_319230(v40, D_L07_00161C30_319080, (char *)moby + 0xC0);
    add_319080(v40, v40, v10);
    {
        float t = (float)D_L07_00161C10_319080 / func_001FA888(func_001F9850(60));
        D_L07_00161C10_319080++;
        v10[0] = v30[0] + (v40[0] - v30[0]) * t;
        v10[1] = v30[1] + (v40[1] - v30[1]) * t;
        v10[2] = v30[2] + (v40[2] - v30[2]) * t;
    }
    if (D_L07_00161C10_319080 >= func_001F9850(60)) {
        qcopy(D_L07_00161C20_319080, D_L07_00161C30_319080);
        norm_319080(v50, (char *)moby + 0xC0, 2.8f);
        *(u128_319080 *)v60 = 0;
        *(u128_319080 *)v70 = 0;
        v60[2] = 0.1f;
        v60[3] = 1.0f;
        v70[0] = 1.0f;
        v70[3] = 1.0f;
        {
            float r3 = func_001FA748(D_L07_00161C40_319080, func_002140F8(45.0f, 315.0f) * 0.017453292f);
            look_319080(D_L07_00161C30_319080, v60, v70, r3);
            D_L07_00161C40_319080 = r3;
        }
        D_L07_00161C10_319080 = 0;
    }
    D_L07_00161C14_319230 = func_001FA748(D_L07_00161C14_319230, 0.1f);
    norm_319080(v50, D_0013F4D0_319230 + 0x210, -1.0f);
    qcopy(vA0, v50);
    cross_319230(v90, v10, vA0);
    norm_319080(v90, v90, 1.0f);
    cross_319230(v80, vA0, v90);
    {
        int m;
        for (m = 0; m < 20; m++) {
            func_001F9C30(v60, D_L07_0020C9C0_319230[m], 0.02f);
            rot_319230(v60, v60, v80);
            add_319080(D_L07_0020DEA0_319230[0][m], D_L07_0020DDC0_319080[0], v60);
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
        func_001F9BF0(v60, D_L07_0020DDC0_319080[i], D_L07_0020DDB0_319080[i]);
        qcopy(v70, v10);
        cross_319230(vB0, v70, v60);
        dot = func_001F9C78(v60, v70);
        len = func_001F9CB8(v60);
        ang = 1.5707964f - acos_319230(dot / (len * 2.8f));
        sm = smooth_319230(&D_L07_0020DE60_319230[i], 0.0f, ang, 0.06f, 0.2f, 0.0f);
        df = func_001FA790(ang, sm);
        if (func_001F9B88(df) > 0.31415927f) {
            sm = func_001FA748(sm, func_001FA790(df, 0.31415927f));
        }
        look_319080(vC0, v60, vB0, sm);
        norm_319080(vD0, vC0, 2.8f);
        madd_319230(vC0, vC0, vD0, 1.0f);
        {
            float l2 = func_001F9CB8(vC0);
            if (l2 > 3.9f) {
                func_001F9C30(vC0, vC0, 3.9f / l2);
            }
        }
        if (i >= 6) {
            madd_319230(vC0, vC0, v60, (float)i / func_001FA888(10) + 0.0f);
        }
        fi = i;
        add_319080(D_L07_0020DDC0_319080[i], D_L07_0020DDB0_319080[i], vC0);
        {
            float dd = seg_319230(vE0, v00, v00, D_L07_0020DDB0_319080[i], D_L07_0020DDC0_319080[i], 0.0f);
            if (dd < best) {
                best = dd;
            }
        }
        qcopy(vE0, v10);
        if (func_001F9B88(v10[2]) > 2.1f) {
            qcopy(v110, v50);
            cross_319230(v120, v110, v10);
            func_001F9BF0(v140, D_L07_0020DDA0_319230[i], D_L07_0020DDB0_319080[i]);
            cross_319230(v130, v110, v140);
            if (func_001F9C78(v120, v130) > 0.0f) {
                func_001F9C30(v110, v110, -1.0f);
            }
        } else {
            norm_319080(v110, D_0013F4D0_319230 + 0x210, -1.0f);
        }
        norm_319080(vF0, vE0, 1.0f);
        cross_319230(v100, vF0, v110);
        norm_319080(v100, v100, 1.0f);
        cross_319230(v110, v100, vF0);
        qcopy(v50, v110);
        {
            int m;
            float sc;
            for (m = 0; m < 20; m++) {
                sc = fi * 0.2f + 0.02f;
                sc += sc * 0.25f * func_001F9FA8(func_001FA748(D_L07_00161C14_319230 + fi * 0.7f, 0.0f));
                func_001F9C30(v120, D_L07_0020C9C0_319230[m], sc);
                rot_319230(v120, v120, vF0);
                add_319080(D_L07_0020DEA0_319230[i][m], D_L07_0020DDC0_319080[i], v120);
            }
        }
        norm_319080(v10, vC0, 2.8f);
    }
    }
    func_001F49B0(func_L07_00318CE0, moby);
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
    RopePuff_319230 *p;
    RopePuff_319230 *q;
    int k;
    nspawn = 0;
    nfly = 0;
    for (k = 0; k < 200; k = kn) {
        kn = k + 1;
        if (D_L07_0020CB00_319230[k].moby == 0 && nspawn < 5) {
            for (j = 9; j >= 0; j--) {
                if (D_L07_0020DE88_319080[j] >= 0x40) {
                    break;
                }
            }
            if (j > 0) {
                if (j >= 2 && nfly < 2) {
                    r = func_002140B0(20);
                    s = func_002140F8(0.2f, 1.0f);
                    madd_319230(v60, D_L07_0020DEA0_319230[0][r], D_L07_0020DEA0_319230[j][r], s);
                    madd_319230(vB0, D_L07_0020DDC0_319080[0], D_L07_0020DDC0_319080[j], s);
                    sc = func_002140F8(60000.0f, 180000.0f);
                    p = spawn_319230(v60, 0.0f, 1.0f, 1.03f, func_002140B0(5) + 1, v70, sc, 0x404040);
                    if (p != 0) {
                        RopeTail_319230 *tail = &p->tail;
                        func_001F9BF0(vC0, v60, vB0);
                        nfly++;
                        norm_319080(tail->vel, vC0, 0.01f);
                        tail->vel[2] += 0.02f;
                    }
                }
                qcopy(v60, D_L07_0020DDC0_319080[j]);
                clear_319080(v70);
                sc = func_002140F8(60000.0f, 180000.0f);
                q = spawn_319230(v60, 0.0f, 1.0f, 1.0f, 6, v70, sc, 0x7F404040);
                D_L07_0020CB00_319230[k].moby = (char *)q;
                if (q != 0) {
                    RopeTail_319230 *tail;
                    q->life = 200;
                    tail = &q->tail;
                    tail->mode = 2;
                    nspawn++;
                    tail->tint = 0x7F;
                    tail->fade = -0x38;
                }
                D_L07_0020CB00_319230[k].idx = j;
                D_L07_0020CB00_319230[k].f4 = 0.0f;
                D_L07_0020CB00_319230[k].f8 = 0.0f;
                D_L07_0020CB00_319230[k].idx2 = func_002140B0(20);
                D_L07_0020CB00_319230[k].fC = func_002140F8(0.1f, 0.8f);
                D_L07_0020CB00_319230[k].f10 = func_002140F8(0.1f, 0.4f);
            }
        }
        if (D_L07_0020CB00_319230[k].moby != 0) {
            func_001F9BF0(v60, D_L07_0020DDB0_319080[D_L07_0020CB00_319230[k].idx],
                          D_L07_0020DDC0_319080[D_L07_0020CB00_319230[k].idx]);
            len = func_001F9CB8(v60);
            D_L07_0020CB00_319230[k].f4 += D_L07_0020CB00_319230[k].f10;
            *(short *)(D_L07_0020CB00_319230[k].moby + 0xA) = 200;
            if (len < D_L07_0020CB00_319230[k].f4) {
                if (--D_L07_0020CB00_319230[k].idx <= 0) {
                    kill_319230(D_L07_0020CB00_319230[k].moby);
                    D_L07_0020CB00_319230[k].moby = 0;
                } else {
                    D_L07_0020CB00_319230[k].f4 -= len;
                    func_001F9BF0(v60, D_L07_0020DDB0_319080[D_L07_0020CB00_319230[k].idx],
                                  D_L07_0020DDC0_319080[D_L07_0020CB00_319230[k].idx]);
                    len = func_001F9CB8(v60);
                }
            }
            if (D_L07_0020CB00_319230[k].moby != 0) {
                t = D_L07_0020CB00_319230[k].f4 / len;
                if (D_L07_0020CB00_319230[k].idx == 2) {
                    *(short *)(D_L07_0020CB00_319230[k].moby + 0xA) = 100;
                } else if (D_L07_0020CB00_319230[k].idx < 2) {
                    *(short *)(D_L07_0020CB00_319230[k].moby + 0xA) = func_001FA898((1.0f - t) * 100.0f);
                    if (*(short *)(D_L07_0020CB00_319230[k].moby + 0xA) <= 60) {
                        *(short *)(D_L07_0020CB00_319230[k].moby + 0xA) = 60;
                    }
                }
                if (D_L07_0020CB00_319230[k].idx == 1) {
                    func_001F9BF0(v70, anchor, D_L07_0020CB00_319230[k].moby + 0x10);
                    norm_319080(v70, v70, D_L07_0020CB00_319230[k].f10);
                    add_319080(D_L07_0020CB00_319230[k].moby + 0x10, D_L07_0020CB00_319230[k].moby + 0x10, v70);
                    *(float *)(D_L07_0020CB00_319230[k].moby + 0xC) *= D_0015EE60 * -0.050000012f + 1.0f;
                } else {
                    func_001F9C30(v60, v60, t);
                    add_319080(D_L07_0020CB00_319230[k].moby + 0x10,
                               D_L07_0020DDC0_319080[D_L07_0020CB00_319230[k].idx], v60);
                    idx2 = D_L07_0020CB00_319230[k].idx2;
                    func_001F9BF0(v70, D_L07_0020DEA0_319230[D_L07_0020CB00_319230[k].idx][idx2],
                                  D_L07_0020DDC0_319080[D_L07_0020CB00_319230[k].idx]);
                    func_001F9BF0(vB0, D_L07_0020DD60_319230[D_L07_0020CB00_319230[k].idx][idx2],
                                  D_L07_0020DDB0_319080[D_L07_0020CB00_319230[k].idx]);
                    lerp_319230(vC0, v70, vB0, t);
                    idx2 = (idx2 + 19) % 20;
                    func_001F9BF0(vD0, D_L07_0020DEA0_319230[D_L07_0020CB00_319230[k].idx][idx2],
                                  D_L07_0020DDC0_319080[D_L07_0020CB00_319230[k].idx]);
                    func_001F9BF0(vE0, D_L07_0020DD60_319230[D_L07_0020CB00_319230[k].idx][idx2],
                                  D_L07_0020DDB0_319080[D_L07_0020CB00_319230[k].idx]);
                    lerp_319230(vF0, vD0, vE0, t);
                    lerp_319230(vC0, vC0, vF0, D_L07_0020CB00_319230[k].f8);
                    D_L07_0020CB00_319230[k].f8 += D_L07_0020CB00_319230[k].fC;
                    if (D_L07_0020CB00_319230[k].f8 >= 1.0f) {
                        D_L07_0020CB00_319230[k].f8 -= 1.0f;
                        D_L07_0020CB00_319230[k].idx2 = idx2;
                    }
                    add_319080(D_L07_0020CB00_319230[k].moby + 0x10, D_L07_0020CB00_319230[k].moby + 0x10, vC0);
                }
            }
        }
    }
    }
    return best;
}
