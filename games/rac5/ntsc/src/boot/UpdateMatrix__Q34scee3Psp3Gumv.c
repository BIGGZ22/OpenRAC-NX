/* w12/b21 — UpdateMatrix__Q34scee3Psp3Gumv
 * Reconstructed from the retail disassembly: flush the per-slot dirty
 * matrices of the Gum context into sceGu, then run the matrix update.
 * Context at 0x01F77900: +0x04 stack index, +0x08 dirty bytes, +0x10 current
 * matrix, +0x14 matrix stack.
 */
typedef struct MATRIX4 {
    float m[16];
} MATRIX4;

typedef struct GUM_CONTEXT {
    unsigned char unk0;        /* 0x00 */
    unsigned char pad0[3];
    int index;                 /* 0x04 */
    unsigned char dirty[6];    /* 0x08 */
    unsigned char pad1[2];
    MATRIX4 *current;          /* 0x10 */
    MATRIX4 *stack[4];         /* 0x14 */
} GUM_CONTEXT;

extern GUM_CONTEXT gum;        /* 0x01F77900 */

void SetMatrix(int type, MATRIX4 *matrix);
void SYSTEM_ScreenMatrixIsDirty(void);
void SYSTEM_ThroughmodeMatrixIsDirty(void);
void SYSTEM_UpdateMatricies(void);

void UpdateMatrix__Q34scee3Psp3Gumv(void) {
    float *value;

    if (gum.dirty[gum.index])
        gum.stack[gum.index] = gum.current;

    if (gum.dirty[0]) {
        if (gum.unk0 != 0) {
            value = (float *)gum.stack[0];
            if (0.0f < *value)
                *value = -*value;
            SetMatrix(0, gum.stack[0]);
        } else {
            SetMatrix(0, gum.stack[0]);
        }
    }

    if (gum.dirty[3])
        SetMatrix(3, gum.stack[3]);
    if (gum.dirty[1])
        SetMatrix(1, gum.stack[1]);
    if (gum.dirty[2])
        SetMatrix(2, gum.stack[2]);
    if (gum.dirty[4])
        SYSTEM_ScreenMatrixIsDirty();
    if (gum.dirty[5])
        SYSTEM_ThroughmodeMatrixIsDirty();

    SYSTEM_UpdateMatricies();

    gum.dirty[0] = 0;
    gum.dirty[1] = 0;
    gum.dirty[2] = 0;
    gum.dirty[3] = 0;
    gum.dirty[4] = 1;
    gum.dirty[5] = 1;
}
