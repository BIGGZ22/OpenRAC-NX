typedef struct TEXMGR_ALPHA_COLOR_TOLERANCE tTEXMGR_ALPHA_COLOR_TOLERANCE;

extern int TEXMGR_LoadTextureFromMemory__FPCUcUibPbT2N22P29tTEXMGR_ALPHA_COLOR_TOLERANCE(
    const unsigned char *data, unsigned int size, int a, unsigned char *b,
    int c, int d, int e, int f);

int TEXMGR_LoadTextureFromMemory__FPCUcUibN22P29tTEXMGR_ALPHA_COLOR_TOLERANCE(
    const unsigned char *data, unsigned int size, int c, int d, int e, int f) {
    unsigned char local = 0;
    return TEXMGR_LoadTextureFromMemory__FPCUcUibPbT2N22P29tTEXMGR_ALPHA_COLOR_TOLERANCE(
        data, size, 0, &local, c, d, e, f);
}
