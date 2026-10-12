/* NON_MATCHING func_L00_00284410 -- src/overlays/shared/pause_00277208.c
 * Best so far: SIZE ours 508 / retail 516, checked 2026-10-12.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Cheat-code recorder: each pad press (masks 0xF0A0) is appended to a 20-entry short log (D_L00_001BA838); at 20
 *   Best p5.c/p7.c: 508 vs 512 bytes, structure and registers otherwise match (found in $7, locals a/b/c computed 
 *   Would unblock: the source shape that makes the count pseudo a copy of the loaded value; budget spent.
 *   t03/q28: p8 (int n = 0x93 bound), p9 (D[cnt]=k; idx=cnt+1), p10 (nested ifs, no early returns) all compile to 
 *   hq13 s05 (4 runs): p11/p12 (the loop limit 0x93 held in a declared local set before the loop) and p13 (one cou
 */
extern void func_L00_00261848(int);
extern int func_0022EE28(int, int, int);
extern void func_L00_00264DB8(int, int);
extern short D_L00_001BA838[];
extern unsigned char D_L00_001B9F20[];
extern short D_L00_001604B8;
extern char D_0013D5EB[];
extern unsigned char D_0013D50F[];
extern char D_0013D355[];

/* cheat-code entry: records pad presses and on 20 checks the sequence against a table */
void func_L00_00284410(void) {
    char *pad = (char *)D_0013A5E0 + 0x2460;
    int btn, c, cnt, i, j, found, match, a, b, k, idx;
    if ((*(long *)(pad + 0x1C0) & 0xF) == 6) {
        btn = *(int *)(pad + 0x1C4);
        if ((btn & 0xF0A0) == 0) return;
        cnt = *(int *)&D_L00_001604B8;
        if (cnt >= 0x14) return;
        if (btn & 0x1000) k = 0;
        else if (btn & 0x4000) k = 1;
        else if (btn & 0x8000) k = 2;
        else if (btn & 0x2000) k = 3;
        else k = (btn & 0x80) ? 4 : 5;
        idx = cnt;
        D_L00_001BA838[idx] = k;
        idx++;
        *(int *)&D_L00_001604B8 = idx;
        if (idx != 0x14) return;
        found = -1;
        for (i = 2; i < 0x93; i++) {
            match = 1;
            for (j = 0; j < 0x14; j++) {
                if (D_L00_001BA838[j] != D_L00_001B9F20[(j * i + i) & 0xFF]) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                found = i - 2;
                break;
            }
        }
        if (found == -1) return;
        a = found - 0x24;
        b = found - 0x37;
        c = found - 0x3D;
        if (found < 0x25) {
            char *p1 = D_0013D5EB + 5;
            unsigned char *p2 = D_0013D50F + 0xB9;
            p1[found] = 1;
            p2[found] = 1;
        } else if ((unsigned)(found - 0x25) < 0x12) {
            func_L00_00261848(a);
        } else if ((unsigned)b < 6) {
            char *p3 = D_0013D355 + 0x13B;
            p3[b] = 1;
        } else if ((unsigned)c < 0x1E) {
            unsigned char *q = D_0013D50F + 1 + c;
            if (*q) return;
            *q = 1;
            func_0022EE28(1, 0, 0);
            func_L00_00264DB8(0x53DB, -1);
        }
    } else {
        *(int *)&D_L00_001604B8 = 0;
    }
}
