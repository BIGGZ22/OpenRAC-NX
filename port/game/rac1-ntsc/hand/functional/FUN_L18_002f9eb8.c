/* Carried from the PAL decompilation's nonmatching/l18_veldin2/func_L18_002FB318.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it. */
extern unsigned long func_001F44B8(int);
extern int D_L18_00162560 MACRO_ADDR;
extern char D_L18_001677C0[];

/* NON_MATCHING FUN_L18_002f9eb8 -- src/overlays/l18_veldin2/vendor_002F9D48.c
 * Best so far: SIZE ours 904 / retail 920, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   What mattered:
 *   - qcopy() in the scale loop keeps the forward (slti/bnez) counter; plain struct copy gets the loop reversed.
 *   - k = D_L18_0016252C read into a local before the loop (qcopy has a memory clobber, else reloaded per iteratio
 *   - pkt[2] assigned before pkt[3]; QVec e declared at function scope (stack slot 0xD0), assigned late.
 *   - `pa = &a;` right before the first func_001F9CF8 and used everywhere (loops too) gives retail's early &a comp
 *   Remaining diff: retail keeps q's address (sp+0xE0) in $18 and a's address in $21 as two long-lived pseudos
 *   (ours uses $s3/$s5 and merges); retail passes 2/3 also reload D_L18_0016252C after the color stores and recomp
 *   &q for the 9EE8 loop arg. A pq pointer (p13) compiled to identical bytes as p12.
 */
typedef struct {
    float v[4];
} __attribute__((aligned(16))) QVb318_c2f9eb8;
extern void func_00233980(int, long);
extern void func_001F9CF8(void *, void *, void *);
extern void func_001F9A10(void *, void *, void *);
extern void func_001F9A28(void *, void *, void *);
extern void func_001F9AD8(void *, void *, void *);
extern void func_L00_001FF500(void *, void *, float);
extern void func_001F9D20(void *, void *, void *);
extern void func_L00_001FD228(void *, void *, int);
extern QVb318_c2f9eb8 D_L18_001DFBD0[];
extern float D_L18_0016252C;
extern short D_L18_00162504;
extern short D_L18_00162510;
extern short D_L18_00162514;
extern short D_L18_00162518;
extern short D_L18_0016251C;

void FUN_L18_002f9eb8(char *moby) {
    QVb318_c2f9eb8 m[4];
    int colors[4];
    float uv[4][2];
    unsigned long pkt[4];
    QVb318_c2f9eb8 a, b, c, d, e;
    QVb318_c2f9eb8 q[4];
    int i, j;
    QVb318_c2f9eb8 *pa;
    QVb318_c2f9eb8 *pq;

    pkt[1] = func_001F44B8(11);
    pkt[2] = 0xFF9000000260;
    pkt[3] = 0x8000000048;
    pkt[0] = 0;
    func_00233980(0x4A, 0);
    func_00233980(0x47, 0x51001);
    pa = &a;
    func_001F9CF8(&d, &D_L18_00162560, moby + 0xC0);
    func_001F9A10(&d, &d, moby + 0x10);
    e = (QVb318_c2f9eb8){{0.0f}};
    e.v[2] = -1.0f;
    e.v[3] = 1.0f;
    func_001F9CF8(pa, &e, moby + 0xC0);
    func_001F9A28(&b, &d, D_L18_001677C0);
    func_001F9AD8(&c, &b, pa);
    func_L00_001FF500(&c, &c, 1.0f);
    func_001F9AD8(&b, pa, &c);
    uv[0][0] = 1.0f; uv[0][1] = 1.0f;
    uv[1][0] = 0.0f; uv[1][1] = 1.0f;
    uv[2][0] = 1.0f; uv[2][1] = 0.0f;
    uv[3][0] = 0.0f; uv[3][1] = 0.0f;
    pq = q;
    colors[0] = colors[1] = colors[2] = colors[3] = *(int *)&D_L18_00162504 | 0xFF000000;
    {
        float z = *(float *)&D_L18_00162514;
        float k = D_L18_0016252C;
        for (i = 0; i < 4; i++) {
            qcopy(&q[i], &D_L18_001DFBD0[i]);
            pq[i].v[2] *= z;
            pq[i].v[0] *= k;
        }
    }
    for (j = 0; j < 4; j++) func_001F9D20(&m[j], &pq[j], pa);
    func_L00_001FD228(m, 0, 0);
    colors[0] = colors[1] = colors[2] = colors[3] = *(int *)&D_L18_00162510 | 0x7F000000;
    {
        float z = *(float *)&D_L18_00162518;
        float k = D_L18_0016252C;
        for (i = 0; i < 4; i++) {
            qcopy(&q[i], &D_L18_001DFBD0[i]);
            pq[i].v[2] *= z;
            pq[i].v[0] *= k;
        }
    }
    for (j = 0; j < 4; j++) func_001F9D20(&m[j], &pq[j], pa);
    func_L00_001FD228(m, 0, 0);
    colors[0] = colors[1] = colors[2] = colors[3] = *(int *)&D_L18_00162510 | 0x7F000000;
    {
        float z = *(float *)&D_L18_0016251C;
        float k = D_L18_0016252C;
        for (i = 0; i < 4; i++) {
            qcopy(&q[i], &D_L18_001DFBD0[i]);
            pq[i].v[2] *= z;
            pq[i].v[0] *= k;
        }
    }
    for (j = 0; j < 4; j++) func_001F9D20(&m[j], &pq[j], pa);
    func_L00_001FD228(m, 0, 0);
}
