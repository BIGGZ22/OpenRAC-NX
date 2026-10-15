/* Walks the seven 0x50-byte slots at 0x1090 of the hero block (slot i at g + i*0x50).
   An empty slot only gets func_L00_00210558(i). For an object p, the mode at 0x10B4 picks
   the work: mode 2 bumps the word at 0x10B0 and calls func_L00_00210558, and slot 0 also
   visits the three 0xB0-byte entries at 0x1990 (func_L00_00205780 on the ones not at -1);
   mode 3 bumps the byte at 0x10AB and may call func_L00_00210340. Then p's flag bit 1
   (byte 0x70) may run func_00213DE0, and p's callback at 0x74 runs unless byte 0x20 is
   0xFE or 0xFD or the slot is the last one. */
extern void func_L00_00205780(char *p);
extern int D_L00_0015F4F8 MACRO_ADDR;

void func_L00_00210E00(void) {
    char *g = (char *)&D_0013F450_0CDF0;
    int i;

    for (i = 0; i < 7; i++) {
        char *e = g + i * 0x50;
        char *p = *(char **)(e + 0x1090);

        if (p == 0) {
            func_L00_00210558(i);
        } else {
            int mode = *(int *)(e + 0x10B4);

            if (mode == 2) {
                *(int *)(e + 0x10B0) = *(int *)(e + 0x10B0) + 1;
                func_L00_00210558(i);
                if (i == 0) {
                    int k;
                    for (k = 0; k < 3; k++) {
                        char *q = g + k * 0xB0;
                        if (*(short *)(q + 0x1990) != -1) {
                            *(short *)(q + 0x1992) = 5;
                            func_L00_00205780(g + 0x18F0 + k * 0xB0);
                        }
                    }
                }
                if ((*(unsigned char *)(p + 0x70) & 2) != 0) {
                    if (*(unsigned char *)(p + 0x53) == 0) {
                        func_00213DE0(p, 1, 0, 2);
                    }
                }
            } else if (mode == 3) {
                *(unsigned char *)(e + 0x10AB) = (unsigned char)(*(unsigned char *)(e + 0x10AB) + 1);
                if ((*(unsigned char *)(p + 0x70) & 2) != 0 || i == 0 || i == 1) {
                    func_L00_00210340(i, D_L00_0015F4F8 + 2);
                }
            }

            {
                unsigned char t = *(unsigned char *)(p + 0x20);
                if (t != 0xFE && t != 0xFD && i != 6) {
                    void (*cb)(void *) = *(void (**)(void *))(p + 0x74);
                    if (cb != 0) {
                        cb(p);
                    }
                }
            }
        }
    }
}
