/* Carried from port/game/rac1-pal/hand/game_scale_matrix.c through the version map (../README.md): names and
 * addresses are this version's, raw addresses in comments are PAL's. Chunk 8 (2026-10-17, checked against the
 * US assembly): FUN_001f9ff8 medium: four rows stored at matrix+0/16/32/48 in the retail order, with the scale
 * on the diagonal and (0, 0, 0, 1) as the fourth row, as here; the VU lane arithmetic for the zero signs was not
 * decoded op by op. Not yet checked in play. */
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
/* Native-only PAL 001FA1C0..001FA1F4: the VU constructor writes a complete
 * uniform-scale matrix, including its translation/homogeneous row.
 * Not a matching PS2 decompilation. */
void FUN_001f9ff8(float* matrix, float scale) {
    int i;
    for (i = 0; i < 16; ++i) matrix[i] = 0.0f;
    /* Retail adds the input scalar to cleared lanes, including for -0. */
    matrix[0] = matrix[5] = matrix[10] = 0.0f + scale;
    matrix[15] = 1.0f;
}
