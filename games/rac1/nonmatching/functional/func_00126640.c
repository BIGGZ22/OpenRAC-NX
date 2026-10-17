/* functional: medium. Handwritten MMI (MPEG motion compensation, 8-wide copy without averaging): per row, joins 8 bytes of src1 and 8 of src2, rotates the 16 bytes right by (shift & 15), and widens the low 8 bytes to 8 halfwords written as one 16-byte store. Two passes (columns 0 and +0x40 source / +0x80 dest); each pass runs rows (a[2]) rows, then a second block of a[3] rows if a[3] is nonzero. MMI becomes plain byte/halfword code; the field meanings are inferred from the registers. */
typedef struct {
    u16 *dst;   /* 0x00 */
    s32 shift;  /* 0x04 */
    s32 rows;   /* 0x08 */
    s32 rows2;  /* 0x0C */
    s32 stride; /* 0x10 */
    u8 *src1;   /* 0x14 */
    u8 *src2;   /* 0x18 */
} MC_126640;

static void row_126640(u16 *d, const u8 *s1, const u8 *s2, s32 n) {
    u8 c[16];
    s32 i;
    for (i = 0; i < 8; i++) {
        c[i] = s1[i];
        c[8 + i] = s2[i];
    }
    for (i = 0; i < 8; i++) {
        d[i] = c[(n + i) & 15];
    }
}

void func_00126640(MC_126640 *a) {
    s32 pass;
    s32 n = a->shift & 15;
    s32 stride = a->stride;
    for (pass = 0; pass < 2; pass++) {
        u8 *s1 = a->src1;
        u8 *s2 = a->src2;
        u8 *d = (u8 *)a->dst;
        s32 first = 1;
        s32 cnt = a->rows;
        if (pass) {
            s1 += 0x40;
            s2 += 0x40;
            d += 0x80;
        }
        for (;;) {
            do {
                row_126640((u16 *)d, s1, s2, n);
                cnt--;
                s1 += stride;
                d += stride * 2;
                s2 += stride;
            } while (cnt > 0);
            s1 += 0x140;
            s2 += 0x140;
            cnt = a->rows2;
            if (!first || cnt == 0) {
                break;
            }
            first = 0;
        }
    }
}
