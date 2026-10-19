/* w12/b21 — DrawRenderTarget__FUiUiP12RenderTarget
 * Reconstructed from the retail disassembly. Clears a render target with a
 * border, then blits it. Globals: g_displist pointer at 0x01F78BC4,
 * gRT_BackBuffer 0x01EF69F8.
 */
typedef struct RenderTarget {
    int unk0;
    int unk4;
    unsigned short width;   /* 0x08 */
    unsigned short height;  /* 0x0A */
} RenderTarget;

typedef struct DLIB_DISPLIST DLIB_DISPLIST;

extern DLIB_DISPLIST *g_displist;      /* 0x01F78BC4 */
extern RenderTarget gRT_BackBuffer;    /* 0x01EF69F8 */

void VRAM_RenderFrom(RenderTarget *target, int flag);
void VRAM_RenderTo(RenderTarget *target, unsigned int state);
void dlib_displist_set_blend_state(DLIB_DISPLIST *dl, unsigned long state);
void UTIL_DrawFlatShadedRect(short x, short y, short w, short h,
                             unsigned int color0, unsigned int color1);
void UTIL_DrawTexturedRect(short x, short y, short w, short h,
                           float u0, float v0, float u1, float v1,
                           unsigned int color0, unsigned int color1);

void DrawRenderTarget__FUiUiP12RenderTarget(unsigned int x, unsigned int y,
                                            RenderTarget *target) {
    DLIB_DISPLIST *dl = g_displist;

    VRAM_RenderFrom(target, 1);
    VRAM_RenderTo(&gRT_BackBuffer, 0);
    dlib_displist_set_blend_state(dl, 0x8000000044UL);

    UTIL_DrawFlatShadedRect(x - 2, y - 2, target->width + 4,
                            target->height + 4, 0xFF00CCFF, 0);
    UTIL_DrawFlatShadedRect(x, y, target->width, target->height,
                            0xFF005500, 0);
    UTIL_DrawTexturedRect(x, y, target->width, target->height,
                          0.0f, 0.0f,
                          (float)target->width, (float)target->height,
                          0xFFFFFFFF, 0x00FFFFFF);
}
