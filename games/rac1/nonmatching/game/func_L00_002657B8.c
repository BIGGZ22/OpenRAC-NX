extern char *D_L00_001ABD80[];
extern char *D_L00_001600A4;
extern char *D_L00_001600A4_sd __asm__("D_L00_001600A4") MACRO_ADDR;
extern char *D_L00_00160098;
extern void func_L00_002514B8(void *);
extern void func_L00_00251E30(void *);

/* Walks the moby table (0x100-byte entries from D_L00_00160098) and rebuilds the
   list headed at D_L00_001600A4 (linked at +0x28) from the entries that are live
   (flag +0x20 has no 0x80 bit, no 0x2 in +0x34) until the 0xFF end marker; then
   for each listed moby: a negative +0x20 skips it, else MobyAnimAdvance unless
   +0x34 & 0x40, the callback at +0x74 if set, and MobyBuildMatrix unless +0x34 & 4. */
void func_L00_002657B8(void) {
    char *q;
    char *prev;
    char *cur;
    void (*fn)(void *);

    D_L00_001ABD80[0] = 0;
    D_L00_001600A4 = 0;
    q = D_L00_00160098;
    prev = 0;
    if ((unsigned char)q[0x20] != 0xFF) {
        do {
            if ((unsigned char)q[0x20] & 0x80) {
                q += 0x100;
            } else if (*(unsigned short *)(q + 0x34) & 0x2) {
                q += 0x100;
            } else {
                if (prev == 0) {
                    D_L00_001600A4_sd = q;
                } else {
                    *(char **)(prev + 0x28) = q;
                }
                prev = q;
                q += 0x100;
            }
        } while ((unsigned char)q[0x20] != 0xFF);
    }
    if (prev != 0) {
        *(char **)(prev + 0x28) = 0;
    }

    cur = D_L00_001600A4;
    while (cur != 0) {
        if ((signed char)cur[0x20] < 0) {
            cur = *(char **)(cur + 0x28);
            continue;
        }
        if (!(*(unsigned short *)(cur + 0x34) & 0x40)) {
            func_L00_002514B8(cur);
        }
        fn = *(void (**)(void *))(cur + 0x74);
        if (fn != 0) {
            fn(cur);
        }
        if (!(*(unsigned short *)(cur + 0x34) & 0x4)) {
            func_L00_00251E30(cur);
        }
        cur = *(char **)(cur + 0x28);
    }
}
