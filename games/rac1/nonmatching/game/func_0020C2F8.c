extern int D_00161000 MACRO_ADDR;
extern int D_0015F558 MACRO_ADDR;
extern unsigned char D_00160860[];
extern unsigned char D_0018D140[];

/* GS upload packets for the texture slots in D_0018D140 (D_0015F558 of them,
   16 bytes each, two 8-byte halves: word addr, u16 at +4 and +6). Each half
   with a non-zero addr appends a 0x70-byte packet at the cursor D_00161000,
   built from five 16-byte templates at D_00160860. The first half writes the
   shifts for its u16 at +4 (halved), the second half takes tw (low byte) and
   th (high byte) of its u16 at +4. The cursor is saved back at the end. */
void func_0020C2F8(void) {
    unsigned char q[0x50];
    unsigned int n;
    unsigned char *e;
    unsigned char *out;
    int i;

    for (i = 0; i < 0x50; i++) {
        q[i] = D_00160860[i];
    }
    n = (unsigned int)D_0015F558;
    e = D_0018D140;
    out = (unsigned char *)D_00161000;

    while (n != 0) {
        int h;
        n--;
        for (h = 0; h < 2; h++) {
            unsigned char *f = e + 8 * h;
            unsigned int addr = *(unsigned int *)f;
            unsigned int v2 = *(unsigned short *)(f + 4);
            unsigned int v9 = *(unsigned short *)(f + 6);
            unsigned int tw;
            unsigned int th;
            unsigned int m;

            if (h == 0 && addr == 0) {
                continue;
            }

            for (i = 0; i < 0x40; i++) {
                out[i] = q[i];
            }
            for (i = 0x40; i < 0x50; i++) {
                out[i] = 0;
            }
            for (i = 0x50; i < 0x60; i++) {
                out[i] = q[i - 0x10];
            }
            *(unsigned int *)(out + 0x48) = 0x53;
            *(unsigned short *)(out + 0x24) = (unsigned short)v9;

            if (h == 0) {
                out[0x27] = (unsigned char)v2;
                v2 = v2 >> 1;
                m = 0x40u >> (v2 & 31);
                *(unsigned int *)(out + 0x30) = 0x10;
                *(unsigned int *)(out + 0x34) = 0x10;
            } else {
                tw = v2 & 0xFF;
                th = v2 >> 8;
                if ((int)tw - 6 > 0) {
                    out[0x26] = (unsigned char)(1u << (((int)tw - 6) & 31));
                }
                *(unsigned int *)(out + 0x34) = 1u << (th & 31);
                *(unsigned int *)(out + 0x30) = 1u << (tw & 31);
                m = ((1u << (th & 31)) << (tw & 31)) >> 4;
            }

            *(unsigned int *)(out + 0x50) = m | 0x8000;
            for (i = 0x60; i < 0x70; i++) {
                out[i] = 0;
            }
            *(unsigned int *)(out + 0x64) = addr;
            *(unsigned int *)(out + 0x60) = 0x30000000u + m;
            *(unsigned int *)(out + 0x6C) = 0x50000000u + m;
            out += 0x70;
        }
        e += 16;
    }
    D_00161000 = (int)out;
}
