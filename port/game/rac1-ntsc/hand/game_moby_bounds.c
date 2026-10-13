/* Carried from port/game/rac1-pal/hand/game_moby_bounds.c through the version map (../README.md): names and
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
/* Native-only PAL 0020EEE8 and the shared 0020ED80 tail: update the
 * animation sphere using the stored basis, regardless of flag 0x100.
 * Decoded from the handwritten retail bodies; not a compiler match.
 */
typedef struct BoundsModel {
    unsigned char pad00[0x48];
    float* sequences[256];
} BoundsModel;
typedef struct BoundsMoby {
    float sphere[4], position[4];
    signed char state;
    unsigned char pad21[3];
    BoundsModel* model;
    unsigned char pad28[4];
    float scale;
    unsigned char pad30[4];
    unsigned short flags;
    unsigned char pad36[0x1A];
    unsigned char snapshot, pad51, sequence, previous;
    float blend;
    unsigned char pad58[0x19], cached_sequence;
    unsigned char pad72[0x22];
    int in_grid;
    unsigned char pad98[8];
    unsigned int grid;
    unsigned char padA4[4];
    unsigned short revision;
    unsigned char padAA[0x16];
    float basis[3][4], cached_sphere[4];
} BoundsMoby;

extern float bounds_snapshots[][4] __asm__("D_001B2C00");
extern void bounds_grid(void*, int) __asm__("func_0020DC20");

/* Same saturating conversion convention as the native collision code. */
static int bounds_integer(float f) {
    if (f != f) return 0;
    if (f >= 2147483648.0f) return 0x7FFFFFFF;
    if (f <= -2147483648.0f) return (-2147483647 - 1);
    return (int)f;
}

void FUN_0020e098(void* object) {
    BoundsMoby* m = (BoundsMoby*)object;
    float local[4], scaled[4], rows[3][4];
    float *current, *previous;
    unsigned int x, y, radius, packed;
    int i, j;
    if (m->state < 0) return;
    for (i = 0; i < 3; ++i) for (j = 0; j < 4; ++j) rows[i][j] = m->basis[i][j];
    if (m->sequence != m->previous) {
        current = m->sequence == 255 ? bounds_snapshots[m->snapshot] : m->model->sequences[m->sequence];
        previous = m->model->sequences[m->previous];
        for (i = 0; i < 4; ++i)
            local[i] = (previous[i] * m->blend + current[i]) - current[i] * m->blend;
    } else if (m->sequence != m->cached_sequence) {
        m->cached_sequence = m->sequence;
        current = m->model->sequences[m->sequence];
        for (i = 0; i < 4; ++i) local[i] = m->cached_sphere[i] = current[i];
    } else {
        for (i = 0; i < 4; ++i) local[i] = m->cached_sphere[i];
    }
    if (m->flags & 0x8000) for (i = 0; i < 3; ++i) rows[1][i] = 0.0f - rows[1][i];
    for (i = 0; i < 3; ++i) for (j = 0; j < 4; ++j) m->basis[i][j] = rows[i][j];
    for (i = 0; i < 4; ++i) scaled[i] = local[i] * m->scale;
    for (i = 0; i < 3; ++i) {
        float value = rows[0][i] * scaled[0];
        value = value + rows[1][i] * scaled[1];
        value = value + rows[2][i] * scaled[2];
        m->sphere[i] = value + m->position[i] * 1024.0f;
    }
    m->sphere[3] = scaled[3];
    ++m->revision;
    if (!m->in_grid) return;
    x = (unsigned int)bounds_integer(m->sphere[0]);
    y = (unsigned int)bounds_integer(m->sphere[1]);
    radius = (unsigned int)bounds_integer(scaled[3]);
    /* Packed arithmetic wraps as four 32-bit lanes. Only the low byte of
     * each shifted result survives, so a logical shift is equivalent here. */
    packed = (((x - radius) >> 14) & 255)
        | ((((y - radius) >> 14) & 255) << 8)
        | ((((x + radius) >> 14) & 255) << 16)
        | ((((y + radius) >> 14) & 255) << 24);
    /* Retail compares the zero-extended packed word to a sign-extended lw. */
    if (!(m->grid & 0x80000000u) && packed == m->grid) return;
    if (packed & 0xC0C0) return;
    bounds_grid(m, (int)packed);
}
