/* Builds the per-group table at D_001B3080 from the index list a2 (a1 entries) and the chain at
 * a0->0x24->0x1C, calls moby_anim_eval_chain, then copies a1 quadwords from scratchpad
 * 0x70000000 + v*64 + 0x30 (v from the table) to a3. */
extern unsigned char D_001B3080[];
extern void func_00211808(void *, void *);

typedef struct { unsigned int w[4]; } __attribute__((aligned(16))) Quad16x;

void func_00211548(void *a0, int a1, void *a2, void *a3) {
    unsigned char *buf = D_001B3080;
    unsigned char *arr2 = buf + 0x80;
    unsigned char *tbl = *(unsigned char **)(*(unsigned char **)((char *)a0 + 0x24) + 0x1C);
    unsigned int *ip = (unsigned int *)a2;
    unsigned char *pz = arr2;
    unsigned char *p2;
    int i, cnt, mx = 0, last;
    unsigned char *out;
    unsigned int v;

    for (i = 0; i < 0x80; i++) buf[i] = 0;

    i = a1;
    do {
        p2 = *(unsigned char **)(tbl + (*ip) * 4 + 4);
        ip++;
        cnt = *(unsigned short *)p2;
        do {
            last = p2[4];
            p2++;
            cnt--;
            buf[last] = 1;
        } while (cnt > 0);
        *pz = (unsigned char)last;
        pz++;
        if (last > mx) mx = last;
    } while (--i > 0);

    buf[0x7F] = (unsigned char)(mx + 1);
    buf[mx] = 0xFF;

    func_00211808(a0, buf);

    i = a1;
    pz = arr2;
    out = (unsigned char *)a3 - 0x10;
    do {
        v = *pz;
        pz++;
        out += 0x10;
        *(Quad16x *)out = *(Quad16x *)(0x70000000u + (v << 6) + 0x30);
    } while (--i > 0);
}
