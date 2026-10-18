extern int vram_viewport_x;
extern int vram_viewport_y;
extern int vram_viewport_w;
extern int vram_viewport_h;

void VRAM_GetViewport__FPiN30(int *x, int *y, int *w, int *h) {
    *x = vram_viewport_x;
    *y = vram_viewport_y;
    *w = vram_viewport_w;
    *h = vram_viewport_h;
}
