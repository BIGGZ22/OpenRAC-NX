/* NON_MATCHING func_L18_002F1420 -- src/overlays/l18_veldin2/vendor_002A8400.c
 * Best so far: BYTES 20/240 (91.7% of the bytes match), checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   ## Round 4 (hand, 2026-10-04): joined function, 20/240 bytes off
 *   func_L18_002F1420 and func_L18_002F1444 are ONE function (config/overlays/joined.tsv; the
 *   check compares all 240 bytes). Written under the first name: `p = D_L18_001AC540[idx]; if (!p)
 *   return 1; ret = 1;` then the list walk. What took it from 134 to 20 bytes: the list walk must
 *   be a `goto` back-edge (`again: ... if (*p++ >= 0) goto again;`). A do/while gets loop.c, which
 *   hoists the constants and the moby table base out of the loop (retail keeps them inside) and
 *   changes the registers. Left: p/data and val/ret swap registers ($t0/$t1, $t3/$t4); source
 *   order of declarations does not change it.
 */
int func_L18_002F1420(int val, int idx, void *src) {
    short *p = (short *)D_L18_001AC540[idx];
    int ret;
    if (p == 0) {
        return 1;
    }
    ret = 1;
again:
    {
        char *m = D_L18_00160058_m + ((*(unsigned short *)p & 0x7FFF) << 8);
        if (*(short *)(m + 0xA6) == 0x54B && ((unsigned char *)m)[0x20] == 1) {
            char *data = *(char **)(m + 0x78);
            int n;
            ret = 0;
            qcopy(m + 0x10, src);
            n = 0;
            do {
                qcopy(data + 0x10 + n * 16, m + 0x10);
                n++;
            } while ((float)n < 4.0f);
            *(int *)(data + 0x16C) = val;
            m[0x31] = 1;
            *(unsigned short *)(m + 0x34) &= 0xFFFE;
            *(int *)(m + 0x94) = *(int *)(*(char **)(m + 0x24) + 0x10);
            m[0x20] = 2;
        }
    }
    if (*p++ >= 0) goto again;
    return ret;
}
