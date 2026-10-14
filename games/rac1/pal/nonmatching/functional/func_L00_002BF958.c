/* functional: medium. Draws up to three glow quads (slots flagged in D_L00_001618E8): per slot the alpha comes from func_001FA898 of D_L00_001618B8[i], the 3-row frame is built from D_L00_001DD3D0[i] (cross products with the camera up vector D_0013E633+0x10AD) and the four corners from D_L00_001DC220 scaled by D_L00_001618D8[i]. */
typedef struct { float f[4]; } V_2BF958 __attribute__((aligned(16)));
typedef struct {
    V_2BF958 corner[4];
    int color[4];
    int unk50, unk54, unk58;
    float unk5C, unk60;
    int unk64;
    float unk68, unk6C;
    long unk70, tex, unk80, unk88;
} Q_2BF958;
extern void vmul_2BF958(float, void *, void *) __asm__("func_001F9C30");
extern int D_L00_001618E8_2BF958[] __asm__("D_L00_001618E8");
extern float D_L00_001618B8_2BF958[] __asm__("D_L00_001618B8");
extern float D_L00_001618D8_2BF958[] __asm__("D_L00_001618D8");
extern V_2BF958 D_L00_001DD3A0_2BF958[] __asm__("D_L00_001DD3A0");
extern V_2BF958 D_L00_001DD3D0_2BF958[] __asm__("D_L00_001DD3D0");
extern V_2BF958 D_L00_001DC220_2BF958[] __asm__("D_L00_001DC220");
extern int D_L00_00161830_2BF958 __asm__("D_L00_00161830");
extern int D_L00_00161834_2BF958 __asm__("D_L00_00161834");

void func_L00_002BF958(void *o) {
    Q_2BF958 quad;
    V_2BF958 scaled, axis, negativeUp, vertex0, vertex1;
    int color;
    int i, j, c;

    quad.tex = func_001F4868_BEF58(8);
    quad.unk88 = 0x8000000048L;
    quad.unk80 = 0xFF9000000260L;
    quad.unk5C = 1.0f;
    quad.unk70 = 0;
    quad.unk50 = 0;
    quad.unk54 = 0;
    quad.unk58 = 0;
    quad.unk60 = 1.0f;
    quad.unk64 = 0;
    quad.unk68 = 1.0f;
    if (((unsigned char *)D_0013E633)[2] != 0) {
        c = D_L00_00161834_2BF958;
    } else {
        c = D_L00_00161830_2BF958;
    }
    color = c & 0xFFFFFF;
    for (i = 0; i < 3; i++) {
        if (D_L00_001618E8_2BF958[i] != 0) {
            unsigned int col = ((unsigned int)func_001FA898_r(D_L00_001618B8_2BF958[i]) << 24) | color;
            quad.color[0] = col;
            quad.color[1] = col;
            quad.color[2] = col;
            quad.color[3] = col;
            vertex0 = D_L00_001DD3A0_2BF958[i];
            scaled = D_L00_001DD3D0_2BF958[i];
            vmul_2BF958(-1.0f, &negativeUp, D_0013E633 + 0x10AD);
            func_001F9CA0(&axis, &scaled, &negativeUp);
            func_L00_001FF4B0(&axis, &axis, 1.0f);
            func_001F9CA0(&negativeUp, &axis, &scaled);
            for (j = 0; j < 4; j++) {
                vmul_2BF958(D_L00_001618D8_2BF958[i], &vertex1, &D_L00_001DC220_2BF958[j]);
                func_001F9EE8(&quad.corner[j], &vertex1, &scaled);
            }
            func_L00_001FD1D8(&quad, 0, 0);
        }
    }
}
