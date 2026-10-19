/* functional: medium. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Handwritten MMI (MPEG motion compensation, 8-wide vertical half-pel): each row is 8 bytes from src1 and 8 from src2 joined and rotated right by (shift & 15), the low 8 bytes widened to halfwords; output row = (previous row + this row + 1) >> 1, one 16-byte store. Two passes (source +0x40, dest +0x80 for the second); each runs a[2] rows, then a second block of a[3] rows (source +0x140) when the low 16 bits of a[3] are nonzero (retail builds that test with a halfword shift, so only 16 bits count). MMI becomes plain halfword code; field meanings inferred from the registers. */
typedef struct {
    u16 *dst;   /* 0x00 */
    s32 shift;  /* 0x04 */
    s32 rows;   /* 0x08 */
    s32 rows2;  /* 0x0C */
    s32 stride; /* 0x10 */
    u8 *src1;   /* 0x14 */
    u8 *src2;   /* 0x18 */
} MC_126790;

static void load_126790(u16 *out, const u8 *s1, const u8 *s2, s32 n) {
    u8 c[16];
    s32 i;
    for (i = 0; i < 8; i++) {
        c[i] = s1[i];
        c[8 + i] = s2[i];
    }
    for (i = 0; i < 8; i++) {
        out[i] = c[(n + i) & 15];
    }
}

void func_00126790(MC_126790 *a) {
    s32 pass;
    s32 n = a->shift & 15;
    s32 stride = a->stride;
    for (pass = 0; pass < 2; pass++) {
        u8 *s1 = a->src1;
        u8 *s2 = a->src2;
        u8 *d = (u8 *)a->dst;
        s32 cnt = a->rows;
        s32 i;
        u16 prev[8];
        u16 cur[8];
        s32 second = 0;
        if (pass) {
            s1 += 0x40;
            s2 += 0x40;
            d += 0x80;
        }
        load_126790(prev, s1, s2, n);
        if (cnt != 0) {
            goto block;
        }
        for (;;) {
            s1 += 0x140;
            s2 += 0x140;
            if (second || (a->rows2 & 0xFFFF) == 0) {
                break;
            }
            second = 1;
            cnt = a->rows2;
        block:
            do {
                s1 += stride;
                s2 += stride;
                load_126790(cur, s1, s2, n);
                cnt--;
                for (i = 0; i < 8; i++) {
                    ((u16 *)d)[i] = (u16)(cur[i] + prev[i] + 1) >> 1;
                    prev[i] = cur[i];
                }
                d += stride * 2;
            } while (cnt > 0);
        }
    }
}
