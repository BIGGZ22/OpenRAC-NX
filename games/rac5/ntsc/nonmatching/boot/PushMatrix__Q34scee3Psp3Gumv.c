/* scee::Psp3Gu::PushMatrix() - duplicate the live matrix one slot up. */
typedef unsigned int u_long128 __attribute__((mode(TI)));

typedef struct {
    float m[16];
} ScePspFMatrix4;

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

void PushMatrix__Q34scee3Psp3Gumv(void) {
    ScePspFMatrix4 *matrix = gu_state_01F77900.current_matrix;
    u_long128 *destination = (u_long128 *)&matrix[1];
    const u_long128 *source = (const u_long128 *)&matrix[0];
    destination[0] = source[0];
    destination[1] = source[1];
    destination[2] = source[2];
    destination[3] = source[3];
    gu_state_01F77900.current_matrix = &matrix[1];
}
