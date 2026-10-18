typedef struct {
    unsigned int flags;   /* 0x00 */
    void *fn;             /* 0x04 */
    short s08;            /* 0x08 */
    char b0A;             /* 0x0A */
    char b0B;             /* 0x0B */
    int w0C;              /* 0x0C */
} PAR_System;

extern int PAR_defaultCallbacks[4]; /* 0x01F1E968 */

void PAR_CreateSystemInternal__FP10PAR_SystemUiPFPviPC9PAR_GroupPC29PAR_UpdateAndTesselateContext_iiiiPFP9PAR_Group_v(
    PAR_System *sys, unsigned int flags, void *fn, int a, int b, int c, int d) {
    sys->w0C = d;
    if (c == -1)
        c = PAR_defaultCallbacks[flags & 3];
    sys->fn = fn;
    sys->s08 = a;
    sys->b0A = b;
    sys->b0B = c;
    sys->flags = flags;
}
