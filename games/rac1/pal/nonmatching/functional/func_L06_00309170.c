/* functional: high. The NTSC decomp's matched C for the same instructions (FUN_L06_00307d30), carried to PAL's
   names by OpenRAC's tools/port.py. Not a PAL match: PAL's compiler gives our 1680 bytes for retail's 1664 bytes. */
void func_L06_00309088(unsigned char *a, unsigned char *m);
typedef int OvlQuad __attribute__((mode(TI)));
extern void func_L06_003089B8(void);
extern float D_L06_00167640_09170[] __asm__("D_L06_00167640");
extern float D_0015EE6C MACRO_ADDR;
extern short D_L06_0016236C;
extern short D_L06_00162370;
extern short D_L06_00162374;
extern short D_L06_00162378;
extern short D_L06_0016237C;
extern unsigned char D_001414F4 NOT_SDA;
extern unsigned char D_0015EEB4 MACRO_ADDR;
extern unsigned char D_0014C150[];
extern short D_0015EE84;
extern short D_L06_00160058;
extern float func_001FA748(float, float);
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern unsigned int func_001FA8A8_09170(float, int, int) __asm__("func_001FA8A8");
extern int func_001FA898(float);
extern float func_001F9D10(void *, void *);
extern float func_00214158(void);
extern void func_001F49B0_09170(int, int) __asm__("func_001F49B0");
extern void func_L00_001FF240(void *, void *, void *);
extern void func_L01_00286530(void *, void *, int, unsigned int, int, int, int, float);
static inline unsigned int or_blue_channel(int blue, unsigned int color) {
    return blue * 0x10000 | color;
}

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/overlays/shared/gameplay/vendor/00307578.c, FUN_L06_00307d30. */
void func_L06_00309170(char *moby) {
    float *data = *(float **)(moby + 0x78);
    int *idata = (int *)data;
    float pos[4];
    float offset[3];
    float result[4];
    float angle;
    int value;
    unsigned int color;
    unsigned int color_channels;
    int i;

    if (*(int *)data == -1) return;
    data[10] = func_001FA748(data[10], (360.0f / (*(float *)&D_L06_00162374)) * 0.017453292f * D_0015EE6C);
    *(unsigned int *)(moby + 0x90) = func_001FA8A8_09170((func_001F9F90(data[10]) + 1.0f) * 0.5f, 0x80464646, 0x80828282);
    switch ((unsigned char)moby[0x20]) {
    case 0:
        idata[5] = idata[4];
        data[6] = func_00214158();
        data[10] = func_00214158();
        moby[0x20] = 1;
        data[9] = (*(float *)&D_L06_0016237C);
        return;
    case 1:
        *(OvlQuad *)pos = *(OvlQuad *)(moby + 0x10);
        pos[2] += 2.75f;
        if (func_001F9D10(D_L06_00167640_09170, moby + 0x10) < 32.0f) {
            data[9] += D_0015EE6C + D_0015EE6C;
            if ((*(float *)&D_L06_0016237C) < data[9]) data[9] = (*(float *)&D_L06_0016237C);
        } else if (36.0f < func_001F9D10(D_L06_00167640_09170, moby + 0x10)) {
            data[9] -= D_0015EE6C + D_0015EE6C;
            if (data[9] < 0.0f) data[9] = 0.0f;
        }
        if (0.0f < data[9]) {
            func_001F49B0_09170((int)func_L06_003089B8, (int)moby);
            data[6] = func_001FA748(data[6], (360.0f / (*(float *)&D_L06_00162370)) * 0.017453292f * D_0015EE6C);
            pos[2] += (*(float *)&D_L06_0016236C) * func_001F9F90(data[6]);
            value = func_001FA898((*(float *)&D_L06_00162378) * func_001F9F90(data[10]));
            color_channels = (value / 2 + 120) * 0x100 | 0x20000000U;
            color_channels = or_blue_channel(value / 3 + 13, color_channels);
            color = color_channels | (value + 150U);
            if (D_001414F4 == 1) {
                if (idata[5] < 10) {
                    func_L01_00286530(pos, moby, idata[5] + 2, color, 0x7f, 4, 0xff, 0.5f);
                } else {
                    if (D_0015EEB4) {
                        angle = func_001FA748(*(float *)(moby + 0x48), 1.5707964f);
                        offset[0] = func_001F9F90(angle) * 0.15f;
                        angle = func_001FA748(*(float *)(moby + 0x48), 1.5707964f);
                        offset[1] = func_001F9FA8(angle) * 0.15f;
                        offset[2] = 0.0f;
                        func_L00_001FF240(result, offset, pos);
                        func_L01_00286530(offset, moby, idata[5] / 10 + 2, color, 0x7f, 4, 0xff, 0.5f);
                        angle = func_001FA748(*(float *)(moby + 0x48), 1.5707964f);
                        offset[0] = func_001F9F90(angle) * -0.4f;
                        angle = func_001FA748(*(float *)(moby + 0x48), 1.5707964f);
                        offset[1] = func_001F9FA8(angle) * -0.4f;
                        offset[2] = 0.0f;
                        func_L00_001FF240(result, offset, pos);
                        func_L01_00286530(offset, moby, idata[5] % 10 + 2, color, 0x7f, 4, 0xff, 0.5f);
                    } else {
                        angle = func_001FA748(*(float *)(moby + 0x48), 1.5707964f);
                        offset[0] = func_001F9F90(angle) * -0.4f;
                        angle = func_001FA748(*(float *)(moby + 0x48), 1.5707964f);
                        offset[1] = func_001F9FA8(angle) * -0.4f;
                        offset[2] = 0.0f;
                        func_L00_001FF240(result, offset, pos);
                        func_L01_00286530(offset, moby, idata[5] / 10 + 2, color, 0x7f, 4, 0xff, 0.5f);
                        angle = func_001FA748(*(float *)(moby + 0x48), 1.5707964f);
                        offset[0] = func_001F9F90(angle) * 0.15f;
                        angle = func_001FA748(*(float *)(moby + 0x48), 1.5707964f);
                        offset[1] = func_001F9FA8(angle) * 0.15f;
                        offset[2] = 0.0f;
                        func_L00_001FF240(result, offset, pos);
                        func_L01_00286530(offset, moby, idata[5] % 10 + 2, color, 0x7f, 4, 0xff, 0.5f);
                    }
                }
            }
        }
        if (idata[5] < 1 || D_0014C150[(unsigned char)moby[0xb0] + (*(int *)&D_0015EE84) * 16] == 0xff) {
            for (i = 0; i < 4; i++) {
                if (idata[i] != -1) func_L06_00309088((unsigned char *)moby, (unsigned char *)((*(int *)&D_L06_00160058) + idata[i] * 0x100));
            }
            moby[0x20] = 2;
        }
        break;
    case 2:
        data[9] -= D_0015EE6C + D_0015EE6C;
        if (data[9] < 0.0f) data[9] = 0.0f;
        if (0.0f < data[9]) func_001F49B0_09170((int)func_L06_003089B8, (int)moby);
        return;
    default:
        return;
    }
}
