/* NON_MATCHING func_L00_00258250 -- src/overlays/shared/mobyproc_00251A78.c
 * Best so far: SIZE ours 584 / retail 596, checked 2026-10-15.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   2026-10-15: compiles again (D_0015EE84 is declared through an alias). This is the ammo crate: it counts
 *   owned items (D_0013D5C8[e]) whose ammo (D_0013D530[e]) is below the maximum (D_L00_001C43B0[e].v), picks
 *   one at random and stores its id. The save bit is D_0014C290[level][save_id >> 5]. Written with those
 *   arrays instead of offsets from D_0013D50F and D_0014171B: still SIZE 584/596. Left: retail selects the
 *   final 1 or 2 with movn from 2, keeps the &D_0015EED0 index sum in another order and has three more nops.
 */
extern int D_0015EE84_258250 __asm__("D_0015EE84") MACRO_ADDR;
extern int func_002140B0(int);
extern unsigned char D_0015EED0[] MACRO_ADDR;
struct Ent { char pad[0xE]; unsigned short v; char pad2[8]; };
extern struct Ent D_L00_001C43B0[];
extern unsigned char D_0013D5C8[];
extern int D_0013D530[];
extern int D_0014C290[][64];

/* picks an index into a list of up to 12 weighted entries and stores it; returns 1 or 2 */
int func_L00_00258250(char *a, int *out) {
    int n10 = 0, n11 = 0, n9 = 0;
    int k, r7, res;
    if (*out <= 0 || (a != 0 && (D_0014C290[D_0015EE84_258250][(short)*(unsigned short *)(a + 0xB2) >> 5] >> (*(unsigned short *)(a + 0xB2) & 0x1F) & 1))) {
        int i;
        for (i = 0; i < 12 && D_0015EED0[i] != 0xFF; i++) {
            int e = D_0015EED0[i] & 0x3F;
            if (D_0013D5C8[e]) {
                int h = D_L00_001C43B0[e].v;
                int w = D_0013D530[e];
                n11++;
                if (h) n9++;
                n10 += w < h;
            }
        }
        k = -1;
        if (n10 != 0) {
            r7 = func_002140B0(n10);
            while (r7 >= 0) {
                int e;
                k++;
                e = D_0015EED0[k] & 0x3F;
                if (D_0013D5C8[e]) {
                    int h = D_L00_001C43B0[e].v;
                    int w = D_0013D530[e];
                    r7 -= w < h;
                }
            }
            res = D_0015EED0[k] & 0x3F;
        } else {
            res = 10;
            if (n11 != 0) {
                r7 = func_002140B0(n9);
                while (r7 >= 0) {
                    int e;
                    k++;
                    e = D_0015EED0[k] & 0x3F;
                    if (D_0013D5C8[e]) {
                        int h = D_L00_001C43B0[e].v;
                        if (h) r7--;
                    }
                }
                res = D_0015EED0[k] & 0x3F;
            }
        }
        *out = res;
    }
    res = 2;
    if (func_002140B0(5)) res = 1;
    return res;
}
