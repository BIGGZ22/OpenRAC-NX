/* NON_MATCHING func_L08_002EB770 -- src/overlays/l08_batalia/vendor_002EAF48.c
 * Best so far: SIZE ours 508 / retail 524, checked 2026-10-08.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * No longer builds in its file (COMPILE failed, 2026-10-09): match its declarations to the file's first.
 * What the last attempts found:
 *   Controller moby (UpdateMoby_467/472): when D_0013D355+0x13B flag set and data+8<0 it deletes mobys in a list; 
 *   Left: retail loops with `b L7DC; lhu` entry and hoists the 0x1D8 constant into $s2 (loop shaped as while-style
 *   w06 round: p6.c (pointer walk `*p++ >= -1`) and p7.c (`for (;;) { ...; if (*p++ < -1) break; }`) both give 508
 */
extern void func_0020D678(void *);
extern int func_0022ED80(int, int, int);
extern float func_00214D28(float *p, float target, float maxstep);
extern char D_0013D355[];
extern char *D_L08_001AC340[];
extern float D_0015EE6C MACRO_ADDR;

// Moby controller: when triggered, deletes nearby mobys; otherwise eases a value in three states.
void func_L08_002EB770(unsigned char *moby) {
    char *data = *(char **)(moby + 0x78);
    int st;
    if (((unsigned char *)(D_0013D355 + 0x13B))[0x2E] != 0 && *(int *)(data + 8) < 0) {
        short *p = (short *)D_L08_001AC340[moby[0x21]];
        int i = 0;
        if (p != 0) {
            do {
                unsigned char *m = (unsigned char *)D_L08_00160058_m + ((*(unsigned short *)(p + i) & 0x7FFF) << 8);
                if (m[0x20] < 0x7F && m != moby) {
                    unsigned short c = *(unsigned short *)(m + 0xA6);
                    if ((unsigned)(c - 0x24C) < 0xB) {
                        func_0020D678(m);
                    } else if ((short)c == 0x1D8) {
                        func_0020D678(m);
                    }
                }
            } while (p[i++] >= -1);
            func_0020D678(moby);
        }
        return;
    }
    st = moby[0x20];
    switch (st) {
    case 0:
        *(float *)(data + 4) = *(float *)(moby + 0x18);
        moby[0x20] = 1;
        break;
    case 1: {
        int k = *(int *)(data + 8);
        if (k >= 0) {
            unsigned char *m = *(unsigned char **)&D_L08_00160058 + (k << 8);
            short c = *(short *)(m + 0xA6);
            if (c == 0x267 || c == 0x23F) {
                if (m[0x20] != 4) return;
                (D_0013D355 + 0x13B)[0x35] = st;
                func_0022ED80(0, 0, (int)moby);
                moby[0x20] = 2;
            } else {
                float f = *(float *)(data + 4);
                func_00214D28((float *)(moby + 0x18), (*(float *)data - f) * **(float **)(m + 0x78) + f, D_0015EE6C * 10.0f);
            }
        }
        break;
    }
    case 2:
        func_00214D28((float *)(moby + 0x18), *(float *)data, D_0015EE6C * 10.0f);
        break;
    }
}
