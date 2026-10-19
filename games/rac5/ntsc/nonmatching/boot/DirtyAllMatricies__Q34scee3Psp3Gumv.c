/* scee::Psp3Gu::DirtyAllMatricies() - flag every cached matrix slot dirty.
   Retail store order is slot 5 first, then slots 0..4. */
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

void DirtyAllMatricies__Q34scee3Psp3Gumv(void) {
    gu_state_01F77900.dirty[0] = 1;
    gu_state_01F77900.dirty[1] = 1;
    gu_state_01F77900.dirty[2] = 1;
    gu_state_01F77900.dirty[3] = 1;
    gu_state_01F77900.dirty[4] = 1;
    gu_state_01F77900.dirty[5] = 1;
}
