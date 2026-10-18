/* scee::Psp3Gu::LoadMatrix(m) - copy a 64-byte matrix into the live matrix slot
   and mark the active slot dirty.  Retail takes the source matrix in $a0 and
   reads the slot number from the gu state, so only one register argument is
   consumed.
   The copy is four 128-bit moves (lq/sq), i.e. the EE u_long128 unit. */
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

void LoadMatrix__Q34scee3Psp3GumPC14ScePspFMatrix4(const ScePspFMatrix4 *m,
                                                   unsigned int unused) {
    u_long128 *destination = (u_long128 *)gu_state_01F77900.current_matrix;
    const u_long128 *source = (const u_long128 *)m;
    (void)unused;
    destination[0] = source[0];
    destination[1] = source[1];
    destination[2] = source[2];
    destination[3] = source[3];
    gu_state_01F77900.dirty[gu_state_01F77900.matrix_mode] = 1;
}
