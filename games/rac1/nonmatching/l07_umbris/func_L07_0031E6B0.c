/* NON_MATCHING func_L07_0031E6B0 -- src/overlays/l07_umbris/vendor_0031BDB8.c
 * Best so far: BYTES 8/1196 (99.3% of the bytes match), checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   function level (pseudo numbering moves the spilled next-index after &ia): BYTES 8/1196 (budget run 2).
 *   - Left (p16): one pair of instructions in the printf call after the inner loop: retail sets $a0
 *   (%lo of D_L07_00211E68) before $a3; ours after. Cause (gcse dump p18.gcse): PRE moves
 *   high(D_L07_00211E68) up to the top of the outer loop body (in front of the inner loop), the
 *   pseudo gets no register and reload rematerialises it, so the scheduler no longer sees a dying
 *   register in the $a0 move. Retail keeps the high in the block. Tried without effect: literal
 *   12.0f/10.0f, while/for/forever shapes of the outer loop, a format pointer local, n = rem after
 *   the call, a string literal (diagnostic only).
 */
typedef struct {
    float x, y, z;
    float len;              /* 0xC: length of the segment to the next node */
} PathNode_31E6B0;

typedef struct {
    int count;
    int pad[3];
    PathNode_31E6B0 nodes[1]; /* 0x10 */
} Path_31E6B0;

typedef struct {
    int vol;                /* 0x00 */
    int path;               /* 0x04 */
    float lo;               /* 0x08: percent */
    float hi;               /* 0x0C */
    float pa;               /* 0x10 */
    float pb;               /* 0x14 */
    int mode;               /* 0x18 */
    int div;                /* 0x1C */
    float total;            /* 0x20 */
    int target;             /* 0x24 */
} SparkVars_31E6B0;

typedef struct {
    char pad00[0x20];
    unsigned char state;    /* 0x20 */
    char pad21[0x57];
    SparkVars_31E6B0 *vars; /* 0x78 */
    char pad7C[0x36];
    short oclass;           /* 0xB2 */
} SparkMoby_31E6B0;

typedef struct {
    char pad00[0x10];
    float pos[4];           /* 0x10 */
    char pad20[0xE0];
} Moby256_31E6B0;

typedef struct {
    char pad00[0xE];
    unsigned short max;     /* 0x0E */
    char pad10[0x8];
} Ent_31E6B0;

extern int func_00215570_31E6B0(void *arg0, int arg1) __asm__("func_00215570");
extern float func_001FA888_31E6B0(int) __asm__("func_001FA888");
extern int func_001FA898_31E6B0(float) __asm__("func_001FA898");
extern float func_001F9D10_31E6B0(void *, void *) __asm__("func_001F9D10");
extern int func_L00_0025E860_31E6B0(void *, void *, int *, float *, float, int) __asm__("func_L00_0025E860");
extern float func_001F9D48_31E6B0(void *, void *) __asm__("func_001F9D48");
extern int func_00120778_31E6B0(float) __asm__("func_00120778");
extern char *func_L00_002C6608_31E6B0(void *, int, int, int) __asm__("func_L00_002C6608");
extern float func_002140F8_31E6B0(float, float) __asm__("func_002140F8");
extern int func_001E9730_31E6B0() __asm__("func_001E9730");
extern void func_0020D678_31E6B0(void *) __asm__("func_0020D678");
extern char D_L07_00211DE0_31E6B0[] __asm__("D_L07_00211DE0");
extern char D_L07_00211E20_31E6B0[] __asm__("D_L07_00211E20");
extern char D_L07_00211E40_31E6B0[] __asm__("D_L07_00211E40");
extern char D_L07_00211E68_31E6B0[] __asm__("D_L07_00211E68");
extern char D_L07_00211E98_31E6B0[] __asm__("D_L07_00211E98");
extern Path_31E6B0 *D_L07_001B0830_31E6B0[] __asm__("D_L07_001B0830");
extern char D_L07_001C43B0_31E6B0[] __asm__("D_L07_001C43B0");
extern int D_L07_00161D30_31E6B0 SDATA(D_L07_00161D30);
extern Moby256_31E6B0 *D_L07_00160058_31E6B0 __asm__("D_L07_00160058") MACRO_ADDR;
extern unsigned char D_0013E633_31E6B0[] __asm__("D_0013E633");
extern unsigned char D_0013D50F_31E6B0[] __asm__("D_0013D50F");

// Update for moby class 1142 on level 07: measures its path once, then places sparks for the listed counters.
void func_L07_0031E6B0(SparkMoby_31E6B0 *moby)
{
    SparkVars_31E6B0 *d = moby->vars;
    float vec[3];
    int ia;
    int fbz;
    int st;

    st = moby->state;
    if (st != 1) {
        int bad;
        int j;
        if (st >= 2) {
            return;
        }
        if (st != 0) {
            return;
        }
        bad = 0;
        if (d->vol == -1) {
            func_001E9730_31E6B0(D_L07_00211DE0_31E6B0);
            bad = 1;
        }
        if (d->path == -1) {
            func_001E9730_31E6B0(D_L07_00211DE0_31E6B0);
            bad = 1;
        }
        if (bad) {
            func_001E9730_31E6B0(D_L07_00211E20_31E6B0, moby->oclass);
            func_0020D678_31E6B0(moby);
            return;
        }
        d->total = 0.0f;
        for (j = 0; j < D_L07_001B0830_31E6B0[d->path]->count; j++) {
            int nj = (j + 1) % D_L07_001B0830_31E6B0[d->path]->count;
            D_L07_001B0830_31E6B0[d->path]->nodes[j].len =
                func_001F9D10_31E6B0(&D_L07_001B0830_31E6B0[d->path]->nodes[j],
                                     &D_L07_001B0830_31E6B0[d->path]->nodes[nj]);
            d->total = d->total + D_L07_001B0830_31E6B0[d->path]->nodes[j].len;
        }
        moby->state = 1;
    } else {
        int done;
        int i, k, m, n, rem;
        float have, max, ratio, at, a, b, v, c, near, step;
        if (func_00215570_31E6B0(D_0013E633_31E6B0 + 0xE9D, d->vol) == 0) {
            return;
        }
        for (i = 0; (&D_L07_00161D30_31E6B0)[i] != -1; i++) {
            k = (&D_L07_00161D30_31E6B0)[i];
            have = func_001FA888_31E6B0(((int *)(D_0013D50F_31E6B0 + 0x21))[k]);
            max = func_001FA888_31E6B0(((Ent_31E6B0 *)D_L07_001C43B0_31E6B0)[k].max);
            ratio = have / max;
            at = func_002140F8_31E6B0(0.0f, d->total);
            if (d->lo / 100.0f <= ratio && ratio < d->hi / 100.0f) {
                Path_31E6B0 *p;
                int q;
                a = max * (d->pa / 100.0f);
                b = max * (d->pb / 100.0f);
                c = a - have;
                v = d->mode == 0 ? (c < b ? c : b) : (b < c ? c : b);
                n = func_001FA898_31E6B0(v + 0.5f);
                q = n / d->div;
                m = n;
                fbz = 0;
                ia = 0;
                p = D_L07_001B0830_31E6B0[d->path];
                if (q != 0) {
                    m = q;
                }
                func_001E9730_31E6B0(D_L07_00211E40_31E6B0, n, m);
                if (n > 0) {
                    near = 12.0f;
                    step = 10.0f;
                    do {
                        rem = n - m;
                        done = 0;
                        do {
                            func_L00_0025E860_31E6B0(p, vec, &ia, (float *)&fbz, at, 1);
                            if (d->target == -1) {
                                done = 1;
                            } else if (near < func_001F9D48_31E6B0(D_L07_00160058_31E6B0[d->target].pos, vec)) {
                                done = 1;
                            } else {
                                at = at + step;
                            }
                        } while (done == 0);
                        n = rem;
                        func_001E9730_31E6B0(D_L07_00211E68_31E6B0, k, m,
                                             func_00120778_31E6B0(vec[0]), func_00120778_31E6B0(vec[1]),
                                             func_00120778_31E6B0(vec[2]), func_00120778_31E6B0(at));
                        func_L00_002C6608_31E6B0(vec, k, m, 0);
                        at = at + func_002140F8_31E6B0(d->total / step, 2.0f);
                        if (rem < m) {
                            m = rem;
                            func_001E9730_31E6B0(D_L07_00211E98_31E6B0, rem);
                        }
                    } while (rem > 0);
                }
            }
        }
        moby->state = 2;
    }
}
