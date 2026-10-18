/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), src/core/00125630.c (games/rac1): func_0012A0F8, matched C there.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */

extern int func_0011D660(void);
extern void func_0011D6A8(void);
extern void dmaRefImage(void *);
extern char D_0015E840[];
struct DmaEnt_c129fb8 {
    int unk0;
    int unk4;
    int a[4];
    int b[69];
    int n;
    int pad[4];
};
struct DmaObj_c129fb8 {
    unsigned char pad_0[0x590];
    struct DmaEnt_c129fb8 e[2];
    int unk810;
};
void dmaRefImage(void *arg0) {
    struct DmaObj_c129fb8 *s = arg0;
    int m;
    int n;
    int i;
    long *p;
    long tag;
    int lo;
    int hi;
    int r;
    m = s->unk810;
    p = (long *)(((unsigned int)D_0015E840 & 0x0FFFFFFF) | 0x20000000);
    n = s->e[m].n;
    i = 0;
    if (n > 0) do {
        lo = s->e[m].a[i] & 0x0FFFFFFF;
        hi = s->e[m].b[i] & 0x0FFFFFFF;
        tag = 3;
        if (i == n - 1)
            tag = 0;
        p[0] = ((long)lo << 32) | 0x30000030;
        p[2] = ((long)hi << 32) | (tag << 28) | 0x30;
        p += 4;
    } while (++i < n);
    r = func_0011D660();
    __asm__ __volatile__("sync");
    *(volatile int *)0x1000D480 = s->e[s->unk810].unk0;
    *(volatile int *)0x1000D430 = (int)D_0015E840;
    *(volatile int *)0x1000D420 = 0;
    *(volatile int *)0x1000D400 = 0x105;
    if (r != 0) {
        func_0011D6A8();
    }
}
