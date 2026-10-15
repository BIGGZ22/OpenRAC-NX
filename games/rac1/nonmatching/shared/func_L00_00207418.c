/* NON_MATCHING func_L00_00207418 -- src/overlays/shared/help_00203E98.c
 * Best so far: SIZE ours 716 / retail 720, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Cheat-code/input-sequence recorder: decides a code s (1..0xC) from game state words at base+0x198/0x208C/0x208
 *   p1 is the same size (720) with the right structure; stopped with 3 runs unspent because the diff is allocator/
 *   retail shares constant 4 (A==4 compare reg reused for s=4) but not 0xA (ours does the reverse), and the loop h
 */
extern int D_0015EFA0 MACRO_ADDR;
extern int D_0015EF20 MACRO_ADDR;
extern unsigned char D_0015EEB4_m[4] __asm__("D_0015EEB4") MACRO_ADDR;
extern unsigned char D_0015EEB0[] MACRO_ADDR;
extern unsigned char D_0015EEC0[] MACRO_ADDR;
extern unsigned char D_L00_00179800[];
extern void func_L00_00264DB8(int arg0, int arg1);

// Records a pressed-input code in a 16-entry ring buffer and toggles any cheat whose sequence now ends the buffer.
void func_L00_00207418(void) {
    char *g;
    char *h;
    int s;
    int i;
    if (D_0015EFA0 == 0 && D_0015EF20 == 0) return;
    g = (char *)D_0013E633 + 0xE1D;
    h = (char *)D_0013E633 + 0xE1D;
    s = 0;
    if (*(int *)(g + 0x198) == func_001F9850(0xF)) {
        if (*(int *)(g + 0x208C) == 4) {
            int b = *(int *)(g + 0x2084);
            if (b == 0xB) {
                int c = *(int *)(g + 0x450);
                if (c == 0) {
                    s = 1;
                    if (D_0015EEB4_m[0]) s = 2;
                } else if (c == 1) {
                    s = 2;
                    if (D_0015EEB4_m[0]) s = 1;
                } else {
                    s = 3;
                    if (c != 3) s = 0;
                }
            } else if (b == 0x11) {
                s = 9;
            } else if (b == 0xA) {
                s = 0xA;
            } else {
                s = 5;
                if (b != 0xE) s = 0;
            }
        } else if (*(int *)(g + 0x208C) == 6) {
            int b = *(int *)(g + 0x2084);
            if (b == 0x14) {
                s = 6;
            } else if (b == 0x13) {
                s = *(int *)(g + 0xA60) + 0xC;
            } else {
                s = 4;
                if (b != 0x15) s = 0;
            }
        } else {
            s = 0xB;
            if (*(int *)(g + 0x2084) != 0x22) s = 0;
        }
    } else if (*(int *)(g + 0x198) == func_001F9850(0x3C)) {
        if (*(int *)(g + 0x2084) == 4) {
            s = 7;
        } else {
            s = 8;
            if (*(int *)(g + 0x2084) != 8) s = 0;
        }
    }
    if (s == 0) return;
    g = h;
    g[0x21C4 + *(int *)(g + 0x21D4)] = s;
    *(int *)(g + 0x21D4) += 1;
    for (i = 0; i < 12; i++) {
        unsigned char *e = D_L00_00179800 + i * 16;
        int n = 0;
        int j;
        int k;
        if (e[0] != 0) {
            unsigned char *p = e;
            do {
                p++;
                n++;
            } while (*p != 0);
        }
        if (n == 0) continue;
        k = *(int *)(g + 0x21D4) - n;
        if (k < 0) k += 16;
        j = 0;
        if (n > 0) {
            if (((unsigned char *)g + 0x21C4)[k] != e[0]) {
                j = -1;
            } else {
                k++;
                for (;;) {
                    j++;
                    if (k >= 16) k -= 16;
                    if (!(j < n)) break;
                    if (((unsigned char *)g + 0x21C4)[k] != e[j]) {
                        j = -1;
                        break;
                    }
                    k++;
                }
            }
        }
        if (j == n) {
            int t = D_0015EEB0[i] == 0;
            D_0015EEC0[i] = 1;
            D_0015EEB0[i] = t;
            func_L00_00264DB8(t ? 0x4FBE : 0x4FBF, -1);
            break;
        }
    }
    g = h;
    if (*(int *)(g + 0x21D4) >= 16) *(int *)(g + 0x21D4) = 0;
}
