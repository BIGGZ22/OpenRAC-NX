/* NON_MATCHING func_L06_00303858 -- src/overlays/l06_blarg/vendor_002FE5D0.c
 * Best so far: SIZE ours 352 / retail 348, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   func_L06_00303858: picks an id from the moby's data block by mode (0/1/2; mode 1 does a round-robin over a 3-s
 *   Best is p3.c (BYTES 88/348, same size; data as a struct with int a[6], b[6] gives retail's base-first addu in 
 *   Would need a source shape making n*4 and data+0x68 loop-invariant locals only in case 1 without the allocator 
 */
extern int D_0015EE88 MACRO_ADDR;
extern char D_0014171B[];
extern short D_L06_00162148;
extern int D_L06_0016214C;
extern int D_L06_00162150;
extern int func_002140B0(int);

typedef struct { char pad0[0x38]; int a[6]; int b[6]; } Pick;
// picks an id for the moby by mode from its data block and stores it offset by 20000
void func_L06_00303858(char *moby, int mode) {
    int n = D_0015EE88;
    Pick *data;
    int id;
    if (n != 0) n = n - 1;
    data = *(Pick **)(moby + 0x78);
    id = -1;
    switch (mode) {
    case 0:
        id = data->a[n];
        break;
    case 1: {
        int *t;
        int cnt = 0;
        int b;
        id = func_002140B0(3);
        t = (int *)&D_L06_00162148;
        if (t[id] == 0) {
            t[id] = mode;
        } else {
            b = id + 1;
            for (;;) {
                id = b % 3;
                if (cnt == 2) {
                    *(int *)&D_L06_00162148 = 0;
                    D_L06_0016214C = 0;
                    D_L06_00162150 = 0;
                }
                cnt++;
                if (cnt >= 4) break;
                if (t[id] == 0) {
                    t[id] = 1;
                    break;
                }
                b = id + 1;
            }
        }
        id = *(int *)((char *)data + 0x68 + (id * 0x18 + n * 4));
        break;
    }
    case 2:
        id = data->b[n];
        break;
    }
    if (id != -1) *(int *)(D_0014171B + 0x100D1) = id + 0x4E20;
}
