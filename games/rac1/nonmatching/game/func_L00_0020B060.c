/* Four slots of random-angle pairs in the state at D_0013E633 + 0xE1D: each slot i checks
   func_001F9908 on its word at +0x1060 + 4i and, if that is nonzero, draws a float (and a
   sign-folded float for slot 3) and words from the random helpers, then copies the slot's two
   floats (+0x1024, +0x1028, stride 16) into the 176-byte records of D_L00_0017A780 at 13 + i.
   Returns at once when the word at +0x2084 is set or the byte at +0x2080 -> +0x53 is set. */
void func_L00_0020B060(void) {
    unsigned char *g = (unsigned char *)D_0013E633 + 0xE1D;
    unsigned char *B = g + 0x1060;
    unsigned char *A = (unsigned char *)D_L00_0017A780;
    int i;

    if (*(int *)(g + 0x2084) != 0) {
        return;
    }
    if (*(unsigned char *)(*(int *)(g + 0x2080) + 0x53) != 0) {
        return;
    }
    for (i = 0; i < 4; i++) {
        unsigned char *p20 = B + 4 * i;
        unsigned char *p17 = g + 16 * i;
        int rec_index = i + 13;
        float f0;
        float f1;
        int r;

        if (func_001F9908((int *)p20) != 0) {
            if (i == 0 || i == 1) {
                float x = func_L00_00258C80(0.17453292f, 0.52359879f);
                int a, b;
                if (i == 0) {
                    *(float *)(g + 0x1028) = x;
                    a = func_001F9850(70);
                    b = func_001F9850(150);
                    r = func_L00_00258BC8(a, b);
                    *(int *)(D_0013E633 + 0x1E7D) = r;
                } else {
                    *(float *)(B - 0x28) = x;
                    a = func_001F9850(40);
                    b = func_001F9850(90);
                    r = func_L00_00258BC8(a, b);
                    *(int *)(B + 4) = r;
                }
            } else if (i == 2) {
                float x = func_L00_00258C80(0.261799395f, 0.87266463f);
                int a, b;
                *(float *)(B - 0x18) = x;
                a = func_001F9850(40);
                b = func_001F9850(90);
                r = func_L00_00258BC8(a, b);
                *(int *)(B + 8) = r;
            } else {
                float w = func_002140F8(-0.0872664601f, 0.52359879f);
                float old;
                float v;
                int a, b;
                *(float *)(p17 + 0x1024) = w;
                old = *(float *)(p17 + 0x1028);
                v = func_L00_00258C80(0.34906584f, 0.959931076f);
                if ((old <= 0.0f && v <= 0.0f) || (old >= 0.0f && v >= 0.0f)) {
                    v = -v;
                }
                *(float *)(p17 + 0x1028) = v;
                a = func_001F9850(35);
                b = func_001F9850(70);
                r = func_L00_00258BC8(a, b);
                *(int *)(p20) = r;
            }
        }
        f1 = *(float *)(p17 + 0x1028);
        f0 = *(float *)(p17 + 0x1024);
        *(float *)(A + rec_index * 0xB0 + 0x64) = f0;
        *(float *)(A + rec_index * 0xB0 + 0x68) = f1;
    }
}
