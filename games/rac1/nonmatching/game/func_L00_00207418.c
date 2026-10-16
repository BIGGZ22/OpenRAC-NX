/* Per-frame pattern recorder for the help overlay. While D_0015EFA0 or D_0015EF20 is set, it
 * reads two state words of the 0x13F450 block (through func_001F9850) and picks an op code from
 * them (0 = none). An op is appended to a 16-entry ring (counter at +0x21D4, entries at +0x21C4).
 * Then the 12 NUL-terminated patterns at D_L00_00179800 (16 bytes each) are compared against the
 * newest ops; the first pattern that ends the ring fires: its flag D_0015EEB0[i] toggles, its
 * D_0015EEC0[i] is set to 1, and func_L00_00264DB8 is called. The counter wraps at 16 after a
 * recording. */
extern int D_0015EFA0 NOT_SDA;
extern int D_0015EF20 NOT_SDA;
extern unsigned char D_L00_00179800[];
extern unsigned char D_0015EEB0[] MACRO_ADDR;
extern unsigned char D_0015EEC0[] MACRO_ADDR;
extern unsigned char D_0015EEB4_m[4] __asm__("D_0015EEB4") MACRO_ADDR;
extern void func_L00_00264DB8(int, int);

void func_L00_00207418(void) {
    char *q;
    int r;
    int op = 0;
    int v8c;
    int v84;
    int i;
    int k;
    int len;
    int start;
    int ok;

    if (D_0015EFA0 == 0 && D_0015EF20 == 0) {
        return;
    }
    q = (char *)D_0013E633 + 0xE1D;
    r = func_001F9850(0xF);
    if (*(int *)(q + 0x198) == r) {
        v8c = *(int *)(q + 0x208C);
        if (v8c == 4) {
            v84 = *(int *)(q + 0x2084);
            if (v84 == 0xB) {
                int v450 = *(int *)(q + 0x450);
                if (v450 == 0) {
                    op = D_0015EEB4_m[0] ? 2 : 1;
                } else if (v450 == 1) {
                    op = D_0015EEB4_m[0] ? 1 : 2;
                } else if (v450 == 3) {
                    op = 3;
                } else {
                    op = 0;
                }
            } else if (v84 == 0x11) {
                op = 9;
            } else if (v84 == 0xA) {
                op = 0xA;
            } else {
                op = (v84 == 0xE) ? 5 : 0;
            }
        } else if (v8c == 6) {
            v84 = *(int *)(q + 0x2084);
            if (v84 == 0x14) {
                op = 6;
            } else if (v84 == 0x13) {
                op = *(int *)(q + 0xA60) + 0xC;
            } else {
                op = (v84 == 0x15) ? 4 : 0;
            }
        } else {
            v84 = *(int *)(q + 0x2084);
            op = (v84 == 0x22) ? 0xB : 0;
        }
    } else {
        r = func_001F9850(0x3C);
        if (*(int *)(q + 0x198) == r) {
            v84 = *(int *)(q + 0x2084);
            if (v84 == 4) {
                op = 7;
            } else if (v84 == 8) {
                op = 8;
            } else {
                op = 0;
            }
        }
    }

    if (op != 0) {
        unsigned char *buf = (unsigned char *)(q + 0x21C4);
        int idx = *(int *)(q + 0x21D4);
        buf[idx] = (unsigned char)op;
        *(int *)(q + 0x21D4) = idx + 1;

        for (i = 0; i < 12; i++) {
            unsigned char *s = D_L00_00179800 + i * 16;
            len = 0;
            while (s[len] != 0) {
                len++;
            }
            if (len == 0) {
                continue;
            }
            start = *(int *)(q + 0x21D4) - len;
            if (start < 0) {
                start += 16;
            }
            ok = 1;
            for (k = 0; k < len; k++) {
                if (buf[(start + k) % 16] != s[k]) {
                    ok = 0;
                    break;
                }
            }
            if (ok) {
                unsigned char old = D_0015EEB0[i];
                D_0015EEC0[i] = 1;
                D_0015EEB0[i] = (unsigned char)(old == 0);
                func_L00_00264DB8((old == 0) ? 0x4FBE : 0x4FBF, -1);
                break;
            }
        }
        if (*(int *)(q + 0x21D4) >= 16) {
            *(int *)(q + 0x21D4) = 0;
        }
    }
}
