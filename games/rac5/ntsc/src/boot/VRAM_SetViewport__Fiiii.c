/* Candidate only: the full-program build and compiler profile are not qualified. */
extern int g_viewport_x;
extern int g_viewport_y;
extern int g_viewport_w;
extern int g_viewport_h;

void VRAM_SetViewport__Fiiii(int x, int y, int w, int h) {
    g_viewport_x = x;
    g_viewport_y = y;
    g_viewport_w = w;
    g_viewport_h = h;
}
