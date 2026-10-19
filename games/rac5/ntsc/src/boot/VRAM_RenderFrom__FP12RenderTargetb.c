typedef struct {
    int field0;
} RenderTarget;

extern void TEXMGR_SetActiveTextureFromRenderTarget__FP12RenderTargetb(RenderTarget *target, int flag);
extern void TEXMGR_Soil__Fv(void);

void VRAM_RenderFrom__FP12RenderTargetb(RenderTarget *target, int flag) {
    TEXMGR_SetActiveTextureFromRenderTarget__FP12RenderTargetb(target, flag);
    TEXMGR_Soil__Fv();
}
