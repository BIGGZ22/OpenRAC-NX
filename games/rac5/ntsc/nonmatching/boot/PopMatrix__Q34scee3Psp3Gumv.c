/* scee::Psp3Gu::PopMatrix() - drop one matrix stack level and flag it dirty. */
typedef struct {
    float m[16];
} __attribute__((aligned(16))) ScePspFMatrix4;

struct GuState {
    unsigned char mirroring;  /* 0x00 */
    unsigned char pad01;      /* 0x01 */
    unsigned char pad02;      /* 0x02 */
    unsigned char pad03;      /* 0x03 */
    int matrix_mode;          /* 0x04 */
    unsigned char dirty[8];   /* 0x08 */
    ScePspFMatrix4 *current_matrix; /* 0x10 */
    void *matrix_stack[4];    /* 0x14 */
};
extern struct GuState gu_state_01F77900; /* 0x01F77900 */

void PopMatrix__Q34scee3Psp3Gumv(void) {
    gu_state_01F77900.current_matrix = gu_state_01F77900.current_matrix - 1;
    gu_state_01F77900.dirty[gu_state_01F77900.matrix_mode] = 1;
}
