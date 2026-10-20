#include "common.h"
#include "structs.h"

extern void func_0012C3B0(void *);

extern int func_0012C680(void *, int);

extern void func_0012C788(void *, int);

extern void func_0012C9C8(void *);

extern int func_0012CA48(void *);

extern int func_0012C878(void *, int);

extern void func_0012CFA0(void *);

/* rac1-decomp src/core/00125630.c: func_00128BA8 */
void func_0012C9C8(void *arg0) {
    int pad;

    func_0012C3B0(arg0);
    pad = -(*(volatile int *)0x10002020 & 7) & 7;
    if (pad != 0) {
        func_0012C788(arg0, pad);
    }
    while (func_0012C680(arg0, 0x18) != 1) {
        func_0012C788(arg0, 8);
    }
}

/* rac1-decomp src/core/00125630.c: func_00128C28 */
int func_0012CA48(void *arg0) {
    *(int *)((char *)arg0 + 0x1B4) = func_0012C878(arg0, 5);
    if (func_0012C878(arg0, 1) != 0) {
        func_0012C878(arg0, 1);
        func_0012C788(arg0, 7);
        func_0012CFA0(arg0);
    }
    return 0;
}


extern int r1_func_00128A58_0012CBC0(void *, int);

extern void r1_func_00129180_0012CBC0(void *);

extern void func_0012CBC0(void *);

extern void r1_func_00128E68_0012CBC0(void *);

extern void r1_func_001291C8_0012CBC0(void *, int);

extern int r1_func_00128A58_0012CBC0();

extern void r1_func_00128E68_0012CBC0();

extern void r1_func_00129180_0012CBC0();

extern void r1_func_001291C8_0012CBC0();

/* rac1-decomp src/core/00125630.c: func_00128DA0 */
/* pictureHeader (libmpeg.a:mpc.o): ISO/IEC 13818-2 6.2.3 picture_header().
 * temporal_reference (+bits 10) is kept live across the whole function to
 * feed r1_func_001291C8_0012CBC0 (updateTempTackData) at the end. picture_coding_type
 * (+0x150) is re-read from the struct at each use rather than kept in a
 * local -- it must survive calls that could (as far as the compiler
 * knows) touch the struct. P_TYPE/B_TYPE (2/3) is the classic
 * (x-2)<2u range check; B_TYPE alone is a plain ==3.
 */
void func_0012CBC0(void *arg0) {
    char *s = (char *)arg0;
    int temporal_reference = r1_func_00128A58_0012CBC0(s, 10);

    *(int *)(s + 0x150) = r1_func_00128A58_0012CBC0(s, 3);   /* picture_coding_type */
    r1_func_00128A58_0012CBC0(s, 16);                         /* vbv_delay, discarded */

    if ((unsigned int)(*(int *)(s + 0x150) - 2) < 2) {
        *(int *)(s + 0x154) = r1_func_00128A58_0012CBC0(s, 1);  /* full_pel_forward_vector */
        *(int *)(s + 0x158) = r1_func_00128A58_0012CBC0(s, 3);  /* forward_f_code */
    }
    if (*(int *)(s + 0x150) == 3) {
        *(int *)(s + 0x15C) = r1_func_00128A58_0012CBC0(s, 1);  /* full_pel_backward_vector */
        *(int *)(s + 0x160) = r1_func_00128A58_0012CBC0(s, 3);  /* backward_f_code */
    }

    r1_func_00129180_0012CBC0(s);   /* extra_bit_information */
    r1_func_00128E68_0012CBC0(s);   /* extension_and_user_data */
    r1_func_001291C8_0012CBC0(s, temporal_reference);  /* Update_Temporal_Reference_Tacking_Data */
}

INCLUDE_ASM("asm/core", func_0012CC88);

INCLUDE_ASM("asm/core", func_0012CDB0);
