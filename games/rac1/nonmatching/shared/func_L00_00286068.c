/* NON_MATCHING func_L00_00286068 -- src/overlays/shared/pause_00277208.c
 * Best so far: SIZE ours 100 / retail 108, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Searches row i (stride 0x100 off D_0014171B+0xBF75) of 64 four-byte entries from slot 63 down for a short equa
 *   Difference: SIZE 80-84 vs 108. Retail peels the first iteration (lh 0xFC($3) before the pointer add, then the 
 *   Unblock: whatever makes gcc 2.95 peel the entry iteration of this top-tested countdown loop; likely a source s
 *   mini7/a02: actual global is D_0014D690. Cached current-value while loop p12/p14 improves to 96/108 bytes and r
 *   Eight runs exhausted. p14 retains first lh at +0xFC; missing two typed-pointer moves, an earlier cursor+0xFC a
 *   hq3 s02 (6 runs, p15-p20): short-pointer slot stepping back 2 shorts, peeled first iteration, for-over-k with 
 */
extern unsigned char D_0014D690[] NOT_SDA;
/* finds a matching or free short slot in a row, scanning backward */
int func_L00_00286068(int i, int v) {
    int k = -1;
    int want;
    short *s;
    if (v < 0) {
        return k;
    }
    want = v + 1;
    s = (short *)(D_0014D690 + (i << 8) + 0xFC);
    k = 63;
    while (k >= 0 && *s != want && *s != 0) {
        k--;
        s -= 2;
    }
    if (k >= 0) {
        *s = want;
    }
    return k;
}
