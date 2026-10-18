typedef struct { float m[16]; } MATRIX4;

typedef struct {
    char pad00[4];
    int offset;
    char dirty;
    char pad09[7];
    MATRIX4 *matrix;
} tGumState;

extern tGumState gGumState; /* 0x01F77900 */

void LoadMatrix__Q34scee3Psp3GumPC7MATRIX4(const MATRIX4 *matrix) {
    MATRIX4 *destination = gGumState.matrix;

    *destination = *matrix;
    *(char *)((char *)&gGumState + gGumState.offset + 8) = 1;
}
