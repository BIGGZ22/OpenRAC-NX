/* scee::Psp::Gum::SetMatrixStack - forward the six moving-model stack sizes.
   The global state block lives at 0x01F77900 (+0x04 matrix mode, +0x08 dirty
   flags, +0x10 live matrix, +0x14 six stack bases).  Each stack base is the
   still-unadvanced cursor of the previous segment, gated on its own size; the
   top slot is seeded from the pinned screen matrix at 0x01EF5670. */

typedef unsigned int u_long128 __attribute__((mode(TI)));

typedef struct {
    float m[16];
} ScePspFMatrix4;

struct GuState {
    unsigned char mirroring;          /* 0x00 */
    unsigned char pad01[3];
    int matrix_mode;                  /* 0x04 */
    unsigned char dirty[8];           /* 0x08 */
    ScePspFMatrix4 *current_matrix;   /* 0x10 */
    ScePspFMatrix4 *matrix_stack[6];  /* 0x14 */
};

extern struct GuState gu_state_01F77900;   /* 0x01F77900 */
extern u_long128 gu_screen_matrix_01EF5670[4];
extern void LoadIdentity__Q34scee3Psp3Gumv(void);

void SetMatrixStack__Q34scee3Psp3GumP14ScePspFMatrix4UiUiUiUiUiUi(
    ScePspFMatrix4 *matrices, unsigned int n0, unsigned int n1,
    unsigned int n2, unsigned int n3, unsigned int n4, unsigned int n5) {
    unsigned int base = ((unsigned int)matrices) & 0x1FFFFFFF;
    unsigned int p1 = base + (n0 << 6);
    unsigned int p2 = p1 + (n1 << 6);
    unsigned int p3 = p2 + (n2 << 6);
    unsigned int p4 = p3 + (n3 << 6);
    unsigned int p5 = p4 + (n4 << 6);
    int i;
    /* every call to gu_state is made through the same lvalue so the base is
       loaded once. */
    gu_state_01F77900.matrix_stack[0] = n0 ? (ScePspFMatrix4 *)base : 0;
    gu_state_01F77900.matrix_stack[1] = n1 ? (ScePspFMatrix4 *)p1 : 0;
    gu_state_01F77900.matrix_stack[2] = n2 ? (ScePspFMatrix4 *)p2 : 0;
    gu_state_01F77900.matrix_stack[3] = n3 ? (ScePspFMatrix4 *)p3 : 0;
    gu_state_01F77900.matrix_stack[4] = n4 ? (ScePspFMatrix4 *)p4 : 0;
    gu_state_01F77900.matrix_stack[5] = n5 ? (ScePspFMatrix4 *)p5 : 0;

    gu_state_01F77900.dirty[0] = n0 != 0;
    gu_state_01F77900.dirty[1] = n1 != 0;
    gu_state_01F77900.dirty[2] = n2 != 0;
    gu_state_01F77900.dirty[3] = n3 != 0;
    gu_state_01F77900.dirty[4] = n4 != 0;
    gu_state_01F77900.dirty[5] = n5 != 0;

    for (i = 0; i < 6; i++) {
        if (gu_state_01F77900.dirty[i]) {
            gu_state_01F77900.current_matrix = gu_state_01F77900.matrix_stack[i];
            LoadIdentity__Q34scee3Psp3Gumv();
        }
    }

    {
        u_long128 *dst = (u_long128 *)gu_state_01F77900.matrix_stack[5];
        const u_long128 *src = gu_screen_matrix_01EF5670;
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
    }
    gu_state_01F77900.matrix_mode = 2;
    gu_state_01F77900.current_matrix = gu_state_01F77900.matrix_stack[2];
}
