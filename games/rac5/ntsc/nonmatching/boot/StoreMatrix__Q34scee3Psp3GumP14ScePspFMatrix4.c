/* scee::Psp3Gu::StoreMatrix(destination) - copy the live matrix out to caller
   storage.  Retail takes the destination in $a0 and the slot number is unused. */
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

void StoreMatrix__Q34scee3Psp3GumP14ScePspFMatrix4(ScePspFMatrix4 *destination,
                                                   unsigned int unused) {
    const u_long128 *source = (const u_long128 *)gu_state_01F77900.current_matrix;
    u_long128 *out = (u_long128 *)destination;
    (void)unused;
    out[0] = source[0];
    out[1] = source[1];
    out[2] = source[2];
    out[3] = source[3];
}
