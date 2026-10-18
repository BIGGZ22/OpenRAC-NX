typedef struct {
    char dirty;
    char pad01[3];
    float f04;
    float f08;
    int f0C;
    float f10;
} tFOG;

extern tFOG g_Fog[];

void FOG_SetProperties__FUiffUif(unsigned int index, int a1, float v1, float v2,
                                 unsigned int mode, float v3) {
    tFOG *fog = &g_Fog[index];

    fog->dirty = 1;
    if (v3 < fog->f10) {
        fog->f04 = v1;
        fog->f08 = v2;
        fog->f0C = a1;
        fog->f10 = v3;
    }
}
