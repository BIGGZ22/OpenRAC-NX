/* functional: medium. Switch on kind-0x1C writing a float into *out and returning 1 (known kinds) or 0; kinds 0x1C,0x1D,0x1F store -1.3090 when D_0013FEF8 > 25.0; kind 0x37 stores -0.9599; includes pieces func_L00_0020DCB4 and the shared return-1 at 0x20DCAC. */
extern float D_0013FEF8;

int func_L00_0020DC68(int kind, float *out) {
    *(int *)out = 0;
    switch (kind) {
    case 0x1C:
    case 0x1D:
    case 0x1F:
        if (25.0f < D_0013FEF8) {
            *out = -1.3089969f;
        }
        return 1;
    case 0x31:
    case 0x32:
    case 0x4A:
    case 0x4B:
    case 0x50:
    case 0x60:
    case 0x6D:
    case 0x6E:
        return 1;
    case 0x37:
        *out = -0.9599311f;
        return 1;
    default:
        return 0;
    }
}
