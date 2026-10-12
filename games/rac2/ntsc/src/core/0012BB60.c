#include "common.h"
#include "structs.h"

extern void func_0012C3B0(void *);

extern void r1_func_0012C468(void *, void *);

#define IPU_CTRL   ((volatile unsigned int *)0x10002010)

/* func_0012C508 returns a short: its epilogue sign-extends $v0. */
extern short func_0012C508(int, int);

extern void func_0012C788(void *, int);

extern int func_0012BB60(void *, int *, int *, int *, void *, void *, void *);

extern char r1_D_00153998[];

extern int func_0012C878(void *, int);

extern void func_0012C230(void *, int *, int *, int, int, int, int, int);

extern void r1_func_00128270_x(void *, int *, int *, int *, int, int, int, int, int, int, int) __asm__("func_0012C090");

extern void func_0012E088(void *, int);

extern void func_0012C380(char *, unsigned int);

typedef struct {
    char pad000[0x11C];
    int unk11C;
    char pad120[0x30];
    int unk150;
    int unk154;
    int unk158;
    int unk15C;
    int unk160;
    int unk164;
    int unk168;
    int unk16C;
    int unk170;
    int unk174;
    char pad178[4];
    int unk17C;
    int unk180;
    char pad184[0x2C];
    int unk1B0;
    int unk1B4;
    char pad1B8[0x810 - 0x1B8];
    int unk810;
    char pad814[0x848 - 0x814];
    int unk848;
} MbDec;

extern void func_0012C008(int *, int, int, int, int);

/* rac1-decomp src/core/00125630.c: func_00127D40 */
/* Decodes one macroblock's modes and motion vectors through the IPU VLC engine (libmpeg _decMB0). Adapted from Lombyte (MIT) for PAL: sdk/library/_decMB0.c, _decMB0. */
int func_0012BB60(void *arg0, int *arg1, int *arg2, int *arg3, void *arg4, void *arg5, void *arg6) {
    MbDec *d = arg0;
    int *mb = arg4;
    int *sp20;
    int *sp24;
    int v;
    int var19;
    int var20;
    int var23;
    int motion_flag;
    int t7;
    int t7b;
    int t4;
    int var103;
    int new_var;
    unsigned int g1;
    unsigned int g2;

    *IPU_CTRL = (*IPU_CTRL & 0xF8FFFFFF) | (d->unk150 << 24);
    sp20 = arg3;
    sp24 = arg5;
    v = func_0012C508((int)d, 1);
    *arg1 = v;
    if (v == 0) {
        r1_func_0012C468(d, r1_D_00153998);
        d->unk11C = 1;
        return 0;
    }
    if (v & 0xC) {
        if (d->unk174 == 3 && d->unk17C != 0) {
            *arg2 = 2;
        } else {
            *arg2 = func_0012C878(d, 2);
        }
    } else if ((v & 1) && d->unk180 != 0) {
        *arg2 = ((d->unk174 ^ 3) == 0) ? 2 : 1;
    }
    if (d->unk174 == 3) {
        var19 = ((*arg2 ^ 1) == 0) ? 2 : 1;
        var20 = *arg2 == 2;
    } else {
        new_var = 2;
        var20 = 0;
        var19 = ((*arg2 ^ new_var) == 0) ? 2 : 1;
    }
    var23 = 0;
    motion_flag = *arg2 == 3;
    if (var20 == 0) {
        var23 = d->unk174 == 3;
    }
    var103 = 0;
    if (d->unk174 == 3 && d->unk17C == 0 && (*arg1 & 3) != 0) {
        var103 = func_0012C878(d, 1);
    } else {
        var103 = 0;
    }
    *sp20 = var103;
    if (*arg1 & 0x10) {
        d->unk1B4 = func_0012C878(d, 5);
    }
    if ((*arg1 & 8) || ((*arg1 & 1) && d->unk180 != 0)) {
        if (d->unk848 != 0) {
            r1_func_00128270_x(d, mb, arg6, sp24, 0, var19, var20, d->unk164 - 1, d->unk168 - 1, motion_flag, var23);
        } else {
            t7 = d->unk158 - 1;
            func_0012C230(d, mb, arg6, t7, t7, 0, 0, d->unk154);
        }
    }
    if (d->unk11C != 0) {
        return 0;
    }
    if (*arg1 & 4) {
        if (d->unk848 != 0) {
            r1_func_00128270_x(d, mb, arg6, sp24, 1, var19, var20, d->unk16C - 1, d->unk170 - 1, 0, var23);
        } else {
            t7b = d->unk160 - 1;
            func_0012C230(d, mb + 2, arg6, t7b, t7b, 0, 0, d->unk15C);
        }
    }
    if (d->unk11C != 0) {
        return 0;
    }
    if ((*arg1 & 1) && d->unk180 != 0) {
        func_0012C788(d, 1);
    }
    if ((*arg1 & 3) != 0) {
        func_0012E088(*(void **)((char *)d + d->unk810 * 0x140 + 0x594), 0x300);
        func_0012C3B0(d);
        g1 = ((*arg1 & 1) << 27) | (d->unk1B4 << 16);
        g2 = (d->unk1B0 << 26) | 0x20000000;
        func_0012C380((char *)d, (g1 | g2) | (*sp20 << 25));
    } else {
        *(int *)((char *)d + d->unk810 * 0x140 + 0x6CC) = 1;
    }
    d->unk1B0 = 0;
    if (d->unk11C != 0) {
        return 0;
    }
    if (((*arg1 & 1) || (d->unk1B0 = 1, (*arg1 & 1) != 0)) && d->unk180 == 0) {
        mb[5] = 0;
        mb[4] = 0;
        mb[1] = 0;
        mb[0] = 0;
        mb[7] = 0;
        mb[6] = 0;
        mb[3] = 0;
        mb[2] = 0;
    }
    t4 = d->unk150;
    if (t4 == 2) {
        if (!(*arg1 & 9)) {
            mb[0] = (mb[1] = (mb[4] = (mb[5] = 0)));
            if (d->unk174 == 3) {
                *arg2 = t4;
            } else {
                *arg2 = 1;
                *sp24 = d->unk174 == 2;
            }
        }
    }
    return 1;
}

/* rac1-decomp src/core/00125630.c: func_001281E8 */
/* decode_motion_vector (libmpeg.a:mpc.o): ISO/IEC 13818-2 motion_vector()
 * component reconstruction, matching mpeg2decode reference source
 * (motion.c) decode_motion_vector() line for line. full_pel_vector is
 * MPEG-1 (ISO/IEC 11172-1) support; called from func_0012C230
 * (motionVector, already matched in this file).
 */
void func_0012C008(int *pred, int r_size, int motion_code, int motion_residual, int full_pel_vector) {
    int lim, vec;

    lim = 16 << r_size;
    vec = full_pel_vector ? (*pred >> 1) : (*pred);

    if (motion_code > 0) {
        vec += ((motion_code - 1) << r_size) + motion_residual + 1;
        if (vec >= lim)
            vec -= lim + lim;
    } else if (motion_code < 0) {
        vec -= ((-motion_code - 1) << r_size) + motion_residual + 1;
        if (vec < -lim)
            vec += lim + lim;
    }
    *pred = full_pel_vector ? (vec << 1) : vec;
}

/* rac1-decomp src/core/00125630.c: func_00128270 */
/* motionVectors (libmpeg.a:mpc.o): ISO/IEC 13818-2 6.2.5.2/6.3.17.2/7.6.3
 * motion_vectors(), matching mpeg2decode reference source (motion.c)
 * almost line for line, with the bitstream-state pointer threaded through
 * explicitly as the new first argument (this file's convention) instead
 * of read from a global. func_0012C230 is motionVector (already matched
 * in this file); func_0012C878 is Get_Bits.
 */
void func_0012C090(void *arg0, int PMV[][2][2], int *dmvector, int mvfs[][2],
                    int s_idx, int motion_vector_count, int mv_format,
                    int h_r_size, int v_r_size, int dmv, int mvscale) {
    if (motion_vector_count == 1) {
        if (mv_format == 0 && dmv == 0) {
            mvfs[1][s_idx] = mvfs[0][s_idx] = func_0012C878(arg0, 1);
        }
        func_0012C230(arg0, PMV[0][s_idx], dmvector, h_r_size, v_r_size, dmv, mvscale, 0);
        PMV[1][s_idx][0] = PMV[0][s_idx][0];
        PMV[1][s_idx][1] = PMV[0][s_idx][1];
    } else {
        mvfs[0][s_idx] = func_0012C878(arg0, 1);
        func_0012C230(arg0, PMV[0][s_idx], dmvector, h_r_size, v_r_size, dmv, mvscale, 0);
        mvfs[1][s_idx] = func_0012C878(arg0, 1);
        func_0012C230(arg0, PMV[1][s_idx], dmvector, h_r_size, v_r_size, dmv, mvscale, 0);
    }
}
