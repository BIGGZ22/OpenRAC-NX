/* scee::Psp3Gu::MatrixMode(slot) - save the live matrix pointer into the slot
   being left and restore the one belonging to the slot being entered. */
struct GuState {
    unsigned char mirroring;  /* 0x00 */
    unsigned char pad01;      /* 0x01 */
    unsigned char pad02;      /* 0x02 */
    unsigned char pad03;      /* 0x03 */
    int matrix_mode;          /* 0x04 */
    unsigned char dirty[8];   /* 0x08 */
    void *current_matrix;     /* 0x10 */
    void *matrix_stack[4];    /* 0x14 */
};
extern struct GuState gu_state_01F77900; /* 0x01F77900 */

void MatrixMode__Q34scee3Psp3GumQ44scee3Psp2Gu11EMatrixType(unsigned int slot) {
    gu_state_01F77900.matrix_stack[gu_state_01F77900.matrix_mode] = gu_state_01F77900.current_matrix;
    gu_state_01F77900.matrix_mode = slot;
    gu_state_01F77900.current_matrix = gu_state_01F77900.matrix_stack[slot];
}
