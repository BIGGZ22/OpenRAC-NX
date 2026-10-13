/* Carried from port/game/rac1-pal/hand/game_joint_matrices.c through the version map (../README.md): names and
 * addresses are this version's, raw addresses in comments are PAL's. Not yet checked in play. */
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native C for PAL's handwritten joint-matrix selector, 002116A0..00211804.
 * This is not a compiler match. The existing native 00211808 evaluates the
 * marked chains into scratchpad; requested terminal matrices retain their
 * input order, including duplicates. The maximum TERMINAL joint, not the
 * maximum dependency, supplies the terminating 0xff mark and byte 0x7f.
 */
extern unsigned char joint_marks[] __asm__("D_001B2D00");
extern unsigned char joint_scratch[] __asm__("D_70000000");
extern void joint_evaluate(void*, void*) __asm__("func_002109B8");

void FUN_00210850(void* object, int count, int* indices, void* matrices) {
    unsigned char* moby = (unsigned char*)object;
    unsigned char* out = (unsigned char*)matrices;
    unsigned char* model = *(unsigned char**)(moby + 0x24);
    unsigned char** chains = *(unsigned char***)(model + 0x1C);
    unsigned char matrix[64];
    int i, j, maximum = 0;
    for (i = 0; i < 128; ++i) {
        joint_marks[i] = 0;
    }
    i = 0;
    /* Retail uses do/while for both counts, even for a zero input count. */
    do {
        unsigned char* chain = chains[indices[i] + 1];
        int length = *(unsigned short*)chain;
        int terminal;
        j = 0;
        do {
            terminal = chain[4 + j];
            joint_marks[terminal] = 1;
            ++j;
        } while (j < length);
        joint_marks[128 + i] = (unsigned char)terminal;
        if (terminal > maximum) {
            maximum = terminal;
        }
        ++i;
    } while (i < count);
    joint_marks[127] = (unsigned char)(maximum + 1);
    joint_marks[maximum] = 255;
    joint_evaluate(moby, joint_marks);
    i = 0;
    do {
        unsigned char* source = joint_scratch + joint_marks[128 + i] * 64;
        /* Four quadword loads precede all four stores in retail. */
        for (j = 0; j < 64; ++j) {
            matrix[j] = source[j];
        }
        for (j = 0; j < 64; ++j) {
            out[i * 64 + j] = matrix[j];
        }
        ++i;
    } while (i < count);
}
