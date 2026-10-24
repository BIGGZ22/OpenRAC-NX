void Cheats_PollCodes(void) {
    char *g;
    int s;
    int i;
    if (D_0015EFA0 == 0 && D_0015EF20 == 0) return;
    g = (char *)D_0013E633 + 0xE1D;
    s = 0;
    if (*(int *)(g + 0x198) == scale_ticks(0xF)) {
        if (*(int *)(g + 0x208C) == 4) {
            int b = *(int *)(g + 0x2084);
            if (b == 0xB) {
                int c = *(int *)(g + 0x450);
                if (c == 0) {
                    s = 1;
                    if (D_0015EEB4[0]) s = 2;
                } else if (c == 1) {
                    s = 2;
                    if (D_0015EEB4[0]) s = 1;
                } else if (c == 3) {
                    s = 3;
                }
            } else if (b == 0x11) {
                s = 9;
            } else if (b == 0xA) {
                s = 0xA;
            } else if (b == 0xE) {
                s = 5;
            }
        } else if (*(int *)(g + 0x208C) == 6) {
            int b = *(int *)(g + 0x2084);
            if (b == 0x14) {
                s = 6;
            } else if (b == 0x13) {
                s = *(int *)(g + 0xA60) + 0xC;
            } else if (b == 0x15) {
                s = 4;
            }
        } else if (*(int *)(g + 0x2084) == 0x22) {
            s = 0xB;
        }
    } else if (*(int *)(g + 0x198) == scale_ticks(0x3C)) {
        if (*(int *)(g + 0x2084) == 4) {
            s = 7;
        } else if (*(int *)(g + 0x2084) == 8) {
            s = 8;
        }
    }
    if (s == 0) return;
    {
    char *q = (char *)D_0013E633 + 0xE1D;
    unsigned char *b0 = (unsigned char *)q + 0x21C4;
    b0[*(int *)(q + 0x21D4)] = s;
    *(int *)(q + 0x21D4) += 1;
    for (i = 0; i < 12; i++) {
        char *g2 = (char *)D_0013E633 + 0xE1D;
        unsigned char *b = (unsigned char *)g2 + 0x21C4;
        int n = 0;
        int j;
        int k;
        while (D_L00_00179800[i][n] != 0) n++;
        if (n == 0) continue;
        k = *(int *)(g2 + 0x21D4) - n;
        if (k < 0) k += 16;
        for (j = 0; j < n; j++) {
            if (*(unsigned char *)(k + (int)b) != D_L00_00179800[i][j]) {
                j = -1;
                break;
            }
            k++;
            if (k >= 16) k -= 16;
        }
        if (j == n) {
            int t = gCheats[i] == 0;
            selector_available[i] = 1;
            gCheats[i] = t;
            ShowBanner(t ? 0x4FBF : 0x4FC0, -1);
            break;
        }
    }
    }
    {
        char *r = (char *)D_0013E633 + 0xE1D;
        if (*(int *)(r + 0x21D4) >= 16) *(int *)(r + 0x21D4) = 0;
    }
}
